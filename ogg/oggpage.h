#ifndef OGGPAGE_H
#define OGGPAGE_H

#include <c++utilities/conversion/types.h>
#include <c++utilities/application/global.h>

#include <vector>
#include <numeric>
#include <istream>

namespace Media {

class LIB_EXPORT OggPage
{
public:
    OggPage();
    OggPage(std::istream &stream, uint64 startOffset, int32 maxSize);

    void parseHeader(std::istream &stream, uint64 startOffset, int32 maxSize);
    static uint32 computeChecksum(std::istream &stream, uint64 startOffset);
    static void updateChecksum(std::iostream &stream, uint64 startOffset);

    uint64 startOffset() const;
    byte streamStructureVersion() const;
    byte headerTypeFlag() const;
    bool isContinued() const;
    bool isFirstpage() const;
    bool isLastPage() const;
    uint64 absoluteGranulePosition() const;
    uint32 streamSerialNumber() const;
    bool matchesStreamSerialNumber(uint32 streamSerialNumber) const;
    uint32 sequenceNumber() const;
    uint32 checksum() const;
    byte segmentTableSize() const;
    const std::vector<uint32> &segmentSizes() const;
    uint32 headerSize() const;
    uint32 totalSize() const;
    uint64 dataOffset(byte segmentIndex = 0) const;
    static uint32 makeSegmentSizeDenotation(std::ostream &stream, uint32 size);

private:
    uint64 m_startOffset;
    byte m_streamStructureVersion;
    byte m_headerTypeFlag;
    uint64 m_absoluteGranulePosition;
    uint32 m_streamSerialNumber;
    uint32 m_sequenceNumber;
    uint32 m_checksum;
    byte m_segmentCount;
    std::vector<uint32> m_segmentSizes;
};

/*!
 * \brief Constructs a new OGG page.
 */
inline OggPage::OggPage() :
    m_startOffset(0),
    m_streamStructureVersion(0),
    m_headerTypeFlag(0),
    m_absoluteGranulePosition(0),
    m_streamSerialNumber(0),
    m_sequenceNumber(0),
    m_checksum(0),
    m_segmentCount(0)
{}

/*!
 * \brief Constructs a new OggPage and instantly parses the header read from the specified \a stream
 *        at the specified \a startOffset.
 */
inline OggPage::OggPage(std::istream &stream, uint64 startOffset, int32 maxSize) :
    OggPage()
{
    parseHeader(stream, startOffset, maxSize);
}

/*!
 * \brief Returns the start offset of the page.
 *
 * The start offset has been specified when calling the parseHeader() method.
 */
inline uint64 OggPage::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the stream structure version.
 */
inline byte OggPage::streamStructureVersion() const
{
    return m_streamStructureVersion;
}

/*!
 * \brief Returns the header type flag.
 * \sa isContinued()
 * \sa isFirstpage()
 * \sa isLastPage()
 */
inline byte OggPage::headerTypeFlag() const
{
    return m_headerTypeFlag;
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
inline uint64 OggPage::absoluteGranulePosition() const
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
inline uint32 OggPage::streamSerialNumber() const
{
    return m_streamSerialNumber;
}

/*!
 * \brief Returns whether the stream serial number of the current instance matches the specified one.
 * \sa streamSerialNumber()
 */
inline bool OggPage::matchesStreamSerialNumber(uint32 streamSerialNumber) const
{
    return m_streamSerialNumber == streamSerialNumber;
}

/*!
 * \brief Returns the page sequence number.
 *
 * Page counter; lets us know if a page is lost (useful where packets span page boundaries).
 */
inline uint32 OggPage::sequenceNumber() const
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
inline uint32 OggPage::checksum() const
{
    return m_checksum;
}

/*!
 * \brief Returns the size of the segment table.
 *
 * The number of segment entries to appear in the segment table.
 */
inline byte OggPage::segmentTableSize() const
{
    return m_segmentCount;
}

/*!
 * \brief Returns the sizes of the segments of the page in byte.
 *
 * The lacing values for each packet segment physically appearing in this page are listed in contiguous order.
 */
inline const std::vector<uint32> &OggPage::segmentSizes() const
{
    return m_segmentSizes;
}

/*!
 * \brief Returns the header size in byte.
 *
 * This is 27 plus the number of segment entries in the segment table.
 */
inline uint32 OggPage::headerSize() const
{
    return 27 + m_segmentCount;
}

/*!
 * \brief Returns the total size of the page in byte.
 */
inline uint32 OggPage::totalSize() const
{
    return headerSize() + std::accumulate(m_segmentSizes.cbegin(), m_segmentSizes.cend(), 0);
}

/*!
 * \brief Returns the data offset of the segment with the specified \a segmentIndex.
 *
 * This is the start offset plus the header size.
 *
 * \sa startOffset()
 * \sa headerSize()
 */
inline uint64 OggPage::dataOffset(byte segmentIndex) const
{
    return startOffset() + headerSize() + std::accumulate(m_segmentSizes.cbegin(), m_segmentSizes.cbegin() + segmentIndex, 0);
}

}

#endif // OGGPAGE_H
