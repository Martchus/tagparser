#ifndef POSITIONINSET_H
#define POSITIONINSET_H

#include <c++utilities/application/global.h>
#include <c++utilities/conversion/stringconversion.h>

#include <string>

namespace Media {

/*!
 * \class Media::PositionInSet
 * \brief The PositionInSet class describes the position of an element in a
 *        set which consists of a certain number of elements.
 *
 * This class is used to parse and store values like "9/11" which are used
 * by some tag formats to denote track positions.
 */
class LIB_EXPORT PositionInSet
{
public:
    constexpr PositionInSet(int32 position = 0, int32 total = 0);
    template <typename StringType = std::string>
    PositionInSet(const StringType &numericString);

    constexpr int32 position() const;
    constexpr int32 total() const;
    constexpr bool isNull() const;

    template <typename StringType = std::string>
    StringType toString() const;

private:
    int32 m_position;
    int32 m_total;
};

/*!
 * \brief Constructs a new Position in set from the specified numeric string.
 * \tparam StringType The type of the string (should be an instantiation of the basic_string class template).
 * \param numericString Specifies the string containing the position and possibly
 *                      the total element count (separated by "/").
 */
template <typename StringType>
PositionInSet::PositionInSet(const StringType &numericString) :
    m_position(0),
    m_total(0)
{
    size_t separator = numericString.find('/');
    if(separator == StringType::npos || separator == numericString.length() - 1) {
        m_position = ConversionUtilities::stringToNumber<int32, StringType>(numericString);
    } else if(separator == 0) {
        m_total = ConversionUtilities::stringToNumber<int32, StringType>(numericString.substr(1));
    } else {
        m_position = ConversionUtilities::stringToNumber<int32, StringType>(numericString.substr(0, separator));
        m_total = ConversionUtilities::stringToNumber<int32, StringType>(numericString.substr(separator + 1));
    }
}

/*!
 * \brief Constructs a new Position in set of the specified element \a position and \a total element count.
 * \param position
 * \param total
 */
constexpr inline PositionInSet::PositionInSet(int32 position, int32 total) :
    m_position(position),
    m_total(total)
{}

/*!
 * \brief Returns the element position of the current instance.
 */
constexpr inline int32 PositionInSet::position() const
{
    return m_position;
}

/*!
 * \brief Returns the total element count of the current instance.
 */
constexpr inline int32 PositionInSet::total() const
{
    return m_total;
}

/*!
 * \brief Returns an indication whether both the element position and total element count is 0.
 */
constexpr inline bool PositionInSet::isNull() const
{
    return m_position == 0 && m_total == 0;
}

/*!
 * \brief Returns the string representation of the current PositionInSet.
 */
template <typename StringType>
StringType PositionInSet::toString() const
{
    std::basic_stringstream<typename StringType::value_type> ss;
    if(m_position) {
        ss << m_position;
    }
    ss << "/";
    if(m_total) {
        ss << m_total;
    }
    return ss.str();
}

}

#endif // POSITIONINSET_H
