#ifndef TAG_PARSER_LANGUAGE_H
#define TAG_PARSER_LANGUAGE_H

#include "./global.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace TagParser {

/// \brief The LocaleFormat enum class specifies the format used by a LocaleDetail.
enum class LocaleFormat : std::uint64_t {
    Unknown, /**< the format is unknown */
    DomainCountry, /**< a country as used by [Internet domains](https://www.iana.org/domains/root/db) (e.g. "de" for Germany or "at" for Austria) */
    ISO_639_1, /**< a language specified via ISO-639-1 code (e.g. "de" for German) */
    ISO_639_2_T, /**< a language specified via ISO-639-2/T code (terminological, e.g. "deu" for German) */
    ISO_639_2_B, /**< a language specified via ISO-639-2/B code (bibliographic, e.g. "ger" for German) */
    BCP_47, /**< a language and/or country according to [BCP 47](https://tools.ietf.org/html/bcp47) using
                the [IANA Language Subtag Registry](https://www.iana.com/assignments/language-subtag-registry/language-subtag-registry)
                (e.g. "de_DE" for the language/country German/Germany or "de_AT" for German/Austria) */
};

/// \brief The LocaleDetail struct specifies a language and/or country.
struct TAG_PARSER_EXPORT LocaleDetail : public std::string {
    explicit LocaleDetail();
    explicit LocaleDetail(std::string_view value, LocaleFormat format);
    explicit LocaleDetail(std::string &&value, LocaleFormat format);
    LocaleFormat format = LocaleFormat::Unknown;
    static const LocaleDetail &getEmpty();
};

/*!
 * \brief Constructs an empty LocaleDetail.
 */
inline LocaleDetail::LocaleDetail()
    : format(LocaleFormat::Unknown)
{
}

/*!
 * \brief Constructs a new LocaleDetail making a copy of \a value.
 */
inline LocaleDetail::LocaleDetail(std::string_view value, LocaleFormat format)
    : std::string(value)
    , format(format)
{
}

/*!
 * \brief Constructs a new LocaleDetail moving the specified \a value.
 */
inline LocaleDetail::LocaleDetail(std::string &&value, LocaleFormat format)
    : std::string(std::move(value))
    , format(format)
{
}

/// \brief The Locale struct specifies a language and/or a country using one or more LocaleDetail objects.
struct TAG_PARSER_EXPORT Locale : public std::vector<LocaleDetail> {
    explicit Locale() = default;
    explicit Locale(std::initializer_list<LocaleDetail> details);
    explicit Locale(std::string &&value, LocaleFormat format);
    explicit Locale(std::string_view value, LocaleFormat format);
    const LocaleDetail &abbreviatedName(LocaleFormat format) const;
    template <typename LocaleFormat, typename... LocaleFormats>
    const LocaleDetail &abbreviatedName(LocaleFormat format, LocaleFormats... moreFormats) const;
    const LocaleDetail &someAbbreviatedName(LocaleFormat preferredFormat = LocaleFormat::BCP_47) const;
    const std::string &fullName() const;
    const std::string &fullOrSomeAbbreviatedName() const;
    std::string toString() const;
};

/*!
 * \brief Returns the abbreviated name of the specified \a format; if not present, checks \a moreFormats.
 */
template <typename LocaleFormat, typename... LocaleFormats>
inline const LocaleDetail &Locale::abbreviatedName(LocaleFormat format, LocaleFormats... moreFormats) const
{
    if (const auto &detail = abbreviatedName(format); !detail.empty()) {
        return detail;
    } else {
        return abbreviatedName(moreFormats...);
    }
}

/*!
 * \brief Constructs a new locale with the specified \a details.
 */
inline Locale::Locale(std::initializer_list<LocaleDetail> details)
    : std::vector<LocaleDetail>(details)
{
}

/*!
 * \brief Constructs a new locale with the specified \a value and \a format.
 */
inline Locale::Locale(std::string &&value, LocaleFormat format)
    : std::vector<LocaleDetail>()
{
    emplace_back(std::move(value), format);
}

/*!
 * \brief Constructs a new locale with the specified \a value and \a format.
 */
inline Locale::Locale(std::string_view value, LocaleFormat format)
    : std::vector<LocaleDetail>()
{
    emplace_back(value, format);
}

} // namespace TagParser

#endif // TAG_PARSER_LANGUAGE_H
