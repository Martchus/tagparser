#include "./language.h"

#include <unordered_map>

namespace TagParser {

/// \cond
static const auto &languageMapping()
{
#include "resources/languages.h"
    return languages;
}
/// \endcond

/*!
 * \brief Returns the language name for the specified ISO-639-2 code (bibliographic, 639-2/B).
 * \remarks If \a isoCode is unknown an empty string is returned.
 */
const std::string &languageNameFromIso(const std::string &isoCode)
{
    const auto &mapping = languageMapping();
    const auto i = mapping.find(isoCode);
    if (i == mapping.cend()) {
        static const std::string empty;
        return empty;
    }
    return i->second;
}

/*!
 * \brief Returns the language name for the specified ISO-639-2 code (bibliographic, 639-2/B).
 * \remarks If \a isoCode is unknown the \a isoCode itself is returned.
 */
const std::string &languageNameFromIsoWithFallback(const std::string &isoCode)
{
    const auto &mapping = languageMapping();
    const auto i = mapping.find(isoCode);
    if (i == mapping.cend()) {
        return isoCode;
    }
    return i->second;
}

} // namespace TagParser
