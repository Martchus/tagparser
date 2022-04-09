#ifndef TAG_PARSER_MP4IDS_H
#define TAG_PARSER_MP4IDS_H

#include "../global.h"

#include <cstdint>
#include <string_view>

namespace TagParser {

class MediaFormat;

namespace Mp4AtomIds {
enum KnownValue : std::uint32_t {
    Av1Configuration = 0x61763143, /**< av1C */
    AvcConfiguration = 0x61766343, /**< avcC */
    BitrateBox = 0x62747274, /**< btrt */
    CleanAperature = 0x636c6170, /**< clap */
    ChunkOffset64 = 0x636f3634, /**< co64 */
    CompositionTimeToSample = 0x63747473, /**< ctts */
    Data = 0x64617461, /**< data */
    DataInformation = 0x64696e66, /**< dinf */
    DataReference = 0x64726566, /**< dref */
    Drms = 0x64726D73, /**< drms */
    Edit = 0x65647473, /**< edts */
    EditList = 0x656C7374, /**< elst */
    Free = 0x66726565, /**< free */
    FileType = 0x66747970, /**< ftyp */
    HandlerReference = 0x68646c72, /**< hdlr */
    HintMediaHeader = 0x686D6864, /**< hmhd */
    ItunesList = 0x696c7374, /**< ilst */
    MediaData = 0x6d646174, /**< mdat */
    MediaHeader = 0x6d646864, /**< mdhd */
    Media = 0x6d646961, /**< mdia */
    Mean = 0x6D65616E, /**< mean */
    MovieExtendsHeader = 0x6D656864, /**< mehd */
    Meta = 0x6d657461, /**< meta */
    MovieFragmentHeader = 0x6D666864, /**< mfhd */
    MovieFragmentRandomAccess = 0x6d667261, /**< mfra */
    MediaInformation = 0x6d696e66, /**< minf */
    MediaInformationHeader = 0x676D6864, /**< gmhd */
    MediaInformationBase = 0x676D696E, /**< gmin */
    MediaInformationText = 0x74657874, /**< text */
    MovieFragment = 0x6d6f6f66, /**< moof */
    Movie = 0x6d6f6f76, /**< moov */
    MovieExtends = 0x6D766578, /**< mvex */
    MovieHeader = 0x6D766864, /**< mvhd */
    Name = 0x6E616D65, /**< name */
    NullMediaHeaderBox = 0x6E6D6864, /**< nmhd */
    PaddingBits = 0x70616462, /**< padb */
    PixalAspectRatio = 0x70617370, /**< pasp */
    ProgressiveDownloadInformation = 0x7064696e, /**< pdin */
    SampleToGroup = 0x73626770, /**< sbgp */
    IndependentAndDisposableSamples = 0x73647470, /**< sdtp */
    SampleGroupDescription = 0x73677064, /**< sgpd */
    Skip = 0x736b6970, /**< skip */
    SoundMediaHeader = 0x736D6864, /**< smhd */
    SampleTable = 0x7374626c, /**< stbl */
    ChunkOffset = 0x7374636f, /**< stco */
    DegradationPriority = 0x73746470, /**< stdp */
    SampleToChunk = 0x73747363, /**< stsc */
    SampleDescription = 0x73747364, /**< stsd */
    ShadowSyncSample = 0x73747368, /**< stsh */
    SyncSample = 0x73747373, /**< stss */
    SampleSize = 0x7374737A, /**< stsz */
    DecodingTimeToSample = 0x73747473, /**< stts */
    CompactSampleSize = 0x73747a32, /**< stz2 */
    SubSampleInformation = 0x73756273, /**< subs */
    TrackFragmentHeader = 0x74666864, /**< tfhd */
    TrackHeader = 0x746b6864, /**< tkhd */
    TrackFragment = 0x74726166, /**< traf */
    Track = 0x7472616b, /**< trak */
    TrackReference = 0x74726566, /**< tref */
    TrackExtends = 0x74726578, /**< trex */
    TrackFragmentRun = 0x7472756E, /**< trun */
    UserData = 0x75647461, /**< udta */
    DataEntryUrl = 0x75726C20, /**< url  */
    DataEntryUrn = 0x75726E20, /**< urn  */
    VideoMediaHeader = 0x766D6864, /**< vmhd */
    Wide = 0x77696465, /**< wide */
};
}

namespace Mp4TagAtomIds {
enum KnownValue : std::uint32_t {
    Album = 0xA9616c62, /**< ©alb */
    AlbumArtist = 0x61415254, /**< aART */
    Artist = 0xA9415254, /**< ©ART */
    Bpm = 0x746d706f, /**< tmpo */
    Category = 0x63617467, /**< catg */
    Comment = 0xA9636d74, /**< ©cmt */
    Composer = 0xA9777274, /**< ©wrt */
    Copyright = 0x63707274, /**< cprt */
    Cover = 0x636f7672, /**< covr */
    Description = 0x64657363, /**< desc */
    DiskPosition = 0x6469736b, /**< disk */
    Encoder = 0xA9746f6f, /**< ©too */
    EpisodeGlobalUniqueId = 0x65676964, /**< egid */
    Extended = 0x2d2d2d2d, /**< ---- */
    GaplessPlayback = 0x70676170, /**< pgap */
    Genre = 0xA967656e, /**< ©gen */
    Grouping = 0xA9677270, /**< ©grp */
    Keywords = 0x6b657977, /**< keyw */
    Lyricist = 0xA9737766, /**< ©swf */
    Lyrics = 0xA96c7972, /**< ©lyr */
    MediaType = 0x7374696B, /**< stik */
    Performers = 0xA9707266, /**< ©prf */
    Podcast = 0x70637374, /**< pcst */
    PodcastUrl = 0x7075726c, /**< purl */
    PreDefinedGenre = 0x676e7265, /**< gnre */
    Producer = 0xA9707264, /**< ©prd */
    PurchaseDate = 0x70757264, /**< purd */
    Rating = 0x72746e67, /**< rtng */
    RecordLabel = 0xA96c6162, /**< ©lab */
    Title = 0xA96e616d, /**< ©nam */
    TrackPosition = 0x74726b6e, /**< trkn */
    TvEpisode = 0x74766573, /**< tves */
    TvEpisodeName = 0x7476656e, /**< tven */
    TvNetworkName = 0x74766e6e, /**< tvnn */
    TvSeason = 0x7476736e, /**< tvsn */
    TvShowName = 0x74767368, /**< tvsh */
    Year = 0xA9646179, /**< ©day */
    Conductor = 0x40636F6E, /** ©con */
    Director = 0x40646972, /**< @dir */
    Narrator = 0xA96E7274, /** ©nrt */
    Publisher = 0xA9707562, /** ©pub */
    SortWith = 0x736F6E6D, /** sonm */
    Compilation = 0x6370696C, /** cpil (flag, set to 1 if it is a compilation) */
    SoundEngineer = 0xA9736E65, /** ©sne */
    ExecutiveProducer = 0xA9787064, /** ©xpd */
    ArtDirector = 0xA9617264, /** ©ard */
    Arranger = 0xA9617267, /** ©arg */
    Author = 0xA9617574, /** ©aut */
    OriginalArtist = 0xA96F7065, /** ©ope */
    Year2 = 0x79727263, /** yrrc (could be mapped to release date but likely not well supported elsewhere (e.g. Kodi uses ©day) */
};
}

namespace Mp4TagExtendedMeanIds {
extern std::string_view iTunes;
}

namespace Mp4TagExtendedNameIds {
extern std::string_view cdec;
extern std::string_view label;
} // namespace Mp4TagExtendedNameIds

namespace Mp4MediaTypeIds {
enum KnownValue : std::uint32_t {
    Sound = 0x736f756e, /**< soun: Sound/Audio */
    Video = 0x76696465, /**< vide: Video */
    Hint = 0x68696e74, /**< hint: Hint */
    Meta = 0x6d657461, /**< meta: Meta */
};
}

namespace FourccIds {
enum KnownValue : std::uint32_t {
    Ac3 = 0x61632d33, /**< ac-3: Dolby Digital */
    Ac4 = 0x61632d34, /**< ac-4: ? */
    AdpcmAcm = 0x6D730002, /**< ms??: ? */
    Agsm = 0x6167736D, /**< agsm */
    Alac = 0x616C6163, /**< alac: Apple Losless Audio Codec */
    Alaw = 0x616C6177, /**< alaw */
    Alaw21 = 0x616C6177, /**< alaw */
    AlphaCompositor = 0x626C6E64, /**< blnd */
    AlphaGain = 0x6761696E, /**< gain */
    Amr = 0x73617762, /**< sawb */
    AmrNarrowband = 0x73616D72, /**< samr */
    Animation = 0x726C6520, /**< rle : Animation */
    Appl1 = 0x6476690, /**< ?Gf? */
    Appl2 = 0x6C70630, /**< ?Ç?0 */
    Apple16BitGray = 0x62313667, /**< b16g */
    Apple32BitGrayWithAlpha = 0x62333261, /**< b32a */
    Apple48BitRgb = 0x62343872, /**< b48r */
    Apple64BitArgb1 = 0x62363461, /**< b64a */
    Apple64BitArgb2 = 0x62617365, /**< base */
    Apple64BitArgb3 = 0x626C6974, /**< blit */
    AppleAnimation = 0x726C6520, /**< rle  */
    AppleAvrJpeg = 0x61767220, /**< avr  */
    AppleBmp = 0x57524C45, /**< WRLE */
    AppleCinepak = 0x63766964, /**< cvid */
    AppleCmyk = 0x636D796B, /**< cmyk */
    AppleComponentVideoYuv422 = 0x79757632, /**< yuv2 */
    AppleCurve = 0x70617468, /**< path */
    AppleDvc = 0x64766320, /**< dvc  */
    AppleDvcpro = 0x64767070, /**< dvpp */
    AppleDvcpro501 = 0x6476356E, /**< dv5n */
    AppleDvcpro502 = 0x64763570, /**< dv5p */
    AppleDvcpro50Ntsc = 0x6476356E, /**< dv5n */
    AppleDvcpro50Pal = 0x64763570, /**< dv5p */
    AppleDvcproPal = 0x64767070, /**< dvpp */
    AppleDvDvcproNtsc = 0x6476630, /**< ?Gf0 */
    AppleDvp = 0x64766370, /**< dvcp */
    AppleDvPal = 0x64766370, /**< dvcp */
    AppleFlc = 0x666C6963, /**< flic */
    AppleGif = 0x67696620, /**< gif  */
    AppleGraphics = 0x736D630, /**< ?6Ö0 */
    AppleGsm101 = 0x6167736D, /**< agsm */
    AppleH261 = 0x68323631, /**< H.261 */
    AppleIntermediateCodec = 0x69636F64, /**< icod */
    AppleLossless = 0x616C6163, /**< alac */
    AppleMacpaint = 0x504E5447, /**< PNTG */
    AppleMicrosoftVideo1 = 0x6D737663, /**< msvc */
    AppleMotionJpegA = 0x6D6A7061, /**< mjpa */
    AppleMotionJpegB = 0x6D6A7062, /**< mjpb */
    AppleMpeg4Compressor = 0x6D703476, /**< mp4v */
    AppleMpeg4Decompressor = 0x6D703476, /**< mp4v */
    AppleOpendmlJpeg = 0x646D6231, /**< dmb1 */
    ApplePhotoCd = 0x6B706364, /**< kpcd */
    ApplePhotoJpeg = 0x6A706567, /**< jpeg */
    ApplePixletVideo = 0x70786C74, /**< pxlt */
    ApplePlanarRgb = 0x38425053, /**< 8BPS */
    ApplePng = 0x706E670, /**< ??æp */
    AppleQuickdraw = 0x71647277, /**< qdrw */
    AppleR408 = 0x72343038, /**< r408 */
    AppleScalingCodec = 0x7363616C, /**< scal */
    AppleSgi = 0x2E534749, /**< .SGI */
    AppleSorensonYuv9Codec = 0x73797639, /**< syv9 */
    AppleTextAtsuiCodec = 0x74657874, /**< text */
    AppleTga = 0x7467610, /**< ?Fv? */
    AppleTiff = 0x74696666, /**< tiff */
    AppleV408 = 0x76343038, /**< v408 */
    AppleVcH263 = 0x68323633, /**< h263 */
    AppleVideo = 0x72707A61, /**< rpza: Apple video */
    AppleYuv420Codec1 = 0x6A343230, /**< j420 */
    AppleYuv420Codec2 = 0x6D797576, /**< myuv */
    AppleYuv420Codec3 = 0x79343230, /**< y420 */
    AppleYuv422Codec2Vuy = 0x32767579, /**< 2vuy */
    AppleYuv422Codec4 = 0x79757678, /**< yuvx */
    AppleYuv422CodecYuvs = 0x79757673, /**< yuvs */
    AppleYuv422CodecYuvu = 0x79757675, /**< yuvu */
    Avc0Media = 0x64726D69, /**< drmi */
    Avc1 = 0x61766331, /**< avc1: H.264/MPEG-4 AVC video */
    Avc2 = 0x61766332, /**< avc2: H.264/MPEG-4 AVC video */
    Avc3 = 0x61766333, /**< avc3: H.264/MPEG-4 AVC video */
    Avc4 = 0x61766334, /**< avc4: H.264/MPEG-4 AVC video */
    Av1_IVF = 0x41563031, /**< AV01: AV1 video (found in IVF) */
    Av1_ISOBMFF = 0x61763031, /**< av01: AV1 video (found in ISOBMFF) */
    Blur = 0x626C7572, /**< blur */
    Bps8 = 0x38627073, /**< 8bps */
    BrightnessAndContrast = 0x6272636F, /**< brco */
    ChannelCompositor = 0x6368616E, /**< chan */
    ChromaKey = 0x636B6579, /**< ckey */
    Cinepak = 0x63766964, /**< cvid: Cinepak */
    Cloud = 0x636C6F75, /**< clou */
    ColorStyle = 0x736F6C72, /**< solr */
    Colorsync = 0x73796E63, /**< sync */
    ColorTint = 0x74696E74, /**< tint */
    CrossFade = 0x64736C76, /**< dslv */
    Cvid = 0x63766964, /**< cvid */
    Divx3Decoder1 = 0x41503431, /**< AP41 */
    Divx3Decoder2 = 0x434F4C30, /**< COL0 */
    Divx3Decoder3 = 0x434F4C31, /**< COL1 */
    Divx3Decoder4 = 0x44495633, /**< DIV3 */
    Divx3Decoder5 = 0x44495634, /**< DIV4 */
    Divx3Decoder6 = 0x44495635, /**< DIV5 */
    Divx3Decoder7 = 0x44495636, /**< DIV6 */
    Divx3Decoder8 = 0x4D504733, /**< MPG3 */
    Divx3Decoder9 = 0x6D706733, /**< mpg3 */
    Divx3Decoder10 = 0x636F6C30, /**< col0 */
    Divx3Decoder11 = 0x636F6C31, /**< col1 */
    Divx3Decoder12 = 0x64697633, /**< div3 */
    Divx3Decoder13 = 0x64697634, /**< div4 */
    Divx3Decoder14 = 0x64697635, /**< div5 */
    Divx3Decoder15 = 0x64697636, /**< div6 */
    Divx4Decoder1 = 0x44495658, /**< DIVX */
    Divx4Decoder2 = 0x64697678, /**< divx */
    Divx5Decoder = 0x44583530, /**< DX50 */
    Drms = 0x64726D73, /**< drms */
    Drmi = 0x64726D69, /**< drmi */
    Dts = 0x6474736C, /**< dtsl */
    DtsH = 0x64747368, /**< dtsh */
    DtsE = 0x64747365, /**< dtse */
    Dvca = 0x64766361, /**< dvca */
    DvcPro501 = 0x64763570, /**< dv5p */
    DvcPro502 = 0x6476356E, /**< dv5n */
    DvcProPal = 0x64767070, /**< dvpp */
    EAc3 = 0x65632D33, /**< ec-3 */
    EdgeDetection = 0x65646765, /**< edge */
    Emboss = 0x656D6273, /**< embs */
    Explode = 0x78706C6F, /**< xplo */
    FilmNoise = 0x666D6E73, /**< fmns */
    Fire = 0x66697265, /**< fire */
    Flac = 0x664C6143, /**< fLaC */
    FlashPixImage = 0x66706978, /**< fpix */
    FlashScreenVideoDecoder = 0x46535631, /**< FSV1 */
    FloatingPoint32Bit = 0x666C3332, /**< fl32 */
    FloatingPoint64Bit = 0x666C3634, /**< fl64 */
    GeneralConvolution = 0x67656E6B, /**< genk */
    Gif = 0x67696620, /**< gif : CompuServe Graphics Interchange Format */
    Glass = 0x676C6173, /**< glas */
    GradientWipe = 0x6D617474, /**< matt */
    Graphics = 0x736D6320, /**< smc : Graphics */
    H263Quicktime = 0x68323633, /**< h263: H.263 (Quicktime) */
    H2633GPP = 0x73323633, /**< s263: H.263 (3GPP format) */
    H264Decoder1 = 0x44415643, /**< DAVC */
    H264Decoder2 = 0x48323634, /**< H264 */
    H264Decoder3 = 0x56535348, /**< VSSH */
    H264Decoder4 = 0x58323634, /**< X264 */
    H264Decoder5 = 0x68323634, /**< h264 */
    H264Decoder6 = 0x78323634, /**< x264 */
    Hdv3 = 0x68647633, /**< hdv3 */
    Hevc1 = 0x68766331, /**< hvc1: H.265/High Efficiency Video Coding */
    Hevc2 = 0x68657631, /**< hev1: H.265/High Efficiency Video Coding */
    HslBalance = 0x68736C62, /**< hslb */
    Ima4 = 0x696D6134, /**< ima4 */
    Ima41 = 0x696D6134, /**< ima4 */
    ImaadpcmAcm = 0x6D730011, /**< ms??: ? */
    Implode = 0x6D706C6F, /**< mplo */
    Int16Be = 0x74776F73, /**< twos */
    Int16Le = 0x736F7774, /**< sowt */
    Int24 = 0x696E3234, /**< in24 */
    Int32 = 0x696E3332, /**< in32 */
    Iris = 0x736D7032, /**< smp2 */
    IvxDecoder1 = 0x33495632, /**< 3IV2 */
    IvxDecoder2 = 0x33495644, /**< 3IVD */
    IvxDecoder3 = 0x33697632, /**< 3iv2 */
    IvxDecoder4 = 0x33697664, /**< 3ivd */
    Jpeg = 0x6a706567, /**< jpeg: JPEG */
    Jpeg2000Decoder = 0x6D6A7032, /**< mjp2 */
    Law21 = 0x756C6177, /**< ulaw */
    LensFlare = 0x6C656E73, /**< lens */
    Lle = 0x726C6520, /**< rle  */
    Mac3 = 0x6D616333, /**< mac3 */
    Mac6 = 0x6D616336, /**< mac6 */
    Mace31 = 0x4D414333, /**< MAC3 */
    Mace61 = 0x4D414336, /**< MAC6 */
    MatrixWipe = 0x736D7034, /**< smp4 */
    DolbyMpl = 0x6D6C7061, /**< mlpa */
    MotionJpegA = 0x6D6A7061, /**< mjpa: Motion-JPEG (format A) */
    MotionJpegB = 0x6D6A7062, /**< mjpb: Motion-JPEG (format B) */
    Mp3 = 0x2e6d7033, /**< .mp3: MPEG-1 Layer 3 */
    Mp3CbrOnly = 0x6D730055, /**< ms?U: MPEG-1 Layer 3 (constant bitrate only) */
    Mpeg = 0x4D504547, /**< MPEG */
    Mpeg2Imx30 = 0x6D78336E, /**< mx3n */
    Mpeg2Imx50 = 0x6D783570, /**< mx5p */
    Mpeg4Audio = 0x6d703461, /**< mp4a: MPEG-4 audio */
    Mpeg4Decoder1 = 0x464D5034, /**< FMP4 */
    Mpeg4Decoder2 = 0x53454447, /**< SEDG */
    Mpeg4Decoder3 = 0x57563146, /**< WV1F */
    Mpeg4Sample = 0x6d703473, /**< mp4s: MPEG-4 stream (other than video/audio) */
    Mpeg4TimedText = 0x74783367, /**< tx3g: MPEG-4 Timed Text / Streaming text format / Part 17 */
    Mpeg4Video = 0x6d703476, /**< mp4v: MPEG-4 video */
    MsMpeg4V1Decoder1 = 0x44495631, /**< DIV1 */
    MsMpeg4V1Decoder2 = 0x64697631, /**< div1 */
    MsMpeg4V1Decoder3 = 0x4D504734, /**< MPG4 */
    MsMpeg4V1Decoder4 = 0x6D706734, /**< mpg4 */
    MsMpeg4V1Decoder5 = 0x4D504731, /**< MPG1 */
    MsMpeg4V1Decoder6 = 0x6D706731, /**< mpg1 */
    MsMpeg4V2Decoder1 = 0x44495632, /**< DIV2 */
    MsMpeg4V2Decoder2 = 0x64697632, /**< div2 */
    MsMpeg4V2Decoder3 = 0x4D503432, /**< MP42 */
    MsMpeg4V2Decoder4 = 0x6D703432, /**< mp42 */
    MsMpeg4V3Decoder1 = 0x4D503433, /**< MP43 */
    MsMpeg4V3Decoder2 = 0x6D703433, /**< mp43 */
    NtscDv25Video = 0x64766320, /**< dvc : NTSC DV-25 video */
    Oggs = 0x4F676753, /**< OggS */
    Opus = 0x4F707573, /**< Opus */
    PalDv25Video = 0x64766370, /**< dvcp: PAL DV-25 video */
    PdfImage = 0x70646620, /**< pdf  */
    Png = 0x706E6720, /**< png : Portable Network Graphics */
    Push = 0x70757368, /**< push */
    Qclp = 0x51636C70, /**< Qclp */
    QdesignMusic1Decoder = 0x51444D43, /**< QDMC */
    QdesignMusic2 = 0x51444D32, /**< QDM2 */
    Qdmc = 0x51444D43, /**< QDMC */
    Qdrw = 0x71647277, /**< qdrw */
    QtvrCubicCodec = 0x63757061, /**< cupa */
    QtvrCylindricalCodec = 0x6C747061, /**< ltpa */
    QualcommPurevoice = 0x51636C70, /**< Qclp */
    QualcommQcelp = 0x51636C71, /**< Qclq */
    Radial = 0x736D7033, /**< smp3 */
    Raw = 0x72617720, /**< raw : Uncompressed RGB */
    RgbBalance = 0x72676262, /**< rgbb */
    Ripple = 0x7269706C, /**< ripl */
    Rpza = 0x72707A61, /**< rpza */
    Rv20 = 0x52563230, /**< RV20 */
    Rv30 = 0x52563330, /**< RV30 */
    Rv40 = 0x52563430, /**< RV40 */
    Sharpen = 0x73687270, /**< shrp */
    Slide = 0x736C6964, /**< slid */
    Smc = 0x736D6320, /**< smc  */
    SorensonH263Decoder = 0x464C5631, /**< FLV1 */
    SorensonVideo3Compressor = 0x53565133, /**< SVQ3 */
    SorensonVideoDecompressor = 0x53565131, /**< SVQ1 */
    Sowt = 0x736F7774, /**< sowt */
    SpecialEffectsAndFilters = 0x67656666, /**< geff */
    Svq1 = 0x73767131, /**< svq1 */
    Svq3 = 0x73767133, /**< svq3 */
    Tiff = 0x74696666, /**< tiff: Tagged Image File Format */
    TravelingMatte = 0x74726176, /**< trav */
    TruemotionVp6Decoder1 = 0x56503632, /**< VP62 */
    TruemotionVp6Decoder2 = 0x56503646, /**< VP6F */
    Twos = 0x74776F73, /**< twos */
    Ulaw = 0x756C6177, /**< ulaw */
    Ulaw21 = 0x756C6177, /**< ulaw */
    VcmImageCodec = 0x4D6A7067, /**< Mjpg */
    Vdva = 0x76647661, /**< vdva */
    Vp8 = 0x56503830, /**< VP80: VP8 video */
    Vp9 = 0x56503930, /**< VP90: VP9 video */
    Vp9_2 = 0x76703039, /**< VP9 video */
    WavPack = 0x5756504B, /**< WVPK */
    WindowsMediaAudio = 0x6F776D61, /**< owma: ? */
    WindowsMediaAudio7 = 0x574D4131, /**< WMA1 */
    WindowsMediaAudio9Professional = 0x574D4133, /**< WMA3 */
    WindowsMediaAudio9Standard = 0x574D4132, /**< WMA2 */
    WindowsMediaVideoV17 = 0x574D5637, /**< WMV7 */
    WindowsMediaVideoV2 = 0x574D5632, /**< WMV2 */
    WindowsMediaVideoV8 = 0x574D5638, /**< WMV8 */
    Wipe = 0x736D7074, /**< smpt */
    WmvImageCodec1 = 0x4D347332, /**< M4s2 */
    WmvImageCodec2 = 0x4D703432, /**< Mp42 */
    WmvImageCodec3 = 0x4D703433, /**< Mp43 */
    WmvImageCodec4 = 0x4D703453, /**< Mp4S */
    WmvImageCodec5 = 0x574D5631, /**< WMV1 */
    WmvImageCodec6 = 0x574D5632, /**< WMV2 */
    WmvImageCodec7 = 0x574D5633, /**< WMV3 */
    XvidDecoder1 = 0x424C5A30, /**< BLZ0 */
    XvidDecoder2 = 0x58564944, /**< XVID */
    XvidDecoder3 = 0x58564958, /**< XVIX */
    XvidDecoder4 = 0x58766944, /**< XviD */
    XvidDecoder5 = 0x78766964, /**< xvid */
    Yuv422HardwareAccelerationCodecYuvs1 = 0x32767579, /**< 2vuy */
    Yuv422HardwareAccelerationCodecYuvs2 = 0x61633136, /**< ac16 */
    Yuv422HardwareAccelerationCodecYuvs3 = 0x61633332, /**< ac32 */
    Yuv422HardwareAccelerationCodecYuvs4 = 0x61634247, /**< acBG */
    Yuv422HardwareAccelerationCodecYuvs5 = 0x79757673, /**< yuvs */
    Zoom = 0x7A6F6F6D, /**< zoom */
};

TAG_PARSER_EXPORT MediaFormat fourccToMediaFormat(std::uint32_t fourccId);

} // namespace FourccIds

namespace Mp4FormatExtensionIds {
enum KnownValue : std::uint32_t {
    GammaLevel
    = 0x67616D61, /**< gama: A 32-bit fixed-point number indicating the gamma level at which the image was captured. The decompressor can use this value to gamma-correct at display time. */
    FieldHandling = 0x6669656C, /**< field: Two 8-bit integers that define field handling. */
    DefaultQuantizationTable = 0x6D6A7174, /**< mjqt: The default quantization table for a Motion-JPEG data stream. */
    DefaultHuffmanTable = 0x6D6A6874, /**< mjht: The default Huffman table for a Motion-JPEG data stream. */
    Mpeg4ElementaryStreamDescriptor
    = 0x65736473, /**< esds: An MPEG-4 elementary stream descriptor atom. This extension is required for MPEG-4 video. */
    Mpeg4ElementaryStreamDescriptor2 = 0x6D346473, /**< m4ds: Alternative if encoded to AVC stanard. */
    AvcConfiguration
    = 0x61766343, /**< avcC: An H.264 AVCConfigurationBox. This extension is required for H.264 video as defined in ISO/IEC 14496-15. */
    PixelAspectRatio = 0x70617370, /**< pasp: Pixel aspect ratio. This extension is mandatory for video formats that use non-square pixels. */
    ColorParameters = 0x636F6C72, /**< colr: An image description extension required for all uncompressed Y´CbCr video types. */
    CleanAperature = 0x636C6170, /**< clap: Spatial relationship of Y´CbCr components relative to a canonical image center. */
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
    Mpeg2AacScaleableSamplingRateProfile, /**< Advanced Audio Coding Scalable Sampling Rate Profile */
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

TAG_PARSER_EXPORT std::string_view streamTypeName(std::uint8_t streamTypeId);

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

TAG_PARSER_EXPORT std::string_view channelConfigString(std::uint8_t config);
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
