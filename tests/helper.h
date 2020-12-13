#ifndef TAGPARSER_TEST_HELPER
#define TAGPARSER_TEST_HELPER

#include "../diagnostics.h"
#include "../localehelper.h"
#include "../size.h"
#include "../tagvalue.h"

#include <ostream>

namespace CppUtilities {

std::ostream &operator<<(std::ostream &os, const TagParser::TagTextEncoding &encoding);

/*!
 * \brief Prints a TagValue UTF-8 encoded to enable CPPUNIT_ASSERT_EQUAL for tag values.
 */
inline std::ostream &operator<<(std::ostream &os, const TagParser::TagValue &tagValue)
{
    os << tagValue.toString(TagParser::TagTextEncoding::Utf8);
    if (!tagValue.description().empty()) {
        os << ", description: " << tagValue.description();
    }
    return os << " (encoding: " << tagValue.dataEncoding() << ", description encoding: " << tagValue.descriptionEncoding() << ')';
}

/*!
 * \brief Prints a PositionInSet to enable using it in CPPUNIT_ASSERT_EQUAL.
 */
inline std::ostream &operator<<(std::ostream &os, const TagParser::PositionInSet &pos)
{
    return os << pos.toString();
}

/*!
 * \brief Prints a Size to enable using it in CPPUNIT_ASSERT_EQUAL.
 */
inline std::ostream &operator<<(std::ostream &os, const TagParser::Size &size)
{
    return os << size.toString();
}

/*!
 * \brief Prints a DiagMessage to enable using it in CPPUNIT_ASSERT_EQUAL.
 */
inline std::ostream &operator<<(std::ostream &os, const TagParser::DiagMessage &diagMessage)
{
    return os << diagMessage.levelName() << ':' << ' ' << diagMessage.message() << ' ' << '(' << diagMessage.context() << ')';
}

/*!
 * \brief Prints a Locale to enable using it in CPPUNIT_ASSERT_EQUAL.
 */
inline std::ostream &operator<<(std::ostream &os, const TagParser::Locale &locale)
{
    return os << locale.toString();
}

} // namespace CppUtilities

#endif // TAGPARSER_TEST_HELPER
