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
namespace Mp4FormatIds {

MediaFormat fourccToMediaFormat(uint32 fourccId)
{
    switch(fourccId) {
    case Mp4FormatIds::Mpeg4Video:
        return GeneralMediaFormat::Mpeg4Video;
    case Mp4FormatIds::Avc1:
    case Mp4FormatIds::Avc2:
    case Mp4FormatIds::Avc3:
    case Mp4FormatIds::Avc4:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4Avc);
    case Mp4FormatIds::H263:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4Asp);
    case Mp4FormatIds::Tiff:
        return GeneralMediaFormat::Tiff;
    case Mp4FormatIds::Raw:
        return GeneralMediaFormat::UncompressedVideoFrames;
    case Mp4FormatIds::Jpeg:
        return GeneralMediaFormat::Jpeg;
    case Mp4FormatIds::Gif:
        return GeneralMediaFormat::Gif;
    case Mp4FormatIds::AdpcmAcm:
        return GeneralMediaFormat::AdpcmAcm;
    case Mp4FormatIds::ImaadpcmAcm:
        return GeneralMediaFormat::ImaadpcmAcm;
    case Mp4FormatIds::Mp3CbrOnly:
        return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
    case Mp4FormatIds::Mpeg4Audio:
        return GeneralMediaFormat::Aac;
    case Mp4FormatIds::Alac:
        return GeneralMediaFormat::Alac;
    case Mp4FormatIds::Ac3:
        return GeneralMediaFormat::Ac3;
    case Mp4FormatIds::Ac4:
        return GeneralMediaFormat::Ac4;
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
    case Aac: return GeneralMediaFormat::Aac;
    case Mpeg2VideoSimpleProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SimpleProfile);
    case Mpeg2VideoMainProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SnrProfile);
    case Mpeg2VideoSnrProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SpatialProfile);
    case Mpeg2VideoSpatialProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2HighProfile);
    case Mpeg2VideoHighProfile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2HighProfile);
    case Mpeg2Video422Profile: return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg2SimpleProfile);
    case AacMainProfile: return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2MainProfile);
    case AacLowComplexityProfile: return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2LowComplexityProfile);
    case AacScaleableSamplingRateProfile: return MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg2ScalableSamplingRateProfile);
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
    default: "";
    }
}

}

/*!
 * \brief Encapsulates all supported MPEG-4 audio object format IDs.
 * \sa http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio
 */
namespace Mpeg4AudioObjectIds {
}

}
