#ifndef TAG_PARSER_AACCODEBOOK_H
#define TAG_PARSER_AACCODEBOOK_H

// NOTE: The AAC parser is still WIP. It does not work yet and its API/ABI may change even in patch releases.

#include <c++utilities/application/global.h>
#include <c++utilities/conversion/types.h>

namespace TagParser {

struct LIB_EXPORT AacHcb
{
    byte offset;
    byte extraBits;
};

struct LIB_EXPORT AacHcb2Pair
{
    byte bits;
    sbyte x;
    sbyte y;
};

struct LIB_EXPORT AacHcb2Quad
{
    byte bits;
    sbyte x;
    sbyte y;
    sbyte v;
    sbyte w;
};

struct LIB_EXPORT AacHcbBinPair
{
    byte isLeaf;
    sbyte data[2];
};

struct LIB_EXPORT AacHcbBinQuad
{
    byte isLeaf;
    sbyte data[4];
};

extern const AacHcb *const aacHcbTable[];
extern const AacHcb2Pair *const aacHcb2PairTable[];
extern const AacHcb2Quad *const aacHcb2QuadTable[];
extern const AacHcbBinPair *const aacHcbBinTable[];
extern const byte aacHcbN[];
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
extern const byte aacHcbSf[][2];

extern const sbyte tHuffmanEnv15dB[120][2];
extern const sbyte fHuffmanEnv15dB[120][2];
extern const sbyte tHuffmanEnvBal15dB[48][2];
extern const sbyte fHuffmanEnvBal15dB[48][2];
extern const sbyte tHuffmanEnv30dB[62][2];
extern const sbyte fHuffmanEnv30dB[62][2];
extern const sbyte tHuffmanEnvBal30dB[24][2];
extern const sbyte fHuffmanEnvBal30dB[24][2];
extern const sbyte tHuffmanNoise30dB[62][2];
extern const sbyte tHuffmanNoiseBal30dB[24][2];

}

#endif // TAG_PARSER_AACCODEBOOK_H
