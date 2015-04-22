#include "oggcontainer.h"

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
    for(auto i : m_commentTable) {
        //fileInfo().stream().seekg(get<1>(i));
        m_iterator.setPageIndex(get<0>(i));
        m_iterator.setSegmentIndex(get<1>(i));
        m_tags[get<2>(i)]->parse(m_iterator);
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
        for(m_iterator.setStream(backupStream), m_iterator.removeFilter(), m_iterator.reset(); m_iterator; m_iterator.nextPage()) {
            const auto &currentPage = m_iterator.currentPage();
            auto pageSize = currentPage.totalSize();
            if(commentTableIterator != commentTableEnd && m_iterator.currentPageIndex() == get<0>(*commentTableIterator) && !currentPage.segmentSizes().empty()) {
                // page needs to be rewritten (not just copied)
                // -> write segments to a buffer first
                stringstream buffer(ios_base::in | ios_base::out | ios_base::binary);
                vector<uint32> newSegmentSizes;
                newSegmentSizes.reserve(currentPage.segmentSizes().size());
                uint64 segmentOffset = m_iterator.currentSegmentOffset();
                vector<uint32>::size_type segmentIndex = 0;
                for(auto segmentSize : currentPage.segmentSizes()) {
                    if(segmentIndex == get<1>(*commentTableIterator)) {
                        // make vorbis comment segment
                        auto offset = buffer.tellp();
                        m_tags[get<2>(*commentTableIterator)]->make(buffer);
                        newSegmentSizes.push_back(buffer.tellp() - offset);
                    } else {
                        // copy other segments unchanged
                        backupStream.seekg(segmentOffset);
                        copy.copy(backupStream, buffer, segmentSize);
                        newSegmentSizes.push_back(segmentSize);
                    }
                    segmentOffset += segmentSize;
                    ++segmentIndex;
                }
                // write buffered data to actual stream
                auto newSegmentSizesIterator = newSegmentSizes.cbegin(), newSegmentSizesEnd = newSegmentSizes.cend();
                uint32 bytesLeft, currentSize;
                // write pages until all data in the buffer is written
                while(newSegmentSizesIterator != newSegmentSizesEnd) {
                    // write header
                    backupStream.seekg(currentPage.startOffset());
                    copy.copy(backupStream, stream(), 27); // just copy from original file
                    updatedPageOffsets.push_back(currentPage.startOffset()); // memorize offset to update checksum later
                    int16 segmentSizesWritten = 0; // in the current page header only
                    // write segment sizes as long as there are segment sizes to be written and
                    // the max number of segment sizes is not exceeded
                    bytesLeft = *newSegmentSizesIterator;
                    currentSize = 0;
                    while(bytesLeft > 0 && segmentSizesWritten < 0xFF) {
                        while(bytesLeft >= 0xFF && segmentSizesWritten < 0xFF) {
                            stream().put(0xFF);
                            bytesLeft -= 0xFF;
                            ++segmentSizesWritten;
                        }
                        if(bytesLeft > 0 && segmentSizesWritten < 0xFF) {
                            stream().put(bytesLeft);
                            bytesLeft = 0;
                            ++segmentSizesWritten;
                        }
                        currentSize += *newSegmentSizesIterator - bytesLeft;
                        if(bytesLeft == 0) {
                            // sizes for the current segemnt have been written -> continue with next segment
                            ++newSegmentSizesIterator;
                            bytesLeft = newSegmentSizesIterator != newSegmentSizesEnd ? *newSegmentSizesIterator : 0;
                        }
                    }
                    // page is full or all segment data has already been written -> write data
                    if(bytesLeft == 0 || newSegmentSizesIterator != newSegmentSizesEnd) {
                        // seek back and write updated page segment number
                        stream().seekp(-1 - segmentSizesWritten, ios_base::cur);
                        stream().put(segmentSizesWritten);
                        stream().seekp(segmentSizesWritten, ios_base::cur);
                        // write actual page data
                        copy.copy(buffer, stream(), currentSize);
                    }
                }
            } else {
                // copy page unchanged
                backupStream.seekg(currentPage.startOffset());
                copy.copy(backupStream, stream(), pageSize);
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
