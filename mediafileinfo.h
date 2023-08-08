#ifndef TAG_PARSER_MEDIAINFO_H
#define TAG_PARSER_MEDIAINFO_H

#include "./abstractcontainer.h"
#include "./basicfileinfo.h"
#include "./settings.h"
#include "./signature.h"

#include <cstdint>
#include <memory>
#include <unordered_set>
#include <vector>

#include <c++utilities/misc/flagenumclass.h>

namespace TagParser {

class Tag;
class Id3v1Tag;
class Id3v2Tag;
class Mp4Container;
class Mp4Atom;
class Mp4Tag;
class MatroskaContainer;
class OggContainer;
class EbmlElement;
class MatroskaTag;
class AbstractTrack;
class VorbisComment;
class Diagnostics;
class AbortableProgressFeedback;

enum class MediaType : unsigned int;
enum class TagType : unsigned int;

/*!
 * \brief The ParsingStatus enum specifies whether a certain part of the file (tracks, tags, ...) has
 *        been parsed yet and if what the parsing result is.
 */
enum class ParsingStatus : std::uint8_t {
    NotParsedYet, /**< the part has not been parsed yet */
    Ok, /**< the part has been parsed and no critical errors occurred */
    NotSupported, /**< tried to parse the part, but the format is not supported */
    CriticalFailure /**< tried to parse the part, but critical errors occurred */
};

/*!
 * \brief The MediaFileStructureFlags enum specifies flags which describing the structure of a media file.
 */
enum class MediaFileStructureFlags : std::uint64_t {
    None, /**< all flags disabled */
    ActualExistingId3v1Tag = (1 << 0), /**< whether an ID3v1 tag was found when parsing the file */
};

/*!
 * \brief The MediaFileHandlingFlags enum specifies flags which controls the behavior of MediaFileInfo objects.
 */
enum class MediaFileHandlingFlags : std::uint64_t {
    None, /**< all flags disabled */
    ForceFullParse
    = (1 << 0), /**< causes the parser to analyse the file structure as deep as possible; might cause long loading times for big files */
    ForceRewrite = (1 << 1), /**< enforces a re-write of the file when applying changes */
    ForceTagPosition = (1 << 2), /**< enforces the tag position when applying changes, see remarks of MediaFileInfo::setTagPosition() */
    ForceIndexPosition = (1 << 3), /**< enforces the index position when applying changes, see remarks of MediaFileInfo::setIndexPosition() */
    NormalizeKnownTagFieldIds = (1 << 4), /**< normalizes known tag field IDs when parsing to match the tag specification's recommendations */
    PreserveRawTimingValues = (1 << 8), /**< preverves raw timing values (so far only used when making MP4 tracks) */
    PreserveMuxingApplication = (1 << 9), /**< preverves the muxing application (so far only used when making Matroska container) */
    PreserveWritingApplication = (1 << 10), /**< preverves the writing application (so far only used when making Matroska container) */
};

} // namespace TagParser

CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(TagParser, TagParser::MediaFileStructureFlags)
CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(TagParser, TagParser::MediaFileHandlingFlags)

namespace TagParser {

struct MediaFileInfoPrivate;

class TAG_PARSER_EXPORT MediaFileInfo : public BasicFileInfo {
public:
    // constructor, destructor
    explicit MediaFileInfo();
    explicit MediaFileInfo(std::string_view path);
    explicit MediaFileInfo(std::string &&path);
    MediaFileInfo(const MediaFileInfo &) = delete;
    MediaFileInfo &operator=(const MediaFileInfo &) = delete;
    ~MediaFileInfo() override;

    // methods to parse file
    void parseContainerFormat(Diagnostics &diag, AbortableProgressFeedback &progress);
    void parseTracks(Diagnostics &diag, AbortableProgressFeedback &progress);
    void parseTags(Diagnostics &diag, AbortableProgressFeedback &progress);
    void parseChapters(Diagnostics &diag, AbortableProgressFeedback &progress);
    void parseAttachments(Diagnostics &diag, AbortableProgressFeedback &progress);
    void parseEverything(Diagnostics &diag, AbortableProgressFeedback &progress);

    // methods to apply changes
    void applyChanges(Diagnostics &diag, AbortableProgressFeedback &progress);

    // methods to get parsed information regarding ...
    // ... the container
    ContainerFormat containerFormat() const;
    std::string_view containerFormatName() const;
    std::string_view containerFormatAbbreviation() const;
    std::string_view containerFormatSubversion() const;
    std::string_view mimeType() const;
    std::uint64_t containerOffset() const;
    std::uint64_t paddingSize() const;
    std::uint64_t effectiveSize() const;
    AbstractContainer *container() const;
    ParsingStatus containerParsingStatus() const;
    // ... the chapters
    ParsingStatus chaptersParsingStatus() const;
    std::vector<AbstractChapter *> chapters() const;
    bool areChaptersSupported() const;
    // ... the attachments
    ParsingStatus attachmentsParsingStatus() const;
    std::vector<AbstractAttachment *> attachments() const;
    bool areAttachmentsSupported() const;
    // ... the tracks
    ParsingStatus tracksParsingStatus() const;
    std::size_t trackCount() const;
    std::vector<AbstractTrack *> tracks() const;
    bool hasTracksOfType(TagParser::MediaType type) const;
    CppUtilities::TimeSpan duration() const;
    double overallAverageBitrate() const;
    std::unordered_set<std::string> availableLanguages(TagParser::MediaType type = TagParser::MediaType::Audio) const;
    std::string technicalSummary() const;
    bool areTracksSupported() const;
    // ... the tags
    ParsingStatus tagsParsingStatus() const;
    bool hasId3v1Tag() const;
    bool hasId3v2Tag() const;
    bool hasAnyTag() const;
    Id3v1Tag *id3v1Tag() const;
    const std::vector<std::unique_ptr<Id3v2Tag>> &id3v2Tags() const;
    void tags(std::vector<Tag *> &tags) const;
    std::vector<Tag *> tags() const;
    void parsedTags(std::vector<Tag *> &tags) const;
    std::vector<Tag *> parsedTags() const;
    Mp4Tag *mp4Tag() const;
    const std::vector<std::unique_ptr<MatroskaTag>> &matroskaTags() const;
    VorbisComment *vorbisComment() const;
    bool areTagsSupported() const;

    // methods to create/remove tags
    bool createAppropriateTags(const TagCreationSettings &settings = TagCreationSettings());
    bool removeId3v1Tag();
    Id3v1Tag *createId3v1Tag();
    bool removeId3v2Tag(Id3v2Tag *tag);
    bool removeAllId3v2Tags();
    Id3v2Tag *createId3v2Tag();
    bool removeTag(Tag *tag);
    void removeAllTags();
    void mergeId3v2Tags();
    bool id3v1ToId3v2();
    bool id3v2ToId3v1();
    VorbisComment *createVorbisComment();
    bool removeVorbisComment();
    void clearParsingResults();

    // methods to get, set object behaviour
    const std::string &backupDirectory() const;
    void setBackupDirectory(std::string_view backupDirectory);
    void setBackupDirectory(std::string &&backupDirectory);
    const std::string &saveFilePath() const;
    void setSaveFilePath(std::string_view saveFilePath);
    void setSaveFilePath(std::string &&saveFilePath);
    const std::string &writingApplication() const;
    void setWritingApplication(std::string_view writingApplication);
    MediaFileHandlingFlags fileHandlingFlags();
    void setFileHandlingFlags(MediaFileHandlingFlags flags);
    bool isForcingFullParse() const;
    void setForceFullParse(bool forceFullParse);
    bool isForcingRewrite() const;
    void setForceRewrite(bool forceRewrite);
    std::size_t minPadding() const;
    void setMinPadding(std::size_t minPadding);
    std::size_t maxPadding() const;
    void setMaxPadding(std::size_t maxPadding);
    std::size_t preferredPadding() const;
    void setPreferredPadding(std::size_t preferredPadding);
    ElementPosition tagPosition() const;
    void setTagPosition(ElementPosition tagPosition);
    bool forceTagPosition() const;
    void setForceTagPosition(bool forceTagPosition);
    ElementPosition indexPosition() const;
    void setIndexPosition(ElementPosition indexPosition);
    bool forceIndexPosition() const;
    void setForceIndexPosition(bool forceTagPosition);
    std::uint64_t maxFullParseSize() const;
    void setMaxFullParseSize(std::uint64_t maxFullParseSize);

protected:
    void invalidated() override;

private:
    // private methods internally used when rewriting the file to apply new tag information
    // currently only the makeMp3File() methods is present; corresponding methods for
    // other formats are outsourced to container classes
    void makeMp3File(Diagnostics &diag, AbortableProgressFeedback &progress);

    // fields related to the container
    ParsingStatus m_containerParsingStatus;
    ContainerFormat m_containerFormat;
    std::streamoff m_containerOffset;
    std::uint64_t m_paddingSize;
    std::uint64_t m_effectiveSize;
    std::vector<std::streamoff> m_actualId3v2TagOffsets;
    std::unique_ptr<AbstractContainer> m_container;
    MediaFileStructureFlags m_fileStructureFlags;

    // fields related to the tracks
    ParsingStatus m_tracksParsingStatus;
    std::unique_ptr<AbstractTrack> m_singleTrack;

    // fields related to the tag
    ParsingStatus m_tagsParsingStatus;
    std::unique_ptr<Id3v1Tag> m_id3v1Tag;
    std::vector<std::unique_ptr<Id3v2Tag>> m_id3v2Tags;

    // fields related to the chapters and the attachments
    ParsingStatus m_chaptersParsingStatus;
    ParsingStatus m_attachmentsParsingStatus;

    // fields specifying object behaviour
    std::string m_backupDirectory;
    std::string m_saveFilePath;
    std::string m_writingApplication;
    std::size_t m_minPadding;
    std::size_t m_maxPadding;
    std::size_t m_preferredPadding;
    ElementPosition m_tagPosition;
    ElementPosition m_indexPosition;
    MediaFileHandlingFlags m_fileHandlingFlags;
    std::uint64_t m_maxFullParseSize;
    std::unique_ptr<MediaFileInfoPrivate> m_p;
};

/*!
 * \brief Returns an indication whether the container format has been parsed yet.
 */
inline ParsingStatus MediaFileInfo::containerParsingStatus() const
{
    return m_containerParsingStatus;
}

/*!
 * \brief Returns the container format of the current file.
 *
 * parseContainerFormat() needs to be called before. Otherwise
 * always ContainerFormat::Unknown will be returned.
 */
inline ContainerFormat MediaFileInfo::containerFormat() const
{
    return m_containerFormat;
}

/*!
 * \brief Returns the name of the container format as C-style string.
 *
 * parseContainerFormat() needs to be called before. Otherwise
 * always the name "Unknown" will be returned.
 *
 * \sa containerFormat()
 * \sa containerFormatAbbreviation()
 * \sa parseContainerFormat()
 */
inline std::string_view MediaFileInfo::containerFormatName() const
{
    return TagParser::containerFormatName(m_containerFormat);
}

/*!
 * \brief Returns the subversion of the container format as C-style string.
 *
 * parseContainerFormat() needs to be called before. Otherwise
 * always an empty string will be returned.
 *
 * \sa containerFormat()
 * \sa containerFormatName()
 * \sa parseContainerFormat()
 */
inline std::string_view MediaFileInfo::containerFormatSubversion() const
{
    return TagParser::containerFormatSubversion(m_containerFormat);
}

/*!
 * \brief Returns the actual container start offset.
 */
inline std::uint64_t MediaFileInfo::containerOffset() const
{
    return static_cast<std::uint64_t>(m_containerOffset);
}

/*!
 * \brief Returns the padding size. Container format and tags should have been parsed yet.
 */
inline std::uint64_t MediaFileInfo::paddingSize() const
{
    return m_paddingSize;
}

/*!
 * \brief Returns the "effective size" of the file if know; otherwise returns 0.
 * \remarks This is the size of the file minus tags at the beginning and the end.
 */
inline std::uint64_t MediaFileInfo::effectiveSize() const
{
    return m_effectiveSize;
}

/*!
 * \brief Returns an indication whether tag information has been parsed yet.
 */
inline ParsingStatus MediaFileInfo::tagsParsingStatus() const
{
    return m_tagsParsingStatus;
}

/*!
 * \brief Returns an indication whether tracks have been parsed yet.
 */
inline ParsingStatus MediaFileInfo::tracksParsingStatus() const
{
    return m_tracksParsingStatus;
}

/*!
 * \brief Returns the number of tracks that could be parsed.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns zero.
 *
 * \sa parseTracks()
 */
inline std::size_t MediaFileInfo::trackCount() const
{
    return m_singleTrack ? 1 : (m_container ? m_container->trackCount() : 0);
}

/*!
 * \brief Returns whether the chapters have been parsed yet.
 */
inline ParsingStatus MediaFileInfo::chaptersParsingStatus() const
{
    return m_chaptersParsingStatus;
}

/*!
 * \brief Returns whether the attachments have been parsed yet.
 */
inline ParsingStatus MediaFileInfo::attachmentsParsingStatus() const
{
    return m_attachmentsParsingStatus;
}

/*!
 * \brief Returns an indication whether an ID3v1 tag is assigned.
 */
inline bool MediaFileInfo::hasId3v1Tag() const
{
    return m_id3v1Tag != nullptr;
}

/*!
 * \brief Returns an indication whether an ID3v2 tag is assigned.
 */
inline bool MediaFileInfo::hasId3v2Tag() const
{
    return !m_id3v2Tags.empty();
}

/*!
 * \brief Returns a pointer to the assigned ID3v1 tag or nullptr if none is assigned.
 *
 * \remarks The MediaFileInfo keeps the ownership over the returned
 *          pointer. The returned ID3v1 tag will be destroyed when the
 *          MediaFileInfo gets invalidated.
 */
inline Id3v1Tag *MediaFileInfo::id3v1Tag() const
{
    return m_id3v1Tag.get();
}

/*!
 * \brief Returns pointers to the assigned ID3v2 tags.
 *
 * \remarks The MediaFileInfo keeps the ownership over the returned
 *          pointers. The returned ID3v2 tags will be destroyed when the
 *          MediaFileInfo gets invalidated.
 */
inline const std::vector<std::unique_ptr<Id3v2Tag>> &MediaFileInfo::id3v2Tags() const
{
    return m_id3v2Tags;
}

/*!
 * \brief Returns the directory used to store backup files.
 * \remarks If empty, backup files will be stored in the same directory of the file being modified.
 * \sa setBackupDirectory()
 */
inline const std::string &MediaFileInfo::backupDirectory() const
{
    return m_backupDirectory;
}

/*!
 * \brief Sets the directory used to store backup files.
 * \remarks If empty, backup files will be stored in the same directory of the file being modified.
 */
inline void MediaFileInfo::setBackupDirectory(std::string_view backupDirectory)
{
    m_backupDirectory = backupDirectory;
}

/*!
 * \brief Sets the directory used to store backup files.
 * \remarks If empty, backup files will be stored in the same directory of the file being modified.
 */
inline void MediaFileInfo::setBackupDirectory(std::string &&backupDirectory)
{
    m_backupDirectory = std::move(backupDirectory);
}

/*!
 * \brief Returns the "save file path" which has been set using setSaveFilePath().
 * \sa setSaveFilePath()
 */
inline const std::string &MediaFileInfo::saveFilePath() const
{
    return m_saveFilePath;
}

/*!
 * \brief Sets the "save file path".
 *
 * If \a saveFilePath is not empty, this path will be used to save the output file
 * when applying changes using applyChanges(). Thus the current file is not modified
 * by applyChanges() in this case and the variable isForcingRewrite() does not
 * affect the behaviour of applyChanges(). If the changes have been applied
 * without fatal errors the "save file path" is cleared and used as the
 * new regular path().
 *
 * By default, this path is empty.
 *
 * \remarks \a saveFilePath mustn't be the current path().
 */
inline void MediaFileInfo::setSaveFilePath(std::string_view saveFilePath)
{
    m_saveFilePath = saveFilePath;
}

/*!
 * \brief Sets the "save file path".
 */
inline void MediaFileInfo::setSaveFilePath(std::string &&saveFilePath)
{
    m_saveFilePath = std::move(saveFilePath);
}

/*!
 * \brief Sets the writing application as container-level meta-data.
 * \remarks This is not read from the file when parsing and only used when saving changes.
 * \sa setWritingApplication() for more details
 */
inline const std::string &MediaFileInfo::writingApplication() const
{
    return m_writingApplication;
}

/*!
 * \brief Sets the writing application as container-level meta-data. Put the name of your application here.
 * \remarks
 * - Currently only used when making Matroska files.
 * - The assigned value is ignored when MediaFileHandlingFlags::PreserveWritingApplication is set.
 */
inline void MediaFileInfo::setWritingApplication(std::string_view writingApplication)
{
    m_writingApplication = writingApplication;
}

/*!
 * \brief Returns the container for the current file.
 *
 * If there is not corresponding subclass of AbstractContainer for the
 * container format or the container has not been parsed yet using
 * the parseContainerFormat() method nullptr will be returned.
 *
 * \sa parseContainerFormat()
 * \sa AbstractContainer
 */
inline AbstractContainer *MediaFileInfo::container() const
{
    return m_container.get();
}

/*!
 * \brief Returns the currently configured file handling flags.
 */
inline MediaFileHandlingFlags MediaFileInfo::fileHandlingFlags()
{
    return m_fileHandlingFlags;
}

/*!
 * \brief Replaces all currently configured file handling flags with the specified \a flags.
 */
inline void MediaFileInfo::setFileHandlingFlags(MediaFileHandlingFlags flags)
{
    m_fileHandlingFlags = flags;
}

/*!
 * \brief Returns an indication whether forcing a full parse is enabled.
 *
 * If enabled the parser will analyse the file structure as deep as possible.
 * This might cause long parsing times for big files.
 *
 * \sa setForceFullParse()
 */
inline bool MediaFileInfo::isForcingFullParse() const
{
    return m_fileHandlingFlags & MediaFileHandlingFlags::ForceFullParse;
}

/*!
 * \brief Sets whether forcing a full parse is enabled.
 * \remarks The setting is applied next time parsing. The current parsing results are not mutated.
 * \sa isForcingFullParse()
 */
inline void MediaFileInfo::setForceFullParse(bool forceFullParse)
{
    CppUtilities::modFlagEnum(m_fileHandlingFlags, MediaFileHandlingFlags::ForceFullParse, forceFullParse);
}

/*!
 * \brief Returns whether forcing rewriting (when applying changes) is enabled.
 */
inline bool MediaFileInfo::isForcingRewrite() const
{
    return m_fileHandlingFlags & MediaFileHandlingFlags::ForceRewrite;
}

/*!
 * \brief Sets whether forcing rewriting (when applying changes) is enabled.
 */
inline void MediaFileInfo::setForceRewrite(bool forceRewrite)
{
    CppUtilities::modFlagEnum(m_fileHandlingFlags, MediaFileHandlingFlags::ForceRewrite, forceRewrite);
}

/*!
 * \brief Returns the minimum padding to be written before the data blocks when applying changes.
 *
 * Minimum padding in front of the file allows adding additional fields afterwards without needing
 * to rewrite the entire file or to put tag information at the end of the file.
 *
 * The default value is 0.
 *
 * \sa maxPadding()
 * \sa tagPosition()
 * \sa setMinPadding()
 */
inline std::size_t MediaFileInfo::minPadding() const
{
    return m_minPadding;
}

/*!
 * \brief Sets the minimum padding to be written before the data blocks when applying changes.
 * \remarks This value might be ignored if not supported by the container/tag format or the corresponding implementation.
 * \sa minPadding()
 */
inline void MediaFileInfo::setMinPadding(std::size_t minPadding)
{
    m_minPadding = minPadding;
}

/*!
 * \brief Returns the maximum padding to be written before the data blocks when applying changes.
 *
 * Maximum padding in front of the file allows adding additional fields afterwards without needing
 * to rewrite the entire file or to put tag information at the end of the file.
 *
 * The default value is 0 which will force the library to rewrite the entire file almost always when
 * applying changes. Increate the value using setMaxPadding() to prevent this.
 *
 * \sa minPadding()
 * \sa tagPosition()
 * \sa setMaxPadding()
 */
inline std::size_t MediaFileInfo::maxPadding() const
{
    return m_maxPadding;
}

/*!
 * \brief Sets the maximum padding to be written before the data blocks when applying changes.
 * \remarks This value might be ignored if not supported by the container/tag format or the corresponding implementation.
 * \sa maxPadding()
 */
inline void MediaFileInfo::setMaxPadding(std::size_t maxPadding)
{
    m_maxPadding = maxPadding;
}

/*!
 * \brief Returns the padding to be written before the data block when applying changes and the file needs to be rewritten anyways.
 *
 * Padding in front of the file allows adding additional fields afterwards without needing
 * to rewrite the entire file or to put tag information at the end of the file.
 */
inline std::size_t MediaFileInfo::preferredPadding() const
{
    return m_preferredPadding;
}

/*!
 * \brief Sets the padding to be written before the data block when applying changes and the file needs to be rewritten anyways.
 * \remarks This value might be ignored if not supported by the container/tag format or the corresponding implementation.
 * \sa preferredPadding()
 */
inline void MediaFileInfo::setPreferredPadding(std::size_t preferredPadding)
{
    m_preferredPadding = preferredPadding;
}

/*!
 * \brief Returns the position (in the output file) where the tag information is written when applying changes.
 * \sa setTagPosition()
 * \remarks To determine the current tag position, use AbstractContainer::determineTagPosition().
 */
inline ElementPosition MediaFileInfo::tagPosition() const
{
    return m_tagPosition;
}

/*!
 * \brief Sets the position (in the output file) where the tag information is written when applying changes.
 *
 * \remarks
 *  - If putting the tags at another position would prevent rewriting the entire file the specified position
 *    might not be used if forceTagPosition() is false.
 *  - However if the specified position is not supported by the container/tag format or by the implementation
 *    for the format it is ignored (even if forceTagPosition() is true).
 *  - Default value is ElementPosition::BeforeData
 */
inline void MediaFileInfo::setTagPosition(ElementPosition tagPosition)
{
    m_tagPosition = tagPosition;
}

/*!
 * \brief Returns whether tagPosition() is forced.
 * \sa setForceTagPosition()
 * \sa tagPosition(), setTagPosition()
 */
inline bool MediaFileInfo::forceTagPosition() const
{
    return m_fileHandlingFlags & MediaFileHandlingFlags::ForceTagPosition;
}

/*!
 * \brief Sets whether tagPosition() is forced.
 * \sa forceTagPosition()
 * \sa tagPosition(), setTagPosition()
 */
inline void MediaFileInfo::setForceTagPosition(bool forceTagPosition)
{
    CppUtilities::modFlagEnum(m_fileHandlingFlags, MediaFileHandlingFlags::ForceTagPosition, forceTagPosition);
}

/*!
 * \brief Returns the position (in the output file) where the index is written when applying changes.
 * \sa setIndexPosition()
 * \remarks To determine the current index position, use AbstractContainer::determineIndexPosition().
 */
inline ElementPosition MediaFileInfo::indexPosition() const
{
    return m_indexPosition;
}

/*!
 * \brief Sets the position (in the output file) where the index is written when applying changes.
 * \remarks Same rules as for tagPosition() apply. If conflicting with tagPosition(), tagPosition() has priority.
 *
 */
inline void MediaFileInfo::setIndexPosition(ElementPosition indexPosition)
{
    m_indexPosition = indexPosition;
}

/*!
 * \brief Returns whether indexPosition() is forced.
 * \sa setForceIndexPosition()
 * \sa indexPosition(), setIndexPosition()
 */
inline bool MediaFileInfo::forceIndexPosition() const
{
    return m_fileHandlingFlags & MediaFileHandlingFlags::ForceIndexPosition;
}

/*!
 * \brief Sets whether indexPosition() is forced.
 * \sa forceIndexPosition()
 * \sa indexPosition(), setIndexPosition()
 */
inline void MediaFileInfo::setForceIndexPosition(bool forceIndexPosition)
{
    CppUtilities::modFlagEnum(m_fileHandlingFlags, MediaFileHandlingFlags::ForceIndexPosition, forceIndexPosition);
}

/*!
 * \brief Returns the maximal file size for a "full parse" in byte.
 * \remarks
 * So far this is Matroska-specific: The "Tags" element (which holds the tag information) is commonly at the end
 * of a Matroska file. Hence the parser needs to walk through the entire file to find the tag information if no
 * "SeekHead" element is present which might causes long loading times. To avoid this a maximal file size for a
 * "full parse" can be specified. The disadvantage is that the parser relies on the presence of a SeekHead element
 * on larger files to retrieve tag information.
 *
 * The default value is 50 MiB.
 *
 * \sa setMaxFullParseSize()
 */
inline std::uint64_t MediaFileInfo::maxFullParseSize() const
{
    return m_maxFullParseSize;
}

/*!
 * \brief Sets the maximal file size for a "full parse" in byte.
 * \sa maxFullParseSize()
 */
inline void MediaFileInfo::setMaxFullParseSize(std::uint64_t maxFullParseSize)
{
    m_maxFullParseSize = maxFullParseSize;
}

} // namespace TagParser

#endif // TAG_PARSER_MEDIAINFO_H
