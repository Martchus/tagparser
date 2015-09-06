#include "id3v2tag.h"
#include "id3v2frameids.h"

#include "tagparser/exceptions.h"

#include <c++utilities/conversion/stringconversion.h>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::Id3v2Tag
 * \brief Implementation of Media::Tag for ID3v2 tags.
 */

uint32 Id3v2Tag::fieldId(KnownField field) const
{
    using namespace Id3v2FrameIds;
    if(m_majorVersion >= 3) {
        switch(field) {
        case KnownField::Album: return lAlbum;
        case KnownField::Artist: return lArtist;
        case KnownField::Comment: return lComment;
        case KnownField::Year: return lYear;
        case KnownField::RecordDate: return lRecordDate;
        case KnownField::Title: return lTitle;
        case KnownField::Genre: return lGenre;
        case KnownField::TrackPosition: return lTrackPosition;
        case KnownField::DiskPosition: return lDiskPosition;
        case KnownField::Encoder: return lEncoder;
        case KnownField::Bpm: return lBpm;
        case KnownField::Cover: return lCover;
        case KnownField::Lyricist: return lWriter;
        case KnownField::Length: return lLength;
        case KnownField::Language: return lLanguage;
        case KnownField::EncoderSettings: return lEncoderSettings;
        case KnownField::Lyrics: return lUnsynchronizedLyrics;
        case KnownField::SynchronizedLyrics: return lSynchronizedLyrics;
        case KnownField::Grouping: return lGrouping;
        case KnownField::RecordLabel: return lRecordLabel;
        case KnownField::Composer: return lComposer;
        case KnownField::Rating: return lRating;
        default:
            ;
        }
    } else {
        switch(field) {
        case KnownField::Album: return sAlbum;
        case KnownField::Artist: return sArtist;
        case KnownField::Comment: return sComment;
        case KnownField::Year: return sYear;
        case KnownField::RecordDate: return sRecordDate;
        case KnownField::Title: return sTitle;
        case KnownField::Genre: return sGenre;
        case KnownField::TrackPosition: return sTrackPosition;
        case KnownField::DiskPosition: return lDiskPosition;
        case KnownField::Encoder: return sEncoder;
        case KnownField::Bpm: return sBpm;
        case KnownField::Cover: return sCover;
        case KnownField::Lyricist: return sWriter;
        case KnownField::Length: return sLength;
        case KnownField::Language: return sLanguage;
        case KnownField::EncoderSettings: return sEncoderSettings;
        case KnownField::Lyrics: return sUnsynchronizedLyrics;
        case KnownField::SynchronizedLyrics: return sSynchronizedLyrics;
        case KnownField::Grouping: return sGrouping;
        case KnownField::RecordLabel: return sRecordLabel;
        case KnownField::Composer: return sComposer;
        case KnownField::Rating: return sRating;
        default:
            ;
        }
    }
    return 0;
}

KnownField Id3v2Tag::knownField(const uint32 &id) const
{
    using namespace Id3v2FrameIds;
    switch(id) {
    case lAlbum: return KnownField::Album;
    case lArtist: return KnownField::Artist;
    case lComment: return KnownField::Comment;
    case lYear: return KnownField::Year;
    case lRecordDate: return KnownField::RecordDate;
    case lTitle: return KnownField::Title;
    case lGenre: return KnownField::Genre;
    case lTrackPosition: return KnownField::TrackPosition;
    case lDiskPosition: return KnownField::DiskPosition;
    case lEncoder: return KnownField::Encoder;
    case lBpm: return KnownField::Bpm;
    case lCover: return KnownField::Cover;
    case lWriter: return KnownField::Lyricist;
    case lLanguage: return KnownField::Language;
    case lLength: return KnownField::Length;
    case lEncoderSettings: return KnownField::EncoderSettings;
    case lUnsynchronizedLyrics: return KnownField::Lyrics;
    case lSynchronizedLyrics: return KnownField::SynchronizedLyrics;
    case lGrouping: return KnownField::Grouping;
    case lRecordLabel: return KnownField::RecordLabel;
    case sAlbum: return KnownField::Album;
    case sArtist: return KnownField::Artist;
    case sComment: return KnownField::Comment;
    case sYear: return KnownField::Year;
    case sRecordDate: return KnownField::RecordDate;
    case sTitle: return KnownField::Title;
    case sGenre: return KnownField::Genre;
    case sTrackPosition: return KnownField::TrackPosition;
    case sEncoder: return KnownField::Encoder;
    case sBpm: return KnownField::Bpm;
    case sCover: return KnownField::Cover;
    case sWriter: return KnownField::Lyricist;
    case sLanguage: return KnownField::Language;
    case sLength: return KnownField::Length;
    case sEncoderSettings: return KnownField::EncoderSettings;
    case sUnsynchronizedLyrics: return KnownField::Lyrics;
    case sSynchronizedLyrics: return KnownField::SynchronizedLyrics;
    case sGrouping: return KnownField::Grouping;
    case sRecordLabel: return KnownField::RecordLabel;
    default: return KnownField::Invalid;
    }
}

TagDataType Id3v2Tag::proposedDataType(const uint32 &id) const
{
    using namespace Id3v2FrameIds;
    switch(id) {
    case lLength: case sLength:
        return TagDataType::TimeSpan;
    case lBpm: case sBpm:
        return TagDataType::Integer;
    case lTrackPosition: case sTrackPosition:
    case lDiskPosition:
        return TagDataType::PositionInSet;
    case lCover: case sCover:
        return TagDataType::Picture;
    default:
        if(Id3v2FrameIds::isTextfield(id)) {
            return TagDataType::Text;
        } else {
            return TagDataType::Undefined;
        }
    }
}

const TagValue &Id3v2Tag::value(const typename Id3v2Frame::identifierType &id) const
{
    const TagValue &res = FieldMapBasedTag<Id3v2Frame, FrameComparer>::value(id);
    if(res.isEmpty()) {
        typename Id3v2Frame::identifierType alternativeId = Id3v2FrameIds::isLongId(id)
                ? Id3v2FrameIds::convertToShortId(id)
                : Id3v2FrameIds::convertToLongId(id);
        if(alternativeId) {
            return FieldMapBasedTag<Id3v2Frame, FrameComparer>::value(alternativeId);
        } else {
            return res;
        }
    }
    return res;
}

bool Id3v2Tag::setValue(const typename Id3v2Frame::identifierType &id, const TagValue &value)
{
    typename Id3v2Frame::identifierType alternativeId = Id3v2FrameIds::isLongId(id)
            ? Id3v2FrameIds::convertToShortId(id)
            : Id3v2FrameIds::convertToLongId(id);
    if(!alternativeId || fields().count(id) || !fields().count(alternativeId)) {
        return FieldMapBasedTag<Id3v2Frame, FrameComparer>::setValue(id, value);
    } else {
        return FieldMapBasedTag<Id3v2Frame, FrameComparer>::setValue(alternativeId, value);
    }
}

/*!
 * \brief Parses tag information from the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Id3v2Tag::parse(istream &stream)
{
    // prepare parsing
    invalidateStatus();
    const string context("parsing ID3v2 tag");
    BinaryReader reader(&stream);
    uint64 startOffset = stream.tellg();
    // read signature: ID3
    if(reader.readUInt24BE() == 0x494433u) {
        // read header data
        byte majorVersion = reader.readByte();
        byte revisionVersion = reader.readByte();
        setVersion(majorVersion, revisionVersion);
        m_flags = reader.readByte();
        m_sizeExcludingHeader = reader.readSynchsafeUInt32BE();
        m_size = m_sizeExcludingHeader + 10;
        if(m_sizeExcludingHeader == 0) {
            addNotification(NotificationType::Warning, "ID3v2 tag seems to be empty.", context);
        } else {
            // check if the version
            if(!isVersionSupported()) {
                addNotification(NotificationType::Critical, "The ID3v2 tag couldn't be parsed, because its version is not supported.", context);
                throw VersionNotSupportedException();
            }
            // read extended header (if present)
            if(hasExtendedHeader()) {
                m_extendedHeaderSize = reader.readSynchsafeUInt32BE();
                if(m_extendedHeaderSize < 6 || m_extendedHeaderSize > m_sizeExcludingHeader) {
                    addNotification(NotificationType::Critical, "Extended header is invalid.", context);
                    throw InvalidDataException();
                }
                stream.seekg(m_extendedHeaderSize - 4, ios_base::cur);
            }
            // read frames
            istream::pos_type pos = stream.tellg();
            uint32 frameSize;
            int32 bytesRemaining = m_sizeExcludingHeader - m_extendedHeaderSize;
            Id3v2Frame frame;
            const uint32 &frameId = frame.id();
            do {
                // seek to next frame
                stream.seekg(pos);
                // parse frame
                try {
                    frame.parse(reader, majorVersion, bytesRemaining);
                    if(frameId) {
                        // add frame if parsing was sucessfull
                        if(Id3v2FrameIds::isTextfield(frameId) && fields().count(frame.id()) == 1) {
                            addNotification(NotificationType::Warning, "The text frame " + frame.frameIdString() + " exists more than once.", context);
                        }
                        fields().insert(pair<fieldType::identifierType, fieldType>(frameId, frame));
                    }
                } catch(NoDataFoundException &) {
                    if(frame.hasPaddingReached()) {
                        m_paddingSize = (startOffset + m_size) - pos;
                        break;
                    }
                } catch(Failure &) {
                    // nothing to do here since notifications will be added anyways
                }
                // add parsing notifications of frame
                addNotifications(context, frame);
                frame.invalidateNotifications();
                // calculate next frame offset
                frameSize = frame.frameSize();
                bytesRemaining -= frameSize;
                pos += frameSize;
            } while (bytesRemaining > 0);
        }
    } else {
        addNotification(NotificationType::Critical, "Signature is invalid.", context);
        throw InvalidDataException();
    }
}

/*!
 * \brief Writes tag information to the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void Id3v2Tag::make(ostream &stream)
{
    // prepare making
    invalidateStatus();
    const string context("making ID3v2 tag");
    // check if version is supported
    // (the version could have been changed using setVersion(...)
    if(!isVersionSupported()) {
        addNotification(NotificationType::Critical, "The ID3v2 tag couldn't be created, because the target version isn't supported.", context);
        throw VersionNotSupportedException();
    }
    // prepare for writing
    BinaryWriter writer(&stream);
    // write header
    writer.writeUInt24BE(0x494433u); // signature
    writer.writeByte(m_majorVersion); // major version
    writer.writeByte(m_revisionVersion); // revision version
    writer.writeByte(m_flags & 0xBF); // flags, but without extended header or compression bit set
    stream.seekp(4, ios_base::cur); // size currently unknown, write it later
    streamoff framesOffset = stream.tellp();
    int framesWritten = 0;
    for(auto i : fields()) {
        Id3v2Frame &frame = i.second;
        // write only valid frames
        if(frame.isValid()) {
            // make the frame
            try {
                frame.make(writer, m_majorVersion);
                ++framesWritten;
            } catch(Failure &) {
                // nothing to do here since notifications will be added anyways
            }
            // add making notifications
            addNotifications(context, frame);
        }
    }
    // calculate and write size
    streamoff endOffset = stream.tellp();
    stream.seekp(framesOffset - 4, ios_base::beg);
    writer.writeSynchsafeUInt32BE(endOffset - framesOffset);
    stream.seekp(endOffset, ios_base::beg);
    if(framesWritten <= 0) { // add a warning notification if an empty ID3v2 tag has been written
        addNotification(NotificationType::Warning, "No frames could be written, an empty ID3v2 tag has been written.", context);
    }
}

/*!
 * \brief Sets the version to the specified \a majorVersion and
 *        the specified \a revisionVersion.
 */
void Id3v2Tag::setVersion(byte majorVersion, byte revisionVersion)
{
    m_majorVersion = majorVersion;
    m_revisionVersion = revisionVersion;
    stringstream versionStream(stringstream::in | stringstream::out);
    versionStream << "2." << static_cast<int>(majorVersion) << "." << static_cast<int>(revisionVersion);
    m_version = versionStream.str();
}

/*!
 * \class Media::FrameComparer
 * \brief Defines the order which is used to store ID3v2 frames.
 *
 * The order is: unique file id, title, other text frames, other frames, cover
 */

/*!
 * \brief Returns true if \a lhs goes before \a rhs; otherwise returns false.
 */
bool FrameComparer::operator()(const uint32 &lhs, const uint32 &rhs) const
{
    if(lhs == rhs) {
        return false;
    }
    if(lhs == Id3v2FrameIds::lUniqueFileId || lhs == Id3v2FrameIds::sUniqueFileId) {
        return true;
    }
    if(rhs == Id3v2FrameIds::lUniqueFileId || rhs == Id3v2FrameIds::sUniqueFileId) {
        return false;
    }
    if(lhs == Id3v2FrameIds::lTitle || lhs == Id3v2FrameIds::sTitle) {
        return true;
    }
    if(rhs == Id3v2FrameIds::lTitle || rhs == Id3v2FrameIds::sTitle) {
        return false;
    }
    bool lhstextfield = Id3v2FrameIds::isTextfield(lhs);
    bool rhstextfield = Id3v2FrameIds::isTextfield(rhs);
    if(lhstextfield && !rhstextfield) {
        return true;
    }
    if(!lhstextfield && rhstextfield) {
        return false;
    }
    if(lhs == Id3v2FrameIds::lCover || lhs == Id3v2FrameIds::sCover) {
        return false;
    }
    if(rhs == Id3v2FrameIds::lCover || rhs == Id3v2FrameIds::sCover) {
        return true;
    }
    return lhs < rhs;
}

}
