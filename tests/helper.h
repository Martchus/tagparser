#ifndef TAGPARSER_TEST_HELPER
#define TAGPARSER_TEST_HELPER

#include "../tagvalue.h"

#include <ostream>

std::ostream &operator <<(std::ostream &os, const Media::TagTextEncoding &encoding);

/*!
 * \brief Prints a TagValue UTF-8 encoded to enable CPPUNIT_ASSERT_EQUAL for tag values.
 */
inline std::ostream &operator <<(std::ostream &os, const Media::TagValue &tagValue)
{
    return os << tagValue.toString(Media::TagTextEncoding::Utf8) << " (encoding: " << tagValue.dataEncoding() << ")";
}

#endif // TAGPARSER_TEST_HELPER
