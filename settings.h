#ifndef TAGPARSER_SETTINGS_H
#define TAGPARSER_SETTINGS_H

#include "./tagtarget.h"

#include <c++utilities/misc/flagenumclass.h>

#include <cstdint>
#include <type_traits>

namespace TagParser {

enum class ElementPosition {
    BeforeData, /**< the element is positioned before the actual data */
    AfterData, /**< the element is positioned after the actual data */
    Keep, /**< the element is placed where it was before */
};

/*!
 * \brief The TagUsage enum specifies the usage of a certain tag type.
 */
enum class TagUsage {
    Always, /**< a tag of the type is always used; a new tag is created if none exists yet */
    KeepExisting, /**< existing tags of the type are kept and updated but no new tag is created  */
    Never, /**< tags of the type are never used; a possibly existing tag of the type is removed */
};

/*!
 * \brief The Flags enum contains options to control the tag creation via MediaFileInfo::createAppropriateTags().
 */
enum class TagCreationFlags : std::uint64_t {
    None = 0, /**< no flags present */
    TreatUnknownFilesAsMp3Files = 1 << 0, /**< treat unknown file formats as MP3 (might make those files unusable) */
    Id3InitOnCreate = 1 << 1, /**< initialize newly created ID3 tags with the values of the already present ID3 tags */
    Id3TransferValuesOnRemoval = 1 << 2, /**< transfer values of removed ID3 tags to remaining ID3 tags (no values will be overwritten) */
    MergeMultipleSuccessiveId3v2Tags = 1 << 3, /**< merge multiple successive ID3v2 tags (see MediaFileInfo::mergeId3v2Tags()) */
    KeepExistingId3v2Version
    = 1 << 4, /**< keep version of existing ID3v2 tags so TagSettings::id3v2version is only used when creating a *new* ID3v2 tag */
};

} // namespace TagParser

CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(TagParser, TagParser::TagCreationFlags);

namespace TagParser {

/*!
 * \brief The TagSettings struct contains settings which can be passed to MediaFileInfo::createAppropriateTags().
 */
struct TagCreationSettings {
    /// \brief Specifies the required targets. If targets are not supported by the container an informal notification is added.
    std::vector<TagTarget> requiredTargets = std::vector<TagTarget>();
    /// \brief Specifies options to control the tag creation. See TagSettings::Flags.
    TagCreationFlags flags = TagCreationFlags::Id3TransferValuesOnRemoval | TagCreationFlags::MergeMultipleSuccessiveId3v2Tags
        | TagCreationFlags::KeepExistingId3v2Version;
    /// \brief Specifies the usage of ID3v1 when creating tags for MP3 files (has no effect when the file is no MP3 file or not treated as one).
    TagUsage id3v1usage = TagUsage::KeepExisting;
    /// \brief Specifies the usage of ID3v2 when creating tags for MP3 files (has no effect when the file is no MP3 file or not treated as one).
    TagUsage id3v2usage = TagUsage::Always;
    /// \brief Specifies the ID3v2 version to be used in case an ID3v2 tag present or will be created. Valid values are 2, 3 and 4.
    std::uint8_t id3v2MajorVersion = 3;

    // workaround for GGC bug 66297 (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66297)
#if __GNUC__ < 7 || (__GNUC__ == 7 && __GNUC_MINOR__ < 2)
    inline
#else
    constexpr
#endif
        TagCreationSettings &
        setFlag(TagCreationFlags flag, bool enabled);
};

#if __GNUC__ < 7 || (__GNUC__ == 7 && __GNUC_MINOR__ < 2)
inline
#else
constexpr
#endif
    TagCreationSettings &
    TagCreationSettings::setFlag(TagCreationFlags flag, bool enabled)
{
    if (enabled) {
        flags += flag;
    } else {
        flags -= flag;
    }
    return *this;
}

} // namespace TagParser

#endif // TAGPARSER_SETTINGS_H
