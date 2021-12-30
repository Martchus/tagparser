#ifndef TAG_PARSER_TAGTARGET_H
#define TAG_PARSER_TAGTARGET_H

#include "./global.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace TagParser {

/*!
 * \brief The TagTargetLevel enum specifies tag target levels.
 */
enum class TagTargetLevel : unsigned char { Unspecified, Shot, Subtrack, Track, Part, Album, Edition, Collection };

TAG_PARSER_EXPORT std::string_view tagTargetLevelName(TagTargetLevel tagTargetLevel);

class TAG_PARSER_EXPORT TagTarget {
public:
    using IdType = std::uint64_t;
    using IdContainerType = std::vector<IdType>;

    explicit TagTarget(std::uint64_t level = 0, IdContainerType tracks = IdContainerType(), IdContainerType chapters = IdContainerType(),
        IdContainerType editions = IdContainerType(), IdContainerType attachments = IdContainerType());

    std::uint64_t level() const;
    void setLevel(std::uint64_t level);
    const std::string &levelName() const;
    void setLevelName(const std::string &levelName);
    const IdContainerType &tracks() const;
    IdContainerType &tracks();
    const IdContainerType &chapters() const;
    IdContainerType &chapters();
    const IdContainerType &editions() const;
    IdContainerType &editions();
    const IdContainerType &attachments() const;
    IdContainerType &attachments();
    bool isEmpty() const;
    void clear();
    std::string toString(const std::function<TagTargetLevel(std::uint64_t)> &tagTargetMapping) const;
    std::string toString(TagTargetLevel tagTargetLevel) const;
    bool operator==(const TagTarget &other) const;
    bool matches(const TagTarget &other) const;

private:
    std::uint64_t m_level;
    std::string m_levelName;
    IdContainerType m_tracks;
    IdContainerType m_chapters;
    IdContainerType m_editions;
    IdContainerType m_attachments;
};

/*!
 * \brief Constructs a new TagTarget with the specified \a level, \a track, \a chapter,
 *        \a edition and \a attachment.
 */
inline TagTarget::TagTarget(
    std::uint64_t level, IdContainerType tracks, IdContainerType chapters, IdContainerType editions, IdContainerType attachments)
    : m_level(level)
    , m_tracks(tracks)
    , m_chapters(chapters)
    , m_editions(editions)
    , m_attachments(attachments)
{
}

/*!
 * \brief Returns the level.
 */
inline std::uint64_t TagTarget::level() const
{
    return m_level ? m_level : 50;
}

/*!
 * \brief Sets the level.
 */
inline void TagTarget::setLevel(std::uint64_t level)
{
    m_level = level;
}

/*!
 * \brief Returns the level name.
 */
inline const std::string &TagTarget::levelName() const
{
    return m_levelName;
}

/*!
 * \brief Sets the level name.
 */
inline void TagTarget::setLevelName(const std::string &levelName)
{
    m_levelName = levelName;
}

/*!
 * \brief Returns the tracks.
 */
inline const TagTarget::IdContainerType &TagTarget::tracks() const
{
    return m_tracks;
}

/*!
 * \brief Returns the tracks.
 */
inline TagTarget::IdContainerType &TagTarget::tracks()
{
    return m_tracks;
}

/*!
 * \brief Returns the chapters.
 */
inline const TagTarget::IdContainerType &TagTarget::chapters() const
{
    return m_chapters;
}

/*!
 * \brief Returns the chapters.
 */
inline TagTarget::IdContainerType &TagTarget::chapters()
{
    return m_chapters;
}

/*!
 * \brief Returns the editions.
 */
inline const TagTarget::IdContainerType &TagTarget::editions() const
{
    return m_editions;
}

/*!
 * \brief Returns the editions.
 */
inline TagTarget::IdContainerType &TagTarget::editions()
{
    return m_editions;
}

/*!
 * \brief Returns the attachments.
 */
inline const TagTarget::IdContainerType &TagTarget::attachments() const
{
    return m_attachments;
}

/*!
 * \brief Returns the attachments.
 */
inline TagTarget::IdContainerType &TagTarget::attachments()
{
    return m_attachments;
}

/*!
 * \brief Returns an indication whether the target is empty.
 */
inline bool TagTarget::isEmpty() const
{
    return m_level == 0 && m_levelName.empty() && m_tracks.empty() && m_chapters.empty() && m_editions.empty() && m_attachments.empty();
}

/*!
 * \brief Clears the TagTarget.
 */
inline void TagTarget::clear()
{
    m_level = 0;
    m_levelName.clear();
    m_tracks.clear();
    m_chapters.clear();
    m_editions.clear();
    m_attachments.clear();
}

/*!
 * \brief Returns whether the tag targets are equal.
 * \returns
 * Returns whether all specifications of the current instance (besides the level name) are equal
 * to the corresponding specification in \a other.
 */
inline bool TagTarget::operator==(const TagTarget &other) const
{
    return level() == other.level() && m_tracks == other.m_tracks && m_chapters == other.m_chapters && m_editions == other.m_editions
        && m_attachments == other.m_attachments;
}

/*!
 * \brief Returns whether the current instance matches \a other.
 * \returns
 * Returns whether all non-empty/non-null specifications of the current instance (besides the level
 * name) are equal to the corresponding specification in \a other.
 */
inline bool TagTarget::matches(const TagTarget &other) const
{
    return (!m_level || level() == other.level()) && (m_tracks.empty() || m_tracks == other.m_tracks)
        && (m_chapters.empty() || m_chapters == other.m_chapters) && (m_editions.empty() || m_editions == other.m_editions)
        && (m_attachments.empty() || m_attachments == other.m_attachments);
}

/*!
 * \brief Returns the string representation of the current instance.
 * \remarks Uses the specified \a tagTargetMapping function to map the assigned level()
 *          to a TagTargetLevel if no levelName() is assigned.
 */
inline std::string TagTarget::toString(const std::function<TagTargetLevel(std::uint64_t)> &tagTargetMapping) const
{
    return toString(tagTargetMapping ? tagTargetMapping(this->level()) : TagTargetLevel::Unspecified);
}

} // namespace TagParser

#endif // TAG_PARSER_TAGTARGET_H
