#include "./oggpage.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/conversion/binaryconversion.h>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::OggPage
 * \brief The OggPage class is used to parse OGG pages.
 * \sa http://www.xiph.org/ogg/doc/framing.html
 */

/*!
 * \brief Parses the header read from the specified \a stream at the specified \a startOffset.
 * \throws Throws InvalidDataException if the capture pattern is not present.
 * \throws Throws TruncatedDataException if the header is truncated (according to \a maxSize).
 */
void OggPage::parseHeader(istream &stream, uint64 startOffset, int32 maxSize)
{
    // prepare reading
    stream.seekg(startOffset);
    BinaryReader reader(&stream);
    if(maxSize < 27) {
        throw TruncatedDataException();
    } else {
        maxSize -= 27;
    }
    // read header values
    if(reader.readUInt32LE() != 0x5367674f) {
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
    if(m_segmentCount > 0) {
        if(maxSize < m_segmentCount) {
            throw TruncatedDataException();
        } else {
            maxSize -= m_segmentCount;
        }
        // read segment size tabe
        m_segmentSizes.push_back(0);
        for(byte i = 0; i < m_segmentCount;) {
            byte entry = reader.readByte();
            maxSize -= entry;
            m_segmentSizes.back() += entry;
            if(++i < m_segmentCount && entry < 0xff) {
                m_segmentSizes.push_back(0);
            }
        }
        // check whether the maximum size is exceeded
        if(maxSize < 0) {
            throw TruncatedDataException();
        }
    }
}

/*!
 * \brief Computes the actual checksum of the page read from the specified \a stream
 *        at the specified \a startOffset.
 */
uint32 OggPage::computeChecksum(istream &stream, uint64 startOffset)
{
    stream.seekg(startOffset);
    uint32 crc = 0x0;
    byte value, segmentTableSize = 0, segmentTableIndex = 0;
    for(uint32 i = 0, segmentLength = 27; i < segmentLength; ++i) {
        switch(i) {
        case 22:
            // bytes 22, 23, 24, 25 hold denoted checksum and must be set to zero
            stream.seekg(4, ios_base::cur);
        case 23: case 24: case 25:
            value = 0;
            break;
        case 26:
            // byte 26 holds the number of segment sizes
            segmentLength += (segmentTableSize = (value = stream.get()));
            break;
        default:
            value = stream.get();
            if(i > 26 && segmentTableIndex < segmentTableSize) {
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
void OggPage::updateChecksum(iostream &stream, uint64 startOffset)
{
    char buff[4];
    LE::getBytes(computeChecksum(stream, startOffset), buff);
    stream.seekp(startOffset + 22);
    stream.write(buff, sizeof(buff));
}

/*!
 * \brief Writes the segment size denotation for the specified segment \a size to the specified stream.
 * \return Returns the number of bytes written.
 */
uint32 OggPage::makeSegmentSizeDenotation(ostream &stream, uint32 size)
{
    uint32 bytesWritten = 1;
    while(size >= 0xff) {
        stream.put(0xff);
        size -= 0xff;
        ++bytesWritten;
    }
    stream.put(size);
    return bytesWritten;
}

}
