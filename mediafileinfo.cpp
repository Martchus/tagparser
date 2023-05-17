#include "./mediafileinfo.h"
#include "./abstracttrack.h"
#include "./backuphelper.h"
#include "./diagnostics.h"
#include "./exceptions.h"
#include "./locale.h"
#include "./progressfeedback.h"
#include "./signature.h"
#include "./tag.h"

#include "./id3/id3v1tag.h"
#include "./id3/id3v2tag.h"

#include "./wav/waveaudiostream.h"

#include "./mpegaudio/mpegaudioframestream.h"

#include "./adts/adtsstream.h"

#include "./ivf/ivfstream.h"

#include "./mp4/mp4atom.h"
#include "./mp4/mp4container.h"
#include "./mp4/mp4ids.h"
#include "./mp4/mp4tag.h"
#include "./mp4/mp4track.h"

#include "./matroska/ebmlelement.h"
#include "./matroska/matroskacontainer.h"
#include "./matroska/matroskatag.h"
#include "./matroska/matroskatrack.h"

#include "./ogg/oggcontainer.h"

#include "./flac/flacmetadata.h"
#include "./flac/flacstream.h"

#include <c++utilities/chrono/timespan.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/path.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <ios>
#include <memory>
#include <system_error>

using namespace std;
using namespace std::placeholders;
using namespace CppUtilities;

/*!
 * \namespace IoUtilities
 * \brief Contains utility classes helping to read and write streams.
 */

namespace TagParser {

/// \brief The MediaFileInfoPrivate struct contains private fields of the MediaFileInfo class.
struct MediaFileInfoPrivate {};

/*!
 * \class TagParser::MediaFileInfo
 * \brief The MediaFileInfo class allows to read and write tag information providing
 *        a container/tag format independent interface.
 *
 * It also provides some technical information such as contained streams.
 *
 * For examples see "cli/mainfeatures.cpp" of the tageditor repository.
 */

/*!
 * \brief Constructs a new MediaFileInfo for the specified file.
 *
 * \param path Specifies the absolute or relative path of the file.
 */
MediaFileInfo::MediaFileInfo(std::string &&path)
    : BasicFileInfo(std::move(path))
    , m_containerParsingStatus(ParsingStatus::NotParsedYet)
    , m_containerFormat(ContainerFormat::Unknown)
    , m_containerOffset(0)
    , m_paddingSize(0)
    , m_effectiveSize(0)
    , m_fileStructureFlags(MediaFileStructureFlags::None)
    , m_tracksParsingStatus(ParsingStatus::NotParsedYet)
    , m_tagsParsingStatus(ParsingStatus::NotParsedYet)
    , m_chaptersParsingStatus(ParsingStatus::NotParsedYet)
    , m_attachmentsParsingStatus(ParsingStatus::NotParsedYet)
    , m_minPadding(0)
    , m_maxPadding(0)
    , m_preferredPadding(0)
    , m_tagPosition(ElementPosition::BeforeData)
    , m_indexPosition(ElementPosition::BeforeData)
    , m_fileHandlingFlags(MediaFileHandlingFlags::ForceRewrite | MediaFileHandlingFlags::ForceTagPosition | MediaFileHandlingFlags::ForceIndexPosition
          | MediaFileHandlingFlags::NormalizeKnownTagFieldIds | MediaFileHandlingFlags::PreserveRawTimingValues)
    , m_maxFullParseSize(0x3200000)
{
}

/*!
 * \brief Constructs a new MediaFileInfo.
 */
MediaFileInfo::MediaFileInfo()
    : MediaFileInfo(std::string())
{
}

/*!
 * \brief Constructs a new MediaFileInfo.
 */
MediaFileInfo::MediaFileInfo(std::string_view path)
    : MediaFileInfo(std::string(path))
{
}

/*!
 * \brief Destroys the MediaFileInfo.
 */
MediaFileInfo::~MediaFileInfo()
{
}

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
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \sa isContainerParsed(), parseTracks(), parseTag(), parseChapters(), parseEverything()
 */
void MediaFileInfo::parseContainerFormat(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    // skip if container format already parsed
    if (containerParsingStatus() != ParsingStatus::NotParsedYet) {
        return;
    }

    static const string context("parsing file header");
    open(); // ensure the file is open
    m_containerFormat = ContainerFormat::Unknown;

    // file size
    m_paddingSize = 0;
    m_containerOffset = 0;
    std::size_t bytesSkippedBeforeContainer = 0;
    std::streamoff id3v2Size = 0;

    // read signatrue
    char buff[16];
    const char *const buffEnd = buff + sizeof(buff), *buffOffset;
startParsingSignature:
    if (progress.isAborted()) {
        diag.emplace_back(DiagLevel::Information, "Parsing the container format has been aborted.", context);
        return;
    }
    if (size() - containerOffset() >= 16) {
        stream().seekg(m_containerOffset, ios_base::beg);
        stream().read(buff, sizeof(buff));

        // skip zero/junk bytes
        // notes:
        // - Only skipping 4 or more consecutive zero bytes at this point because some signatures start with up to 4 zero bytes.
        // - It seems that most players/tools¹ skip junk bytes, at least when reading MP3 files. Hence the tagparser library is following
        //   the same approach. (¹e.g. ffmpeg: "[mp3 @ 0x559e1f4cbd80] Skipping 1670 bytes of junk at 1165.")
        std::size_t bytesSkipped = 0;
        for (buffOffset = buff; buffOffset != buffEnd && !(*buffOffset); ++buffOffset, ++bytesSkipped)
            ;
        if (bytesSkipped >= 4) {
        skipJunkBytes:
            m_containerOffset += static_cast<std::streamoff>(bytesSkipped);
            m_paddingSize += bytesSkipped;

            // give up after 0x800 bytes
            if ((bytesSkippedBeforeContainer += bytesSkipped) >= 0x800u) {
                m_containerParsingStatus = ParsingStatus::NotSupported;
                m_containerFormat = ContainerFormat::Unknown;
                m_containerOffset = id3v2Size;
                return;
            }

            // try again
            goto startParsingSignature;
        }

        // parse signature
        switch ((m_containerFormat = parseSignature(buff, sizeof(buff)))) {
        case ContainerFormat::Id3v2Tag:
            // save position of ID3v2 tag
            m_actualId3v2TagOffsets.push_back(m_containerOffset);
            if (m_actualId3v2TagOffsets.size() == 2) {
                diag.emplace_back(DiagLevel::Warning, "There is more than just one ID3v2 header at the beginning of the file.", context);
            }

            // read ID3v2 header
            stream().seekg(m_containerOffset + 5, ios_base::beg);
            stream().read(buff, 5);

            // set the container offset to skip ID3v2 header
            m_containerOffset += toNormalInt(BE::toInt<std::uint32_t>(buff + 1)) + 10;
            if ((*buff) & 0x10) {
                // footer present
                m_containerOffset += 10;
            }
            id3v2Size = m_containerOffset;

            // continue reading signature
            goto startParsingSignature;

        case ContainerFormat::Mp4:
        case ContainerFormat::QuickTime: {
            // MP4/QuickTime is handled using Mp4Container instance
            m_container = make_unique<Mp4Container>(*this, m_containerOffset);
            try {
                static_cast<Mp4Container *>(m_container.get())->validateElementStructure(diag, progress, &m_paddingSize);
            } catch (const OperationAbortedException &) {
                diag.emplace_back(DiagLevel::Information, "Validating the MP4 element structure has been aborted.", context);
            } catch (const Failure &) {
                m_containerParsingStatus = ParsingStatus::CriticalFailure;
            }
            break;
        }
        case ContainerFormat::Ebml: {
            // EBML/Matroska is handled using MatroskaContainer instance
            auto container = make_unique<MatroskaContainer>(*this, m_containerOffset);
            try {
                container->parseHeader(diag, progress);
                if (container->documentType() == "matroska") {
                    m_containerFormat = ContainerFormat::Matroska;
                } else if (container->documentType() == "webm") {
                    m_containerFormat = ContainerFormat::Webm;
                }
                if (isForcingFullParse()) {
                    // validating the element structure of Matroska files takes too long when
                    // parsing big files so do this only when explicitly desired
                    container->validateElementStructure(diag, progress, &m_paddingSize);
                    container->validateIndex(diag, progress);
                }
            } catch (const OperationAbortedException &) {
                diag.emplace_back(DiagLevel::Information, "Validating the Matroska element structure has been aborted.", context);
            } catch (const Failure &) {
                m_containerParsingStatus = ParsingStatus::CriticalFailure;
            }
            m_container = std::move(container);
            break;
        }
        case ContainerFormat::Ogg:
            // Ogg is handled by OggContainer instance
            m_container = make_unique<OggContainer>(*this, m_containerOffset);
            static_cast<OggContainer *>(m_container.get())->setChecksumValidationEnabled(isForcingFullParse());
            break;
        case ContainerFormat::Unknown:
        case ContainerFormat::ApeTag:
            // skip APE tag if the specified size makes sense at all
            if (m_containerFormat == ContainerFormat::ApeTag) {
                if (const auto apeEnd = m_containerOffset + 32 + LE::toUInt32(buff + 12); apeEnd <= static_cast<std::streamoff>(size())) {
                    // take record of APE tag
                    diag.emplace_back(DiagLevel::Critical,
                        argsToString("Found an APE tag at the beginning of the file at offset ", m_containerOffset,
                            ". This tag format is not supported and the tag will therefore be ignored. It will NOT be preserved when saving as "
                            "placing an APE tag at the beginning of a file is strongly unrecommended."),
                        context);
                    // continue reading signature
                    m_containerOffset = apeEnd;
                    goto startParsingSignature;
                }
                m_containerFormat = ContainerFormat::Unknown;
            }

            // check for magic numbers at odd offsets
            // -> check for tar (magic number at offset 0x101)
            if (size() > 0x107) {
                stream().seekg(0x101);
                stream().read(buff, 6);
                if (buff[0] == 0x75 && buff[1] == 0x73 && buff[2] == 0x74 && buff[3] == 0x61 && buff[4] == 0x72 && buff[5] == 0x00) {
                    m_containerFormat = ContainerFormat::Tar;
                    break;
                }
            }
            // skip previously determined zero-bytes or try our luck on the next byte
            if (!bytesSkipped) {
                ++bytesSkipped;
            }
            goto skipJunkBytes;
        default:;
        }
    }

    if (bytesSkippedBeforeContainer) {
        diag.emplace_back(DiagLevel::Warning, argsToString(bytesSkippedBeforeContainer, " bytes of junk skipped"), context);
    }

    // set parsing status
    if (m_containerParsingStatus == ParsingStatus::NotParsedYet) {
        if (m_containerFormat == ContainerFormat::Unknown) {
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
 * hasTracksOfType() will return the parsed information.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \remarks parseContainerFormat() must be called before.
 * \sa areTracksParsed(), parseContainerFormat(), parseTags(), parseChapters(), parseEverything()
 */
void MediaFileInfo::parseTracks(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    // skip if tracks already parsed
    if (tracksParsingStatus() != ParsingStatus::NotParsedYet) {
        return;
    }
    static const string context("parsing tracks");

    try {
        // parse tracks via container object
        if (m_container) {
            m_container->parseTracks(diag, progress);
            m_tracksParsingStatus = ParsingStatus::Ok;
            return;
        }

        // parse tracks via track object for "single-track"-formats
        switch (m_containerFormat) {
        case ContainerFormat::Adts:
            m_singleTrack = make_unique<AdtsStream>(stream(), m_containerOffset);
            break;
        case ContainerFormat::Flac:
            m_singleTrack = make_unique<FlacStream>(*this, m_containerOffset);
            break;
        case ContainerFormat::Ivf:
            m_singleTrack = make_unique<IvfStream>(stream(), m_containerOffset);
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
        if (m_containerFormat != ContainerFormat::Flac) {
            // ensure the effective size has been determined
            // note: This is not required for FLAC and should also be avoided as parseTags() will invoke
            //       parseTracks() when dealing with FLAC files.
            parseTags(diag, progress);
            m_singleTrack->setSize(m_effectiveSize);
        }
        m_singleTrack->parseHeader(diag, progress);

        // take padding for some "single-track" formats into account
        switch (m_containerFormat) {
        case ContainerFormat::Flac:
            m_paddingSize += static_cast<FlacStream *>(m_singleTrack.get())->paddingSize();
            break;
        default:;
        }

        m_tracksParsingStatus = ParsingStatus::Ok;

    } catch (const NotImplementedException &) {
        diag.emplace_back(DiagLevel::Information, "Parsing tracks is not implemented for the container format of the file.", context);
        m_tracksParsingStatus = ParsingStatus::NotSupported;
    } catch (const OperationAbortedException &) {
        diag.emplace_back(DiagLevel::Information, "Parsing tracks has been aborted.", context);
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Critical, "Unable to parse tracks.", context);
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
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \remarks parseContainerFormat() must be called before.
 * \sa isTagParsed(), parseContainerFormat(), parseTracks(), parseChapters(), parseEverything()
 */
void MediaFileInfo::parseTags(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    // skip if tags already parsed
    if (tagsParsingStatus() != ParsingStatus::NotParsedYet) {
        return;
    }
    static const string context("parsing tag");

    // check for ID3v1 tag
    auto effectiveSize = static_cast<std::streamoff>(size());
    if (effectiveSize >= 128) {
        m_id3v1Tag = make_unique<Id3v1Tag>();
        try {
            stream().seekg(effectiveSize - 128, std::ios_base::beg);
            m_id3v1Tag->parse(stream(), diag);
            m_fileStructureFlags += MediaFileStructureFlags::ActualExistingId3v1Tag;
            effectiveSize -= 128;
        } catch (const NoDataFoundException &) {
            m_id3v1Tag.reset();
        } catch (const OperationAbortedException &) {
            diag.emplace_back(DiagLevel::Information, "Parsing ID3v1 tag has been aborted.", context);
            return;
        } catch (const Failure &) {
            m_tagsParsingStatus = ParsingStatus::CriticalFailure;
            diag.emplace_back(DiagLevel::Critical, "Unable to parse ID3v1 tag.", context);
        }
    }

    // check for APE tag at the end of the file (APE tags a the beginning are already covered when parsing the container format)
    if (constexpr auto apeHeaderSize = 32; effectiveSize >= apeHeaderSize) {
        const auto footerOffset = effectiveSize - apeHeaderSize;
        char buffer[apeHeaderSize];
        stream().seekg(footerOffset, std::ios_base::beg);
        stream().read(buffer, sizeof(buffer));
        if (BE::toInt<std::uint64_t>(buffer) == 0x4150455441474558ul /* APETAGEX */) {
            // take record of APE tag
            const auto tagSize = static_cast<std::streamoff>(LE::toInt<std::uint32_t>(buffer + 12));
            const auto flags = LE::toInt<std::uint32_t>(buffer + 20);
            // subtract tag size (footer size and contents) from effective size
            if (tagSize <= effectiveSize) {
                effectiveSize -= tagSize;
            }
            // subtract header size (not included in tag size) from effective size if flags indicate presence of header
            if ((flags & 0x80000000u) && (apeHeaderSize <= effectiveSize)) {
                effectiveSize -= apeHeaderSize;
            }
            diag.emplace_back(DiagLevel::Warning,
                argsToString("Found an APE tag at the end of the file at offset ", (footerOffset - tagSize),
                    ". This tag format is not supported and the tag will therefore be ignored. It will be preserved when saving as-is."),
                context);
        }
    }

    // check for ID3v2 tags: the offsets of the ID3v2 tags have already been parsed when parsing the container format
    m_id3v2Tags.clear();
    for (const auto offset : m_actualId3v2TagOffsets) {
        auto id3v2Tag = make_unique<Id3v2Tag>();
        stream().seekg(offset, ios_base::beg);
        try {
            id3v2Tag->parse(stream(), size() - static_cast<std::uint64_t>(offset), diag);
            m_paddingSize += id3v2Tag->paddingSize();
        } catch (const NoDataFoundException &) {
            continue;
        } catch (const OperationAbortedException &) {
            diag.emplace_back(DiagLevel::Information, "Parsing ID3v2 tags has been aborted.", context);
            return;
        } catch (const Failure &) {
            m_tagsParsingStatus = ParsingStatus::CriticalFailure;
            diag.emplace_back(DiagLevel::Critical, "Unable to parse ID3v2 tag.", context);
        }
        m_id3v2Tags.emplace_back(id3v2Tag.release());
    }

    // compute effective size
    m_effectiveSize = static_cast<std::uint64_t>(effectiveSize - m_containerOffset);

    // check for tags in tracks (FLAC only) or via container object
    try {
        if (m_containerFormat == ContainerFormat::Flac) {
            parseTracks(diag, progress);
            if (m_tagsParsingStatus == ParsingStatus::NotParsedYet) {
                m_tagsParsingStatus = m_tracksParsingStatus;
            }
            return;
        } else if (m_container) {
            m_container->parseTags(diag, progress);
        } else if (m_containerFormat != ContainerFormat::MpegAudioFrames) {
            throw NotImplementedException();
        }

        // set status, but do not override error/unsupported status form ID3 tags here
        if (m_tagsParsingStatus == ParsingStatus::NotParsedYet) {
            m_tagsParsingStatus = ParsingStatus::Ok;
        }

    } catch (const NotImplementedException &) {
        // set status to not supported, but do not override parsing status from ID3 tags here
        if (m_tagsParsingStatus == ParsingStatus::NotParsedYet) {
            m_tagsParsingStatus = ParsingStatus::NotSupported;
        }
        diag.emplace_back(DiagLevel::Information, "Parsing tags is not implemented for the container format of the file.", context);
    } catch (const OperationAbortedException &) {
        diag.emplace_back(DiagLevel::Information, "Parsing tags from container/streams has been aborted.", context);
        return;
    } catch (const Failure &) {
        m_tagsParsingStatus = ParsingStatus::CriticalFailure;
        diag.emplace_back(DiagLevel::Critical, "Unable to parse tag.", context);
    }
}

/*!
 * \brief Parses the chapters of the current file.
 *
 * This method parses the chapters of the current file if not been parsed yet.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \remarks parseContainerFormat() must be called before.
 * \sa areChaptersParsed(), parseContainerFormat(), parseTracks(), parseTags(), parseEverything()
 */
void MediaFileInfo::parseChapters(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    // skip if chapters already parsed
    if (chaptersParsingStatus() != ParsingStatus::NotParsedYet) {
        return;
    }
    static const string context("parsing chapters");

    try {
        // parse chapters via container object
        if (!m_container) {
            throw NotImplementedException();
        }
        m_container->parseChapters(diag, progress);
        m_chaptersParsingStatus = ParsingStatus::Ok;
    } catch (const NotImplementedException &) {
        m_chaptersParsingStatus = ParsingStatus::NotSupported;
        diag.emplace_back(DiagLevel::Information, "Parsing chapters is not implemented for the container format of the file.", context);
    } catch (const Failure &) {
        m_chaptersParsingStatus = ParsingStatus::CriticalFailure;
        diag.emplace_back(DiagLevel::Critical, "Unable to parse chapters.", context);
    }
}

/*!
 * \brief Parses the attachments of the current file.
 *
 * This method parses the attachments of the current file if not been parsed yet.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \remarks parseContainerFormat() must be called before.
 * \sa areChaptersParsed(), parseContainerFormat(), parseTracks(), parseTags(), parseEverything()
 */
void MediaFileInfo::parseAttachments(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    // skip if attachments already parsed
    if (attachmentsParsingStatus() != ParsingStatus::NotParsedYet) {
        return;
    }
    static const string context("parsing attachments");

    try {
        // parse attachments via container object
        if (!m_container) {
            throw NotImplementedException();
        }
        m_container->parseAttachments(diag, progress);
        m_attachmentsParsingStatus = ParsingStatus::Ok;
    } catch (const NotImplementedException &) {
        m_attachmentsParsingStatus = ParsingStatus::NotSupported;
        diag.emplace_back(DiagLevel::Information, "Parsing attachments is not implemented for the container format of the file.", context);
    } catch (const Failure &) {
        m_attachmentsParsingStatus = ParsingStatus::CriticalFailure;
        diag.emplace_back(DiagLevel::Critical, "Unable to parse attachments.", context);
    }
}

/*!
 * \brief Parses the container format, the tracks and the tag information of the current file.
 *
 * See the individual methods to for more details and exceptions which might be thrown.
 * \sa parseContainerFormat(), parseTracks(), parseTags()
 */
void MediaFileInfo::parseEverything(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    parseContainerFormat(diag, progress);
    if (progress.isAborted()) {
        return;
    }
    parseTracks(diag, progress);
    if (progress.isAborted()) {
        return;
    }
    parseTags(diag, progress);
    if (progress.isAborted()) {
        return;
    }
    parseChapters(diag, progress);
    if (progress.isAborted()) {
        return;
    }
    parseAttachments(diag, progress);
}

/*!
 * \brief Ensures appropriate tags are created according the given \a settings.
 * \return Returns whether appropriate tags could be created for the file.
 * \remarks
 *  - Tags must have been parsed before invoking this method (otherwise it will just return false).
 *  - The ID3 related arguments are only practiced when the file format is MP3 or when the file format is unknown and \a treatUnknownFilesAsMp3Files is true.
 *  - Tags might be removed as well. For example the existing ID3v1 tag of an MP3 file will be removed if \a id3v1Usage is set to TagUsage::Never.
 *  - The method might do nothing if present tag(s) already match the given specifications.
 *  - This is only a convenience method. The task could be done by manually using the methods createId3v1Tag(), createId3v2Tag(), removeId3v1Tag() ... as well.
 *  - Some tag information might be discarded. For example when an ID3v2 tag needs to be removed (TagSettings::id3v2usage is set to TagUsage::Never) and an ID3v1 tag will be created instead not all fields can be transferred.
 */
bool MediaFileInfo::createAppropriateTags(const TagCreationSettings &settings)
{
    // check if tags have been parsed yet (tags must have been parsed yet to create appropriate tags)
    if (tagsParsingStatus() == ParsingStatus::NotParsedYet) {
        return false;
    }

    // check if tags need to be created/adjusted/removed
    const auto requiredTargets(settings.requiredTargets);
    const auto flags(settings.flags);
    const auto targetsRequired = !requiredTargets.empty() && (requiredTargets.size() != 1 || !requiredTargets.front().isEmpty());
    auto targetsSupported = false;
    if (areTagsSupported() && m_container) {
        // container object takes care of tag management
        if (targetsRequired) {
            // check whether container supports targets
            if (m_container->tagCount()) {
                // all tags in the container should support targets if the first one supports targets
                targetsSupported = m_container->tag(0)->supportsTarget();
            } else {
                // try to create a new tag and check whether targets are supported
                auto *const tag = m_container->createTag();
                if (tag && (targetsSupported = tag->supportsTarget())) {
                    tag->setTarget(requiredTargets.front());
                }
            }
            if (targetsSupported) {
                for (const auto &target : requiredTargets) {
                    m_container->createTag(target);
                }
            }
        } else {
            // no targets are required -> just ensure that at least one tag is present
            m_container->createTag();
        }
        return true;
    }

    // no container object present
    switch (m_containerFormat) {
    case ContainerFormat::Flac:
        static_cast<FlacStream *>(m_singleTrack.get())->createVorbisComment();
        break;
    default:
        // create ID3 tag(s)
        if (!hasAnyTag() && !(flags & TagCreationFlags::TreatUnknownFilesAsMp3Files)) {
            switch (containerFormat()) {
            case ContainerFormat::Adts:
            case ContainerFormat::Aiff:
            case ContainerFormat::MpegAudioFrames:
            case ContainerFormat::WavPack:
                break;
            default:
                return false;
            }
        }
        // create ID3 tags according to id3v2usage and id3v2usage
        // always create ID3v1 tag -> ensure there is one
        if (settings.id3v1usage == TagUsage::Always && !id3v1Tag()) {
            auto *const id3v1Tag = createId3v1Tag();
            if (flags & TagCreationFlags::Id3InitOnCreate) {
                for (const auto &id3v2Tag : id3v2Tags()) {
                    // overwrite existing values to ensure default ID3v1 genre "Blues" is updated as well
                    id3v1Tag->insertValues(*id3v2Tag, true);
                    // ID3v1 does not support all text encodings which might be used in ID3v2
                    id3v1Tag->ensureTextValuesAreProperlyEncoded();
                }
            }
        }
        if (settings.id3v2usage == TagUsage::Always && !hasId3v2Tag()) {
            // always create ID3v2 tag -> ensure there is one and set version
            auto *const id3v2Tag = createId3v2Tag();
            id3v2Tag->setVersion(settings.id3v2MajorVersion, 0);
            if ((flags & TagCreationFlags::Id3InitOnCreate) && id3v1Tag()) {
                id3v2Tag->insertValues(*id3v1Tag(), true);
            }
        }
    }

    if (flags & TagCreationFlags::MergeMultipleSuccessiveId3v2Tags) {
        mergeId3v2Tags();
    }
    // remove ID3 tags according to settings
    if (settings.id3v1usage == TagUsage::Never && hasId3v1Tag()) {
        // transfer tags to ID3v2 tag before removing
        if ((flags & TagCreationFlags::Id3TransferValuesOnRemoval) && hasId3v2Tag()) {
            id3v2Tags().front()->insertValues(*id3v1Tag(), false);
        }
        removeId3v1Tag();
    }
    if (settings.id3v2usage == TagUsage::Never) {
        if ((flags & TagCreationFlags::Id3TransferValuesOnRemoval) && hasId3v1Tag()) {
            // transfer tags to ID3v1 tag before removing
            for (const auto &tag : id3v2Tags()) {
                id3v1Tag()->insertValues(*tag, false);
            }
        }
        removeAllId3v2Tags();
    } else if (!(flags & TagCreationFlags::KeepExistingId3v2Version)) {
        // set version of ID3v2 tag according user preferences
        for (const auto &tag : id3v2Tags()) {
            tag->setVersion(settings.id3v2MajorVersion, 0);
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
 * \throws Throws TagParser::Failure or a derived exception when a making error occurs.
 *
 * \remarks Tags and tracks need to be parsed without errors before this method can be called.
 *          All previous parsing results are cleared (using clearParsingResults()). Hence
 *          the file must be reparsed. All related objects (tags, tracks, ...) might get invalidated.
 *          This includes notifications of these objects as well.
 *
 * \sa clearParsingResults()
 */
void MediaFileInfo::applyChanges(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    static const string context("making file");
    diag.emplace_back(DiagLevel::Information, "Changes are about to be applied.", context);
    bool previousParsingSuccessful = true;
    switch (tagsParsingStatus()) {
    case ParsingStatus::Ok:
    case ParsingStatus::NotSupported:
        break;
    default:
        previousParsingSuccessful = false;
        diag.emplace_back(DiagLevel::Critical, "Tags have to be parsed without critical errors before changes can be applied.", context);
    }
    switch (tracksParsingStatus()) {
    case ParsingStatus::Ok:
    case ParsingStatus::NotSupported:
        break;
    default:
        previousParsingSuccessful = false;
        diag.emplace_back(DiagLevel::Critical, "Tracks have to be parsed without critical errors before changes can be applied.", context);
    }
    if (!previousParsingSuccessful) {
        throw InvalidDataException();
    }
    if (m_container) { // container object takes care
        // ID3 tags can not be applied in this case -> add warnings if ID3 tags have been assigned
        if (hasId3v1Tag()) {
            diag.emplace_back(DiagLevel::Warning, "Assigned ID3v1 tag can't be attached and will be ignored.", context);
        }
        if (hasId3v2Tag()) {
            diag.emplace_back(DiagLevel::Warning, "Assigned ID3v2 tag can't be attached and will be ignored.", context);
        }
        m_tracksParsingStatus = ParsingStatus::NotParsedYet;
        m_tagsParsingStatus = ParsingStatus::NotParsedYet;
        try {
            m_container->makeFile(diag, progress);
        } catch (...) {
            // since the file might be messed up, invalidate the parsing results
            clearParsingResults();
            throw;
        }
    } else { // implementation if no container object is present
        // assume the file is a MP3 file
        try {
            makeMp3File(diag, progress);
        } catch (...) {
            // since the file might be messed up, invalidate the parsing results
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
string_view MediaFileInfo::containerFormatAbbreviation() const
{
    MediaType mediaType = MediaType::Unknown;
    unsigned int version = 0;
    switch (m_containerFormat) {
    case ContainerFormat::Ogg: {
        // check for video track or whether only Opus or Speex tracks are present
        const auto &tracks = static_cast<OggContainer *>(m_container.get())->tracks();
        if (tracks.empty()) {
            break;
        }
        bool onlyOpus = true, onlySpeex = true;
        for (const auto &track : static_cast<OggContainer *>(m_container.get())->tracks()) {
            if (track->mediaType() == MediaType::Video) {
                mediaType = MediaType::Video;
            }
            if (track->format().general != GeneralMediaFormat::Opus) {
                onlyOpus = false;
            }
            if (track->format().general != GeneralMediaFormat::Speex) {
                onlySpeex = false;
            }
        }
        if (onlyOpus) {
            version = static_cast<unsigned int>(GeneralMediaFormat::Opus);
        } else if (onlySpeex) {
            version = static_cast<unsigned int>(GeneralMediaFormat::Speex);
        }
        break;
    }
    case ContainerFormat::Matroska:
    case ContainerFormat::Mp4:
        mediaType = hasTracksOfType(MediaType::Video) ? MediaType::Video : MediaType::Audio;
        break;
    case ContainerFormat::MpegAudioFrames:
        if (m_singleTrack) {
            version = m_singleTrack->format().sub;
        }
        break;
    default:;
    }
    return TagParser::containerFormatAbbreviation(m_containerFormat, mediaType, version);
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
string_view MediaFileInfo::mimeType() const
{
    MediaType mediaType;
    switch (m_containerFormat) {
    case ContainerFormat::Mp4:
    case ContainerFormat::Ogg:
    case ContainerFormat::Matroska:
        mediaType = hasTracksOfType(MediaType::Video) ? MediaType::Video : MediaType::Audio;
        break;
    default:
        mediaType = MediaType::Unknown;
    }
    return TagParser::containerMimeType(m_containerFormat, mediaType);
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
    size_t trackCount = 0;
    size_t containerTrackCount = 0;
    if (m_singleTrack) {
        trackCount = 1;
    }
    if (m_container) {
        trackCount += (containerTrackCount = m_container->trackCount());
    }
    res.reserve(trackCount);

    if (m_singleTrack) {
        res.push_back(m_singleTrack.get());
    }
    for (size_t i = 0; i != containerTrackCount; ++i) {
        res.push_back(m_container->track(i));
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
    if (tracksParsingStatus() == ParsingStatus::NotParsedYet) {
        return false;
    }
    if (m_singleTrack && m_singleTrack->mediaType() == type) {
        return true;
    } else if (m_container) {
        for (size_t i = 0, count = m_container->trackCount(); i != count; ++i) {
            if (m_container->track(i)->mediaType() == type) {
                return true;
            }
        }
    }
    return false;
}

/*!
 * \brief Returns the overall duration of the file if known; otherwise
 *        returns a TimeSpan with zero ticks.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns false.
 *
 * \sa parseTracks()
 */
CppUtilities::TimeSpan MediaFileInfo::duration() const
{
    if (m_container) {
        return m_container->duration();
    } else if (m_singleTrack) {
        return m_singleTrack->duration();
    }
    return TimeSpan();
}

/*!
 * \brief Returns the overall average bitrate in kbit/s of the file if known; otherwise
 *        returns 0.0.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns false.
 *
 * \sa parseTracks()
 */
double MediaFileInfo::overallAverageBitrate() const
{
    const auto duration = this->duration();
    if (duration.isNull()) {
        return 0.0;
    }
    return 0.0078125 * static_cast<double>(size()) / duration.totalSeconds();
}

/*!
 * \brief Determines the available languages for specified media type (by default MediaType::Audio).
 *
 * If \a type is MediaType::Unknown, all media types are considered.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns an empty set.
 *
 * \sa parseTracks()
 */
unordered_set<string> MediaFileInfo::availableLanguages(MediaType type) const
{
    unordered_set<string> res;
    if (m_container) {
        for (size_t i = 0, count = m_container->trackCount(); i != count; ++i) {
            const AbstractTrack *const track = m_container->track(i);
            if (type != MediaType::Unknown && track->mediaType() != type) {
                continue;
            }
            if (const auto &language = track->locale().someAbbreviatedName(); !language.empty()) {
                res.emplace(language);
            }
        }
    } else if (m_singleTrack && (type == MediaType::Unknown || m_singleTrack->mediaType() == type)) {
        if (const auto &language = m_singleTrack->locale().someAbbreviatedName(); !language.empty()) {
            res.emplace(language);
        }
    }
    return res;
}

/*!
 * \brief Generates a short technical summary about the file's tracks.
 *
 * parseTracks() needs to be called before. Otherwise this
 * method always returns an empty string.
 *
 * Example (exact format might change in the future!):
 * "H.264-720p / HE-AAC-6ch-eng / HE-AAC-2ch-ger / SRT-eng / SRT-ger"
 *
 * \sa parseTracks()
 */
string MediaFileInfo::technicalSummary() const
{
    if (m_container) {
        const size_t trackCount = m_container->trackCount();
        vector<string> parts;
        parts.reserve(trackCount);
        for (size_t i = 0; i != trackCount; ++i) {
            const string description(m_container->track(i)->description());
            if (!description.empty()) {
                parts.emplace_back(std::move(description));
            }
        }
        return joinStrings(parts, " / ");
    } else if (m_singleTrack) {
        return m_singleTrack->description();
    }
    return string();
}

/*!
 * \brief Removes a possibly assigned ID3v1 tag from the current file.
 *
 * To apply the removal and other changings call the applyChanges() method.
 *
 * \returns Returns whether there was an ID3v1 tag assigned which could be removed.
 * \remarks Invalidates the removed tag object (eg. returned via tags() or id3v1Tag()).
 * \sa applyChanges()
 */
bool MediaFileInfo::removeId3v1Tag()
{
    if (tagsParsingStatus() == ParsingStatus::NotParsedYet) {
        return false;
    }
    if (m_id3v1Tag) {
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
    if (tagsParsingStatus() == ParsingStatus::NotParsedYet) {
        return nullptr;
    }
    if (!m_id3v1Tag) {
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
 * \returns Returns whether there the an ID3v2 tag could be removed.
 * \remarks Invalidates all removed tag objects (eg. returned via tags() or id3v2Tags()).
 * \sa applyChanges()
 */
bool MediaFileInfo::removeId3v2Tag(Id3v2Tag *tag)
{
    if (tagsParsingStatus() == ParsingStatus::NotParsedYet) {
        return false;
    }
    for (auto i = m_id3v2Tags.begin(), end = m_id3v2Tags.end(); i != end; ++i) {
        if (i->get() == tag) {
            m_id3v2Tags.erase(i);
            return true;
        }
    }
    return false;
}

/*!
 * \brief Removes all assigned ID3v2 tags from the current file.
 *
 * To apply the removal and other changings call the applyChanges() method
 * \returns Returns whether there where ID3v2 tags assigned which could be removed.
 * \remarks Invalidates all removed tag objects (eg. returned via tags() or id3v2Tags()).
 * \sa applyChanges()
 */
bool MediaFileInfo::removeAllId3v2Tags()
{
    if (tagsParsingStatus() == ParsingStatus::NotParsedYet || m_id3v2Tags.empty()) {
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
    if (m_id3v2Tags.empty()) {
        m_id3v2Tags.emplace_back(make_unique<Id3v2Tag>());
    }
    return m_id3v2Tags.front().get();
}

/*!
 * \brief Removes a possibly assigned \a tag from the current file.
 *
 * To apply the removal and other changings call the applyChanges() method.
 *
 * \param tag Specifies the tag to be removed. The tag will not only be detached from the
 *            file, it will be destroyed as well. Might be nullptr for convenience (eg.
 *            you might want to call file.removeTag(file.mp4Tag()) to ensure no MP4 tag
 *            is present without checking before).
 * \remarks Invalidates the removed tag object if it has been removed.
 *
 * \sa applyChanges()
 */
bool MediaFileInfo::removeTag(Tag *tag)
{
    if (!tag) {
        return false;
    }

    // remove tag via container
    if (m_container) {
        return m_container->removeTag(tag);
    }

    // remove tag via track for "single-track" formats
    if (m_singleTrack && m_containerFormat == ContainerFormat::Flac) {
        auto *const flacStream(static_cast<FlacStream *>(m_singleTrack.get()));
        if (flacStream->vorbisComment() == tag) {
            return flacStream->removeVorbisComment();
        }
    }

    // remove ID3 tags
    if (m_id3v1Tag.get() == tag) {
        m_id3v1Tag.reset();
        return true;
    }
    for (auto i = m_id3v2Tags.begin(), end = m_id3v2Tags.end(); i != end; ++i) {
        if (i->get() == tag) {
            m_id3v2Tags.erase(i);
            return true;
        }
    }
    return false;
}

/*!
 * \brief Removes all assigned tags from the file.
 * \remarks Invalidates all removed tag objects (eg. returned via tags()).
 *
 * To apply the removal and other changings call the applyChanges() method.
 */
void MediaFileInfo::removeAllTags()
{
    if (m_container) {
        m_container->removeAllTags();
    }
    if (m_singleTrack && m_containerFormat == ContainerFormat::Flac) {
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
    if (m_container && m_container->chapterCount()) {
        return true;
    }
    switch (m_containerFormat) {
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
    if (m_container && m_container->attachmentCount()) {
        return true;
    }
    switch (m_containerFormat) {
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
    if (trackCount()) {
        return true;
    }
    switch (m_containerFormat) {
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
    switch (m_containerFormat) {
    case ContainerFormat::Adts:
    case ContainerFormat::Flac:
    case ContainerFormat::Matroska:
    case ContainerFormat::MpegAudioFrames:
    case ContainerFormat::Mp4:
    case ContainerFormat::Ogg:
    case ContainerFormat::WavPack:
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
    return (m_containerFormat == ContainerFormat::Mp4 || m_containerFormat == ContainerFormat::QuickTime) && m_container
            && m_container->tagCount() > 0
        ? static_cast<Mp4Container *>(m_container.get())->tags().front().get()
        : nullptr;
}

/*!
 * \brief Returns pointers to the assigned Matroska tags.
 * \remarks The MediaFileInfo keeps the ownership over the returned
 *          pointers. The returned Matroska tags will be destroyed when the
 *          MediaFileInfo is invalidated.
 */
const vector<unique_ptr<MatroskaTag>> &MediaFileInfo::matroskaTags() const
{
    // matroska files might contain multiple tags (targeting different scopes)
    if (m_containerFormat == ContainerFormat::Matroska && m_container) {
        return static_cast<MatroskaContainer *>(m_container.get())->tags();
    } else {
        static const std::vector<std::unique_ptr<MatroskaTag>> empty;
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
        : (m_containerFormat == ContainerFormat::Flac && m_singleTrack ? static_cast<FlacStream *>(m_singleTrack.get())->vorbisComment() : nullptr);
}

/*!
 * \brief Returns all chapters assigned to the current file.
 * \remarks The MediaFileInfo keeps the ownership over the object which will be destroyed when the
 *          MediaFileInfo is invalidated.
 */
vector<AbstractChapter *> MediaFileInfo::chapters() const
{
    vector<AbstractChapter *> res;
    if (m_container) {
        const size_t count = m_container->chapterCount();
        res.reserve(count);
        for (size_t i = 0; i != count; ++i) {
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
    if (m_container) {
        const size_t count = m_container->attachmentCount();
        res.reserve(count);
        for (size_t i = 0; i != count; ++i) {
            res.push_back(m_container->attachment(i));
        }
    }
    return res;
}

/*!
 * \brief Clears all parsing results and assigned/created/changed information such as
 *        detected container format, tracks, tags, ...
 *
 * This allows a rescan of the file using parsing methods like parseContainerFormat().
 * Otherwise, these methods do nothing if the information to be parsed has already been
 * gathered.
 *
 * \remarks Any pointers previously returned by tags(), tracks(), ... object should be
 *          considered invalidated.
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
    m_fileStructureFlags = MediaFileStructureFlags::None;
    m_container.reset();
    m_singleTrack.reset();
}

/*!
 * \brief Merges the assigned ID3v2 tags into a single ID3v2 tag.
 *
 * Some files I've got contain multiple successive ID3v2 tags. If the tags of
 * such an file is parsed by this class, these tags will be kept separate.
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
    if (begin == end) {
        return;
    }
    Id3v2Tag &first = **begin;
    auto isecond = begin + 1;
    if (isecond == end) {
        return;
    }
    for (auto i = isecond; i != end; ++i) {
        first.insertFields(**i, false);
    }
    m_id3v2Tags.erase(isecond, end);
}

/*!
 * \brief Converts an existing ID3v1 tag into an ID3v2 tag.
 *
 * Effectively merges all ID3 tags into a single ID3v2 tag.
 *
 * \remarks Does nothing if
 * - there is no ID3v1 tag assigned.
 * - the file format isn't known to support ID3 tags (unless there is an ID3 tag present).
 * - the tags of the current file haven't been parsed using the parseTags() method.
 */
bool MediaFileInfo::id3v1ToId3v2()
{
    if (tagsParsingStatus() == ParsingStatus::NotParsedYet || !areTagsSupported() || !hasId3v1Tag()) {
        return false;
    }
    return createAppropriateTags(TagCreationSettings{
        {}, TagCreationFlags::MergeMultipleSuccessiveId3v2Tags | TagCreationFlags::KeepExistingId3v2Version, TagUsage::Never, TagUsage::Always, 3 });
}

/*!
 * \brief Converts the existing ID3v2 tags into an ID3v1 tag.
 *
 * Effectively merges all ID3 tags into a single ID3v1 tag.
 *
 * \remarks Does nothing if
 * - there is not at least one ID3v2 tag assigned.
 * - the file format isn't known to support ID3 tags (unless there is an ID3 tag present).
 * - the tags of the current file haven't been parsed using the parseTags() method.
 */
bool MediaFileInfo::id3v2ToId3v1()
{
    if (tagsParsingStatus() == ParsingStatus::NotParsedYet || !areTagsSupported() || !hasId3v2Tag()) {
        return false;
    }
    return createAppropriateTags(TagCreationSettings{
        {}, TagCreationFlags::MergeMultipleSuccessiveId3v2Tags | TagCreationFlags::KeepExistingId3v2Version, TagUsage::Always, TagUsage::Never, 3 });
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
    switch (m_containerFormat) {
    case ContainerFormat::Ogg:
        if (m_container) {
            return static_cast<OggContainer *>(m_container.get())->createTag(TagTarget());
        }
        break;
    case ContainerFormat::Flac:
        if (m_singleTrack) {
            return static_cast<FlacStream *>(m_singleTrack.get())->createVorbisComment();
        }
        break;
    default:;
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
    switch (m_containerFormat) {
    case ContainerFormat::Ogg:
        if (m_container) {
            bool hadTags = static_cast<OggContainer *>(m_container.get())->tagCount();
            static_cast<OggContainer *>(m_container.get())->removeAllTags();
            return hadTags;
        }
        break;
    case ContainerFormat::Flac:
        if (m_singleTrack) {
            return static_cast<FlacStream *>(m_singleTrack.get())->removeVorbisComment();
        }
        break;
    default:;
    }
    return false;
}

/*!
 * \brief Stores all tags assigned to the current file in the specified vector.
 * \remarks
 * - Previous elements of the vector will not be cleared.
 * - Includes tags which have only been assigned, e.g. via createAppropriateTags(), even if
 *   those tags have not been stored to disk yet via applyChanges().
 * - The MediaFileInfo keeps the ownership over the tags which will be
 *   destroyed when the MediaFileInfo is invalidated.
 */
void MediaFileInfo::tags(std::vector<Tag *> &tags) const
{
    if (hasId3v1Tag()) {
        tags.push_back(m_id3v1Tag.get());
    }
    for (const unique_ptr<Id3v2Tag> &tag : m_id3v2Tags) {
        tags.push_back(tag.get());
    }
    if (m_containerFormat == ContainerFormat::Flac && m_singleTrack) {
        if (auto *const vorbisComment = static_cast<const FlacStream *>(m_singleTrack.get())->vorbisComment()) {
            tags.push_back(vorbisComment);
        }
    }
    if (m_container) {
        for (size_t i = 0, count = m_container->tagCount(); i < count; ++i) {
            tags.push_back(m_container->tag(i));
        }
    }
}

/*!
 * \brief Returns all tags assigned to the current file.
 * \remarks
 * - Includes tags which have only been assigned, e.g. via createAppropriateTags(), even if
 *   those tags have not been stored to disk yet via applyChanges().
 * - The MediaFileInfo keeps the ownership over the tags which will be
 *   destroyed when the MediaFileInfo is invalidated.
 */
vector<Tag *> MediaFileInfo::tags() const
{
    auto res = vector<Tag *>();
    tags(res);
    return res;
}

/*!
 * \brief Returns an indication whether a tag of any format is assigned.
 * \remarks
 * - Includes tags which have only been assigned, e.g. via createAppropriateTags(), even if
 *   those tags have not been stored to disk yet via applyChanges().
 */
bool MediaFileInfo::hasAnyTag() const
{
    return hasId3v1Tag() || hasId3v2Tag() || (m_container && m_container->tagCount())
        || (m_containerFormat == ContainerFormat::Flac && static_cast<FlacStream *>(m_singleTrack.get())->vorbisComment());
}

/*!
 * \brief Returns all tags parsed from the current file.
 * \remarks
 * - Previous elements of the vector will not be cleared.
 * - Does **not** include tags which have been assigned, e.g. via createAppropriateTags() but
 *   have not been stored to disk yet via applyChanges().
 * - The MediaFileInfo keeps the ownership over the tags which will be
 *   destroyed when the MediaFileInfo is invalidated.
 */
void MediaFileInfo::parsedTags(std::vector<Tag *> &tags) const
{
    if (hasId3v1Tag() && m_id3v1Tag->size()) {
        tags.push_back(m_id3v1Tag.get());
    }
    for (const unique_ptr<Id3v2Tag> &tag : m_id3v2Tags) {
        if (tag->size()) {
            tags.push_back(tag.get());
        }
    }
    if (m_containerFormat == ContainerFormat::Flac && m_singleTrack) {
        if (auto *const vorbisComment = static_cast<const FlacStream *>(m_singleTrack.get())->vorbisComment()) {
            if (vorbisComment->size()) {
                tags.push_back(vorbisComment);
            }
        }
    }
    if (m_container) {
        for (size_t i = 0, count = m_container->tagCount(); i < count; ++i) {
            if (auto *const tag = m_container->tag(i); tag->size()) {
                tags.push_back(tag);
            }
        }
    }
}

/*!
 * \brief Returns all tags parsed from the current file.
 * \remarks
 * - Does **not** include tags which have been assigned, e.g. via createAppropriateTags() but
 *   have not been stored to disk yet via applyChanges().
 * - The MediaFileInfo keeps the ownership over the tags which will be
 *   destroyed when the MediaFileInfo is invalidated.
 */
std::vector<Tag *> MediaFileInfo::parsedTags() const
{
    auto res = vector<Tag *>();
    parsedTags(res);
    return res;
}

/*!
 * \brief Reimplemented from BasicFileInfo::invalidated().
 */
void MediaFileInfo::invalidated()
{
    BasicFileInfo::invalidated();
    clearParsingResults();
}

/*!
 * \brief Internally used to save chanings of MP3/FLAC files and any other files which might have ID3 tags.
 */
void MediaFileInfo::makeMp3File(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    static const string context("making MP3/FLAC file");

    // don't rewrite the complete file if there are no ID3v2/FLAC tags present or to be written
    if (!isForcingRewrite() && m_id3v2Tags.empty() && m_actualId3v2TagOffsets.empty() && m_saveFilePath.empty()
        && m_containerFormat != ContainerFormat::Flac) {
        // alter ID3v1 tag
        if (!m_id3v1Tag) {
            // remove ID3v1 tag
            if (!(m_fileStructureFlags & MediaFileStructureFlags::ActualExistingId3v1Tag)) {
                diag.emplace_back(DiagLevel::Information, "Nothing to be changed.", context);
                return;
            }
            progress.updateStep("Removing ID3v1 tag ...");
            stream().close();
            auto ec = std::error_code();
            std::filesystem::resize_file(makeNativePath(BasicFileInfo::pathForOpen(path())), size() - 128, ec);
            if (!ec) {
                reportSizeChanged(size() - 128);
            } else {
                diag.emplace_back(DiagLevel::Critical, "Unable to truncate file to remove ID3v1 tag: " + ec.message(), context);
                throw std::ios_base::failure("Unable to truncate file to remove ID3v1 tag.");
            }
            return;
        } else {
            // add or update ID3v1 tag
            if (m_fileStructureFlags & MediaFileStructureFlags::ActualExistingId3v1Tag) {
                progress.updateStep("Updating existing ID3v1 tag ...");
                // ensure the file is still open / not readonly
                open();
                stream().seekp(-128, ios_base::end);
                try {
                    m_id3v1Tag->make(stream(), diag);
                } catch (const Failure &) {
                    diag.emplace_back(DiagLevel::Warning, "Unable to write ID3v1 tag.", context);
                }
            } else {
                progress.updateStep("Adding new ID3v1 tag ...");
                // ensure the file is still open / not readonly
                open();
                stream().seekp(0, ios_base::end);
                try {
                    m_id3v1Tag->make(stream(), diag);
                } catch (const Failure &) {
                    diag.emplace_back(DiagLevel::Warning, "Unable to write ID3v1 tag.", context);
                }
            }
            // prevent deferring final write operations (to catch and handle possible errors here)
            stream().flush();
        }
        return;
    }

    // ID3v2 needs to be modified
    FlacStream *const flacStream = (m_containerFormat == ContainerFormat::Flac ? static_cast<FlacStream *>(m_singleTrack.get()) : nullptr);
    progress.updateStep(flacStream ? "Updating FLAC tags ..." : "Updating ID3v2 tags ...");

    // prepare ID3v2 tags
    vector<Id3v2TagMaker> makers;
    makers.reserve(m_id3v2Tags.size());
    std::uint64_t tagsSize = 0;
    for (auto &tag : m_id3v2Tags) {
        try {
            makers.emplace_back(tag->prepareMaking(diag));
            tagsSize += makers.back().requiredSize();
        } catch (const Failure &) {
        }
    }

    // determine stream offset and make track/format specific metadata
    std::uint32_t streamOffset; // where the actual stream starts
    stringstream flacMetaData(ios_base::in | ios_base::out | ios_base::binary);
    flacMetaData.exceptions(ios_base::badbit | ios_base::failbit);
    std::streamoff startOfLastMetaDataBlock;
    if (flacStream) {
        // if it is a raw FLAC stream, make FLAC metadata
        startOfLastMetaDataBlock = flacStream->makeHeader(flacMetaData, diag);
        tagsSize += static_cast<std::uint64_t>(flacMetaData.tellp());
        streamOffset = flacStream->streamOffset();
    } else {
        // make no further metadata, just use the container offset as stream offset
        streamOffset = static_cast<std::uint32_t>(m_containerOffset);
    }

    // check whether rewrite is required
    bool rewriteRequired = isForcingRewrite() || !m_saveFilePath.empty() || (tagsSize > streamOffset);
    size_t padding = 0;
    if (!rewriteRequired) {
        // rewriting is not forced and new tag is not too big for available space
        // -> calculate new padding
        padding = streamOffset - tagsSize;
        // -> check whether the new padding matches specifications
        if (padding < minPadding() || padding > maxPadding()) {
            rewriteRequired = true;
        }
    }
    if (makers.empty() && !flacStream) {
        // an ID3v2 tag is not written and it is not a FLAC stream
        // -> can't include padding
        if (padding) {
            // but padding would be present -> need to rewrite
            padding = 0; // can't write the preferred padding despite rewriting
            rewriteRequired = true;
        }
    } else if (rewriteRequired) {
        // rewriting is forced or new ID3v2 tag is too big for available space
        // -> use preferred padding when rewriting anyways
        padding = preferredPadding();
    } else if (makers.empty() && flacStream && padding && padding < 4) {
        // no ID3v2 tag -> must include padding in FLAC stream
        // but padding of 1, 2, and 3 byte isn't possible -> need to rewrite
        padding = preferredPadding();
        rewriteRequired = true;
    }
    if (rewriteRequired && flacStream && makers.empty() && padding) {
        // the first 4 byte of FLAC padding actually don't count because these
        // can not be used for additional meta data
        padding += 4;
    }
    progress.updateStep(rewriteRequired ? "Preparing streams for rewriting ..." : "Preparing streams for updating ...");

    // setup stream(s) for writing
    // -> define variables needed to handle output stream and backup stream (required when rewriting the file)
    string originalPath = path(), backupPath;
    NativeFileStream &outputStream = stream();
    NativeFileStream backupStream; // create a stream to open the backup/original file for the case rewriting the file is required

    if (rewriteRequired) {
        if (m_saveFilePath.empty()) {
            // move current file to temp dir and reopen it as backupStream, recreate original file
            try {
                BackupHelper::createBackupFileCanonical(backupDirectory(), originalPath, backupPath, outputStream, backupStream);
                // recreate original file, define buffer variables
                outputStream.open(originalPath, ios_base::out | ios_base::binary | ios_base::trunc);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(
                    DiagLevel::Critical, argsToString("Creation of temporary file (to rewrite the original file) failed: ", failure.what()), context);
                throw;
            }
        } else {
            // open the current file as backupStream and create a new outputStream at the specified "save file path"
            try {
                close();
                backupStream.exceptions(ios_base::badbit | ios_base::failbit);
                backupStream.open(BasicFileInfo::pathForOpen(path()).data(), ios_base::in | ios_base::binary);
                outputStream.open(BasicFileInfo::pathForOpen(m_saveFilePath).data(), ios_base::out | ios_base::binary | ios_base::trunc);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical, argsToString("Opening streams to write output file failed: ", failure.what()), context);
                throw;
            }
        }

    } else { // !rewriteRequired
        // reopen original file to ensure it is opened for writing
        try {
            close();
            outputStream.open(BasicFileInfo::pathForOpen(path()).data(), ios_base::in | ios_base::out | ios_base::binary);
        } catch (const std::ios_base::failure &failure) {
            diag.emplace_back(DiagLevel::Critical, argsToString("Opening the file with write permissions failed: ", failure.what()), context);
            throw;
        }
    }
    // TODO: fix code duplication

    // start actual writing
    try {
        // ensure we can cast padding safely to uint32
        if (padding > numeric_limits<std::uint32_t>::max()) {
            padding = numeric_limits<std::uint32_t>::max();
            diag.emplace_back(
                DiagLevel::Critical, argsToString("Preferred padding is not supported. Setting preferred padding to ", padding, '.'), context);
        }

        if (!makers.empty()) {
            // write ID3v2 tags
            progress.updateStep("Writing ID3v2 tag ...");
            for (auto i = makers.begin(), end = makers.end() - 1; i != end; ++i) {
                i->make(outputStream, 0, diag);
            }
            // include padding into the last ID3v2 tag
            makers.back().make(outputStream, (flacStream && padding && padding < 4) ? 0 : static_cast<std::uint32_t>(padding), diag);
        }

        if (flacStream) {
            if (padding && startOfLastMetaDataBlock) {
                // if appending padding, ensure the last flag of the last "METADATA_BLOCK_HEADER" is not set
                flacMetaData.seekg(startOfLastMetaDataBlock);
                flacMetaData.seekp(startOfLastMetaDataBlock);
                flacMetaData.put(static_cast<std::uint8_t>(flacMetaData.peek()) & (0x80u - 1));
                flacMetaData.seekg(0);
            }

            // write FLAC metadata
            outputStream << flacMetaData.rdbuf();

            // write padding
            if (padding) {
                flacStream->makePadding(outputStream, static_cast<std::uint32_t>(padding), true, diag);
            }
        }

        if (makers.empty() && !flacStream) {
            // just write padding (however, padding should be set to 0 in this case?)
            for (; padding; --padding) {
                outputStream.put(0);
            }
        }

        // copy / skip actual stream data
        // -> determine media data size
        std::uint64_t mediaDataSize = size() - streamOffset;
        if (m_fileStructureFlags & MediaFileStructureFlags::ActualExistingId3v1Tag) {
            mediaDataSize -= 128;
        }

        if (rewriteRequired) {
            // copy data from original file
            switch (m_containerFormat) {
            case ContainerFormat::MpegAudioFrames:
                progress.updateStep("Writing MPEG audio frames ...");
                break;
            default:
                progress.updateStep("Writing data ...");
            }
            backupStream.seekg(static_cast<streamoff>(streamOffset));
            CopyHelper<0x4000> copyHelper;
            copyHelper.callbackCopy(backupStream, stream(), mediaDataSize, std::bind(&AbortableProgressFeedback::isAborted, std::ref(progress)),
                std::bind(&AbortableProgressFeedback::updateStepPercentage, std::ref(progress), _1));
        } else {
            // just skip actual stream data
            outputStream.seekp(static_cast<std::streamoff>(mediaDataSize), ios_base::cur);
        }

        // write ID3v1 tag
        if (m_id3v1Tag) {
            progress.updateStep("Writing ID3v1 tag ...");
            try {
                m_id3v1Tag->make(stream(), diag);
            } catch (const Failure &) {
                diag.emplace_back(DiagLevel::Warning, "Unable to write ID3v1 tag.", context);
            }
        }

        // handle streams
        if (rewriteRequired) {
            // report new size
            reportSizeChanged(static_cast<std::uint64_t>(outputStream.tellp()));
            // "save as path" is now the regular path
            if (!saveFilePath().empty()) {
                reportPathChanged(saveFilePath());
                m_saveFilePath.clear();
            }
            // prevent deferring final write operations (to catch and handle possible errors here); stream is useless for further
            // usage anyways because it is write-only
            outputStream.close();
        } else {
            const auto newSize = static_cast<std::uint64_t>(outputStream.tellp());
            if (newSize < size()) {
                // file is smaller after the modification -> truncate
                // -> prevent deferring final write operations
                outputStream.close();
                // -> truncate file
                auto ec = std::error_code();
                std::filesystem::resize_file(makeNativePath(BasicFileInfo::pathForOpen(path())), newSize, ec);
                if (!ec) {
                    reportSizeChanged(newSize);
                } else {
                    diag.emplace_back(DiagLevel::Critical, "Unable to truncate the file: " + ec.message(), context);
                }
            } else {
                // file is longer after the modification
                // -> prevent deferring final write operations (to catch and handle possible errors here)
                outputStream.flush();
                // -> report new size
                reportSizeChanged(newSize);
            }
        }

    } catch (...) {
        BackupHelper::handleFailureAfterFileModifiedCanonical(*this, originalPath, backupPath, outputStream, backupStream, diag, context);
    }
}

} // namespace TagParser
