#ifndef MEDIA_BITMAPINFOHEADER_H
#define MEDIA_BITMAPINFOHEADER_H

#include <c++utilities/application/global.h>
#include <c++utilities/conversion/types.h>

namespace IoUtilities {
class BinaryReader;
}

namespace Media {

class LIB_EXPORT BitmapInfoHeader
{
public:
    BitmapInfoHeader();

    void parse(IoUtilities::BinaryReader &reader);

    uint32 size;
    uint32 width;
    uint32 height;
    uint16 planes;
    uint16 bitCount;
    uint32 compression;
    uint32 imageSize;
    uint32 horizontalResolution;
    uint32 verticalResolution;
    uint32 clrUsed;
    uint32 clrImportant;
};

} // namespace Media

#endif // MEDIA_BITMAPINFOHEADER_H
