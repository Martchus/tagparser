#ifndef TAG_PARSER_MARGIN_H
#define TAG_PARSER_MARGIN_H

#include "./global.h"

#include <c++utilities/conversion/stringbuilder.h>

#include <cstdint>
#include <string>

namespace TagParser {

/*!
 * \brief The Margin class defines the four margins of a rectangle.
 */
class TAG_PARSER_EXPORT Margin {
public:
    constexpr explicit Margin(std::uint32_t top = 0, std::uint32_t left = 0, std::uint32_t bottom = 0, std::uint32_t right = 0);
    constexpr std::uint32_t top() const;
    void setTop(std::uint32_t top);
    constexpr std::uint32_t left() const;
    void setLeft(std::uint32_t left);
    constexpr std::uint32_t bottom() const;
    void setBottom(std::uint32_t bottom);
    constexpr std::uint32_t right() const;
    void setRight(std::uint32_t right);
    constexpr bool isNull() const;
    std::string toString() const;

private:
    std::uint32_t m_top;
    std::uint32_t m_left;
    std::uint32_t m_bottom;
    std::uint32_t m_right;
};

/*!
 * \brief Constructs a Margin.
 */
constexpr Margin::Margin(std::uint32_t top, std::uint32_t left, std::uint32_t bottom, std::uint32_t right)
    : m_top(top)
    , m_left(left)
    , m_bottom(bottom)
    , m_right(right)
{
}

/*!
 * \brief Returns the top margin.
 */
constexpr std::uint32_t Margin::top() const
{
    return m_top;
}

/*!
 * \brief Sets the top margin to \a top.
 */
inline void Margin::setTop(std::uint32_t top)
{
    m_top = top;
}

/*!
 * \brief Returns the left margin.
 */
constexpr std::uint32_t Margin::left() const
{
    return m_left;
}

/*!
 * \brief Sets the left margin to \a left.
 */
inline void Margin::setLeft(std::uint32_t left)
{
    m_left = left;
}

/*!
 * \brief Returns the bottom margin.
 */
constexpr std::uint32_t Margin::bottom() const
{
    return m_bottom;
}

/*!
 * \brief Sets the bottom margin to \a bottom.
 */
inline void Margin::setBottom(std::uint32_t bottom)
{
    m_bottom = bottom;
}

/*!
 * \brief Returns the right margin.
 */
constexpr std::uint32_t Margin::right() const
{
    return m_right;
}

/*!
 * \brief Sets the right margin to \a right.
 */
inline void Margin::setRight(std::uint32_t right)
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
    return CppUtilities::argsToString("top: ", m_top, "; left: ", m_left, "; bottom: ", m_bottom, "; right: ", m_right);
}

} // namespace TagParser

#endif // TAG_PARSER_MARGIN_H
