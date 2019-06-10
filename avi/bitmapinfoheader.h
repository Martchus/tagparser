#ifndef TAG_PARSER_BITMAPINFOHEADER_H
#define TAG_PARSER_BITMAPINFOHEADER_H

#include "../global.h"

#include <cstdint>

namespace CppUtilities {
class BinaryReader;
}

namespace TagParser {

class TAG_PARSER_EXPORT BitmapInfoHeader {
public:
    BitmapInfoHeader();

    void parse(CppUtilities::BinaryReader &reader);

    std::uint32_t size;
    std::uint32_t width;
    std::uint32_t height;
    std::uint16_t planes;
    std::uint16_t bitCount;
    std::uint32_t compression;
    std::uint32_t imageSize;
    std::uint32_t horizontalResolution;
    std::uint32_t verticalResolution;
    std::uint32_t clrUsed;
    std::uint32_t clrImportant;
};

} // namespace TagParser

#endif // TAG_PARSER_BITMAPINFOHEADER_H
