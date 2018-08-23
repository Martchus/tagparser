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
    Av1Configuration();
    uint64 markerAndVersion;
    uint64 profileAndLevel;
    byte tier;
    byte highBitdepth;
    byte twelveBit;
    byte monochrome;
    byte chromaSubsamplingX;
    byte chromaSubsamplingY;
    uint16 chromaSamplePosition;

    void parse(IoUtilities::BinaryReader &reader, uint64 maxSize, Diagnostics &diag);
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
