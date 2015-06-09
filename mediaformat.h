#ifndef MEDIAFORMAT_H
#define MEDIAFORMAT_H

#include <c++utilities/application/global.h>

#include <utility>

namespace Media {

/*!
 * \brief The MediaType enum specifies the type of media data (audio, video, text, ...).
 */
enum class MediaType
{
    Unknown,
    Audio,
    Video,
    Text,
    Hint
};

/*!
 * \brief The GeneralMediaFormat enum specifies the general format of media data (PCM, MPEG-4, PNG, ...).
 */
enum class GeneralMediaFormat
{
    Unknown,
    Aac, /**< Advanced Video Coding */
    Ac3, /**< Dolby Digital */
    Ac4, /**< AC-4 */
    AdpcmAcm, /**< ADPCM ACM */
    AfxStream, /**< AFX Stream */
    Alac, /**< Apple Lossless Audio Codec */
    Als, /**< ALS */
    Bitmap, /**< Windows Bitmap */
    Dirac, /**< Dirac */
    Dts, /**< DTS */
    DtsHd, /**< DTS-HD */
    EAc3, /**< Dolby Digital Plus */
    Evrc, /**< EVRC */
    Flac, /**< FLAC */
    FontDataStream, /**< Font Data Stream */
    Gif, /**< GIF */
    Gpp2Cmf, /**< 3GPP2 Compact Multimedia Format (CMF) */
    ImaadpcmAcm, /**< IMAADPCM ACM */
    ImageSubtitle, /**< Image subtitle */
    InteractionStream, /**< Interaction Stream */
    Jpeg, /**< JPEG */
    OggKate, /**< Karaoke And Text Encapsulation */
    MicrosoftAudioCodecManager, /**< Microsoft Audio Codec Manager (ACM) */
    MicrosoftVideoCodecManager, /**< Microsoft Video Codec Manager (VCM) */
    Mpeg1Audio, /**< MPEG-1 Audio */
    Mpeg1Video, /**< MPEG-1 Vudio */
    Mpeg2Audio, /**< MPEG-2 Audio */
    Mpeg2Video, /**< MPEG-2 Vudio */
    Mpeg4Video, /**< MPEG-4 */
    Mpc, /**< Musepack */
    Pcm, /**< Pulse Code Modulation */
    Png, /**< PNG */
    ProRes, /**< ProRes */
    Qcelp, /**< QCELP */
    QuicktimeAudio, /**< Quicktime Audio */
    QuicktimeVideo, /**< Quicktime Video */
    RealAudio, /**< Real Audio */
    RealVideo, /**< Real Video */
    Sa0c, /**< SAOC */
    Smv, /**< SMV */
    StreamingTextStream, /**< Streaming Text Stream */
    SynthesizedTextureStream, /**< Synthesized Texture Stream */
    Systems, /**< Systems */
    TextSubtitle, /**< Text subtitle */
    Theora, /**< Theora */
    Tiff, /**< TIFF */
    Tta, /**< The True Audio lessles audio compressor */
    UncompressedVideoFrames, /**< uncompressed RGB */
    Vc1, /**< VC-1 */
    VobBtn, /**< VobBtn */
    VobSub, /**< VobSub */
    Vorbis, /**< Vorbis */
    WavPack /**< WavPack */
};

/*!
 * \brief Encapsulates sub formats.
 *
 * For instance "Layer 3" is a sub format of MPEG-1 audio.
 */
namespace SubFormats {

enum : unsigned char {
    None
};

enum Mpeg1AudioLayer : unsigned char {
    Mpeg1Layer1 = 1,
    Mpeg1Layer2,
    Mpeg1Layer3
};

enum AacProfile : unsigned char {
    AacMpeg2MainProfile = 1,
    AacMpeg2LowComplexityProfile,
    AacMpeg2SpectralBandReplicationProfile,
    AacMpeg2ScalableSamplingRateProfile,
    AacMpeg4MainProfile,
    AacMpeg4LowComplexityProfile,
    AacMpeg4SpectralBandReplicationProfile,
    AacMpeg4ScalableSamplingRateProfile,
    AacMpeg4LongTermPrediction,
    AacMpeg4ERLowComplecityProfile,
    AacMpeg4ERScalableSampingRateProfile,
    AacMpeg4ERLongTermPrediction,
    AacMpeg4ERLowDelay,
    AacMpeg4EREnhancedLowDelay
};

enum Mpeg2VideoProfile : unsigned char {
    Mpeg2SimpleProfile = 1,
    Mpeg2MainProfile,
    Mpeg2SnrProfile,
    Mpeg2SpatialProfile,
    Mpeg2HighProfile,
    Mpeg2422Profile
};

enum Mpeg4VideoProfile : unsigned char {
    Mpeg4Sp = 1,
    Mpeg4Asp,
    Mpeg4Avc,
    Mpeg4AvcParams,
    Mpeg4MsV3
};

enum DtsSpecifier : unsigned char {
    DtsExpress = 1,
    DtsLossless,
    DtsHdHighResolution,
    DtsHdMasterAudio,
};

enum PcmVersion : unsigned char {
    PcmIntBe = 1,
    PcmIntLe,
    PcmFloatIeee
};

enum TextSubtitle : unsigned char {
    TextSubBasicUtf8 = 1,
    TextSubSubtitlesFormat,
    TextSubAdvancedSubtitlesFormat,
    TextSubUniversalSubtitleFormat
};

enum ImageSubtitle : unsigned char {
    ImgSubBmp = 1
};

}

/*!
 * \brief Encapsulates extension formats.
 */
namespace ExtensionFormats {
enum AudioFormatExtensions : unsigned char {
    SpectralBandReplication = 1,
    ParametricStereo = 2
};
}

class LIB_EXPORT MediaFormat
{
public:
    MediaFormat(GeneralMediaFormat general = GeneralMediaFormat::Unknown, unsigned char sub = 0, unsigned char extension = 0);

    const char *name() const;
    const char *abbreviation() const;
    operator bool() const;
    MediaFormat &operator+=(const MediaFormat &other);

    GeneralMediaFormat general;
    unsigned char sub;
    unsigned char extension;
};

/*!
 * \brief Constructs a new media format.
 */
inline MediaFormat::MediaFormat(GeneralMediaFormat general, unsigned char sub, unsigned char extension) :
    general(general),
    sub(sub),
    extension(extension)
{}

/*!
 * \brief "Adds" information from another instance to the object.
 */
inline MediaFormat &MediaFormat::operator+=(const MediaFormat &other)
{
    if(other) {
        general = other.general;
        if(other.sub) {
            sub = other.sub;
        }
        if(other.extension) {
            extension = other.extension;
        }
    }
    return *this;
}

/*!
 * \brief Returns whether the media format is known.
 */
inline MediaFormat::operator bool() const
{
    return general != GeneralMediaFormat::Unknown;
}

}

#endif // MEDIAFORMAT_H
