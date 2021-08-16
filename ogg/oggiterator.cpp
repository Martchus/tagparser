#include "./oggiterator.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>

#include <iostream>
#include <limits>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::OggIterator
 * \brief The OggIterator class helps iterating through all segments of an OGG bitstream.
 *
 * If an OggIterator has just been constructed it is invalid. To fetch the first page from
 * the stream call the reset() method. The iterator will now point to the first segment of the
 * first page.
 *
 * To go on call the appropriate methods. Parsing exceptions and IO exceptions might occur during iteration.
 *
 * The internal buffer of OGG pages might be accessed using the pages() method.
 */

/*!
 * \brief Sets the stream and related parameters and clears all available pages.
 * \remarks Invalidates the iterator. Use reset() to continue iteration.
 */
void OggIterator::clear(istream &stream, std::uint64_t startOffset, std::uint64_t streamSize)
{
    m_stream = &stream;
    m_startOffset = startOffset;
    m_streamSize = streamSize;
    m_pages.clear();
}

/*!
 * \brief Resets the iterator to point at the first segment of the first page (matching the filter if set).
 *
 * Fetched pages (directly accessible through the page() method) remain after resetting the iterator. Use
 * clear() to clear all pages.
 */
void OggIterator::reset()
{
    for (m_page = m_segment = m_offset = 0; m_page < m_pages.size() || fetchNextPage(); ++m_page) {
        const OggPage &page = m_pages[m_page];
        if (!page.segmentSizes().empty() && matchesFilter(page)) {
            // page is not empty and matches ID filter if set
            m_offset = page.startOffset() + page.headerSize();
            break;
        }
    }
    // no matching page found -> iterator is invalid
}

/*!
 * \brief Increases the current position by one page.
 * \remarks The iterator must be valid. The iterator might be invalidated.
 */
void OggIterator::nextPage()
{
    while (++m_page < m_pages.size() || fetchNextPage()) {
        const OggPage &page = m_pages[m_page];
        if (!page.segmentSizes().empty() && matchesFilter(page)) {
            // page is not empty and matches ID filter if set
            m_segment = m_bytesRead = 0;
            m_offset = page.startOffset() + page.headerSize();
            return;
        }
    }
    // no next page available -> iterator is in invalid state
}

/*!
 * \brief Increases the current position by one segment.
 * \remarks The iterator must be valid. The iterator might be invalidated.
 */
void OggIterator::nextSegment()
{
    const OggPage &page = m_pages[m_page];
    if (matchesFilter(page) && ++m_segment < page.segmentSizes().size()) {
        // current page has next segment
        m_bytesRead = 0;
        m_offset += page.segmentSizes()[m_segment - 1];
    } else {
        // next (matching) page has next segment
        nextPage();
    }
}

/*!
 * \brief Decreases the current position by one page.
 * \remarks The iterator must be valid. The iterator might be invalidated.
 */
void OggIterator::previousPage()
{
    while (m_page) {
        const OggPage &page = m_pages[--m_page];
        if (matchesFilter(page)) {
            m_offset = page.dataOffset(m_segment = page.segmentSizes().size() - 1);
            return;
        }
    }
}

/*!
 * \brief Decreases the current position by one segment.
 * \remarks The iterator must be valid. The iterator might be invalidated.
 */
void OggIterator::previousSegment()
{
    const OggPage &page = m_pages[m_page];
    if (m_segment && matchesFilter(page)) {
        m_offset -= page.segmentSizes()[m_segment--];
    } else {
        previousPage();
    }
}

/*!
 * \brief Reads \a count bytes from the OGG stream and writes it to the specified \a buffer.
 * \remarks
 *          - Might increase the current page index and/or the current segment index.
 *          - Page headers are skipped (this is the whole purpose of this method).
 * \throws Throws a TruncatedDataException if the end of the stream is reached before \a count bytes
 *         have been read.
 * \sa readAll()
 * \sa currentCharacterOffset()
 * \sa seekForward()
 */
void OggIterator::read(char *buffer, std::size_t count)
{
    std::size_t bytesRead = 0;
    while (*this && count) {
        const auto available = remainingBytesInCurrentSegment();
        stream().seekg(static_cast<std::streamoff>(currentCharacterOffset()));
        if (count <= available) {
            stream().read(buffer + bytesRead, static_cast<std::streamsize>(count));
            m_bytesRead += count;
            return;
        }
        stream().read(buffer + bytesRead, static_cast<std::streamsize>(available));
        nextSegment();
        bytesRead += available;
        count -= available;
    }
    if (count) {
        // still bytes to read but no more available
        throw TruncatedDataException();
    }
}

/*!
 * \brief Reads all bytes from the OGG stream and writes it to the specified \a buffer.
 * \remarks
 *          - Might increase the current page index and/or the current segment index.
 *          - Page headers are skipped (this is the whole purpose of this method).
 *          - Does not write more than \a max bytes to the buffer.
 * \returns Returns the number of bytes read from the OGG stream. This might be less than \a max in
 *          case not that many bytes were available.
 * \sa read()
 * \sa currentCharacterOffset()
 * \sa seekForward()
 * \deprecated Remove this unused function in v11.
 */
std::size_t OggIterator::readAll(char *buffer, std::size_t max)
{
    auto bytesRead = std::size_t(0);
    while (*this && max) {
        const auto available = remainingBytesInCurrentSegment();
        stream().seekg(static_cast<std::streamoff>(currentCharacterOffset()), std::ios_base::beg);
        if (max <= available) {
            stream().read(buffer + bytesRead, static_cast<std::streamsize>(max));
            m_bytesRead += max;
            return bytesRead + max;
        } else {
            stream().read(buffer + bytesRead, static_cast<std::streamsize>(available));
            nextSegment();
            bytesRead += available;
            max -= available;
        }
    }
    return bytesRead;
}

/*!
 * \brief Advances the position of the next character to be read from the OGG stream by \a count bytes.
 * \remarks
 *          - Might increase the current page index and/or the current segment index.
 *          - Page headers are skipped (this is the whole purpose of this method).
 *          - Seeking backward is not implemented yet since there is currently no use for such a method.
 * \throws Throws a TruncatedDataException if the end of the stream is exceeded.
 * \sa currentCharacterOffset()
 * \sa read()
 */
void OggIterator::ignore(std::size_t count)
{
    while (*this) {
        const auto available = currentSegmentSize() - m_bytesRead;
        if (count <= available) {
            m_bytesRead += count;
            return;
        } else {
            nextSegment();
            count -= available;
        }
    }
    throw TruncatedDataException();
}

/*!
 * \brief Fetches the next page at the specified \a offset.
 *
 * This allows to omit parts of a file which is useful to
 * - find the last page faster by skipping pages in the middle (last page is required for calculating
 *   the files duration).
 * - recover parsing after after an error occurred.
 *
 * Regardless of the current iterator position, this method will assume the page at \a offset comes after
 * the last known page. Hence \a offset must be greater than OggPage::startOffset() + OggPage::totalSize() of the
 * last known page. This is checked by the method.
 *
 * If the OGG capture pattern is not present at \a offset, up to 65307 bytes (max. size of an OGG page) are
 * skipped. So in a valid stream, this method will always succeed if \a offset is less than the stream size minus
 * 65307.
 *
 * If a page could be found, it is appended to pages() and the iterator position is set to the first segment of
 * that page. If no page could be found, this method does not alter the iterator.
 *
 * \returns Returns an indication whether a page could be found.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Failure when a parsing error occurs.
 */
bool OggIterator::resyncAt(std::uint64_t offset)
{
    // check whether offset is valid
    if (offset >= streamSize() || offset < (m_pages.empty() ? m_startOffset : m_pages.back().startOffset() + m_pages.back().totalSize())) {
        return false;
    }

    // find capture pattern 'OggS'
    stream().seekg(static_cast<streamoff>(offset));
    std::uint8_t lettersFound = 0;
    for (std::uint64_t bytesAvailable = max<std::uint64_t>(streamSize() - offset, 65307ul); bytesAvailable >= 27; --bytesAvailable) {
        switch (static_cast<char>(stream().get())) {
        case 'O':
            lettersFound = 1;
            break;
        case 'g':
            lettersFound = lettersFound == 1 || lettersFound == 2 ? lettersFound + 1 : 0;
            break;
        case 'S':
            if (lettersFound == 3) {
                // capture pattern found
                const auto currentOffset = stream().tellg();
                // -> try to parse an OGG page at this position
                try {
                    m_pages.emplace_back(stream(), static_cast<std::uint64_t>(stream().tellg()) - 4,
                        bytesAvailable > numeric_limits<std::int32_t>::max() ? numeric_limits<std::int32_t>::max()
                                                                             : static_cast<std::int32_t>(bytesAvailable));
                    setPageIndex(m_pages.size() - 1);
                    return true;
                } catch (const Failure &) {
                    stream().seekg(currentOffset);
                }
            }
            [[fallthrough]];
        default:
            lettersFound = 0;
        }
    }
    return false;
}

/*!
 * \brief Fetches the next page.
 *
 * A new page can only be fetched if the current page is the last page in the buffer and if the
 * end of the input stream has not been reached yet.
 *
 * \returns Returns an indication whether the next page could be fetched.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Failure when a parsing error occurs.
 */
bool OggIterator::fetchNextPage()
{
    if (m_page == m_pages.size()) { // can only fetch the next page if the current page is the last page
        m_offset = m_pages.empty() ? m_startOffset : m_pages.back().startOffset() + m_pages.back().totalSize();
        if (m_offset < m_streamSize) {
            const std::uint64_t bytesAvailable = m_streamSize - m_offset;
            m_pages.emplace_back(*m_stream, m_offset,
                bytesAvailable > numeric_limits<std::int32_t>::max() ? numeric_limits<std::int32_t>::max()
                                                                     : static_cast<std::int32_t>(bytesAvailable));
            return true;
        }
    }
    return false;
}

} // namespace TagParser
