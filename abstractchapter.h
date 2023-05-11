#ifndef TAG_PARSER_ABSTRACTCHAPTER_H
#define TAG_PARSER_ABSTRACTCHAPTER_H

#include "./localeawarestring.h"

#include <c++utilities/chrono/timespan.h>

#include <memory>
#include <string>
#include <vector>

namespace TagParser {

class AbortableProgressFeedback;
class Diagnostics;
struct AbstractChapterPrivate;

class TAG_PARSER_EXPORT AbstractChapter {
public:
    virtual ~AbstractChapter();

    std::uint64_t id() const;
    const std::vector<LocaleAwareString> &names() const;
    CppUtilities::TimeSpan startTime() const;
    CppUtilities::TimeSpan endTime() const;
    const std::vector<std::uint64_t> &tracks() const;
    bool isHidden() const;
    bool isEnabled() const;
    std::string label() const;
    virtual AbstractChapter *nestedChapter(std::size_t index);
    virtual const AbstractChapter *nestedChapter(std::size_t index) const;
    virtual std::size_t nestedChapterCount() const;
    virtual void clear();
    void parse(Diagnostics &diag, AbortableProgressFeedback &progress);
    void parseNested(Diagnostics &diag, AbortableProgressFeedback &progress);

protected:
    AbstractChapter();
    virtual void internalParse(Diagnostics &diag, AbortableProgressFeedback &progress) = 0;

    std::uint64_t m_id;
    std::vector<LocaleAwareString> m_names;
    CppUtilities::TimeSpan m_startTime;
    CppUtilities::TimeSpan m_endTime;
    std::vector<std::uint64_t> m_tracks;
    std::unique_ptr<AbstractChapterPrivate> m_p;
    bool m_hidden;
    bool m_enabled;
};

/*!
 * \brief Returns the chapter ID if known; otherwise returns zero.
 */
inline std::uint64_t AbstractChapter::id() const
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
inline CppUtilities::TimeSpan AbstractChapter::startTime() const
{
    return m_startTime;
}

/*!
 * \brief Returns the end time if known; otherwise returns a negative time span.
 */
inline CppUtilities::TimeSpan AbstractChapter::endTime() const
{
    return m_endTime;
}

/*!
 * \brief Returns a list of tracks on which the chapter applies.
 */
inline const std::vector<std::uint64_t> &AbstractChapter::tracks() const
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
inline AbstractChapter *AbstractChapter::nestedChapter(std::size_t)
{
    return nullptr;
}

/*!
 * \brief Returns the nested chapter with the specified \a index.
 */
inline const AbstractChapter *AbstractChapter::nestedChapter(std::size_t) const
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

} // namespace TagParser

#endif // TAG_PARSER_ABSTRACTCHAPTER_H
