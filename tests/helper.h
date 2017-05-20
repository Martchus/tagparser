#ifndef TAGPARSER_TEST_HELPER
#define TAGPARSER_TEST_HELPER

#include "../tagvalue.h"

#include <ostream>

/*!
 * \brief Prints a TagTextEncoding to enable CPPUNIT_ASSERT_EQUAL for tag values.
 */
std::ostream &operator <<(std::ostream &os, const Media::TagTextEncoding &encoding)
{
    using namespace Media;
    switch(encoding) {
    case TagTextEncoding::Unspecified:
        return os << "unspecified";
    case TagTextEncoding::Latin1:
        return os << "Latin-1";
    case TagTextEncoding::Utf8:
        return os << "UTF-8";
    case TagTextEncoding::Utf16LittleEndian:
        return os << "UTF-16 LE";
    case TagTextEncoding::Utf16BigEndian:
        return os << "UTF-16 BE";
    }
    return os;
}

/*!
 * \brief Prints a TagValue UTF-8 encoded to enable CPPUNIT_ASSERT_EQUAL for tag values.
 */
std::ostream &operator <<(std::ostream &os, const Media::TagValue &tagValue)
{
    os << tagValue.toString(Media::TagTextEncoding::Utf8) << " (encoding: " << tagValue.dataEncoding() << ")";
    return os;
}

#endif // TAGPARSER_TEST_HELPER
