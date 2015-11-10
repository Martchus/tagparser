#ifndef MEDIA_LOCALEAWARESTRING_H
#define MEDIA_LOCALEAWARESTRING_H

#include <c++utilities/application/global.h>

#include <string>
#include <vector>

namespace Media {

class LIB_EXPORT LocaleAwareString : public std::string
{
public:
    LocaleAwareString(const std::string &value = std::string());
    ~LocaleAwareString();

    const std::vector<std::string> &languages() const;
    std::vector<std::string> &languages();
    const std::vector<std::string> &countries() const;
    std::vector<std::string> &countries();

private:
    std::vector<std::string> m_languages;
    std::vector<std::string> m_countries;
};

inline LocaleAwareString::LocaleAwareString(const std::string &value) :
    std::string(value)
{}

inline LocaleAwareString::~LocaleAwareString()
{}

inline const std::vector<std::string> &LocaleAwareString::languages() const
{
    return m_languages;
}

inline std::vector<std::string> &LocaleAwareString::languages()
{
    return m_languages;
}

inline const std::vector<std::string> &LocaleAwareString::countries() const
{
    return m_countries;
}

inline std::vector<std::string> &LocaleAwareString::countries()
{
    return m_countries;
}

} // namespace Media

#endif // MEDIA_LOCALEAWARESTRING_H
