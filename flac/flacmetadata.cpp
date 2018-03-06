#include "./flacmetadata.h"

#include "../exceptions.h"
#include "../tagvalue.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/io/bitreader.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <cstring>
#include <memory>

using namespace std;
using namespace ConversionUtilities;
using namespace IoUtilities;

namespace TagParser {

/*!
 * \class Media::FlacMetaDataBlockHeader
 * \brief The FlacMetaDataBlockHeader class is a FLAC "METADATA_BLOCK_HEADER" parser and maker.
 * \sa https://xiph.org/flac/format.html
 */

/*!
 * \brief Parses the FLAC "METADATA_BLOCK_HEADER" which is read using the specified \a iterator.
 * \remarks The specified \a buffer must be at least 4 bytes long.
 */
void FlacMetaDataBlockHeader::parseHeader(const char *buffer)
{
    m_last = *buffer & 0x80;
    m_type = *buffer & (0x80 - 1);
    m_dataSize = BE::toUInt24(buffer + 1);
}

/*!
 * \brief Writes the header to the specified \a outputStream.
 * \remarks Writes always 4 bytes.
 */
void FlacMetaDataBlockHeader::makeHeader(std::ostream &outputStream)
{
    byte buff[4];
    *buff = (m_last ? (0x80 | m_type) : m_type);
    BE::getBytes24(m_dataSize, reinterpret_cast<char *>(buff) + 1);
    outputStream.write(reinterpret_cast<char *>(buff), sizeof(buff));
}

/*!
 * \class Media::FlacMetaDataBlockStreamInfo
 * \brief The FlacMetaDataBlockStreamInfo class is a FLAC "METADATA_BLOCK_STREAMINFO" parser.
 * \sa https://xiph.org/flac/format.html
 */

/*!
 * \brief Parses the FLAC "METADATA_BLOCK_STREAMINFO" which is read using the specified \a iterator.
 * \remarks The specified \a buffer must be at least 0x22 bytes long.
 */
void FlacMetaDataBlockStreamInfo::parse(const char *buffer)
{
    BitReader reader(buffer, 0x22);
    m_minBlockSize = reader.readBits<uint16>(16);
    m_maxBlockSize = reader.readBits<uint16>(16);
    m_minFrameSize = reader.readBits<uint32>(24);
    m_maxFrameSize = reader.readBits<uint32>(24);
    m_samplingFrequency = reader.readBits<uint32>(20);
    m_channelCount = reader.readBits<byte>(3) + 1;
    m_bitsPerSample = reader.readBits<byte>(5) + 1;
    m_totalSampleCount = reader.readBits<uint64>(36);
    memcpy(m_md5Sum, buffer + 0x22 - sizeof(m_md5Sum), sizeof(m_md5Sum));
}

/*!
 * \class Media::FlacMetaDataBlockPicture
 * \brief The FlacMetaDataBlockPicture class is a FLAC "METADATA_BLOCK_PICTURE" parser and maker.
 * \sa https://xiph.org/flac/format.html
 */

/*!
 * \brief Parses the FLAC "METADATA_BLOCK_PICTURE".
 *
 * \a maxSize specifies the maximum size of the structure.
 */
void FlacMetaDataBlockPicture::parse(istream &inputStream, uint32 maxSize)
{
    CHECK_MAX_SIZE(32);
    BinaryReader reader(&inputStream);
    m_pictureType = reader.readUInt32BE();
    uint32 size = reader.readUInt32BE();
    CHECK_MAX_SIZE(size);
    m_value.setMimeType(reader.readString(size));
    size = reader.readUInt32BE();
    CHECK_MAX_SIZE(size);
    m_value.setDescription(reader.readString(size));
    // skip width, height, color depth, number of colors used
    inputStream.seekg(4 * 4, ios_base::cur);
    size = reader.readUInt32BE();
    CHECK_MAX_SIZE(size);
    if(size) {
        auto data = make_unique<char[]>(size);
        inputStream.read(data.get(), size);
        m_value.assignData(move(data), size, TagDataType::Picture);
    } else {
        m_value.clearData();
    }
}

/*!
 * \brief Returns the number of bytes make() will write.
 * \remarks Any changes to the object will invalidate this value.
 */
uint32 FlacMetaDataBlockPicture::requiredSize() const
{
    return 32 + m_value.mimeType().size() + m_value.description().size() + m_value.dataSize();
}

/*!
 * \brief Makes the FLAC "METADATA_BLOCK_PICTURE".
 */
void FlacMetaDataBlockPicture::make(ostream &outputStream)
{
    BinaryWriter writer(&outputStream);
    writer.writeUInt32BE(pictureType());
    writer.writeUInt32BE(m_value.mimeType().size());
    writer.writeString(m_value.mimeType());
    writer.writeUInt32BE(m_value.description().size());
    writer.writeString(m_value.description());
    writer.writeUInt32BE(0); // skip width
    writer.writeUInt32BE(0); // skip height
    writer.writeUInt32BE(0); // skip color depth
    writer.writeUInt32BE(0); // skip number of colors used
    writer.writeUInt32BE(m_value.dataSize());
    writer.write(value().dataPointer(), m_value.dataSize());
}


}
