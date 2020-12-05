#ifndef TAG_PARSER_ASPECTRATIO_H
#define TAG_PARSER_ASPECTRATIO_H

#include "./global.h"

#include <c++utilities/conversion/stringbuilder.h>

#include <cstdint>
#include <string>

namespace TagParser {

struct TAG_PARSER_EXPORT AspectRatio {
    constexpr explicit AspectRatio();
    explicit AspectRatio(std::uint8_t aspectRatioType);
    constexpr explicit AspectRatio(std::uint16_t numerator, std::uint16_t denominator);
    constexpr bool isValid() const;
    constexpr bool isExtended() const;
    std::string toString() const;

    std::uint8_t type;
    std::uint16_t numerator;
    std::uint16_t denominator;
};

/*!
 * \brief Constructs an invalid aspect ratio.
 */
constexpr AspectRatio::AspectRatio()
    : type(0)
    , numerator(0)
    , denominator(0)
{
}

/*!
 * \brief Constructs a aspect ratio with the specified \a numerator and \a denominator.
 * \remarks Allows defining a custom aspect ratio, hence counts as "extended" (see isExtended()).
 */
constexpr AspectRatio::AspectRatio(std::uint16_t numerator, std::uint16_t denominator)
    : type(0xFF)
    , numerator(numerator)
    , denominator(denominator)
{
}

/*!
 * \brief Returns an indication whether the aspect ratio is present and valid.
 */
constexpr bool AspectRatio::isValid() const
{
    return type && numerator && denominator;
}

/*!
 * \brief Returns whether numerator and denominator must be read from extended SAR header.
 */
constexpr bool AspectRatio::isExtended() const
{
    return type == 0xFF;
}

/*!
 * \brief Returns the string representation "numerator : denominator".
 */
inline std::string AspectRatio::toString() const
{
    return CppUtilities::argsToString(numerator, " : ", denominator);
}

} // namespace TagParser

#endif // TAG_PARSER_ASPECTRATIO_H
