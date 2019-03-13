#ifndef TAG_PARSER_VORBISPACKAGETYPES_H
#define TAG_PARSER_VORBISPACKAGETYPES_H

#include <cstdint>

namespace TagParser {

/*!
 * \brief Encapsulates known Vorbis package type IDs.
 */
namespace VorbisPackageTypes {
enum KnownType : std::uint8_t { Identification = 0x1, Comments = 0x3, Setup = 0x5 };
}

} // namespace TagParser

#endif // TAG_PARSER_VORBISPACKAGETYPES_H
