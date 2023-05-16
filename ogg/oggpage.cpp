#include "./oggpage.h"

#include "../exceptions.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/io/binaryreader.h>

#include <limits>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::OggPage
 * \brief The OggPage class is used to parse OGG pages.
 * \sa http://www.xiph.org/ogg/doc/framing.html
 * \todo Add field for additional flags in v11.
 */

/*!
 * \brief Parses the header read from the specified \a stream at the specified \a startOffset.
 * \throws Throws InvalidDataException if the capture pattern is not present.
 * \throws Throws TruncatedDataException if the header is truncated (according to \a maxSize).
 */
void OggPage::parseHeader(istream &stream, std::uint64_t startOffset, std::int32_t maxSize)
{
    // prepare reading
    stream.seekg(static_cast<streamoff>(startOffset));
    BinaryReader reader(&stream);
    if (maxSize < 27) {
        throw TruncatedDataException();
    } else {
        maxSize -= 27;
    }
    // read header values
    if (reader.readUInt32LE() != 0x5367674f) {
        throw InvalidDataException();
    }
    m_startOffset = startOffset;
    m_streamStructureVersion = reader.readByte();
    m_headerTypeFlag = reader.readByte();
    m_absoluteGranulePosition = reader.readUInt64LE();
    m_streamSerialNumber = reader.readUInt32LE();
    m_sequenceNumber = reader.readUInt32LE();
    m_checksum = reader.readUInt32LE();
    m_segmentCount = reader.readByte();
    m_segmentSizes.clear();
    if (m_segmentCount > 0) {
        if (maxSize < m_segmentCount) {
            throw TruncatedDataException();
        } else {
            maxSize -= m_segmentCount;
        }
        // read segment size table
        m_segmentSizes.emplace_back(0);
        for (std::uint8_t i = 0; i < m_segmentCount;) {
            std::uint8_t entry = reader.readByte();
            maxSize -= entry;
            m_segmentSizes.back() += entry;
            if (++i < m_segmentCount && entry < 0xFF) {
                m_segmentSizes.emplace_back(0);
            } else if (i == m_segmentCount && entry == 0xFF) {
                m_lastSegmentUnconcluded = true;
            }
        }
        // check whether the maximum size is exceeded
        if (maxSize < 0) {
            throw TruncatedDataException();
        }
    }
}

/*!
 * \brief Computes the actual checksum of the page read from the specified \a stream
 *        at the specified \a startOffset.
 */
std::uint32_t OggPage::computeChecksum(istream &stream, std::uint64_t startOffset)
{
    stream.seekg(static_cast<streamoff>(startOffset));
    std::uint32_t crc = 0x0;
    std::uint8_t value, segmentTableSize = 0, segmentTableIndex = 0;
    for (std::uint32_t i = 0, segmentLength = 27; i != segmentLength; ++i) {
        switch (i) {
        case 22:
            // bytes 22, 23, 24, 25 hold denoted checksum and must be set to zero
            stream.seekg(4, ios_base::cur);
            [[fallthrough]];
        case 23:
        case 24:
        case 25:
            value = 0;
            break;
        case 26:
            // byte 26 holds the number of segment sizes
            segmentLength += (segmentTableSize = (value = static_cast<std::uint8_t>(stream.get())));
            break;
        default:
            value = static_cast<std::uint8_t>(stream.get());
            if (i > 26 && segmentTableIndex < segmentTableSize) {
                // bytes 27 to (27 + segment size count) hold page size
                segmentLength += value;
                ++segmentTableIndex;
            }
        }
        crc = (crc << 8) ^ BinaryReader::crc32Table[((crc >> 24) & 0xFF) ^ value];
    }
    return crc;
}

/*!
 * \brief Updates the checksum of the page read from the specified \a stream
 *        at the specified \a startOffset.
 */
void OggPage::updateChecksum(iostream &stream, std::uint64_t startOffset)
{
    char buff[4];
    LE::getBytes(computeChecksum(stream, startOffset), buff);
    stream.seekp(static_cast<streamoff>(startOffset + 22));
    stream.write(buff, sizeof(buff));
}

/*!
 * \brief Writes the segment size denotation for the specified segment \a size to the specified stream.
 * \return Returns the number of bytes written.
 * \deprecated This function is unused and should be removed in v11.
 */
std::uint32_t OggPage::makeSegmentSizeDenotation(ostream &stream, std::uint32_t size)
{
    std::uint32_t bytesWritten = 1;
    while (size >= 0xff) {
        stream.put(static_cast<char>(0xff));
        size -= 0xff;
        ++bytesWritten;
    }
    stream.put(static_cast<char>(size));
    return bytesWritten;
}

} // namespace TagParser
