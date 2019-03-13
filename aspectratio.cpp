#include "./aspectratio.h"

using namespace std;

namespace TagParser {

/*!
 * \struct TagParser::AspectRatio
 * \brief The AspectRatio struct defines an aspect ratio.
 */

/*!
 * \brief Constructs a PAR form the specified AVC aspectRatioType.
 */
AspectRatio::AspectRatio(std::uint8_t aspectRatioType)
{
    static const AspectRatio predefinedPars[] = { AspectRatio(), AspectRatio(1, 1), AspectRatio(12, 11), AspectRatio(10, 11), AspectRatio(16, 11),
        AspectRatio(40, 33), AspectRatio(24, 11), AspectRatio(20, 11), AspectRatio(32, 11), AspectRatio(80, 33), AspectRatio(18, 11),
        AspectRatio(15, 11), AspectRatio(64, 33), AspectRatio(160, 99), AspectRatio(4, 3), AspectRatio(3, 2), AspectRatio(2, 1) };
    if (aspectRatioType < (sizeof(predefinedPars) / sizeof(AspectRatio))) {
        *this = predefinedPars[aspectRatioType];
    } else {
        numerator = denominator = 0;
    }
    type = aspectRatioType;
}

} // namespace TagParser
