#include "./helper.h"

namespace CppUtilities {

/*!
 * \brief Prints a TagTextEncoding to enable CPPUNIT_ASSERT_EQUAL for tag values.
 */
std::ostream &operator<<(std::ostream &os, const TagParser::TagTextEncoding &encoding)
{
    using namespace TagParser;
    switch (encoding) {
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

} // namespace CppUtilities
