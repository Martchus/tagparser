#ifndef TAG_PARSER_AV1CONFIGURATION_H
#define TAG_PARSER_AV1CONFIGURATION_H

#include "../global.h"

#include <cstdint>

namespace CppUtilities {
class BinaryReader;
}

namespace TagParser {

class MediaFormat;
class Diagnostics;

struct TAG_PARSER_EXPORT Av1Configuration {
    Av1Configuration();
    std::uint64_t markerAndVersion;
    std::uint64_t profileAndLevel;
    std::uint8_t tier;
    std::uint8_t highBitdepth;
    std::uint8_t twelveBit;
    std::uint8_t monochrome;
    std::uint8_t chromaSubsamplingX;
    std::uint8_t chromaSubsamplingY;
    std::uint16_t chromaSamplePosition;

    void parse(CppUtilities::BinaryReader &reader, std::uint64_t maxSize, Diagnostics &diag);
};

/*!
 * \brief Constructs an empty AVC configuration.
 */
inline Av1Configuration::Av1Configuration()
    : markerAndVersion(0)
    , profileAndLevel(0)
    , tier(0)
    , highBitdepth(0)
    , twelveBit(0)
    , monochrome(0)
    , chromaSubsamplingX(0)
    , chromaSubsamplingY(0)
    , chromaSamplePosition(0)
{
}

} // namespace TagParser

#endif // TAG_PARSER_AV1CONFIGURATION_H
