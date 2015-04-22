#ifndef MP4TAGATOMNAMES_H
#define MP4TAGATOMNAMES_H

#include <c++utilities/conversion/types.h>

namespace Media
{

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
    DataEntryUrl  = 0x75726C20,
    DataEntryUrn  = 0x75726E20,
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
    Sound = 0x736f756e,
    Video = 0x76696465,
    Hint = 0x68696e74,
    Meta = 0x6d657461
};
}

namespace Mp4FormatIds {
enum KnownValue : uint32 {
    Mpeg4Visual = 0x6d703476,
    Avc1 = 0x61766331,
    Avc2 = 0x61766332,
    Avc3 = 0x61766333,
    Avc4 = 0x61766334,
    H263 = 0x68323633,
    Tiff = 0x74696666,
    Jpeg = 0x6a706567,
    Raw = 0x72617720,
    Gif = 0x67696620,
    Mp3 = 0x2e6d7033,
    Mpeg4Audio = 0x6d703461,
    Alac = 0x616C6163,
    Ac3 = 0x61632d33,
    Ac4 = 0x61632d34,
    AdpcmAcm = 0x6D730002,
    ImaadpcmAcm = 0x6D730011,
    Mp3CbrOnly = 0x6D730055
};
}

namespace Mp4FormatConfigurationIds {
enum KnownValue : uint32 {
    AvcC = 0x61766343
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
