#include "./id3v1tag.h"
#include "./id3genres.h"

#include "../exceptions.h"

#include <c++utilities/conversion/conversionexception.h>

#include <cstring>

using namespace std;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::Id3v1Tag
 * \brief Implementation of Media::Tag for ID3v1 tags.
 */

/*!
 * \brief Constructs a new tag.
 */
Id3v1Tag::Id3v1Tag()
{}

TagType Id3v1Tag::type() const
{
    return TagType::Id3v1Tag;
}

const char *Id3v1Tag::typeName() const
{
    return tagName;
}

bool Id3v1Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return Tag::canEncodingBeUsed(encoding);
}

/*!
 * \brief Parses tag information from the specified \a stream.
 * \param stream Specifies the stream to read from.
 * \param autoSeek Specifies whether the parser should automatically seek at the end of stream.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Id3v1Tag::parse(std::istream &stream, bool autoSeek)
{
    invalidateStatus();
    static const string context("parsing ID3v1 tag");
    if(autoSeek) {
        stream.seekg(-128, ios_base::end);
    }
    char buffer[128];
    stream.read(buffer, 128);
    if(buffer[0] == 0x54
            && buffer[1] == 0x41
            && buffer[2] == 0x47) {
        m_size = 128;
        readValue(m_title, 30, buffer + 3);
        readValue(m_artist, 30, buffer + 33);
        readValue(m_album, 30, buffer + 63);
        readValue(m_year, 4, buffer + 93);
        if(buffer[125] == 0) {
            readValue(m_comment, 28, buffer + 97);
            m_version = "1.1";
        } else {
            readValue(m_comment, 30, buffer + 97);
            m_version = "1.0";
        }
        readValue(m_comment, buffer[125] == 0 ? 28 : 30, buffer + 97);
        if(buffer[125] == 0) {
            m_trackPos.assignPosition(PositionInSet(*reinterpret_cast<char *>(buffer + 126), 0));
        }
        m_genre.assignStandardGenreIndex(*reinterpret_cast<unsigned char *>(buffer + 127));
    } else {
        addNotification(NotificationType::Critical, "\"TAG\" identifier not found.", context);
        throw NoDataFoundException();
    }
}

/*!
 * \brief Parses tag information from the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Id3v1Tag::parse(std::iostream &stream)
{
    return parse(stream, false);
}

/*!
 * \brief Writes tag information to the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void Id3v1Tag::make(ostream &stream)
{
    invalidateStatus();
    static const string context("making ID3v1 tag");
    char buffer[30];
    buffer[0] = 0x54;
    buffer[1] = 0x41;
    buffer[2] = 0x47;
    stream.write(buffer, 3);
    // write text fields
    writeValue(m_title, 30, buffer, stream);
    writeValue(m_artist, 30, buffer, stream);
    writeValue(m_album, 30, buffer, stream);
    writeValue(m_year, 4, buffer, stream);
    writeValue(m_comment, 28, buffer, stream);
    // write numeric fields
    buffer[0] = 0x0; // empty byte
    buffer[1] = 0x0; // track nr
    buffer[2] = 0x0; // genre
    // track
    try {
    if(!m_trackPos.isEmpty() && m_trackPos.type() == TagDataType::PositionInSet)
        buffer[1] = m_trackPos.toPositionInSet().position();
    } catch(const ConversionException &) {
        addNotification(NotificationType::Warning, "Track position field can not be set because given value can not be converted appropriately.", context);
    }
    // genre
    try {
        buffer[2] = m_genre.toStandardGenreIndex();
    } catch(const ConversionException &) {
        addNotification(NotificationType::Warning, "Genre field can not be set because given value can not be converted appropriately.", context);
    }
    stream.write(buffer, 3);
    stream.flush();
}

const TagValue &Id3v1Tag::value(KnownField field) const
{
    switch(field) {
    case KnownField::Title:
        return m_title;
    case KnownField::Artist:
        return m_artist;
    case KnownField::Album:
        return m_album;
    case KnownField::Year:
        return m_year;
    case KnownField::Comment:
        return m_comment;
    case KnownField::TrackPosition:
        return m_trackPos;
    case KnownField::Genre:
        return m_genre;
    default:
        return TagValue::empty();
    }
}

bool Id3v1Tag::setValue(KnownField field, const TagValue &value)
{
    switch(field) {
    case KnownField::Title:
        m_title = value;
        break;
    case KnownField::Artist:
        m_artist = value;
        break;
    case KnownField::Album:
        m_album = value;
        break;
    case KnownField::Year:
        m_year = value;
        break;
    case KnownField::Comment:
        m_comment = value;
        break;
    case KnownField::TrackPosition:
        m_trackPos = value;
        break;
    case KnownField::Genre:
        m_genre = value;
        break;
    default:
        return false;
    }
    return true;
}

bool Id3v1Tag::setValueConsideringTypeInfo(KnownField field, const TagValue &value, const string &)
{
    return setValue(field, value);
}

bool Id3v1Tag::hasField(KnownField field) const
{
    switch(field) {
    case KnownField::Title:
        return !m_title.isEmpty();
    case KnownField::Artist:
        return !m_artist.isEmpty();
    case KnownField::Album:
        return !m_album.isEmpty();
    case KnownField::Year:
        return !m_year.isEmpty();
    case KnownField::Comment:
        return !m_comment.isEmpty();
    case KnownField::TrackPosition:
        return !m_trackPos.isEmpty();
    case KnownField::Genre:
        return !m_genre.isEmpty();
    default:
        return false;
    }
}

void Id3v1Tag::removeAllFields()
{
    m_title.clearDataAndMetadata();
    m_artist.clearDataAndMetadata();
    m_album.clearDataAndMetadata();
    m_year.clearDataAndMetadata();
    m_comment.clearDataAndMetadata();
    m_trackPos.clearDataAndMetadata();
    m_genre.clearDataAndMetadata();
}

unsigned int Id3v1Tag::fieldCount() const
{
    int count = 0;
    for(const auto &value : {m_title, m_artist, m_album,
            m_year, m_comment, m_trackPos, m_genre}) {
        if(!value.isEmpty()) {
            ++count;
        }
    }
    return count;
}

bool Id3v1Tag::supportsField(KnownField field) const
{
    switch(field) {
    case KnownField::Title:
    case KnownField::Artist:
    case KnownField::Album:
    case KnownField::Year:
    case KnownField::Comment:
    case KnownField::TrackPosition:
    case KnownField::Genre:
        return true;
    default:
        return false;
    }
}

void Id3v1Tag::ensureTextValuesAreProperlyEncoded()
{
    m_title.convertDataEncodingForTag(this);
    m_artist.convertDataEncodingForTag(this);
    m_album.convertDataEncodingForTag(this);
    m_year.convertDataEncodingForTag(this);
    m_comment.convertDataEncodingForTag(this);
    m_trackPos.convertDataEncodingForTag(this);
    m_genre.convertDataEncodingForTag(this);
}

/*!
 * \brief Internally used to read values with the specified \a maxLength from the specified \a buffer.
 */
void Id3v1Tag::readValue(TagValue &value, size_t maxLength, const char *buffer)
{
    const char *end = buffer + maxLength - 1;
    while((*end == 0x0 || *end == ' ') && end >= buffer) {
        --end;
        --maxLength;
    }
    value.assignData(buffer, maxLength, TagDataType::Text, TagTextEncoding::Latin1);
}


/*!
 * \brief Internally used to write values.
 */
void Id3v1Tag::writeValue(const TagValue &value, size_t length, char *buffer, ostream &targetStream)
{
    memset(buffer, 0, length);
    try {
        value.toString().copy(buffer, length);
    } catch(const ConversionException &) {
        addNotification(NotificationType::Warning, "Field can not be set because given value can not be converted appropriately.", "making ID3v1 tag field");
    }
    targetStream.write(buffer, length);
}

}
