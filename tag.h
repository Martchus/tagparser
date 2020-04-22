#ifndef TAG_PARSER_TAG_H
#define TAG_PARSER_TAG_H

#include "./tagtarget.h"
#include "./tagvalue.h"

#include <c++utilities/io/binaryreader.h>

#include <cstdint>
#include <string>
#include <type_traits>

namespace TagParser {

/*!
 * \brief Specifies the tag type.
 *
 * \sa Tag::type()
 */
enum class TagType : unsigned int {
    Unspecified = 0x00, /**< The tag type is unspecified. */
    Id3v1Tag = 0x01, /**< The tag is a TagParser::Id3v1Tag. */
    Id3v2Tag = 0x02, /**< The tag is a TagParser::Id3v2Tag. */
    Mp4Tag = 0x04, /**< The tag is a TagParser::Mp4Tag. */
    MatroskaTag = 0x08, /**< The tag is a TagParser::MatroskaTag. */
    VorbisComment = 0x10, /**< The tag is a TagParser::VorbisComment. */
    OggVorbisComment = 0x20 /**< The tag is a TagParser::OggVorbisComment. */
};

/*!
 * \brief Specifies the field.
 *
 * These "known" fields are used to specify a field without using
 * the field identifier used by the underlaying tag type.
 *
 * Not all fields are supported by all tag types (see Tag::supportsField()).
 *
 * Mapping proposed by HAK: https://wiki.hydrogenaud.io/index.php?title=Tag_Mapping
 *
 * \sa Tag::type()
 */
enum class KnownField : unsigned int {
    Invalid = std::numeric_limits<unsigned int>::max(), /**< invalid field name, do not map this value when subclassing Tag */
    Title = 0, /**< title */
    Album, /**< album/collection */
    Artist, /**< artist/band */
    Genre, /**< genre */
    Year, /**< record date, deprecated - FIXME v10: remove in favor of RecordDate and ReleaseDate */
    Comment, /**< comment */
    Bpm, /**< beats per minute */
    Bps, /**< beats per second */
    Lyricist, /**< lyricist */
    TrackPosition, /**< track/part number and total track/part count */
    DiskPosition, /**< disk number and total disk count */
    PartNumber, /**< track/part number */
    TotalParts, /**< total track/part count */
    Encoder, /**< encoder */
    RecordDate, /**< record date */
    Performers, /**< performers */
    Length, /**< length */
    Language, /**< language */
    EncoderSettings, /**< encoder settings */
    Lyrics, /**< lyrics */
    SynchronizedLyrics, /**< synchronized lyrics */
    Grouping, /**< grouping */
    RecordLabel, /**< record label */
    Cover, /**< cover */
    Composer, /**< composer */
    Rating, /**< rating */
    Description, /**< description */
    Vendor, /**< vendor */
    AlbumArtist, /**< album artist */
    ReleaseDate, /**< release date */
};

/*!
 * \brief The first valid entry in the TagParser::KnownField enum.
 */
constexpr KnownField firstKnownField = KnownField::Title;

/*!
 * \brief The last valid entry in the TagParser::KnownField enum.
 */
constexpr KnownField lastKnownField = KnownField::ReleaseDate;

/*!
 * \brief The number of valid entries in the TagParser::KnownField enum.
 */
constexpr unsigned int knownFieldArraySize = static_cast<unsigned int>(lastKnownField) + 1;

/*!
 * \brief Returns whether the specified \a field is deprecated and should not be used anymore.
 */
constexpr bool isKnownFieldDeprecated(KnownField field)
{
    return field == KnownField::Year;
}

/*!
 * \brief Returns the next known field skipping any deprecated fields. Returns KnownField::Invalid if there is not next field.
 */
constexpr KnownField nextKnownField(KnownField field)
{
    const auto next = field == lastKnownField ? KnownField::Invalid : static_cast<KnownField>(static_cast<int>(field) + 1);
    return isKnownFieldDeprecated(next) ? nextKnownField(next) : next;
}

class TAG_PARSER_EXPORT Tag {
public:
    virtual ~Tag();

    virtual TagType type() const;
    virtual const char *typeName() const;
    std::string toString() const;
    virtual TagTextEncoding proposedTextEncoding() const;
    virtual bool canEncodingBeUsed(TagTextEncoding encoding) const;
    virtual const TagValue &value(KnownField field) const = 0;
    virtual std::vector<const TagValue *> values(KnownField field) const;
    virtual bool setValue(KnownField field, const TagValue &value) = 0;
    virtual bool setValues(KnownField field, const std::vector<TagValue> &values);
    virtual bool hasField(KnownField field) const = 0;
    virtual void removeAllFields() = 0;
    const std::string &version() const;
    std::uint32_t size() const;
    virtual bool supportsTarget() const;
    const TagTarget &target() const;
    void setTarget(const TagTarget &target);
    virtual TagTargetLevel targetLevel() const;
    const char *targetLevelName() const;
    bool isTargetingLevel(TagTargetLevel tagTargetLevel) const;
    std::string targetString() const;
    virtual unsigned int fieldCount() const = 0;
    virtual bool supportsField(KnownField field) const = 0;
    virtual TagDataType proposedDataType(KnownField field) const;
    virtual bool supportsDescription(KnownField field) const;
    virtual bool supportsMimeType(KnownField field) const;
    virtual bool supportsMultipleValues(KnownField field) const;
    virtual unsigned int insertValues(const Tag &from, bool overwrite);
    virtual void ensureTextValuesAreProperlyEncoded() = 0;

protected:
    Tag();

    std::string m_version;
    std::uint32_t m_size;
    TagTarget m_target;
};

inline TagType Tag::type() const
{
    return TagType::Unspecified;
}

inline const char *Tag::typeName() const
{
    return "unspecified";
}

inline TagTextEncoding Tag::proposedTextEncoding() const
{
    return TagTextEncoding::Latin1;
}

inline bool Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == proposedTextEncoding();
}

inline const std::string &Tag::version() const
{
    return m_version;
}

inline std::uint32_t Tag::size() const
{
    return m_size;
}

inline bool Tag::supportsTarget() const
{
    return false;
}

inline const TagTarget &Tag::target() const
{
    return m_target;
}

inline void Tag::setTarget(const TagTarget &target)
{
    m_target = target;
}

inline TagTargetLevel Tag::targetLevel() const
{
    return TagTargetLevel::Unspecified;
}

inline const char *Tag::targetLevelName() const
{
    return supportsTarget() ? tagTargetLevelName(targetLevel()) : nullptr;
}

inline bool Tag::isTargetingLevel(TagTargetLevel tagTargetLevel) const
{
    return !supportsTarget() || static_cast<std::uint8_t>(targetLevel()) >= static_cast<std::uint8_t>(tagTargetLevel);
}

inline std::string Tag::targetString() const
{
    return target().toString(targetLevel());
}

inline TagDataType Tag::proposedDataType(KnownField field) const
{
    switch (field) {
    case KnownField::Bpm:
    case KnownField::Bps:
    case KnownField::Rating:
    case KnownField::PartNumber:
    case KnownField::TotalParts:
        return TagDataType::Integer;
    case KnownField::Cover:
        return TagDataType::Picture;
    case KnownField::Length:
        return TagDataType::TimeSpan;
    case KnownField::TrackPosition:
    case KnownField::DiskPosition:
        return TagDataType::PositionInSet;
    case KnownField::Genre:
        return TagDataType::StandardGenreIndex;
    case KnownField::SynchronizedLyrics:
        return TagDataType::Undefined; // not supported so far
    default:
        return TagDataType::Text;
    }
}

inline bool Tag::supportsDescription(KnownField) const
{
    return false;
}

inline bool Tag::supportsMimeType(KnownField) const
{
    return false;
}

inline bool Tag::supportsMultipleValues(KnownField) const
{
    return false;
}

} // namespace TagParser

#endif // TAG_PARSER_TAG_H
