#include "./flacmetadata.h"

#include "../exceptions.h"
#include "../tagvalue.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/io/bitreader.h>

#include <cstring>
#include <memory>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::FlacMetaDataBlockHeader
 * \brief The FlacMetaDataBlockHeader class is a FLAC "METADATA_BLOCK_HEADER" parser and maker.
 * \sa https://xiph.org/flac/format.html
 */

/*!
 * \brief Parses the FLAC "METADATA_BLOCK_HEADER" which is read using the specified \a iterator.
 * \remarks The specified \a buffer must be at least 4 bytes long.
 */
void FlacMetaDataBlockHeader::parseHeader(std::string_view buffer)
{
    m_last = static_cast<std::uint8_t>(buffer[0] & 0x80);
    m_type = static_cast<std::uint8_t>(buffer[0] & (0x80 - 1));
    m_dataSize = BE::toUInt24(buffer.data() + 1);
}

/*!
 * \brief Writes the header to the specified \a outputStream.
 * \remarks Writes always 4 bytes.
 */
void FlacMetaDataBlockHeader::makeHeader(std::ostream &outputStream)
{
    std::uint8_t buff[4];
    *buff = (m_last ? (0x80 | m_type) : m_type);
    BE::getBytes24(m_dataSize, reinterpret_cast<char *>(buff) + 1);
    outputStream.write(reinterpret_cast<char *>(buff), sizeof(buff));
}

/*!
 * \class TagParser::FlacMetaDataBlockStreamInfo
 * \brief The FlacMetaDataBlockStreamInfo class is a FLAC "METADATA_BLOCK_STREAMINFO" parser.
 * \sa https://xiph.org/flac/format.html
 */

/*!
 * \brief Parses the FLAC "METADATA_BLOCK_STREAMINFO" which is read using the specified \a iterator.
 * \remarks The specified \a buffer must be at least 0x22 bytes long.
 */
void FlacMetaDataBlockStreamInfo::parse(std::string_view buffer)
{
    auto reader = BitReader(buffer.data(), 0x22);
    m_minBlockSize = reader.readBits<std::uint16_t>(16);
    m_maxBlockSize = reader.readBits<std::uint16_t>(16);
    m_minFrameSize = reader.readBits<std::uint32_t>(24);
    m_maxFrameSize = reader.readBits<std::uint32_t>(24);
    m_samplingFrequency = reader.readBits<std::uint32_t>(20);
    m_channelCount = reader.readBits<std::uint8_t>(3) + 1;
    m_bitsPerSample = reader.readBits<std::uint8_t>(5) + 1;
    m_totalSampleCount = reader.readBits<std::uint64_t>(36);
    std::memcpy(m_md5Sum, buffer.data() + 0x22u - sizeof(m_md5Sum), sizeof(m_md5Sum));
}

/*!
 * \class TagParser::FlacMetaDataBlockPicture
 * \brief The FlacMetaDataBlockPicture class is a FLAC "METADATA_BLOCK_PICTURE" parser and maker.
 * \sa https://xiph.org/flac/format.html
 */

/*!
 * \brief Parses the FLAC "METADATA_BLOCK_PICTURE".
 *
 * \a maxSize specifies the maximum size of the structure.
 */
void FlacMetaDataBlockPicture::parse(istream &inputStream, std::uint32_t maxSize)
{
    CHECK_MAX_SIZE(32);
    BinaryReader reader(&inputStream);
    m_pictureType = reader.readUInt32BE();
    std::uint32_t size = reader.readUInt32BE();
    CHECK_MAX_SIZE(size);
    m_value.setMimeType(reader.readString(size));
    size = reader.readUInt32BE();
    CHECK_MAX_SIZE(size);
    m_value.setDescription(reader.readString(size));
    // skip width, height, color depth, number of colors used
    inputStream.seekg(4 * 4, ios_base::cur);
    size = reader.readUInt32BE();
    CHECK_MAX_SIZE(size);
    if (size) {
        auto data = make_unique<char[]>(size);
        inputStream.read(data.get(), size);
        m_value.assignData(std::move(data), size, TagDataType::Picture);
    } else {
        m_value.clearData();
    }
}

/*!
 * \brief Returns the number of bytes make() will write.
 * \remarks Any changes to the object will invalidate this value.
 * \throws Throws an InvalidDataException() if the assigned data is too big.
 */
std::uint32_t FlacMetaDataBlockPicture::requiredSize() const
{
    const auto requiredSize(32 + m_value.mimeType().size() + m_value.description().size() + m_value.dataSize());
    if (requiredSize > numeric_limits<std::uint32_t>::max()) {
        throw InvalidDataException();
    }
    return static_cast<std::uint32_t>(requiredSize);
}

/*!
 * \brief Makes the FLAC "METADATA_BLOCK_PICTURE".
 * \throws Throws an InvalidDataException() if the assigned data can not be serialized into a "METADATA_BLOCK_PICTURE" structure.
 */
void FlacMetaDataBlockPicture::make(ostream &outputStream)
{
    if (m_value.mimeType().size() > numeric_limits<std::uint32_t>::max() || m_value.description().size() > numeric_limits<std::uint32_t>::max()
        || m_value.dataSize() > numeric_limits<std::uint32_t>::max()) {
        throw InvalidDataException();
    }
    BinaryWriter writer(&outputStream);
    writer.writeUInt32BE(pictureType());
    writer.writeUInt32BE(static_cast<std::uint32_t>(m_value.mimeType().size()));
    writer.writeString(m_value.mimeType());
    writer.writeUInt32BE(static_cast<std::uint32_t>(m_value.description().size()));
    writer.writeString(m_value.description());
    writer.writeUInt32BE(0); // skip width
    writer.writeUInt32BE(0); // skip height
    writer.writeUInt32BE(0); // skip color depth
    writer.writeUInt32BE(0); // skip number of colors used
    writer.writeUInt32BE(static_cast<std::uint32_t>(m_value.dataSize()));
    writer.write(value().dataPointer(), static_cast<streamoff>(m_value.dataSize()));
}

} // namespace TagParser
