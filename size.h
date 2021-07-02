#ifndef TAG_PARSER_SIZE_H
#define TAG_PARSER_SIZE_H

#include "./global.h"

#include <c++utilities/conversion/stringbuilder.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace TagParser {

/*!
 * \brief The Size class defines the size of a two-dimensional object using integer point precision.
 */
class TAG_PARSER_EXPORT Size {
public:
    constexpr Size();
    constexpr Size(std::uint32_t width, std::uint32_t height);

    constexpr std::uint32_t width() const;
    constexpr std::uint32_t height() const;
    void setWidth(std::uint32_t value);
    void setHeight(std::uint32_t value);
    constexpr std::uint32_t resolution() const;
    std::string_view abbreviation() const;

    bool constexpr isNull() const;
    bool constexpr operator==(const Size &other) const;
    bool constexpr operator>=(const Size &other) const;
    std::string toString() const;

private:
    std::uint32_t m_width;
    std::uint32_t m_height;
};

/*!
 * \brief Constructs a new Size.
 */
constexpr Size::Size()
    : m_width(0)
    , m_height(0)
{
}

/*!
 * \brief Constructs a new Size of the specified \a width and \a height.
 */
constexpr Size::Size(std::uint32_t width, std::uint32_t height)
    : m_width(width)
    , m_height(height)
{
}

/*!
 * \brief Returns the width.
 */
constexpr std::uint32_t Size::width() const
{
    return m_width;
}

/*!
 * \brief Returns the height.
 */
constexpr std::uint32_t Size::height() const
{
    return m_height;
}

/*!
 * \brief Sets the width.
 */
inline void Size::setWidth(std::uint32_t value)
{
    m_width = value;
}

/*!
 * \brief Sets the height.
 */
inline void Size::setHeight(std::uint32_t value)
{
    m_height = value;
}

/*!
 * \brief Returns the resolution of the current instance (product of with and height).
 */
constexpr std::uint32_t Size::resolution() const
{
    return m_width * m_height;
}

/*!
 * \brief Returns an indication whether both the width and height is 0.
 */
constexpr bool Size::isNull() const
{
    return (m_width == 0) && (m_height == 0);
}

/*!
 * \brief Returns whether this instance equals \a other.
 */
constexpr bool Size::operator==(const Size &other) const
{
    return (m_width == other.m_width) && (m_height == other.m_height);
}

/*!
 * \brief Returns whether this instance is greater than \a other.
 * \remarks Both dimensions must be greater. This operator does *not* take the resolution() into account.
 */
constexpr bool Size::operator>=(const Size &other) const
{
    return (m_width >= other.m_width) && (m_height >= other.m_height);
}

/*!
 * \brief Returns the string representation of the current size.
 */
inline std::string Size::toString() const
{
    return CppUtilities::argsToString("width: ", m_width, ", height: ", m_height);
}

} // namespace TagParser

#endif // TAG_PARSER_SIZE_H
