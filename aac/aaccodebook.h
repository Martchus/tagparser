#ifndef TAG_PARSER_AACCODEBOOK_H
#define TAG_PARSER_AACCODEBOOK_H

// NOTE: The AAC parser is still WIP. It does not work yet and its API/ABI may change even in patch releases.

#include "../global.h"

#include <cstdint>

namespace TagParser {

/// \cond

struct TAG_PARSER_EXPORT AacHcb {
    std::uint8_t offset;
    std::uint8_t extraBits;
};

struct TAG_PARSER_EXPORT AacHcb2Pair {
    std::uint8_t bits;
    std::int8_t x;
    std::int8_t y;
};

struct TAG_PARSER_EXPORT AacHcb2Quad {
    std::uint8_t bits;
    std::int8_t x;
    std::int8_t y;
    std::int8_t v;
    std::int8_t w;
};

struct TAG_PARSER_EXPORT AacHcbBinPair {
    std::uint8_t isLeaf;
    std::int8_t data[2];
};

struct TAG_PARSER_EXPORT AacHcbBinQuad {
    std::uint8_t isLeaf;
    std::int8_t data[4];
};

extern const AacHcb *const aacHcbTable[];
extern const AacHcb2Pair *const aacHcb2PairTable[];
extern const AacHcb2Quad *const aacHcb2QuadTable[];
extern const AacHcbBinPair *const aacHcbBinTable[];
extern const std::uint8_t aacHcbN[];
extern const int aacHcb2QuadTableSize[];
extern const int aacHcb2PairTableSize[];
extern const int aacHcbBinTableSize[];

extern const AacHcb aacHcb1Step1[];
extern const AacHcb2Quad aacHcb1Step2[];
extern const AacHcb aacHcb2Step1[];
extern const AacHcb2Quad aacHcb2Step2[];
extern const AacHcbBinQuad aacHcb3[];
extern const AacHcb aacHcb4Step1[];
extern const AacHcb2Quad aacHcb4Step2[];
extern const AacHcbBinPair aacHcb5[];
extern const AacHcb aacHcb6Step1[];
extern const AacHcb2Pair aacHcb6Step2[];
extern const AacHcbBinPair aacHcb7[];
extern const AacHcb aacHcb8Step1[];
extern const AacHcb2Pair aacHcb8Step2[];
extern const AacHcbBinPair aacHcb9[];
extern const AacHcb aacHcb10Step1[];
extern const AacHcb2Pair aacHcb10Step2[];
extern const AacHcb aacHcb11Step1[];
extern const AacHcb2Pair aacHcb11Step2[];
extern const std::uint8_t aacHcbSf[][2];

extern const std::int8_t tHuffmanEnv15dB[120][2];
extern const std::int8_t fHuffmanEnv15dB[120][2];
extern const std::int8_t tHuffmanEnvBal15dB[48][2];
extern const std::int8_t fHuffmanEnvBal15dB[48][2];
extern const std::int8_t tHuffmanEnv30dB[62][2];
extern const std::int8_t fHuffmanEnv30dB[62][2];
extern const std::int8_t tHuffmanEnvBal30dB[24][2];
extern const std::int8_t fHuffmanEnvBal30dB[24][2];
extern const std::int8_t tHuffmanNoise30dB[62][2];
extern const std::int8_t tHuffmanNoiseBal30dB[24][2];

/// \endcond

} // namespace TagParser

#endif // TAG_PARSER_AACCODEBOOK_H
