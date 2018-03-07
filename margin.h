#ifndef TAG_PARSER_MARGIN_H
#define TAG_PARSER_MARGIN_H

#include "./global.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/types.h>

#include <string>

namespace TagParser {

/*!
 * \brief The Margin class defines the four margins of a rectangle.
 */
class TAG_PARSER_EXPORT Margin {
public:
    constexpr Margin(uint32 top = 0, uint32 left = 0, uint32 bottom = 0, uint32 right = 0);
    constexpr uint32 top() const;
    void setTop(uint32 top);
    constexpr uint32 left() const;
    void setLeft(uint32 left);
    constexpr uint32 bottom() const;
    void setBottom(uint32 bottom);
    constexpr uint32 right() const;
    void setRight(uint32 right);
    constexpr bool isNull() const;
    std::string toString() const;

private:
    uint32 m_top;
    uint32 m_left;
    uint32 m_bottom;
    uint32 m_right;
};

/*!
 * \brief Constructs a Margin.
 */
constexpr Margin::Margin(uint32 top, uint32 left, uint32 bottom, uint32 right)
    : m_top(top)
    , m_left(left)
    , m_bottom(bottom)
    , m_right(right)
{
}

/*!
 * \brief Returns the top margin.
 */
constexpr uint32 Margin::top() const
{
    return m_top;
}

/*!
 * \brief Sets the top margin to \a top.
 */
inline void Margin::setTop(uint32 top)
{
    m_top = top;
}

/*!
 * \brief Returns the left margin.
 */
constexpr uint32 Margin::left() const
{
    return m_left;
}

/*!
 * \brief Sets the left margin to \a left.
 */
inline void Margin::setLeft(uint32 left)
{
    m_left = left;
}

/*!
 * \brief Returns the bottom margin.
 */
constexpr uint32 Margin::bottom() const
{
    return m_bottom;
}

/*!
 * \brief Sets the bottom margin to \a bottom.
 */
inline void Margin::setBottom(uint32 bottom)
{
    m_bottom = bottom;
}

/*!
 * \brief Returns the right margin.
 */
constexpr uint32 Margin::right() const
{
    return m_right;
}

/*!
 * \brief Sets the right margin to \a right.
 */
inline void Margin::setRight(uint32 right)
{
    m_right = right;
}

/*!
 * \brief Returns true if all margins are is 0; otherwise returns false;
 */
constexpr bool Margin::isNull() const
{
    return m_top == 0 && m_left == 0 && m_bottom == 0 && m_right == 0;
}

/*!
 * \brief Returns a string representation of the margin.
 */
inline std::string Margin::toString() const
{
    return ConversionUtilities::argsToString("top: ", m_top, "; left: ", m_left, "; bottom: ", m_bottom, "; right: ", m_right);
}

} // namespace TagParser

#endif // TAG_PARSER_MARGIN_H
