#include "./mediafileinfo.h"
#include "./exceptions.h"
#include "./tag.h"
#include "./signature.h"
#include "./abstracttrack.h"
#include "./backuphelper.h"

#include "./id3/id3v1tag.h"
#include "./id3/id3v2tag.h"

#include "./wav/waveaudiostream.h"

#include "./mpegaudio/mpegaudioframestream.h"

#include "./adts/adtsstream.h"

#include "./mp4/mp4container.h"
#include "./mp4/mp4atom.h"
#include "./mp4/mp4tag.h"
#include "./mp4/mp4ids.h"
#include "./mp4/mp4track.h"

#include "./matroska/ebmlelement.h"
#include "./matroska/matroskacontainer.h"
#include "./matroska/matroskatag.h"
#include "./matroska/matroskatrack.h"

#include "./ogg/oggcontainer.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/misc/memory.h>

#include <unistd.h>
#include <cstdio>
#include <algorithm>
#include <iomanip>
#include <ios>
#include <system_error>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

namespace Media {

#ifdef FORCE_FULL_PARSE_DEFAULT
# define MEDIAINFO_CPP_FORCE_FULL_PARSE true
#else
# define MEDIAINFO_CPP_FORCE_FULL_PARSE false
#endif

/*!
 * \class Media::MediaFileInfo
 * \brief The MediaFileInfo class extends the BasicFileInfo class.
 *
 * The MediaFileInfo class allows to read and edit meta information
 * of MP3 files with ID3 tag and MP4 files with iTunes tag. It also
 * provides some technical information about these types of files such
 * as contained streams.
 * A bunch of other file types (see Media::ContainerFormat) can be recognized.
 */

/*!
 * \brief Constructs a new MediaFileInfo.
 */
MediaFileInfo::MediaFileInfo() :
    m_containerParsed(false),
    m_containerFormat(ContainerFormat::Unknown),
    m_containerOffset(0),
    m_actualExistingId3v1Tag(false),
    m_tracksParsed(false),
    m_tagParsed(false),
    m_forceFullParse(MEDIAINFO_CPP_FORCE_FULL_PARSE)
{}

/*!
 * \brief Constructs a new MediaFileInfo for the specified file.
 *
 * \param path Specifies the absolute or relative path of the file.
 */
MediaFileInfo::MediaFileInfo(const string &path) :
    BasicFileInfo(path),
    m_containerParsed(false),
    m_containerFormat(ContainerFormat::Unknown),
    m_containerOffset(0),
    m_actualExistingId3v1Tag(false),
    m_tracksParsed(false),
    m_tagParsed(false),
    m_forceFullParse(MEDIAINFO_CPP_FORCE_FULL_PARSE)
{}

/*!
 * \brief Destroys the MediaFileInfo.
 */
MediaFileInfo::~MediaFileInfo()
{}

/*!
 * \brief Parses the container format of the current file.
 *
 * This method parses the container of the current file format if it has not been
 * parsed yet.
 *
 * After calling this method the methods containerFormat(), containerFormatName(),
 * containerFormatAbbreviation(), containerFormatSubversion(), containerMimeType(),
 * container(), mp4Container() and matroskaContainer() will return the parsed
 * information.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \sa isContainerParsed()
 * \sa parseTracks()
 * \sa parseTag()
 * \sa parseChapters()
 * \sa parseEverything()
 */
void MediaFileInfo::parseContainerFormat()
{
    if(isContainerParsed()) {
        // there's no need to read the container format twice
        return;
    }
    invalidateStatus();
    static const string context("parsing file header");
    open(); // ensure the file is open
    m_containerFormat = ContainerFormat::Unknown;
    // file size
    m_paddingSize = 0x0u;
    m_containerOffset = 0;
    // read signatrue
    char buff[16];
startParsingSignature:
    if(size() - m_containerOffset >= 16) {
        stream().seekg(m_containerOffset, ios_base::beg);
        stream().read(buff, sizeof(buff));
        // skip zero bytes/padding
        if(!(*reinterpret_cast<uint32 *>(buff))) {
            // give up after 0x100 bytes
            if(m_paddingSize >= 0x100u) {
                m_containerParsed = true;
                m_containerFormat = ContainerFormat::Unknown;
                return;
            }
            m_containerOffset += sizeof(uint32); // set the container offset
            m_paddingSize += sizeof(uint32);
            goto startParsingSignature; // read signature again
        }
        if(m_paddingSize) {
            addNotification(NotificationType::Warning, ConversionUtilities::numberToString(m_paddingSize) + " zero-bytes skipped at the beginning of the file.", context);
        }
        // parse signature
        switch(m_containerFormat = parseSignature(buff, sizeof(buff))) {
        case ContainerFormat::Id2v2Tag:
            m_actualId3v2TagOffsets.push_back(m_containerOffset);
            if(m_actualId3v2TagOffsets.size() == 2) {
                addNotification(NotificationType::Warning, "There is more then just one ID3v2 header at the beginning of the file.", context);
            }
            stream().seekg(m_containerOffset + 6, ios_base::beg);
            stream().read(buff, 4);
            // set the container offset to skip ID3v2 header
            m_containerOffset += ConversionUtilities::toNormalInt(ConversionUtilities::BE::toUInt32(buff)) + 10;
            goto startParsingSignature; // read signature again
        case ContainerFormat::Mp4: {
            m_container = make_unique<Mp4Container>(*this, m_containerOffset);
            NotificationList notifications;
            try {
                static_cast<Mp4Container *>(m_container.get())->validateElementStructure(notifications, &m_paddingSize);
            } catch (Failure &) {
                // nothing to do here, notifications will be added
            }
            addNotifications(notifications);
            break;
        } case ContainerFormat::Ebml: {
            unique_ptr<MatroskaContainer> container = make_unique<MatroskaContainer>(*this, m_containerOffset);
            NotificationList notifications;
            try {
                container->parseHeader();
                if(container->documentType() == "matroska") {
                    m_containerFormat = ContainerFormat::Matroska;
                } else if(container->documentType() == "webm") {
                    m_containerFormat = ContainerFormat::Webm;
                }
                if(m_forceFullParse) {
                    // validating the element structure of Matroska files takes too long when
                    // parsing big files so do this only when explicitely desired
                    container->validateElementStructure(notifications, &m_paddingSize);
                    container->validateIndex();
                }
            } catch(Failure &) {
                // nothing to do, notificatons will be added
            }
            m_container = move(container);
            addNotifications(notifications);
            break;
        } case ContainerFormat::Ogg:
            m_container = make_unique<OggContainer>(*this, m_containerOffset);
            static_cast<OggContainer *>(m_container.get())->setChecksumValidationEnabled(m_forceFullParse);
            break;
        default:
            ;
        }
    }
    m_containerParsed = true;
}

/*!
 * \brief Parses the tracks of the current file.
 *
 * This method parses the tracks of the current file if not been parsed yet.
 * After calling this method the methods trackCount(), tracks(), and
 * hasTracksOfType() will return the parsed
 * information.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \remarks parseContainerFormat() is called before the tracks will be parsed.
 *
 * \sa areTracksParsed()
 * \sa parseContainerFormat()
 * \sa parseTag()
 * \sa parseChapters()
 * \sa parseEverything()
 */
void MediaFileInfo::parseTracks()
{
    if(areTracksParsed()) { // there's no need to read the file segment twice
        return;
    }
    parseContainerFormat(); // ensure the container format has been load yet
    static const string context("parsing tracks");
    try {
        if(m_container) {
            m_container->parseTracks();
        } else {
            switch(m_containerFormat) {
            case ContainerFormat::RiffWave:
                m_singleTrack = make_unique<WaveAudioStream>(stream(), m_containerOffset);
                break;
            case ContainerFormat::MpegAudioFrames:
                m_singleTrack = make_unique<MpegAudioFrameStream>(stream(), m_containerOffset);
                break;
            case ContainerFormat::Adts:
                m_singleTrack = make_unique<AdtsStream>(stream(), m_containerOffset);
                break;
            default:
                throw NotImplementedException();
            }
            m_singleTrack->parseHeader();
        }
    } catch (NotImplementedException &) {
        addNotification(NotificationType::Information, "Parsing tracks is not implemented for the container format of the file.", context);
    } catch (Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse tracks.", context);
    }
    m_tracksParsed = true;
}

/*!
 * \brief Parses the tag(s) of the current file.
 *
 * This method parses the tag(s) of the current file if not been parsed yet.
 * After calling this method the methods id3v1Tag(), id3v2Tags(),
 * mp4Tag() and allTags() will return the parsed information.
 *
 * Previously assigned but not applied tag information will be discarted.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \remarks parseContainerFormat() is called before the tags informations will be parsed.
 *
 * \sa isTagParsed()
 * \sa parseContainerFormat()
 * \sa parseTracks()
 * \sa parseChapters()
 * \sa parseEverything()
 */
void MediaFileInfo::parseTags()
{
    if(areTagsParsed()) {
        return;
    }
    parseContainerFormat(); // ensure the container format has been load yet
    static const string context("parsing tag");
    // check for id3v1 tag
    if(size() >= 128) {
        m_id3v1Tag = make_unique<Id3v1Tag>();
        try {
            m_id3v1Tag->parse(stream(), true);
            m_actualExistingId3v1Tag = true;
        } catch(NoDataFoundException &) {
            m_id3v1Tag.reset(); // no ID3v1 tag found
        } catch(Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse ID3v1 tag.", context);
        }
    }
    // the offsets of the ID3v2 tags have already been parsed when parsing the container format
    m_id3v2Tags.clear();
    for(streamoff offset : m_actualId3v2TagOffsets) {
        unique_ptr<Id3v2Tag> id3v2Tag = make_unique<Id3v2Tag>();
        stream().seekg(offset, ios_base::beg);
        try {
            id3v2Tag->parse(stream());
            m_paddingSize += id3v2Tag->paddingSize();
        } catch(NoDataFoundException &) {
            continue;
        } catch(Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse ID3v2 tag.", context);
        }
        m_id3v2Tags.emplace_back(id3v2Tag.release());
    }
    if(m_container) {
        try {
            m_container->parseTags();
        } catch (NotImplementedException &) {
            addNotification(NotificationType::Information, "Parsing tags is not implemented for the container format of the file.", context);
        } catch (Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse tag.", context);
        }
    }
    m_tagParsed = true;
}

/*!
 * \brief Parses the chapters of the current file.
 *
 * This method parses the chapters of the current file if not been parsed yet.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \remarks parseContainerFormat() is called before the tags informations will be parsed.
 *
 * \sa areChaptersParsed()
 * \sa parseContainerFormat()
 * \sa parseTracks()
 * \sa parseTags()
 * \sa parseEverything()
 */
void MediaFileInfo::parseChapters()
{
    static const string context("parsing chapters");
    try {
        if(m_container) {
            m_container->parseChapters();
        } else {
            throw NotImplementedException();
        }
    } catch (NotImplementedException &) {
        addNotification(NotificationType::Information, "Parsing chapters is not implemented for the container format of the file.", context);
    } catch (Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse chapters.", context);
    }
}

void MediaFileInfo::parseAttachments()
{
    static const string context("parsing attachments");
    try {
        if(m_container) {
            m_container->parseAttachments();
        } else {
            throw NotImplementedException();
        }
    } catch (NotImplementedException &) {
        addNotification(NotificationType::Information, "Parsing attachments is not implemented for the container format of the file.", context);
    } catch (Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse attachments.", context);
    }
}

/*!
 * \brief Parses the container format, the tracks and the tag information of the current file.
 *
 * See the individual methods to for more details and exceptions which might be thrown.
 *
 * \sa parseContainerFormat();
 * \sa parseTracks();
 * \sa parseTag();
 */
void MediaFileInfo::parseEverything()
{
    parseContainerFormat();
    parseTracks();
    parseTags();
    parseChapters();
    parseAttachments();
}

/*!
 * \brief Ensures appropriate tags are created.
 * \param treatUnknownFilesAsMp3Files Specifies whether unknown file formats should be treated as MP3.
 * \param id3v1usage Specifies the usage of ID3v1 when creating tags for MP3 files.
 * \param id3v2usage Specifies the usage of ID3v2 when creating tags for MP3 files.
 * \param mergeMultipleSuccessiveId3v2Tags Specifies whether multiple successive ID3v2 tags should be merged (see mergeId3v2Tags()).
 * \param keepExistingId3v2version Specifies whether the version of existing ID3v2 tags should be updated.
 * \param id3v2version Specifies the IDv2 version to be used. Valid values are 2, 3 and 4.
 * \return Returns an indication whether appropriate tags could be created for the file.
 * \remarks
 *          - The ID3 related arguments are only practiced when the file format is MP3 or when the file format
 *            is unknown and \a treatUnknownFilesAsMp3Files is true. These arguments are ignored when creating
 *            tags for other known file formats such as MP4.
 *          - Tags might be removed as well. For example the existing ID3v1 tag of an MP3 file will be removed
 *            if \a id3v1Usage is set to TagUsage::Never.
 *          - The method might do nothing if the file already has appropriate tags.
 *          - This is only a convenience method. The task can be done by manually using the methods createId3v1Tag(),
 *            createId3v2Tag(), removeId3v1Tag() ... as well.
 *          - Some tag information might be discarded. For example when an ID3v2 tag needs to be removed (\a id3v2usage is set to TagUsage::Never)
 *            and an ID3v1 tag will be created instead not all fields can be transfered.
 */
bool MediaFileInfo::createAppropriateTags(bool treatUnknownFilesAsMp3Files, TagUsage id3v1usage, TagUsage id3v2usage, bool mergeMultipleSuccessiveId3v2Tags, bool keepExistingId3v2version, uint32 id3v2version)
{
    // check if tags need to be created/adjusted/removed
    if(m_container) { // container object takes care of tag management
        m_container->createTag();
    } else { // no container object present; creation of ID3 tag is possible
        if(!hasAnyTag() && !treatUnknownFilesAsMp3Files) {
            switch(containerFormat()) {
            case ContainerFormat::MpegAudioFrames:
            case ContainerFormat::Adts:
                break;
            default:
                return false;
            }
        }
        // create ID3 tags according to id3v2usage and id3v2usage
        if(id3v1usage == TagUsage::Always) {
            // always create ID3v1 tag -> ensure there is one
            createId3v1Tag();
        }
        if(id3v2usage == TagUsage::Always) {
            // always create ID3v2 tag -> ensure there is one and set version
            if(!hasId3v2Tag()) {
                createId3v2Tag()->setVersion(id3v2version, 0);
            }
        }
        if(mergeMultipleSuccessiveId3v2Tags) {
            mergeId3v2Tags();
        }
        // remove ID3 tags according to id3v2usage and id3v2usage
        if(id3v1usage == TagUsage::Never) {
            if(hasId3v1Tag()) {
                if(hasId3v2Tag()) {
                    // transfer tags to ID3v2 tag before removing
                    id3v2Tags().front()->insertValues(*id3v1Tag(), false);
                }
                removeId3v1Tag();
            }
        }
        if(id3v2usage == TagUsage::Never) {
            if(hasId3v1Tag()) {
                // transfer tags to ID3v1 tag before removing
                for(const unique_ptr<Id3v2Tag> &tag : id3v2Tags()) {
                    id3v1Tag()->insertValues(*tag, false);
                }
            }
            removeAllId3v2Tags();
        } else if(!keepExistingId3v2version) {
            // set version of ID3v2 tag according user preferences
            for(const unique_ptr<Id3v2Tag> &tag : id3v2Tags()) {
                tag->setVersion(id3v2version, 0);
            }
        }
    }
    return true;
}


/*!
 * \brief Applies assigned/changed tag information to the current file.
 *
 * This method applies previously assigned tag information to the current file.
 *
 * Depending on the changes to be applied the file will be rewritten.
 *
 * When the file needs to be rewritten it will be renamed. A new file with the old name
 * will be created to replace the old file.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 *
 * \remarks The method parseTag() needs to be called before this method can be called.
 *
 * \sa isTagParsed()
 * \sa parseTag()
 */
void MediaFileInfo::applyChanges()
{   
    const string context("making file");
    addNotification(NotificationType::Information, "Changes are about to be applied.", context);
    if(!areTagsParsed() && !areTracksParsed()) {
        addNotification(NotificationType::Critical, "Tags and tracks have to be parsed before changes can be applied.", context);
        throw InvalidDataException();
    }
    if(m_container) { // container object takes care
        // ID3 tags can not be applied in this case -> add warnings if ID3 tags have been assigned
        if(hasId3v1Tag()) {
            addNotification(NotificationType::Warning, "Assigned ID3v1 tag can't be attached and will be ignored.", context);
        }
        if(hasId3v2Tag()) {
            addNotification(NotificationType::Warning, "Assigned ID3v2 tag can't be attached and will be ignored.", context);
        }
        m_container->forwardStatusUpdateCalls(this);
        m_container->makeFile();
        m_tracksParsed &= m_container->areTracksParsed();
        m_tagParsed &= m_container->areTagsParsed();
    } else { // implementation if no container object is present
        // assume the file is a MP3 file
        makeMp3File();
    }
}

/*!
 * \brief Returns the abbreviation of the container format as C-style string.
 *
 * This abbreviation might be used as file extension.
 *
 * parseContainerFormat() needs to be called before. Otherwise
 * always an empty string will be returned.
 *
 * \sa containerFormat()
 * \sa containerFormatName()
 * \sa parseContainerFormat()
 */
const char *MediaFileInfo::containerFormatAbbreviation() const
{
    MediaType mediaType = MediaType::Unknown;
    unsigned int version = 0;
    switch(m_containerFormat) {
    case ContainerFormat::Ogg:
    case ContainerFormat::Matroska:
    case ContainerFormat::Mp4:
        mediaType = hasTracksOfType(MediaType::Video) ? MediaType::Video : MediaType::Audio;
        break;
    case ContainerFormat::MpegAudioFrames:
        if(m_singleTrack) {
            version = m_singleTrack->format().sub;
        }
        break;
    default:
        ;
    }
    return Media::containerFormatAbbreviation(m_containerFormat, mediaType, version);
}

/*!
 * \brief Returns the MIME-type of the container format as C-style string.
 *
 * parseContainerFormat() needs to be called before. Otherwise
 * always an empty string will be returned.
 *
 * \sa containerFormat()
 * \sa containerFormatName()
 * \sa parseContainerFormat()
 */
const char *MediaFileInfo::mimeType() const
{
    switch(m_containerFormat) {
    case ContainerFormat::Asf:
        return "video/x-ms-asf";
    case ContainerFormat::Gif87a:
    case ContainerFormat::Gif89a:
        return "image/gif";
    case ContainerFormat::Jpeg:
        return "image/jpeg";
    case ContainerFormat::Png:
        return "image/png";
    case ContainerFormat::MpegAudioFrames:
        return "audio/mpeg";
    case ContainerFormat::Mp4:
        if(hasTracksOfType(MediaType::Video)) {
            return "video/mp4";
        }
        return "audio/mp4";
    case ContainerFormat::Ogg:
        if(hasTracksOfType(MediaType::Video)) {
            return "video/ogg";
        }
        return "audio/ogg";
    case ContainerFormat::Matroska:
        if(hasTracksOfType(MediaType::Video)) {
            return "video/x-matroska";
        }
        return "audio/x-matroska";
    case ContainerFormat::Bzip2:
        return "application/x-bzip";
    case ContainerFormat::Gzip:
        return "application/gzip";
    case ContainerFormat::Lha:
        return "application/x-lzh-compressed";
    case ContainerFormat::Rar:
        return "application/x-rar-compressed";
    case ContainerFormat::Lzip:
        return "application/x-lzip";
    case ContainerFormat::Zip:
        return "application/zip";
    case ContainerFormat::SevenZ:
        return "application/x-7z-compressed";
    case ContainerFormat::WindowsBitmap:
        return "image/bmp";
    case ContainerFormat::WindowsIcon:
        return "image/vnd.microsoft.icon";
    default:
        return "";
    }
}

/*!
 * \brief Returns the tracks for the current file.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns an empty vector.
 *
 * \remarks The MediaFileInfo keeps the ownership over the returned
 *          pointers. The returned Tracks will be destroyed when the
 *          MediaFileInfo gets invalidated.
 *
 * \sa parseTracks()
 */
vector<AbstractTrack *> MediaFileInfo::tracks() const
{
    vector<AbstractTrack *> res;
    if(m_singleTrack) {
        res.push_back(m_singleTrack.get());
    }
    if(m_container) {
        for(size_t i = 0, count = m_container->trackCount(); i < count; ++i) {
            res.push_back(m_container->track(i));
        }
    }
    return res;
}

/*!
 * \brief Returns an indication whether the current file has tracks of the specified \a type.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns false.
 *
 * \sa parseTracks()
 */
bool MediaFileInfo::hasTracksOfType(MediaType type) const
{
    if(!areTracksParsed()) {
        return false;
    }
    if(type == MediaType::Audio && m_singleTrack) {
        return true;
    } else if(m_container) {
        for(size_t i = 0, count = m_container->trackCount(); i < count; ++i) {
            if(m_container->track(i)->mediaType() == type) {
                return true;
            }
        }

    }
    return false;
}

/*!
 * \brief Returns the overall duration of the file is known; otherwise
 *        returns a TimeSpan with zero ticks.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns false.
 *
 * \sa parseTracks()
 */
ChronoUtilities::TimeSpan MediaFileInfo::duration() const
{
    TimeSpan res;
    if(m_container) {
        res = m_container->duration();
    } else {
        for(const AbstractTrack *track : tracks()) {
            if(track->duration() > res) {
                res = track->duration();
            }
        }
    }
    return res;
}

/*!
 * \brief Removes a possibly assigned ID3v1 tag from the current file.
 *
 * To apply the removal and other changings call the applyChanges() method.
 *
 * \returns Returns whether there was an ID3v1 tag assigned which could be removed.
 *
 * \sa applyChanges()
 */
bool MediaFileInfo::removeId3v1Tag()
{
    if(!areTagsParsed()) {
        return false;
    }
    if(m_id3v1Tag) {
        m_id3v1Tag.reset();
        return true;
    }
    return false;
}

/*!
 * \brief Creates an ID3v1 tag for the current file.
 *
 * This method does nothing the tags of the current file haven't been parsed using
 * the parseTags() method.
 *
 * If the file has already an ID3v1 tag no new tag will be created.
 *
 * To apply the created tag and other changings call the applyChanges() method.
 *
 * \returns Returns the ID3v1 tag of the current file or nullptr if the tag haven't been
 *          parsed yet.
 *
 * \sa applyChanges()
 */
Id3v1Tag *MediaFileInfo::createId3v1Tag()
{
    if(!areTagsParsed()) {
        return nullptr;
    }
    if(!m_id3v1Tag) {
        m_id3v1Tag = make_unique<Id3v1Tag>();
    }
    return m_id3v1Tag.get();
}

/*!
 * \brief Removes an assigned ID3v2 tag from the current file.
 *
 * To apply the removal and other changings call the applyChanges() method.
 *
 * \param tag Specifies the ID3v2 tag to be removed.
 *
 * \returns Returns whether there the an ID3v2 tag could be removed.
 *
 * \remarks The \a tag will be destroyed by the MediaFileInfo if it could be removed.
 *
 * \sa applyChanges()
 */
bool MediaFileInfo::removeId3v2Tag(Id3v2Tag *tag)
{
    if(!areTagsParsed()) {
        return false;
    }
    for(auto i = m_id3v2Tags.begin(), end = m_id3v2Tags.end(); i != end; ++i) {
        if(i->get() == tag) {
            m_id3v2Tags.erase(i);
            return true;
        }
    }
    return false;
}

/*!
 * \brief Removes all assigned ID3v2 tags from the current file.
 *
 * To apply the removal and other changings call the applyChanges() method.
 *
 * \returns Returns whether there where ID3v2 tags assigned which could be removed.
 *
 * \sa applyChanges()
 */
bool MediaFileInfo::removeAllId3v2Tags()
{
    if(!areTagsParsed() || m_id3v2Tags.size() == 0) {
        return false;
    }
    m_id3v2Tags.clear();
    return true;
}

/*!
 * \brief Creates an ID3v2 tag for the current file.
 *
 * This method does nothing the tags of the current file haven't been parsed using
 * the parseTags() method.
 *
 * If the file has already an ID3v2 tag no new tag will be created.
 *
 * To apply the created tag and other changings call the applyChanges() method.
 *
 * \returns Returns the first ID3v2 tag of the current file.
 *
 * \remarks The MediaFileInfo keeps the ownership over the created tag. It will be
 *          destroyed when the MediaFileInfo gets invalidated.
 *
 * \sa applyChanges()
 */
Id3v2Tag *MediaFileInfo::createId3v2Tag()
{
    if(!m_id3v2Tags.size()) {
        m_id3v2Tags.emplace_back(make_unique<Id3v2Tag>());
    }
    return m_id3v2Tags.front().get();
}

/*!
 * \brief Removes a possibly assigned \a tag from the current file.
 *
 * \param tag Specifies the tag to be removed. The tag will not only be detached from the
 *            file, it will be destroyed as well. Might be nullptr (for convenience; eg.
 *            you might want to call file.removeTag(file.mp4Tag()) to ensure no MP4 tag
 *            is present without checking before).
 *
 * To apply the removal and other changings call the applyChanges() method.
 *
 * \sa applyChanges()
 */
void MediaFileInfo::removeTag(Tag *tag)
{
    if(tag) {
        if(m_container) {
            m_container->removeTag(tag);
        }
        if(m_id3v1Tag.get() == tag) {
            m_id3v1Tag.reset();
        }
        for(auto i = m_id3v2Tags.begin(), end = m_id3v2Tags.end(); i != end; ++i) {
            if(i->get() == tag) {
                m_id3v2Tags.erase(i);
                break;
            }
        }
    }
}

/*!
 * \brief Removes all assigned tags from the file.
 *
 * To apply the removal and other changings call the applyChanges() method.
 */
void MediaFileInfo::removeAllTags()
{
    if(m_container) {
        m_container->removeAllTags();
    }
    m_id3v1Tag.reset();
    m_id3v2Tags.clear();
}

/*!
 * \brief Returns an indication whether this library supports parsing the chapters of the current file.
 */
bool MediaFileInfo::areChaptersSupported() const
{
    if(m_container && m_container->chapterCount()) {
        return true;
    }
    switch(m_containerFormat) {
    case ContainerFormat::Matroska:
    case ContainerFormat::Webm:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns an indication whether this library supports attachment format of the current file.
 */
bool MediaFileInfo::areAttachmentsSupported() const
{
    if(m_container && m_container->attachmentCount()) {
        return true;
    }
    switch(m_containerFormat) {
    case ContainerFormat::Matroska:
    case ContainerFormat::Webm:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns an indication whether this library supports parsing the tracks information of the current file.
 */
bool MediaFileInfo::areTracksSupported() const
{
    if(trackCount()) {
        return true;
    }
    switch(m_containerFormat) {
    case ContainerFormat::Mp4:
    case ContainerFormat::MpegAudioFrames:
    case ContainerFormat::RiffWave:
    case ContainerFormat::Ogg:
    case ContainerFormat::Matroska:
        case ContainerFormat::Webm:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns an indication whether this library supports the tag format of the current file.
 */
bool MediaFileInfo::areTagsSupported() const
{
    if(hasAnyTag()) {
        return true;
    }
    switch(m_containerFormat) {
    case ContainerFormat::Mp4:
    case ContainerFormat::MpegAudioFrames:
    case ContainerFormat::Ogg:
    case ContainerFormat::Matroska:
    case ContainerFormat::Webm:
    case ContainerFormat::Adts:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns a pointer to the assigned MP4 tag or nullptr if none is assigned.
 *
 * \remarks The MediaFileInfo keeps the ownership over the returned
 *          pointer. The returned MP4 tag will be destroyed when the
 *          MediaFileInfo gets invalidated.
 */
Mp4Tag *MediaFileInfo::mp4Tag() const
{
    // simply return the first tag here since MP4 files never contain multiple tags
    return m_containerFormat == ContainerFormat::Mp4 && m_container && m_container->tagCount() > 0 ? static_cast<Mp4Container *>(m_container.get())->tags().front().get() : nullptr;
}

/*!
 * \brief Returns pointers to the assigned Matroska tags.
 *
 * \remarks The MediaFileInfo keeps the ownership over the returned
 *          pointers. The returned Matroska tags will be destroyed when the
 *          MediaFileInfo gets invalidated.
 */
const vector<unique_ptr<MatroskaTag> > &MediaFileInfo::matroskaTags() const
{
    // matroska files might contain multiple tags (targeting different scopes)
    if(m_containerFormat == ContainerFormat::Matroska && m_container) {
        return static_cast<MatroskaContainer *>(m_container.get())->tags();
    } else {
        static const std::vector<std::unique_ptr<MatroskaTag> > empty;
        return empty;
    }
}

/*!
 * \brief Returns all chapters assigned to the current file.
 *
 * \remarks The MediaFileInfo keeps the ownership over the chapters which will be
 *          destroyed when the MediaFileInfo gets invalidated.
 */
vector<AbstractChapter *> MediaFileInfo::chapters() const
{
    vector<AbstractChapter *> res;
    if(m_container) {
        size_t count = m_container->chapterCount();
        res.reserve(count);
        for(size_t i = 0; i < count; ++i) {
            res.push_back(m_container->chapter(i));
        }
    }
    return res;
}

/*!
 * \brief Returns all attachments assigned to the current file.
 *
 * \remarks The MediaFileInfo keeps the ownership over the attachments which will be
 *          destroyed when the MediaFileInfo gets invalidated.
 */
vector<AbstractAttachment *> MediaFileInfo::attachments() const
{
    vector<AbstractAttachment *> res;
    if(m_container) {
        size_t count = m_container->attachmentCount();
        res.reserve(count);
        for(size_t i = 0; i < count; ++i) {
            res.push_back(m_container->attachment(i));
        }
    }
    return res;
}

/*!
 * \brief Returns an indication whether at least one related object (track,
 *        tag, container) has notifications.
 */
bool MediaFileInfo::haveRelatedObjectsNotifications() const
{
    if(m_container) {
        if(m_container->hasNotifications()) {
            return true;
        }
    }
    for(const auto *track : tracks()) {
        if(track->hasNotifications()) {
            return true;
        }
    }
    for(const auto *tag : tags()) {
        if(tag->hasNotifications()) {
            return true;
        }
    }
    for(const auto *chapter : chapters()) {
        if(chapter->hasNotifications()) {
            return true;
        }
    }
    return false;
}

/*!
 * \brief Returns the worst notification type including related objects such as track,
 *        tag and container.
 */
NotificationType MediaFileInfo::worstNotificationTypeIncludingRelatedObjects() const
{    
    NotificationType type = worstNotificationType();
    if(type == Notification::worstNotificationType()) {
        return type;
    }
    if(m_container) {
        type |= m_container->worstNotificationType();
    }
    if(type == Notification::worstNotificationType()) {
        return type;
    }
    for(const auto *track : tracks()) {
        type |= track->worstNotificationType();
        if(type == Notification::worstNotificationType()) {
            return type;
        }
    }
    for(const auto *tag : tags()) {
        type |= tag->worstNotificationType();
        if(type == Notification::worstNotificationType()) {
            return type;
        }
    }
    for(const auto *chapter : chapters()) {
        type |= chapter->worstNotificationType();
        if(type == Notification::worstNotificationType()) {
            return type;
        }
    }
    return type;
}

/*!
 * \brief Returns the noficications of the current instance and all related
 *        objects (tracks, tags, container).
 */
NotificationList MediaFileInfo::gatherRelatedNotifications() const
{
    NotificationList notifications(this->notifications());
    if(m_container) {
        notifications.insert(notifications.end(), m_container->notifications().cbegin(), m_container->notifications().cend());
    }
    for(const auto *track : tracks()) {
        notifications.insert(notifications.end(), track->notifications().cbegin(), track->notifications().cend());
    }
    for(const auto *tag : tags()) {
        notifications.insert(notifications.end(), tag->notifications().cbegin(), tag->notifications().cend());
    }
    for(const auto *chapter : chapters()) {
        notifications.insert(notifications.end(), chapter->notifications().cbegin(), chapter->notifications().cend());
    }
    return notifications;
}

/*!
 * \brief Clears all parsing results and assigned/created/changed information such as
 *        container format, tracks, tags, ...
 *
 * This allows a rescan of the file using parsing methods like parseContainerFormat().
 * (These methods do nothing if the information to be parsed has already been gathered.)
 */
void MediaFileInfo::clearParsingResults()
{
    m_containerParsed = false;
    m_containerFormat = ContainerFormat::Unknown;
    m_containerOffset = 0;
    m_paddingSize = 0;
    m_tracksParsed = false;
    m_tagParsed = false;
    m_id3v1Tag.reset();
    m_id3v2Tags.clear();
    m_actualId3v2TagOffsets.clear();
    m_actualExistingId3v1Tag = false;
    m_container.reset();
    m_singleTrack.reset();
}

/*!
 * \brief Merges the assigned ID3v2 tags.
 *
 * Some files I've got contain multiple sucessive ID3v2 tags. If the tags of
 * such an file is parsed by this class, these tags will be kept seperate.
 * This method merges all assigned ID3v2 tags. All fields from the additional
 * ID3v2 tags will be inserted to the first tag. All assigned ID3v2 tag instances
 * except thefirst will be destroyed.
 *
 * A possibly assigned ID3v1 tag remains unaffected.
 *
 * This method does nothing the tags of the current file haven't been parsed using
 * the parseTags() method.
 *
 * \sa id3v2Tags()
 */
void MediaFileInfo::mergeId3v2Tags()
{
    auto begin = m_id3v2Tags.begin(), end = m_id3v2Tags.end();
    if(begin != end) {
        Id3v2Tag &first = **begin;
        auto isecond = begin + 1;
        if(isecond != end) {
            for(auto i = isecond; i != end; ++i) {
                first.insertFields(**i, false);
            }
            m_id3v2Tags.erase(isecond, end - 1);
        }
    }
}

/*!
 * \brief Stores all tags assigned to the current file in the specified vector.
 *
 * Previous elements of the vector will not be cleared.
 *
 * \remarks The MediaFileInfo keeps the ownership over the tags which will be
 *          destroyed when the MediaFileInfo gets invalidated.
 */
void MediaFileInfo::tags(vector<Tag *> &tags) const
{
    if(hasId3v1Tag())
        tags.push_back(m_id3v1Tag.get());
    for(const unique_ptr<Id3v2Tag> &tag : m_id3v2Tags) {
        tags.push_back(tag.get());
    }
    if(m_container) {
        for(size_t i = 0, count = m_container->tagCount(); i < count; ++i) {
            tags.push_back(m_container->tag(i));
        }
    }
}

/*!
 * \brief Returns all tags assigned to the current file.
 *
 * \remarks The MediaFileInfo keeps the ownership over the tags which will be
 *          destroyed when the MediaFileInfo gets invalidated.
 */
vector<Tag *> MediaFileInfo::tags() const
{
    vector<Tag *> res;
    tags(res);
    return res;
}

/*!
 * \brief Reimplemented from BasicFileInfo::invalidated().
 */
void MediaFileInfo::invalidated()
{
    BasicFileInfo::invalidated();
    invalidateStatus();
    invalidateNotifications();
    clearParsingResults();
}

/*!
 * \brief Internally used to chanings of a MP3 file (or theoretically any file) with ID3 tags.
 */
void MediaFileInfo::makeMp3File()
{
    const string context("making MP3 file");
    // there's no need to rewrite the complete file
    if(m_id3v2Tags.size() == 0 && m_actualId3v2TagOffsets.size() == 0) {
        if(m_actualExistingId3v1Tag) { // there is currently an ID3v1 tag at the end of the file
            if(m_id3v1Tag) { // the file shall still have an ID3v1 tag
                updateStatus("No need to rewrite the whole file, just writing ID3v1 tag ...");
                open(); // ensure the file is still open
                stream().seekp(-128, ios_base::end);
                try {
                    m_id3v1Tag->make(stream());
                } catch(Failure &) {
                    addNotification(NotificationType::Warning, "Unable to write ID3v1 tag.", context);
                }
            } else { // the currently existing id3v1 tag shall be removed
                updateStatus("No need to rewrite the whole file, just truncating it to remove ID3v1 tag ...");
                stream().close();
                if(truncate(path().c_str(), size() - 128) == 0) {
                    reportSizeChanged(size() - 128);
                } else {
                    addNotification(NotificationType::Critical, "Unable to truncate file to remove ID3v1 tag.", context);
                    throw ios_base::failure("Unable to truncate file to remove ID3v1 tag.");
                }
            }
        } else { // the doesn't file need to be rewritten
            if(m_id3v1Tag) {
                updateStatus("No need to rewrite the whole file, just writing ID3v1 tag.");
                open(); // ensure the file is still open
                stream().seekp(0, ios_base::end);
                try {
                    m_id3v1Tag->make(stream());
                } catch(Failure &) {
                    addNotification(NotificationType::Warning, "Unable to write ID3v1 tag.", context);
                }
            } else {
                addNotification(NotificationType::Information, "Nothing to be changed.", context);
            }
        }
    } else { // the file needs to be rewritten
        updateStatus("Prepareing for rewriting MP3 file ...");
        close(); // close the file (if its opened)
        string backupPath;
        fstream backupStream;
        try {
            BackupHelper::createBackupFile(path(), backupPath, backupStream);
            backupStream.seekg(m_containerOffset);
            // recreate original file with new/changed ID3 tags
            stream().open(path(), ios_base::out | ios_base::binary | ios_base::trunc);
            updateStatus("Writing ID3v2 tag ...");
            // write id3v2 tags
            int counter = 1;
            for(auto &id3v2Tag : m_id3v2Tags) {
                try {
                    id3v2Tag->make(stream());
                } catch(Failure &) {
                    if(m_id3v2Tags.size()) {
                        addNotification(NotificationType::Warning, "Unable to write " + ConversionUtilities::numberToString(counter) + ". ID3v2 tag.", context);
                    } else {
                        addNotification(NotificationType::Warning, "Unable to write ID3v2 tag.", context);
                    }
                }
                ++counter;
            }
            // recopy backup
            updateStatus("Writing mpeg audio frames ...");
            uint64 remainingBytes = size() - backupStream.tellg(), read;
            if(m_actualExistingId3v1Tag) {
                remainingBytes -= 128;
            }
            const unsigned int bufferSize = 0x4000;
            char buffer[bufferSize];
            while(remainingBytes > 0) {
                if(isAborted()) {
                    throw OperationAbortedException();
                }
                backupStream.read(buffer, remainingBytes > bufferSize ? bufferSize : remainingBytes);
                read = backupStream.gcount();
                stream().write(buffer, read);
                remainingBytes -= read;
                updatePercentage(static_cast<double>(backupStream.tellg()) / static_cast<double>(size()));
            }
            // write id3v1 tag
            updateStatus("Writing ID3v1 tag ...");
            if(m_id3v1Tag) {
                try {
                    m_id3v1Tag->make(stream());
                } catch(Failure &) {
                    addNotification(NotificationType::Warning, "Unable to write ID3v1 tag.", context);
                }
            }
            stream().flush(); // ensure everything has been actually written
            reportSizeChanged(stream().tellp()); // report new size
            close(); // stream is useless for further usage because it is write only
        } catch(OperationAbortedException &) {
            addNotification(NotificationType::Information, "Rewriting file to apply new tag information has been aborted.", context);
            BackupHelper::restoreOriginalFileFromBackupFile(path(), backupPath, stream(), backupStream);
            throw;
        } catch(ios_base::failure &ex) {
            addNotification(NotificationType::Critical, "IO error occured when rewriting file to apply new tag information.\n" + string(ex.what()), context);
            BackupHelper::restoreOriginalFileFromBackupFile(path(), backupPath, stream(), backupStream);
            throw;
        }
    }
}

}
