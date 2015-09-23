#include "./ebmlelement.h"
#include "./matroskatrack.h"
#include "./matroskacontainer.h"
#include "./matroskaid.h"

#include "../avi/bitmapinfoheader.h"

#include "../wav/waveaudiostream.h"

#include "../mp4/mp4ids.h"
#include "../mp4/mp4track.h"

#include "../mediaformat.h"
#include "../exceptions.h"

#include <c++utilities/conversion/stringconversion.h>

using namespace std;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::MatroskaTrack
 * \brief Implementation of Media::AbstractTrack for the Matroska container.
 */

/*!
 * \brief Constructs a new track for the specified \a trackElement.
 *
 * Each track element (ID: MatroskaId::TrackEntry) holds header information
 * for one track in the Matroska file.
 */
MatroskaTrack::MatroskaTrack(EbmlElement &trackElement) :
    AbstractTrack(trackElement.stream(), trackElement.startOffset()),
    m_trackElement(&trackElement)
{}

/*!
 * \brief Destroys the track.
 */
MatroskaTrack::~MatroskaTrack()
{}

TrackType MatroskaTrack::type() const
{
    return TrackType::MatroskaTrack;
}

/*!
 * \brief Returns the MediaFormat for the specified Matroska codec ID.
 */
MediaFormat MatroskaTrack::codecIdToMediaFormat(const string &codecId)
{
    auto parts = splitString<vector<string> >(codecId, "/", EmptyPartsTreat::Keep, 3);
    parts.resize(3);
    const auto &part1 = parts[0], &part2 = parts[1], &part3 = parts[2];
    MediaFormat fmt;
    if(part1 == "V_MS" && part2 == "VFW" && part3 == "FOURCC") {
        fmt.general = GeneralMediaFormat::MicrosoftVideoCodecManager;
    } else if(part1 == "V_UNCOMPRESSED") {
        fmt.general = GeneralMediaFormat::UncompressedVideoFrames;
    } else if(part1 == "V_MPEG4") {
        fmt.general = GeneralMediaFormat::Mpeg4Video;
        if(part2 == "ISO") {
            if(part3 == "SP") {
                fmt.sub = SubFormats::Mpeg4SimpleProfile1;
            } else if(part3 == "ASP") {
                fmt.sub = SubFormats::Mpeg4AdvancedSimpleProfile1;
            } else if(part3 == "AVC") {
                fmt.general = GeneralMediaFormat::Avc;
            }
        } else if(part2 == "MS" && part3 == "V3") {
            fmt.sub = SubFormats::Mpeg4SimpleProfile1;
        }
    } else if(part1 == "V_MPEG1") {
        fmt.general = GeneralMediaFormat::Mpeg1Video;
    } else if(part1 == "V_MPEG2") {
        fmt.general = GeneralMediaFormat::Mpeg2Video;
    } else if(part1 == "V_REAL") {
        fmt.general = GeneralMediaFormat::RealVideo;
    } else if(part1 == "V_QUICKTIME") {
        fmt.general = GeneralMediaFormat::QuicktimeVideo;
    } else if(part1 == "V_THEORA") {
        fmt.general = GeneralMediaFormat::Theora;
    } else if(part1 == "V_PRORES") {
        fmt.general = GeneralMediaFormat::ProRes;
    } else if(part1 == "V_VP8") {
        fmt.general = GeneralMediaFormat::Vp8;
    } else if(part1 == "V_VP9") {
        fmt.general = GeneralMediaFormat::Vp9;
    } else if(part1 == "A_MPEG") {
        fmt.general = GeneralMediaFormat::Mpeg1Audio;
        if(part2 == "L1") {
            fmt.sub = SubFormats::Mpeg1Layer1;
        } else if(part2 == "L2") {
            fmt.sub = SubFormats::Mpeg1Layer2;
        } else if(part2 == "L3") {
            fmt.sub = SubFormats::Mpeg1Layer3;
        }
    } else if(part1 == "A_PCM") {
        fmt.general = GeneralMediaFormat::Pcm;
        if(part2 == "INT") {
            if(part3 == "BIG") {
                fmt.sub = SubFormats::PcmIntBe;
            } else if(part3 == "LIT") {
                fmt.sub = SubFormats::PcmIntLe;
            }
        } else if (part2 == "FLOAT" && part3 == "IEEE") {
            fmt.sub = SubFormats::PcmFloatIeee;
        }
    } else if(part1 == "A_MPC") {
        fmt.general = GeneralMediaFormat::Mpc;
    } else if(part1 == "A_AC3") {
        fmt.general = GeneralMediaFormat::Ac3;
    } else if(part1 == "A_ALAC") {
        fmt.general = GeneralMediaFormat::Alac;
    } else if(part1 == "A_DTS") {
        fmt.general = GeneralMediaFormat::Dts;
        if(part2 == "EXPRESS") {
            fmt.sub = SubFormats::DtsExpress;
        } else if(part2 == "LOSSLESS") {
            fmt.sub = SubFormats::DtsLossless;
        }
    } else if(part1 == "A_VORBIS") {
        fmt.general = GeneralMediaFormat::Vorbis;
    } else if(part1 == "A_FLAC") {
        fmt.general = GeneralMediaFormat::Flac;
    } else if(part1 == "A_REAL") {
        fmt.general = GeneralMediaFormat::RealAudio;
    } else if(part1 == "A_MS" && part2 == "ACM") {
        fmt.general = GeneralMediaFormat::MicrosoftAudioCodecManager;
    } else if(part1 == "A_AAC") {
        fmt.general = GeneralMediaFormat::Aac;
        if(part2 == "MPEG2") {
            if(part3 == "MAIN") {
                fmt.sub = SubFormats::AacMpeg2MainProfile;
            } else if(part3 == "LC") {
                fmt.sub = SubFormats::AacMpeg2LowComplexityProfile;
            } else if(part3 == "SBR") {
                fmt.sub = SubFormats::AacMpeg2LowComplexityProfile;
                fmt.extension = ExtensionFormats::SpectralBandReplication;
            } else if(part3 == "SSR") {
                fmt.sub = SubFormats::AacMpeg2ScalableSamplingRateProfile;
            }
        } else if(part2 == "MPEG4") {
            if(part3 == "MAIN") {
                fmt.sub = SubFormats::AacMpeg4MainProfile;
            } else if(part3 == "LC") {
                fmt.sub = SubFormats::AacMpeg4LowComplexityProfile;
            } else if(part3 == "SBR") {
                fmt.sub = SubFormats::AacMpeg4LowComplexityProfile;
                fmt.extension = ExtensionFormats::SpectralBandReplication;
            } else if(part3 == "SSR") {
                fmt.sub = SubFormats::AacMpeg4ScalableSamplingRateProfile;
            } else if(part3 == "LTP") {
                fmt.sub = SubFormats::AacMpeg4LongTermPrediction;
            }
        }
    } else if(part1 == "A_QUICKTIME") {
        fmt.general = GeneralMediaFormat::QuicktimeAudio;
    } else if(part1 == "A_TTA1") {
        fmt.general = GeneralMediaFormat::Tta;
    } else if(part1 == "A_WAVPACK4") {
        fmt.general = GeneralMediaFormat::WavPack;
    } else if(part1 == "S_TEXT") {
        fmt.general = GeneralMediaFormat::TextSubtitle;
        if(part2 == "UTF8") {
            fmt.sub = SubFormats::TextSubBasicUtf8;
        } else if(part2 == "SSA") {
            fmt.sub = SubFormats::TextSubSubtitlesFormat;
        } else if(part2 == "ASS") {
            fmt.sub = SubFormats::TextSubAdvancedSubtitlesFormat;
        } else if(part2 == "USF") {
            fmt.sub = SubFormats::TextSubUniversalSubtitleFormat;
        }
    } else if(part1 == "S_IMAGE") {
        fmt.general = GeneralMediaFormat::ImageSubtitle;
        if(part2 == "BMP") {
            fmt.sub = SubFormats::ImgSubBmp;
        }
    } else if(part1 == "S_VOBSUB") {
        fmt.general = GeneralMediaFormat::VobSub;
    } else if(part1 == "S_KATE") {
        fmt.general = GeneralMediaFormat::OggKate;
    } else if(part1 == "B_VOBBTN") {
        fmt.general = GeneralMediaFormat::VobBtn;
    } else if(part1 == "V_MSWMV") {
        fmt.general = GeneralMediaFormat::Vc1;
    }
    return fmt;
}

void MatroskaTrack::internalParseHeader()
{
    static const string context("parsing header of Matroska track");
    try {
    m_trackElement->parse();
    } catch(Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse track element.", context);
        throw;
    }
    // read information about the track from the childs of the track entry element
    for(EbmlElement *trackInfoElement = m_trackElement->firstChild(), *subElement = nullptr; trackInfoElement; trackInfoElement = trackInfoElement->nextSibling()) {
        try {
            trackInfoElement->parse();
        } catch (Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse track information element.", context);
            break;
        }
        uint32 defaultDuration = 0;
        switch(trackInfoElement->id()) {
        case MatroskaIds::TrackType:
            switch(trackInfoElement->readUInteger()) {
            case MatroskaTrackType::Video:
                m_mediaType = MediaType::Video;
                break;
            case MatroskaTrackType::Audio:
                m_mediaType = MediaType::Audio;
                break;
            case MatroskaTrackType::Subtitle:
                m_mediaType = MediaType::Text;
                break;
            default:
                m_mediaType = MediaType::Unknown;
            }
            break;
        case MatroskaIds::TrackVideo:
            subElement = trackInfoElement->firstChild();
            while(subElement) {
                try {
                    subElement->parse();
                } catch (Failure &) {
                    addNotification(NotificationType::Critical, "Unable to parse video track element.", context);
                    break;
                }
                switch(subElement->id()) {
                case MatroskaIds::DisplayWidth:
                    m_displaySize.setWidth(subElement->readUInteger());
                    break;
                case MatroskaIds::DisplayHeight:
                    m_displaySize.setHeight(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelWidth:
                    m_pixelSize.setWidth(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelHeight:
                    m_pixelSize.setHeight(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelCropTop:
                    m_cropping.setTop(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelCropLeft:
                    m_cropping.setLeft(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelCropBottom:
                    m_cropping.setBottom(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelCropRight:
                    m_cropping.setRight(subElement->readUInteger());
                    break;
                case MatroskaIds::FrameRate:
                    m_fps = subElement->readFloat();
                    break;
                case MatroskaIds::FlagInterlaced:
                    m_interlaced = subElement->readUInteger();
                    break;
                case MatroskaIds::ColorSpace:
                    m_colorSpace = subElement->readUInteger();
                    break;
                default:
                    ;
                }
                subElement = subElement->nextSibling();
            }
            break;
        case MatroskaIds::TrackAudio:
            subElement = trackInfoElement->firstChild();
            while(subElement) {
                try {
                    subElement->parse();
                } catch (Failure &) {
                    addNotification(NotificationType::Critical, "Unable to parse audio track element.", context);
                    break;
                }
                switch(subElement->id()) {
                case MatroskaIds::BitDepth:
                    m_bitsPerSample = subElement->readUInteger();
                    break;
                case MatroskaIds::Channels:
                    m_channelCount = subElement->readUInteger();
                    break;
                case MatroskaIds::SamplingFrequency:
                    if(!m_samplingFrequency) {
                        m_samplingFrequency = subElement->readFloat();
                    }
                    break;
                case MatroskaIds::OutputSamplingFrequency:
                    if(!m_extensionSamplingFrequency) {
                        m_extensionSamplingFrequency = subElement->readFloat();
                    }
                    break;
                default:
                    ;
                }
                subElement = subElement->nextSibling();
            }
            break;
        case MatroskaIds::TrackNumber:
            m_trackNumber = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackUID:
            m_id = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackName:
            m_name = trackInfoElement->readString();
            break;
        case MatroskaIds::TrackLanguage:
            m_language = trackInfoElement->readString();
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
            m_enabled = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackFlagDefault:
            m_default = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackFlagForced:
            m_forced = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackFlagLacing:
            m_lacing = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::DefaultDuration:
            defaultDuration = trackInfoElement->readUInteger();
            break;
        default:
            ;
        }
        switch(m_mediaType) {
        case MediaType::Video:
            if(m_fps == 0 && defaultDuration != 0) {
                m_fps = 1000000000.0 / static_cast<double>(defaultDuration);
            }
            break;
        default:
            ;
        }
    }
    // read further information from the CodecPrivate element for some codecs
    switch(m_format.general) {
    EbmlElement *codecPrivateElement;
    case GeneralMediaFormat::MicrosoftVideoCodecManager:
        if((codecPrivateElement = m_trackElement->childById(MatroskaIds::CodecPrivate))) {
            // parse bitmap info header to determine actual format
            if(codecPrivateElement->dataSize() >= 0x28) {
                m_istream->seekg(codecPrivateElement->dataOffset());
                BitmapInfoHeader bitmapInfoHeader;
                bitmapInfoHeader.parse(reader());
                m_formatId.reserve(m_formatId.size() + 7);
                m_formatId += " \"";
                m_formatId += interpretIntegerAsString(bitmapInfoHeader.compression);
                m_formatId += "\"";
                m_format += FourccIds::fourccToMediaFormat(bitmapInfoHeader.compression);
            } else {
                addNotification(NotificationType::Critical, "BITMAPINFOHEADER structure (in \"CodecPrivate\"-element) is truncated.", context);
            }
        }
        break;
    case GeneralMediaFormat::MicrosoftAudioCodecManager:
        if((codecPrivateElement = m_trackElement->childById(MatroskaIds::CodecPrivate))) {
            // parse WAVE header to determine actual format
            if(codecPrivateElement->dataSize() >= 16) {
                m_istream->seekg(codecPrivateElement->dataOffset());
                WaveFormatHeader waveFormatHeader;
                waveFormatHeader.parse(reader());
                WaveAudioStream::addInfo(waveFormatHeader, *this);
            } else {
                addNotification(NotificationType::Critical, "BITMAPINFOHEADER structure (in \"CodecPrivate\"-element) is truncated.", context);
            }
        }
        break;
    case GeneralMediaFormat::Aac:
        if((codecPrivateElement = m_trackElement->childById(MatroskaIds::CodecPrivate))) {
            auto audioSpecificConfig = Mp4Track::parseAudioSpecificConfig(*this, *m_istream, codecPrivateElement->dataOffset(), codecPrivateElement->dataSize());
            m_format += Mpeg4AudioObjectIds::idToMediaFormat(audioSpecificConfig->audioObjectType, audioSpecificConfig->sbrPresent, audioSpecificConfig->psPresent);
            if(audioSpecificConfig->sampleFrequencyIndex == 0xF) {
                //m_samplingFrequency = audioSpecificConfig->sampleFrequency;
            } else if(audioSpecificConfig->sampleFrequencyIndex < sizeof(mpeg4SamplingFrequencyTable)) {
                //m_samplingFrequency = mpeg4SamplingFrequencyTable[audioSpecificConfig->sampleFrequencyIndex];
            } else {
                addNotification(NotificationType::Warning, "Audio specific config has invalid sample frequency index.", context);
            }
            if(audioSpecificConfig->extensionSampleFrequencyIndex == 0xF) {
                //m_extensionSamplingFrequency = audioSpecificConfig->extensionSampleFrequency;
            } else if(audioSpecificConfig->extensionSampleFrequencyIndex < sizeof(mpeg4SamplingFrequencyTable)) {
                //m_extensionSamplingFrequency = mpeg4SamplingFrequencyTable[audioSpecificConfig->extensionSampleFrequencyIndex];
            } else {
                addNotification(NotificationType::Warning, "Audio specific config has invalid extension sample frequency index.", context);
            }
            m_channelConfig = audioSpecificConfig->channelConfiguration;
        }
        break;
    default:
        ;
    }
    if(m_format.general == GeneralMediaFormat::Unknown && m_formatName.empty()) {
        if(startsWith<string>(m_formatId, "V_") || startsWith<string>(m_formatId, "A_") || startsWith<string>(m_formatId, "S_")) {
            m_formatName = m_formatId.substr(2);
        } else {
            m_formatName = m_formatId;
        }
        m_formatName.append(" (unknown)");
    }
}

}
