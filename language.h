#ifndef TAG_PARSER_LANGUAGE_H
#define TAG_PARSER_LANGUAGE_H

#include "./global.h"

#include <c++utilities/conversion/stringbuilder.h>

#include <cstdint>
#include <string>

namespace TagParser {

/*!
 * \brief Returns whether \a languageSpecification is not empty or undefined.
 */
inline bool isLanguageDefined(const std::string &languageSpecification)
{
    return !languageSpecification.empty() && languageSpecification != "und";
}

TAG_PARSER_EXPORT const std::string &languageNameFromIso(const std::string &isoCode);
TAG_PARSER_EXPORT const std::string &languageNameFromIsoWithFallback(const std::string &isoCode);

} // namespace TagParser

#endif // TAG_PARSER_LANGUAGE_H
