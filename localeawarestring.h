#ifndef TAG_PARSER_LOCALEAWARESTRING_H
#define TAG_PARSER_LOCALEAWARESTRING_H

#include "./localehelper.h"

#include <string>
#include <vector>

namespace TagParser {

/*!
 * \brief The LocaleAwareString class is a standard string with locale information (languages, countries).
 */
class TAG_PARSER_EXPORT LocaleAwareString : public std::string {
public:
    explicit LocaleAwareString(const std::string &value = std::string());
    explicit LocaleAwareString(std::string &&value);
    ~LocaleAwareString();

    const Locale &locale() const;
    Locale &locale();

private:
    Locale m_locale;
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
 * \brief Returns the associated locale.
 */
inline const Locale &LocaleAwareString::locale() const
{
    return m_locale;
}

/*!
 * \brief Returns the associated locale.
 */
inline Locale &LocaleAwareString::locale()
{
    return m_locale;
}

} // namespace TagParser

#endif // TAG_PARSER_LOCALEAWARESTRING_H
