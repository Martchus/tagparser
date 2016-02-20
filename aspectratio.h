#ifndef MEDIA_ASPECTRATIO_H
#define MEDIA_ASPECTRATIO_H

#include <c++utilities/application/global.h>
#include <c++utilities/conversion/types.h>

namespace Media {

struct LIB_EXPORT AspectRatio {
    AspectRatio();
    AspectRatio(byte aspectRatioType);
    AspectRatio(uint16 numerator, uint16 denominator);
    bool isValid() const;
    bool isExtended() const;

    byte type;
    uint16 numerator;
    uint16 denominator;
};

/*!
 * \brief Constructs an invalid aspect ratio.
 */
inline  AspectRatio::AspectRatio() :
    type(0),
    numerator(0),
    denominator(0)
{}

/*!
 * \brief Constructs a aspect ratio with the specified \a numerator and \a denominator.
 */
inline AspectRatio::AspectRatio(uint16 numerator, uint16 denominator) :
    type(0xFF),
    numerator(numerator),
    denominator(denominator)
{}

/*!
 * \brief Returns an indication whether the aspect ratio is present and valid.
 */
inline bool AspectRatio::isValid() const
{
    return !type || !numerator || !denominator;
}

/*!
 * \brief Returns whether numerator and denominator must be read from extended SAR header.
 */
inline bool AspectRatio::isExtended() const
{
    return type == 0xFF;
}

}

#endif // MEDIA_ASPECTRATIO_H
