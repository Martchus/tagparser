#include "mp4ids.h"

#include "../mediaformat.h"

namespace Media {

/*!
 * \brief Encapsulates the most common MP4 atom IDs.
 */
namespace Mp4AtomIds {
}

/*!
 * \brief Encapsulates IDs of MP4 atoms holding tag information.
 */
namespace Mp4TagAtomIds {
}

/*!
 * \brief Encapsulates "mean values" used in iTunes style MP4 tags.
 */
namespace Mp4TagExtendedMeanIds {
const char *iTunes = "com.apple.iTunes";
}

/*!
 * \brief Encapsulates "name values" used in iTunes style MP4 tags.
 */
namespace Mp4TagExtendedNameIds {
const char *cdec = "cdec";
}

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

MediaFormat fourccToMediaFormat(uint32 fourccId)
{
    switch(fourccId) {
    case Mpeg:
        return GeneralMediaFormat::Mpeg1Video;
    case Mpeg2Imx30: case Mpeg2Imx50:
        return GeneralMediaFormat::Mpeg2Video;
    case Mpeg4Video:
        return GeneralMediaFormat::Mpeg4Video;
    case Avc1: case Avc2: case Avc3: case Avc4: case H264Decoder1: case H264Decoder2:
    case H264Decoder3: case H264Decoder4: case H264Decoder5: case H264Decoder6:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4Avc);
    case H263: case XvidDecoder1: case XvidDecoder2:
    case XvidDecoder3: case XvidDecoder4: case XvidDecoder5:
    case Divx5Decoder:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4Asp);
    case Divx4Decoder1: case Divx4Decoder2: case Divx4Decoder3:
    case Divx4Decoder4: case Divx4Decoder5: case Divx4Decoder6: case Divx4Decoder7:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4Sp);
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
    case Mp3CbrOnly: case Mp3:
        return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
    case Mpeg4Audio:
        return GeneralMediaFormat::Aac;
    case Alac:
        return GeneralMediaFormat::Alac;
    case Ac3:
        return GeneralMediaFormat::Ac3;
    case Ac4:
        return GeneralMediaFormat::Ac4;
    case Rv20: case Rv30: case Rv40:
        return GeneralMediaFormat::RealVideo;
    case Int24: case Int32:
        return MediaFormat(GeneralMediaFormat::Pcm);
    case Int16Be:
        return MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmIntBe);
    case Int16Le:
        return MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmIntLe);
    case FloatingPoint32Bit: case FloatingPoint64Bit:
        return MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmFloatIeee);
    // TODO: map more FOURCCs
    default:
        return GeneralMediaFormat::Unknown;
    }
}

}

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
 * \brief Returns the Media::MediaFormat denoted by the specified MPEG-4 stream ID.
 */
MediaFormat streamObjectTypeFormat(byte streamObjectTypeId)
{
    switch(streamObjectTypeId) {
    case SystemsIso144961: return GeneralMediaFormat::Systems;
    case SystemsIso144961v2: return MediaFormat(GeneralMediaFormat::Systems, 2);
    case InteractionStream: return GeneralMediaFormat::InteractionStream;
    case AfxStream: return GeneralMediaFormat::AfxStream;
    case FontDataStream: return GeneralMediaFormat::FontDataStream;
    case SynthesizedTextureStream: return GeneralMediaFormat::SynthesizedTextureStream;
    case StreamingTextStream: return GeneralMediaFormat::StreamingTextStream;
    case Mpeg4Visual: return GeneralMediaFormat::Mpeg4Video;
    case Avc: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4Avc);
    case ParameterSetsForAvc: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4AvcParams);
    case Als: return GeneralMediaFormat::Als;
    case Sa0c: return GeneralMediaFormat::Sa0c;
    case Aac: return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LowComplexityProfile);
    case Mpeg2VideoSimpleProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SimpleProfile);
    case Mpeg2VideoMainProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SnrProfile);
    case Mpeg2VideoSnrProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SpatialProfile);
    case Mpeg2VideoSpatialProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2HighProfile);
    case Mpeg2VideoHighProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2HighProfile);
    case Mpeg2Video422Profile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SimpleProfile);
    case Mpeg2AacMainProfile: return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2MainProfile);
    case Mpeg2AacLowComplexityProfile: return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2LowComplexityProfile);
    case Mpeg2AacScaleableSamplingRateProfile: return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2ScalableSamplingRateProfile);
    case Mpeg2Audio: return GeneralMediaFormat::Mpeg2Audio;
    case Mpeg1Video: return GeneralMediaFormat::Mpeg1Video;
    case Mpeg1Audio: return GeneralMediaFormat::Mpeg1Audio;
    case Jpeg: return GeneralMediaFormat::Jpeg;
    case Png: return GeneralMediaFormat::Png;
    case Evrc: case PrivateEvrc: return GeneralMediaFormat::Evrc;
    case Smv: return GeneralMediaFormat::Smv;
    case Gpp2Cmf: return GeneralMediaFormat::Gpp2Cmf;
    case Vc1: return GeneralMediaFormat::Vc1;
    case Dirac: return GeneralMediaFormat::Dirac;
    case Ac3: case PrivateAc3: return GeneralMediaFormat::Ac3;
    case EAc3: return GeneralMediaFormat::EAc3;
    case Dts: case PrivateDts: return GeneralMediaFormat::Dts;
    case DtsHdHighResolution: return MediaFormat(GeneralMediaFormat::DtsHd, SubFormats::DtsHdHighResolution);
    case DtsHdMasterAudio: return MediaFormat(GeneralMediaFormat::DtsHd, SubFormats::DtsHdMasterAudio);
    case DtsHdExpress: return MediaFormat(GeneralMediaFormat::DtsHd, SubFormats::DtsExpress);
    case PrivateOgg: case PrivateOgg2: return GeneralMediaFormat::Vorbis;
    case PrivateQcelp: return GeneralMediaFormat::Qcelp;
    default: return MediaFormat();
    }
}

}

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
const char *streamTypeName(byte streamTypeId)
{
    switch(streamTypeId) {
    case ObjectDescriptor: return "object descriptor";
    case ClockReference: return "clock reference";
    case SceneDescriptor: return "scene descriptor";
    case Visual: return "visual";
    case Audio: return "audio";
    case Mpeg7: return "MPEG-7";
    case Ipmps: return "IMPS";
    case ObjectContentInfo: return "object content info";
    case MpegJava: return "MPEG Java";
    case Interaction: return "interaction";
    case Ipmp: return "IPMP";
    case FontData: return "font data";
    case StreamingText: return "streaming text";
    default: return "";
    }
}

}

/*!
 * \brief Encapsulates all supported MPEG-4 audio object format IDs.
 * \sa http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio
 */
namespace Mpeg4AudioObjectIds {

LIB_EXPORT MediaFormat idToMediaFormat(byte mpeg4AudioObjectId, bool sbrPresent, bool psPresent)
{
    MediaFormat fmt;
    switch(mpeg4AudioObjectId) {
    case AacMain:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4MainProfile);
        break;
    case AacLc:
        fmt = MediaFormat(GeneralMediaFormat::Aac, sbrPresent ? SubFormats::AacMpeg4SpectralBandReplicationProfile : SubFormats::AacMpeg4LowComplexityProfile);
        break;
    case AacSsr:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4ScalableSamplingRateProfile);
        break;
    case AacLtp:
        fmt = MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LongTermPrediction);
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
    case Layer1:
        fmt = MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer1);
        break;
    case Layer2:
        fmt = MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer2);
        break;
    case Layer3:
        fmt = MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
        break;
    default:
        ;
    }
    if(sbrPresent) {
        fmt.extension |= ExtensionFormats::SpectralBandReplication;
    }
    if(psPresent) {
        fmt.extension |= ExtensionFormats::ParametricStereo;
    }
    return fmt;
}

}

}
