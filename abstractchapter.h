#ifndef MEDIA_ABSTRACTCHAPTER_H
#define MEDIA_ABSTRACTCHAPTER_H

#include "./statusprovider.h"
#include "./localeawarestring.h"

#include <c++utilities/chrono/timespan.h>

#include <string>
#include <vector>

namespace Media {

class TAG_PARSER_EXPORT AbstractChapter : public StatusProvider
{
public:
    virtual ~AbstractChapter();

    uint64 id() const;
    const std::vector<LocaleAwareString> &names() const;
    ChronoUtilities::TimeSpan startTime() const;
    ChronoUtilities::TimeSpan endTime() const;
    const std::vector<uint64> &tracks() const;
    bool isHidden() const;
    bool isEnabled() const;
    std::string label() const;
    virtual AbstractChapter *nestedChapter(std::size_t index);
    virtual const AbstractChapter *nestedChapter(std::size_t index) const;
    virtual std::size_t nestedChapterCount() const;
    virtual void clear();
    void parse();
    void parseNested();

protected:
    AbstractChapter();
    virtual void internalParse() = 0;

    uint64 m_id;
    std::vector<LocaleAwareString> m_names;
    ChronoUtilities::TimeSpan m_startTime;
    ChronoUtilities::TimeSpan m_endTime;
    std::vector<uint64> m_tracks;
    bool m_hidden;
    bool m_enabled;
};

/*!
 * \brief Returns the chapter ID if known; otherwise returns zero.
 */
inline uint64 AbstractChapter::id() const
{
    return m_id;
}

/*!
 * \brief Returns the chapter name.
 */
inline const std::vector<LocaleAwareString> &AbstractChapter::names() const
{
    return m_names;
}

/*!
 * \brief Returns the start time if known; otherwise returns a negative time span.
 */
inline ChronoUtilities::TimeSpan AbstractChapter::startTime() const
{
    return m_startTime;
}

/*!
 * \brief Returns the end time if known; otherwise returns a negative time span.
 */
inline ChronoUtilities::TimeSpan AbstractChapter::endTime() const
{
    return m_endTime;
}

/*!
 * \brief Returns a list of tracks on which the chapter applies.
 */
inline const std::vector<uint64> &AbstractChapter::tracks() const
{
    return m_tracks;
}

/*!
 * \brief Returns whether the chapter is flagged as hidden.
 */
inline bool AbstractChapter::isHidden() const
{
    return m_hidden;
}

/*!
 * \brief Returns whether the chapter is flagged as enabled.
 */
inline bool AbstractChapter::isEnabled() const
{
    return m_enabled;
}

/*!
 * \brief Returns the nested chapter with the specified \a index.
 */
inline AbstractChapter *AbstractChapter::nestedChapter(std::size_t )
{
    return nullptr;
}

/*!
 * \brief Returns the nested chapter with the specified \a index.
 */
inline const AbstractChapter *AbstractChapter::nestedChapter(std::size_t ) const
{
    return nullptr;
}

/*!
 * \brief Returns the number of nested chapters.
 */
inline std::size_t AbstractChapter::nestedChapterCount() const
{
    return 0;
}

} // namespace Media

#endif // MEDIA_ABSTRACTCHAPTER_H
