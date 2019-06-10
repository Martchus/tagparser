#include "./bitmapinfoheader.h"

#include <c++utilities/io/binaryreader.h>

using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::BitmapInfoHeader
 * \brief The BitmapInfoHeader class parses the BITMAPINFOHEADER structure defined by MS.
 */

/*!
 * \brief Constructs a new BitmapInfoHeader.
 */
BitmapInfoHeader::BitmapInfoHeader()
    : size(0)
    , width(0)
    , height(0)
    , planes(0)
    , bitCount(0)
    , compression(0)
    , imageSize(0)
    , horizontalResolution(0)
    , verticalResolution(0)
    , clrUsed(0)
    , clrImportant(0)
{
}

/*!
 * \brief Parses the BITMAPINFOHEADER structure using the specified \a reader.
 * \remarks 0x28 byte will be read from the associated stream.
 */
void BitmapInfoHeader::parse(BinaryReader &reader)
{
    size = reader.readUInt32LE();
    width = reader.readUInt32LE();
    height = reader.readUInt32LE();
    planes = reader.readUInt16LE();
    bitCount = reader.readUInt16LE();
    compression = reader.readUInt32BE();
    imageSize = reader.readUInt32LE();
    horizontalResolution = reader.readUInt32LE();
    verticalResolution = reader.readUInt32LE();
    clrUsed = reader.readUInt32LE();
    clrImportant = reader.readUInt32LE();
}

} // namespace TagParser
