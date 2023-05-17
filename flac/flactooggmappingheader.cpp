#include "./flactooggmappingheader.h"

#include "../ogg/oggiterator.h"

#include "../exceptions.h"

#include <c++utilities/conversion/binaryconversion.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::FlacToOggMappingHeader
 * \brief The FlacToOggMappingHeader class is a FLAC-to-Ogg mapping header parser.
 * \sa https://xiph.org/flac/ogg_mapping.html
 */

/*!
 * \brief Parses the FLAC-to-Ogg mapping header which is read using the specified \a iterator.
 * \remarks The header is assumed to start at the current position of \a iterator.
 */
void FlacToOggMappingHeader::parseHeader(OggIterator &iterator)
{
    // prepare parsing
    constexpr auto idSize = 0x05, mappingHeaderSize = 0x0D, blockHeaderSize = 0x04, streamInfoSize = 0x22;
    char buff[mappingHeaderSize + blockHeaderSize + streamInfoSize - idSize];
    iterator.read(buff, idSize);
    if (*buff != 0x7Fu || BE::toInt<std::uint32_t>(buff + 1) != 0x464C4143u) {
        throw InvalidDataException(); // not FLAC-to-Ogg mapping header
    }
    iterator.read(buff, sizeof(buff));

    // parse FLAC-to-Ogg mapping header
    m_majorVersion = static_cast<std::uint8_t>(*(buff + 0x00));
    m_minorVersion = static_cast<std::uint8_t>(*(buff + 0x01));
    m_headerCount = BE::toInt<std::uint16_t>(buff + 0x02);
    if (BE::toInt<std::uint32_t>(buff + 0x04) != 0x664C6143u) {
        throw InvalidDataException(); // native FLAC signature not present
    }

    // parse "METADATA_BLOCK_HEADER"
    FlacMetaDataBlockHeader header;
    header.parseHeader(std::string_view(buff + mappingHeaderSize - idSize, blockHeaderSize));
    if (header.type() != FlacMetaDataBlockType::StreamInfo) {
        throw InvalidDataException(); // "METADATA_BLOCK_STREAMINFO" expected
    }
    if (header.dataSize() < streamInfoSize) {
        throw TruncatedDataException(); // "METADATA_BLOCK_STREAMINFO" is truncated
    }

    // parse "METADATA_BLOCK_STREAMINFO"
    m_streamInfo.parse(std::string_view(buff + mappingHeaderSize + blockHeaderSize - idSize, streamInfoSize));
}

} // namespace TagParser
