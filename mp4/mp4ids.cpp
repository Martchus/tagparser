#include "./mp4ids.h"

#include "../mediaformat.h"

namespace TagParser {

/*!
 * \brief Encapsulates the most common MP4 atom IDs.
 */
namespace Mp4AtomIds {
}

/*!
 * \brief Encapsulates IDs of MP4 atoms holding tag information.
 * \sa See http://atomicparsley.sourceforge.net/mpeg-4files.html and
 *     https://exiftool.org/TagNames/QuickTime.html for a table of known iTunes metadata atom IDs.
 */
namespace Mp4TagAtomIds {
}

/*!
 * \brief Encapsulates "mean values" used in iTunes style MP4 tags.
 */
namespace Mp4TagExtendedMeanIds {
std::string_view iTunes = "com.apple.iTunes";
}

/*!
 * \brief Encapsulates "name values" used in iTunes style MP4 tags.
 */
namespace Mp4TagExtendedNameIds {
std::string_view cdec = "cdec";
std::string_view label = "LABEL";
} // namespace Mp4TagExtendedNameIds

/*!
 * \brief Encapsulates all supported MP4 media type IDs.
 */
namespace Mp4MediaTypeIds {
}

/*!
 * \brief Encapsulates all supported MP4 media format IDs (aka "FOURCCs").
 * \sa http://wiki.multimedia.cx/?title=QuickTime_container
 */
namespace FourccIds {

MediaFormat fourccToMediaFormat(std::uint32_t fourccId)
{
    switch (fourccId) {
    case Mpeg:
        return GeneralMediaFormat::Mpeg1Video;
    case Mpeg2Imx30:
    case Mpeg2Imx50:
        return GeneralMediaFormat::Mpeg2Video;
    case Mpeg4Video:
        return GeneralMediaFormat::Mpeg4Video;
    case Mpeg4TimedText:
        return GeneralMediaFormat::Mpeg4TimedText;
    case Hevc1:
    case Hevc2:
        return GeneralMediaFormat::Hevc;
    case Avc1:
    case Avc2:
    case Avc3:
    case Avc4:
    case H264Decoder1:
    case H264Decoder2:
    case H264Decoder3:
    case H264Decoder4:
    case H264Decoder5:
    case H264Decoder6:
        return GeneralMediaFormat::Avc;
    case Av1_IVF:
    case Av1_ISOBMFF:
        return GeneralMediaFormat::Av1;
    case Divx4Decoder1:
    case Divx4Decoder2:
    case H263Quicktime:
    case H2633GPP:
    case XvidDecoder1:
    case XvidDecoder2:
    case XvidDecoder3:
    case XvidDecoder4:
    case XvidDecoder5:
    case Divx5Decoder:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4AdvancedSimpleProfile0);
    case Divx3Decoder1:
    case Divx3Decoder2:
    case Divx3Decoder3:
    case Divx3Decoder4:
    case Divx3Decoder5:
    case Divx3Decoder6:
    case Divx3Decoder7:
    case Divx3Decoder8:
    case Divx3Decoder9:
    case Divx3Decoder10:
    case Divx3Decoder11:
    case Divx3Decoder12:
    case Divx3Decoder13:
    case Divx3Decoder14:
    case Divx3Decoder15:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4SimpleProfile0);
    case Tiff:
        return GeneralMediaFormat::Tiff;
    case AppleTextAtsuiCodec:
        return GeneralMediaFormat::TimedText;
    case Raw:
        return GeneralMediaFormat::UncompressedVideoFrames;
    case Jpeg:
        return GeneralMediaFormat::Jpeg;
    case Gif:
        return GeneralMediaFormat::Gif;
    case Png:
        return GeneralMediaFormat::Png;
    case AdpcmAcm:
        return GeneralMediaFormat::AdpcmAcm;
    case ImaadpcmAcm:
        return GeneralMediaFormat::ImaadpcmAcm;
    case Mp3CbrOnly:
    case Mp3:
        return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
    case Mpeg4Audio:
        return GeneralMediaFormat::Aac;
    case Alac:
        return GeneralMediaFormat::Alac;
    case Ac3:
        return GeneralMediaFormat::Ac3;
    case EAc3:
        return GeneralMediaFormat::EAc3;
    case DolbyMpl:
        return GeneralMediaFormat::DolbyMlp;
    case Ac4:
        return GeneralMediaFormat::Ac4;
    case Rv20:
    case Rv30:
    case Rv40:
        return GeneralMediaFormat::RealVideo;
    case Int24:
    case Int32:
        return MediaFormat(GeneralMediaFormat::Pcm);
    case Int16Be:
        return MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmIntBe);
    case Int16Le:
        return MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmIntLe);
    case FloatingPoint32Bit:
    case FloatingPoint64Bit:
        return MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmFloatIeee);
    case Amr:
    case AmrNarrowband:
        return MediaFormat(GeneralMediaFormat::Amr);
    case Dts:
    case DtsH:
        return MediaFormat(GeneralMediaFormat::Dts);
    case DtsE:
        return MediaFormat(GeneralMediaFormat::Dts, SubFormats::DtsExpress);
    case WindowsMediaAudio:
    case WindowsMediaAudio7:
    case WindowsMediaAudio9Professional:
    case WindowsMediaAudio9Standard:
        return MediaFormat(GeneralMediaFormat::WindowsMediaAudio);
    case MsMpeg4V1Decoder1:
    case MsMpeg4V1Decoder2:
    case MsMpeg4V1Decoder3:
    case MsMpeg4V1Decoder4:
    case MsMpeg4V1Decoder5:
    case MsMpeg4V1Decoder6:
        return MediaFormat(GeneralMediaFormat::MicrosoftMpeg4, 1);
    case MsMpeg4V2Decoder1:
    case MsMpeg4V2Decoder2:
    case MsMpeg4V2Decoder3:
    case MsMpeg4V2Decoder4:
        return MediaFormat(GeneralMediaFormat::MicrosoftMpeg4, 2);
    case MsMpeg4V3Decoder1:
    case MsMpeg4V3Decoder2:
        return MediaFormat(GeneralMediaFormat::MicrosoftMpeg4, 3);
    case Vp8:
        return GeneralMediaFormat::Vp8;
    case Vp9:
    case Vp9_2:
        return GeneralMediaFormat::Vp9;
    case WavPack:
        return MediaFormat(GeneralMediaFormat::WavPack);
    case WindowsMediaVideoV17:
        return MediaFormat(GeneralMediaFormat::WindowsMediaVideo, 1);
    case WindowsMediaVideoV2:
    case WindowsMediaVideoV8:
        return MediaFormat(GeneralMediaFormat::WindowsMediaVideo, 2);
    case Flac:
        return GeneralMediaFormat::Flac;
    case Opus:
        return GeneralMediaFormat::Opus;
    // TODO: map more FOURCCs
    default:
        return GeneralMediaFormat::Unknown;
    }
}

} // namespace FourccIds

/*!
 * \brief Encapsulates all supported MP4 media format description extensions.
 * \sa https://developer.apple.com/library/mac/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html
 */
namespace Mp4FormatExtensionIds {
}

/*!
 * \brief Encapsulates all supported MPEG-4 elementary stream object IDs.
 */
namespace Mpeg4ElementaryStreamObjectIds {

/*!
 * \brief Returns the TagParser::MediaFormat denoted by the specified MPEG-4 stream ID.
 */
MediaFormat streamObjectTypeFormat(std::uint8_t streamObjectTypeId)
{
    switch (streamObjectTypeId) {
    case SystemsIso144961:
        return GeneralMediaFormat::Systems;
    case SystemsIso144961v2:
        return MediaFormat(GeneralMediaFormat::Systems, 2);
    case InteractionStream:
        return GeneralMediaFormat::InteractionStream;
    case AfxStream:
        return GeneralMediaFormat::AfxStream;
    case FontDataStream:
        return GeneralMediaFormat::FontDataStream;
    case SynthesizedTextureStream:
        return GeneralMediaFormat::SynthesizedTextureStream;
    case StreamingTextStream:
        return GeneralMediaFormat::StreamingTextStream;
    case Mpeg4Visual:
        return GeneralMediaFormat::Mpeg4Video;
    case Avc:
        return GeneralMediaFormat::Avc;
    case ParameterSetsForAvc:
        return GeneralMediaFormat::Avc;
    case Als:
        return GeneralMediaFormat::Als;
    case Sa0c:
        return GeneralMediaFormat::Sa0c;
    case Aac:
        return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LowComplexityProfile);
    case Mpeg2VideoSimpleProfile:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SimpleProfile);
    case Mpeg2VideoMainProfile:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SnrProfile);
    case Mpeg2VideoSnrProfile:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SpatialProfile);
    case Mpeg2VideoSpatialProfile:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2HighProfile);
    case Mpeg2VideoHighProfile:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2HighProfile);
    case Mpeg2Video422Profile:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SimpleProfile);
    case Mpeg2AacMainProfile:
        return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2MainProfile);
    case Mpeg2AacLowComplexityProfile:
        return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2LowComplexityProfile);
    case Mpeg2AacScaleableSamplingRateProfile:
        return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2ScalableSamplingRateProfile);
    case Mpeg2Audio:
        return GeneralMediaFormat::Mpeg2Audio;
    case Mpeg1Video:
        return GeneralMediaFormat::Mpeg1Video;
    case Mpeg1Audio:
        return GeneralMediaFormat::Mpeg1Audio;
    case Jpeg:
        return GeneralMediaFormat::Jpeg;
    case Png:
        return GeneralMediaFormat::Png;
    case Evrc:
    case PrivateEvrc:
        return GeneralMediaFormat::Evrc;
    case Smv:
        return GeneralMediaFormat::Smv;
    case Gpp2Cmf:
        return GeneralMediaFormat::Gpp2Cmf;
    case Vc1:
        return GeneralMediaFormat::Vc1;
    case Dirac:
        return GeneralMediaFormat::Dirac;
    case Ac3:
    case PrivateAc3:
        return GeneralMediaFormat::Ac3;
    case EAc3:
        return GeneralMediaFormat::EAc3;
    case Dts:
    case PrivateDts:
        return GeneralMediaFormat::Dts;
    case DtsHdHighResolution:
        return MediaFormat(GeneralMediaFormat::DtsHd, SubFormats::DtsHdHighResolution);
    case DtsHdMasterAudio:
        return MediaFormat(GeneralMediaFormat::DtsHd, SubFormats::DtsHdMasterAudio);
    case DtsHdExpress:
        return MediaFormat(GeneralMediaFormat::DtsHd, SubFormats::DtsExpress);
    case PrivateOgg:
    case PrivateOgg2:
        return GeneralMediaFormat::Vorbis;
    case PrivateVobSub:
        return GeneralMediaFormat::VobSub;
    case PrivateQcelp:
        return GeneralMediaFormat::Qcelp;
    default:
        return MediaFormat();
    }
}

} // namespace Mpeg4ElementaryStreamObjectIds

/*!
 * \brief Encapsulates all known MPEG-4 descriptor IDs.
 */
namespace Mpeg4DescriptorIds {
}

/*!
 * \brief Returns the name of the stream type denoted by the specified MPEG-4 stream type ID.
 */
namespace Mpeg4ElementaryStreamTypeIds {

/*!
 * \brief Returns the name of the stream type denoted by the specified MPEG-4 stream type ID.
 */
std::string_view streamTypeName(std::uint8_t streamTypeId)
{
    switch (streamTypeId) {
    case ObjectDescriptor:
        return "object descriptor";
    case ClockReference:
        return "clock reference";
    case SceneDescriptor:
        return "scene descriptor";
    case Visual:
        return "visual";
    case Audio:
        return "audio";
    case Mpeg7:
        return "MPEG-7";
    case Ipmps:
        return "IMPS";
    case ObjectContentInfo:
        return "object content info";
    case MpegJava:
        return "MPEG Java";
    case Interaction:
        return "interaction";
    case Ipmp:
        return "IPMP";
    case FontData:
        return "font data";
    case StreamingText:
        return "streaming text";
    default:
        return "";
    }
}

} // namespace Mpeg4ElementaryStreamTypeIds

/*!
 * \brief Encapsulates all supported MPEG-4 audio object type IDs.
 * \sa http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio
 */
namespace Mpeg4AudioObjectIds {

TAG_PARSER_EXPORT MediaFormat idToMediaFormat(std::uint8_t mpeg4AudioObjectId, bool sbrPresent, bool psPresent)
{
    MediaFormat fmt;
    switch (mpeg4AudioObjectId) {
    case AacMain:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4MainProfile);
        break;
    case AacLc:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LowComplexityProfile);
        break;
    case AacSsr:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4ScalableSamplingRateProfile);
        break;
    case AacLtp:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LongTermPrediction);
        break;
    case Sbr:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LowComplexityProfile, ExtensionFormats::SpectralBandReplication);
        break;
    case AacScalable:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4ScalableSamplingRateProfile);
        break;
    case ErAacLc:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4ERLowComplecityProfile);
        break;
    case ErAacLtp:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4ERLongTermPrediction);
        break;
    case ErAacLd:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4ERLowDelay);
        break;
    case Ps:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LowComplexityProfile, ExtensionFormats::ParametricStereo);
        break;
    case Layer1:
        fmt = MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer1);
        break;
    case Layer2:
        fmt = MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer2);
        break;
    case Layer3:
        fmt = MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
        break;
    default:;
    }
    if (sbrPresent) {
        fmt.extension |= ExtensionFormats::SpectralBandReplication;
    }
    if (psPresent) {
        fmt.extension |= ExtensionFormats::ParametricStereo;
    }
    return fmt;
}

} // namespace Mpeg4AudioObjectIds

std::uint32_t mpeg4SamplingFrequencyTable[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350 };

/*!
 * \brief Encapsulates all supported MPEG-4 channel configurations.
 */
namespace Mpeg4ChannelConfigs {

/*!
 * \brief Returns the string representation for the specified MPEG-4 channel config.
 */
std::string_view channelConfigString(std::uint8_t config)
{
    switch (config) {
    case AotSpecificConfig:
        return "defined in AOT Specific Config";
    case FrontCenter:
        return "1 channel: front-center";
    case FrontLeftFrontRight:
        return "2 channels: front-left, front-right";
    case FrontCenterFrontLeftFrontRight:
        return "3 channels: front-center, front-left, front-right";
    case FrontCenterFrontLeftFrontRightBackCenter:
        return "4 channels: front-center, front-left, front-right, back-center";
    case FrontCenterFrontLeftFrontRightBackLeftBackRight:
        return "5 channels: front-center, front-left, front-right, back-left, back-right";
    case FrontCenterFrontLeftFrontRightBackLeftBackRightLFEChannel:
        return "6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel";
    case FrontCenterFrontLeftFrontRightSideLeftSideRightBackLeftBackRightLFEChannel:
        return "8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel";
    default:
        return std::string_view();
    }
}

/*!
 * \brief Returns the channel count for the specified MPEG-4 channel config.
 */
std::uint8_t channelCount(std::uint8_t config)
{
    switch (config) {
    case FrontCenter:
        return 1;
    case FrontLeftFrontRight:
        return 2;
    case FrontCenterFrontLeftFrontRight:
        return 3;
    case FrontCenterFrontLeftFrontRightBackCenter:
        return 4;
    case FrontCenterFrontLeftFrontRightBackLeftBackRight:
        return 5;
    case FrontCenterFrontLeftFrontRightBackLeftBackRightLFEChannel:
        return 6;
    case FrontCenterFrontLeftFrontRightSideLeftSideRightBackLeftBackRightLFEChannel:
        return 8;
    default:
        return 0;
    }
}

} // namespace Mpeg4ChannelConfigs

/*!
 * \brief Encapsulates MPEG-4 video (14496-2) codes.
 */
namespace Mpeg4VideoCodes {
}

/*!
 * \brief Encapsulates MPEG-2 video codes.
 */
namespace Mpeg2VideoCodes {
}

} // namespace TagParser
