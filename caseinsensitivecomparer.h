#ifndef TAG_PARSER_CASEINSENSITIVECOMPARER
#define TAG_PARSER_CASEINSENSITIVECOMPARER

#include "./global.h"

#include <string>

#include <iostream>

namespace TagParser {

/*!
 * \brief The CaseInsensitiveCharComparer struct defines a method for case-insensivive character comparison (less).
 */
struct TAG_PARSER_EXPORT CaseInsensitiveCharComparer {
    static constexpr unsigned char toLower(const unsigned char c)
    {
        return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
    }

    bool operator()(const unsigned char lhs, const unsigned char rhs) const
    {
        return toLower(lhs) < toLower(rhs);
    }
};

/*!
 * \brief The CaseInsensitiveStringComparer struct defines a method for case-insensivive string comparison (less).
 */
struct TAG_PARSER_EXPORT CaseInsensitiveStringComparer {
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), CaseInsensitiveCharComparer());
    }
    bool operator()(std::string_view lhs, std::string_view rhs) const
    {
        return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), CaseInsensitiveCharComparer());
    }
};

} // namespace TagParser

#endif // TAG_PARSER_CASEINSENSITIVECOMPARER
