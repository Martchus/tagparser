#ifndef MP4TAGATOMNAMES_H
#define MP4TAGATOMNAMES_H

#include <c++utilities/conversion/types.h>

namespace Media
{

class MediaFormat;

namespace Mp4AtomIds {
enum KnownValue : uint32 {
    AvcConfiguration = 0x61766343,
    BitrateBox = 0x62747274,
    CleanAperature = 0x636c6170,
    ChunkOffset64 = 0x636f3634,
    CompositionTimeToSample = 0x63747473,
    Data = 0x64617461,
    DataInformation = 0x64696e66,
    DataReference = 0x64726566,
    Drms = 0x64726D73,
    Edts = 0x65647473,
    Edit = 0x656C7374,
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
enum KnownValue : uint32 {
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
}

namespace Mp4MediaTypeIds {
enum KnownValue : uint32 {
    Sound = 0x736f756e, /**< Sound/Audio */
    Video = 0x76696465, /**< Video */
    Hint = 0x68696e74, /**< Hint */
    Meta = 0x6d657461 /**< Meta */
};
}

namespace Mp4FormatIds {
enum KnownValue : uint32 {
    Cinepak = 0x63766964, /**< Cinepak */
    Mpeg4Video = 0x6d703476, /**< MPEG-4 video */
    Graphics = 0x736D6320, /**< Graphics */
    Animation = 0x726C6520, /**< Animation */
    AppleVideo = 0x72707A61, /**< Apple video */
    Png = 0x706E6720, /**< Portable Network Graphics */
    Avc1 = 0x61766331, /**< H.264/MPEG-4 AVC video */
    Avc2 = 0x61766332, /**< H.264/MPEG-4 AVC video */
    Avc3 = 0x61766333, /**< H.264/MPEG-4 AVC video */
    Avc4 = 0x61766334, /**< H.264/MPEG-4 AVC video */
    H263 = 0x68323633, /**< H.263/MPEG-4 ASP video */
    Tiff = 0x74696666, /**< Tagged Image File Format */
    Jpeg = 0x6a706567, /**< JPEG */
    Raw = 0x72617720, /**< Uncompressed RGB */
    Gif = 0x67696620, /**< CompuServe Graphics Interchange Format */
    NtscDv25Video = 0x64766320, /**< NTSC DV-25 video */
    PalDv25Video = 0x64766370, /**< PAL DV-25 video */
    MotionJpegA = 0x6D6A7061, /**< Motion-JPEG (format A) */
    MotionJpegB = 0x6D6A7062, /**< Motion-JPEG (format B) */
    Mp3 = 0x2e6d7033, /**< MPEG-1 Layer 3 */
    Mpeg4Audio = 0x6d703461, /**< MPEG-4 audio */
    Mpeg4Stream = 0x6d703473, /**< MPEG-4 stream (other then video/audio) */
    Alac = 0x616C6163, /**< Apple Losless Audio Codec */
    Ac3 = 0x61632d33, /**< Dolby Digital */
    Ac4 = 0x61632d34, /**< ? */
    AdpcmAcm = 0x6D730002, /**< ? */
    ImaadpcmAcm = 0x6D730011, /**< ? */
    Mp3CbrOnly = 0x6D730055 /**< MPEG-1 Layer 3 (constant bitrate only) */
};

MediaFormat fourccToMediaFormat(uint32 fourccId);

}

namespace Mp4FormatExtensionIds {
enum KnownValue : uint32 {
    GammaLevel = 0x67616D61, /**< A 32-bit fixed-point number indicating the gamma level at which the image was captured. The decompressor can use this value to gamma-correct at display time. */
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
enum KnownValue : byte {
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
    AacMainProfile, /**< Advanced Audio Coding Main Profile */
    AacLowComplexityProfile, /**< Advanced Audio Coding Low Complexity Profile */
    AacScaleableSamplingRateProfile, /**< Advanced Audio Coding Scaleable Sampling Rate Profile */
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
    PrivateQcelp = 0xE1 /**< QCELP */
};

MediaFormat streamObjectTypeFormat(byte streamObjectTypeId);

}

namespace Mpeg4ElementaryStreamTypeIds {
enum KnownValue : byte {
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

const char *streamTypeName(byte streamTypeId);

}

namespace Mpeg4DescriptorIds {
enum KnownValue : byte {
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
enum KnownValue : byte {
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
}

/*!
 * \brief Specifies the tag type.
 */
enum class Mp4TagMediaType : byte {
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
enum class Mp4TagContentRating : byte {
    None = 0, /**< None */
    Clean = 2, /**< Clean */
    Explicit = 4 /**< Explicit */
};

/*!
 * \brief Specifies the account type.
 */
enum class AccountType : byte
{
    Itunes = 0,
    Aol = 1,
    Undefined = 255
};

/*!
 * \brief Specifies the country.
 */
enum class CountryCode
{
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

}

#endif // MP4TAGATOMNAMES_H
