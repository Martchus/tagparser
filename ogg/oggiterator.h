#ifndef TAG_PARSER_OGGITERATOR_H
#define TAG_PARSER_OGGITERATOR_H

#include "./oggpage.h"

#include <iosfwd>
#include <vector>

namespace TagParser {

class TAG_PARSER_EXPORT OggIterator {
public:
    OggIterator(std::istream &stream, std::uint64_t startOffset, std::uint64_t streamSize);

    void clear(std::istream &stream, std::uint64_t startOffset, std::uint64_t streamSize);
    std::istream &stream();
    void setStream(std::istream &stream);
    std::uint64_t startOffset() const;
    std::uint64_t streamSize() const;
    void reset();
    void nextPage();
    void nextSegment();
    void previousPage();
    void previousSegment();
    const std::vector<OggPage> &pages() const;
    std::vector<OggPage> &pages();
    const OggPage &currentPage() const;
    std::uint64_t currentPageOffset() const;
    std::vector<OggPage>::size_type currentPageIndex() const;
    void setPageIndex(std::vector<OggPage>::size_type index);
    void setSegmentIndex(std::vector<std::uint32_t>::size_type index);
    std::vector<std::uint32_t>::size_type currentSegmentIndex() const;
    std::uint64_t currentSegmentOffset() const;
    std::uint64_t currentCharacterOffset() const;
    std::uint64_t tellg() const;
    std::uint32_t currentSegmentSize() const;
    std::uint64_t remainingBytesInCurrentSegment() const;
    std::uint64_t bytesReadFromCurrentSegment() const;
    void setFilter(std::uint32_t streamSerialId);
    void removeFilter();
    bool isLastPageFetched() const;
    void read(char *buffer, std::size_t count);
    std::size_t readAll(char *buffer, std::size_t max);
    void ignore(std::size_t count = 1);
    bool bytesRemaining(std::size_t atLeast) const;
    bool resyncAt(std::uint64_t offset);

    operator bool() const;
    OggIterator &operator++();
    OggIterator operator++(int);
    OggIterator &operator--();
    OggIterator operator--(int);

private:
    bool fetchNextPage();
    bool matchesFilter(const OggPage &page);

    std::istream *m_stream;
    std::uint64_t m_startOffset;
    std::uint64_t m_streamSize;
    std::vector<OggPage> m_pages;
    std::vector<OggPage>::size_type m_page;
    std::vector<std::uint32_t>::size_type m_segment;
    std::uint64_t m_offset;
    std::uint64_t m_bytesRead;
    bool m_hasIdFilter;
    std::uint32_t m_idFilter;
};

/*!
 * \brief Constructs a new iterator for the specified \a stream of \a streamSize bytes at the specified \a startOffset.
 */
inline OggIterator::OggIterator(std::istream &stream, std::uint64_t startOffset, std::uint64_t streamSize)
    : m_stream(&stream)
    , m_startOffset(startOffset)
    , m_streamSize(streamSize)
    , m_page(0)
    , m_segment(0)
    , m_offset(0)
    , m_bytesRead(0)
    , m_hasIdFilter(false)
    , m_idFilter(0)
{
}

/*!
 * \brief Returns the stream.
 *
 * The stream has been specified when constructing the iterator and might be changed using the setStream() methods.
 */
inline std::istream &OggIterator::stream()
{
    return *m_stream;
}

/*!
 * \brief Sets the stream.
 * \remarks The new stream must have the same data as the old stream to keep the iterator in a sane state.
 * \sa stream()
 */
inline void OggIterator::setStream(std::istream &stream)
{
    m_stream = &stream;
}

/*!
 * \brief Returns the start offset (which has been specified when constructing the iterator).
 */
inline std::uint64_t OggIterator::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the stream size (which has been specified when constructing the iterator).
 */
inline std::uint64_t OggIterator::streamSize() const
{
    return m_streamSize;
}

/*!
 * \brief Returns a vector of containing the OGG pages that have been fetched yet.
 */
inline const std::vector<OggPage> &OggIterator::pages() const
{
    return m_pages;
}

/*!
 * \brief Returns a vector of containing the OGG pages that have been fetched yet.
 */
inline std::vector<OggPage> &OggIterator::pages()
{
    return m_pages;
}

/*!
 * \brief Returns the current OGG page.
 * \remarks Calling this method when the iterator is invalid causes undefined behaviour.
 */
inline const OggPage &OggIterator::currentPage() const
{
    return m_pages[m_page];
}

/*!
 * \brief Returns the start offset of the current OGG page.
 * \remarks Calling this method when the iterator is invalid causes undefined behaviour.
 */
inline std::uint64_t OggIterator::currentPageOffset() const
{
    return m_pages[m_page].startOffset();
}

/*!
 * \brief Returns an indication whether the iterator is valid.
 *
 * The iterator is invalid when it has just been constructed. Incrementing and decrementing
 * might cause invalidation.
 *
 * If the iterator is invalid, it can be reset using the reset() method.
 *
 * Some methods cause undefined behaviour if called on an invalid iterator.
 */
inline OggIterator::operator bool() const
{
    return m_page < m_pages.size() && m_segment < m_pages[m_page].segmentSizes().size();
}

/*!
 * \brief Returns the index of the current page if the iterator is valid; otherwise an undefined index is returned.
 */
inline std::vector<OggPage>::size_type OggIterator::currentPageIndex() const
{
    return m_page;
}

/*!
 * \brief Sets the current page index.
 * \remarks This method should never be called with an \a index out of range (which is defined by the number of fetched pages), since this would cause undefined behaviour.
 */
inline void OggIterator::setPageIndex(std::vector<OggPage>::size_type index)
{
    const OggPage &page = m_pages[m_page = index];
    m_segment = 0;
    m_offset = page.startOffset() + page.headerSize();
}

/*!
 * \brief Sets the current segment index.
 *
 * This method should never be called with an \a index out of range (which is defined by the number of segments in the current page), since this causes undefined behaviour.
 */
inline void OggIterator::setSegmentIndex(std::vector<std::uint32_t>::size_type index)
{
    const OggPage &page = m_pages[m_page];
    m_offset = page.dataOffset(m_segment = index);
}

/*!
 * \brief Returns the index of the current segment (in the current page) if the iterator is valid; otherwise an undefined index is returned.
 */
inline std::vector<std::uint32_t>::size_type OggIterator::currentSegmentIndex() const
{
    return m_segment;
}

/*!
 * \brief Returns the start offset of the current segment in the input stream if the iterator is valid; otherwise an undefined offset is returned.
 * \sa currentCharacterOffset()
 */
inline std::uint64_t OggIterator::currentSegmentOffset() const
{
    return m_offset;
}

/*!
 * \brief Returns the offset of the current character in the input stream if the iterator is valid; otherwise an undefined offset is returned.
 * \sa currentSegmentOffset()
 */
inline std::uint64_t OggIterator::currentCharacterOffset() const
{
    return m_offset + m_bytesRead;
}

/*!
 * \brief Same as currentCharacterOffset(); only provided for compliance with std::istream.
 */
inline std::uint64_t OggIterator::tellg() const
{
    return currentCharacterOffset();
}

/*!
 * \brief Returns the size of the current segment.
 *
 * This method should never be called on an invalid iterator, since this causes undefined behaviour.
 */
inline std::uint32_t OggIterator::currentSegmentSize() const
{
    return m_pages[m_page].segmentSizes()[m_segment];
}

/*!
 * \brief Returns the number of bytes left to read in the current segment.
 */
inline std::uint64_t OggIterator::remainingBytesInCurrentSegment() const
{
    return currentSegmentSize() - m_bytesRead;
}

/*!
 * \brief Returns the number of bytes read from the current segment.
 */
inline uint64_t OggIterator::bytesReadFromCurrentSegment() const
{
    return m_bytesRead;
}

/*!
 * \brief Allows to filter pages by the specified \a streamSerialId.
 *
 * Pages which do not match the specified \a streamSerialId will be skipped when getting the previous or
 * the next page.
 *
 * \sa removeFilter()
 */
inline void OggIterator::setFilter(std::uint32_t streamSerialId)
{
    m_hasIdFilter = true;
    m_idFilter = streamSerialId;
}

/*!
 * \brief Removes a previously set filter.
 * \sa setFilter()
 */
inline void OggIterator::removeFilter()
{
    m_hasIdFilter = false;
}

/*!
 * \brief Returns whether the last page has already been fetched.
 */
inline bool OggIterator::isLastPageFetched() const
{
    return (m_pages.empty() ? m_startOffset : m_pages.back().startOffset() + m_pages.back().totalSize()) >= m_streamSize;
}

/*!
 * \brief Returns whether there are \a atLeast bytes remaining.
 * \deprecated Remove this unused function in v11.
 */
inline bool OggIterator::bytesRemaining(size_t atLeast) const
{
    return *this && currentCharacterOffset() + atLeast <= streamSize();
}

/*!
 * \brief Increments the current position by one segment if the iterator is valid; otherwise nothing happens.
 */
inline OggIterator &OggIterator::operator++()
{
    nextSegment();
    return *this;
}

/*!
 * \brief Increments the current position by one segment if the iterator is valid; otherwise nothing happens.
 */
inline OggIterator OggIterator::operator++(int)
{
    OggIterator tmp = *this;
    nextSegment();
    return tmp;
}

/*!
 * \brief Decrements the current position by one segment if the iterator is valid; otherwise nothing happens.
 */
inline OggIterator &OggIterator::operator--()
{
    previousSegment();
    return *this;
}

/*!
 * \brief Decrements the current position by one segment if the iterator is valid; otherwise nothing happens.
 */
inline OggIterator OggIterator::operator--(int)
{
    OggIterator tmp = *this;
    previousSegment();
    return tmp;
}

/*!
 * \brief Returns whether the specified \a page matches the current filter.
 */
inline bool OggIterator::matchesFilter(const OggPage &page)
{
    return !m_hasIdFilter || m_idFilter == page.streamSerialNumber();
}

} // namespace TagParser

#endif // TAG_PARSER_OGGITERATOR_H
