#include "./id3v2tag.h"
#include "./id3v2frameids.h"

#include "../diagnostics.h"
#include "../exceptions.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>

#include <iostream>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::Id3v2Tag
 * \brief Implementation of TagParser::Tag for ID3v2 tags.
 */

/*!
 * \brief Allows multiple values for some fields.
 * \remarks The standard defines no general rule applicable to all fields.
 */
bool Id3v2Tag::supportsMultipleValues(KnownField field) const
{
    switch (field) {
    case KnownField::Album:
    case KnownField::Artist:
    case KnownField::RecordDate:
    case KnownField::ReleaseDate:
    case KnownField::Title:
    case KnownField::Genre:
    case KnownField::TrackPosition:
    case KnownField::DiskPosition:
    case KnownField::Encoder:
    case KnownField::Bpm:
    case KnownField::Lyricist:
    case KnownField::Length:
    case KnownField::Language:
    case KnownField::EncoderSettings:
    case KnownField::Grouping:
    case KnownField::RecordLabel:
    case KnownField::Composer:
    case KnownField::AlbumArtist:
        return m_majorVersion > 3;
    case KnownField::Rating:
    case KnownField::Comment:
    case KnownField::Cover:
    case KnownField::Lyrics:
    case KnownField::SynchronizedLyrics:
        return true;
    default:
        return false;
    }
}

void Id3v2Tag::ensureTextValuesAreProperlyEncoded()
{
    const auto encoding = proposedTextEncoding();
    for (auto &field : fields()) {
        auto &value = field.second.value();
        value.convertDataEncoding(encoding);
        value.convertDescriptionEncoding(encoding);
    }
}

/*!
 * \brief Adds additional values as well.
 */
void Id3v2Tag::internallyGetValuesFromField(const Id3v2Tag::FieldType &field, std::vector<const TagValue *> &values) const
{
    if (!field.value().isEmpty()) {
        values.emplace_back(&field.value());
    }
    for (const auto &value : field.additionalValues()) {
        if (!value.isEmpty()) {
            values.emplace_back(&value);
        }
    }
}

/*!
 * \brief Uses default implementation for non-text frames and applies special handling to text frames.
 *
 * - Ensure text frames are unique.
 * - Allow to store multiple values inside the same text frame.
 */
bool Id3v2Tag::internallySetValues(const IdentifierType &id, const std::vector<TagValue> &values)
{
    // use default implementation for non-text frames
    if (!Id3v2FrameIds::isTextFrame(id)) {
        return CRTPBase::internallySetValues(id, values);
    }

    // find existing text frame
    auto range = fields().equal_range(id);
    auto frameIterator = range.first;

    // use existing frame or insert new text frame
    auto valuesIterator = values.cbegin();
    if (frameIterator != range.second) {
        ++range.first;
        // add primary value to existing frame
        if (valuesIterator != values.cend()) {
            frameIterator->second.setValue(*valuesIterator);
            ++valuesIterator;
        } else {
            frameIterator->second.value().clearDataAndMetadata();
        }
    } else {
        // skip if there is no existing frame but also no values to be assigned
        if (valuesIterator == values.cend()) {
            return true;
        }
        // add primary value to new frame
        frameIterator = fields().insert(make_pair(id, Id3v2Frame(id, *valuesIterator)));
        ++valuesIterator;
    }

    // add additional values to frame
    frameIterator->second.additionalValues() = vector<TagValue>(valuesIterator, values.cend());

    // remove remaining existing values (there are more existing values than specified ones)
    for (; range.first != range.second; ++range.first) {
        range.first->second.setValue(TagValue());
    }
    return true;
}

Id3v2Tag::IdentifierType Id3v2Tag::internallyGetFieldId(KnownField field) const
{
    using namespace Id3v2FrameIds;
    if (m_majorVersion >= 3) {
        switch (field) {
        case KnownField::Album:
            return lAlbum;
        case KnownField::Artist:
            return lArtist;
        case KnownField::Comment:
            return lComment;
        case KnownField::RecordDate:
            return lRecordingTime; // (de)serializer converts to/from lYear/lRecordingDates/lDate/lTime
        case KnownField::ReleaseDate:
            return lReleaseTime;
        case KnownField::Title:
            return lTitle;
        case KnownField::Genre:
            return lGenre;
        case KnownField::TrackPosition:
            return lTrackPosition;
        case KnownField::DiskPosition:
            return lDiskPosition;
        case KnownField::Encoder:
            return lEncoder;
        case KnownField::Bpm:
            return lBpm;
        case KnownField::Cover:
            return lCover;
        case KnownField::Lyricist:
            return lWriter;
        case KnownField::Length:
            return lLength;
        case KnownField::Language:
            return lLanguage;
        case KnownField::EncoderSettings:
            return lEncoderSettings;
        case KnownField::Lyrics:
            return lUnsynchronizedLyrics;
        case KnownField::SynchronizedLyrics:
            return lSynchronizedLyrics;
        case KnownField::Grouping:
            return lContentGroupDescription;
        case KnownField::Publisher:
            return lRecordLabel;
        case KnownField::Composer:
            return lComposer;
        case KnownField::PlayCounter:
            return lPlayCounter;
        case KnownField::Rating:
            return lRating;
        case KnownField::AlbumArtist:
            return lAlbumArtist;
        case KnownField::RemixedBy:
            return lRemixedBy;
        case KnownField::Copyright:
            return lCopyright;
        case KnownField::TaggingDate:
            return lTaggingTime;
        case KnownField::EncodingDate:
            return lEncodingTime;
        case KnownField::OriginalReleaseDate:
            return lOriginalReleaseTime;
        case KnownField::Mood:
            return lMood;
        default:;
        }
    } else {
        switch (field) {
        case KnownField::Album:
            return sAlbum;
        case KnownField::Artist:
            return sArtist;
        case KnownField::Comment:
            return sComment;
        case KnownField::RecordDate:
            return lRecordingTime; // (de)serializer converts to/from sYear/sRecordingDates/sDate/sTime
        case KnownField::Title:
            return sTitle;
        case KnownField::Genre:
            return sGenre;
        case KnownField::TrackPosition:
            return sTrackPosition;
        case KnownField::DiskPosition:
            return sDiskPosition;
        case KnownField::Encoder:
            return sEncoder;
        case KnownField::Bpm:
            return sBpm;
        case KnownField::Cover:
            return sCover;
        case KnownField::Lyricist:
            return sWriter;
        case KnownField::Length:
            return sLength;
        case KnownField::Language:
            return sLanguage;
        case KnownField::EncoderSettings:
            return sEncoderSettings;
        case KnownField::Lyrics:
            return sUnsynchronizedLyrics;
        case KnownField::SynchronizedLyrics:
            return sSynchronizedLyrics;
        case KnownField::Grouping:
            return sContentGroupDescription;
        case KnownField::Publisher:
            return sRecordLabel;
        case KnownField::Composer:
            return sComposer;
        case KnownField::PlayCounter:
            return sPlayCounter;
        case KnownField::Rating:
            return sRating;
        case KnownField::AlbumArtist:
            return sAlbumArtist;
        case KnownField::RemixedBy:
            return sRemixedBy;
        case KnownField::Copyright:
            return sCopyright;
        default:;
        }
    }
    return 0;
}

KnownField Id3v2Tag::internallyGetKnownField(const IdentifierType &id) const
{
    using namespace Id3v2FrameIds;
    switch (id) {
    case lAlbum:
        return KnownField::Album;
    case lArtist:
        return KnownField::Artist;
    case lComment:
        return KnownField::Comment;
    case lRecordingTime:
    case lYear:
        return KnownField::RecordDate;
    case lTitle:
        return KnownField::Title;
    case lGenre:
        return KnownField::Genre;
    case lTrackPosition:
        return KnownField::TrackPosition;
    case lDiskPosition:
        return KnownField::DiskPosition;
    case lEncoder:
        return KnownField::Encoder;
    case lBpm:
        return KnownField::Bpm;
    case lCover:
        return KnownField::Cover;
    case lWriter:
        return KnownField::Lyricist;
    case lLanguage:
        return KnownField::Language;
    case lLength:
        return KnownField::Length;
    case lEncoderSettings:
        return KnownField::EncoderSettings;
    case lUnsynchronizedLyrics:
        return KnownField::Lyrics;
    case lSynchronizedLyrics:
        return KnownField::SynchronizedLyrics;
    case lAlbumArtist:
        return KnownField::AlbumArtist;
    case lRemixedBy:
        return KnownField::RemixedBy;
    case lCopyright:
        return KnownField::Copyright;
    case lContentGroupDescription:
        return KnownField::Grouping;
    case lRecordLabel:
        return KnownField::Publisher;
    case lTaggingTime:
        return KnownField::TaggingDate;
    case lEncodingTime:
        return KnownField::EncodingDate;
    case lOriginalReleaseTime:
        return KnownField::OriginalReleaseDate;
    case lMood:
        return KnownField::Mood;
    case lPlayCounter:
        return KnownField::PlayCounter;
    case lRating:
        return KnownField::Rating;
    case lISRC:
        return KnownField::ISRC;
    case sAlbum:
        return KnownField::Album;
    case sArtist:
        return KnownField::Artist;
    case sComment:
        return KnownField::Comment;
    case sYear:
        return KnownField::RecordDate;
    case sTitle:
        return KnownField::Title;
    case sGenre:
        return KnownField::Genre;
    case sTrackPosition:
        return KnownField::TrackPosition;
    case sEncoder:
        return KnownField::Encoder;
    case sBpm:
        return KnownField::Bpm;
    case sCover:
        return KnownField::Cover;
    case sWriter:
        return KnownField::Lyricist;
    case sLanguage:
        return KnownField::Language;
    case sLength:
        return KnownField::Length;
    case sEncoderSettings:
        return KnownField::EncoderSettings;
    case sUnsynchronizedLyrics:
        return KnownField::Lyrics;
    case sSynchronizedLyrics:
        return KnownField::SynchronizedLyrics;
    case sAlbumArtist:
        return KnownField::Grouping;
    case sRecordLabel:
        return KnownField::Publisher;
    case sRemixedBy:
        return KnownField::RemixedBy;
    case sCopyright:
        return KnownField::Copyright;
    case sPlayCounter:
        return KnownField::PlayCounter;
    case sRating:
        return KnownField::Rating;
    case sISRC:
        return KnownField::ISRC;
    default:
        return KnownField::Invalid;
    }
}

TagDataType Id3v2Tag::internallyGetProposedDataType(const std::uint32_t &id) const
{
    using namespace Id3v2FrameIds;
    switch (id) {
    case lLength:
    case sLength:
        return TagDataType::TimeSpan;
    case lBpm:
    case sBpm:
    case lYear:
    case sYear:
    case lPlayCounter:
    case sPlayCounter:
        return TagDataType::Integer;
    case lTrackPosition:
    case sTrackPosition:
    case lDiskPosition:
        return TagDataType::PositionInSet;
    case lCover:
    case sCover:
        return TagDataType::Picture;
    case lRating:
    case sRating:
        return TagDataType::Popularity;
    default:
        if (Id3v2FrameIds::isTextFrame(id)) {
            return TagDataType::Text;
        } else {
            return TagDataType::Undefined;
        }
    }
}

/*!
 * \brief Converts the lYear/lRecordingDates/lDate/lTime/sYear/sRecordingDates/sDate/sTime fields found in v2.3.0 to lRecordingTime.
 * \remarks
 * - Do not get rid of the "old" fields after the conversion so the raw fields can still be checked.
 * - The make function converts back if necassary and deletes unsupported fields.
 */
void Id3v2Tag::convertOldRecordDateFields(const std::string &diagContext, Diagnostics &diag)
{
    // skip if it is a v2.4.0 tag and lRecordingTime is present
    if (majorVersion() >= 4 && fields().find(Id3v2FrameIds::lRecordingTime) != fields().cend()) {
        return;
    }

    // parse values of lYear/lRecordingDates/lDate/lTime/sYear/sRecordingDates/sDate/sTime fields
    auto expr = DateTimeExpression();
    auto year = 1, month = 1, day = 1, hour = 0, minute = 0;
    if (const auto &v = value(Id3v2FrameIds::lYear)) {
        expr.parts |= DateTimeParts::Year;
        try {
            year = v.toInteger();
        } catch (const ConversionException &e) {
            diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse year from \"TYER\" frame: ", e.what()), diagContext);
        }
    }
    if (const auto &v = value(Id3v2FrameIds::lDate)) {
        expr.parts |= DateTimeParts::Day | DateTimeParts::Month;
        try {
            auto str = v.toString();
            if (str.size() != 4) {
                throw ConversionException("format is not DDMM");
            }
            day = stringToNumber<unsigned short>(std::string_view(str.data() + 0, 2));
            month = stringToNumber<unsigned short>(std::string_view(str.data() + 2, 2));
        } catch (const ConversionException &e) {
            diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse month and day from \"TDAT\" frame: ", e.what()), diagContext);
        }
    }
    if (const auto &v = value(Id3v2FrameIds::lTime)) {
        expr.parts |= DateTimeParts::Hour | DateTimeParts::Minute;
        try {
            auto str = v.toString();
            if (str.size() != 4) {
                throw ConversionException("format is not HHMM");
            }
            hour = stringToNumber<unsigned short>(std::string_view(str.data() + 0, 2));
            minute = stringToNumber<unsigned short>(std::string_view(str.data() + 2, 2));
        } catch (const ConversionException &e) {
            diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse hour and minute from \"TIME\" frame: ", +e.what()), diagContext);
        }
    }

    // set the field values as DateTime
    if (expr.parts == DateTimeParts::None) {
        return;
    }
    try {
        expr.value = DateTime::fromDateAndTime(year, month, day, hour, minute);
        setValue(Id3v2FrameIds::lRecordingTime, TagValue(expr));
    } catch (const ConversionException &e) {
        try {
            // try to set at least the year
            expr.parts = DateTimeParts::Year;
            expr.value = DateTime::fromDate(year);
            setValue(Id3v2FrameIds::lRecordingTime, TagValue(expr));
            diag.emplace_back(DiagLevel::Critical,
                argsToString(
                    "Unable to parse the full date of the recording. Only the 'Year' frame could be parsed; related frames failed: ", e.what()),
                diagContext);
        } catch (const ConversionException &) {
        }
        diag.emplace_back(
            DiagLevel::Critical, argsToString("Unable to parse a valid date from the 'Year' frame and related frames: ", e.what()), diagContext);
    }
}

/*!
 * \brief Parses tag information from the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Id3v2Tag::parse(istream &stream, const std::uint64_t maximalSize, Diagnostics &diag)
{
    // prepare parsing
    static const string context("parsing ID3v2 tag");
    BinaryReader reader(&stream);
    const auto startOffset = static_cast<std::uint64_t>(stream.tellg());

    // check whether the header is truncated
    if (maximalSize && maximalSize < 10) {
        diag.emplace_back(DiagLevel::Critical, "ID3v2 header is truncated (at least 10 bytes expected).", context);
        throw TruncatedDataException();
    }

    // read signature: ID3
    if (reader.readUInt24BE() != 0x494433u) {
        diag.emplace_back(DiagLevel::Critical, "Signature is invalid.", context);
        throw InvalidDataException();
    }
    // read header data
    const std::uint8_t majorVersion = reader.readByte();
    const std::uint8_t revisionVersion = reader.readByte();
    setVersion(majorVersion, revisionVersion);
    m_flags = reader.readByte();
    m_sizeExcludingHeader = reader.readSynchsafeUInt32BE();
    m_size = 10 + m_sizeExcludingHeader;
    if (m_sizeExcludingHeader == 0) {
        diag.emplace_back(DiagLevel::Warning, "ID3v2 tag seems to be empty.", context);
        return;
    }

    // check if the version
    if (!isVersionSupported()) {
        diag.emplace_back(DiagLevel::Critical, "The ID3v2 tag couldn't be parsed, because its version is not supported.", context);
        throw VersionNotSupportedException();
    }

    // read extended header (if present)
    if (hasExtendedHeader()) {
        if (maximalSize && maximalSize < 14) {
            diag.emplace_back(DiagLevel::Critical, "Extended header denoted but not present.", context);
            throw TruncatedDataException();
        }
        m_extendedHeaderSize = reader.readSynchsafeUInt32BE();
        if (m_extendedHeaderSize < 6 || m_extendedHeaderSize > m_sizeExcludingHeader || (maximalSize && maximalSize < (10 + m_extendedHeaderSize))) {
            diag.emplace_back(DiagLevel::Critical, "Extended header is invalid/truncated.", context);
            throw TruncatedDataException();
        }
        stream.seekg(m_extendedHeaderSize - 4, ios_base::cur);
    }

    // how many bytes remain for frames and padding?
    std::uint32_t bytesRemaining = m_sizeExcludingHeader - m_extendedHeaderSize;
    if (maximalSize && bytesRemaining > maximalSize) {
        bytesRemaining = static_cast<std::uint32_t>(maximalSize);
        diag.emplace_back(DiagLevel::Critical, "Frames are truncated.", context);
    }

    // read frames
    auto pos = static_cast<std::uint64_t>(stream.tellg());
    while (bytesRemaining) {
        // seek to next frame
        stream.seekg(static_cast<streamoff>(pos));
        // parse frame
        Id3v2Frame frame;
        try {
            frame.parse(reader, majorVersion, bytesRemaining, diag);
            if (Id3v2FrameIds::isTextFrame(frame.id()) && fields().count(frame.id()) == 1) {
                diag.emplace_back(DiagLevel::Warning, "The text frame " % frame.idToString() + " exists more than once.", context);
            }
            fields().emplace(frame.id(), std::move(frame));
        } catch (const NoDataFoundException &) {
            if (frame.hasPaddingReached()) {
                m_paddingSize = startOffset + m_size - pos;
                break;
            }
        } catch (const Failure &) {
        }

        // calculate next frame offset
        if (frame.totalSize() <= bytesRemaining) {
            pos += frame.totalSize();
            bytesRemaining -= frame.totalSize();
        } else {
            pos += bytesRemaining;
            bytesRemaining = 0;
        }
    }

    if (m_handlingFlags & Id3v2HandlingFlags::ConvertRecordDateFields) {
        convertOldRecordDateFields(context, diag);
    }

    // check for extended header
    if (!hasFooter()) {
        return;
    }
    if (maximalSize && m_size + 10 < maximalSize) {
        // the footer does not provide additional information, just check the signature
        stream.seekg(static_cast<streamoff>(startOffset + (m_size += 10)));
        if (reader.readUInt24LE() != 0x494433u) {
            diag.emplace_back(DiagLevel::Critical, "Footer signature is invalid.", context);
        }
        // skip remaining footer
        stream.seekg(7, ios_base::cur);
    } else {
        diag.emplace_back(DiagLevel::Critical, "Footer denoted but not present.", context);
        throw TruncatedDataException();
    }
}

/*!
 * \brief Prepares making.
 * \returns Returns a Id3v2TagMaker object which can be used to actually make the tag.
 * \remarks The tag must NOT be mutated after making is prepared when it is intended to actually
 *          make the tag using the make method of the returned object.
 * \throws Throws TagParser::Failure or a derived exception when a making error occurs.
 *
 * This method might be useful when it is necessary to know the size of the tag before making it.
 * \sa make()
 */
Id3v2TagMaker Id3v2Tag::prepareMaking(Diagnostics &diag)
{
    return Id3v2TagMaker(*this, diag);
}

/*!
 * \brief Writes tag information to the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 */
void Id3v2Tag::make(ostream &stream, std::uint32_t padding, Diagnostics &diag)
{
    prepareMaking(diag).make(stream, padding, diag);
}

/*!
 * \brief Sets the version to the specified \a majorVersion and
 *        the specified \a revisionVersion.
 */
void Id3v2Tag::setVersion(std::uint8_t majorVersion, std::uint8_t revisionVersion)
{
    m_majorVersion = majorVersion;
    m_revisionVersion = revisionVersion;
    m_version = argsToString('2', '.', majorVersion, '.', revisionVersion);
}

/*!
 * \class TagParser::FrameComparer
 * \brief Defines the order which is used to store ID3v2 frames.
 *
 * The order is: unique file id, title, other text frames, other frames, cover
 */

/*!
 * \brief Returns true if \a lhs goes before \a rhs; otherwise returns false.
 * \remarks Long and short IDs are treated equal if the short ID can be converted to
 *          the corresponding long ID. Otherwise short IDs go before long IDs.
 */
bool FrameComparer::operator()(std::uint32_t lhs, std::uint32_t rhs) const
{
    if (lhs == rhs) {
        return false;
    }

    const bool lhsLong = Id3v2FrameIds::isLongId(lhs);
    const bool rhsLong = Id3v2FrameIds::isLongId(rhs);
    if (lhsLong != rhsLong) {
        if (!lhsLong) {
            lhs = Id3v2FrameIds::convertToLongId(lhs);
            if (!lhs) {
                return true;
            }
        } else if (!rhsLong) {
            rhs = Id3v2FrameIds::convertToLongId(rhs);
            if (!rhs) {
                return true;
            }
        }
    }

    if (lhs == Id3v2FrameIds::lUniqueFileId || lhs == Id3v2FrameIds::sUniqueFileId) {
        return true;
    }
    if (rhs == Id3v2FrameIds::lUniqueFileId || rhs == Id3v2FrameIds::sUniqueFileId) {
        return false;
    }
    if (lhs == Id3v2FrameIds::lTitle || lhs == Id3v2FrameIds::sTitle) {
        return true;
    }
    if (rhs == Id3v2FrameIds::lTitle || rhs == Id3v2FrameIds::sTitle) {
        return false;
    }

    const bool lhstextfield = Id3v2FrameIds::isTextFrame(lhs);
    const bool rhstextfield = Id3v2FrameIds::isTextFrame(rhs);
    if (lhstextfield && !rhstextfield) {
        return true;
    }
    if (!lhstextfield && rhstextfield) {
        return false;
    }

    if (lhs == Id3v2FrameIds::lCover || lhs == Id3v2FrameIds::sCover) {
        return false;
    }
    if (rhs == Id3v2FrameIds::lCover || rhs == Id3v2FrameIds::sCover) {
        return true;
    }
    return lhs < rhs;
}

/*!
 * \class TagParser::Id3v2TagMaker
 * \brief The Id3v2TagMaker class helps writing ID3v2 tags.
 *
 * An instance can be obtained using the Id3v2Tag::prepareMaking() method.
 */

/*!
 * \brief Removes all old (major version <= 3) record date related fields.
 */
void Id3v2Tag::removeOldRecordDateRelatedFields()
{
    for (auto field : { Id3v2FrameIds::lYear, Id3v2FrameIds::lRecordingDates, Id3v2FrameIds::lDate, Id3v2FrameIds::lTime }) {
        fields().erase(field);
    }
}

/*!
 * \brief Prepare the fields to save the record data according to the ID3v2 version.
 */
void Id3v2Tag::prepareRecordDataForMaking(const std::string &diagContext, Diagnostics &diag)
{
    // get rid of lYear/lRecordingDates/lDate/lTime/sYear/sRecordingDates/sDate/sTime if writing v2.4.0 or newer
    // note: If the tag was initially v2.3.0 or older the "old" fields have already been converted to lRecordingTime when
    //        parsing and the generic accessors propose using lRecordingTime in any case.
    if (majorVersion() >= 4) {
        removeOldRecordDateRelatedFields();
        return;
    }

    // convert lRecordingTime to old fields for v2.3.0 and older
    const auto recordingTimeFieldIterator = fields().find(Id3v2FrameIds::lRecordingTime);
    // -> If the auto-created lRecordingTime field (see note above) has been completely removed write the old fields as-is.
    //    This allows one to bypass this handling and set the old fields explicitly.
    if (recordingTimeFieldIterator == fields().cend()) {
        return;
    }
    // -> simply remove all old fields if lRecordingTime is set to an empty value
    const auto &recordingTime = recordingTimeFieldIterator->second.value();
    if (recordingTime.isEmpty()) {
        removeOldRecordDateRelatedFields();
        return;
    }
    // -> convert lRecordingTime (which is supposed to be an ISO string) to a DateTime
    try {
        const auto dateTimeExpr = recordingTime.toDateTimeExpression();
        const auto &asDateTime = dateTimeExpr.value;
        // -> remove any existing old fields to avoid any leftovers
        removeOldRecordDateRelatedFields();
        // -> assign old fields from parsed DateTime
        std::stringstream year, date, time;
        if (dateTimeExpr.parts & DateTimeParts::Year) {
            year << std::setfill('0') << std::setw(4) << asDateTime.year();
            setValue(Id3v2FrameIds::lYear, TagValue(year.str()));
        }
        if (dateTimeExpr.parts & (DateTimeParts::Day | DateTimeParts::Month)) {
            date << std::setfill('0') << std::setw(2) << asDateTime.day() << std::setfill('0') << std::setw(2) << asDateTime.month();
            setValue(Id3v2FrameIds::lDate, TagValue(date.str()));
        }
        if (dateTimeExpr.parts & DateTimeParts::Time) {
            time << std::setfill('0') << std::setw(2) << asDateTime.hour() << std::setfill('0') << std::setw(2) << asDateTime.minute();
            setValue(Id3v2FrameIds::lTime, TagValue(time.str()));
        }
        if (dateTimeExpr.parts & (DateTimeParts::Second | DateTimeParts::SubSecond)) {
            diag.emplace_back(DiagLevel::Warning,
                "The recording time field (TDRC) has been truncated to full minutes when converting to corresponding fields for older ID3v2 "
                "versions.",
                diagContext);
        }
    } catch (const ConversionException &e) {
        try {
            diag.emplace_back(DiagLevel::Critical,
                argsToString("Unable to convert recording time field (TDRC) with the value \"", recordingTime.toString(),
                    "\" to corresponding fields for older ID3v2 versions: ", e.what()),
                diagContext);
        } catch (const ConversionException &) {
            diag.emplace_back(DiagLevel::Critical,
                argsToString("Unable to convert recording time field (TRDA) to corresponding fields for older ID3v2 versions: ", e.what()),
                diagContext);
        }
    }
    // -> get rid of lRecordingTime
    fields().erase(Id3v2FrameIds::lRecordingTime);
}

/*!
 * \brief Prepares making the specified \a tag.
 * \sa See Id3v2Tag::prepareMaking() for more information.
 */
Id3v2TagMaker::Id3v2TagMaker(Id3v2Tag &tag, Diagnostics &diag)
    : m_tag(tag)
    , m_framesSize(0)
{
    static const string context("making ID3v2 tag");

    // check if version is supported
    // (the version could have been changed using setVersion())
    if (!tag.isVersionSupported()) {
        diag.emplace_back(DiagLevel::Critical, "The ID3v2 tag version isn't supported.", context);
        throw VersionNotSupportedException();
    }

    if (m_tag.m_handlingFlags & Id3v2HandlingFlags::ConvertRecordDateFields) {
        tag.prepareRecordDataForMaking(context, diag);
    }

    // prepare frames
    m_maker.reserve(tag.fields().size());
    for (auto &pair : tag.fields()) {
        try {
            m_maker.emplace_back(pair.second.prepareMaking(tag.majorVersion(), diag));
            m_framesSize += m_maker.back().requiredSize();
        } catch (const Failure &) {
        }
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
 *                throw TagParser::Failure or a derived exception.
 */
void Id3v2TagMaker::make(std::ostream &stream, std::uint32_t padding, Diagnostics &diag)
{
    CPP_UTILITIES_UNUSED(diag)

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
    for (auto &maker : m_maker) {
        maker.make(writer);
    }

    // write padding
    for (; padding; --padding) {
        stream.put(0);
    }
}

} // namespace TagParser
