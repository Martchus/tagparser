#include "oggcontainer.h"

#include "tagparser/mediafileinfo.h"
#include "tagparser/backuphelper.h"

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
    GenericContainer<MediaFileInfo, VorbisComment, OggStream, OggPage>(fileInfo, startOffset),//AbstractContainer(stream, startOffset)
    m_iterator(fileInfo.stream(), startOffset, fileInfo.size()),
    m_validateChecksums(false)
{}

/*!
 * \brief Destroys the container.
 */
OggContainer::~OggContainer()
{}

void OggContainer::internalParseHeader()
{
    static const string context("parsing OGG bitstream header");
    // iterate through pages using OggIterator helper class
    try {
        uint32 pageSequenceNumber = 0;
        // ensure iterator is setup properly
        for(m_iterator.removeFilter(), m_iterator.reset(); m_iterator; m_iterator.nextPage()) {
            const OggPage &page = m_iterator.currentPage();
            if(m_validateChecksums) {
                if(page.checksum() != OggPage::computeChecksum(stream(), page.startOffset())) {
                    addNotification(NotificationType::Warning, "The denoted checksum of the OGG page at " + ConversionUtilities::numberToString(m_iterator.currentSegmentOffset()) + " does not match the computed checksum.", context);
                }
            }
            if(!m_streamsBySerialNo.count(page.streamSerialNumber())) {
                // new stream serial number recognized -> add new stream
                m_streamsBySerialNo[page.streamSerialNumber()] = m_tracks.size();
                m_tracks.emplace_back(new OggStream(*this, m_iterator.currentPageIndex()));
            }
            if(pageSequenceNumber != page.sequenceNumber()) {
                if(pageSequenceNumber != 0) {
                    addNotification(NotificationType::Warning, "Page is missing (page sequence number omitted).", context);
                    pageSequenceNumber = page.sequenceNumber();
                }
            } else {
                ++pageSequenceNumber;
            }
        }
    } catch(TruncatedDataException &) {
        // thrown when page exceeds max size
        addNotification(NotificationType::Critical, "The OGG file is truncated.", context);
        throw;
    } catch(InvalidDataException &) {
        // thrown when first 4 byte do not match capture pattern
        addNotification(NotificationType::Critical, "Capture pattern \"OggS\" at " + ConversionUtilities::numberToString(m_iterator.currentSegmentOffset()) + " expected.", context);
        throw;
    }
}

void OggContainer::internalParseTags()
{
    parseTracks(); // tracks needs to be parsed because tags are stored at stream level
    for(auto &i : m_commentTable) {
        //fileInfo().stream().seekg(get<1>(i));
        m_iterator.setPageIndex(i.firstPageIndex);
        m_iterator.setSegmentIndex(i.firstSegmentIndex);
        m_tags[i.tagIndex]->parse(m_iterator);
        i.lastPageIndex = m_iterator.currentPageIndex();
        i.lastSegmentIndex = m_iterator.currentSegmentIndex();
    }
}

void OggContainer::ariseComment(vector<OggPage>::size_type pageIndex, vector<uint32>::size_type segmentIndex)
{
    m_commentTable.emplace_back(pageIndex, segmentIndex, m_tags.size());
    m_tags.emplace_back(make_unique<VorbisComment>());
}

void OggContainer::internalParseTracks()
{
    if(!areTracksParsed()) {
        parseHeader();
        static const string context("parsing OGG bit streams");
        for(auto &stream : m_tracks) {
            try { // try to parse header
                stream->parseHeader();
                if(stream->duration() > m_duration) {
                    m_duration = stream->duration();
                }
            } catch(Failure &) {
                addNotification(NotificationType::Critical, "Unable to parse stream at " + ConversionUtilities::numberToString(stream->startOffset()) + ".", context);
            }
        }
    }
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
        CopyHelper<65307> copy;
        auto commentTableIterator = m_commentTable.cbegin(), commentTableEnd = m_commentTable.cend();
        vector<uint64> updatedPageOffsets;
        uint32 pageSequenceNumber = 0;
        for(m_iterator.setStream(backupStream), m_iterator.removeFilter(), m_iterator.reset(); m_iterator; m_iterator.nextPage()) {
            const auto &currentPage = m_iterator.currentPage();
            auto pageSize = currentPage.totalSize();
            // check whether the Vorbis Comment is present in this Ogg page
            // -> then the page needs to be rewritten
            if(commentTableIterator != commentTableEnd
                    && m_iterator.currentPageIndex() >= commentTableIterator->firstPageIndex
                    && m_iterator.currentPageIndex() <= commentTableIterator->lastPageIndex
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
                        if((m_iterator.currentPageIndex() > commentTableIterator->firstPageIndex || segmentIndex >= commentTableIterator->firstSegmentIndex)
                                && (m_iterator.currentPageIndex() < commentTableIterator->lastPageIndex || segmentIndex <= commentTableIterator->lastSegmentIndex)) {
                            // prevent making the comment twice if it spreads over multiple pages
                            if(m_iterator.currentPageIndex() == commentTableIterator->firstPageIndex) {
                                // make Vorbis Comment segment
                                auto offset = buffer.tellp();
                                m_tags[commentTableIterator->tagIndex]->make(buffer);
                                newSegmentSizes.push_back(buffer.tellp() - offset);
                            }
                            if(m_iterator.currentPageIndex() > commentTableIterator->lastPageIndex
                                    || (m_iterator.currentPageIndex() == commentTableIterator->lastPageIndex && segmentIndex > commentTableIterator->lastSegmentIndex)) {
                                ++commentTableIterator;
                            }
                        } else {
                            // copy other segments unchanged
                            backupStream.seekg(segmentOffset);
                            copy.copy(backupStream, buffer, segmentSize);
                            newSegmentSizes.push_back(segmentSize);
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
                        copy.copy(backupStream, stream(), 27); // just copy header from original file
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
                        while(bytesLeft > 0 && segmentSizesWritten < 0xFF) {
                            while(bytesLeft >= 0xFF && segmentSizesWritten < 0xFF) {
                                stream().put(0xFF);
                                currentSize += 0xFF;
                                bytesLeft -= 0xFF;
                                ++segmentSizesWritten;
                            }
                            if(bytesLeft > 0 && segmentSizesWritten < 0xFF) {
                                // bytes left is here < 0xFF
                                stream().put(bytesLeft);
                                currentSize += bytesLeft;
                                bytesLeft = 0;
                                ++segmentSizesWritten;
                            }
                            if(bytesLeft == 0) {
                                // sizes for the segment have been written
                                // -> continue with next segment
                                if(++newSegmentSizesIterator != newSegmentSizesEnd) {
                                    bytesLeft = *newSegmentSizesIterator;
                                    continuePreviousSegment = false;
                                }
                            }
                        }
                        // there are no bytes left in the current segment; remove continue flag
                        if(bytesLeft == 0) {
                            continuePreviousSegment = false;
                        }
                        // page is full or all segment data has been covered
                        // -> write segment table size (segmentSizesWritten) and segment data
                        // -> seek back and write updated page segment number
                        stream().seekp(-1 - segmentSizesWritten, ios_base::cur);
                        stream().put(segmentSizesWritten);
                        stream().seekp(segmentSizesWritten, ios_base::cur);
                        // -> write actual page data
                        copy.copy(buffer, stream(), currentSize);
                        ++pageSequenceNumber;
                    }
                }
            } else {
                if(pageSequenceNumber != m_iterator.currentPageIndex()) {
                    // just update page sequence number
                    backupStream.seekg(currentPage.startOffset());
                    updatedPageOffsets.push_back(stream().tellp()); // memorize offset to update checksum later
                    copy.copy(backupStream, stream(), 27);
                    stream().seekp(-9, ios_base::cur);
                    writer().writeUInt32LE(pageSequenceNumber);
                    stream().seekp(5, ios_base::cur);
                    copy.copy(backupStream, stream(), pageSize - 27);
                } else {
                    // copy page unchanged
                    backupStream.seekg(currentPage.startOffset());
                    copy.copy(backupStream, stream(), pageSize);
                }
                ++pageSequenceNumber;
            }
        }
        // close backups stream; reopen new file as readable stream
        backupStream.close();
        fileInfo().close();
        fileInfo().open();
        // update checksums of modified pages
        for(auto offset : updatedPageOffsets) {
            OggPage::updateChecksum(fileInfo().stream(), offset);
        }
        // clear iterator
        m_iterator = OggIterator(fileInfo().stream(), startOffset(), fileInfo().size());
    } catch(OperationAbortedException &) {
        addNotification(NotificationType::Information, "Rewriting file to apply new tag information has been aborted.", context);
        BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, fileInfo().stream(), backupStream);
        m_iterator.setStream(fileInfo().stream());
        throw;
    } catch(ios_base::failure &ex) {
        addNotification(NotificationType::Critical, "IO error occured when rewriting file to apply new tag information.\n" + string(ex.what()), context);
        BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, fileInfo().stream(), backupStream);
        m_iterator.setStream(fileInfo().stream());
        throw;
    }
}

}
