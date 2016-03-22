#include "./oggiterator.h"

#include "../exceptions.h"

#include <iostream>

using namespace std;

namespace Media {

/*!
 * \class Media::OggIterator
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
void OggIterator::clear(istream &stream, uint64 startOffset, uint64 streamSize)
{
    m_stream = &stream;
    m_startOffset = startOffset;
    m_streamSize = streamSize;
    m_pages.clear();
}

/*!
 * \brief Resets the iterator to point at the first segment of the first page (matching the filter if set).
 *
 * Fetched pages (directly accessable through the page() method) remain after resetting the iterator. Use
 * the clear method to clear all pages.
 */
void OggIterator::reset()
{
    for(m_page = m_segment =  m_offset = 0; m_page < m_pages.size() || fetchNextPage(); ++m_page) {
        const OggPage &page = m_pages[m_page];
        if(!page.segmentSizes().empty() && matchesFilter(page)) {
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
    while(++m_page < m_pages.size() || fetchNextPage()) {
        const OggPage &page = m_pages[m_page];
        if(!page.segmentSizes().empty() && matchesFilter(page)) {
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
    if(matchesFilter(page) && ++m_segment < page.segmentSizes().size()) {
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
    while(m_page) {
        const OggPage &page = m_pages[--m_page];
        if(matchesFilter(page)) {
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
    if(m_segment && matchesFilter(page)) {
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
 * \sa currentCharacterOffset()
 * \sa seekForward()
 */
void OggIterator::read(char *buffer, size_t count)
{
    size_t bytesRead = 0;
    while(*this && count) {
        uint32 available = currentSegmentSize() - m_bytesRead;
        stream().seekg(currentCharacterOffset());
        if(count <= available) {
            stream().read(buffer + bytesRead, count);
            m_bytesRead += count;
            return;
        } else {
            stream().read(buffer + bytesRead, available);
            nextSegment();
            bytesRead += available;
            count -= available;
        }
    }
    if(count) {
        // still bytes to read but no more available
        throw TruncatedDataException();
    }
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
void OggIterator::seekForward(size_t count)
{
    while(*this && count) {
        uint32 available = currentSegmentSize() - m_bytesRead;
        if(count <= available) {
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
    if(m_page == m_pages.size()) { // can only fetch the next page if the current page is the last page
        m_offset = m_pages.empty() ? m_startOffset : m_pages.back().startOffset() + m_pages.back().totalSize();
        if(m_offset < m_streamSize) {
            m_pages.emplace_back(*m_stream, m_offset, static_cast<int32>(m_streamSize - m_offset));
            return true;
        }
    }
    return false;
}

}
