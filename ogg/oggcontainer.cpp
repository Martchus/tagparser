#include "./oggcontainer.h"

#include "../mediafileinfo.h"
#include "../backuphelper.h"

#include <c++utilities/io/copy.h>
#include <c++utilities/misc/memory.h>

using namespace std;
using namespace IoUtilities;

namespace Media {

/*!
 * \class Media::OggContainer
 * \brief Implementation of Media::AbstractContainer for OGG files.
 */

/*!
 * \brief Constructs a new container for the specified \a stream at the specified \a startOffset.
 */
OggContainer::OggContainer(MediaFileInfo &fileInfo, uint64 startOffset) :
    GenericContainer<MediaFileInfo, VorbisComment, OggStream, OggPage>(fileInfo, startOffset),
    m_iterator(fileInfo.stream(), startOffset, fileInfo.size()),
    m_validateChecksums(false)
{}

OggContainer::~OggContainer()
{}

void OggContainer::reset()
{
    m_iterator.reset();
}

/*!
 * \brief Creates a new tag.
 * \sa AbstractContainer::createTag()
 * \remarks Tracks must be parsed before because tags are stored on track level!
 */
VorbisComment *OggContainer::createTag(const TagTarget &target)
{
    if(!target.isEmpty()) {
        // targets are not supported here, so the specified target should be empty
        // -> just be consistent with generic implementation here
        for(auto &tag : m_tags) {
            if(tag->target() == target && !tag->oggParams().removed) {
                return tag.get();
            }
        }
        for(auto &tag : m_tags) {
            if(tag->target() == target) {
                tag->oggParams().removed = false;
                return tag.get();
            }
        }
    } else if(VorbisComment *comment = tag(0)) {
        comment->oggParams().removed = false;
        return comment;
    } else if(!m_tags.empty()) {
        m_tags.front()->oggParams().removed = false;
        return m_tags.front().get();
    }

    // a new tag needs to be created
    // -> determine an appropriate track for the tag
    // -> just use the first Vorbis/Opus track
    // -> TODO: provide interface for specifying a specific track
    for(const auto &track : m_tracks) {
        switch(track->format().general) {
        case GeneralMediaFormat::Vorbis:
        case GeneralMediaFormat::Opus:
            // check whether start page has a valid value
            if(track->startPage() < m_iterator.pages().size()) {
                ariseComment(track->startPage(), static_cast<size_t>(-1), track->format().general);
                m_tags.back()->setTarget(target); // also for consistency
                return m_tags.back().get();
            } else {
                // TODO: error handling?
            }
        default:
            ;
        }
    }
    return nullptr;
}

VorbisComment *OggContainer::tag(size_t index)
{
    size_t i = 0;
    for(const auto &tag : m_tags) {
        if(!tag->oggParams().removed) {
            if(index == i) {
                return tag.get();
            }
            ++i;
        }
    }
    return nullptr;
}

size_t OggContainer::tagCount() const
{
    size_t count = 0;
    for(const auto &tag : m_tags) {
        if(!tag->oggParams().removed) {
            ++count;
        }
    }
    return count;
}

/*!
 * \brief Actually just flags the specified \a tag as removed and clears all assigned fields.
 *
 * This specialization is neccessary because completeley removing the tag whould also
 * remove the OGG parameter which are needed when appying the changes.
 *
 * \remarks Seems like common players aren't able to play Vorbis when no comment is present.
 *          So do NOT use this method to remove tags from Vorbis, just call removeAllFields() on \a tag.
 * \sa AbstractContainer::removeTag()
 */
bool OggContainer::removeTag(Tag *tag)
{
    for(auto &existingTag : m_tags) {
        if(static_cast<Tag *>(existingTag.get()) == tag) {
            existingTag->removeAllFields();
            existingTag->oggParams().removed = true;
            return true;
        }
    }
    return false;
}

/*!
 * \brief Actually just flags all tags as removed and clears all assigned fields.
 *
 * This specialization is neccessary because completeley removing the tag whould also
 * remove the OGG parameter which are needed when appying the changes.
 *
 * \remarks Seems like common players aren't able to play Vorbis when no comment is present.
 *          So do NOT use this method to remove tags from Vorbis, just call removeAllFields() on all tags.
 * \sa AbstractContainer::removeAllTags()
 */
void OggContainer::removeAllTags()
{
    for(auto &existingTag : m_tags) {
        existingTag->removeAllFields();
        existingTag->oggParams().removed = true;
    }
}

void OggContainer::internalParseHeader()
{
    static const string context("parsing OGG bitstream header");
    // iterate through pages using OggIterator helper class
    try {
        // ensure iterator is setup properly
        for(m_iterator.removeFilter(), m_iterator.reset(); m_iterator; m_iterator.nextPage()) {
            const OggPage &page = m_iterator.currentPage();
            if(m_validateChecksums) {
                if(page.checksum() != OggPage::computeChecksum(stream(), page.startOffset())) {
                    addNotification(NotificationType::Warning, "The denoted checksum of the OGG page at " + ConversionUtilities::numberToString(m_iterator.currentSegmentOffset()) + " does not match the computed checksum.", context);
                }
            }
            OggStream *stream;
            try {
                stream = m_tracks[m_streamsBySerialNo.at(page.streamSerialNumber())].get();
            } catch(const out_of_range &) {
                // new stream serial number recognized -> add new stream
                m_streamsBySerialNo[page.streamSerialNumber()] = m_tracks.size();
                m_tracks.emplace_back(make_unique<OggStream>(*this, m_iterator.currentPageIndex()));
                stream = m_tracks.back().get();
            }
            if(stream->m_currentSequenceNumber != page.sequenceNumber()) {
                if(stream->m_currentSequenceNumber) {
                    addNotification(NotificationType::Warning, "Page is missing (page sequence number omitted).", context);
                }
                stream->m_currentSequenceNumber = page.sequenceNumber() + 1;
            } else {
                ++stream->m_currentSequenceNumber;
            }
        }
    } catch(const TruncatedDataException &) {
        // thrown when page exceeds max size
        addNotification(NotificationType::Critical, "The OGG file is truncated.", context);
    } catch(const InvalidDataException &) {
        // thrown when first 4 byte do not match capture pattern
        addNotification(NotificationType::Critical, "Capture pattern \"OggS\" at " + ConversionUtilities::numberToString(m_iterator.currentSegmentOffset()) + " expected.", context);
    }
}

void OggContainer::internalParseTags()
{
    // tracks needs to be parsed before because tags are stored at stream level
    parseTracks();
    for(auto &comment : m_tags) {
        OggParameter &params = comment->oggParams();
        m_iterator.setPageIndex(params.firstPageIndex);
        m_iterator.setSegmentIndex(params.firstSegmentIndex);
        switch(params.streamFormat) {
        case GeneralMediaFormat::Vorbis:
            comment->parse(m_iterator);
            break;
        case GeneralMediaFormat::Opus:
            // skip header (has already been detected by OggStream)
            m_iterator.seekForward(8);
            comment->parse(m_iterator, true);
            break;
        default:
            addNotification(NotificationType::Critical, "Stream format not supported.", "parsing tags from OGG streams");
        }
        params.lastPageIndex = m_iterator.currentPageIndex();
        params.lastSegmentIndex = m_iterator.currentSegmentIndex();
    }
}

void OggContainer::ariseComment(std::size_t pageIndex, std::size_t segmentIndex, GeneralMediaFormat mediaFormat)
{
    m_tags.emplace_back(make_unique<VorbisComment>());
    m_tags.back()->oggParams().set(pageIndex, segmentIndex, mediaFormat);
}

void OggContainer::internalParseTracks()
{
    static const string context("parsing OGG stream");
    for(auto &stream : m_tracks) {
        try { // try to parse header
            stream->parseHeader();
            if(stream->duration() > m_duration) {
                m_duration = stream->duration();
            }
        } catch(const Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse stream at " + ConversionUtilities::numberToString(stream->startOffset()) + ".", context);
        }
    }
}

/*!
 * \brief Writes the specified \a comment with the given \a params to the specified \a buffer and
 *        adds the number of bytes written to \a newSegmentSizes.
 */
void makeVorbisCommentSegment(stringstream &buffer, CopyHelper<65307> &copyHelper, vector<uint32> &newSegmentSizes, VorbisComment *comment, OggParameter *params)
{
    auto offset = buffer.tellp();
    switch(params->streamFormat) {
    case GeneralMediaFormat::Vorbis:
        comment->make(buffer);
        break;
    case GeneralMediaFormat::Opus:
        ConversionUtilities::BE::getBytes(0x4F70757354616773u, copyHelper.buffer());
        buffer.write(copyHelper.buffer(), 8);
        comment->make(buffer, true);
        break;
    default:
        ;
    }
    newSegmentSizes.push_back(buffer.tellp() - offset);
}

void OggContainer::internalMakeFile()
{
    const string context("making OGG file");
    updateStatus("Prepare for rewriting OGG file ...");
    parseTags(); // tags need to be parsed before the file can be rewritten
    fileInfo().close();
    string backupPath;
    fstream backupStream;
    try {
        BackupHelper::createBackupFile(fileInfo().path(), backupPath, backupStream);
        // recreate original file
        fileInfo().stream().open(fileInfo().path(), ios_base::out | ios_base::binary | ios_base::trunc);

        // prepare iterating comments
        VorbisComment *currentComment;
        OggParameter *currentParams;
        auto tagIterator = m_tags.cbegin(), tagEnd = m_tags.cend();
        if(tagIterator != tagEnd) {
            currentParams = &(currentComment = tagIterator->get())->oggParams();
        } else {
            currentComment = nullptr;
            currentParams = nullptr;
        }

        // define misc variables
        CopyHelper<65307> copyHelper;
        vector<uint64> updatedPageOffsets;
        unordered_map<uint32, uint32> pageSequenceNumberBySerialNo;

        // iterate through all pages of the original file
        for(m_iterator.setStream(backupStream), m_iterator.removeFilter(), m_iterator.reset(); m_iterator; m_iterator.nextPage()) {
            const OggPage &currentPage = m_iterator.currentPage();
            const auto pageSize = currentPage.totalSize();
            uint32 &pageSequenceNumber = pageSequenceNumberBySerialNo[currentPage.streamSerialNumber()];
            // check whether the Vorbis Comment is present in this Ogg page
            if(currentComment
                    && m_iterator.currentPageIndex() >= currentParams->firstPageIndex
                    && m_iterator.currentPageIndex() <= currentParams->lastPageIndex
                    && !currentPage.segmentSizes().empty()) {
                // page needs to be rewritten (not just copied)
                // -> write segments to a buffer first
                stringstream buffer(ios_base::in | ios_base::out | ios_base::binary);
                vector<uint32> newSegmentSizes;
                newSegmentSizes.reserve(currentPage.segmentSizes().size());
                uint64 segmentOffset = m_iterator.currentSegmentOffset();
                vector<uint32>::size_type segmentIndex = 0;
                for(const auto segmentSize : currentPage.segmentSizes()) {
                    if(segmentSize) {
                        // check whether this segment contains the Vorbis Comment
                        if((m_iterator.currentPageIndex() >= currentParams->firstPageIndex && segmentIndex >= currentParams->firstSegmentIndex)
                                && (m_iterator.currentPageIndex() <= currentParams->lastPageIndex && segmentIndex <= currentParams->lastSegmentIndex)) {
                            // prevent making the comment twice if it spreads over multiple pages/segments
                            if(!currentParams->removed
                                    && ((m_iterator.currentPageIndex() == currentParams->firstPageIndex
                                         && m_iterator.currentSegmentIndex() == currentParams->firstSegmentIndex))) {
                                makeVorbisCommentSegment(buffer, copyHelper, newSegmentSizes, currentComment, currentParams);
                            }

                            // proceed with next comment?
                            if(m_iterator.currentPageIndex() > currentParams->lastPageIndex
                                    || (m_iterator.currentPageIndex() == currentParams->lastPageIndex && segmentIndex > currentParams->lastSegmentIndex)) {
                                if(++tagIterator != tagEnd) {
                                    currentParams = &(currentComment = tagIterator->get())->oggParams();
                                } else {
                                    currentComment = nullptr;
                                    currentParams = nullptr;
                                }
                            }
                        } else {
                            // copy other segments unchanged
                            backupStream.seekg(segmentOffset);
                            copyHelper.copy(backupStream, buffer, segmentSize);
                            newSegmentSizes.push_back(segmentSize);

                            // check whether there is a new comment to be inserted into the current page
                            if(m_iterator.currentPageIndex() == currentParams->lastPageIndex && currentParams->firstSegmentIndex == static_cast<size_t>(-1)) {
                                makeVorbisCommentSegment(buffer, copyHelper, newSegmentSizes, currentComment, currentParams);
                                // proceed with next comment
                                if(++tagIterator != tagEnd) {
                                    currentParams = &(currentComment = tagIterator->get())->oggParams();
                                } else {
                                    currentComment = nullptr;
                                    currentParams = nullptr;
                                }
                            }
                        }
                        segmentOffset += segmentSize;
                    }
                    ++segmentIndex;
                }

                // write buffered data to actual stream
                auto newSegmentSizesIterator = newSegmentSizes.cbegin(), newSegmentSizesEnd = newSegmentSizes.cend();
                bool continuePreviousSegment = false;
                if(newSegmentSizesIterator != newSegmentSizesEnd) {
                    uint32 bytesLeft = *newSegmentSizesIterator;
                    // write pages until all data in the buffer is written
                    while(newSegmentSizesIterator != newSegmentSizesEnd) {
                        // write header
                        backupStream.seekg(currentPage.startOffset());
                        updatedPageOffsets.push_back(stream().tellp()); // memorize offset to update checksum later
                        copyHelper.copy(backupStream, stream(), 27); // just copy header from original file
                        // set continue flag
                        stream().seekp(-22, ios_base::cur);
                        stream().put(currentPage.headerTypeFlag() & (continuePreviousSegment ? 0xFF : 0xFE));
                        continuePreviousSegment = true;
                        // adjust page sequence number
                        stream().seekp(12, ios_base::cur);
                        writer().writeUInt32LE(pageSequenceNumber);
                        stream().seekp(5, ios_base::cur);
                        int16 segmentSizesWritten = 0; // in the current page header only
                        // write segment sizes as long as there are segment sizes to be written and
                        // the max number of segment sizes (255) is not exceeded
                        uint32 currentSize = 0;
                        while(bytesLeft && segmentSizesWritten < 0xFF) {
                            while(bytesLeft >= 0xFF && segmentSizesWritten < 0xFF) {
                                stream().put(0xFF);
                                currentSize += 0xFF;
                                bytesLeft -= 0xFF;
                                ++segmentSizesWritten;
                            }
                            if(bytesLeft && segmentSizesWritten < 0xFF) {
                                // bytes left is here < 0xFF
                                stream().put(bytesLeft);
                                currentSize += bytesLeft;
                                bytesLeft = 0;
                                ++segmentSizesWritten;
                            }
                            if(!bytesLeft) {
                                // sizes for the segment have been written
                                // -> continue with next segment
                                if(++newSegmentSizesIterator != newSegmentSizesEnd) {
                                    bytesLeft = *newSegmentSizesIterator;
                                    continuePreviousSegment = false;
                                }
                            }
                        }

                        // there are no bytes left in the current segment; remove continue flag
                        if(!bytesLeft) {
                            continuePreviousSegment = false;
                        }

                        // page is full or all segment data has been covered
                        // -> write segment table size (segmentSizesWritten) and segment data
                        // -> seek back and write updated page segment number
                        stream().seekp(-1 - segmentSizesWritten, ios_base::cur);
                        stream().put(segmentSizesWritten);
                        stream().seekp(segmentSizesWritten, ios_base::cur);
                        // -> write actual page data
                        copyHelper.copy(buffer, stream(), currentSize);
                        ++pageSequenceNumber;
                    }
                }

            } else {
                if(pageSequenceNumber != m_iterator.currentPageIndex()) {
                    // just update page sequence number
                    backupStream.seekg(currentPage.startOffset());
                    updatedPageOffsets.push_back(stream().tellp()); // memorize offset to update checksum later
                    copyHelper.copy(backupStream, stream(), 27);
                    stream().seekp(-9, ios_base::cur);
                    writer().writeUInt32LE(pageSequenceNumber);
                    stream().seekp(5, ios_base::cur);
                    copyHelper.copy(backupStream, stream(), pageSize - 27);
                } else {
                    // copy page unchanged
                    backupStream.seekg(currentPage.startOffset());
                    copyHelper.copy(backupStream, stream(), pageSize);
                }
                ++pageSequenceNumber;
            }
        }

        fileInfo().reportSizeChanged(stream().tellp());

        // close backups stream; reopen new file as readable stream
        backupStream.close();
        fileInfo().close();
        fileInfo().stream().open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);

        // update checksums of modified pages
        for(auto offset : updatedPageOffsets) {
            OggPage::updateChecksum(fileInfo().stream(), offset);
        }

        // clear iterator
        m_iterator.clear(fileInfo().stream(), startOffset(), fileInfo().size());

    } catch(const OperationAbortedException &) {
        addNotification(NotificationType::Information, "Rewriting file to apply new tag information has been aborted.", context);
        BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, fileInfo().stream(), backupStream);
        m_iterator.setStream(fileInfo().stream());
        throw;

    } catch(const ios_base::failure &ex) {
        addNotification(NotificationType::Critical, "IO error occured when rewriting file to apply new tag information.\n" + string(ex.what()), context);
        BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, fileInfo().stream(), backupStream);
        m_iterator.setStream(fileInfo().stream());
        throw;
    }
}

}
