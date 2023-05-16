#ifndef TAG_PARSER_OGGPAGE_H
#define TAG_PARSER_OGGPAGE_H

#include "../global.h"

#include <cstdint>
#include <iosfwd>
#include <numeric>
#include <vector>

namespace TagParser {

class TAG_PARSER_EXPORT OggPage {
public:
    OggPage();
    OggPage(std::istream &stream, std::uint64_t startOffset, std::int32_t maxSize);

    void parseHeader(std::istream &stream, std::uint64_t startOffset, std::int32_t maxSize);
    static std::uint32_t computeChecksum(std::istream &stream, std::uint64_t startOffset);
    static void updateChecksum(std::iostream &stream, std::uint64_t startOffset);

    std::uint64_t startOffset() const;
    std::uint8_t streamStructureVersion() const;
    std::uint8_t headerTypeFlag() const;
    bool isContinued() const;
    bool isFirstpage() const;
    bool isLastPage() const;
    bool isLastSegmentUnconcluded() const;
    std::uint64_t absoluteGranulePosition() const;
    std::uint32_t streamSerialNumber() const;
    bool matchesStreamSerialNumber(std::uint32_t streamSerialNumber) const;
    std::uint32_t sequenceNumber() const;
    std::uint32_t checksum() const;
    std::uint8_t segmentTableSize() const;
    const std::vector<std::uint32_t> &segmentSizes() const;
    std::uint32_t headerSize() const;
    std::uint32_t dataSize() const;
    std::uint32_t totalSize() const;
    std::uint64_t dataOffset(std::vector<std::uint32_t>::size_type segmentIndex = 0) const;
    static std::uint32_t makeSegmentSizeDenotation(std::ostream &stream, std::uint32_t size);

private:
    std::uint64_t m_startOffset;
    std::uint8_t m_streamStructureVersion;
    std::uint8_t m_headerTypeFlag;
    std::uint64_t m_absoluteGranulePosition;
    std::uint32_t m_streamSerialNumber;
    std::uint32_t m_sequenceNumber;
    std::uint32_t m_checksum;
    std::uint8_t m_segmentCount;
    bool m_lastSegmentUnconcluded;
    std::vector<std::uint32_t> m_segmentSizes;
};

/*!
 * \brief Constructs a new OGG page.
 */
inline OggPage::OggPage()
    : m_startOffset(0)
    , m_streamStructureVersion(0)
    , m_headerTypeFlag(0)
    , m_absoluteGranulePosition(0)
    , m_streamSerialNumber(0)
    , m_sequenceNumber(0)
    , m_checksum(0)
    , m_segmentCount(0)
    , m_lastSegmentUnconcluded(false)
{
}

/*!
 * \brief Constructs a new OggPage and instantly parses the header read from the specified \a stream
 *        at the specified \a startOffset.
 */
inline OggPage::OggPage(std::istream &stream, std::uint64_t startOffset, std::int32_t maxSize)
    : OggPage()
{
    parseHeader(stream, startOffset, maxSize);
}

/*!
 * \brief Returns the start offset of the page.
 *
 * The start offset has been specified when calling the parseHeader() method.
 */
inline std::uint64_t OggPage::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the stream structure version.
 */
inline std::uint8_t OggPage::streamStructureVersion() const
{
    return m_streamStructureVersion;
}

/*!
 * \brief Returns the header type flag.
 * \sa isContinued()
 * \sa isFirstpage()
 * \sa isLastPage()
 */
inline std::uint8_t OggPage::headerTypeFlag() const
{
    return m_headerTypeFlag & 0xF; // last 4 bits are used internally
}

/*!
 * \brief Returns whether this page is a continued packed (true) or a fresh packed (false).
 */
inline bool OggPage::isContinued() const
{
    return m_headerTypeFlag & 0x01;
}

/*!
 * \brief Returns whether this page is the first page of the logical bitstream.
 */
inline bool OggPage::isFirstpage() const
{
    return m_headerTypeFlag & 0x02;
}

/*!
 * \brief Returns whether this page is the last page of the logical bitstream.
 */
inline bool OggPage::isLastPage() const
{
    return m_headerTypeFlag & 0x04;
}

/*!
 * \brief Returns whether the last segment is unconcluded (the last lacing value of the last segment is 0xFF).
 */
inline bool OggPage::isLastSegmentUnconcluded() const
{
    return m_lastSegmentUnconcluded;
}

/*!
 * \brief Returns the absolute granule position.
 *
 * The position specified is the total samples encoded after including all packets finished on this
 * page (packets begun on this page but continuing on to the next page do not count). The rationale
 * here is that the position specified in the frame header of the last page tells how long the data
 * coded by the bitstream is. A truncated stream will still return the proper number of samples that
 * can be decoded fully.
 *
 * A special value of '-1' (in two's complement) indicates that no packets finish on this page.
 */
inline std::uint64_t OggPage::absoluteGranulePosition() const
{
    return m_absoluteGranulePosition;
}

/*!
 * \brief Returns the stream serial number.
 *
 * Ogg allows for separate logical bitstreams to be mixed at page granularity in a physical bitstream.
 * The most common case would be sequential arrangement, but it is possible to interleave pages for
 * two separate bitstreams to be decoded concurrently. The serial number is the means by which pages
 * physical pages are associated with a particular logical stream.
 */
inline std::uint32_t OggPage::streamSerialNumber() const
{
    return m_streamSerialNumber;
}

/*!
 * \brief Returns whether the stream serial number of the current instance matches the specified one.
 * \sa streamSerialNumber()
 */
inline bool OggPage::matchesStreamSerialNumber(std::uint32_t streamSerialNumber) const
{
    return m_streamSerialNumber == streamSerialNumber;
}

/*!
 * \brief Returns the page sequence number.
 *
 * Page counter; lets us know if a page is lost (useful where packets span page boundaries).
 */
inline std::uint32_t OggPage::sequenceNumber() const
{
    return m_sequenceNumber;
}

/*!
 * \brief Returns the page checksum.
 *
 * 32 bit CRC value (direct algorithm, initial val and final XOR = 0, generator polynomial=0x04c11db7).
 * The value is computed over the entire header (with the CRC field in the header set to zero) and then
 * continued over the page. The CRC field is then filled with the computed value.
 *
 * \sa This method returns the checksum denoted by the header. To compute the actual checksum use
 *     the computeChecksum() method.
 */
inline std::uint32_t OggPage::checksum() const
{
    return m_checksum;
}

/*!
 * \brief Returns the size of the segment table.
 *
 * The number of segment entries to appear in the segment table.
 */
inline std::uint8_t OggPage::segmentTableSize() const
{
    return m_segmentCount;
}

/*!
 * \brief Returns the sizes of the segments of the page in byte.
 *
 * The lacing values for each packet segment physically appearing in this page are listed in contiguous order.
 */
inline const std::vector<std::uint32_t> &OggPage::segmentSizes() const
{
    return m_segmentSizes;
}

/*!
 * \brief Returns the header size in byte.
 *
 * This is 27 plus the number of segment entries in the segment table.
 */
inline std::uint32_t OggPage::headerSize() const
{
    return 27 + m_segmentCount;
}

/*!
 * \brief Returns the data size in byte.
 */
inline std::uint32_t OggPage::dataSize() const
{
    return std::accumulate(m_segmentSizes.cbegin(), m_segmentSizes.cend(), 0u);
}

/*!
 * \brief Returns the total size of the page in byte.
 */
inline std::uint32_t OggPage::totalSize() const
{
    return headerSize() + dataSize();
}

/*!
 * \brief Returns the data offset of the segment with the specified \a segmentIndex.
 *
 * This is the start offset plus the header size.
 *
 * \sa startOffset()
 * \sa headerSize()
 */
inline std::uint64_t OggPage::dataOffset(std::vector<std::uint32_t>::size_type segmentIndex) const
{
    return startOffset() + headerSize()
        + std::accumulate<decltype(m_segmentSizes)::const_iterator, std::uint64_t>(
            m_segmentSizes.cbegin(), m_segmentSizes.cbegin() + static_cast<decltype(m_segmentSizes)::difference_type>(segmentIndex), 0u);
}

} // namespace TagParser

#endif // TAG_PARSER_OGGPAGE_H
