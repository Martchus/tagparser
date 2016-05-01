#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include "./signature.h"
#include "./statusprovider.h"
#include "./basicfileinfo.h"
#include "./abstractcontainer.h"

#include <string>
#include <fstream>
#include <vector>
#include <memory>

namespace Media {

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
class WaveAudioStream;
class MpegAudioFrameStream;
class VorbisComment;

enum class MediaType;
enum class TagType : unsigned int;

/*!
 * \brief The TagUsage enum specifies the usage of a certain tag type.
 */
enum class TagUsage
{
    Always, /**< a tag of the type is always used; a new tag is created if none exists yet */
    KeepExisting, /**< existing tags of the type are kept and updated but no new tag is created  */
    Never /**< tags of the type are never used; a possibly existing tag of the type is removed */
};

enum class ElementPosition
{
    BeforeData, /**< the element is positioned before the actual data */
    AfterData, /**< the element is positioned after the actual data */
    Keep /**< the element is placed at its previous position */
};

/*!
 * \brief The ParsingStatus enum specifies whether a certain part of the file (tracks, tags, ...) has
 *        been parsed yet and if what the parsing result is.
 */
enum class ParsingStatus : byte
{
    NotParsedYet, /**< the part has not been parsed yet */
    Ok, /**< the part has been parsed and no critical errors occured */
    NotSupported, /**< tried to parse the part, but the format is not supported */
    CriticalFailure /**< tried to parse the part, but critical errors occured */
};

class LIB_EXPORT MediaFileInfo : public BasicFileInfo, public StatusProvider
{
public:
    // constructor, destructor
    MediaFileInfo();
    MediaFileInfo(const std::string &path);
    MediaFileInfo(const MediaFileInfo &) = delete;
    MediaFileInfo &operator=(const MediaFileInfo &) = delete;
    ~MediaFileInfo();

    // methods to parse file
    void parseContainerFormat();
    void parseTracks();
    void parseTags();
    void parseChapters();
    void parseAttachments();
    void parseEverything();

    // methods to apply changes
    void applyChanges();

    // methods to get parsed information regarding ...
    // ... the container
    ContainerFormat containerFormat() const;
    const char *containerFormatName() const;
    const char *containerFormatAbbreviation() const;
    const char *containerFormatSubversion() const;
    const char *mimeType() const;
    uint64 containerOffset() const;
    uint64 paddingSize() const;
    AbstractContainer *container() const;
    ParsingStatus containerParsingStatus() const;
    // ... the capters
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
    bool hasTracksOfType(Media::MediaType type) const;
    ChronoUtilities::TimeSpan duration() const;
    bool areTracksSupported() const;
    // ... the tags
    ParsingStatus tagsParsingStatus() const;
    bool hasId3v1Tag() const;
    bool hasId3v2Tag() const;
    bool hasAnyTag() const;
    Id3v1Tag *id3v1Tag() const;
    const std::vector<std::unique_ptr<Id3v2Tag> > &id3v2Tags() const;
    void tags(std::vector<Tag *> &tags) const;
    std::vector<Tag *> tags() const;
    Mp4Tag *mp4Tag() const;
    const std::vector<std::unique_ptr<MatroskaTag> > &matroskaTags() const;
    VorbisComment *vorbisComment() const;
    bool areTagsSupported() const;

    // methods to create/remove tags
    bool createAppropriateTags(bool treatUnknownFilesAsMp3Files = false, TagUsage id3v1usage = TagUsage::KeepExisting,
                               TagUsage id3v2usage = TagUsage::Always, bool mergeMultipleSuccessiveId3v2Tags = true,
                               bool keepExistingId3v2version = true, uint32 id3v2version = 3, const std::vector<TagTarget> &requiredTargets = std::vector<TagTarget>());
    bool removeId3v1Tag();
    Id3v1Tag *createId3v1Tag();
    bool removeId3v2Tag(Id3v2Tag *tag);
    bool removeAllId3v2Tags();
    Id3v2Tag *createId3v2Tag();
    void removeTag(Tag *tag);
    void removeAllTags();
    void mergeId3v2Tags();

    // methods to get/wipe notifications
    bool haveRelatedObjectsNotifications() const;
    NotificationType worstNotificationTypeIncludingRelatedObjects() const;
    void gatherRelatedNotifications(NotificationList &notifications) const;
    NotificationList gatherRelatedNotifications() const;
    void clearParsingResults();

    // methods to get, set object behaviour
    const std::string &saveFilePath() const;
    void setSaveFilePath(const std::string &saveFilePath);
    bool isForcingFullParse() const;
    void setForceFullParse(bool forceFullParse);
    bool isForcingRewrite() const;
    void setForceRewrite(bool forceRewrite);
    size_t minPadding() const;
    void setMinPadding(size_t minPadding);
    size_t maxPadding() const;
    void setMaxPadding(size_t maxPadding);
    size_t preferredPadding() const;
    void setPreferredPadding(size_t preferredPadding);
    ElementPosition tagPosition() const;
    void setTagPosition(ElementPosition tagPosition);
    bool forceTagPosition() const;
    void setForceTagPosition(bool forceTagPosition);
    ElementPosition indexPosition() const;
    void setIndexPosition(ElementPosition indexPosition);
    bool forceIndexPosition() const;
    void setForceIndexPosition(bool forceTagPosition);

protected:
    virtual void invalidated();

private:
    // private methods internally used when rewriting the file to apply new tag information
    // currently only the makeMp3File() methods is present; corresponding methods for
    // other formats are outsourced to container classes
    void makeMp3File();

    // fields related to the container
    ParsingStatus m_containerParsingStatus;
    ContainerFormat m_containerFormat;
    std::streamoff m_containerOffset;
    uint64 m_paddingSize;
    bool m_actualExistingId3v1Tag;
    std::list<std::streamoff> m_actualId3v2TagOffsets;
    std::unique_ptr<AbstractContainer> m_container;

    // fields related to the tracks
    ParsingStatus m_tracksParsingStatus;
    std::unique_ptr<AbstractTrack> m_singleTrack;

    // fields related to the tag
    ParsingStatus m_tagsParsingStatus;
    std::unique_ptr<Id3v1Tag> m_id3v1Tag;
    std::vector<std::unique_ptr<Id3v2Tag> > m_id3v2Tags;

    // fields related to the chapters and the attachments
    ParsingStatus m_chaptersParsingStatus;
    ParsingStatus m_attachmentsParsingStatus;

    // fields specifying object behaviour
    std::string m_saveFilePath;
    bool m_forceFullParse;
    bool m_forceRewrite;
    size_t m_minPadding;
    size_t m_maxPadding;
    size_t m_preferredPadding;
    ElementPosition m_tagPosition;
    bool m_forceTagPosition;
    ElementPosition m_indexPosition;
    bool m_forceIndexPosition;
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
inline const char *MediaFileInfo::containerFormatName() const
{
    return Media::containerFormatName(m_containerFormat);
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
inline const char *MediaFileInfo::containerFormatSubversion() const
{
    return Media::containerFormatSubversion(m_containerFormat);
}

/*!
 * \brief Returns the actual container start offset.
 */
inline uint64 MediaFileInfo::containerOffset() const
{
    return m_containerOffset;
}

/*!
 * \brief Returns the padding size. Container format and tags should have been parsed yet.
 */
inline uint64 MediaFileInfo::paddingSize() const
{
    return m_paddingSize;
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
inline size_t MediaFileInfo::trackCount() const
{
    return m_singleTrack ? 1 : 0 + m_container->trackCount();
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
    return m_id3v2Tags.size();
}

/*!
 * \brief Returns an indication whether a tag of any format is assigned.
 */
inline bool MediaFileInfo::hasAnyTag() const
{
    return hasId3v1Tag() || hasId3v2Tag() || (m_container && m_container->tagCount());
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
inline const std::vector<std::unique_ptr<Id3v2Tag> > &MediaFileInfo::id3v2Tags() const
{
    return m_id3v2Tags;
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
inline void MediaFileInfo::setSaveFilePath(const std::string &saveFilePath)
{
    m_saveFilePath = saveFilePath;
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
 * \brief Returns an indication whether forcing a full parse is enabled.
 *
 * If enabled the parser will analyse the file structure as deep as possible.
 * This might cause long parsing times for big files.
 *
 * \sa setForceFullParse()
 */
inline bool MediaFileInfo::isForcingFullParse() const
{
    return m_forceFullParse;
}

/*!
 * \brief Sets whether forcing a full parse is enabled.
 * \remarks The setting is applied next time parsing. The current parsing results are not mutated.
 * \sa isForcingFullParse()
 */
inline void MediaFileInfo::setForceFullParse(bool forceFullParse)
{
    m_forceFullParse = forceFullParse;
}

/*!
 * \brief Returns whether forcing rewriting (when applying changes) is enabled.
 */
inline bool MediaFileInfo::isForcingRewrite() const
{
    return m_forceRewrite;
}

/*!
 * \brief Sets whether forcing rewriting (when applying changes) is enabled.
 */
inline void MediaFileInfo::setForceRewrite(bool forceRewrite)
{
    m_forceRewrite = forceRewrite;
}

/*!
 * \brief Returns the minimum padding to be written before the data blocks when applying changes.
 *
 * Padding in front of the file allows adding additional fields afterwards whithout needing
 * to rewrite the entire file or to put tag information at the end of the file.
 *
 * \sa maxPadding()
 * \sa tagPosition()
 * \sa setMinPadding()
 */
inline size_t MediaFileInfo::minPadding() const
{
    return m_minPadding;
}

/*!
 * \brief Sets the minimum padding to be written before the data blocks when applying changes.
 * \remarks This value might be ignored if not supported by the container/tag format or the corresponding implementation.
 * \sa minPadding()
 */
inline void MediaFileInfo::setMinPadding(size_t minPadding)
{
    m_minPadding = minPadding;
}

/*!
 * \brief Returns the maximum padding to be written before the data blocks when applying changes.
 *
 * Padding in front of the file allows adding additional fields afterwards whithout needing
 * to rewrite the entire file or to put tag information at the end of the file.
 *
 * \sa minPadding()
 * \sa tagPosition()
 * \sa setMaxPadding()
 */
inline size_t MediaFileInfo::maxPadding() const
{
    return m_maxPadding;
}

/*!
 * \brief Sets the maximum padding to be written before the data blocks when applying changes.
 * \remarks This value might be ignored if not supported by the container/tag format or the corresponding implementation.
 * \sa maxPadding()
 */
inline void MediaFileInfo::setMaxPadding(size_t maxPadding)
{
    m_maxPadding = maxPadding;
}

/*!
 * \brief Returns the padding to be written before the data block when applying changes and the file needs to be rewritten anyways.
 *
 * Padding in front of the file allows adding additional fields afterwards whithout needing
 * to rewrite the entire file or to put tag information at the end of the file.
 */
inline size_t MediaFileInfo::preferredPadding() const
{
    return m_preferredPadding;
}

/*!
 * \brief Sets the padding to be written before the data block when applying changes and the file needs to be rewritten anyways.
 * \remarks This value might be ignored if not supported by the container/tag format or the corresponding implementation.
 * \sa preferredPadding()
 */
inline void MediaFileInfo::setPreferredPadding(size_t preferredPadding)
{
    m_preferredPadding = preferredPadding;
}

/*!
 * \brief Returns the position (in the output file) where the tag information is written when applying changes.
 * \sa setTagPosition()
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
    return m_forceTagPosition;
}

/*!
 * \brief Sets whether tagPosition() is forced.
 * \sa forceTagPosition()
 * \sa tagPosition(), setTagPosition()
 */
inline void MediaFileInfo::setForceTagPosition(bool forceTagPosition)
{
    m_forceTagPosition = forceTagPosition;
}

/*!
 * \brief Returns the position (in the output file) where the index is written when applying changes.
 * \sa setIndexPosition()
 *
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
    return m_forceIndexPosition;
}

/*!
 * \brief Sets whether indexPosition() is forced.
 * \sa forceIndexPosition()
 * \sa indexPosition(), setIndexPosition()
 */
inline void MediaFileInfo::setForceIndexPosition(bool forceIndexPosition)
{
    m_forceIndexPosition = forceIndexPosition;
}

}

#endif // MEDIAINFO_H
