#include "./oggcontainer.h"

#include "../flac/flacmetadata.h"

#include "../backuphelper.h"
#include "../mediafileinfo.h"
#include "../progressfeedback.h"
#include "../tagtarget.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/io/copy.h>

#include <memory>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::OggVorbisComment
 * \brief Specialization of TagParser::VorbisComment for Vorbis comments inside an OGG stream.
 */

std::string_view OggVorbisComment::typeName() const
{
    switch (m_oggParams.streamFormat) {
    case GeneralMediaFormat::Flac:
        return "Vorbis comment (in FLAC stream)";
    case GeneralMediaFormat::Opus:
        return "Vorbis comment (in Opus stream)";
    case GeneralMediaFormat::Theora:
        return "Vorbis comment (in Theora stream)";
    default:
        return "Vorbis comment";
    }
}

/*!
 * \class TagParser::OggContainer
 * \brief Implementation of TagParser::AbstractContainer for OGG files.
 */

/*!
 * \brief Constructs a new container for the specified \a stream at the specified \a startOffset.
 */
OggContainer::OggContainer(MediaFileInfo &fileInfo, std::uint64_t startOffset)
    : GenericContainer<MediaFileInfo, OggVorbisComment, OggStream, OggPage>(fileInfo, startOffset)
    , m_iterator(fileInfo.stream(), startOffset, fileInfo.size())
    , m_validateChecksums(false)
{
}

OggContainer::~OggContainer()
{
}

void OggContainer::reset()
{
    m_iterator.reset();
}

/*!
 * \brief Creates a new tag.
 * \sa AbstractContainer::createTag()
 * \remarks
 *  - Tracks must be parsed before because tags are stored on track level!
 *  - The track can be specified via the \a target argument. However, only the first track of tracks() array of \a target is considered.
 *  - If tracks() array of \a target is empty, the first track/tag is picked.
 *  - Vorbis streams should always have a tag assigned; this method allows creation of a tag for Vorbis streams if none is present though.
 *  - FLAC streams should always have a tag assigned; this method does *not* allow creation of a tag for FLAC streams if none is present though.
 */
OggVorbisComment *OggContainer::createTag(const TagTarget &target)
{
    if (!target.tracks().empty()) {
        // return the tag for the first matching track ID
        for (auto &tag : m_tags) {
            if (!tag->target().tracks().empty() && tag->target().tracks().front() == target.tracks().front() && !tag->oggParams().removed) {
                return tag.get();
            }
        }
        // not tag found -> try to re-use a tag which has been flagged as removed
        for (auto &tag : m_tags) {
            if (!tag->target().tracks().empty() && tag->target().tracks().front() == target.tracks().front()) {
                tag->oggParams().removed = false;
                return tag.get();
            }
        }
    } else if (OggVorbisComment *comment = tag(0)) {
        // no track ID specified -> just return the first tag (if one exists)
        return comment;
    } else if (!m_tags.empty()) {
        // no track ID specified -> just return the first tag (try to re-use a tag which has been flagged as removed)
        m_tags.front()->oggParams().removed = false;
        return m_tags.front().get();
    }

    // a new tag needs to be created
    // -> determine an appropriate track for the tag
    // -> just use the first Vorbis/Opus track
    for (const auto &track : m_tracks) {
        if (target.tracks().empty() || target.tracks().front() == track->id()) {
            switch (track->format().general) {
            case GeneralMediaFormat::Vorbis:
            case GeneralMediaFormat::Opus:
                // check whether start page has a valid value
                if (track->startPage() < m_iterator.pages().size()) {
                    announceComment(track->startPage(), numeric_limits<size_t>::max(), false, track->format().general);
                    m_tags.back()->setTarget(target);
                    return m_tags.back().get();
                } else {
                    // TODO: error handling?
                }
                break;
            default:;
            }
            // TODO: allow adding tags to FLAC tracks (not really important, because a tag should always be present)
        }
    }
    return nullptr;
}

OggVorbisComment *OggContainer::tag(size_t index)
{
    size_t i = 0;
    for (const auto &tag : m_tags) {
        if (!tag->oggParams().removed) {
            if (index == i) {
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
    for (const auto &tag : m_tags) {
        if (!tag->oggParams().removed) {
            ++count;
        }
    }
    return count;
}

/*!
 * \brief Actually just flags the specified \a tag as removed and clears all assigned fields.
 *
 * This specialization is necessary because removing the tag completely would also
 * remove the OGG parameter which are needed when applying changes.
 *
 * \remarks Seems like common players aren't able to play Vorbis when no comment is present.
 *          So do NOT use this method to remove tags from Vorbis, just call Tag::removeAllFields() on \a tag.
 */
bool OggContainer::removeTag(Tag *tag)
{
    for (auto &existingTag : m_tags) {
        if (static_cast<Tag *>(existingTag.get()) == tag) {
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
 * This specialization is necessary because completeley removing the tag would also
 * remove the OGG parameter which are needed when applying the changes.
 *
 * \remarks Seems like common players aren't able to play Vorbis when no comment is present.
 *          So do NOT use this method to remove tags from Vorbis, just call removeAllFields() on all tags.
 * \sa AbstractContainer::removeAllTags()
 */
void OggContainer::removeAllTags()
{
    for (auto &existingTag : m_tags) {
        existingTag->removeAllFields();
        existingTag->oggParams().removed = true;
    }
}

void OggContainer::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing OGG bitstream header");
    bool pagesSkipped = false, continueFromHere = false;

    // iterate through pages using OggIterator helper class
    try {
        // ensure iterator is setup properly
        for (m_iterator.removeFilter(), m_iterator.reset(); m_iterator;
             continueFromHere ? [&] { continueFromHere = false; }() : m_iterator.nextPage()) {
            progress.stopIfAborted();
            const OggPage &page = m_iterator.currentPage();
            if (m_validateChecksums && page.checksum() != OggPage::computeChecksum(stream(), page.startOffset())) {
                diag.emplace_back(DiagLevel::Warning,
                    argsToString(
                        "The denoted checksum of the OGG page at ", m_iterator.currentSegmentOffset(), " does not match the computed checksum."),
                    context);
            }
            OggStream *stream;
            std::uint64_t lastNewStreamOffset = 0;
            if (const auto streamIndex = m_streamsBySerialNo.find(page.streamSerialNumber()); streamIndex != m_streamsBySerialNo.end()) {
                stream = m_tracks[streamIndex->second].get();
            } else {
                // new stream serial number recognized -> add new stream
                m_streamsBySerialNo[page.streamSerialNumber()] = m_tracks.size();
                m_tracks.emplace_back(make_unique<OggStream>(*this, m_iterator.currentPageIndex()));
                stream = m_tracks.back().get();
                lastNewStreamOffset = page.startOffset();
            }
            if (!pagesSkipped) {
                stream->m_size += page.dataSize();
            }
            if (stream->m_currentSequenceNumber != page.sequenceNumber()) {
                if (stream->m_currentSequenceNumber) {
                    diag.emplace_back(DiagLevel::Warning,
                        argsToString("Page of stream ", page.streamSerialNumber(), " missing; page sequence number ", stream->m_currentSequenceNumber,
                            " omitted at ", page.startOffset(), ", found ", page.sequenceNumber(), " instead."),
                        context);
                }
                stream->m_currentSequenceNumber = page.sequenceNumber() + 1;
            } else {
                ++stream->m_currentSequenceNumber;
            }

            // skip pages in the middle of a big file (still more than 100 MiB to parse) if no new track has been seen since the last 20 MiB
            if (!fileInfo().isForcingFullParse() && (fileInfo().size() - page.startOffset()) > (100 * 0x100000)
                && (page.startOffset() - lastNewStreamOffset) > (20 * 0x100000)) {
                if (m_iterator.resyncAt(fileInfo().size() - (20 * 0x100000))) {
                    const OggPage &resyncedPage = m_iterator.currentPage();
                    // prevent warning about missing pages by resetting the sequence number of all streams and invalidate the stream size
                    for (auto &trackStream : m_tracks) {
                        trackStream->m_currentSequenceNumber = 0;
                        trackStream->m_size = 0;
                    }
                    pagesSkipped = continueFromHere = true;
                    diag.emplace_back(DiagLevel::Information,
                        argsToString("Pages in the middle of the file (", dataSizeToString(resyncedPage.startOffset() - page.startOffset()),
                            ") have been skipped to improve parsing speed. Hence track sizes can not be computed. Maybe not even all tracks could be "
                            "detected. Force a full parse to prevent this."),
                        context);
                } else {
                    // abort if skipping pages didn't work
                    diag.emplace_back(DiagLevel::Critical,
                        "Unable to re-sync after skipping OGG pages in the middle of the file. Try forcing a full parse.", context);
                    return;
                }
            }
        }
    } catch (const TruncatedDataException &) {
        // thrown when page exceeds max size
        diag.emplace_back(DiagLevel::Critical, "The OGG file is truncated.", context);
    } catch (const InvalidDataException &) {
        // thrown when first 4 byte do not match capture pattern
        const auto expectedOffset = m_iterator.currentSegmentOffset();
        diag.emplace_back(DiagLevel::Critical, argsToString("Capture pattern \"OggS\" at ", expectedOffset, " expected."), context);
        if (m_iterator.resyncAt(expectedOffset)) {
            diag.emplace_back(DiagLevel::Warning,
                argsToString("Found next capture pattern \"OggS\" at ", m_iterator.currentPageOffset(), ". Skipped ",
                    m_iterator.currentPageOffset() - expectedOffset, " invalid bytes."),
                context);
            continueFromHere = true;
        } else {
            diag.emplace_back(DiagLevel::Critical,
                argsToString(
                    "Aborting after not being able to find any \"OggS\" capture patterns within 65307 bytes (from offset ", expectedOffset, ")."),
                context);
        }
    }
}

void OggContainer::internalParseTags(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    // tracks needs to be parsed before because tags are stored at stream level
    parseTracks(diag, progress);
    auto flags = VorbisCommentFlags::None;
    if (fileInfo().fileHandlingFlags() & MediaFileHandlingFlags::ConvertTotalFields) {
        flags += VorbisCommentFlags::ConvertTotalFields;
    }
    for (auto &comment : m_tags) {
        OggParameter &params = comment->oggParams();
        m_iterator.setPageIndex(params.firstPageIndex);
        m_iterator.setSegmentIndex(params.firstSegmentIndex);
        m_iterator.setFilter(m_iterator.currentPage().streamSerialNumber());
        switch (params.streamFormat) {
        case GeneralMediaFormat::Vorbis:
            comment->parse(m_iterator, flags, diag);
            break;
        case GeneralMediaFormat::Opus:
            // skip header (has already been detected by OggStream)
            m_iterator.ignore(8);
            comment->parse(m_iterator, flags | VorbisCommentFlags::NoSignature | VorbisCommentFlags::NoFramingByte, diag);
            break;
        case GeneralMediaFormat::Flac:
            m_iterator.ignore(4);
            comment->parse(m_iterator, flags | VorbisCommentFlags::NoSignature | VorbisCommentFlags::NoFramingByte, diag);
            break;
        default:
            diag.emplace_back(DiagLevel::Critical, "Stream format not supported.", "parsing tags from OGG streams");
        }
        params.lastPageIndex = m_iterator.currentPageIndex();
        params.lastSegmentIndex = m_iterator.currentSegmentIndex();
    }
}

/*!
 * \brief Announces the existence of a Vorbis comment.
 *
 * The start offset of the comment is specified by \a pageIndex and \a segmentIndex.
 *
 * The format of the stream the comment belongs to is specified by \a mediaFormat.
 * Valid values are GeneralMediaFormat::Vorbis, GeneralMediaFormat::Opus
 * and GeneralMediaFormat::Flac.
 *
 * \remarks This method is called by OggStream when parsing the header.
 */
void OggContainer::announceComment(std::size_t pageIndex, std::size_t segmentIndex, bool lastMetaDataBlock, GeneralMediaFormat mediaFormat)
{
    auto &tag = m_tags.emplace_back(make_unique<OggVorbisComment>());
    tag->oggParams().set(pageIndex, segmentIndex, lastMetaDataBlock, mediaFormat);
    tag->target().tracks().emplace_back(m_iterator.pages()[pageIndex].streamSerialNumber());
}

void OggContainer::internalParseTracks(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    static const string context("parsing OGG stream");
    for (auto &stream : m_tracks) {
        if (progress.isAborted()) {
            throw OperationAbortedException();
        }
        try { // try to parse header
            stream->parseHeader(diag, progress);
            if (stream->duration() > m_duration) {
                m_duration = stream->duration();
            }
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse stream at ", stream->startOffset(), '.'), context);
        }
    }
}

/*!
 * \brief Writes the specified \a comment with the given \a params to the specified \a buffer and
 *        adds the number of bytes written to \a newSegmentSizes.
 */
void OggContainer::makeVorbisCommentSegment(stringstream &buffer, CopyHelper<65307> &copyHelper, vector<std::uint32_t> &newSegmentSizes,
    VorbisComment *comment, OggParameter *params, Diagnostics &diag)
{
    const auto offset = buffer.tellp();
    switch (params->streamFormat) {
    case GeneralMediaFormat::Vorbis:
        comment->make(buffer, VorbisCommentFlags::None, diag);
        break;
    case GeneralMediaFormat::Opus:
        BE::getBytes(static_cast<std::uint64_t>(0x4F70757354616773u), copyHelper.buffer());
        buffer.write(copyHelper.buffer(), 8);
        comment->make(buffer, VorbisCommentFlags::NoSignature | VorbisCommentFlags::NoFramingByte, diag);
        break;
    case GeneralMediaFormat::Flac: {
        // Vorbis comment must be wrapped in "METADATA_BLOCK_HEADER"
        FlacMetaDataBlockHeader header;
        header.setLast(params->lastMetaDataBlock);
        header.setType(FlacMetaDataBlockType::VorbisComment);

        // write the header later, when the size is known
        buffer.write(copyHelper.buffer(), 4);

        comment->make(buffer, VorbisCommentFlags::NoSignature | VorbisCommentFlags::NoFramingByte, diag);

        // finally make the header
        header.setDataSize(static_cast<std::uint32_t>(buffer.tellp() - offset - 4));
        if (header.dataSize() > 0xFFFFFF) {
            diag.emplace_back(
                DiagLevel::Critical, "Size of Vorbis comment exceeds size limit for FLAC \"METADATA_BLOCK_HEADER\".", "making Vorbis Comment");
        }
        buffer.seekp(offset);
        header.makeHeader(buffer);
        buffer.seekp(header.dataSize(), ios_base::cur);
        break;
    }
    default:;
    }
    newSegmentSizes.push_back(static_cast<std::uint32_t>(buffer.tellp() - offset));
}

void OggContainer::internalMakeFile(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    const string context("making OGG file");
    progress.nextStepOrStop("Prepare for rewriting OGG file ...");
    parseTags(diag, progress); // tags need to be parsed before the file can be rewritten
    string originalPath = fileInfo().path(), backupPath;
    NativeFileStream backupStream;

    if (fileInfo().saveFilePath().empty()) {
        // move current file to temp dir and reopen it as backupStream, recreate original file
        try {
            BackupHelper::createBackupFileCanonical(fileInfo().backupDirectory(), originalPath, backupPath, fileInfo().stream(), backupStream);
            // recreate original file, define buffer variables
            fileInfo().stream().open(originalPath, ios_base::out | ios_base::binary | ios_base::trunc);
        } catch (const std::ios_base::failure &failure) {
            diag.emplace_back(
                DiagLevel::Critical, argsToString("Creation of temporary file (to rewrite the original file) failed: ", failure.what()), context);
            throw;
        }
    } else {
        // open the current file as backupStream and create a new outputStream at the specified "save file path"
        try {
            backupStream.exceptions(ios_base::badbit | ios_base::failbit);
            backupStream.open(BasicFileInfo::pathForOpen(fileInfo().path()).data(), ios_base::in | ios_base::binary);
            fileInfo().close();
            fileInfo().stream().open(
                BasicFileInfo::pathForOpen(fileInfo().saveFilePath()).data(), ios_base::out | ios_base::binary | ios_base::trunc);
        } catch (const std::ios_base::failure &failure) {
            diag.emplace_back(DiagLevel::Critical, argsToString("Opening streams to write output file failed: ", failure.what()), context);
            throw;
        }
    }

    const auto totalFileSize = fileInfo().size();
    try {
        progress.nextStepOrStop("Writing OGG pages ...");

        // prepare iterating comments
        OggVorbisComment *currentComment;
        OggParameter *currentParams;
        auto tagIterator = m_tags.cbegin(), tagEnd = m_tags.cend();
        if (tagIterator != tagEnd) {
            currentParams = &(currentComment = tagIterator->get())->oggParams();
        } else {
            currentComment = nullptr;
            currentParams = nullptr;
        }

        // define misc variables
        const OggPage *lastPage = nullptr;
        auto copyHelper = CopyHelper<65307>();
        auto updatedPageOffsets = std::vector<std::uint64_t>();
        auto nextPageOffset = std::uint64_t();
        auto pageSequenceNumberBySerialNo = std::unordered_map<std::uint32_t, std::uint32_t>();

        // iterate through all pages of the original file
        auto updateTick = 0u;
        for (m_iterator.setStream(backupStream), m_iterator.removeFilter(), m_iterator.reset(); m_iterator; m_iterator.nextPage(), ++updateTick) {
            const OggPage &currentPage = m_iterator.currentPage();
            if (updateTick % 10) {
                progress.updateStepPercentage(static_cast<std::uint8_t>(currentPage.startOffset() * 100ul / totalFileSize));
                progress.stopIfAborted();
            }

            // check for gaps
            // note: This is not just to print diag messages but also for taking into account that the parser might skip pages
            //       unless a full parse has been enforced.
            if (lastPage && currentPage.startOffset() != nextPageOffset) {
                m_iterator.pages().resize(m_iterator.currentPageIndex() - 1); // drop all further pages after the last consecutively parsed one
                if (m_iterator.resyncAt(nextPageOffset)) {
                    // try again at the page we've just found
                    const auto actuallyNextPageOffset = m_iterator.currentPageOffset();
                    if (actuallyNextPageOffset != nextPageOffset) {
                        diag.emplace_back(DiagLevel::Critical,
                            argsToString("Expected OGG page at offset ", nextPageOffset, " but found the next OGG page only at offset ",
                                actuallyNextPageOffset, ". Skipped ", (actuallyNextPageOffset - nextPageOffset), " invalid bytes."),
                            context);
                        nextPageOffset = actuallyNextPageOffset;
                    }
                    m_iterator.previousPage();
                    continue;
                } else {
                    diag.emplace_back(DiagLevel::Critical,
                        argsToString(
                            "Expected OGG page at offset ", nextPageOffset, " but could not find any further pages. Skipped the rest of the file."),
                        context);
                    break;
                }
            }
            const auto pageSize = currentPage.totalSize();
            std::uint32_t &pageSequenceNumber = pageSequenceNumberBySerialNo[currentPage.streamSerialNumber()];
            lastPage = &currentPage;
            nextPageOffset = currentPage.startOffset() + pageSize;

            // check whether the Vorbis Comment is present in this Ogg page
            if (currentComment && m_iterator.currentPageIndex() >= currentParams->firstPageIndex
                && m_iterator.currentPageIndex() <= currentParams->lastPageIndex && !currentPage.segmentSizes().empty()) {
                // page needs to be rewritten (not just copied)
                // -> write segments to a buffer first
                stringstream buffer(ios_base::in | ios_base::out | ios_base::binary);
                vector<std::uint32_t> newSegmentSizes;
                newSegmentSizes.reserve(currentPage.segmentSizes().size());
                std::uint64_t segmentOffset = m_iterator.currentSegmentOffset();
                vector<std::uint32_t>::size_type segmentIndex = 0;
                for (const auto segmentSize : currentPage.segmentSizes()) {
                    if (!segmentSize) {
                        ++segmentIndex;
                        continue;
                    }
                    // check whether this segment contains the Vorbis Comment
                    if (currentParams
                        && (m_iterator.currentPageIndex() >= currentParams->firstPageIndex && segmentIndex >= currentParams->firstSegmentIndex)
                        && (m_iterator.currentPageIndex() <= currentParams->lastPageIndex && segmentIndex <= currentParams->lastSegmentIndex)) {
                        // prevent making the comment twice if it spreads over multiple pages/segments
                        if (!currentParams->removed
                            && ((m_iterator.currentPageIndex() == currentParams->firstPageIndex
                                && m_iterator.currentSegmentIndex() == currentParams->firstSegmentIndex))) {
                            makeVorbisCommentSegment(buffer, copyHelper, newSegmentSizes, currentComment, currentParams, diag);
                        }

                        // proceed with next comment?
                        if (m_iterator.currentPageIndex() > currentParams->lastPageIndex
                            || (m_iterator.currentPageIndex() == currentParams->lastPageIndex && segmentIndex > currentParams->lastSegmentIndex)) {
                            if (++tagIterator != tagEnd) {
                                currentParams = &(currentComment = tagIterator->get())->oggParams();
                            } else {
                                currentComment = nullptr;
                                currentParams = nullptr;
                            }
                        }
                    } else {
                        // copy other segments unchanged
                        backupStream.seekg(static_cast<streamoff>(segmentOffset));
                        copyHelper.copy(backupStream, buffer, segmentSize);
                        newSegmentSizes.push_back(segmentSize);

                        // check whether there is a new comment to be inserted into the current page
                        if (currentParams && m_iterator.currentPageIndex() == currentParams->lastPageIndex
                            && currentParams->firstSegmentIndex == numeric_limits<size_t>::max()) {
                            if (!currentParams->removed) {
                                makeVorbisCommentSegment(buffer, copyHelper, newSegmentSizes, currentComment, currentParams, diag);
                            }
                            // proceed with next comment
                            if (++tagIterator != tagEnd) {
                                currentParams = &(currentComment = tagIterator->get())->oggParams();
                            } else {
                                currentComment = nullptr;
                                currentParams = nullptr;
                            }
                        }
                    }
                    segmentOffset += segmentSize;
                    ++segmentIndex;
                }

                // write buffered data to actual stream
                if (auto newSegmentSizesIterator = newSegmentSizes.cbegin(), newSegmentSizesEnd = newSegmentSizes.cend();
                    newSegmentSizesIterator != newSegmentSizesEnd) {
                    auto bytesLeft = *newSegmentSizesIterator;
                    auto continuePreviousSegment = false, needsZeroLacingValue = false;
                    // write pages until all data in the buffer is written
                    while (newSegmentSizesIterator != newSegmentSizesEnd) {
                        // write page header
                        backupStream.seekg(static_cast<streamoff>(currentPage.startOffset()));
                        updatedPageOffsets.push_back(static_cast<std::uint64_t>(stream().tellp())); // memorize offset to update checksum later
                        copyHelper.copy(backupStream, stream(), 27); // just copy header from original file
                        // set continue flag
                        stream().seekp(-22, ios_base::cur);
                        stream().put(static_cast<char>(currentPage.headerTypeFlag() & (continuePreviousSegment ? 0xFF : 0xFE)));
                        continuePreviousSegment = true;
                        // adjust page sequence number
                        stream().seekp(12, ios_base::cur);
                        writer().writeUInt32LE(pageSequenceNumber);
                        stream().seekp(5, ios_base::cur);
                        std::int16_t segmentSizesWritten = 0; // in the current page header only
                        // write segment sizes as long as there are segment sizes to be written and
                        // the max number of segment sizes (255) is not exceeded
                        std::uint32_t currentSize = 0;
                        while ((bytesLeft || needsZeroLacingValue) && segmentSizesWritten < 0xFF) {
                            while (bytesLeft > 0xFF && segmentSizesWritten < 0xFF) {
                                stream().put(static_cast<char>(0xFF));
                                currentSize += 0xFF;
                                bytesLeft -= 0xFF;
                                ++segmentSizesWritten;
                            }
                            if ((bytesLeft || needsZeroLacingValue) && segmentSizesWritten < 0xFF) {
                                // bytes left is here <= 0xFF
                                stream().put(static_cast<char>(bytesLeft));
                                currentSize += bytesLeft;
                                needsZeroLacingValue = bytesLeft == 0xFF;
                                bytesLeft = 0;
                                ++segmentSizesWritten;
                            }
                            if (!bytesLeft && !needsZeroLacingValue) {
                                // sizes for the segment have been written
                                // -> continue with next segment
                                if (++newSegmentSizesIterator != newSegmentSizesEnd) {
                                    bytesLeft = *newSegmentSizesIterator;
                                    continuePreviousSegment = false;
                                }
                            }
                        }

                        // there are no bytes left in the current segment; remove continue flag
                        if (!bytesLeft && !needsZeroLacingValue) {
                            continuePreviousSegment = false;
                        }

                        // page is full or all segment data has been covered
                        // -> write segment table size (segmentSizesWritten) and segment data
                        // -> seek back and write updated page segment number
                        stream().seekp(-1 - segmentSizesWritten, ios_base::cur);
                        stream().put(static_cast<char>(segmentSizesWritten));
                        stream().seekp(segmentSizesWritten, ios_base::cur);
                        // -> write actual page data
                        copyHelper.copy(buffer, stream(), currentSize);

                        ++pageSequenceNumber;
                    }
                }

            } else {
                if (pageSequenceNumber != m_iterator.currentPageIndex()) {
                    // just update page sequence number
                    backupStream.seekg(static_cast<streamoff>(currentPage.startOffset()));
                    updatedPageOffsets.push_back(static_cast<std::uint64_t>(stream().tellp())); // memorize offset to update checksum later
                    copyHelper.copy(backupStream, stream(), 27);
                    stream().seekp(-9, ios_base::cur);
                    writer().writeUInt32LE(pageSequenceNumber);
                    stream().seekp(5, ios_base::cur);
                    copyHelper.copy(backupStream, stream(), pageSize - 27);
                } else {
                    // copy page unchanged
                    backupStream.seekg(static_cast<streamoff>(currentPage.startOffset()));
                    copyHelper.copy(backupStream, stream(), pageSize);
                }
                ++pageSequenceNumber;
            }
        }

        // report new size
        fileInfo().reportSizeChanged(static_cast<std::uint64_t>(stream().tellp()));
        progress.updateStepPercentage(100);

        // "save as path" is now the regular path
        if (!fileInfo().saveFilePath().empty()) {
            fileInfo().reportPathChanged(fileInfo().saveFilePath());
            fileInfo().setSaveFilePath(string());
        }

        // close backups stream; reopen new file as readable stream
        backupStream.close();
        fileInfo().close();
        fileInfo().stream().open(BasicFileInfo::pathForOpen(fileInfo().path()).data(), ios_base::in | ios_base::out | ios_base::binary);

        // update checksums of modified pages
        progress.nextStepOrStop("Updating checksums ...");
        updateTick = 0u;
        for (auto offset : updatedPageOffsets) {
            if (updateTick++ % 10) {
                progress.updateStepPercentage(static_cast<std::uint8_t>(offset * 100ul / fileInfo().size()));
                progress.stopIfAborted();
            }
            OggPage::updateChecksum(fileInfo().stream(), offset);
        }

        // prevent deferring final write operations (to catch and handle possible errors here)
        fileInfo().stream().flush();
        progress.updateStepPercentage(100);

        // clear iterator
        m_iterator.clear(fileInfo().stream(), startOffset(), fileInfo().size());

    } catch (...) {
        m_iterator.setStream(fileInfo().stream());
        BackupHelper::handleFailureAfterFileModifiedCanonical(fileInfo(), originalPath, backupPath, fileInfo().stream(), backupStream, diag, context);
    }
}

} // namespace TagParser
