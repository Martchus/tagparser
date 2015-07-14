#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include "statusprovider.h"
#include "basicfileinfo.h"
#include "abstractcontainer.h"

#include <string>
#include <fstream>
#include <vector>
#include <memory>

namespace Media
{

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

enum class MediaType;
enum class ContainerFormat;
enum class TagType : unsigned int;

/*!
 * \brief Specifies the usage of a certain tag type.
 */
enum class TagUsage
{
    Always, /**< a tag of the type is always used; a new tag is created if none exists yet */
    KeepExisting, /**< existing tags of the type are kept and updated but no new tag is created  */
    Never /**< tags of the type are never used; a possibly existing tag of the type is removed */
};

class LIB_EXPORT MediaFileInfo : public BasicFileInfo, public StatusProvider
{
public:
    // constructor, destructor
    MediaFileInfo();
    MediaFileInfo(const std::string &path);
    MediaFileInfo(const MediaFileInfo &) = delete;
    MediaFileInfo &operator=(const MediaFileInfo &) = delete;
    virtual ~MediaFileInfo();

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
    bool isContainerParsed() const;
    // ... the capters
    bool areChaptersParsed() const;
    std::vector<AbstractChapter *> chapters() const;
    bool areChaptersSupported() const;
    // ... the attachments
    bool areAttachmentsParsed() const;
    std::vector<AbstractAttachment *> attachments() const;
    bool areAttachmentsSupported() const;
    // ... the tracks
    bool areTracksParsed() const;
    std::size_t trackCount() const;
    std::vector<AbstractTrack *> tracks() const;
    bool hasTracksOfType(Media::MediaType type) const;
    ChronoUtilities::TimeSpan duration() const;
    bool areTracksSupported() const;
    // ... the tags
    bool areTagsParsed() const;
    bool hasId3v1Tag() const;
    bool hasId3v2Tag() const;
    bool hasAnyTag() const;
    Id3v1Tag *id3v1Tag() const;
    const std::vector<std::unique_ptr<Id3v2Tag> > &id3v2Tags() const;
    void tags(std::vector<Tag *> &tags) const;
    std::vector<Tag *> tags() const;
    Mp4Tag *mp4Tag() const;
    const std::vector<std::unique_ptr<MatroskaTag> > &matroskaTags() const;
    bool areTagsSupported() const;

    // methods to create/remove tags
    bool createAppropriateTags(bool treatUnknownFilesAsMp3Files = false, TagUsage id3v1usage = TagUsage::KeepExisting,
                               TagUsage id3v2usage = TagUsage::Always, bool mergeMultipleSucessiveId3v2Tags = true,
                               bool keepExistingId3v2version = true, uint32 id3v2version = 3);
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
    NotificationList gatherRelatedNotifications() const;
    void clearParsingResults();

    // methods to get, set object behaviour
    bool isForcingFullParse() const;
    void setForceFullParse(bool forceFullParse);

protected:
    virtual void invalidated();

private:
    // private methods internally used when rewriting the file to apply new tag information
    // currently only the makeMp3File() methods is present; corresponding methods for
    // other formats are outsourced to container classes
    void makeMp3File();
    // fields related to the container
    bool m_containerParsed;
    ContainerFormat m_containerFormat;
    std::streamoff m_containerOffset;
    uint64 m_paddingSize;
    bool m_actualExistingId3v1Tag;
    std::list<std::streamoff> m_actualId3v2TagOffsets;
    std::unique_ptr<AbstractContainer> m_container;
    // fields related to the tracks
    bool m_tracksParsed;
    std::unique_ptr<AbstractTrack> m_singleTrack;
    // fields related to the tag
    bool m_tagParsed;
    std::unique_ptr<Id3v1Tag> m_id3v1Tag;
    std::vector<std::unique_ptr<Id3v2Tag> > m_id3v2Tags;
    // fields specifying object behaviour
    bool m_forceFullParse;
};

/*!
 * \brief Returns an indication whether the container format has been parsed yet.
 */
inline bool MediaFileInfo::isContainerParsed() const
{
    return m_containerParsed;
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
inline bool MediaFileInfo::areTagsParsed() const
{
    return m_tagParsed;
}

/*!
 * \brief Returns an indication whether tracks have been parsed yet.
 */
inline bool MediaFileInfo::areTracksParsed() const
{
    return m_tracksParsed;
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
inline bool MediaFileInfo::areChaptersParsed() const
{
    return m_container && m_container->areChaptersParsed();
}

/*!
 * \brief Returns whether the attachments have been parsed yet.
 */
inline bool MediaFileInfo::areAttachmentsParsed() const
{
    return m_container && m_container->areAttachmentsParsed();
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

}

#endif // MEDIAINFO_H
