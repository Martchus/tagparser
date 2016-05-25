#ifndef TAG_H
#define TAG_H

#include "./statusprovider.h"
#include "./tagvalue.h"
#include "./tagtarget.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/io/binaryreader.h>

#include <fstream>
#include <type_traits>
#include <string>

namespace Media {

/*!
 * \brief Specifies the tag type.
 *
 * \sa Tag::type()
 */
enum class TagType : unsigned int
{
    Unspecified = 0x00, /**< The tag type is unspecified. */
    Id3v1Tag = 0x01, /**< The tag is a Media::Id3v1Tag. */
    Id3v2Tag = 0x02, /**< The tag is a Media::Id3v2Tag. */
    Mp4Tag = 0x04, /**< The tag is a Media::Mp4Tag. */
    MatroskaTag = 0x08, /**< The tag is a Media::MatroskaTag. */
    VorbisComment = 0x10, /**< The tag is a Media::VorbisComment. */
    OggVorbisComment = 0x20 /**< The tag is a Media::OggVorbisComment. */
};

/*!
 * \brief Specifies the field.
 *
 * These "known" fields are used to specify a field without using
 * the field identifier used by the underlaying tag type.
 *
 * Not all fields are supported by all tag types (see Tag::supportsField()).
 *
 * \sa Tag::type()
 */
enum class KnownField : unsigned int
{
    Invalid = static_cast<unsigned int>(-1), /**< invalid field name, do not map this value when subclassing Tag */
    Title = 0, /**< title */
    Album, /**< album/collection */
    Artist, /**< artist/band */
    Genre, /**< genre */
    Year, /**< year */
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
    Vendor /**< vendor */
};

/*!
 * \brief The first valid entry in the Media::KnownField enum.
 */
constexpr KnownField firstKnownField = KnownField::Title;

/*!
 * \brief The last valid entry in the Media::KnownField enum.
 */
constexpr KnownField lastKnownField = KnownField::Vendor;

/*!
 * \brief The number of valid entries in the Media::KnownField enum.
 */
constexpr unsigned int knownFieldArraySize = static_cast<unsigned int>(lastKnownField) + 1;

/*!
 * \brief Returns the next known field. Returns KnownField::Invalid if there is not next field.
 */
constexpr KnownField nextKnownField(KnownField field)
{
    return field == lastKnownField ? KnownField::Invalid : static_cast<KnownField>(static_cast<int>(field) + 1);
}

class LIB_EXPORT Tag : public StatusProvider
{
public:
    virtual ~Tag();

    virtual TagType type() const;
    virtual const char *typeName() const;
    std::string toString() const;
    virtual TagTextEncoding proposedTextEncoding() const;
    virtual bool canEncodingBeUsed(TagTextEncoding encoding) const;
    virtual const TagValue &value(KnownField field) const = 0;
    virtual bool setValue(KnownField field, const TagValue &value) = 0;
    virtual bool hasField(KnownField field) const = 0;
    virtual void removeAllFields() = 0;
    const std::string &version() const;
    uint32 size() const;
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
    virtual unsigned int insertValues(const Tag &from, bool overwrite);
//    Tag *parent() const;
//    bool setParent(Tag *tag);
//    Tag *nestedTag(size_t index) const;
//    size_t nestedTagCount() const;
//    const std::vector<Tag *> nestedTags() const;
//    virtual bool supportsNestedTags() const;
//    virtual bool supportsChild(Tag *child);

protected:
    Tag();

    std::string m_version;
    uint32 m_size;
    TagTarget m_target;
//    Tag *m_parent;
//    std::vector<Tag *> m_nestedTags;
};

/*!
 * \brief Returns the type of the tag as Media::TagType.
 *
 * This is TagType::Unspecified by default and might be overwritten
 * when subclassing.
 */
inline TagType Tag::type() const
{
    return TagType::Unspecified;
}

/*!
 * \brief Returns the type name of the tag as C-style string.
 *
 * This is "unspecified" by default and might be overwritten
 * when subclassing.
 */
inline const char *Tag::typeName() const
{
    return "unspecified";
}

/*!
 * \brief Returns the proposed text encoding.
 *
 * This is TagTextEncoding::Latin1 by default an might be
 * overwritten when subclassing.
 *
 * The tag class and its subclasses do not perform any conversions.
 * You have to provide all string values using an encoding which is
 * appropriate for the specific tag type. This method returns such
 * an encoding.
 *
 * \sa canEncodingBeUsed()
 */
inline TagTextEncoding Tag::proposedTextEncoding() const
{
    return TagTextEncoding::Latin1;
}

/*!
 * \brief Returns an indication whether the specified \a encoding
 *        can be used to provide string values for the tag.
 *
 * Only the proposedTextEncoding() is accepted by default. This might
 * be overwritten when subclassing.
 *
 * The tag class and its subclasses do not perform any conversions.
 * You have to provide all string values using an encoding which is
 * appropriate for the specific tag type. This method is meant to
 * determine if a particular \a encoding can be used.
 *
 * \sa canEncodingBeUsed()
 */
inline bool Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == proposedTextEncoding();
}

/*!
 * \brief Returns the version of the tag as std::string.
 * The version denotation depends on the tag type.
 */
inline const std::string &Tag::version() const
{
    return m_version;
}

/*!
 * \brief Returns the size of the tag in bytes.
 * The tag needs to be parsed before.
 */
inline uint32 Tag::size() const
{
    return m_size;
}

/*!
 * \brief Returns an indication whether a target is supported by the tag.
 *
 * If no target is supported, setting a target using setTarget()
 * has no effect when saving the tag.
 *
 * Most tag types don't support this feature so the default implementation
 * returns always false. This might be overwritten when subclassing.
 */
inline bool Tag::supportsTarget() const
{
    return false;
}

/*!
 * \brief Returns the target of tag.
 *
 * \sa supportsTarget()
 * \sa setTarget()
 */
inline const TagTarget &Tag::target() const
{
    return m_target;
}

/*!
 * \brief Sets the target of tag.
 *
 * Most tag types don't support this feature so setting
 * the target has no effect when saving the file.
 *
 * \sa supportsTarget()
 * \sa target()
 */
inline void Tag::setTarget(const TagTarget &target)
{
    m_target = target;
}

/*!
 * \brief Returns the name of the current tag target level.
 * \remarks Returns TagTargetLevel::Unspecified if target levels are not supported by the tag.
 */
inline TagTargetLevel Tag::targetLevel() const
{
    return TagTargetLevel::Unspecified;
}

/*!
 * \brief Returns the name of the current target level.
 * \remarks Returns nullptr if target levels are not supported by the tag.
 */
inline const char *Tag::targetLevelName() const
{
    return supportsTarget() ? tagTargetLevelName(targetLevel()) : nullptr;
}

/*!
 * \brief Returns whether the tag is targeting the specified \a tagTargetLevel.
 * \remarks If targets are not supported by the tag it is considered targeting
 *          everything and hence this method returns always true in this case.
 */
inline bool Tag::isTargetingLevel(TagTargetLevel tagTargetLevel) const
{
    return !supportsTarget() || static_cast<byte>(targetLevel()) >= static_cast<byte>(tagTargetLevel);
}

/*!
 * \brief Returns the string representation for the assigned tag target.
 */
inline std::string Tag::targetString() const
{
    return target().toString(targetLevel());
}

/*!
 * \brief Returns the proposed data type for the specified \a field as TagDataType.
 *
 * Most values need to be provided as string (see proposedTextEncoding() and
 * canEncodingBeUsed()). Other values need to be provided as integer or an other
 * TagDataType. This method helps to determine which type is required for a particular
 * \a field. The tag class and its subclasses try to convert the provided values.
 * Nevertheless it is recommend to use the proposed data types to avoid conversion
 * failures.
 *
 * The default implementation returns a data type which is most commonly used for
 * the specified \a field. The default implementation might be overwritten when
 * subclassing.
 */
inline TagDataType Tag::proposedDataType(KnownField field) const
{
    switch(field) {
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
    default:
        return TagDataType::Text;
    }
}

/*!
 * \brief Returns an indications whether the specified field supports descriptions.
 *
 * If you assign a description to a field value and the field does not support
 * descriptions the description is ignored when saving the tag.
 *
 * The default implementation returns false for all fields. This might be overwritten
 * when subclassing.
 */
inline bool Tag::supportsDescription(KnownField ) const
{
    return false;
}

/*!
 * \brief Returns an indications whether the specified field supports mime types.
 *
 * If you assign a mime types to a field value and the field does not support
 * mime types the mime type is ignored when saving the tag.
 *
 * The default implementation returns false for all fields. This might be overwritten
 * when subclassing.
 */
inline bool Tag::supportsMimeType(KnownField ) const
{
    return false;
}

///*!
// * \brief Returns the parent of the tag.
// */
//inline Tag *Tag::parent() const
//{
//    return m_parent;
//}

///*!
// * \brief Returns the nested tags.
// */
//inline const std::vector<Tag *> Tag::nestedTags() const
//{
//    return m_nestedTags;
//}

///*!
// * \brief Returns whether the tag supports nested tags (generally).
// */
//inline bool Tag::supportsNestedTags() const
//{
//    return false;
//}

///*!
// * \brief Returns whether \a child might be added as nested tag.
// *
// * The default implementation returns true when nested tag are
// * generally supported and the \a child is of the same type
// * as this tag.
// */
//inline bool Tag::supportsChild(Tag *child)
//{
//    return supportsNestedTags() && type() == child->type();
//}

///*!
// * \brief Returns the nested tag with the specified \a index.
// */
//inline Tag *Tag::nestedTag(size_t ) const
//{
//    return nullptr;
//}

///*!
// * \brief Returns the number of nested tags.
// */
//inline size_t Tag::nestedTagCount() const
//{
//    return 0;
//}

}

#endif // TAG_H
