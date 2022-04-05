#include "./matroskatrack.h"
#include "./ebmlelement.h"
#include "./matroskacontainer.h"
#include "./matroskaid.h"
#include "./matroskatag.h"

#include "../avi/bitmapinfoheader.h"

#include "../wav/waveaudiostream.h"

#include "../avc/avcconfiguration.h"

#include "../mp4/mp4ids.h"
#include "../mp4/mp4track.h"

#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/conversion/stringconversion.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::MatroskaTrack
 * \brief Implementation of TagParser::AbstractTrack for the Matroska container.
 */

/*!
 * \brief Constructs a new track for the specified \a trackElement.
 *
 * Each track element (ID: MatroskaId::TrackEntry) holds header information
 * for one track in the Matroska file.
 */
MatroskaTrack::MatroskaTrack(EbmlElement &trackElement)
    : AbstractTrack(trackElement.stream(), trackElement.startOffset())
    , m_trackElement(&trackElement)
{
}

/*!
 * \brief Destroys the track.
 */
MatroskaTrack::~MatroskaTrack()
{
}

TrackType MatroskaTrack::type() const
{
    return TrackType::MatroskaTrack;
}

/*!
 * \brief Returns the MediaFormat for the specified Matroska codec ID.
 * \todo Use an std::unordered_map here.
 */
MediaFormat MatroskaTrack::codecIdToMediaFormat(const string &codecId)
{
    auto parts = splitString<vector<string>>(codecId, "/", EmptyPartsTreat::Keep, 3);
    parts.resize(3);
    const auto &part1 = parts[0], &part2 = parts[1], &part3 = parts[2];
    MediaFormat fmt;
    if (part1 == "V_MS" && part2 == "VFW" && part3 == "FOURCC") {
        fmt.general = GeneralMediaFormat::MicrosoftVideoCodecManager;
    } else if (part1 == "V_UNCOMPRESSED") {
        fmt.general = GeneralMediaFormat::UncompressedVideoFrames;
    } else if (part1 == "V_MPEG4") {
        fmt.general = GeneralMediaFormat::Mpeg4Video;
        if (part2 == "ISO") {
            if (part3 == "SP") {
                fmt.sub = SubFormats::Mpeg4SimpleProfile1;
            } else if (part3 == "ASP") {
                fmt.sub = SubFormats::Mpeg4AdvancedSimpleProfile1;
            } else if (part3 == "AVC") {
                fmt.general = GeneralMediaFormat::Avc;
            }
        } else if (part2 == "MS" && part3 == "V3") {
            fmt.sub = SubFormats::Mpeg4SimpleProfile1;
        }
    } else if (part1 == "V_MPEG1") {
        fmt.general = GeneralMediaFormat::Mpeg1Video;
    } else if (part1 == "V_MPEG2") {
        fmt.general = GeneralMediaFormat::Mpeg2Video;
    } else if (part1 == "V_REAL") {
        fmt.general = GeneralMediaFormat::RealVideo;
    } else if (part1 == "V_QUICKTIME") {
        fmt.general = GeneralMediaFormat::QuicktimeVideo;
    } else if (part1 == "V_THEORA") {
        fmt.general = GeneralMediaFormat::Theora;
    } else if (part1 == "V_PRORES") {
        fmt.general = GeneralMediaFormat::ProRes;
    } else if (part1 == "V_VP8") {
        fmt.general = GeneralMediaFormat::Vp8;
    } else if (part1 == "V_VP9") {
        fmt.general = GeneralMediaFormat::Vp9;
    } else if (part1 == "V_AV1") {
        fmt.general = GeneralMediaFormat::Av1;
    } else if (part1 == "A_MPEG") {
        fmt.general = GeneralMediaFormat::Mpeg1Audio;
        if (part2 == "L1") {
            fmt.sub = SubFormats::Mpeg1Layer1;
        } else if (part2 == "L2") {
            fmt.sub = SubFormats::Mpeg1Layer2;
        } else if (part2 == "L3") {
            fmt.sub = SubFormats::Mpeg1Layer3;
        }
    } else if (part1 == "V_MPEGH" && part2 == "ISO" && part3 == "HEVC") {
        fmt.general = GeneralMediaFormat::Hevc;
    } else if (part1 == "A_PCM") {
        fmt.general = GeneralMediaFormat::Pcm;
        if (part2 == "INT") {
            if (part3 == "BIG") {
                fmt.sub = SubFormats::PcmIntBe;
            } else if (part3 == "LIT") {
                fmt.sub = SubFormats::PcmIntLe;
            }
        } else if (part2 == "FLOAT" && part3 == "IEEE") {
            fmt.sub = SubFormats::PcmFloatIeee;
        }
    } else if (part1 == "A_MPC") {
        fmt.general = GeneralMediaFormat::Mpc;
    } else if (part1 == "A_AC3") {
        fmt.general = GeneralMediaFormat::Ac3;
    } else if (part1 == "A_EAC3") {
        fmt.general = GeneralMediaFormat::EAc3;
    } else if (part1 == "A_ALAC") {
        fmt.general = GeneralMediaFormat::Alac;
    } else if (part1 == "A_DTS") {
        fmt.general = GeneralMediaFormat::Dts;
        if (part2 == "EXPRESS") {
            fmt.sub = SubFormats::DtsExpress;
        } else if (part2 == "LOSSLESS") {
            fmt.sub = SubFormats::DtsLossless;
        }
    } else if (part1 == "A_VORBIS") {
        fmt.general = GeneralMediaFormat::Vorbis;
    } else if (part1 == "A_FLAC") {
        fmt.general = GeneralMediaFormat::Flac;
    } else if (part1 == "A_OPUS") {
        fmt.general = GeneralMediaFormat::Opus;
    } else if (part1 == "A_REAL") {
        fmt.general = GeneralMediaFormat::RealAudio;
    } else if (part1 == "A_MS" && part2 == "ACM") {
        fmt.general = GeneralMediaFormat::MicrosoftAudioCodecManager;
    } else if (part1 == "A_AAC") {
        fmt.general = GeneralMediaFormat::Aac;
        if (part2 == "MPEG2") {
            if (part3 == "MAIN") {
                fmt.sub = SubFormats::AacMpeg2MainProfile;
            } else if (part3 == "LC") {
                fmt.sub = SubFormats::AacMpeg2LowComplexityProfile;
            } else if (part3 == "SBR") {
                fmt.sub = SubFormats::AacMpeg2LowComplexityProfile;
                fmt.extension = ExtensionFormats::SpectralBandReplication;
            } else if (part3 == "SSR") {
                fmt.sub = SubFormats::AacMpeg2ScalableSamplingRateProfile;
            }
        } else if (part2 == "MPEG4") {
            if (part3 == "MAIN") {
                fmt.sub = SubFormats::AacMpeg4MainProfile;
            } else if (part3 == "LC") {
                fmt.sub = SubFormats::AacMpeg4LowComplexityProfile;
            } else if (part3 == "SBR") {
                fmt.sub = SubFormats::AacMpeg4LowComplexityProfile;
                fmt.extension = ExtensionFormats::SpectralBandReplication;
            } else if (part3 == "SSR") {
                fmt.sub = SubFormats::AacMpeg4ScalableSamplingRateProfile;
            } else if (part3 == "LTP") {
                fmt.sub = SubFormats::AacMpeg4LongTermPrediction;
            }
        }
    } else if (part1 == "A_QUICKTIME") {
        fmt.general = GeneralMediaFormat::QuicktimeAudio;
    } else if (part1 == "A_TTA1") {
        fmt.general = GeneralMediaFormat::Tta;
    } else if (part1 == "A_WAVPACK4") {
        fmt.general = GeneralMediaFormat::WavPack;
    } else if (part1 == "S_TEXT") {
        fmt.general = GeneralMediaFormat::TextSubtitle;
        if (part2 == "UTF8") {
            fmt.sub = SubFormats::PlainUtf8Subtitle;
        } else if (part2 == "SSA") {
            fmt.sub = SubFormats::SubStationAlpha;
        } else if (part2 == "ASS") {
            fmt.sub = SubFormats::AdvancedSubStationAlpha;
        } else if (part2 == "USF") {
            fmt.sub = SubFormats::UniversalSubtitleFormat;
        } else if (part2 == "WEBVTT") {
            fmt.sub = SubFormats::WebVideoTextTracksFormat;
        }
    } else if (part1 == "S_IMAGE") {
        fmt.general = GeneralMediaFormat::ImageSubtitle;
        if (part2 == "BMP") {
            fmt.sub = SubFormats::ImgSubBmp;
        }
    } else if (part1 == "S_VOBSUB") {
        fmt.general = GeneralMediaFormat::VobSub;
    } else if (part1 == "S_KATE") {
        fmt.general = GeneralMediaFormat::OggKate;
    } else if (part1 == "B_VOBBTN") {
        fmt.general = GeneralMediaFormat::VobBtn;
    } else if (part1 == "S_DVBSUB") {
        fmt.general = GeneralMediaFormat::DvbSub;
    } else if (part1 == "V_MSWMV") {
        fmt.general = GeneralMediaFormat::Vc1;
    }
    return fmt;
}

/// \cond

template <typename PropertyType, typename ConversionFunction>
void MatroskaTrack::assignPropertyFromTagValue(const std::unique_ptr<MatroskaTag> &tag, std::string_view fieldId, PropertyType &property,
    const ConversionFunction &conversionFunction, Diagnostics &diag)
{
    const TagValue &value = tag->value(std::string(fieldId));
    if (!value.isEmpty()) {
        try {
            property = conversionFunction(value);
        } catch (const ConversionException &) {
            string message;
            try {
                message = argsToString("Ignoring invalid value \"", value.toString(TagTextEncoding::Utf8), "\" of \"", fieldId, '\"', '.');
            } catch (const ConversionException &) {
                message = argsToString("Ignoring invalid value of \"", fieldId, '\"', '.');
            }
            diag.emplace_back(DiagLevel::Warning, message, argsToString("reading track statatistic from \"", tag->toString(), '\"'));
        }
    }
}

template <typename NumberType, Traits::EnableIf<std::is_integral<NumberType>> * = nullptr> NumberType tagValueToNumber(const TagValue &tagValue)
{
    // optimization for Latin1/UTF-8 strings
    if (tagValue.type() == TagDataType::Text) {
        switch (tagValue.dataEncoding()) {
        case TagTextEncoding::Latin1:
        case TagTextEncoding::Utf8:
            return bufferToNumber<NumberType>(tagValue.dataPointer(), tagValue.dataSize());
        default:;
        }
    }
    // generic conversion
    return stringToNumber<NumberType>(tagValue.toString(TagTextEncoding::Utf8));
}

template <typename NumberType, Traits::EnableIf<std::is_floating_point<NumberType>> * = nullptr>
NumberType tagValueToBitrate(const TagValue &tagValue)
{
    return stringToNumber<NumberType>(tagValue.toString(TagTextEncoding::Utf8)) / 1000;
}

/// \endcond

/*!
 * \brief Reads track-specific statistics from the specified \a tags.
 * \remarks
 * - Those statistics are generated might be generated by some Muxers, eg. mkvmerge 7.0.0 or newer.
 * - Only tags targeting the track are considered. Hence the track ID must have been determined
 *   before (either by calling parseHeader() or setId()).
 * \sa https://github.com/mbunkus/mkvtoolnix/wiki/Automatic-tag-generation for list of track-specific
 *     tag fields written by mkvmerge
 */
void MatroskaTrack::readStatisticsFromTags(const std::vector<std::unique_ptr<MatroskaTag>> &tags, Diagnostics &diag)
{
    using namespace std::placeholders;
    using namespace MatroskaTagIds::TrackSpecific;
    for (const auto &tag : tags) {
        const TagTarget &target = tag->target();
        if (find(target.tracks().cbegin(), target.tracks().cend(), id()) == target.tracks().cend()) {
            continue;
        }
        assignPropertyFromTagValue(tag, numberOfBytes(), m_size, &tagValueToNumber<std::uint64_t>, diag);
        assignPropertyFromTagValue(tag, numberOfFrames(), m_sampleCount, &tagValueToNumber<std::uint64_t>, diag);
        assignPropertyFromTagValue(tag, MatroskaTagIds::TrackSpecific::duration(), m_duration, bind(&TagValue::toTimeSpan, _1), diag);
        assignPropertyFromTagValue(tag, MatroskaTagIds::TrackSpecific::bitrate(), m_bitrate, &tagValueToBitrate<double>, diag);
        assignPropertyFromTagValue(tag, writingDate(), m_modificationTime, bind(&TagValue::toDateTime, _1), diag);
        if (m_creationTime.isNull()) {
            m_creationTime = m_modificationTime;
        }
    }
}

void MatroskaTrack::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing header of Matroska track");
    try {
        m_trackElement->parse(diag);
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Critical, "Unable to parse track element.", context);
        throw;
    }
    // read information about the track from the children of the track entry element
    auto hasIsoLanguage = false;
    m_flags = TrackFlags::Default | TrackFlags::Enabled;
    for (EbmlElement *trackInfoElement = m_trackElement->firstChild(), *subElement = nullptr; trackInfoElement;
         trackInfoElement = trackInfoElement->nextSibling()) {
        try {
            trackInfoElement->parse(diag);
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Unable to parse track information element.", context);
            break;
        }
        std::uint64_t defaultDuration = 0;
        switch (trackInfoElement->id()) {
        case MatroskaIds::TrackType:
            switch (trackInfoElement->readUInteger()) {
            case MatroskaTrackType::Video:
                m_mediaType = MediaType::Video;
                break;
            case MatroskaTrackType::Audio:
                m_mediaType = MediaType::Audio;
                break;
            case MatroskaTrackType::Subtitle:
                m_mediaType = MediaType::Text;
                break;
            case MatroskaTrackType::Buttons:
                m_mediaType = MediaType::Buttons;
                break;
            case MatroskaTrackType::Control:
                m_mediaType = MediaType::Control;
                break;
            default:
                m_mediaType = MediaType::Unknown;
            }
            break;
        case MatroskaIds::TrackVideo:
            for (subElement = trackInfoElement->firstChild(); subElement; subElement = subElement->nextSibling()) {
                try {
                    subElement->parse(diag);
                } catch (const Failure &) {
                    diag.emplace_back(DiagLevel::Critical, "Unable to parse video track element.", context);
                    break;
                }
                switch (subElement->id()) {
                case MatroskaIds::DisplayWidth:
                    m_displaySize.setWidth(static_cast<std::uint32_t>(subElement->readUInteger()));
                    break;
                case MatroskaIds::DisplayHeight:
                    m_displaySize.setHeight(static_cast<std::uint32_t>(subElement->readUInteger()));
                    break;
                case MatroskaIds::PixelWidth:
                    m_pixelSize.setWidth(static_cast<std::uint32_t>(subElement->readUInteger()));
                    break;
                case MatroskaIds::PixelHeight:
                    m_pixelSize.setHeight(static_cast<std::uint32_t>(subElement->readUInteger()));
                    break;
                case MatroskaIds::PixelCropTop:
                    m_cropping.setTop(static_cast<std::uint32_t>(subElement->readUInteger()));
                    break;
                case MatroskaIds::PixelCropLeft:
                    m_cropping.setLeft(static_cast<std::uint32_t>(subElement->readUInteger()));
                    break;
                case MatroskaIds::PixelCropBottom:
                    m_cropping.setBottom(static_cast<std::uint32_t>(subElement->readUInteger()));
                    break;
                case MatroskaIds::PixelCropRight:
                    m_cropping.setRight(static_cast<std::uint32_t>(subElement->readUInteger()));
                    break;
                case MatroskaIds::FrameRate:
                    m_fps = static_cast<std::uint32_t>(subElement->readFloat());
                    break;
                case MatroskaIds::FlagInterlaced:
                    modFlagEnum(m_flags, TrackFlags::Interlaced, subElement->readUInteger());
                    break;
                case MatroskaIds::ColorSpace:
                    m_colorSpace = static_cast<std::uint32_t>(subElement->readUInteger());
                    break;
                default:;
                }
            }
            break;
        case MatroskaIds::TrackAudio:
            for (subElement = trackInfoElement->firstChild(); subElement; subElement = subElement->nextSibling()) {
                try {
                    subElement->parse(diag);
                } catch (const Failure &) {
                    diag.emplace_back(DiagLevel::Critical, "Unable to parse audio track element.", context);
                    break;
                }
                switch (subElement->id()) {
                case MatroskaIds::BitDepth:
                    m_bitsPerSample = static_cast<std::uint16_t>(subElement->readUInteger());
                    break;
                case MatroskaIds::Channels:
                    m_channelCount = static_cast<std::uint16_t>(subElement->readUInteger());
                    break;
                case MatroskaIds::SamplingFrequency:
                    if (!m_samplingFrequency) {
                        m_samplingFrequency = static_cast<std::uint32_t>(subElement->readFloat());
                    }
                    break;
                case MatroskaIds::OutputSamplingFrequency:
                    if (!m_extensionSamplingFrequency) {
                        m_extensionSamplingFrequency = static_cast<std::uint32_t>(subElement->readFloat());
                    }
                    break;
                default:;
                }
            }
            break;
        case MatroskaIds::TrackNumber:
            m_trackNumber = static_cast<std::uint32_t>(trackInfoElement->readUInteger());
            break;
        case MatroskaIds::TrackUID:
            m_id = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackName:
            m_name = trackInfoElement->readString();
            break;
        case MatroskaIds::TrackLanguage:
            m_locale.emplace_back(trackInfoElement->readString(), LocaleFormat::ISO_639_2_B);
            hasIsoLanguage = true;
            break;
        case MatroskaIds::TrackLanguageIETF:
            m_locale.emplace_back(trackInfoElement->readString(), LocaleFormat::BCP_47);
            break;
        case MatroskaIds::CodecID:
            m_format = codecIdToMediaFormat(m_formatId = trackInfoElement->readString());
            break;
        case MatroskaIds::CodecName:
            m_formatName = trackInfoElement->readString();
            break;
        case MatroskaIds::CodecDelay:
            break; // TODO
        case MatroskaIds::TrackFlagEnabled:
            modFlagEnum(m_flags, TrackFlags::Enabled, trackInfoElement->readUInteger());
            break;
        case MatroskaIds::TrackFlagDefault:
            modFlagEnum(m_flags, TrackFlags::Default, trackInfoElement->readUInteger());
            break;
        case MatroskaIds::TrackFlagForced:
            modFlagEnum(m_flags, TrackFlags::Forced, trackInfoElement->readUInteger());
            break;
        case MatroskaIds::TrackFlagLacing:
            modFlagEnum(m_flags, TrackFlags::Lacing, trackInfoElement->readUInteger());
            break;
        case MatroskaIds::DefaultDuration:
            defaultDuration = trackInfoElement->readUInteger();
            break;
        default:;
        }
        switch (m_mediaType) {
        case MediaType::Video:
            if (!m_fps && defaultDuration) {
                m_fps = static_cast<std::uint32_t>(1000000000.0 / static_cast<double>(defaultDuration));
            }
            break;
        default:;
        }
    }

    // read further information from the CodecPrivate element for some codecs
    EbmlElement *codecPrivateElement;
    switch (m_format.general) {
    case GeneralMediaFormat::MicrosoftVideoCodecManager:
        if ((codecPrivateElement = m_trackElement->childById(MatroskaIds::CodecPrivate, diag))) {
            // parse bitmap info header to determine actual format
            if (codecPrivateElement->dataSize() >= 0x28) {
                m_istream->seekg(static_cast<streamoff>(codecPrivateElement->dataOffset()));
                BitmapInfoHeader bitmapInfoHeader;
                bitmapInfoHeader.parse(reader());
                m_formatId.reserve(m_formatId.size() + 7);
                m_formatId += " \"";
                m_formatId += interpretIntegerAsString(bitmapInfoHeader.compression);
                m_formatId += "\"";
                m_format += FourccIds::fourccToMediaFormat(bitmapInfoHeader.compression);
            } else {
                diag.emplace_back(DiagLevel::Critical, "BITMAPINFOHEADER structure (in \"CodecPrivate\"-element) is truncated.", context);
            }
        }
        break;
    case GeneralMediaFormat::MicrosoftAudioCodecManager:
        if ((codecPrivateElement = m_trackElement->childById(MatroskaIds::CodecPrivate, diag))) {
            // parse WAVE header to determine actual format
            m_istream->seekg(static_cast<streamoff>(codecPrivateElement->dataOffset()));
            WaveFormatHeader waveFormatHeader;
            waveFormatHeader.parse(reader(), codecPrivateElement->dataSize(), diag);
            WaveAudioStream::addInfo(waveFormatHeader, *this);
        }
        break;
    case GeneralMediaFormat::Aac:
        if ((codecPrivateElement = m_trackElement->childById(MatroskaIds::CodecPrivate, diag))) {
            auto audioSpecificConfig
                = Mp4Track::parseAudioSpecificConfig(*m_istream, codecPrivateElement->dataOffset(), codecPrivateElement->dataSize(), diag);
            m_format += Mpeg4AudioObjectIds::idToMediaFormat(
                audioSpecificConfig->audioObjectType, audioSpecificConfig->sbrPresent, audioSpecificConfig->psPresent);
            if (audioSpecificConfig->sampleFrequencyIndex == 0xF) {
                //m_samplingFrequency = audioSpecificConfig->sampleFrequency;
            } else if (audioSpecificConfig->sampleFrequencyIndex < sizeof(mpeg4SamplingFrequencyTable)) {
                //m_samplingFrequency = mpeg4SamplingFrequencyTable[audioSpecificConfig->sampleFrequencyIndex];
            } else {
                diag.emplace_back(DiagLevel::Warning, "Audio specific config has invalid sample frequency index.", context);
            }
            if (audioSpecificConfig->extensionSampleFrequencyIndex == 0xF) {
                //m_extensionSamplingFrequency = audioSpecificConfig->extensionSampleFrequency;
            } else if (audioSpecificConfig->extensionSampleFrequencyIndex < sizeof(mpeg4SamplingFrequencyTable)) {
                //m_extensionSamplingFrequency = mpeg4SamplingFrequencyTable[audioSpecificConfig->extensionSampleFrequencyIndex];
            } else {
                diag.emplace_back(DiagLevel::Warning, "Audio specific config has invalid extension sample frequency index.", context);
            }
            m_channelConfig = audioSpecificConfig->channelConfiguration;
            m_extensionChannelConfig = audioSpecificConfig->extensionChannelConfiguration;
        }
        break;
    case GeneralMediaFormat::Avc:
        if ((codecPrivateElement = m_trackElement->childById(MatroskaIds::CodecPrivate, diag))) {
            auto avcConfig = make_unique<TagParser::AvcConfiguration>();
            try {
                m_istream->seekg(static_cast<streamoff>(codecPrivateElement->dataOffset()));
                avcConfig->parse(m_reader, codecPrivateElement->dataSize(), diag);
                Mp4Track::addInfo(*avcConfig, *this);
            } catch (const TruncatedDataException &) {
                diag.emplace_back(DiagLevel::Critical, "AVC configuration is truncated.", context);
            } catch (const Failure &) {
                diag.emplace_back(DiagLevel::Critical, "AVC configuration is invalid.", context);
            }
        }
        break;
    default:;
    }

    // parse format name for unknown formats
    if (m_format.general == GeneralMediaFormat::Unknown && m_formatName.empty()) {
        if (startsWith<string>(m_formatId, "V_") || startsWith<string>(m_formatId, "A_") || startsWith<string>(m_formatId, "S_")) {
            m_formatName = m_formatId.substr(2);
        } else {
            m_formatName = m_formatId;
        }
        m_formatName.append(" (unknown)");
    }

    // use pixel size as display size if display size not specified
    if (!m_displaySize.width()) {
        m_displaySize.setWidth(m_pixelSize.width());
    }
    if (!m_displaySize.height()) {
        m_displaySize.setHeight(m_pixelSize.height());
    }

    // set English if no ISO language has been specified (it is the default value of MatroskaIds::TrackLanguage)
    if (!hasIsoLanguage) {
        m_locale.emplace_back("eng"sv, LocaleFormat::ISO_639_2_B);
    }
}

/*!
 * \class TagParser::MatroskaTrackHeaderMaker
 * \brief The MatroskaTrackHeaderMaker class helps writing Matroska "TrackEntry"-elements storing track header information.
 *
 * An instance can be obtained using the MatroskaTrack::prepareMakingHeader() method.
 */

/*!
 * \brief Prepares making the header for the specified \a track.
 * \sa See MatroskaTrack::prepareMakingHeader() for more information.
 */
MatroskaTrackHeaderMaker::MatroskaTrackHeaderMaker(const MatroskaTrack &track, Diagnostics &diag)
    : m_track(track)
    , m_language(m_track.locale().abbreviatedName(LocaleFormat::ISO_639_2_B, LocaleFormat::Unknown))
    , m_languageIETF(m_track.locale().abbreviatedName(LocaleFormat::BCP_47))
    , m_dataSize(0)
{
    CPP_UTILITIES_UNUSED(diag);

    // calculate size for recognized elements
    m_dataSize += 2u + 1u + EbmlElement::calculateUIntegerLength(m_track.id());
    m_dataSize += 1u + 1u + EbmlElement::calculateUIntegerLength(m_track.trackNumber());
    m_dataSize += 1u + 1u + EbmlElement::calculateUIntegerLength(m_track.isEnabled());
    m_dataSize += 1u + 1u + EbmlElement::calculateUIntegerLength(m_track.isDefault());
    m_dataSize += 2u + 1u + EbmlElement::calculateUIntegerLength(m_track.isForced());
    if (!m_track.name().empty()) {
        m_dataSize += 2u + EbmlElement::calculateSizeDenotationLength(m_track.name().size()) + m_track.name().size();
    }

    // compute size of the mandatory "Language" element (if there's no language set, the 3 byte long value "und" is used)
    const auto languageSize = m_language.empty() ? 3 : m_language.size();
    const auto languageElementSize = 3 + EbmlElement::calculateSizeDenotationLength(languageSize) + languageSize;
    // compute size of the optional "LanguageIETF" element
    const auto languageIETFElementSize
        = m_languageIETF.empty() ? 0 : (3 + EbmlElement::calculateSizeDenotationLength(m_languageIETF.size()) + m_languageIETF.size());
    m_dataSize += languageElementSize + languageIETFElementSize;

    // calculate size for other elements
    for (EbmlElement *trackInfoElement = m_track.m_trackElement->firstChild(); trackInfoElement; trackInfoElement = trackInfoElement->nextSibling()) {
        switch (trackInfoElement->id()) {
        case MatroskaIds::TrackNumber:
        case MatroskaIds::TrackUID:
        case MatroskaIds::TrackName:
        case MatroskaIds::TrackLanguage:
        case MatroskaIds::TrackLanguageIETF:
        case MatroskaIds::TrackFlagEnabled:
        case MatroskaIds::TrackFlagDefault:
        case MatroskaIds::TrackFlagForced:
            // skip recognized elements
            break;
        default:
            trackInfoElement->makeBuffer();
            m_dataSize += trackInfoElement->totalSize();
        }
    }
    m_sizeDenotationLength = EbmlElement::calculateSizeDenotationLength(m_dataSize);
    m_requiredSize = 1u + m_sizeDenotationLength + m_dataSize;
}

/*!
 * \brief Saves the header for the track (specified when constructing the object) to the
 *        specified \a stream (makes a "TrackEntry"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw TagParser::Failure or a derived exception.
 */
void MatroskaTrackHeaderMaker::make(ostream &stream) const
{
    // make ID and size
    char buffer[9];
    *buffer = static_cast<char>(MatroskaIds::TrackEntry);
    EbmlElement::makeSizeDenotation(m_dataSize, buffer + 1);
    stream.write(buffer, 1 + m_sizeDenotationLength);

    // make recognized elements
    EbmlElement::makeSimpleElement(stream, MatroskaIds::TrackUID, m_track.id());
    EbmlElement::makeSimpleElement(stream, MatroskaIds::TrackNumber, m_track.trackNumber());
    EbmlElement::makeSimpleElement(stream, MatroskaIds::TrackFlagEnabled, m_track.isEnabled());
    EbmlElement::makeSimpleElement(stream, MatroskaIds::TrackFlagDefault, m_track.isDefault());
    EbmlElement::makeSimpleElement(stream, MatroskaIds::TrackFlagForced, m_track.isForced());
    if (!m_track.name().empty()) {
        EbmlElement::makeSimpleElement(stream, MatroskaIds::TrackName, m_track.name());
    }
    EbmlElement::makeSimpleElement(stream, MatroskaIds::TrackLanguage, m_language.empty() ? "und" : m_language);
    if (!m_languageIETF.empty()) {
        EbmlElement::makeSimpleElement(stream, MatroskaIds::TrackLanguageIETF, m_languageIETF);
    }

    // make other elements
    for (EbmlElement *trackInfoElement = m_track.m_trackElement->firstChild(); trackInfoElement; trackInfoElement = trackInfoElement->nextSibling()) {
        switch (trackInfoElement->id()) {
        case MatroskaIds::TrackNumber:
        case MatroskaIds::TrackUID:
        case MatroskaIds::TrackName:
        case MatroskaIds::TrackLanguage:
        case MatroskaIds::TrackLanguageIETF:
        case MatroskaIds::TrackFlagEnabled:
        case MatroskaIds::TrackFlagDefault:
        case MatroskaIds::TrackFlagForced:
            // skip recognized elements
            break;
        default:
            trackInfoElement->copyBuffer(stream);
        }
    }
}

} // namespace TagParser
