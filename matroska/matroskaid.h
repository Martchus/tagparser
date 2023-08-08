#ifndef TAG_PARSER_MATROSKAIDS_H
#define TAG_PARSER_MATROSKAIDS_H

#include "../global.h"

#include <cstdint>
#include <string_view>

namespace TagParser {

namespace MatroskaIds {

/*!
 * \brief Encapsulates all top level ID values.
 */
enum TopLevelIds {
    Segment = 0x18538067,
    SegmentInfo = 0x1549A966,
    Tracks = 0x1654AE6B,
    Cues = 0x1C53BB6B,
    Tags = 0x1254C367,
    SeekHead = 0x114D9B74,
    Cluster = 0x1F43B675,
    Attachments = 0x1941A469,
    Chapters = 0x1043A770
};

/*!
 * \brief Encapsulates all ID values in the SeekHead master.
 */
enum SeekHeadIds {
    Seek = 0x4DBB,
};

/*!
 * \brief Encapsulates all ID values in the Seek master.
 */
enum SeekIds { SeekID = 0x53AB, SeekPosition = 0x53AC };

/*!
 * \brief Encapsulates all ID values in the SegmentInfo master.
 */
enum SegmentInfoIds {
    TimeCodeScale = 0x2AD7B1,
    Duration = 0x4489,
    WrittingApp = 0x5741, // TODOv13: change to WritingApp
    MuxingApp = 0x4D80,
    DateUTC = 0x4461,
    SegmentUID = 0x73A4,
    SegmentFileName = 0x7384,
    PrevUID = 0x3CB923,
    PrevFileName = 0x3C83AB,
    NexUID = 0x3EB923,
    NextFileName = 0x3E83BB,
    Title = 0x7BA9,
    SegmentFamily = 0x4444,
    ChapterTranslate = 0x6924
};

/*!
 * \brief Encapsulates all ID values in the ChapterTranslate master.
 */
enum ChapterTranslateIds { ChapterTranslateEditionUID = 0x69fc, ChapterTranslateCodec = 0x69bf, ChapterTranslateID = 0x69a5 };

/*!
 * \brief Encapsulates all ID values in the Tracks master.
 */
enum TracksIds {
    TrackEntry = 0xAE,
};

/*!
 * \brief Encapsulates all ID values in the TrackEntry master.
 */
enum TrackEntryIds {
    TrackNumber = 0xD7,
    TrackUID = 0x73C5,
    TrackType = 0x83,
    TrackFlagEnabled = 0xB9,
    TrackFlagDefault = 0x88,
    TrackFlagForced = 0x55AA,
    TrackFlagHearingImpaired = 0x55AB,
    TrackFlagVisualImpaired = 0x55AC,
    TrackFlagTextDescriptions = 0x55AD,
    TrackFlagOriginal = 0x55AE,
    TrackFlagCommentary = 0x55AF,
    TrackFlagLacing = 0x9C,
    MinCache = 0x6DE7,
    MaxCache = 0x6DF8,
    DefaultDuration = 0x23E383,
    DefaultDecodedFieldDuration = 0x234e7a,
    TrackTimeCodeScale = 0x23314F,
    TrackOffset = 0x537F,
    MaxBlockAdditionId = 0x55EE,
    TrackName = 0x536E,
    TrackLanguage = 0x22B59C,
    TrackLanguageIETF = 0x22B59D,
    CodecID = 0x86,
    CodecPrivate = 0x63A2,
    CodecName = 0x258688,
    AttachmentLink = 0x7446,
    CodecSettings = 0x3A9697,
    CodecInfoUrl = 0x3B4040,
    CodecDownloadUrl = 0x26B240,
    CodecDecodeAll = 0xAA,
    TrackOverlay = 0x6FAB,
    CodecDelay = 0x56aa,
    SeekPreRoll = 0x56bb,
    TrackTranslate = 0x6624,
    TrackAudio = 0xE1,
    TrackVideo = 0xE0,
    TrackOperation = 0xe2,
    TrickTrackUID = 0xc0,
    TrickTrackSegmentUID = 0xc1,
    TrickTrackFlag = 0xc6,
    TrickMasterTrackUID = 0xc7,
    TrickMasterTrackSegmentUID = 0xc4,
    ContentEncodings = 0x6D80
};

/*!
 * \brief Encapsulates all ID values in the TrackTranslate master.
 */
enum TrackTranslateIds { TrackTranslateEditionUID = 0x66fc, TrackTranslateCodec = 0x66bf, TrackTranslateTrackID = 0x66a5 };

/*!
 * \brief Encapsulates all ID values in the TrackVideo master.
 */
enum TrackVideoIds {
    FlagInterlaced = 0x9A,
    StereoMode = 0x53B8,
    AlphaMode = 0x53c0,
    OldStereoMode = 0x53b9,
    PixelWidth = 0xB0,
    PixelHeight = 0xBA,
    PixelCropBottom = 0x54AA,
    PixelCropTop = 0x54BB,
    PixelCropLeft = 0x54CC,
    PixelCropRight = 0x54DD,
    DisplayWidth = 0x54B0,
    DisplayHeight = 0x54BA,
    DisplayUnit = 0x54B2,
    AspectRatioType = 0x54B3,
    ColorSpace = 0x2EB524,
    GammaValue = 0x2FB523,
    FrameRate = 0x2383E3
};

/*!
 * \brief Encapsulates all ID values in the TrackAudio master.
 */
enum TrackAudioIds { SamplingFrequency = 0xB5, OutputSamplingFrequency = 0x78B5, Channels = 0x9F, ChannelsPositions = 0x7D7B, BitDepth = 0x6264 };

/*!
 * \brief Encapsulates all ID values in the TrackOperation master.
 */
enum TrackOperationIds { TrackCombinePlanes = 0xe3, TrackJoinBlocks = 0xe9 };

/*!
 * \brief Encapsulates all ID values in the TrackCombinePlanes master.
 */
enum TrackCombinePlanesIds { TrackPlane = 0xe4 };

/*!
 * \brief Encapsulates all ID values in the TrackPlane master.
 */
enum TrackPlaneIds { TrackPlaneUID = 0xe5, TrackPlaneType = 0xe6 };

/*!
 * \brief Encapsulates all ID values in the TrackJoinBlocks master.
 */
enum TrackJoinBlocksIds { TrackJoinUID = 0xed };

/*!
 * \brief Encapsulates all ID values in the ContentEncodings master.
 */
enum ContentEncodingsIds { ContentEncoding = 0x6240 };

/*!
 * \brief Encapsulates all ID values in the ContentEncoding master.
 */
enum ContentEncodingIds {
    ContentEncodingOrder = 0x5031,
    ContentEncodingScope = 0x5032,
    ContentEncodingType = 0x5033,
    ContentCompression = 0x5034,
    ContentEncryption = 0x5035
};

/*!
 * \brief Encapsulates all ID values in the ContentCompression master.
 */
enum ContentCompressionIds { ContentCompAlgo = 0x4254, ContentCompSettings = 0x4255 };

/*!
 * \brief Encapsulates all ID values in the ContentEncryption master.
 */
enum ContentEncryptionIds {
    ContentEncAlgo = 0x47e1,
    ContentEncKeyID = 0x47e2,
    ContentSignature = 0x47e3,
    ContentSigKeyID = 0x47e4,
    ContentSigAlgo = 0x47e5,
    ContentSigHashAlgo = 0x47e6
};

/*!
 * \brief Encapsulates all ID values in the Tags master.
 */
enum TagsIds {
    Tag = 0x7373,
};

/*!
 * \brief Encapsulates all ID values in the Tag master.
 */
enum TagIds {
    SimpleTag = 0x67C8,
    Targets = 0x63C0,
};

/*!
 * \brief Encapsulates all ID values in the SimpleTag master.
 */
enum SimpleTagIds {
    TagName = 0x45A3,
    TagString = 0x4487,
    TagLanguage = 0x447A,
    TagLanguageIETF = 0x447B,
    TagDefault = 0x4484,
    TagBinary = 0x4485,
};

/*!
 * \brief Encapsulates all ID values in the Targets master.
 */
enum TargetsIds {
    TargetTypeValue = 0x68ca,
    TargetType = 0x63ca,
    TagTrackUID = 0x63c5,
    TagEditionUID = 0x63c9,
    TagChapterUID = 0x63c4,
    TagAttachmentUID = 0x63c6
};

/*!
 * \brief Encapsulates all ID values in the Cues master.
 */
enum CuesIds { CuePoint = 0xbb };

/*!
 * \brief Encapsulates all ID values in the CuePoint master.
 */
enum CuePointIds { CueTime = 0xb3, CueTrackPositions = 0xb7 };

/*!
 * \brief Encapsulates all ID values in the CueTrackPositions master.
 */
enum CueTrackPositionsIds {
    CueTrack = 0xf7,
    CueClusterPosition = 0xf1,
    CueRelativePosition = 0xf0,
    CueDuration = 0xb2,
    CueBlockNumber = 0x5378,
    CueCodecState = 0xea,
    CueReference = 0xdb
};

/*!
 * \brief Encapsulates all ID values in the CueReference master.
 */
enum CueReferenceIds { CueRefTime = 0x96, CueRefCluster = 0x97, CueRefNumber = 0x535f, CueRefCodecState = 0xeb };

/*!
 * \brief Encapsulates all ID values in the Attachments master.
 */
enum AttachmentsIds { AttachedFile = 0x61a7 };

/*!
 * \brief Encapsulates all ID values in the AttachedFile master.
 */
enum AttachedFileIds {
    FileDescription = 0x467e,
    FileName = 0x466e,
    FileMimeType = 0x4660,
    FileData = 0x465c,
    FileUID = 0x46ae,
    FileReferral = 0x4675,
    FileUsedStartTime = 0x4661,
    FileUsedEndTime = 0x4662
};

/*!
 * \brief Encapsulates all ID values in the Chapters master.
 */
enum ChaptersIds { EditionEntry = 0x45b9 };

/*!
 * \brief Encapsulates all ID values in the EditionEntry master.
 */
enum EditionEntryIds {
    EditionUID = 0x45bc,
    EditionFlagHidden = 0x45bd,
    EditionFlagDefault = 0x45db,
    EditionFlagOrdered = 0x45dd,
    ChapterAtom = 0xb6
};

/*!
 * \brief Encapsulates all ID values in the ChapterAtom master.
 */
enum ChapterAtomIds {
    ChapterUID = 0x73c4,
    ChapterStringUID = 0x5654,
    ChapterTimeStart = 0x91,
    ChapterTimeEnd = 0x92,
    ChapterFlagHidden = 0x98,
    ChapterFlagEnabled = 0x4598,
    ChapterSegmentUID = 0x6e67,
    ChapterSegmentEditionUID = 0x6ebc,
    ChapterPhysicalEquiv = 0x63c3,
    ChapterTrack = 0x8f,
    ChapterDisplay = 0x80,
    ChapProcess = 0x6944
};

/*!
 * \brief Encapsulates all ID values in the ChapterTrack master.
 */
enum ChapterTrackIds { ChapterTrackNumber = 0x89 };

/*!
 * \brief Encapsulates all ID values in the ChapterDisplay master.
 */
enum ChapterDisplayIds { ChapString = 0x85, ChapLanguage = 0x437c, ChapLanguageIETF = 0x437D, ChapCountry = 0x437e };

/*!
 * \brief Encapsulates all ID values in the ChapProcess master.
 */
enum ChapProcessIds { ChapProcessCodecID = 0x6955, ChapProcessPrivate = 0x450d, ChapProcessCommand = 0x6911 };

/*!
 * \brief Encapsulates all ID values in the ChapProcessCommand master.
 */
enum ChapProcessCommandIds { ChapProcessTime = 0x6922, ChapProcessData = 0x6933 };

/*!
 * \brief Encapsulates all ID values in the Cluster master.
 */
enum ClusterIds {
    Timecode = 0xe7,
    SilentTracks = 0x5854,
    Position = 0xa7,
    PrevSize = 0xab,
    SimpleBlock = 0xa3,
    BlockGroup = 0xa0,
    EncryptedBlock = 0xaf
};

/*!
 * \brief Encapsulates all ID values in the SilentTracks master.
 */
enum SilentTracksIds { SilentTrackNumber = 0x58d7 };

/*!
 * \brief Encapsulates all ID values in the BlockGroup master.
 */
enum BlockGroupIds {
    Block = 0xa1,
    BlockVirtual = 0xa2,
    BlockAdditions = 0x75a1,
    BlockDuration = 0x9b,
    ReferencePriority = 0xfa,
    ReferenceBlock = 0xfb,
    ReferenceVirtual = 0xfd,
    CodecState = 0xa4,
    DiscardPadding = 0x75a2,
    Slices = 0x8e,
    ReferenceFrame = 0xc8,
};

/*!
 * \brief Encapsulates all ID values in the BlockAdditions master.
 */
enum BlockAdditionsIds { BlockMore = 0xa6 };

/*!
 * \brief Encapsulates all ID values in the BlockMore master.
 */
enum BlockMoreIds { BlockAddID = 0xee, BlockAdditional = 0x45 };

/*!
 * \brief Encapsulates all ID values in the Slices master.
 */
enum SlicesIds { TimeSlice = 0xe8 };

/*!
 * \brief Encapsulates all ID values in the TimeSlice master.
 */
enum TimeSliceIds { LaceNumber = 0xcc, FrameNumber = 0xcd, BlockAdditionID = 0xcb, Delay = 0xce, SliceDuration = 0xcf };

/*!
 * \brief Encapsulates all ID values in the ReferenceFrame master.
 */
enum ReferenceFrameIds { ReferenceOffset = 0xc9, ReferenceTimeCode = 0xca };

} // namespace MatroskaIds

namespace MatroskaTrackType {

enum KnownValues { Video = 0x1, Audio = 0x2, Complex = 0x3, Logo = 0x10, Subtitle = 0x11, Buttons = 0x12, Control = 0x20 };
}

enum class MatroskaElementLevel : std::uint8_t {
    TopLevel = 0x0,
    Level1,
    Level2,
    Level3,
    Level4,
    Level5,
    Level6,
    Global = 0xFE,
    Unknown = 0xFF,
};

constexpr bool operator>(MatroskaElementLevel lhs, MatroskaElementLevel rhs)
{
    return static_cast<std::uint8_t>(lhs) < static_cast<std::uint8_t>(MatroskaElementLevel::Global)
        && static_cast<std::uint8_t>(rhs) < static_cast<std::uint8_t>(MatroskaElementLevel::Global)
        && static_cast<std::uint8_t>(lhs) > static_cast<std::uint8_t>(rhs);
}

constexpr bool operator>(std::uint8_t lhs, MatroskaElementLevel rhs)
{
    return lhs < static_cast<std::uint8_t>(MatroskaElementLevel::Global)
        && static_cast<std::uint8_t>(rhs) < static_cast<std::uint8_t>(MatroskaElementLevel::Global)
        && static_cast<std::uint8_t>(lhs) > static_cast<std::uint8_t>(rhs);
}

constexpr bool operator<(MatroskaElementLevel lhs, MatroskaElementLevel rhs)
{
    return static_cast<std::uint8_t>(lhs) < static_cast<std::uint8_t>(MatroskaElementLevel::Global)
        && static_cast<std::uint8_t>(rhs) < static_cast<std::uint8_t>(MatroskaElementLevel::Global)
        && static_cast<std::uint8_t>(lhs) < static_cast<std::uint8_t>(rhs);
}

constexpr bool operator>=(MatroskaElementLevel lhs, MatroskaElementLevel rhs)
{
    return lhs == rhs || lhs > rhs;
}

constexpr bool operator<=(MatroskaElementLevel lhs, MatroskaElementLevel rhs)
{
    return lhs == rhs || lhs < rhs;
}

TAG_PARSER_EXPORT std::string_view matroskaIdName(std::uint32_t matroskaId);
TAG_PARSER_EXPORT MatroskaElementLevel matroskaIdLevel(std::uint32_t matroskaId);

} // namespace TagParser

#endif // TAG_PARSER_MATROSKAIDS_H
