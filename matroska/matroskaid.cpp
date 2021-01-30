#include "./matroskaid.h"
#include "./ebmlid.h"

namespace TagParser {

/*!
 * \brief Encapsulates the most common Matroska element IDs.
 */
namespace MatroskaIds {
}

/*!
 * \brief Encapsulates the most common Matroska track IDs.
 */
namespace MatroskaTrackType {
}

/*!
 * \brief Returns a string for the specified \a matroskaId
 *        if known; otherwise returns an empty string.
 */
std::string_view matroskaIdName(std::uint32_t matroskaId)
{
    using namespace EbmlIds;
    using namespace MatroskaIds;

    switch (matroskaId) {
    // top-level IDs
    case Header:
        return "header";
    case Version:
        return "version";
    case ReadVersion:
        return "read version";
    case MaxIdLength:
        return "max id length";
    case MaxSizeLength:
        return "max size length";
    case DocType:
        return "document type";
    case DocTypeVersion:
        return "document version";
    case DocTypeReadVersion:
        return "document read version";
    case Void:
        return "void";
    case Crc32:
        return "CRC-32";
    case Segment:
        return "segment";
    case SegmentInfo:
        return "segment info";
    case Tracks:
        return "tracks";
    case Cues:
        return "cues";
    case Tags:
        return "tags";
    case SeekHead:
        return "seek head";
    case Cluster:
        return "cluster";
    case Attachments:
        return "attachments";
    case Chapters:
        return "chapters";

        // IDs in the SeekHead master
    case Seek:
        return "seek";

        // IDs in the Seek master
    case SeekID:
        return "seek id";
    case SeekPosition:
        return "seek position";

        // IDs in the SegmentInfo master
    case TimeCodeScale:
        return "time scale code";
    case Duration:
        return "duration";
    case WrittingApp:
        return "writing application";
    case MuxingApp:
        return "muxing application";
    case DateUTC:
        return "date UTC";
    case SegmentUID:
        return "unique segment ID";
    case SegmentFileName:
        return "segment file name";
    case PrevUID:
        return "previous unique id";
    case PrevFileName:
        return "previous file name";
    case NexUID:
        return "next unique ID";
    case NextFileName:
        return "next file name";
    case Title:
        return "title";
    case SegmentFamily:
        return "segment family";
    case ChapterTranslate:
        return "chapter translate";

        // IDs in ChapterTranslate master
    case ChapterTranslateEditionUID:
        return "chapter translate edition UID";
    case ChapterTranslateCodec:
        return "chapter translate codec";
    case ChapterTranslateID:
        return "chapter translate ID";

        // IDs in the Tracks master
    case TrackEntry:
        return "track entry";

        // IDs in the TrackEntry master
    case TrackNumber:
        return "track number";
    case TrackUID:
        return "unique track id";
    case TrackType:
        return "track type";
    case TrackAudio:
        return "audio track";
    case TrackVideo:
        return "video track";
    case ContentEncodings:
        return "content encodings";
    case CodecID:
        return "codec id";
    case CodecPrivate:
        return "codec private";
    case CodecName:
        return "codec name";
    case TrackName:
        return "track name";
    case TrackLanguage:
        return "track language";
    case TrackLanguageIETF:
        return "track language IETF";
    case TrackFlagEnabled:
        return "track enabled";
    case TrackFlagDefault:
        return "default track";
    case TrackFlagForced:
        return "forced track";
    case TrackFlagLacing:
        return "track lacing";
    case MinCache:
        return "track minimum cache";
    case MaxCache:
        return "track maximum cache";
    case DefaultDuration:
        return "track default duration";
    case TrackTimeCodeScale:
        return "track time code scale";
    case MaxBlockAdditionId:
        return "max block addition ID";
    case AttachmentLink:
        return "track attachment link";
    case TrackOverlay:
        return "track overlay";
    case TrackTranslate:
        return "track translate";
    case TrackOffset:
        return "track offset";
    case CodecSettings:
        return "codec settings";
    case CodecInfoUrl:
        return "codec info url";
    case CodecDownloadUrl:
        return "codec download url";
    case CodecDecodeAll:
        return "codec decode all";

        // IDs in the TrackTranslate master
    case TrackTranslateEditionUID:
        return "track translate edition UID";
    case TrackTranslateCodec:
        return "track translate codec";
    case TrackTranslateTrackID:
        return "track translate ID";

        // IDs in the TrackVideo master
    case FrameRate:
        return "video frame rate";
    case DisplayWidth:
        return "video display width";
    case DisplayHeight:
        return "video display height";
    case DisplayUnit:
        return "video display unit";
    case PixelWidth:
        return "video pixel width";
    case PixelHeight:
        return "video pixel height";
    case PixelCropBottom:
        return "video pixel crop bottom";
    case PixelCropTop:
        return "video pixel crop top";
    case PixelCropLeft:
        return "video pixel crop left";
    case PixelCropRight:
        return "video pixel crop right";
    case FlagInterlaced:
        return "video flag interlaced";
    case StereoMode:
        return "video stereo mode";
    case AspectRatioType:
        return "video aspect ratio type";
    case ColorSpace:
        return "video color space";
    case GammaValue:
        return "video gamma value";

        // IDs in the TrackAudio master
    case SamplingFrequency:
        return "audio sampling frequence";
    case BitDepth:
        return "audio bit depth";
    case Channels:
        return "audio channels";
    case ChannelsPositions:
        return "audio channel positions";
    case OutputSamplingFrequency:
        return "audio output sample frequence";

        // IDs in the TrackOperation master
    case TrackCombinePlanes:
        return "track combine planes";
    case TrackJoinBlocks:
        return "track join blocks";

        // IDs in the TrackCombinePlanes master
    case TrackPlane:
        return "track plane";

        // IDs in the TrackPlane master
    case TrackPlaneUID:
        return "track plane UID";
    case TrackPlaneType:
        return "track plane type";

        // IDs in the TrackJoinBlocks master
    case TrackJoinUID:
        return "track join UID";

        // IDs in the ContentEncoding master
    case ContentEncodingOrder:
        return "content encoding order";
    case ContentEncodingScope:
        return "content encoding scope";
    case ContentEncodingType:
        return "content encoding type";
    case ContentCompression:
        return "content encoding compression";
    case ContentEncryption:
        return "content encoding encryption";

        // IDs in the ContentEncryption master
    case ContentEncAlgo:
        return "content encryption algorithmus";
    case ContentEncKeyID:
        return "content encryption key ID";
    case ContentSignature:
        return "content encryption signature";
    case ContentSigKeyID:
        return "content encryption signature key ID";
    case ContentSigAlgo:
        return "content encryption signature algorithmus";
    case ContentSigHashAlgo:
        return "content encryption signature hash algorithmus";

        // IDs in the Tags master
    case Tag:
        return "tag";

        // IDs in the Tag master
    case SimpleTag:
        return "simple tag";
    case Targets:
        return "targets";

        // IDs in the SimpleTag master
    case TagName:
        return "tag name";
    case TagString:
        return "tag string";
    case TagLanguage:
        return "tag language";
    case TagLanguageIETF:
        return "tag language IETF";
    case TagDefault:
        return "tag default";
    case TagBinary:
        return "tag binary";

        // IDs in the Targets master
    case TargetTypeValue:
        return "target type value";
    case TargetType:
        return "target type";
    case TagTrackUID:
        return "tag track UID";
    case TagEditionUID:
        return "tag edition UID";
    case TagChapterUID:
        return "tag chapter UID";
    case TagAttachmentUID:
        return "tag attachment UID";

        // IDs in the Cues master
    case CuePoint:
        return "cue point";

        // IDs in the CuePoint master
    case CueTime:
        return "cue time";
    case CueTrackPositions:
        return "cue track positions";

        // IDs in the CueTrackPositions master
    case CueTrack:
        return "cue track";
    case CueClusterPosition:
        return "cue cluster position";
    case CueRelativePosition:
        return "cue relative position";
    case CueDuration:
        return "cue duration";
    case CueBlockNumber:
        return "cue block number";
    case CueCodecState:
        return "cue codec state";
    case CueReference:
        return "cue reference";

        // IDs in the CueReference master
    case CueRefTime:
        return "cue reference time";
    case CueRefCluster:
        return "cue reference cluster";
    case CueRefNumber:
        return "cue reference number";
    case CueRefCodecState:
        return "cue reference codec state";

        // IDs in the Attachments master
    case AttachedFile:
        return "attached file";

        // IDs in the AttachedFile master
    case FileDescription:
        return "file description";
    case FileName:
        return "file name";
    case FileMimeType:
        return "file mime type";
    case FileData:
        return "file data";
    case FileUID:
        return "file UID";
    case FileReferral:
        return "file referral";
    case FileUsedStartTime:
        return "file used start time";
    case FileUsedEndTime:
        return "file used end time";

        // IDs in the Chapters master
    case EditionEntry:
        return "edition entry";

        // IDs in the EditionEntry master
    case EditionUID:
        return "edition UID";
    case EditionFlagHidden:
        return "edition flag hidden";
    case EditionFlagDefault:
        return "edition flag default";
    case EditionFlagOrdered:
        return "edition flag ordered";
    case ChapterAtom:
        return "chapter atom";

        // IDs in the ChapterAtom master
    case ChapterUID:
        return "chapter UID";
    case ChapterStringUID:
        return "chapter string UID";
    case ChapterTimeStart:
        return "chapter time start";
    case ChapterTimeEnd:
        return "chapter time end";
    case ChapterFlagHidden:
        return "chapter flag hidden";
    case ChapterFlagEnabled:
        return "chapter flag enabled";
    case ChapterSegmentUID:
        return "chapter segment UID";
    case ChapterSegmentEditionUID:
        return "chapter segment edition UID";
    case ChapterPhysicalEquiv:
        return "chapter physical equiv";
    case ChapterTrack:
        return "chapter track";
    case ChapterDisplay:
        return "chapter display";
    case ChapProcess:
        return "chapter process";

        // IDs in the ChaptgerTrack master
    case ChapterTrackNumber:
        return "chapter track number";

        // IDs in the ChapterDisplay master
    case ChapString:
        return "chap string";
    case ChapLanguage:
        return "chap language";
    case ChapLanguageIETF:
        return "chap language IETF";
    case ChapCountry:
        return "chap country";

        // IDs in the ChapProcess master
    case ChapProcessCodecID:
        return "chap process ID";
    case ChapProcessPrivate:
        return "chap process private";
    case ChapProcessCommand:
        return "chap process command";

        // IDs in the ChapProcessCommand master
    case ChapProcessTime:
        return "chap process time";
    case ChapProcessData:
        return "chap process data";

        // IDs in the Cluster master
    case Timecode:
        return "timecode";
    case SilentTracks:
        return "silent tracks";
    case Position:
        return "position";
    case PrevSize:
        return "previous size";
    case SimpleBlock:
        return "simple block";
    case BlockGroup:
        return "block group";
    case EncryptedBlock:
        return "encrypted block";

        // IDs in the SilentTracks master
    case SilentTrackNumber:
        return "silent track number";

        // IDs in the BlockGroup master
    case Block:
        return "block";
    case BlockVirtual:
        return "block virtual";
    case BlockAdditions:
        return "block additions";
    case BlockDuration:
        return "block duration";
    case ReferencePriority:
        return "reference priority";
    case ReferenceBlock:
        return "reference block";
    case ReferenceVirtual:
        return "reference virtual";
    case CodecState:
        return "codec state";
    case DiscardPadding:
        return "discard padding";
    case Slices:
        return "slices";
    case ReferenceFrame:
        return "reference frame";

        // IDs in the BlockAdditions master
    case BlockMore:
        return "block more";

        // IDs in the BlockMore master
    case BlockAddID:
        return "block add ID";
    case BlockAdditional:
        return "block additional";

        // IDs in the Slices master
    case TimeSlice:
        return "time slice";

        // IDs in the TimeSlice master
    case LaceNumber:
        return "lace number";
    case FrameNumber:
        return "frame number";
    case BlockAdditionID:
        return "block addition ID";
    case Delay:
        return "delay";
    case SliceDuration:
        return "slice duration";

        // IDs in the ReferenceFrame master
    case ReferenceOffset:
        return "reference offset";
    case ReferenceTimeCode:
        return "reference time code";

    default:
        return std::string_view();
    }
}

/*!
 * \brief Returns the level at which elements with the specified \a matroskaId are supposed
 *        to occur in a Matroska file.
 */
MatroskaElementLevel matroskaIdLevel(std::uint32_t matroskaId)
{
    using namespace EbmlIds;
    using namespace MatroskaIds;
    switch (matroskaId) {
    case Header:
    case Segment:
        return MatroskaElementLevel::TopLevel;
    case SeekHead:
    case SegmentInfo:
    case Cluster:
    case Tracks:
    case Cues:
    case Attachments:
    case Chapters:
    case Tags:
        return MatroskaElementLevel::Level1;
    case Seek:
    case SegmentUID:
    case SegmentFileName:
    case PrevUID:
    case PrevFileName:
    case NexUID:
    case NextFileName:
    case SegmentFamily:
    case ChapterTranslate:
    case TimeCodeScale:
    case Duration:
    case DateUTC:
    case Title:
    case MuxingApp:
    case WrittingApp:
    case Timecode:
    case SilentTracks:
    case Position:
    case PrevSize:
    case SimpleBlock:
    case BlockGroup:
    case EncryptedBlock:
    case TrackEntry:
    case CuePoint:
    case AttachedFile:
    case EditionEntry:
    case Tag:
        return MatroskaElementLevel::Level2;
    case SeekID:
    case SeekPosition:
    case ChapterTranslateEditionUID:
    case ChapterTranslateCodec:
    case ChapterTranslateID:
    case SilentTrackNumber:
    case BlockVirtual:
    case BlockAdditions:
    case BlockDuration:
    case ReferencePriority:
    case ReferenceBlock:
    case ReferenceVirtual:
    case CodecState:
    case DiscardPadding:
    case Slices:
    case TrackNumber:
    case TrackUID:
    case TrackType:
    case TrackFlagEnabled:
    case TrackFlagDefault:
    case TrackFlagForced:
    case TrackFlagLacing:
    case MinCache:
    case MaxCache:
    case DefaultDuration:
    case DefaultDecodedFieldDuration:
    case TrackTimeCodeScale:
    case TrackOffset:
    case MaxBlockAdditionId:
    case TrackName:
    case TrackLanguage:
    case TrackLanguageIETF:
    case CodecID:
    case CodecPrivate:
    case CodecName:
    case AttachmentLink:
    case CodecSettings:
    case CodecInfoUrl:
    case CodecDownloadUrl:
    case CodecDecodeAll:
    case TrackOverlay:
    case CodecDelay:
    case SeekPreRoll:
    case TrackTranslate:
    case TrackVideo:
    case TrackAudio:
    case ContentEncodings:
    case CueTime:
    case CueTrackPositions:
    case FileDescription:
    case FileName:
    case FileMimeType:
    case FileData:
    case FileUID:
    case FileReferral:
    case FileUsedStartTime:
    case FileUsedEndTime:
    case EditionUID:
    case EditionFlagHidden:
    case EditionFlagDefault:
    case EditionFlagOrdered:
    case Targets:
        return MatroskaElementLevel::Level3;
    case BlockMore:
    case TimeSlice:
    case ContentEncoding:
    case CueTrack:
    case CueClusterPosition:
    case CueRelativePosition:
    case CueDuration:
    case CueBlockNumber:
    case CueCodecState:
    case CueReference:
    case TargetTypeValue:
    case TargetType:
    case TagTrackUID:
    case TagEditionUID:
    case TagChapterUID:
    case TagAttachmentUID:
        return MatroskaElementLevel::Level4;
    case BlockAddID:
    case BlockAdditional:
    case LaceNumber:
    case FrameNumber:
    case BlockAdditionID:
    case Delay:
    case SliceDuration:
    case ReferenceFrame:
    case ReferenceOffset:
    case ReferenceTimeCode:
    case CueRefTime:
    case CueRefCluster:
    case CueRefNumber:
    case CueRefCodecState:
        return MatroskaElementLevel::Level5;
    case Void:
    case Crc32:
        return MatroskaElementLevel::Global;
    default:
        return MatroskaElementLevel::Unknown;
    }
}

} // namespace TagParser
