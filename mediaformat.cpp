#include "mediaformat.h"

namespace Media {

using namespace SubFormats;

/*!
 * \class Media::MediaFormat
 * \brief The MediaFormat class specifies the format of media data.
 */

/*!
 * \brief Returns the name of the media format as C-style string.
 *
 * Returns an empty string if no name is available.
 */
const char *MediaFormat::name() const
{
    switch(general) {
    case GeneralMediaFormat::Aac:
        switch(sub) {
        case AacMpeg2MainProfile: return "Advanced Audio Coding Main Profile";
        case AacMpeg2LowComplexityProfile: return "Advanced Audio Coding Low Complexity Profile";
        case AacMpeg2SpectralBandReplicationProfile: return "Advanced Audio Coding Low Complexity with Spectral Band Replication Profile";
        case AacMpeg2ScalableSamplingRateProfile: return "Advanced Audio Coding Scalable Sampling Rate Profile";
        case AacMpeg4MainProfile: return "Advanced Audio Coding Main Profile";
        case AacMpeg4LowComplexityProfile: return "Advanced Audio Coding Low Complexity Profile";
        case AacMpeg4SpectralBandReplicationProfile: return "Advanced Audio Coding Low Complexity with Spectral Band Replication Profile";
        case AacMpeg4ScalableSamplingRateProfile: return "Advanced Audio Coding Scaleable Sampling Rate Profile";
        case AacMpeg4LongTermPrediction: return "Advanced Audio Coding Long Term Predicition";
        case AacMpeg4ERLowComplecityProfile: return "Advanced Audio Coding Error Resilient Low Complexity Profile";
        case AacMpeg4ERScalableSampingRateProfile: return "Advanced Audio Coding Error Resilient Scalable Sampling Rate Profile";
        case AacMpeg4ERLongTermPrediction: return "Advanced Audio Coding Error Resilient Long Term Predicition";
        case AacMpeg4ERLowDelay: return "Advanced Audio Coding Error Resilient Low Delay";
        case AacMpeg4EREnhancedLowDelay: return "Advanced Audio Coding Error Resilient Enhanced Low Delay";
        default: return "Advanced Audio Coding";
        }
    case GeneralMediaFormat::Ac3: return "Dolby Digital";
    case GeneralMediaFormat::Ac4: return "AC-4";
    case GeneralMediaFormat::AdpcmAcm: return "ADPCM ACM";
    case GeneralMediaFormat::AfxStream: return "AFX Stream";
    case GeneralMediaFormat::Alac: return "Apple Lossless Audio Codec";
    case GeneralMediaFormat::Als: return "ALS";
    case GeneralMediaFormat::Bitmap: return "Windows Bitmap";
    case GeneralMediaFormat::Dirac: return "Dirac";
    case GeneralMediaFormat::Dts: return "DTS";
        switch(sub) {
        case DtsLossless: return "DTS Lossless";
        case DtsExpress: return "DTS Express";
        default: return "DTS";
        }
    case GeneralMediaFormat::DtsHd: return "DTS-HD";
        switch(sub) {
        case DtsHdHighResolution: return "DTS-HD High Resolution";
        case DtsHdMasterAudio: return "DTS-HD Master Audio";
        case DtsExpress: return "DTS-HD Express";
        default: return "DTS-HD";
        }
    case GeneralMediaFormat::EAc3: return "Dolby Digital Plus";
    case GeneralMediaFormat::Evrc: return "EVRC";
    case GeneralMediaFormat::Flac: return "Free Lossless Audio Codec";
    case GeneralMediaFormat::FontDataStream: return "Font Data Stream";
    case GeneralMediaFormat::Gif: return "GIF";
    case GeneralMediaFormat::Gpp2Cmf: return "3GPP2 Compact Multimedia Format (CMF)";
    case GeneralMediaFormat::ImaadpcmAcm: return "IMAADPCM ACM";
    case GeneralMediaFormat::ImageSubtitle:
        switch(sub) {
        case SubFormats::ImgSubBmp: return "Bitmap subtitle";
        default: return "Image subtitle";
        }
    case GeneralMediaFormat::InteractionStream: return "Interaction Stream";
    case GeneralMediaFormat::Jpeg: return "JPEG";
    case GeneralMediaFormat::OggKate: return "Karaoke And Text Encapsulation";
    case GeneralMediaFormat::MicrosoftAudioCodecManager: return "Microsoft Audio Codec Manager";
    case GeneralMediaFormat::MicrosoftVideoCodecManager: return "Microsoft Video Codec Manager";
    case GeneralMediaFormat::Mpeg1Audio:
        switch(sub) {
        case Mpeg1Layer1: return "MPEG-1 Layer 1";
        case Mpeg1Layer2: return "MPEG-1 Layer 2";
        case Mpeg1Layer3: return "MPEG-1 Layer 3";
        default: return "MPEG-1 Audio";
        }
    case GeneralMediaFormat::Mpeg1Video: return "MPEG-1 Video";
    case GeneralMediaFormat::Mpeg2Audio:
        switch(sub) {
        case Mpeg1Layer1: return "MPEG-2 Layer 1";
        case Mpeg1Layer2: return "MPEG-2 Layer 2";
        case Mpeg1Layer3: return "MPEG-2 Layer 3";
        default: return "MPEG-2 Audio";
        }
    case GeneralMediaFormat::Mpeg2Video:
        switch(sub) {
        case Mpeg2SimpleProfile: return "MPEG-2 Video Simple Profile";
        case Mpeg2MainProfile: return "MPEG-2 Video Main Profile";
        case Mpeg2SnrProfile: return "MPEG-2 Video SNR Profile";
        case Mpeg2SpatialProfile: return "MPEG-2 Video Spatial Profile";
        case Mpeg2HighProfile: return "MPEG-2 Video High Profile";
        case Mpeg2422Profile: return "MPEG-2 Video 422 Profile";
        default: return "MPEG-2 Video";
        }
    case GeneralMediaFormat::Mpeg4Video:
        switch(sub) {
        case Mpeg4Sp: return "MPEG-4 Simple Profile";
        case Mpeg4Asp: return "MPEG-4 Advanced Simple Profile";
        case Mpeg4Avc: return "MPEG-4 Advanced Video Coding";
        case Mpeg4AvcParams: return "Parameter for MPEG-4 Advanced Video Coding";
        case Mpeg4MsV3: return "MPEG-4 Microsoft V3";
        default: return "MPEG-4 Visual";
        }
    case GeneralMediaFormat::Mpc: return "Musepack SV8";
    case GeneralMediaFormat::Pcm:
        switch(sub) {
        case PcmIntBe: return "Pulse Code Modulation (integer, big endian)";
        case PcmIntLe: return "Pulse Code Modulation (integer, little endian)";
        case PcmFloatIeee: return "Pulse Code Modulation (float, IEEE)";
        default: return "Pulse Code Modulation";
        }
    case GeneralMediaFormat::Png: return "Portable Network Graphics";
    case GeneralMediaFormat::ProRes: return "ProRes";
    case GeneralMediaFormat::Qcelp: return "QCELP";
    case GeneralMediaFormat::QuicktimeAudio: return "Quicktime Audio";
    case GeneralMediaFormat::QuicktimeVideo: return "Quicktime Video";
    case GeneralMediaFormat::RealAudio: return "Real Audio";
    case GeneralMediaFormat::RealVideo: return "Real Video";
    case GeneralMediaFormat::Sa0c: return "SAOC";
    case GeneralMediaFormat::Smv: return "SMV";
    case GeneralMediaFormat::StreamingTextStream: return "Streaming Text Stream";
    case GeneralMediaFormat::SynthesizedTextureStream: return "Synthesized Texture Stream";
    case GeneralMediaFormat::Systems: return "Systems";
    case GeneralMediaFormat::TextSubtitle:
        switch(sub) {
        case SubFormats::TextSubBasicUtf8: return "UTF-8 Plain Text subtitles";
        case SubFormats::TextSubSubtitlesFormat: return "Subtitles Format";
        case SubFormats::TextSubAdvancedSubtitlesFormat: return "Advanced Subtitles Format";
        case SubFormats::TextSubUniversalSubtitleFormat: return "Universal Subtitle Format";
        default: return "Text subtitle";
        }
    case GeneralMediaFormat::Theora: return "Theora";
    case GeneralMediaFormat::Tiff: return "Tagged Image File Format";
    case GeneralMediaFormat::TimedText: return "Timed Text";
    case GeneralMediaFormat::Tta: return "The True Audio";
    case GeneralMediaFormat::UncompressedVideoFrames: return "uncompressed video frames";
    case GeneralMediaFormat::Vc1: return "Windows Media Video";
    case GeneralMediaFormat::VobBtn: return "VobBtn Buttons";
    case GeneralMediaFormat::VobSub: return "VobSub";
    case GeneralMediaFormat::Vorbis: return "Vorbis";
    case GeneralMediaFormat::Vp8: return "VP8";
    case GeneralMediaFormat::WavPack: return "WavPack";
    default: return "unknown";
    }
}

/*!
 * \brief Returns the abbreviation of the media format as C-style string.
 *
 * Returns an empty string if no abbreviation is available.
 */
const char *MediaFormat::abbreviation() const
{
    switch(general) {
    case GeneralMediaFormat::Aac:
        switch(sub) {
        case AacMpeg2MainProfile: return "MPEG-2 AAC Main";
        case AacMpeg2LowComplexityProfile: return "MPEG-2 AAC-LC";
        case AacMpeg2SpectralBandReplicationProfile: return "MPEG-2-SBR";
        case AacMpeg2ScalableSamplingRateProfile: return "MPEG-2 AAC-SSR";
        case AacMpeg4MainProfile: return "MPEG-4 AAC Main";
        case AacMpeg4LowComplexityProfile: return "MPEG-4 AAC-LC";
        case AacMpeg4SpectralBandReplicationProfile: return "MPEG-4 HE-AAC";
        case AacMpeg4ScalableSamplingRateProfile: return "MPEG-4 AAC-SSR";
        case AacMpeg4LongTermPrediction: return "MPEG-4 AAC-LTP";
        case AacMpeg4ERLowComplecityProfile: return "MPEG-4 ER AAC-LC";
        case AacMpeg4ERScalableSampingRateProfile: return "MPEG-4 ER AAC-LC";
        case AacMpeg4ERLongTermPrediction: return "MPEG-4 ER AAC-LTP";
        case AacMpeg4ERLowDelay: return "MPEG-4 ER AAC-LD";
        case AacMpeg4EREnhancedLowDelay: return "MPEG-4 ER AAC-ELD";
        default: return "AAC";
        }
    case GeneralMediaFormat::Ac3: return "AC-3";
    case GeneralMediaFormat::Ac4: return "AC-4";
    case GeneralMediaFormat::AdpcmAcm: return "ADPCM ACM";
    case GeneralMediaFormat::AfxStream: return "AFX";
    case GeneralMediaFormat::Alac: return "ALAC";
    case GeneralMediaFormat::Als: return "ALS";
    case GeneralMediaFormat::Bitmap: return "BMP";
    case GeneralMediaFormat::Dirac: return "Dirac";
    case GeneralMediaFormat::Dts: return "DTS";
        switch(sub) {
        case DtsLossless: return "DTS Lossless";
        case DtsExpress: return "DTS LBR";
        default: return "DTS";
        }
    case GeneralMediaFormat::DtsHd: return "DTS-HD";
        switch(sub) {
        case DtsHdHighResolution: return "DTS-HD High Resolution";
        case DtsHdMasterAudio: return "DTS-HD Master Audio";
        case DtsExpress: return "DTS-HD Express";
        default: return "DTS-HD";
        }
    case GeneralMediaFormat::EAc3: return "E-AC-3";
    case GeneralMediaFormat::Evrc: return "EVRC";
    case GeneralMediaFormat::Flac: return "FLAC";
    case GeneralMediaFormat::FontDataStream: return "FDS";
    case GeneralMediaFormat::Gif: return "GIF";
    case GeneralMediaFormat::Gpp2Cmf: return "3GPP2 CMF";
    case GeneralMediaFormat::ImaadpcmAcm: return "IMAADPCM ACM";
    case GeneralMediaFormat::ImageSubtitle:
        switch(sub) {
        case SubFormats::ImgSubBmp: return "BMP subtitle";
        default: return "Image subtitle";
        }
    case GeneralMediaFormat::InteractionStream: return "Interaction Stream";
    case GeneralMediaFormat::Jpeg: return "JPEG";
    case GeneralMediaFormat::OggKate: return "OggKate";
    case GeneralMediaFormat::MicrosoftAudioCodecManager: return "MS ACM";
    case GeneralMediaFormat::MicrosoftVideoCodecManager: return "MS VCM";
    case GeneralMediaFormat::Mpeg1Audio:
        switch(sub) {
        case Mpeg1Layer1: return "MP1";
        case Mpeg1Layer2: return "MP2";
        case Mpeg1Layer3: return "MP3";
        default: return "MPEG-1 Audio";
        }
    case GeneralMediaFormat::Mpeg1Video: return "MP1";
    case GeneralMediaFormat::Mpeg2Audio:
        switch(sub) {
        case Mpeg1Layer1: return "MP1";
        case Mpeg1Layer2: return "MP2";
        case Mpeg1Layer3: return "MP3";
        default: return "MPEG-2 Audio";
        }
    case GeneralMediaFormat::Mpeg2Video:
        switch(sub) {
        case Mpeg2SimpleProfile: return "MPEG-2 SP";
        case Mpeg2MainProfile: return "MPEG-2 Main";
        case Mpeg2SnrProfile: return "MPEG-2 SNR";
        case Mpeg2SpatialProfile: return "MPEG-2 Spatial";
        case Mpeg2HighProfile: return "MPEG-2 High";
        case Mpeg2422Profile: return "MPEG-2 422";
        default: return "MPEG-2 Video";
        }
    case GeneralMediaFormat::Mpeg4Video:
        switch(sub) {
        case Mpeg4Sp: return "MPEG-4 SP";
        case Mpeg4Asp: return "H.263";
        case Mpeg4Avc: return "H.264";
        case Mpeg4AvcParams: return "H.264 params";
        case Mpeg4MsV3: return "MPEG-4 MS V3";
        default: return "MPEG-4 Visual";
        }
    case GeneralMediaFormat::Mpc: return "MPC";
    case GeneralMediaFormat::Pcm:
        switch(sub) {
        case PcmIntBe: return "PCM (int, BE)";
        case PcmIntLe: return "PCM (int, LE)";
        case PcmFloatIeee: return "PCM IEEE";
        default: return "PCM";
        }
    case GeneralMediaFormat::Png: return "PNG";
    case GeneralMediaFormat::ProRes: return "ProRes";
    case GeneralMediaFormat::Qcelp: return "QCELP";
    case GeneralMediaFormat::QuicktimeAudio: return "Quicktime Audio";
    case GeneralMediaFormat::QuicktimeVideo: return "Quicktime Video";
    case GeneralMediaFormat::RealAudio: return "Real Audio";
    case GeneralMediaFormat::RealVideo: return "Real Video";
    case GeneralMediaFormat::Sa0c: return "SAOC";
    case GeneralMediaFormat::Smv: return "SMV";
    case GeneralMediaFormat::StreamingTextStream: return "Streaming Text Stream";
    case GeneralMediaFormat::SynthesizedTextureStream: return "Synthesized Texture Stream";
    case GeneralMediaFormat::Systems:
        switch(sub) {
        case 2: return "Systems v2";
        default: return "Systems";
        }
    case GeneralMediaFormat::TextSubtitle:
        switch(sub) {
        case SubFormats::TextSubBasicUtf8: return "UTF-8 Sub";
        case SubFormats::TextSubSubtitlesFormat: return "SSA";
        case SubFormats::TextSubAdvancedSubtitlesFormat: return "ASS";
        case SubFormats::TextSubUniversalSubtitleFormat: return "USF";
        default: return "Text subtitle";
        }
    case GeneralMediaFormat::Theora: return "Theora";
    case GeneralMediaFormat::Tiff: return "TIFF";
    case GeneralMediaFormat::TimedText: return "Timed Text";
    case GeneralMediaFormat::Tta: return "TTA";
    case GeneralMediaFormat::UncompressedVideoFrames: return "uncompressed video frames";
    case GeneralMediaFormat::Vc1: return "VC-1";
    case GeneralMediaFormat::VobBtn: return "VobBtn";
    case GeneralMediaFormat::VobSub: return "VobSub";
    case GeneralMediaFormat::Vorbis: return "Vorbis";
    case GeneralMediaFormat::Vp8: return "VP8";
    case GeneralMediaFormat::WavPack: return "WavPack";
    default: return "";
    }
}

}
