#ifndef TAG_PARSER_MP4IDS_H
#define TAG_PARSER_MP4IDS_H

#include "../global.h"

#include <cstdint>

namespace TagParser {

class MediaFormat;

namespace Mp4AtomIds {
enum KnownValue : std::uint32_t {
    Av1Configuration = 0x61763143,
    AvcConfiguration = 0x61766343,
    BitrateBox = 0x62747274,
    CleanAperature = 0x636c6170,
    ChunkOffset64 = 0x636f3634,
    CompositionTimeToSample = 0x63747473,
    Data = 0x64617461,
    DataInformation = 0x64696e66,
    DataReference = 0x64726566,
    Drms = 0x64726D73,
    Edts = 0x65647473, // FIXME: remove in next major release
    Edit = 0x65647473,
    EditList = 0x656C7374,
    Free = 0x66726565,
    FileType = 0x66747970,
    HandlerReference = 0x68646c72,
    HintMediaHeader = 0x686D6864,
    ItunesList = 0x696c7374,
    MediaData = 0x6d646174,
    MediaHeader = 0x6d646864,
    Media = 0x6d646961,
    Mean = 0x6D65616E,
    MovieExtendsHeader = 0x6D656864,
    Meta = 0x6d657461,
    MovieFragmentHeader = 0x6D666864,
    MovieFragmentRandomAccess = 0x6d667261,
    MediaInformation = 0x6d696e66,
    MediaInformationHeader = 0x676D6864,
    MediaInformationBase = 0x676D696E,
    MediaInformationText = 0x74657874,
    MovieFragment = 0x6d6f6f66,
    Movie = 0x6d6f6f76,
    MovieExtends = 0x6D766578,
    MovieHeader = 0x6D766864,
    Name = 0x6E616D65,
    NullMediaHeaderBox = 0x6E6D6864,
    PaddingBits = 0x70616462,
    PixalAspectRatio = 0x70617370,
    ProgressiveDownloadInformation = 0x7064696e,
    SampleToGroup = 0x73626770,
    IndependentAndDisposableSamples = 0x73647470,
    SampleGroupDescription = 0x73677064,
    Skip = 0x736b6970,
    SoundMediaHeader = 0x736D6864,
    SampleTable = 0x7374626c,
    ChunkOffset = 0x7374636f,
    DegradationPriority = 0x73746470,
    SampleToChunk = 0x73747363,
    SampleDescription = 0x73747364,
    ShadowSyncSample = 0x73747368,
    SyncSample = 0x73747373,
    SampleSize = 0x7374737A,
    DecodingTimeToSample = 0x73747473,
    CompactSampleSize = 0x73747a32,
    SubSampleInformation = 0x73756273,
    TrackFragmentHeader = 0x74666864,
    TrackHeader = 0x746b6864,
    TrackFragment = 0x74726166,
    Track = 0x7472616b,
    TrackReference = 0x74726566,
    TrackExtends = 0x74726578,
    TrackFragmentRun = 0x7472756E,
    UserData = 0x75647461,
    DataEntryUrl = 0x75726C20,
    DataEntryUrn = 0x75726E20,
    VideoMediaHeader = 0x766D6864,
    Wide = 0x77696465
};
}

namespace Mp4TagAtomIds {
enum KnownValue : std::uint32_t {
    Album = 0xA9616c62,
    AlbumArtist = 0x61415254,
    Artist = 0xA9415254,
    Bpm = 0x746d706f,
    Category = 0x63617467,
    Comment = 0xA9636d74,
    Composer = 0xA9777274,
    Copyright = 0x63707274,
    Cover = 0x636f7672,
    Description = 0x64657363,
    DiskPosition = 0x6469736b,
    Encoder = 0xA9746f6f,
    EpisodeGlobalUniqueId = 0x65676964,
    Extended = 0x2d2d2d2d,
    GaplessPlayback = 0x70676170,
    Genre = 0xA967656e,
    Grouping = 0xA9677270,
    Keywords = 0x6b657977,
    Lyricist = 0xA9737766,
    Lyrics = 0xA96c7972,
    MediaType = 0x7374696B,
    Performers = 0xA9707266,
    Podcast = 0x70637374,
    PodcastUrl = 0x7075726c,
    PreDefinedGenre = 0x676e7265,
    Producer = 0xA9707264,
    PurchaseDate = 0x70757264,
    Rating = 0x72746e67,
    RecordLabel = 0xA96c6162,
    Title = 0xA96e616d,
    TrackPosition = 0x74726b6e,
    TvEpisode = 0x74766573,
    TvEpisodeName = 0x7476656e,
    TvNetworkName = 0x74766e6e,
    TvSeason = 0x7476736e,
    TvShowName = 0x74767368,
    Year = 0xA9646179
};
}

namespace Mp4TagExtendedMeanIds {
extern const char *iTunes;
}

namespace Mp4TagExtendedNameIds {
extern const char *cdec;
extern const char *label;
} // namespace Mp4TagExtendedNameIds

namespace Mp4MediaTypeIds {
enum KnownValue : std::uint32_t {
    Sound = 0x736f756e, /**< Sound/Audio */
    Video = 0x76696465, /**< Video */
    Hint = 0x68696e74, /**< Hint */
    Meta = 0x6d657461 /**< Meta */
};
}

namespace FourccIds {
enum KnownValue : std::uint32_t {
    Ac3 = 0x61632d33, /**< Dolby Digital */
    Ac4 = 0x61632d34, /**< ? */
    AdpcmAcm = 0x6D730002, /**< ? */
    Agsm = 0x6167736D,
    Alac = 0x616C6163, /**< Apple Losless Audio Codec */
    Alaw = 0x616C6177,
    Alaw21 = 0x616C6177,
    AlphaCompositor = 0x626C6E64,
    AlphaGain = 0x6761696E,
    Amr = 0x73617762,
    AmrNarrowband = 0x73616D72,
    Animation = 0x726C6520, /**< Animation */
    Appl1 = 0x6476690,
    Appl2 = 0x6C70630,
    Apple16BitGray = 0x62313667,
    Apple32BitGrayWithAlpha = 0x62333261,
    Apple48BitRgb = 0x62343872,
    Apple64BitArgb1 = 0x62363461,
    Apple64BitArgb2 = 0x62617365,
    Apple64BitArgb3 = 0x626C6974,
    AppleAnimation = 0x726C6520,
    AppleAvrJpeg = 0x61767220,
    AppleBmp = 0x57524C45,
    AppleCinepak = 0x63766964,
    AppleCmyk = 0x636D796B,
    AppleComponentVideoYuv422 = 0x79757632,
    AppleCurve = 0x70617468,
    AppleDvc = 0x64766320,
    AppleDvcpro = 0x64767070,
    AppleDvcpro501 = 0x6476356E,
    AppleDvcpro502 = 0x64763570,
    AppleDvcpro50Ntsc = 0x6476356E,
    AppleDvcpro50Pal = 0x64763570,
    AppleDvcproPal = 0x64767070,
    AppleDvDvcproNtsc = 0x6476630,
    AppleDvp = 0x64766370,
    AppleDvPal = 0x64766370,
    AppleFlc = 0x666C6963,
    AppleGif = 0x67696620,
    AppleGraphics = 0x736D630,
    AppleGsm101 = 0x6167736D,
    AppleH261 = 0x68323631,
    AppleIntermediateCodec = 0x69636F64,
    AppleLossless = 0x616C6163,
    AppleMacpaint = 0x504E5447,
    AppleMicrosoftVideo1 = 0x6D737663,
    AppleMotionJpegA = 0x6D6A7061,
    AppleMotionJpegB = 0x6D6A7062,
    AppleMpeg4Compressor = 0x6D703476,
    AppleMpeg4Decompressor = 0x6D703476,
    AppleOpendmlJpeg = 0x646D6231,
    ApplePhotoCd = 0x6B706364,
    ApplePhotoJpeg = 0x6A706567,
    ApplePixletVideo = 0x70786C74,
    ApplePlanarRgb = 0x38425053,
    ApplePng = 0x706E670,
    AppleQuickdraw = 0x71647277,
    AppleR408 = 0x72343038,
    AppleScalingCodec = 0x7363616C,
    AppleSgi = 0x2E534749,
    AppleSorensonYuv9Codec = 0x73797639,
    AppleTextAtsuiCodec = 0x74657874,
    AppleTga = 0x7467610,
    AppleTiff = 0x74696666,
    AppleV408 = 0x76343038,
    AppleVcH263 = 0x68323633,
    AppleVideo = 0x72707A61, /**< Apple video */
    AppleYuv420Codec1 = 0x6A343230,
    AppleYuv420Codec2 = 0x6D797576,
    AppleYuv420Codec3 = 0x79343230,
    AppleYuv422Codec2Vuy = 0x32767579,
    AppleYuv422Codec4 = 0x79757678,
    AppleYuv422CodecYuvs = 0x79757673,
    AppleYuv422CodecYuvu = 0x79757675,
    Avc0Media = 0x64726D69,
    Avc1 = 0x61766331, /**< H.264/MPEG-4 AVC video */
    Avc2 = 0x61766332, /**< H.264/MPEG-4 AVC video */
    Avc3 = 0x61766333, /**< H.264/MPEG-4 AVC video */
    Avc4 = 0x61766334, /**< H.264/MPEG-4 AVC video */
    Av1_IVF = 0x41563031, /**< AV1 video (found in IVF) */
    Av1_ISOBMFF = 0x61763031, /**< AV1 video (found in ISOBMFF) */
    Blur = 0x626C7572,
    Bps8 = 0x38627073,
    BrightnessAndContrast = 0x6272636F,
    ChannelCompositor = 0x6368616E,
    ChromaKey = 0x636B6579,
    Cinepak = 0x63766964, /**< Cinepak */
    Cloud = 0x636C6F75,
    ColorStyle = 0x736F6C72,
    Colorsync = 0x73796E63,
    ColorTint = 0x74696E74,
    CrossFade = 0x64736C76,
    Cvid = 0x63766964,
    Divx3Decoder1 = 0x41503431,
    Divx3Decoder2 = 0x434F4C30,
    Divx3Decoder3 = 0x434F4C31,
    Divx3Decoder4 = 0x44495633,
    Divx3Decoder5 = 0x44495634,
    Divx3Decoder6 = 0x44495635,
    Divx3Decoder7 = 0x44495636,
    Divx3Decoder8 = 0x4D504733,
    Divx3Decoder9 = 0x6D706733,
    Divx3Decoder10 = 0x636F6C30,
    Divx3Decoder11 = 0x636F6C31,
    Divx3Decoder12 = 0x64697633,
    Divx3Decoder13 = 0x64697634,
    Divx3Decoder14 = 0x64697635,
    Divx3Decoder15 = 0x64697636,
    Divx4Decoder1 = 0x44495658,
    Divx4Decoder2 = 0x64697678,
    Divx5Decoder = 0x44583530,
    Drms = 0x64726D73,
    Drmi = 0x64726D69,
    Dts = 0x6474736C,
    DtsH = 0x64747368,
    DtsE = 0x64747365,
    Dvca = 0x64766361,
    DvcPro501 = 0x64763570,
    DvcPro502 = 0x6476356E,
    DvcProPal = 0x64767070,
    EAc3 = 0x65632D33,
    EdgeDetection = 0x65646765,
    Emboss = 0x656D6273,
    Explode = 0x78706C6F,
    FilmNoise = 0x666D6E73,
    Fire = 0x66697265,
    Flac = 0x664C6143,
    FlashPixImage = 0x66706978,
    FlashScreenVideoDecoder = 0x46535631,
    FloatingPoint32Bit = 0x666C3332,
    FloatingPoint64Bit = 0x666C3634,
    GeneralConvolution = 0x67656E6B,
    Gif = 0x67696620, /**< CompuServe Graphics Interchange Format */
    Glass = 0x676C6173,
    GradientWipe = 0x6D617474,
    Graphics = 0x736D6320, /**< Graphics */
    H263Quicktime = 0x68323633, /**< H.263/MPEG-4 ASP video (Quicktime) */
    H2633GPP = 0x73323633, /**< H.263 (3GPP format) */
    H264Decoder1 = 0x44415643,
    H264Decoder2 = 0x48323634,
    H264Decoder3 = 0x56535348,
    H264Decoder4 = 0x58323634,
    H264Decoder5 = 0x68323634,
    H264Decoder6 = 0x78323634,
    Hdv3 = 0x68647633,
    Hevc1 = 0x68766331, /**< H.265/High Efficiency Video Coding  */
    Hevc2 = 0x68657631, /**< H.265/High Efficiency Video Coding */
    HslBalance = 0x68736C62,
    Ima4 = 0x696D6134,
    Ima41 = 0x696D6134,
    ImaadpcmAcm = 0x6D730011, /**< ? */
    Implode = 0x6D706C6F,
    Int16Be = 0x74776F73,
    Int16Le = 0x736F7774,
    Int24 = 0x696E3234,
    Int32 = 0x696E3332,
    Iris = 0x736D7032,
    IvxDecoder1 = 0x33495632,
    IvxDecoder2 = 0x33495644,
    IvxDecoder3 = 0x33697632,
    IvxDecoder4 = 0x33697664,
    Jpeg = 0x6a706567, /**< JPEG */
    Jpeg2000Decoder = 0x6D6A7032,
    Law21 = 0x756C6177,
    LensFlare = 0x6C656E73,
    Lle = 0x726C6520,
    Mac3 = 0x6D616333,
    Mac6 = 0x6D616336,
    Mace31 = 0x4D414333,
    Mace61 = 0x4D414336,
    MatrixWipe = 0x736D7034,
    DolbyMpl = 0x6D6C7061,
    MotionJpegA = 0x6D6A7061, /**< Motion-JPEG (format A) */
    MotionJpegB = 0x6D6A7062, /**< Motion-JPEG (format B) */
    Mp3 = 0x2e6d7033, /**< MPEG-1 Layer 3 */
    Mp3CbrOnly = 0x6D730055, /**< MPEG-1 Layer 3 (constant bitrate only) */
    Mpeg = 0x4D504547,
    Mpeg2Imx30 = 0x6D78336E,
    Mpeg2Imx50 = 0x6D783570,
    Mpeg4Audio = 0x6d703461, /**< MPEG-4 audio */
    Mpeg4Decoder1 = 0x464D5034,
    Mpeg4Decoder2 = 0x53454447,
    Mpeg4Decoder3 = 0x57563146,
    Mpeg4Sample = 0x6d703473, /**< MPEG-4 stream (other than video/audio) */
    Mpeg4TimedText = 0x74783367, /**< MPEG-4 Timed Text / Streaming text format / Part 17 */
    Mpeg4Video = 0x6d703476, /**< MPEG-4 video */
    MsMpeg4V1Decoder1 = 0x44495631,
    MsMpeg4V1Decoder2 = 0x64697631,
    MsMpeg4V1Decoder3 = 0x4D504734,
    MsMpeg4V1Decoder4 = 0x6D706734,
    MsMpeg4V1Decoder5 = 0x4D504731,
    MsMpeg4V1Decoder6 = 0x6D706731,
    MsMpeg4V2Decoder1 = 0x44495632,
    MsMpeg4V2Decoder2 = 0x64697632,
    MsMpeg4V2Decoder3 = 0x4D503432,
    MsMpeg4V2Decoder4 = 0x6D703432,
    MsMpeg4V3Decoder1 = 0x4D503433,
    MsMpeg4V3Decoder2 = 0x6D703433,
    NtscDv25Video = 0x64766320, /**< NTSC DV-25 video */
    Oggs = 0x4F676753,
    Opus = 0x4F707573,
    PalDv25Video = 0x64766370, /**< PAL DV-25 video */
    PdfImage = 0x70646620,
    Png = 0x706E6720, /**< Portable Network Graphics */
    Push = 0x70757368,
    Qclp = 0x51636C70,
    QdesignMusic1Decoder = 0x51444D43,
    QdesignMusic2 = 0x51444D32,
    Qdmc = 0x51444D43,
    Qdrw = 0x71647277,
    QtvrCubicCodec = 0x63757061,
    QtvrCylindricalCodec = 0x6C747061,
    QualcommPurevoice = 0x51636C70,
    QualcommQcelp = 0x51636C71,
    Radial = 0x736D7033,
    Raw = 0x72617720, /**< Uncompressed RGB */
    RgbBalance = 0x72676262,
    Ripple = 0x7269706C,
    Rpza = 0x72707A61,
    Rv20 = 0x52563230,
    Rv30 = 0x52563330,
    Rv40 = 0x52563430,
    Sharpen = 0x73687270,
    Slide = 0x736C6964,
    Smc = 0x736D6320,
    SorensonH263Decoder = 0x464C5631,
    SorensonVideo3Compressor = 0x53565133,
    SorensonVideoDecompressor = 0x53565131,
    Sowt = 0x736F7774,
    SpecialEffectsAndFilters = 0x67656666,
    Svq1 = 0x73767131,
    Svq3 = 0x73767133,
    Tiff = 0x74696666, /**< Tagged Image File Format */
    TravelingMatte = 0x74726176,
    TruemotionVp6Decoder1 = 0x56503632,
    TruemotionVp6Decoder2 = 0x56503646,
    Twos = 0x74776F73,
    Ulaw = 0x756C6177,
    Ulaw21 = 0x756C6177,
    VcmImageCodec = 0x4D6A7067,
    Vdva = 0x76647661,
    Vp8 = 0x56503830, /**< VP8 video */
    Vp9 = 0x56503930, /**< VP9 video */
    Vp9_2 = 0x76703039, /**< VP9 video */
    WavPack = 0x5756504B,
    WindowsMediaAudio = 0x6F776D61, /**< ? */
    WindowsMediaAudio7 = 0x574D4131,
    WindowsMediaAudio9Professional = 0x574D4133,
    WindowsMediaAudio9Standard = 0x574D4132,
    WindowsMediaVideoV17 = 0x574D5637,
    WindowsMediaVideoV2 = 0x574D5632,
    WindowsMediaVideoV8 = 0x574D5638,
    Wipe = 0x736D7074,
    WmvImageCodec1 = 0x4D347332,
    WmvImageCodec2 = 0x4D703432,
    WmvImageCodec3 = 0x4D703433,
    WmvImageCodec4 = 0x4D703453,
    WmvImageCodec5 = 0x574D5631,
    WmvImageCodec6 = 0x574D5632,
    WmvImageCodec7 = 0x574D5633,
    XvidDecoder1 = 0x424C5A30,
    XvidDecoder2 = 0x58564944,
    XvidDecoder3 = 0x58564958,
    XvidDecoder4 = 0x58766944,
    XvidDecoder5 = 0x78766964,
    Yuv422HardwareAccelerationCodecYuvs1 = 0x32767579,
    Yuv422HardwareAccelerationCodecYuvs2 = 0x61633136,
    Yuv422HardwareAccelerationCodecYuvs3 = 0x61633332,
    Yuv422HardwareAccelerationCodecYuvs4 = 0x61634247,
    Yuv422HardwareAccelerationCodecYuvs5 = 0x79757673,
    Zoom = 0x7A6F6F6D
};

TAG_PARSER_EXPORT MediaFormat fourccToMediaFormat(std::uint32_t fourccId);

} // namespace FourccIds

namespace Mp4FormatExtensionIds {
enum KnownValue : std::uint32_t {
    GammaLevel
    = 0x67616D61, /**< A 32-bit fixed-point number indicating the gamma level at which the image was captured. The decompressor can use this value to gamma-correct at display time. */
    FieldHandling = 0x6669656C, /**< Two 8-bit integers that define field handling. */
    DefaultQuantizationTable = 0x6D6A7174, /**< The default quantization table for a Motion-JPEG data stream. */
    DefaultHuffmanTable = 0x6D6A6874, /**< The default Huffman table for a Motion-JPEG data stream. */
    Mpeg4ElementaryStreamDescriptor = 0x65736473, /**< An MPEG-4 elementary stream descriptor atom. This extension is required for MPEG-4 video. */
    Mpeg4ElementaryStreamDescriptor2 = 0x6D346473, /**< Alternative if encoded to AVC stanard. */
    AvcConfiguration = 0x61766343, /**< An H.264 AVCConfigurationBox. This extension is required for H.264 video as defined in ISO/IEC 14496-15. */
    PixelAspectRatio = 0x70617370, /**< Pixel aspect ratio. This extension is mandatory for video formats that use non-square pixels. */
    ColorParameters = 0x636F6C72, /**< An image description extension required for all uncompressed Y´CbCr video types. */
    CleanAperature = 0x636C6170 /**< Spatial relationship of Y´CbCr components relative to a canonical image center. */
};
}

namespace Mpeg4ElementaryStreamObjectIds {
enum KnownValue : std::uint8_t {
    SystemsIso144961 = 0x01, /**< Systems */
    SystemsIso144961v2, /**< Systems (version 2) */
    InteractionStream, /**< Interaction Stream */
    AfxStream = 0x05, /**< AFX Stream */
    FontDataStream, /**< Font Data Stream */
    SynthesizedTextureStream, /**< Synthesized Texture Stream */
    StreamingTextStream, /**< Streaming Text Stream */
    Mpeg4Visual = 0x20, /**< MPEG-4 Visual */
    Avc, /**< Advanced Video Coding */
    ParameterSetsForAvc, /**< Parameter Sets for Advanced Video Coding */
    Als = 0x24, /**< ALS */
    Sa0c = 0x2B, /**< SAOC */
    Aac = 0x40, /**< Audio ISO/IEC 14496-3 (AAC) */
    Mpeg2VideoSimpleProfile = 0x60, /**< MPEG-2 Video Simple Profile */
    Mpeg2VideoMainProfile, /**< MPEG-2 Video Main Profile */
    Mpeg2VideoSnrProfile, /**< MPEG-2 Video SNR Profile */
    Mpeg2VideoSpatialProfile, /**< MPEG-2 Video Spatial Profile */
    Mpeg2VideoHighProfile, /**< MPEG-2 Video High Profile */
    Mpeg2Video422Profile, /**< MPEG-2 Video 422 Profile */
    Mpeg2AacMainProfile, /**< Advanced Audio Coding Main Profile */
    Mpeg2AacLowComplexityProfile, /**< Advanced Audio Coding Low Complexity Profile */
    Mpeg2AacScaleableSamplingRateProfile, /**< Advanced Audio Coding Scaleable Sampling Rate Profile */
    Mpeg2Audio, /**< MPEG-2 Audio */
    Mpeg1Video, /**< MPEG-1 Video */
    Mpeg1Audio, /**< MPEG-1 Audio */
    Jpeg, /**< JPEG */
    Png, /**< PNG */
    Evrc = 0xA0, /**< EVRC */
    Smv, /**< SMV */
    Gpp2Cmf, /**< 3GPP2 Compact Multimedia Format (CMF) */
    Vc1, /**< VC-1 */
    Dirac, /**< Dirac */
    Ac3, /**< AC-3 */
    EAc3, /**< E-AC-3 */
    Dts, /**< DTS */
    DtsHdHighResolution, /**< DTS-HD High Resolution */
    DtsHdMasterAudio, /**< DTS-HD Master Audio */
    DtsHdExpress, /**< DTS-HD Express */
    PrivateEvrc = 0xD1, /**< EVRC */
    PrivateAc3 = 0xD3, /**< AC-3 */
    PrivateDts, /**< DTS */
    PrivateOgg = 0xDD, /**< Ogg */
    PrivateOgg2, /**< Ogg */
    PrivateVobSub = 0xE0, /**< VobSub */
    PrivateQcelp = 0xE1, /**< QCELP */
};

TAG_PARSER_EXPORT MediaFormat streamObjectTypeFormat(std::uint8_t streamObjectTypeId);

} // namespace Mpeg4ElementaryStreamObjectIds

namespace Mpeg4ElementaryStreamTypeIds {
enum KnownValue : std::uint8_t {
    ObjectDescriptor = 0x01,
    ClockReference,
    SceneDescriptor,
    Visual,
    Audio,
    Mpeg7,
    Ipmps,
    ObjectContentInfo,
    MpegJava,
    Interaction,
    Ipmp,
    FontData,
    StreamingText
};

TAG_PARSER_EXPORT const char *streamTypeName(std::uint8_t streamTypeId);

} // namespace Mpeg4ElementaryStreamTypeIds

namespace Mpeg4DescriptorIds {
enum KnownValue : std::uint8_t {
    ObjectDescr = 0x01,
    InitialObjectDescr,
    ElementaryStreamDescr,
    DecoderConfigDescr,
    DecoderSpecificInfo,
    SlConfigDescr,
    ContentIdentDescr,
    SupplContentIdentDescr,
    IpiDescPointer,
    IpmpDescPointer,
    IpmpDescr,
    QoSDescr,
    RegistrationDescr,
    EsIdInc,
    EsIdRef,
    Mp4I0d,
    Mp40d,
    IplDescrPointerRef,
    ExtendedProfileLevelDescr,
    ProfileLevelIndicationIndexDescr,
    ContentClassificationDescr = 0x40,
    KeyWordDescr,
    RatingDescr,
    LanguageDescr,
    ShortTextualDescr,
    ExpandedTextualDescr,
    ContentCreatorNameDescr,
    ContentCreationDateDescr,
    IcicCreatorDateDescr,
    SmpteCameraPositionDescr,
    SegmentDescr,
    MediaTimeDescr,
    IpmpToolsListDescr = 0x60,
    IpmpToolTag,
    FlexMuxTimingDescr,
    FlexMuxCodeTableDescr,
    ExtSlConfigDescr,
    FlexMuxIdentDescr,
    DependencyPointer,
    DependencyMaker,
    FlexMuxChannelDescr,
    UserPrivate = 0xC0
};
}

namespace Mpeg4AudioObjectIds {
enum KnownValue : std::uint8_t {
    Null = 0,
    AacMain,
    AacLc, /**< low complexity */
    AacSsr, /**< scalable sample rate */
    AacLtp, /**< long term prediction */
    Sbr, /**< spectral band replication */
    AacScalable,
    TwinVq,
    Celp, /**< code excited linear prediction */
    Hxvc, /**< harmonic vector excitation coding */
    Ttsi = 12, /**< text-to-speech interface */
    MainSynthesis,
    WavetableSynthesis,
    GeneralMidi,
    AlgorithmicSynthesisAudioEffects,
    ErAacLc, /**< error resillent AAC LC */
    ErAacLtp = 19,
    ErAacScalable,
    ErTwinVq,
    ErBsac,
    ErAacLd,
    ErCelp,
    ErHvxc,
    ErHiln,
    ErParametric,
    Ssc,
    Ps,
    MpegSurround,
    EscapeValue,
    Layer1,
    Layer2,
    Layer3,
    Dst,
    Als, /**< audio lossless */
    Sls, /**< scalable lossless */
    ErAacEld, /**< enhanced low delay */
    SmrSimple, /**< symbolic music representation */
    SmrMain,
    UsacNoSbr, /**< unified speech and audio coding */
    Saoc, /**< spatial audio object coding (no SBR) */
    LdMpegSurround,
    Usac /**< unified speech and audio coding */
};

TAG_PARSER_EXPORT MediaFormat idToMediaFormat(std::uint8_t mpeg4AudioObjectId, bool sbrPresent = false, bool psPresent = false);

} // namespace Mpeg4AudioObjectIds

extern std::uint32_t mpeg4SamplingFrequencyTable[13];

namespace Mpeg4ChannelConfigs {
enum Mpeg4ChannelConfig : std::uint8_t {
    AotSpecificConfig = 0,
    FrontCenter,
    FrontLeftFrontRight,
    FrontCenterFrontLeftFrontRight,
    FrontCenterFrontLeftFrontRightBackCenter,
    FrontCenterFrontLeftFrontRightBackLeftBackRight,
    FrontCenterFrontLeftFrontRightBackLeftBackRightLFEChannel,
    FrontCenterFrontLeftFrontRightSideLeftSideRightBackLeftBackRightLFEChannel
};

TAG_PARSER_EXPORT const char *channelConfigString(std::uint8_t config);
TAG_PARSER_EXPORT std::uint8_t channelCount(std::uint8_t config);

} // namespace Mpeg4ChannelConfigs

namespace Mpeg4VideoCodes {
enum KnownValue : std::uint8_t {
    VideoObjectStart = 0x00,
    VideoObjectLayerStart = 0x20,
    VisualObjectSequenceStart = 0xB0,
    VisualObjectSequendeEnd = 0xB1,
    UserDataStart = 0xB2,
    GroupOfVopStart = 0xB3,
    VideoSessionError = 0xB4,
    VisualObjectStart = 0xB5,
    VopStart = 0xB6,
    FbaObjectStart = 0xBA,
    FbaObjectPlaneStart = 0xBB,
    MeshObjectStart = 0xBC,
    MeshObjectPlaneStart = 0xBD,
    StillTextureObjectStart = 0xBE,
    TextureSpatialLayerStart = 0xBF,
    TextureSnrLayerStart = 0xC0,
    TextureTitleStart = 0xC1,
    TextureShapeLayerStart = 0xC2,
    StuffingStart = 0xC3
};
}

namespace Mpeg2VideoCodes {
enum KnownValue : std::uint8_t { Pic = 0x00, Seq = 0xB3, Ext = 0xB5, Gop = 0xB8 };
}

/*!
 * \brief Specifies the tag type.
 */
enum class Mp4TagMediaType : std::uint8_t {
    Movie = 0, /**< Movie */
    Music = 1, /**< Music */
    Audiobook = 2, /**< Audiobook */
    MusicVideo = 6, /**< MusicVideo */
    Movie2 = 9, /**< Movie */
    TvShow = 10, /**< TvShow */
    Booklet = 11, /**< Booklet */
    Ringtone = 14 /**< Ringtone */
};

/*!
 * \brief Specifies the tag content rating.
 */
enum class Mp4TagContentRating : std::uint8_t {
    None = 0, /**< None */
    Clean = 2, /**< Clean */
    Explicit = 4 /**< Explicit */
};

/*!
 * \brief Specifies the account type.
 */
enum class AccountType : std::uint8_t { Itunes = 0, Aol = 1, Undefined = 255 };

/*!
 * \brief Specifies the country.
 */
enum class CountryCode {
    Usa = 143441,
    Fra = 143442,
    Deu = 143443,
    Gbr = 143444,
    Aut = 143445,
    Bel = 143446,
    Fin = 143447,
    Grc = 143448,
    Irl = 143449,
    Ita = 143450,
    Lux = 143451,
    Nld = 143452,
    Prt = 143453,
    Esp = 143454,
    Can = 143455,
    Swe = 143456,
    Nor = 143457,
    Dnk = 143458,
    Che = 143459,
    Aus = 143460,
    Nzl = 143461,
    Jpn = 143462,
    Undefined = 0
};

} // namespace TagParser

#endif // TAG_PARSER_MP4IDS_H
