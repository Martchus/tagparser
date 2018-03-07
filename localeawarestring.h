#ifndef TAG_PARSER_LOCALEAWARESTRING_H
#define TAG_PARSER_LOCALEAWARESTRING_H

#include "./global.h"

#include <string>
#include <vector>

namespace TagParser {

/*!
 * \brief The LocaleAwareString class is a standard string with locale information (languages, countries).
 */
class TAG_PARSER_EXPORT LocaleAwareString : public std::string {
public:
    LocaleAwareString(const std::string &value = std::string());
    LocaleAwareString(std::string &&value);
    ~LocaleAwareString();

    const std::vector<std::string> &languages() const;
    std::vector<std::string> &languages();
    const std::vector<std::string> &countries() const;
    std::vector<std::string> &countries();

private:
    std::vector<std::string> m_languages;
    std::vector<std::string> m_countries;
};

/*!
 * \brief Constructs a new LocaleAwareString from the specified standard string.
 */
inline LocaleAwareString::LocaleAwareString(const std::string &value)
    : std::string(value)
{
}

/*!
 * \brief Constructs a new LocaleAwareString from the specified standard string.
 */
inline LocaleAwareString::LocaleAwareString(std::string &&value)
    : std::string(value)
{
}

/*!
 * \brief Destroys the instance.
 */
inline LocaleAwareString::~LocaleAwareString()
{
}

/*!
 * \brief Returns associated languages.
 */
inline const std::vector<std::string> &LocaleAwareString::languages() const
{
    return m_languages;
}

/*!
 * \brief Returns associated languages.
 */
inline std::vector<std::string> &LocaleAwareString::languages()
{
    return m_languages;
}

/*!
 * \brief Returns associated countries.
 */
inline const std::vector<std::string> &LocaleAwareString::countries() const
{
    return m_countries;
}

/*!
 * \brief Returns associated countries.
 */
inline std::vector<std::string> &LocaleAwareString::countries()
{
    return m_countries;
}

} // namespace TagParser

#endif // TAG_PARSER_LOCALEAWARESTRING_H
