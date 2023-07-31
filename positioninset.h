#ifndef TAG_PARSER_POSITIONINSET_H
#define TAG_PARSER_POSITIONINSET_H

#include "./global.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/misc/traits.h>

#include <string>

namespace TagParser {

/*!
 * \class TagParser::PositionInSet
 * \brief The PositionInSet class describes the position of an element in a
 *        set which consists of a certain number of elements.
 *
 * This class is used to parse and store values like "9/11" which are used
 * by some tag formats to denote track positions.
 */
class TAG_PARSER_EXPORT PositionInSet {
public:
    constexpr explicit PositionInSet(std::int32_t position = 0, std::int32_t total = 0);
    template <typename StringType = std::string,
        CppUtilities::Traits::EnableIfAny<CppUtilities::Traits::IsSpecializationOf<StringType, std::basic_string>> * = nullptr>
    PositionInSet(const StringType &numericString);

    constexpr std::int32_t position() const;
    void setPosition(std::int32_t position);
    constexpr std::int32_t total() const;
    void setTotal(std::int32_t total);
    constexpr bool isNull() const;
    constexpr bool operator==(const PositionInSet &other) const;

    template <typename StringType = std::string,
        CppUtilities::Traits::EnableIfAny<CppUtilities::Traits::IsSpecializationOf<StringType, std::basic_string>> * = nullptr>
    StringType toString() const;

private:
    std::int32_t m_position;
    std::int32_t m_total;
};

/*!
 * \brief Constructs a new Position in set from the specified numeric string.
 * \tparam StringType The type of the string (should be an instantiation of the basic_string class template).
 * \param numericString Specifies the string containing the position and possibly
 *                      the total element count (separated by "/").
 */
template <typename StringType, CppUtilities::Traits::EnableIfAny<CppUtilities::Traits::IsSpecializationOf<StringType, std::basic_string>> *>
PositionInSet::PositionInSet(const StringType &numericString)
    : m_position(0)
    , m_total(0)
{
    const auto separator = numericString.find('/');
    if (separator == StringType::npos || separator == numericString.length() - 1) {
        m_position = CppUtilities::stringToNumber<std::int32_t, StringType>(numericString);
    } else if (separator == 0) {
        m_total = CppUtilities::stringToNumber<std::int32_t, StringType>(numericString.substr(1));
    } else {
        m_position = CppUtilities::stringToNumber<std::int32_t, StringType>(numericString.substr(0, separator));
        m_total = CppUtilities::stringToNumber<std::int32_t, StringType>(numericString.substr(separator + 1));
    }
}

/*!
 * \brief Constructs a new Position in set of the specified element \a position and \a total element count.
 * \param position
 * \param total
 */
constexpr inline PositionInSet::PositionInSet(std::int32_t position, std::int32_t total)
    : m_position(position)
    , m_total(total)
{
}

/*!
 * \brief Returns the element position of the current instance.
 */
constexpr inline std::int32_t PositionInSet::position() const
{
    return m_position;
}

/*!
 * \brief Sets the element position of the current instance.
 */
inline void PositionInSet::setPosition(int32_t position)
{
    m_position = position;
}

/*!
 * \brief Returns the total element count of the current instance.
 */
constexpr inline std::int32_t PositionInSet::total() const
{
    return m_total;
}

/*!
 * \brief Sets the total element count of the current instance.
 */
inline void PositionInSet::setTotal(int32_t total)
{
    m_total = total;
}

/*!
 * \brief Returns an indication whether both the element position and total element count is 0.
 */
constexpr inline bool PositionInSet::isNull() const
{
    return m_position == 0 && m_total == 0;
}

/*!
 * \brief Returns whether this instance equals \a other.
 */
constexpr inline bool PositionInSet::operator==(const PositionInSet &other) const
{
    return m_position == other.m_position && m_total == other.m_total;
}

/*!
 * \brief Returns the string representation of the current PositionInSet.
 */
template <typename StringType, CppUtilities::Traits::EnableIfAny<CppUtilities::Traits::IsSpecializationOf<StringType, std::basic_string>> *>
StringType PositionInSet::toString() const
{
    std::basic_stringstream<typename StringType::value_type> ss;
    if (m_position) {
        ss << m_position;
    }
    if (m_total) {
        ss << '/' << m_total;
    }
    return ss.str();
}

} // namespace TagParser

#endif // TAG_PARSER_POSITIONINSET_H
