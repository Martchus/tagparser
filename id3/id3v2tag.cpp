#include "./id3v2tag.h"
#include "./id3v2frameids.h"

#include "../exceptions.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/conversion/stringbuilder.h>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::Id3v2Tag
 * \brief Implementation of Media::Tag for ID3v2 tags.
 */

Id3v2Tag::IdentifierType Id3v2Tag::internallyGetFieldId(KnownField field) const
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

KnownField Id3v2Tag::internallyGetKnownField(const IdentifierType &id) const
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

TagDataType Id3v2Tag::internallyGetProposedDataType(const uint32 &id) const
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
        if(Id3v2FrameIds::isTextFrame(id)) {
            return TagDataType::Text;
        } else {
            return TagDataType::Undefined;
        }
    }
}

/*!
 * \brief Parses tag information from the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Id3v2Tag::parse(istream &stream, const uint64 maximalSize)
{
    // prepare parsing
    invalidateStatus();
    static const string context("parsing ID3v2 tag");
    BinaryReader reader(&stream);
    uint64 startOffset = stream.tellg();

    // check whether the header is truncated
    if(maximalSize && maximalSize < 10) {
        addNotification(NotificationType::Critical, "ID3v2 header is truncated (at least 10 bytes expected).", context);
        throw TruncatedDataException();
    }

    // read signature: ID3
    if(reader.readUInt24BE() == 0x494433u) {
        // read header data
        byte majorVersion = reader.readByte();
        byte revisionVersion = reader.readByte();
        setVersion(majorVersion, revisionVersion);
        m_flags = reader.readByte();
        m_sizeExcludingHeader = reader.readSynchsafeUInt32BE();
        m_size = 10 + m_sizeExcludingHeader;
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
                if(maximalSize && maximalSize < 14) {
                    addNotification(NotificationType::Critical, "Extended header denoted but not present.", context);
                    throw TruncatedDataException();
                }
                m_extendedHeaderSize = reader.readSynchsafeUInt32BE();
                if(m_extendedHeaderSize < 6 || m_extendedHeaderSize > m_sizeExcludingHeader || (maximalSize && maximalSize < (10 + m_extendedHeaderSize))) {
                    addNotification(NotificationType::Critical, "Extended header is invalid/truncated.", context);
                    throw TruncatedDataException();
                }
                stream.seekg(m_extendedHeaderSize - 4, ios_base::cur);
            }

            // how many bytes remain for frames and padding?
            uint32 bytesRemaining = m_sizeExcludingHeader - m_extendedHeaderSize;
            if(maximalSize && bytesRemaining > maximalSize) {
                bytesRemaining = maximalSize;
                addNotification(NotificationType::Critical, "Frames are truncated.", context);
            }

            // read frames
            auto pos = stream.tellg();
            Id3v2Frame frame;
            while(bytesRemaining) {
                // seek to next frame
                stream.seekg(pos);
                // parse frame
                try {
                    frame.parse(reader, majorVersion, bytesRemaining);
                    if(frame.id()) {
                        // add frame if parsing was successfull
                        if(Id3v2FrameIds::isTextFrame(frame.id()) && fields().count(frame.id()) == 1) {
                            addNotification(NotificationType::Warning, "The text frame " % frame.frameIdString() + " exists more than once.", context);
                        }
                        fields().insert(make_pair(frame.id(), frame));
                    }
                } catch(const NoDataFoundException &) {
                    if(frame.hasPaddingReached()) {
                        m_paddingSize = startOffset + m_size - pos;
                        break;
                    }
                } catch(const Failure &) {
                    // nothing to do here since notifications will be added anyways
                }

                // add parsing notifications of frame
                addNotifications(context, frame);
                frame.invalidateNotifications();

                // calculate next frame offset
                if(frame.totalSize() <= bytesRemaining) {
                    pos += frame.totalSize();
                    bytesRemaining -= frame.totalSize();
                } else {
                    pos += bytesRemaining;
                    bytesRemaining = 0;
                }
            }

            // check for extended header
            if(hasFooter()) {
                if(maximalSize && m_size + 10 < maximalSize) {
                    // the footer does not provide additional information, just check the signature
                    stream.seekg(startOffset + (m_size += 10));
                    if(reader.readUInt24LE() != 0x494433u) {
                        addNotification(NotificationType::Critical, "Footer signature is invalid.", context);
                    }
                    // skip remaining footer
                    stream.seekg(7, ios_base::cur);
                } else {
                    addNotification(NotificationType::Critical, "Footer denoted but not present.", context);
                    throw TruncatedDataException();
                }
            }
        }
    } else {
        addNotification(NotificationType::Critical, "Signature is invalid.", context);
        throw InvalidDataException();
    }
}

/*!
 * \brief Prepares making.
 * \returns Returns a Id3v2TagMaker object which can be used to actually make the tag.
 * \remarks The tag must NOT be mutated after making is prepared when it is intended to actually
 *          make the tag using the make method of the returned object.
 * \throws Throws Media::Failure or a derived exception when a making error occurs.
 *
 * This method might be useful when it is necessary to know the size of the tag before making it.
 * \sa make()
 */
Id3v2TagMaker Id3v2Tag::prepareMaking()
{
    return Id3v2TagMaker(*this);
}

/*!
 * \brief Writes tag information to the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void Id3v2Tag::make(ostream &stream, uint32 padding)
{
    prepareMaking().make(stream, padding);
}

/*!
 * \brief Sets the version to the specified \a majorVersion and
 *        the specified \a revisionVersion.
 */
void Id3v2Tag::setVersion(byte majorVersion, byte revisionVersion)
{
    m_majorVersion = majorVersion;
    m_revisionVersion = revisionVersion;
    m_version = argsToString('2', '.', majorVersion, '.', revisionVersion);
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
    const bool lhsLong = Id3v2FrameIds::isLongId(lhs);
    const bool rhsLong = Id3v2FrameIds::isLongId(rhs);
    if(((lhsLong && !rhsLong) && (lhs == Id3v2FrameIds::convertToLongId(rhs)))
            || ((!lhsLong && rhsLong) && (Id3v2FrameIds::convertToLongId(lhs) == rhs))) {
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
    bool lhstextfield = Id3v2FrameIds::isTextFrame(lhs);
    bool rhstextfield = Id3v2FrameIds::isTextFrame(rhs);
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

/*!
 * \class Media::Id3v2TagMaker
 * \brief The Id3v2TagMaker class helps writing ID3v2 tags.
 *
 * An instance can be obtained using the Id3v2Tag::prepareMaking() method.
 */

/*!
 * \brief Prepares making the specified \a tag.
 * \sa See Id3v2Tag::prepareMaking() for more information.
 */
Id3v2TagMaker::Id3v2TagMaker(Id3v2Tag &tag) :
    m_tag(tag),
    m_framesSize(0)
{
    tag.invalidateStatus();
    static const string context("making ID3v2 tag");

    // check if version is supported
    // (the version could have been changed using setVersion())
    if(!tag.isVersionSupported()) {
        tag.addNotification(NotificationType::Critical, "The ID3v2 tag version isn't supported.", context);
        throw VersionNotSupportedException();
    }

    // prepare frames
    m_maker.reserve(tag.fields().size());
    for(auto &pair : tag.fields()) {
        try {
            m_maker.emplace_back(pair.second.prepareMaking(tag.majorVersion()));
            m_framesSize += m_maker.back().requiredSize();
        } catch(const Failure &) {
            // nothing to do here; notifications will be added anyways
        }
        m_tag.addNotifications(pair.second);
    }

    // calculate required size
    // -> header + size of frames
    m_requiredSize = 10 + m_framesSize;
}

/*!
 * \brief Saves the tag (specified when constructing the object) to the
 *        specified \a stream.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw Media::Failure or a derived exception.
 */
void Id3v2TagMaker::make(std::ostream &stream, uint32 padding)
{
    BinaryWriter writer(&stream);

    // write header
    // -> signature
    writer.writeUInt24BE(0x494433u);
    // -> version
    writer.writeByte(m_tag.majorVersion());
    writer.writeByte(m_tag.revisionVersion());
    // -> flags, but without extended header or compression bit set
    writer.writeByte(m_tag.flags() & 0xBF);
    // -> size (excluding header)
    writer.writeSynchsafeUInt32BE(m_framesSize + padding);

    // write frames
    for(auto &maker : m_maker) {
        maker.make(writer);
    }

    // write padding
    for(; padding; --padding) {
        stream.put(0);
    }
}

}
