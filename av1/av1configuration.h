#ifndef TAG_PARSER_AV1CONFIGURATION_H
#define TAG_PARSER_AV1CONFIGURATION_H

#include "../global.h"

#include <c++utilities/conversion/types.h>

namespace IoUtilities {
class BinaryReader;
}

namespace TagParser {

class MediaFormat;
class Diagnostics;

struct TAG_PARSER_EXPORT Av1Configuration {
    constexpr Av1Configuration();
    // FIXME: make this uint32 in v9
    uint64 markerAndVersion;

    // FIXME: get rid of unused members in v9
    uint64 profileAndLevel;
    byte tier;
    byte highBitdepth;
    byte twelveBit;
    byte monochrome;
    byte chromaSubsamplingX;
    byte chromaSubsamplingY;
    uint16 chromaSamplePosition;

    void parse(IoUtilities::BinaryReader &reader, uint64 maxSize, Diagnostics &diag);

    constexpr byte marker() const;
    constexpr byte version() const;
    constexpr byte sequenceProfile() const;
    constexpr byte sequenceLevelIndex() const;
    constexpr byte sequenceTier() const;
    constexpr byte isHighBitDepth() const;
    constexpr byte isTwelveBit() const;
    constexpr byte isMonochrome() const;
    constexpr byte hasChromaSubsamplingX() const;
    constexpr byte hasChromaSubsamplingY() const;
    constexpr byte chromaSamplePos() const;
};

/*!
 * \brief Constructs an empty AVC configuration.
 */
constexpr Av1Configuration::Av1Configuration()
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

constexpr byte Av1Configuration::marker() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1)) & 0x01);
}

constexpr byte Av1Configuration::version() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7)) & 0x7F);
}

constexpr byte Av1Configuration::sequenceProfile() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3)) & 0x07);
}

constexpr byte Av1Configuration::sequenceLevelIndex() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3 - 5)) & 0x1F);
}

constexpr byte Av1Configuration::sequenceTier() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3 - 5 - 1)) & 0x01);
}

constexpr byte Av1Configuration::isHighBitDepth() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3 - 5 - 1 - 1)) & 0x01);
}

constexpr byte Av1Configuration::isTwelveBit() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3 - 5 - 1 - 1)) & 0x01);
}

constexpr byte Av1Configuration::isMonochrome() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3 - 5 - 1 - 1 - 1)) & 0x01);
}

constexpr byte Av1Configuration::hasChromaSubsamplingX() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3 - 5 - 1 - 1 - 1 - 1)) & 0x01);
}

constexpr byte Av1Configuration::hasChromaSubsamplingY() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3 - 5 - 1 - 1 - 1 - 1 - 1)) & 0x01);
}

constexpr byte Av1Configuration::chromaSamplePos() const
{
    return static_cast<byte>((markerAndVersion >> (32 - 1 - 7 - 3 - 5 - 1 - 1 - 1 - 1 - 1 - 1)) & 0x03);
}

} // namespace TagParser

#endif // TAG_PARSER_AV1CONFIGURATION_H
