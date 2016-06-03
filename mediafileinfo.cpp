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

#include "./flac/flacstream.h"
#include "./flac/flacmetadata.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/misc/memory.h>

#include <unistd.h>

#include <cstdio>
#include <algorithm>
#include <iomanip>
#include <ios>
#include <system_error>
#include <functional>

using namespace std;
using namespace std::placeholders;
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
 * \brief The MediaFileInfo class allows to read and write tag information providing
 *        a container/tag format independent interface.
 *
 * It also provides some technical information such as contained streams.
 *
 * For examples see "cli/mainfeatures.cpp" of the tageditor repository.
 */

/*!
 * \brief Constructs a new MediaFileInfo.
 */
MediaFileInfo::MediaFileInfo() :
    m_containerParsingStatus(ParsingStatus::NotParsedYet),
    m_containerFormat(ContainerFormat::Unknown),
    m_containerOffset(0),
    m_actualExistingId3v1Tag(false),
    m_tracksParsingStatus(ParsingStatus::NotParsedYet),
    m_tagsParsingStatus(ParsingStatus::NotParsedYet),
    m_chaptersParsingStatus(ParsingStatus::NotParsedYet),
    m_attachmentsParsingStatus(ParsingStatus::NotParsedYet),
    m_forceFullParse(MEDIAINFO_CPP_FORCE_FULL_PARSE),
    m_forceRewrite(true),
    m_minPadding(0),
    m_maxPadding(0),
    m_preferredPadding(0),
    m_tagPosition(ElementPosition::BeforeData),
    m_forceTagPosition(true),
    m_indexPosition(ElementPosition::BeforeData),
    m_forceIndexPosition(true)
{}

/*!
 * \brief Constructs a new MediaFileInfo for the specified file.
 *
 * \param path Specifies the absolute or relative path of the file.
 */
MediaFileInfo::MediaFileInfo(const string &path) :
    BasicFileInfo(path),
    m_containerParsingStatus(ParsingStatus::NotParsedYet),
    m_containerFormat(ContainerFormat::Unknown),
    m_containerOffset(0),
    m_actualExistingId3v1Tag(false),
    m_tracksParsingStatus(ParsingStatus::NotParsedYet),
    m_tagsParsingStatus(ParsingStatus::NotParsedYet),
    m_chaptersParsingStatus(ParsingStatus::NotParsedYet),
    m_attachmentsParsingStatus(ParsingStatus::NotParsedYet),
    m_forceFullParse(MEDIAINFO_CPP_FORCE_FULL_PARSE),
    m_forceRewrite(true),
    m_minPadding(0),
    m_maxPadding(0),
    m_preferredPadding(0),
    m_tagPosition(ElementPosition::BeforeData),
    m_forceTagPosition(true),
    m_indexPosition(ElementPosition::BeforeData),
    m_forceIndexPosition(true)
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
    if(containerParsingStatus() != ParsingStatus::NotParsedYet) {
        // there's no need to read the container format twice
        return;
    }

    invalidateStatus();
    static const string context("parsing file header");
    open(); // ensure the file is open
    m_containerFormat = ContainerFormat::Unknown;

    // file size
    m_paddingSize = 0;
    m_containerOffset = 0;

    // read signatrue
    char buff[16];
    const char *const buffEnd = buff + sizeof(buff), *buffOffset;
startParsingSignature:
    if(size() - m_containerOffset >= 16) {
        stream().seekg(m_containerOffset, ios_base::beg);
        stream().read(buff, sizeof(buff));

        // skip zero bytes/padding
        size_t bytesSkipped = 0;
        for(buffOffset = buff; buffOffset != buffEnd && !(*buffOffset); ++buffOffset, ++bytesSkipped);
        if(bytesSkipped >= 4) {
            m_containerOffset += bytesSkipped;

            // give up after 0x100 bytes
            if((m_paddingSize += bytesSkipped) >= 0x100u) {
                m_containerParsingStatus = ParsingStatus::NotSupported;
                m_containerFormat = ContainerFormat::Unknown;
                return;
            }

            // try again
            goto startParsingSignature;
        }
        if(m_paddingSize) {
            addNotification(NotificationType::Warning, numberToString(m_paddingSize) + " zero-bytes skipped at the beginning of the file.", context);
        }

        // parse signature
        switch((m_containerFormat = parseSignature(buff, sizeof(buff)))) {
        case ContainerFormat::Id2v2Tag:
            // save position of ID3v2 tag
            m_actualId3v2TagOffsets.push_back(m_containerOffset);
            if(m_actualId3v2TagOffsets.size() == 2) {
                addNotification(NotificationType::Warning, "There is more then just one ID3v2 header at the beginning of the file.", context);
            }

            // read ID3v2 header
            stream().seekg(m_containerOffset + 5, ios_base::beg);
            stream().read(buff, 5);

            // set the container offset to skip ID3v2 header
            m_containerOffset += toNormalInt(BE::toUInt32(buff + 1)) + 10;
            if((*buff) & 0x10) {
                // footer present
                m_containerOffset += 10;
            }

            // continue reading signature
            goto startParsingSignature;

        case ContainerFormat::Mp4:
        case ContainerFormat::QuickTime: {
            // MP4/QuickTime is handled using Mp4Container instance
            m_container = make_unique<Mp4Container>(*this, m_containerOffset);
            NotificationList notifications;
            try {
                static_cast<Mp4Container *>(m_container.get())->validateElementStructure(notifications, &m_paddingSize);
            } catch(const Failure &) {
                m_containerParsingStatus = ParsingStatus::CriticalFailure;
            }
            addNotifications(notifications);
            break;

        } case ContainerFormat::Ebml: {
            // EBML/Matroska is handled using MatroskaContainer instance
            auto container = make_unique<MatroskaContainer>(*this, m_containerOffset);
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
            } catch(const Failure &) {
                m_containerParsingStatus = ParsingStatus::CriticalFailure;
            }
            m_container = move(container);
            addNotifications(notifications);
            break;
        } case ContainerFormat::Ogg:
            // Ogg is handled using OggContainer instance
            m_container = make_unique<OggContainer>(*this, m_containerOffset);
            static_cast<OggContainer *>(m_container.get())->setChecksumValidationEnabled(m_forceFullParse);
            break;
        case ContainerFormat::Unknown:
            // container format is still unknown -> check for magic numbers at odd offsets
            // -> check for tar (magic number at offset 0x101)
            if(size() > 0x107) {
                stream().seekg(0x101);
                stream().read(buff, 6);
                if(buff[0] == 0x75 && buff[1] == 0x73 && buff[2] == 0x74 && buff[3] == 0x61 && buff[4] == 0x72 && buff[5] == 0x00) {
                    m_containerFormat = ContainerFormat::Tar;
                    break;
                }
            }
        default:
            ;
        }
    }

    // set parsing status
    if(m_containerParsingStatus == ParsingStatus::NotParsedYet) {
        if(m_containerFormat == ContainerFormat::Unknown) {
            m_containerParsingStatus = ParsingStatus::NotSupported;
        } else {
            m_containerParsingStatus = ParsingStatus::Ok;
        }
    }
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
    if(tracksParsingStatus() != ParsingStatus::NotParsedYet) { // there's no need to read the tracks twice
        return;
    }
    parseContainerFormat(); // ensure the container format has been load yet
    static const string context("parsing tracks");
    try {
        if(m_container) {
            m_container->parseTracks();
        } else {
            switch(m_containerFormat) {
            case ContainerFormat::Adts:
                m_singleTrack = make_unique<AdtsStream>(stream(), m_containerOffset);
                break;
            case ContainerFormat::Flac:
                m_singleTrack = make_unique<FlacStream>(*this, m_containerOffset);
                break;
            case ContainerFormat::MpegAudioFrames:
                m_singleTrack = make_unique<MpegAudioFrameStream>(stream(), m_containerOffset);
                break;
            case ContainerFormat::RiffWave:
                m_singleTrack = make_unique<WaveAudioStream>(stream(), m_containerOffset);
                break;
            default:
                throw NotImplementedException();
            }
            m_singleTrack->parseHeader();

            switch(m_containerFormat) {
            case ContainerFormat::Flac:
                // FLAC streams might container padding
                m_paddingSize += static_cast<FlacStream *>(m_singleTrack.get())->paddingSize();
                break;
            default:
                ;
            }
        }
        m_tracksParsingStatus = ParsingStatus::Ok;
    } catch(const NotImplementedException &) {
        addNotification(NotificationType::Information, "Parsing tracks is not implemented for the container format of the file.", context);
        m_tracksParsingStatus = ParsingStatus::NotSupported;
    } catch(const Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse tracks.", context);
        m_tracksParsingStatus = ParsingStatus::CriticalFailure;
    }
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
    if(tagsParsingStatus() != ParsingStatus::NotParsedYet) { // there's no need to read the tags twice
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
        } catch(const NoDataFoundException &) {
            m_id3v1Tag.reset(); // no ID3v1 tag found
        } catch(const Failure &) {
            m_tagsParsingStatus = ParsingStatus::CriticalFailure;
            addNotification(NotificationType::Critical, "Unable to parse ID3v1 tag.", context);
        }
    }
    // the offsets of the ID3v2 tags have already been parsed when parsing the container format
    m_id3v2Tags.clear();
    for(const auto offset : m_actualId3v2TagOffsets) {
        auto id3v2Tag = make_unique<Id3v2Tag>();
        stream().seekg(offset, ios_base::beg);
        try {
            id3v2Tag->parse(stream(), size() - offset);
            m_paddingSize += id3v2Tag->paddingSize();
        } catch(const NoDataFoundException &) {
            continue;
        } catch(const Failure &) {
            m_tagsParsingStatus = ParsingStatus::CriticalFailure;
            addNotification(NotificationType::Critical, "Unable to parse ID3v2 tag.", context);
        }
        m_id3v2Tags.emplace_back(id3v2Tag.release());
    }
    if(m_container) {
        try {
            m_container->parseTags();
        } catch(const NotImplementedException &) {
            if(m_tagsParsingStatus == ParsingStatus::NotParsedYet) {
                // do not override parsing status from ID3 tags here
                m_tagsParsingStatus = ParsingStatus::NotSupported;
            }
            addNotification(NotificationType::Information, "Parsing tags is not implemented for the container format of the file.", context);
        } catch(const Failure &) {
            m_tagsParsingStatus = ParsingStatus::CriticalFailure;
            addNotification(NotificationType::Critical, "Unable to parse tag.", context);
        }
    }
    if(m_tagsParsingStatus == ParsingStatus::NotParsedYet) {
        // do not override error status here
        m_tagsParsingStatus = ParsingStatus::Ok;
    }
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
    if(chaptersParsingStatus() != ParsingStatus::NotParsedYet) { // there's no need to read the chapters twice
        return;
    }
    static const string context("parsing chapters");
    try {
        if(m_container) {
            m_container->parseChapters();
            m_chaptersParsingStatus = ParsingStatus::Ok;
        } else {
            throw NotImplementedException();
        }
    } catch (NotImplementedException &) {
        m_chaptersParsingStatus = ParsingStatus::NotSupported;
        addNotification(NotificationType::Information, "Parsing chapters is not implemented for the container format of the file.", context);
    } catch (Failure &) {
        m_chaptersParsingStatus = ParsingStatus::CriticalFailure;
        addNotification(NotificationType::Critical, "Unable to parse chapters.", context);
    }
}

void MediaFileInfo::parseAttachments()
{
    if(attachmentsParsingStatus() != ParsingStatus::NotParsedYet) { // there's no need to read the attachments twice
        return;
    }
    static const string context("parsing attachments");
    try {
        if(m_container) {
            m_container->parseAttachments();
            m_attachmentsParsingStatus = ParsingStatus::Ok;
        } else {
            throw NotImplementedException();
        }
    } catch (NotImplementedException &) {
        m_attachmentsParsingStatus = ParsingStatus::NotSupported;
        addNotification(NotificationType::Information, "Parsing attachments is not implemented for the container format of the file.", context);
    } catch (Failure &) {
        m_attachmentsParsingStatus = ParsingStatus::CriticalFailure;
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
 * \param requiredTargets Specifies the required targets. Targets are ignored if not supported by the container.
 * \return Returns an indication whether appropriate tags could be created for the file.
 * \remarks
 *  - The ID3 related arguments are only practiced when the file format is MP3 or when the file format
 *    is unknown and \a treatUnknownFilesAsMp3Files is true. These arguments are ignored when creating
 *    tags for other known file formats such as MP4.
 *  - Tags might be removed as well. For example the existing ID3v1 tag of an MP3 file will be removed
 *    if \a id3v1Usage is set to TagUsage::Never.
 *  - The method might do nothing if the file already has appropriate tags.
 *  - This is only a convenience method. The task can be done by manually using the methods createId3v1Tag(),
 *    createId3v2Tag(), removeId3v1Tag() ... as well.
 *  - Some tag information might be discarded. For example when an ID3v2 tag needs to be removed (\a id3v2usage is set to TagUsage::Never)
 *    and an ID3v1 tag will be created instead not all fields can be transfered.
 */
bool MediaFileInfo::createAppropriateTags(bool treatUnknownFilesAsMp3Files, TagUsage id3v1usage, TagUsage id3v2usage, bool mergeMultipleSuccessiveId3v2Tags, bool keepExistingId3v2version, uint32 id3v2version, const std::vector<TagTarget> &requiredTargets)
{
    // check if tags need to be created/adjusted/removed
    bool targetsRequired = !requiredTargets.empty() && (requiredTargets.size() != 1 || !requiredTargets.front().isEmpty());
    bool targetsSupported = false;
    if(areTagsSupported() && m_container) {
        // container object takes care of tag management
        if(targetsRequired) {
            // check whether container supports targets
            if(m_container->tagCount()) {
                // all tags in the container should support targets if the first one supports targets
                targetsSupported = m_container->tag(0)->supportsTarget();
            } else {
                // try to create a new tag and check whether targets are supported
                auto *tag = m_container->createTag();
                if(tag) {
                    if((targetsSupported = tag->supportsTarget())) {
                        tag->setTarget(requiredTargets.front());
                    }
                }
            }
            if(targetsSupported) {
                for(const auto &target : requiredTargets) {
                    m_container->createTag(target);
                }
            }
        } else {
            // no targets are required -> just ensure that at least one tag is present
            m_container->createTag();
        }
    } else {
        // no container object present
        if(m_containerFormat == ContainerFormat::Flac) {
            // creation of Vorbis comment is possible
            static_cast<FlacStream *>(m_singleTrack.get())->createVorbisComment();
        } else {
            // creation of ID3 tag is possible
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
                for(const auto &tag : id3v2Tags()) {
                    id3v1Tag()->insertValues(*tag, false);
                }
            }
            removeAllId3v2Tags();
        } else if(!keepExistingId3v2version) {
            // set version of ID3v2 tag according user preferences
            for(const auto &tag : id3v2Tags()) {
                tag->setVersion(id3v2version, 0);
            }
        }
    }
    if(targetsRequired && !targetsSupported) {
        addNotification(NotificationType::Warning, "The container/tags do not support targets. The specified targets are ignored.", "creating tags");
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
 * \remarks Tags and tracks need to be parsed without errors before this method can be called.
 *          All previous parsing results are cleared (using clearParsingResults()). Hence
 *          the file must be reparsed. All related objects (tags, tracks, ...) might get invalidated.
 *          This includes notifications of these objects as well.
 *
 * \sa clearParsingResults()
 */
void MediaFileInfo::applyChanges()
{   
    const string context("making file");
    addNotification(NotificationType::Information, "Changes are about to be applied.", context);
    bool previousParsingSuccessful = true;
    switch(tagsParsingStatus()) {
    case ParsingStatus::Ok:
    case ParsingStatus::NotSupported:
        break;
    default:
        previousParsingSuccessful = false;
        addNotification(NotificationType::Critical, "Tags have to be parsed without critical errors before changes can be applied.", context);
    }
    switch(tracksParsingStatus()) {
    case ParsingStatus::Ok:
    case ParsingStatus::NotSupported:
        break;
    default:
        previousParsingSuccessful = false;
        addNotification(NotificationType::Critical, "Tracks have to be parsed without critical errors before changes can be applied.", context);
    }
    if(!previousParsingSuccessful) {
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
        m_tracksParsingStatus = ParsingStatus::NotParsedYet;
        m_tagsParsingStatus = ParsingStatus::NotParsedYet;
        try {
            m_container->makeFile();
            addNotifications(*m_container);
        } catch(...) {
            addNotifications(*m_container);
            clearParsingResults();
            throw;
        }
    } else { // implementation if no container object is present
        // assume the file is a MP3 file
        try {
            makeMp3File();
        } catch(...) {
            clearParsingResults();
            throw;
        }
    }
    clearParsingResults();
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
        // check whether only Opus tracks are present
        version = static_cast<unsigned int>(GeneralMediaFormat::Opus);
        for(const auto &track : static_cast<OggContainer *>(m_container.get())->tracks()) {
            if(track->format().general != GeneralMediaFormat::Opus) {
                version = 0;
                break;
            }
        }
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
    MediaType mediaType;
    switch(m_containerFormat) {
    case ContainerFormat::Mp4:
    case ContainerFormat::Ogg:
    case ContainerFormat::Matroska:
        mediaType = hasTracksOfType(MediaType::Video) ? MediaType::Video : MediaType::Audio;
        break;
    default:
        mediaType = MediaType::Unknown;
    }
    return Media::containerMimeType(m_containerFormat, mediaType);
}

/*!
 * \brief Returns the tracks for the current file.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns an empty vector.
 *
 * \remarks The MediaFileInfo keeps the ownership over the returned
 *          pointers. The returned Tracks will be destroyed when the
 *          MediaFileInfo is invalidated.
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
    if(tracksParsingStatus() != ParsingStatus::NotParsedYet) {
        if(m_singleTrack && m_singleTrack->mediaType() == type) {
            return true;
        } else if(m_container) {
            for(size_t i = 0, count = m_container->trackCount(); i < count; ++i) {
                if(m_container->track(i)->mediaType() == type) {
                    return true;
                }
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
    if(tagsParsingStatus() != ParsingStatus::NotParsedYet) {
        if(m_id3v1Tag) {
            m_id3v1Tag.reset();
            return true;
        }
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
    if(tagsParsingStatus() == ParsingStatus::NotParsedYet) {
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
    if(tagsParsingStatus() != ParsingStatus::NotParsedYet) {
        for(auto i = m_id3v2Tags.begin(), end = m_id3v2Tags.end(); i != end; ++i) {
            if(i->get() == tag) {
                m_id3v2Tags.erase(i);
                return true;
            }
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
 * \sa applyChanges()
 */
bool MediaFileInfo::removeAllId3v2Tags()
{
    if(tagsParsingStatus() == ParsingStatus::NotParsedYet || m_id3v2Tags.empty()) {
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
 * \remarks The MediaFileInfo keeps the ownership over the created tag. It will be
 *          destroyed when the MediaFileInfo is invalidated.
 * \sa applyChanges()
 */
Id3v2Tag *MediaFileInfo::createId3v2Tag()
{
    if(m_id3v2Tags.empty()) {
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
    if(m_singleTrack && m_containerFormat == ContainerFormat::Flac) {
        static_cast<FlacStream *>(m_singleTrack.get())->removeVorbisComment();
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
    switch(m_containerFormat) {
    case ContainerFormat::Adts:
    case ContainerFormat::Flac:
    case ContainerFormat::Matroska:
    case ContainerFormat::MpegAudioFrames:
    case ContainerFormat::Mp4:
    case ContainerFormat::Ogg:
    case ContainerFormat::Webm:
        // these container formats are supported
        return true;
    default:
        // the container format is unsupported
        // -> an ID3 tag might be already present, in this case the tags are considered supported
        return !m_container && (hasId3v1Tag() || hasId3v2Tag());
    }
}

/*!
 * \brief Returns a pointer to the assigned MP4 tag or nullptr if none is assigned.
 * \remarks The MediaFileInfo keeps the ownership over the object which will be destroyed when the
 *          MediaFileInfo is invalidated.
 */
Mp4Tag *MediaFileInfo::mp4Tag() const
{
    // simply return the first tag here since MP4 files never contain multiple tags
    return (m_containerFormat == ContainerFormat::Mp4 || m_containerFormat == ContainerFormat::QuickTime) && m_container && m_container->tagCount() > 0 ? static_cast<Mp4Container *>(m_container.get())->tags().front().get() : nullptr;
}

/*!
 * \brief Returns pointers to the assigned Matroska tags.
 * \remarks The MediaFileInfo keeps the ownership over the returned
 *          pointers. The returned Matroska tags will be destroyed when the
 *          MediaFileInfo is invalidated.
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
 * \brief Returns a pointer to the first assigned Vorbis comment or nullptr if none is assigned.
 * \remarks The MediaFileInfo keeps the ownership over the object which will be destroyed when the
 *          MediaFileInfo is invalidated.
 */
VorbisComment *MediaFileInfo::vorbisComment() const
{
    return m_containerFormat == ContainerFormat::Ogg && m_container && m_container->tagCount()
            ? static_cast<OggContainer *>(m_container.get())->tags().front().get()
            : (m_containerFormat == ContainerFormat::Flac && m_singleTrack
               ? static_cast<FlacStream *>(m_singleTrack.get())->vorbisComment()
               : nullptr);
}

/*!
 * \brief Returns all chapters assigned to the current file.
 * \remarks The MediaFileInfo keeps the ownership over the object which will be destroyed when the
 *          MediaFileInfo is invalidated.
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
 * \remarks The MediaFileInfo keeps the ownership over the object which will be destroyed when the
 *          MediaFileInfo is invalidated.
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
 * \brief Returns the notifications of the current instance and all related
 *        objects (tracks, tags, container, ...).
 * \remarks The specified list is not cleared before notifications are added.
 */
void MediaFileInfo::gatherRelatedNotifications(NotificationList &notifications) const
{
    notifications.insert(notifications.end(), this->notifications().cbegin(), this->notifications().cend());
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
    for(const auto *attachment : attachments()) {
        notifications.insert(notifications.end(), attachment->notifications().cbegin(), attachment->notifications().cend());
    }
}

/*!
 * \brief Returns the notifications of the current instance and all related
 *        objects (tracks, tags, container, ...).
 */
NotificationList MediaFileInfo::gatherRelatedNotifications() const
{
    NotificationList notifications;
    gatherRelatedNotifications(notifications);
    return notifications;
}

/*!
 * \brief Clears all parsing results and assigned/created/changed information such as
 *        container format, tracks, tags, ...
 *
 * This allows a rescan of the file using parsing methods like parseContainerFormat().
 * (These methods do nothing if the information to be parsed has already been gathered.)
 *
 * \remarks Any pointers previously returned by tags(), tracks(), ... object are invalidated.
 */
void MediaFileInfo::clearParsingResults()
{
    m_containerParsingStatus = ParsingStatus::NotParsedYet;
    m_containerFormat = ContainerFormat::Unknown;
    m_containerOffset = 0;
    m_paddingSize = 0;
    m_tracksParsingStatus = ParsingStatus::NotParsedYet;
    m_tagsParsingStatus = ParsingStatus::NotParsedYet;
    m_chaptersParsingStatus = ParsingStatus::NotParsedYet;
    m_attachmentsParsingStatus = ParsingStatus::NotParsedYet;
    m_id3v1Tag.reset();
    m_id3v2Tags.clear();
    m_actualId3v2TagOffsets.clear();
    m_actualExistingId3v1Tag = false;
    m_container.reset();
    m_singleTrack.reset();
}

/*!
 * \brief Merges the assigned ID3v2 tags into a single ID3v2 tag.
 *
 * Some files I've got contain multiple successive ID3v2 tags. If the tags of
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
 * \brief Converts an existing ID3v1 tag into an ID3v2 tag.
 *
 * Effectively merges all ID3 tags into a single ID3v2 tag.
 *
 * Does nothing if there are currently no ID3 tags assigned and the file format
 * isn't known to support ID3 tags.
 *
 * This method does nothing the tags of the current file haven't been parsed using
 * the parseTags() method.
 */
bool MediaFileInfo::id3v1ToId3v2()
{
    if(!areTagsSupported() || !m_container) {
        return createAppropriateTags(false, TagUsage::Never, TagUsage::Always, true, true, 3);
    } else {
        return false;
    }
}

/*!
 * \brief Converts the existing ID3v2 tags into an ID3v1 tag.
 *
 * Effectively merges all ID3 tags into a single ID3v1 tag.
 *
 * Does nothing if there are currently no ID3 tags assigned and the file format
 * isn't known to support ID3 tags.
 *
 * This method does nothing the tags of the current file haven't been parsed using
 * the parseTags() method.
 */
bool MediaFileInfo::id3v2ToId3v1()
{
    if(!areTagsSupported() || !m_container) {
        return createAppropriateTags(false, TagUsage::Always, TagUsage::Never, true, true, 3);
    } else {
        return false;
    }
}

/*!
 * \brief Creates a Vorbis comment for the current file.
 *
 * This method does nothing if the tags/tracks of the current file haven't been parsed using
 * the parseTags() and parseTracks() methods.
 *
 * If the file has already a Vorbis comment no new tag will be created.
 *
 * To apply the created tag and other changings call the applyChanges() method.
 *
 * \returns Returns the Vorbis comment or nullptr if creation is not possible.
 *
 * \sa applyChanges()
 */
VorbisComment *MediaFileInfo::createVorbisComment()
{
    switch(m_containerFormat) {
    case ContainerFormat::Ogg:
        if(m_container) {
            return static_cast<OggContainer *>(m_container.get())->createTag(TagTarget());
        }
        break;
    case ContainerFormat::Flac:
        if(m_singleTrack) {
            return static_cast<FlacStream *>(m_singleTrack.get())->createVorbisComment();
        }
        break;
    default:
        ;
    }
    return nullptr;
}

/*!
 * \brief Removes all assigned Vorbis comment from the current file.
 *
 * To apply the removal and other changings call the applyChanges() method.
 *
 * \returns Returns whether there was an Vorbis comment assigned which could be removed.
 *
 * \sa applyChanges()
 */
bool MediaFileInfo::removeVorbisComment()
{
    switch(m_containerFormat) {
    case ContainerFormat::Ogg:
        if(m_container) {
            bool hadTags = static_cast<OggContainer *>(m_container.get())->tagCount();
            static_cast<OggContainer *>(m_container.get())->removeAllTags();
            return hadTags;
        }
        break;
    case ContainerFormat::Flac:
        if(m_singleTrack) {
            return static_cast<FlacStream *>(m_singleTrack.get())->removeVorbisComment();
        }
        break;
    default:
        ;
    }
    return false;
}

/*!
 * \brief Stores all tags assigned to the current file in the specified vector.
 *
 * Previous elements of the vector will not be cleared.
 *
 * \remarks The MediaFileInfo keeps the ownership over the tags which will be
 *          destroyed when the MediaFileInfo is invalidated.
 */
void MediaFileInfo::tags(vector<Tag *> &tags) const
{
    if(hasId3v1Tag()) {
        tags.push_back(m_id3v1Tag.get());
    }
    for(const unique_ptr<Id3v2Tag> &tag : m_id3v2Tags) {
        tags.push_back(tag.get());
    }
    if(m_containerFormat == ContainerFormat::Flac && m_singleTrack) {
        if(auto *vorbisComment = static_cast<FlacStream *>(m_singleTrack.get())->vorbisComment()) {
            tags.push_back(vorbisComment);
        }
    }
    if(m_container) {
        for(size_t i = 0, count = m_container->tagCount(); i < count; ++i) {
            tags.push_back(m_container->tag(i));
        }
    }
}

/*!
 * \brief Returns an indication whether a tag of any format is assigned.
 */
bool MediaFileInfo::hasAnyTag() const
{
    return hasId3v1Tag()
            || hasId3v2Tag()
            || (m_container && m_container->tagCount())
            || (m_containerFormat == ContainerFormat::Flac && static_cast<FlacStream *>(m_singleTrack.get())->vorbisComment());
}

/*!
 * \brief Returns all tags assigned to the current file.
 *
 * \remarks The MediaFileInfo keeps the ownership over the tags which will be
 *          destroyed when the MediaFileInfo is invalidated.
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
    static const string context("making MP3 file");
    // there's no need to rewrite the complete file if there are no ID3v2 tags present or to be written
    if(!isForcingRewrite() && m_id3v2Tags.empty() && m_actualId3v2TagOffsets.empty() && m_saveFilePath.empty() && m_containerFormat != ContainerFormat::Flac) {
        if(m_actualExistingId3v1Tag) {
            // there is currently an ID3v1 tag at the end of the file
            if(m_id3v1Tag) {
                // the file shall still have an ID3v1 tag
                updateStatus("Updating ID3v1 tag ...");
                // ensure the file is still open / not readonly
                open();
                stream().seekp(-128, ios_base::end);
                try {
                    m_id3v1Tag->make(stream());
                } catch(const Failure &) {
                    addNotification(NotificationType::Warning, "Unable to write ID3v1 tag.", context);
                }
            } else {
                // the currently existing ID3v1 tag shall be removed
                updateStatus("Removing ID3v1 tag ...");
                stream().close();
                if(truncate(path().c_str(), size() - 128) == 0) {
                    reportSizeChanged(size() - 128);
                } else {
                    addNotification(NotificationType::Critical, "Unable to truncate file to remove ID3v1 tag.", context);
                    throw ios_base::failure("Unable to truncate file to remove ID3v1 tag.");
                }
            }

        } else {
            // there is currently no ID3v1 tag at the end of the file
            if(m_id3v1Tag) {
                updateStatus("Adding ID3v1 tag ...");
                // ensure the file is still open / not readonly
                open();
                stream().seekp(0, ios_base::end);
                try {
                    m_id3v1Tag->make(stream());
                } catch(const Failure &) {
                    addNotification(NotificationType::Warning, "Unable to write ID3v1 tag.", context);
                }
            } else {
                addNotification(NotificationType::Information, "Nothing to be changed.", context);
            }
        }

    } else {
        // ID3v2 needs to be modified
        updateStatus("Updating ID3v2 tags ...");

        // prepare ID3v2 tags
        vector<Id3v2TagMaker> makers;
        makers.reserve(m_id3v2Tags.size());
        uint32 tagsSize = 0;
        for(auto &tag : m_id3v2Tags) {
            try {
                makers.emplace_back(tag->prepareMaking());
                tagsSize += makers.back().requiredSize();
            } catch(const Failure &) {
                // nothing to do: notifications added anyways
            }
            addNotifications(*tag);
        }

        // check whether it is a raw FLAC stream
        FlacStream *flacStream = (m_containerFormat == ContainerFormat::Flac ? static_cast<FlacStream *>(m_singleTrack.get()) : nullptr);
        uint32 streamOffset; // where the actual stream starts
        stringstream flacMetaData(ios_base::in | ios_base::out | ios_base::binary);
        flacMetaData.exceptions(ios_base::badbit | ios_base::failbit);
        uint32 startOfLastMetaDataBlock;

        if(flacStream) {
            // if it is a raw FLAC stream, make FLAC metadata
            startOfLastMetaDataBlock = flacStream->makeHeader(flacMetaData);
            tagsSize += flacMetaData.tellp();
            streamOffset = flacStream->streamOffset();
        } else {
            // make no further metadata, just use the container offset as stream offset
            streamOffset = static_cast<uint32>(m_containerOffset);
        }

        // check whether rewrite is required
        bool rewriteRequired = isForcingRewrite() || !m_saveFilePath.empty() || (tagsSize > streamOffset);
        uint32 padding = 0;
        if(!rewriteRequired) {
            // rewriting is not forced and new tag is not too big for available space
            // -> calculate new padding
            padding = streamOffset - tagsSize;
            // -> check whether the new padding matches specifications
            if(padding < minPadding() || padding > maxPadding()) {
                rewriteRequired = true;
            }
        }
        if(makers.empty() && !flacStream) {
            // an ID3v2 tag is not written and it is not a FLAC stream
            // -> can't include padding
            if(padding) {
                // but padding would be present -> need to rewrite
                padding = 0; // can't write the preferred padding despite rewriting
                rewriteRequired = true;
            }
        } else if(rewriteRequired) {
            // rewriting is forced or new ID3v2 tag is too big for available space
            // -> use preferred padding when rewriting anyways
            padding = preferredPadding();
        } else if(makers.empty() && flacStream && padding && padding < 4) {
            // no ID3v2 tag -> must include padding in FLAC stream
            // but padding of 1, 2, and 3 byte isn't possible -> need to rewrite
            padding = preferredPadding();
            rewriteRequired = true;
        }
        if(rewriteRequired && flacStream && makers.empty() && padding) {
            // the first 4 byte of FLAC padding actually don't count because these
            // can not be used for additional meta data
            padding += 4;
        }
        updateStatus(rewriteRequired ? "Preparing streams for rewriting ..." : "Preparing streams for updating ...");

        // setup stream(s) for writing
        // -> define variables needed to handle output stream and backup stream (required when rewriting the file)
        string backupPath;
        fstream &outputStream = stream();
        fstream backupStream; // create a stream to open the backup/original file for the case rewriting the file is required

        if(rewriteRequired) {
            if(m_saveFilePath.empty()) {
                // move current file to temp dir and reopen it as backupStream, recreate original file
                try {
                    BackupHelper::createBackupFile(path(), backupPath, outputStream, backupStream);
                    // recreate original file, define buffer variables
                    outputStream.open(path(), ios_base::out | ios_base::binary | ios_base::trunc);
                } catch(const ios_base::failure &) {
                    addNotification(NotificationType::Critical, "Creation of temporary file (to rewrite the original file) failed.", context);
                    throw;
                }
            } else {
                // open the current file as backupStream and create a new outputStream at the specified "save file path"
                try {
                    backupStream.exceptions(ios_base::badbit | ios_base::failbit);
                    backupStream.open(path(), ios_base::in | ios_base::binary);
                    outputStream.open(m_saveFilePath, ios_base::out | ios_base::binary | ios_base::trunc);
                } catch(const ios_base::failure &) {
                    addNotification(NotificationType::Critical, "Opening streams to write output file failed.", context);
                    throw;
                }
            }

        } else { // !rewriteRequired
            // reopen original file to ensure it is opened for writing
            try {
                close();
                outputStream.open(path(), ios_base::in | ios_base::out | ios_base::binary);
            } catch(const ios_base::failure &) {
                addNotification(NotificationType::Critical, "Opening the file with write permissions failed.", context);
                throw;
            }
        }

        // start actual writing
        try {
            if(!makers.empty()) {
                // write ID3v2 tags
                updateStatus("Writing ID3v2 tag ...");
                for(auto i = makers.begin(), end = makers.end() - 1; i != end; ++i) {
                    i->make(outputStream, 0);
                }
                // include padding into the last ID3v2 tag
                makers.back().make(outputStream, (flacStream && padding && padding < 4) ? 0 : padding);
            }

            if(flacStream) {
                if(padding && startOfLastMetaDataBlock) {
                    // if appending padding, ensure the last flag of the last "METADATA_BLOCK_HEADER" is not set
                    flacMetaData.seekg(startOfLastMetaDataBlock);
                    flacMetaData.seekp(startOfLastMetaDataBlock);
                    flacMetaData.put(static_cast<byte>(flacMetaData.peek()) & (0x80u - 1));
                    flacMetaData.seekg(0);
                }

                // write FLAC metadata
                outputStream << flacMetaData.rdbuf();

                // write padding
                if(padding) {
                    flacStream->makePadding(outputStream, padding, true);
                }
            }

            if(makers.empty() && !flacStream){
                // just write padding (however, padding should be set to 0 in this case?)
                for(; padding; --padding) {
                    outputStream.put(0);
                }
            }

            // copy / skip actual stream data
            // -> determine media data size
            uint64 mediaDataSize = size() - streamOffset;
            if(m_actualExistingId3v1Tag) {
                mediaDataSize -= 128;
            }

            if(rewriteRequired) {
                // copy data from original file
                switch(m_containerFormat) {
                case ContainerFormat::MpegAudioFrames:
                    updateStatus("Writing MPEG audio frames ...");
                    break;
                default:
                    updateStatus("Writing frames ...");
                }
                backupStream.seekg(streamOffset);
                CopyHelper<0x4000> copyHelper;
                copyHelper.callbackCopy(backupStream, stream(), mediaDataSize, bind(&StatusProvider::isAborted, this), bind(&StatusProvider::updatePercentage, this, _1));
                updatePercentage(100.0);
            } else {
                // just skip actual stream data
                outputStream.seekp(mediaDataSize, ios_base::cur);
            }

            // write ID3v1 tag
            if(m_id3v1Tag) {
                updateStatus("Writing ID3v1 tag ...");
                try {
                    m_id3v1Tag->make(stream());
                } catch(const Failure &) {
                    addNotification(NotificationType::Warning, "Unable to write ID3v1 tag.", context);
                }
            }

            // handle streams
            if(rewriteRequired) {
                // report new size
                reportSizeChanged(outputStream.tellp());
                // "save as path" is now the regular path
                if(!saveFilePath().empty()) {
                    reportPathChanged(saveFilePath());
                    m_saveFilePath.clear();
                }
                // stream is useless for further usage because it is write-only
                outputStream.close();
            } else {
                const auto newSize = static_cast<uint64>(outputStream.tellp());
                if(newSize < size()) {
                    // file is smaller after the modification -> truncate
                    // -> close stream before truncating
                    outputStream.close();
                    // -> truncate file
                    if(truncate(path().c_str(), newSize) == 0) {
                        reportSizeChanged(newSize);
                    } else {
                        addNotification(NotificationType::Critical, "Unable to truncate the file.", context);
                    }
                } else {
                    // file is longer after the modification -> just report new size
                    reportSizeChanged(newSize);
                }
            }

        } catch(...) {
            BackupHelper::handleFailureAfterFileModified(*this, backupPath, outputStream, backupStream, context);
        }
    }
}

}
