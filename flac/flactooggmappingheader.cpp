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
    char buff[0x0D + 0x04 + 0x22 - 0x05];
    iterator.read(buff, 5);
    if (*buff != 0x7Fu || BE::toUInt32(buff + 1) != 0x464C4143u) {
        throw InvalidDataException(); // not FLAC-to-Ogg mapping header
    }
    iterator.read(buff, sizeof(buff));

    // parse FLAC-to-Ogg mapping header
    m_majorVersion = static_cast<std::uint8_t>(*(buff + 0x00));
    m_minorVersion = static_cast<std::uint8_t>(*(buff + 0x01));
    m_headerCount = BE::toUInt16(buff + 0x02);
    if (BE::toUInt32(buff + 0x04) != 0x664C6143u) {
        throw InvalidDataException(); // native FLAC signature not present
    }

    // parse "METADATA_BLOCK_HEADER"
    FlacMetaDataBlockHeader header;
    header.parseHeader(buff + 0x0D - 0x05);
    if (header.type() != FlacMetaDataBlockType::StreamInfo) {
        throw InvalidDataException(); // "METADATA_BLOCK_STREAMINFO" expected
    }
    if (header.dataSize() < 0x22) {
        throw TruncatedDataException(); // "METADATA_BLOCK_STREAMINFO" is truncated
    }

    // parse "METADATA_BLOCK_STREAMINFO"
    m_streamInfo.parse(buff + 0x0D + 0x04 - 0x05);
}

} // namespace TagParser
