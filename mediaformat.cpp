#include "./mediaformat.h"

namespace TagParser {

using namespace SubFormats;

/*!
 * \class TagParser::MediaFormat
 * \brief The MediaFormat class specifies the format of media data.
 */

/*!
 * \brief Returns the name of the media format as C-style string.
 *
 * Returns an empty string if no name is available.
 */
std::string_view MediaFormat::name() const
{
    switch (general) {
    case GeneralMediaFormat::Aac:
        switch (sub) {
        case AacMpeg2MainProfile:
            return "Advanced Audio Coding Main Profile";
        case AacMpeg2LowComplexityProfile:
            return "Advanced Audio Coding Low Complexity Profile";
        case AacMpeg2ScalableSamplingRateProfile:
            return "Advanced Audio Coding Scalable Sampling Rate Profile";
        case AacMpeg4MainProfile:
            return "Advanced Audio Coding Main Profile";
        case AacMpeg4LowComplexityProfile:
            return "Advanced Audio Coding Low Complexity Profile";
        case AacMpeg4ScalableSamplingRateProfile:
            return "Advanced Audio Coding Scalable Sampling Rate Profile";
        case AacMpeg4LongTermPrediction:
            return "Advanced Audio Coding Long Term Predicition";
        case AacMpeg4ERLowComplecityProfile:
            return "Advanced Audio Coding Error Resilient Low Complexity Profile";
        case AacMpeg4ERScalableSampingRateProfile:
            return "Advanced Audio Coding Error Resilient Scalable Sampling Rate Profile";
        case AacMpeg4ERLongTermPrediction:
            return "Advanced Audio Coding Error Resilient Long Term Predicition";
        case AacMpeg4ERLowDelay:
            return "Advanced Audio Coding Error Resilient Low Delay";
        case AacMpeg4EREnhancedLowDelay:
            return "Advanced Audio Coding Error Resilient Enhanced Low Delay";
        default:
            return "Advanced Audio Coding";
        }
    case GeneralMediaFormat::Ac3:
        return "Dolby Digital";
    case GeneralMediaFormat::Ac4:
        return "AC-4";
    case GeneralMediaFormat::AdpcmAcm:
        return "ADPCM ACM";
    case GeneralMediaFormat::AfxStream:
        return "AFX Stream";
    case GeneralMediaFormat::Alac:
        return "Apple Lossless Audio Codec";
    case GeneralMediaFormat::Als:
        return "ALS";
    case GeneralMediaFormat::Amr:
        return "Adaptive Multi-Rate audio codec";
    case GeneralMediaFormat::Avc:
        switch (sub) {
        case AvcCavlc444IntraProfile:
            return "Advanced Video Coding CAVLC 4:4:4 Intra Profile";
        case AvcBaselineProfile:
            return "Advanced Video Coding Basline Profile";
        case AvcMainProfile:
            return "Advanced Video Coding Main Profile";
        case AvcScalableBaselineProfile:
            return "Advanced Video Coding Scalable Basline Profile";
        case AvcScalableHighProfile:
            return "Advanced Video Coding Scalable High Profile";
        case AvcExtendedProfile:
            return "Advanced Video Coding Extended Profile";
        case AvcHighProfile:
            return "Advanced Video Coding High Profile";
        case AvcHigh10Profile:
            return "Advanced Video Coding High 10 Profile";
        case AvcHighMultiviewProfile:
            return "Advanced Video Coding Multiview Profile";
        case AvcHigh422Profile:
            return "Advanced Video Coding High 4:2:2 Profile";
        case AvcStereoHighProfile:
            return "Advanced Video Coding Stereo High Profile";
        case AvcHighMultiviewDepthProfile:
            return "Advanced Video Coding Multiview Depth High Profile";
        case AvcHigh444Profile:
            return "Advanced Video Coding High 4:4:4 Profile";
        case AvcHigh444PredictiveProfile:
            return "Advanced Video Coding High 4:4:4 Predictive Profile";
        default:
            return "Advanced Video Coding";
        }
    case GeneralMediaFormat::Av1:
        return "AOMedia Video 1";
    case GeneralMediaFormat::Bitmap:
        return "Windows Bitmap";
    case GeneralMediaFormat::Daala:
        return "Daala";
    case GeneralMediaFormat::Dirac:
        return "Dirac";
    case GeneralMediaFormat::Dts:
        switch (sub) {
        case DtsLossless:
            return "DTS Lossless";
        case DtsExpress:
            return "DTS Express";
        default:
            return "DTS";
        }
    case GeneralMediaFormat::DtsHd:
        switch (sub) {
        case DtsHdHighResolution:
            return "DTS-HD High Resolution";
        case DtsHdMasterAudio:
            return "DTS-HD Master Audio";
        case DtsExpress:
            return "DTS-HD Express";
        default:
            return "DTS-HD";
        }
    case GeneralMediaFormat::EAc3:
        return "Dolby Digital Plus";
    case GeneralMediaFormat::Evrc:
        return "EVRC";
    case GeneralMediaFormat::Flac:
        return "Free Lossless Audio Codec";
    case GeneralMediaFormat::FontDataStream:
        return "Font Data Stream";
    case GeneralMediaFormat::Gif:
        return "GIF";
    case GeneralMediaFormat::Gpp2Cmf:
        return "3GPP2 Compact Multimedia Format (CMF)";
    case GeneralMediaFormat::Hevc:
        return "High Efficiency Video Coding";
    case GeneralMediaFormat::Vcc:
        return "Versatile Video Coding";
    case GeneralMediaFormat::ImaadpcmAcm:
        return "IMAADPCM ACM";
    case GeneralMediaFormat::ImageSubtitle:
        switch (sub) {
        case SubFormats::ImgSubBmp:
            return "Bitmap subtitle";
        default:
            return "Image subtitle";
        }
    case GeneralMediaFormat::InteractionStream:
        return "Interaction Stream";
    case GeneralMediaFormat::Jpeg:
        return "JPEG";
    case GeneralMediaFormat::OggKate:
        return "Karaoke And Text Encapsulation";
    case GeneralMediaFormat::Opus:
        return "Opus";
    case GeneralMediaFormat::MicrosoftAudioCodecManager:
        return "Microsoft Audio Codec Manager";
    case GeneralMediaFormat::MicrosoftMpeg4:
        switch (sub) {
        case 1:
            return "Microsoft MPEG-4 version 1";
        case 2:
            return "Microsoft MPEG-4 version 2";
        case 3:
            return "Microsoft MPEG-4 version 3";
        default:
            return "Microsoft MPEG-4";
        }
    case GeneralMediaFormat::MicrosoftVideoCodecManager:
        return "Microsoft Video Codec Manager";
    case GeneralMediaFormat::DolbyMlp:
        return "Dolby TrueHD";
    case GeneralMediaFormat::Mpeg1Audio:
        switch (sub) {
        case Mpeg1Layer1:
            return "MPEG-1 Layer 1";
        case Mpeg1Layer2:
            return "MPEG-1 Layer 2";
        case Mpeg1Layer3:
            return "MPEG-1 Layer 3";
        default:
            return "MPEG-1 Audio";
        }
    case GeneralMediaFormat::Mpeg1Video:
        return "MPEG-1 Video";
    case GeneralMediaFormat::Mpeg2Audio:
        switch (sub) {
        case Mpeg1Layer1:
            return "MPEG-2 Layer 1";
        case Mpeg1Layer2:
            return "MPEG-2 Layer 2";
        case Mpeg1Layer3:
            return "MPEG-2 Layer 3";
        default:
            return "MPEG-2 Audio";
        }
    case GeneralMediaFormat::Mpeg2Video:
        switch (sub) {
        case Mpeg2SimpleProfile:
            return "MPEG-2 Video Simple Profile";
        case Mpeg2MainProfile:
            return "MPEG-2 Video Main Profile";
        case Mpeg2SnrProfile:
            return "MPEG-2 Video SNR Profile";
        case Mpeg2SpatialProfile:
            return "MPEG-2 Video Spatial Profile";
        case Mpeg2HighProfile:
            return "MPEG-2 Video High Profile";
        case Mpeg2422Profile:
            return "MPEG-2 Video 422 Profile";
        default:
            return "MPEG-2 Video";
        }
    case GeneralMediaFormat::Mpeg4Video:
        switch (sub) {
        case Mpeg4SimpleProfile1:
            return "MPEG-4 Simple Profile L1";
        case Mpeg4SimpleProfile2:
            return "MPEG-4 Simple Profile L2";
        case Mpeg4SimpleProfile3:
            return "MPEG-4 Simple Profile L2";
        case Mpeg4SimpleProfile0:
            return "MPEG-4 Simple Profile";
        case Mpeg4SimpleScalableProfile0:
            return "MPEG-4 Simple Scalable Profile";
        case Mpeg4SimpleScalableProfile1:
            return "MPEG-4 Simple Scalable Profile L1";
        case Mpeg4SimpleScalableProfile2:
            return "MPEG-4 Simple Scalable Profile L2";
        case Mpeg4CoreProfile1:
            return "MPEG-4 Core Profile L1";
        case Mpeg4CoreProfiel2:
            return "MPEG-4 Core Profile L2";
        case Mpeg4MainProfile2:
            return "MPEG-4 Main Profile L2";
        case Mpeg4MainProfile3:
            return "MPEG-4 Main Profile L3";
        case Mpeg4MainProfile4:
            return "MPEG-4 Main Profile L4";
        case Mpeg4NBitPrifle2:
            return "MPEG-4 N-Bit Profile L2";
        case Mpeg4ScalableTextureProfile1:
            return "MPEG-4 Scalable Texture Profile L1";
        case Mpeg4SimpleFaceAnimationProfile1:
            return "MPEG-4 Simple Face Animation Profile L1";
        case Mpeg4SimpleFaceAnimationProfile2:
            return "MPEG-4 Simple Face Animation Profile L2";
        case Mpeg4SimpleFbaProfile1:
            return "MPEG-4 Simple FBA Profile L1";
        case Mpeg4SimpleFbaProfile2:
            return "MPEG-4 Simple FBA Profile L2";
        case Mpeg4BasicAnimatedTextureProfiel1:
            return "MPEG-4 Basic Animated Texture Profile L1";
        case Mpeg4BasicAnimatedTextureProfiel2:
            return "MPEG-4 Basic Animated Texture Profile L2";
        case Mpeg4AvcProfile:
            return "MPEG-4 Advanced Audio Coding Profile";
        case Mpeg4HybridProfile1:
            return "MPEG-4 Hybrid Profile L1";
        case Mpeg4HybridProfile2:
            return "MPEG-4 Hybrid Profile L2";
        case Mpeg4AdvancedRealTimeSimpleProfile1:
            return "MPEG-4 Basic Animated Texture Profile L1";
        case Mpeg4AdvancedRealTimeSimpleProfile2:
            return "MPEG-4 Basic Animated Texture Profile L2";
        case Mpeg4AdvancedRealTimeSimpleProfile3:
            return "MPEG-4 Basic Animated Texture Profile L3";
        case Mpeg4AdvancedRealTimeSimpleProfile4:
            return "MPEG-4 Basic Animated Texture Profile L4";
        case Mpeg4CoreScalableProfile1:
            return "MPEG-4 Core Scalable Profile L1";
        case Mpeg4CoreScalableProfile2:
            return "MPEG-4 Core Scalable Profile L2";
        case Mpeg4CoreScalableProfile3:
            return "MPEG-4 Core Scalable Profile L3";
        case Mpeg4AdvancedCodingEfficiencyProfile1:
            return "MPEG-4 Advanced Coding Efficiency Profile L1";
        case Mpeg4AdvancedCodingEfficiencyProfile2:
            return "MPEG-4 Advanced Coding Efficiency Profile L2";
        case Mpeg4AdvancedCodingEfficiencyProfile3:
            return "MPEG-4 Advanced Coding Efficiency Profile L3";
        case Mpeg4AdvancedCodingEfficiencyProfile4:
            return "MPEG-4 Advanced Coding Efficiency Profile L4";
        case Mpeg4AdvancedCoreProfile1:
            return "MPEG-4 Advanced Core Profile L1";
        case Mpeg4AdvancedCoreProfile2:
            return "MPEG-4 Advanced Core Profile L2";
        case Mpeg4AdvancedScalableTexture1:
            return "MPEG-4 Advanced Scalable Texture L1";
        case Mpeg4AdvancedScalableTexture2:
            return "MPEG-4 Advanced Scalable Texture L2";
        case Mpeg4SimpleStudioProfile1:
            return "MPEG-4 Simple Studio Profile L1";
        case Mpeg4SimpleStudioProfile2:
            return "MPEG-4 Simple Studio Profile L2";
        case Mpeg4SimpleStudioProfile3:
            return "MPEG-4 Simple Studio Profile L3";
        case Mpeg4SimpleStudioProfile4:
            return "MPEG-4 Simple Studio Profile L4";
        case Mpeg4CoreStudioProfile1:
            return "MPEG-4 Core Studio Profile L1";
        case Mpeg4CoreStudioProfile2:
            return "MPEG-4 Core Studio Profile L2";
        case Mpeg4CoreStudioProfile3:
            return "MPEG-4 Core Studio Profile L3";
        case Mpeg4CoreStudioProfile4:
            return "MPEG-4 Core Studio Profile L4";
        case Mpeg4AdvancedSimpleProfile0:
            return "MPEG-4 Advanced Simple Profile";
        case Mpeg4AdvancedSimpleProfile1:
            return "MPEG-4 Advanced Simple Profile L1";
        case Mpeg4AdvancedSimpleProfile2:
            return "MPEG-4 Advanced Simple Profile L2";
        case Mpeg4AdvancedSimpleProfile3:
            return "MPEG-4 Advanced Simple Profile L3";
        case Mpeg4AdvancedSimpleProfile4:
            return "MPEG-4 Advanced Simple Profile L4";
        case Mpeg4AdvancedSimpleProfile5:
            return "MPEG-4 Advanced Simple Profile L5";
        case Mpeg4AdvancedSimpleProfile3b:
            return "MPEG-4 Advanced Simple Profile L3b";
        case Mpeg4FineGranularityScalableProfile0:
            return "MPEG-4 Fine Granularity Scalable Profile";
        case Mpeg4FineGranularityScalableProfile1:
            return "MPEG-4 Fine Granularity Scalable Profile L1";
        case Mpeg4FineGranularityScalableProfile2:
            return "MPEG-4 Fine Granularity Scalable Profile L2";
        case Mpeg4FineGranularityScalableProfile3:
            return "MPEG-4 Fine Granularity Scalable Profile L3";
        case Mpeg4FineGranularityScalableProfile4:
            return "MPEG-4 Fine Granularity Scalable Profile L4";
        case Mpeg4FineGranularityScalableProfile5:
            return "MPEG-4 Fine Granularity Scalable Profile L5";
        default:
            return "MPEG-4 Visual";
        }
    case GeneralMediaFormat::Mpeg4TimedText:
        return "MPEG-4 Timed Text";
    case GeneralMediaFormat::Mpc:
        return "Musepack SV8";
    case GeneralMediaFormat::Pcm:
        switch (sub) {
        case PcmIntBe:
            return "Pulse Code Modulation (integer, big endian)";
        case PcmIntLe:
            return "Pulse Code Modulation (integer, little endian)";
        case PcmFloatIeee:
            return "Pulse Code Modulation (float, IEEE)";
        default:
            return "Pulse Code Modulation";
        }
    case GeneralMediaFormat::Png:
        return "Portable Network Graphics";
    case GeneralMediaFormat::ProRes:
        return "ProRes";
    case GeneralMediaFormat::Qcelp:
        return "QCELP";
    case GeneralMediaFormat::QuicktimeAudio:
        return "Quicktime Audio";
    case GeneralMediaFormat::QuicktimeVideo:
        return "Quicktime Video";
    case GeneralMediaFormat::RealAudio:
        return "Real Audio";
    case GeneralMediaFormat::RealVideo:
        return "Real Video";
    case GeneralMediaFormat::Sa0c:
        return "SAOC";
    case GeneralMediaFormat::Smv:
        return "SMV";
    case GeneralMediaFormat::StreamingTextStream:
        return "Streaming Text Stream";
    case GeneralMediaFormat::SynthesizedTextureStream:
        return "Synthesized Texture Stream";
    case GeneralMediaFormat::Systems:
        switch (sub) {
        case 2:
            return "Systems v2";
        default:
            return "Systems";
        }
    case GeneralMediaFormat::TextSubtitle:
        switch (sub) {
        case SubFormats::PlainUtf8Subtitle:
            return "plain UTF-8 subtitle";
        case SubFormats::SubStationAlpha:
            return "SubStation Alpha";
        case SubFormats::AdvancedSubStationAlpha:
            return "Advanced SubStation Alpha";
        case SubFormats::UniversalSubtitleFormat:
            return "Universal Subtitle Format";
        case SubFormats::WebVideoTextTracksFormat:
            return "Web Video Text Tracks Format";
        default:
            return "Text subtitle";
        }
    case GeneralMediaFormat::Theora:
        return "Theora";
    case GeneralMediaFormat::Tiff:
        return "Tagged Image File Format";
    case GeneralMediaFormat::TimedText:
        return "Timed Text";
    case GeneralMediaFormat::Tta:
        return "The True Audio";
    case GeneralMediaFormat::UncompressedVideoFrames:
        return "uncompressed video frames";
    case GeneralMediaFormat::Vc1:
        return "Windows Media Video";
    case GeneralMediaFormat::VobBtn:
        return "VobBtn Buttons";
    case GeneralMediaFormat::VobSub:
        return "VobSub";
    case GeneralMediaFormat::Vorbis:
        return "Vorbis";
    case GeneralMediaFormat::Vp8:
        return "VP8";
    case GeneralMediaFormat::Vp9:
        return "VP9";
    case GeneralMediaFormat::WavPack:
        return "WavPack";
    case GeneralMediaFormat::WindowsMediaAudio:
        return "Windows Media Audio";
    case GeneralMediaFormat::WindowsMediaVideo:
        switch (sub) {
        case 1:
            return "Windows Media Video v1/v7";
        case 2:
            return "Windows Media Video v2/v8";
        default:
            return "Windows Media Video";
        }
    case GeneralMediaFormat::DvbSub:
        return "DVB subtitles";
    case GeneralMediaFormat::Speex:
        return "Speex";
    case GeneralMediaFormat::MonkeysAudio:
        return "Monkey's Audio";
    default:
        return "unknown";
    }
}

/*!
 * \brief Returns the abbreviation of the media format as C-style string.
 *
 * Returns an empty string if no abbreviation is available.
 */
std::string_view MediaFormat::abbreviation() const
{
    switch (general) {
    case GeneralMediaFormat::Aac:
        switch (sub) {
        case AacMpeg2MainProfile:
            return "MPEG-2 AAC Main";
        case AacMpeg2LowComplexityProfile:
            return "MPEG-2 AAC-LC";
        case AacMpeg2ScalableSamplingRateProfile:
            return "MPEG-2 AAC-SSR";
        case AacMpeg4MainProfile:
            return "MPEG-4 AAC Main";
        case AacMpeg4LowComplexityProfile:
            return "MPEG-4 AAC-LC";
        case AacMpeg4ScalableSamplingRateProfile:
            return "MPEG-4 AAC-SSR";
        case AacMpeg4LongTermPrediction:
            return "MPEG-4 AAC-LTP";
        case AacMpeg4ERLowComplecityProfile:
            return "MPEG-4 ER AAC-LC";
        case AacMpeg4ERScalableSampingRateProfile:
            return "MPEG-4 ER AAC-LC";
        case AacMpeg4ERLongTermPrediction:
            return "MPEG-4 ER AAC-LTP";
        case AacMpeg4ERLowDelay:
            return "MPEG-4 ER AAC-LD";
        case AacMpeg4EREnhancedLowDelay:
            return "MPEG-4 ER AAC-ELD";
        default:
            return "AAC";
        }
    case GeneralMediaFormat::Ac3:
        return "AC-3";
    case GeneralMediaFormat::Ac4:
        return "AC-4";
    case GeneralMediaFormat::AdpcmAcm:
        return "ADPCM ACM";
    case GeneralMediaFormat::AfxStream:
        return "AFX";
    case GeneralMediaFormat::Alac:
        return "ALAC";
    case GeneralMediaFormat::Als:
        return "ALS";
    case GeneralMediaFormat::Amr:
        return "AMR";
    case GeneralMediaFormat::Avc:
        switch (sub) {
        case AvcCavlc444IntraProfile:
            return "H.264 CAVLC 4:4:4 Intra";
        case AvcBaselineProfile:
            return "H.264 Basline";
        case AvcMainProfile:
            return "H.264 Main";
        case AvcScalableBaselineProfile:
            return "H.264 Scalable Basline";
        case AvcScalableHighProfile:
            return "H.264 Scalable High";
        case AvcExtendedProfile:
            return "H.264 Extended";
        case AvcHighProfile:
            return "H.264 High";
        case AvcHigh10Profile:
            return "H.264 High 10";
        case AvcHighMultiviewProfile:
            return "H.264 Multiview";
        case AvcHigh422Profile:
            return "H.264 High 4:2:2";
        case AvcStereoHighProfile:
            return "H.264 Stereo High";
        case AvcHighMultiviewDepthProfile:
            return "H.264 Multiview Depth High";
        case AvcHigh444Profile:
            return "H.264 High 4:4:4";
        case AvcHigh444PredictiveProfile:
            return "H.264 High 4:4:4 Predictive";
        default:
            return "H.264";
        }
    case GeneralMediaFormat::Av1:
        return "AV1";
    case GeneralMediaFormat::Bitmap:
        return "BMP";
    case GeneralMediaFormat::Daala:
        return "Daala";
    case GeneralMediaFormat::Dirac:
        return "Dirac";
    case GeneralMediaFormat::Dts:
        switch (sub) {
        case DtsLossless:
            return "DTS Lossless";
        case DtsExpress:
            return "DTS LBR";
        default:
            return "DTS";
        }
    case GeneralMediaFormat::DtsHd:
        switch (sub) {
        case DtsHdHighResolution:
            return "DTS-HD High Resolution";
        case DtsHdMasterAudio:
            return "DTS-HD Master Audio";
        case DtsExpress:
            return "DTS-HD Express";
        default:
            return "DTS-HD";
        }
    case GeneralMediaFormat::EAc3:
        return "E-AC-3";
    case GeneralMediaFormat::Evrc:
        return "EVRC";
    case GeneralMediaFormat::Flac:
        return "FLAC";
    case GeneralMediaFormat::FontDataStream:
        return "FDS";
    case GeneralMediaFormat::Gif:
        return "GIF";
    case GeneralMediaFormat::Gpp2Cmf:
        return "3GPP2 CMF";
    case GeneralMediaFormat::Hevc:
        return "H.265";
    case GeneralMediaFormat::Vcc:
        return "H.266";
    case GeneralMediaFormat::ImaadpcmAcm:
        return "IMAADPCM ACM";
    case GeneralMediaFormat::ImageSubtitle:
        switch (sub) {
        case SubFormats::ImgSubBmp:
            return "BMP subtitle";
        default:
            return "Image subtitle";
        }
    case GeneralMediaFormat::InteractionStream:
        return "Interaction Stream";
    case GeneralMediaFormat::Jpeg:
        return "JPEG";
    case GeneralMediaFormat::OggKate:
        return "OggKate";
    case GeneralMediaFormat::Opus:
        return "Opus";
    case GeneralMediaFormat::MicrosoftAudioCodecManager:
        return "MS ACM";
    case GeneralMediaFormat::MicrosoftMpeg4:
        switch (sub) {
        case 1:
            return "MS MPEG-4 v1";
        case 2:
            return "MS MPEG-4 v2";
        case 3:
            return "MS MPEG-4 v3";
        default:
            return "MS MPEG-4";
        }
    case GeneralMediaFormat::MicrosoftVideoCodecManager:
        return "MS VCM";
    case GeneralMediaFormat::DolbyMlp:
        return "Dolby TrueHD";
    case GeneralMediaFormat::Mpeg1Audio:
        switch (sub) {
        case Mpeg1Layer1:
            return "MP1";
        case Mpeg1Layer2:
            return "MP2";
        case Mpeg1Layer3:
            return "MP3";
        default:
            return "MPEG-1 Audio";
        }
    case GeneralMediaFormat::Mpeg1Video:
        return "MP1";
    case GeneralMediaFormat::Mpeg2Audio:
        switch (sub) {
        case Mpeg1Layer1:
            return "MP1";
        case Mpeg1Layer2:
            return "MP2";
        case Mpeg1Layer3:
            return "MP3";
        default:
            return "MPEG-2 Audio";
        }
    case GeneralMediaFormat::Mpeg2Video:
        switch (sub) {
        case Mpeg2SimpleProfile:
            return "MPEG-2 SP";
        case Mpeg2MainProfile:
            return "MPEG-2 Main";
        case Mpeg2SnrProfile:
            return "MPEG-2 SNR";
        case Mpeg2SpatialProfile:
            return "MPEG-2 Spatial";
        case Mpeg2HighProfile:
            return "MPEG-2 High";
        case Mpeg2422Profile:
            return "MPEG-2 422";
        default:
            return "MPEG-2 Video";
        }
    case GeneralMediaFormat::Mpeg4Video:
        switch (sub) {
        case Mpeg4SimpleProfile1:
        case Mpeg4SimpleProfile2:
        case Mpeg4SimpleProfile3:
        case Mpeg4SimpleProfile0:
            return "MPEG-4 SP";
        case Mpeg4AdvancedSimpleProfile0:
        case Mpeg4AdvancedSimpleProfile1:
        case Mpeg4AdvancedSimpleProfile2:
        case Mpeg4AdvancedSimpleProfile3:
        case Mpeg4AdvancedSimpleProfile4:
        case Mpeg4AdvancedSimpleProfile5:
        case Mpeg4AdvancedSimpleProfile3b:
            return "MPEG-4 ASP";
        case Mpeg4AvcProfile:
            return "H.264";
        default:
            return "MPEG-4 Visual";
        }
    case GeneralMediaFormat::Mpc:
        return "MPC";
    case GeneralMediaFormat::Pcm:
        switch (sub) {
        case PcmIntBe:
            return "PCM (int, BE)";
        case PcmIntLe:
            return "PCM (int, LE)";
        case PcmFloatIeee:
            return "PCM IEEE";
        default:
            return "PCM";
        }
    case GeneralMediaFormat::Png:
        return "PNG";
    case GeneralMediaFormat::ProRes:
        return "ProRes";
    case GeneralMediaFormat::Qcelp:
        return "QCELP";
    case GeneralMediaFormat::QuicktimeAudio:
        return "Quicktime Audio";
    case GeneralMediaFormat::QuicktimeVideo:
        return "Quicktime Video";
    case GeneralMediaFormat::RealAudio:
        return "Real Audio";
    case GeneralMediaFormat::RealVideo:
        return "Real Video";
    case GeneralMediaFormat::Sa0c:
        return "SAOC";
    case GeneralMediaFormat::Smv:
        return "SMV";
    case GeneralMediaFormat::StreamingTextStream:
        return "Streaming Text Stream";
    case GeneralMediaFormat::SynthesizedTextureStream:
        return "Synthesized Texture Stream";
    case GeneralMediaFormat::Systems:
        switch (sub) {
        case 2:
            return "Systems v2";
        default:
            return "Systems";
        }
    case GeneralMediaFormat::TextSubtitle:
        switch (sub) {
        case SubFormats::PlainUtf8Subtitle:
            return "";
        case SubFormats::SubStationAlpha:
            return "SSA";
        case SubFormats::AdvancedSubStationAlpha:
            return "ASS";
        case SubFormats::UniversalSubtitleFormat:
            return "USF";
        case SubFormats::WebVideoTextTracksFormat:
            return "WebVTT";
        default:
            return "";
        }
    case GeneralMediaFormat::Theora:
        return "Theora";
    case GeneralMediaFormat::Tiff:
        return "TIFF";
    case GeneralMediaFormat::TimedText:
        return "Timed Text";
    case GeneralMediaFormat::Tta:
        return "TTA";
    case GeneralMediaFormat::UncompressedVideoFrames:
        return "uncompressed video frames";
    case GeneralMediaFormat::Vc1:
        return "VC-1";
    case GeneralMediaFormat::VobBtn:
        return "VobBtn";
    case GeneralMediaFormat::VobSub:
        return "VobSub";
    case GeneralMediaFormat::Vorbis:
        return "Vorbis";
    case GeneralMediaFormat::Vp8:
        return "VP8";
    case GeneralMediaFormat::Vp9:
        return "VP9";
    case GeneralMediaFormat::WavPack:
        return "WavPack";
    case GeneralMediaFormat::WindowsMediaAudio:
        return "WMA";
    case GeneralMediaFormat::WindowsMediaVideo:
        return "WMV";
    case GeneralMediaFormat::DvbSub:
        return "DVBSUB";
    case GeneralMediaFormat::Speex:
        return "Speex";
    default:
        return "";
    }
}

/*!
 * \brief Returns a short abbreviation of the media format as C-style string.
 *
 * Returns an empty string if no abbreviation is available.
 */
std::string_view MediaFormat::shortAbbreviation() const
{
    switch (general) {
    case GeneralMediaFormat::Aac:
        switch (sub) {
        case AacMpeg2MainProfile:
        case AacMpeg4MainProfile:
            return "AAC-Main";
        case AacMpeg2LowComplexityProfile:
        case AacMpeg4LowComplexityProfile:
        case AacMpeg4ERLowComplecityProfile:
            switch (extension) {
                using namespace ExtensionFormats;
            case SpectralBandReplication:
            case ParametricStereo:
            case (SpectralBandReplication | ParametricStereo):
                return "HE-AAC";
            default:
                return "AAC-LC";
            }
        case AacMpeg4ERScalableSampingRateProfile:
            return "AAC-LC";
        case AacMpeg2ScalableSamplingRateProfile:
        case AacMpeg4ScalableSamplingRateProfile:
            return "AAC-SSR";
        case AacMpeg4LongTermPrediction:
        case AacMpeg4ERLongTermPrediction:
            return "AAC-LTP";
        case AacMpeg4ERLowDelay:
            return "AAC-LD";
        case AacMpeg4EREnhancedLowDelay:
            return "AAC-ELD";
        default:
            return "AAC";
        }
    case GeneralMediaFormat::Ac3:
        return "AC3";
    case GeneralMediaFormat::Ac4:
        return "AC4";
    case GeneralMediaFormat::AdpcmAcm:
        return "ADPCM-ACM";
    case GeneralMediaFormat::AfxStream:
        return "AFX";
    case GeneralMediaFormat::Alac:
        return "ALAC";
    case GeneralMediaFormat::Als:
        return "ALS";
    case GeneralMediaFormat::Amr:
        return "AMR";
    case GeneralMediaFormat::Avc:
        switch (sub) {
        case AvcCavlc444IntraProfile:
            return "H.264-CAVLC";
        case AvcBaselineProfile:
            return "H.264-Basline";
        case AvcMainProfile:
            return "H.264-Main";
        case AvcScalableBaselineProfile:
            return "H.264-Scalable-Basline";
        case AvcScalableHighProfile:
            return "H.264-Scalable-High";
        case AvcExtendedProfile:
            return "H.264-Extended";
        case AvcHighProfile:
            return "H.264-High";
        case AvcHigh10Profile:
            return "H.264-High-10";
        case AvcHighMultiviewProfile:
            return "H.264-Multiview";
        case AvcHigh422Profile:
            return "H.264-High-4:2:2";
        case AvcStereoHighProfile:
            return "H.264-Stereo-High";
        case AvcHighMultiviewDepthProfile:
            return "H.264-Multiview-Depth-High";
        case AvcHigh444Profile:
            return "H.264-High-4:4:4";
        case AvcHigh444PredictiveProfile:
            return "H.264-High-4:4:4-Predictive";
        default:
            return "H.264";
        }
    case GeneralMediaFormat::Av1:
        return "AV1";
    case GeneralMediaFormat::Bitmap:
        return "BMP";
    case GeneralMediaFormat::Daala:
        return "Daala";
    case GeneralMediaFormat::Dirac:
        return "Dirac";
    case GeneralMediaFormat::Dts:
        switch (sub) {
        case DtsLossless:
            return "DTS-Lossless";
        case DtsExpress:
            return "DTS-LBR";
        default:
            return "DTS";
        }
    case GeneralMediaFormat::DtsHd:
        return "DTS-HD";
    case GeneralMediaFormat::EAc3:
        return "E-AC-3";
    case GeneralMediaFormat::Evrc:
        return "EVRC";
    case GeneralMediaFormat::Flac:
        return "FLAC";
    case GeneralMediaFormat::FontDataStream:
        return "FDS";
    case GeneralMediaFormat::Gif:
        return "GIF";
    case GeneralMediaFormat::Gpp2Cmf:
        return "3GPP2-CMF";
    case GeneralMediaFormat::Hevc:
        return "H.265";
    case GeneralMediaFormat::Vcc:
        return "H.266";
    case GeneralMediaFormat::ImaadpcmAcm:
        return "IMAADPCM-ACM";
    case GeneralMediaFormat::ImageSubtitle:
        return "BMP";
    case GeneralMediaFormat::Jpeg:
        return "JPEG";
    case GeneralMediaFormat::OggKate:
        return "OggKate";
    case GeneralMediaFormat::Opus:
        return "Opus";
    case GeneralMediaFormat::MicrosoftAudioCodecManager:
        return "MS-ACM";
    case GeneralMediaFormat::MicrosoftMpeg4:
        return "MS-MPEG-4";
    case GeneralMediaFormat::MicrosoftVideoCodecManager:
        return "MS-VCM";
    case GeneralMediaFormat::DolbyMlp:
        return "TrueHD";
    case GeneralMediaFormat::Mpeg1Audio:
        switch (sub) {
        case Mpeg1Layer1:
            return "MP1";
        case Mpeg1Layer2:
            return "MP2";
        default:
            // since MP3 is backward compatible, it is ok to use it also as fallback
            return "MP3";
        }
    case GeneralMediaFormat::Mpeg1Video:
        return "MP1";
    case GeneralMediaFormat::Mpeg2Audio:
        switch (sub) {
        case Mpeg1Layer1:
            return "MP1";
        case Mpeg1Layer2:
            return "MP2";
        default:
            // since MP3 is backward compatible, it is ok to use it also as fallback
            return "MP3";
        }
    case GeneralMediaFormat::Mpeg2Video:
        switch (sub) {
        case Mpeg2SimpleProfile:
            return "MPEG-2-SP";
        case Mpeg2MainProfile:
            return "MPEG-2-Main";
        case Mpeg2SnrProfile:
            return "MPEG-2-SNR";
        case Mpeg2SpatialProfile:
            return "MPEG-2-Spatial";
        case Mpeg2HighProfile:
            return "MPEG-2-High";
        case Mpeg2422Profile:
            return "MPEG-2-422";
        default:
            return "MPEG-2";
        }
    case GeneralMediaFormat::Mpeg4Video:
        switch (sub) {
        case Mpeg4SimpleProfile1:
        case Mpeg4SimpleProfile2:
        case Mpeg4SimpleProfile3:
        case Mpeg4SimpleProfile0:
            return "MPEG-4-SP";
        case Mpeg4AdvancedSimpleProfile0:
        case Mpeg4AdvancedSimpleProfile1:
        case Mpeg4AdvancedSimpleProfile2:
        case Mpeg4AdvancedSimpleProfile3:
        case Mpeg4AdvancedSimpleProfile4:
        case Mpeg4AdvancedSimpleProfile5:
        case Mpeg4AdvancedSimpleProfile3b:
            return "MPEG-4-ASP";
        case Mpeg4AvcProfile:
            return "H.264";
        default:
            return "MPEG-4-Visual";
        }
    case GeneralMediaFormat::Mpc:
        return "MPC";
    case GeneralMediaFormat::Pcm:
        return "PCM";
    case GeneralMediaFormat::Png:
        return "PNG";
    case GeneralMediaFormat::ProRes:
        return "ProRes";
    case GeneralMediaFormat::Qcelp:
        return "QCELP";
    case GeneralMediaFormat::QuicktimeAudio:
        return "Qt-Audio";
    case GeneralMediaFormat::QuicktimeVideo:
        return "Qt-Video";
    case GeneralMediaFormat::RealAudio:
        return "Real-Audio";
    case GeneralMediaFormat::RealVideo:
        return "Real-Video";
    case GeneralMediaFormat::Sa0c:
        return "SAOC";
    case GeneralMediaFormat::Smv:
        return "SMV";
    case GeneralMediaFormat::Systems:
        return "Systems";
    case GeneralMediaFormat::TextSubtitle:
        switch (sub) {
        case SubFormats::PlainUtf8Subtitle:
            return "UTF-8";
        case SubFormats::SubStationAlpha:
            return "SSA";
        case SubFormats::AdvancedSubStationAlpha:
            return "ASS";
        case SubFormats::UniversalSubtitleFormat:
            return "USF";
        case SubFormats::WebVideoTextTracksFormat:
            return "WebVTT";
        default:
            return "";
        }
    case GeneralMediaFormat::Theora:
        return "Theora";
    case GeneralMediaFormat::Tiff:
        return "TIFF";
    case GeneralMediaFormat::TimedText:
        return "Timed-Text";
    case GeneralMediaFormat::Tta:
        return "TTA";
    case GeneralMediaFormat::UncompressedVideoFrames:
        return "RAW";
    case GeneralMediaFormat::Vc1:
        return "VC-1";
    case GeneralMediaFormat::VobBtn:
        return "VobBtn";
    case GeneralMediaFormat::VobSub:
        return "VobSub";
    case GeneralMediaFormat::Vorbis:
        return "Vorbis";
    case GeneralMediaFormat::Vp8:
        return "VP8";
    case GeneralMediaFormat::Vp9:
        return "VP9";
    case GeneralMediaFormat::WavPack:
        return "WavPack";
    case GeneralMediaFormat::WindowsMediaAudio:
        return "WMA";
    case GeneralMediaFormat::WindowsMediaVideo:
        return "WMV";
    case GeneralMediaFormat::DvbSub:
        return "DVBSUB";
    case GeneralMediaFormat::Speex:
        return "Speex";
    case GeneralMediaFormat::MonkeysAudio:
        return "APE";
    default:
        return "";
    }
}

/*!
 * \brief Returns the abbreviation of the media format as C-style string.
 *
 * Returns an empty string if no abbreviation is available.
 */
std::string_view MediaFormat::extensionName() const
{
    switch (general) {
        using namespace ExtensionFormats;
    case GeneralMediaFormat::Aac:
        switch (extension) {
        case SpectralBandReplication:
            return "Spectral Band Replication / HE-AAC";
        case ParametricStereo:
            return "Parametric Stereo / HE-AAC v2"; // PS always implies SBR?
        case (SpectralBandReplication | ParametricStereo):
            return "Spectral Band Replication and Parametric Stereo / HE-AAC v2";
        default:;
        }
        break;
    default:;
    }
    return "";
}

/*!
 * \brief Returns the string representation for the specified \a mediaType.
 */
std::string_view mediaTypeName(MediaType mediaType)
{
    switch (mediaType) {
    case MediaType::Unknown:
        return "Other";
    case MediaType::Audio:
        return "Audio";
    case MediaType::Video:
        return "Video";
    case MediaType::Text:
        return "Subititle";
    case MediaType::Buttons:
        return "Buttons";
    case MediaType::Control:
        return "Control";
    case MediaType::Hint:
        return "Hint";
    case MediaType::Meta:
        return "Meta-data";
    }
    return "";
}

} // namespace TagParser
