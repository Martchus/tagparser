#ifndef SIZE_H
#define SIZE_H

#include "./global.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/conversion/stringbuilder.h>

#include <string>

namespace Media {

/*!
 * \brief The Size class defines the size of a two-dimensional object using integer point precision.
 */
class TAG_PARSER_EXPORT Size
{
public:
    constexpr Size();
    constexpr Size(uint32 width, uint32 height);

    constexpr uint32 width() const;
    constexpr uint32 height() const;
    void setWidth(uint32 value);
    void setHeight(uint32 value);

    bool constexpr isNull() const;
    bool constexpr operator==(const Size &other) const;
    std::string toString() const;

private:
    uint32 m_width;
    uint32 m_height;
};

/*!
 * \brief Constructs a new Size.
 */
constexpr Size::Size() :
    m_width(0),
    m_height(0)
{}

/*!
 * \brief Constructs a new Size of the specified \a width and \a height.
 */
constexpr Size::Size(uint32 width, uint32 height) :
    m_width(width),
    m_height(height)
{}

/*!
 * \brief Returns the width.
 */
constexpr uint32 Size::width() const
{
    return m_width;
}

/*!
 * \brief Returns the height.
 */
constexpr uint32 Size::height() const
{
    return m_height;
}

/*!
 * \brief Sets the width.
 */
inline void Size::setWidth(uint32 value)
{
    m_width = value;
}

/*!
 * \brief Sets the height.
 */
inline void Size::setHeight(uint32 value)
{
    m_height = value;
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
 * \brief Returns the string representation of the current size.
 */
inline std::string Size::toString() const
{
    return ConversionUtilities::argsToString("width: ", m_width, ", height: ", m_height);
}

}

#endif // SIZE_H
