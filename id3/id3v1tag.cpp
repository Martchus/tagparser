#include "./id3v1tag.h"

#include "../diagnostics.h"
#include "../exceptions.h"

#include <c++utilities/conversion/conversionexception.h>
#include <c++utilities/conversion/stringbuilder.h>

#include <cstring>
#include <initializer_list>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::Id3v1Tag
 * \brief Implementation of TagParser::Tag for ID3v1 tags.
 */

/*!
 * \brief Constructs a new tag.
 */
Id3v1Tag::Id3v1Tag()
{
}

TagType Id3v1Tag::type() const
{
    return TagType::Id3v1Tag;
}

std::string_view Id3v1Tag::typeName() const
{
    return tagName;
}

/*!
 * \brief Returns only true for TagTextEncoding::Latin1.
 * \remarks
 * The encoding to be used within ID3v1 tags is not standardized but it seems that Latin-1 is the most
 * commonly used character set and hence safest to use. Hence that is the only encoding which can be safely
 * recommended here. Despite that, the Id3v1Tag class is actually able to deal with UTF-8 as well. It will
 * use the BOM to detect and serialize UTF-8.
 */
bool Id3v1Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == TagTextEncoding::Latin1;
}

/*!
 * \brief Parses tag information from the specified \a stream.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Id3v1Tag::parse(std::istream &stream, Diagnostics &diag)
{
    CPP_UTILITIES_UNUSED(diag)
    char buffer[128];
    stream.read(buffer, 128);
    if (buffer[0] != 0x54 || buffer[1] != 0x41 || buffer[2] != 0x47) {
        throw NoDataFoundException();
    }
    m_size = 128;
    readValue(m_title, 30, buffer + 3);
    readValue(m_artist, 30, buffer + 33);
    readValue(m_album, 30, buffer + 63);
    readValue(m_year, 4, buffer + 93);
    const auto is11 = buffer[125] == 0;
    if (is11) {
        readValue(m_comment, 28, buffer + 97);
        m_version = "1.1";
    } else {
        readValue(m_comment, 30, buffer + 97);
        m_version = "1.0";
    }
    readValue(m_comment, is11 ? 28 : 30, buffer + 97);
    if (is11) {
        m_trackPos.assignPosition(PositionInSet(*reinterpret_cast<char *>(buffer + 126), 0));
    }
    m_genre.assignStandardGenreIndex(*reinterpret_cast<unsigned char *>(buffer + 127));
}

/*!
 * \brief Writes tag information to the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 */
void Id3v1Tag::make(ostream &stream, Diagnostics &diag)
{
    static const string context("making ID3v1 tag");
    char buffer[30];
    buffer[0] = 0x54;
    buffer[1] = 0x41;
    buffer[2] = 0x47;
    stream.write(buffer, 3);

    // write text fields
    writeValue(m_title, 30, buffer, stream, diag);
    writeValue(m_artist, 30, buffer, stream, diag);
    writeValue(m_album, 30, buffer, stream, diag);
    writeValue(m_year, 4, buffer, stream, diag);
    writeValue(m_comment, 28, buffer, stream, diag);

    // set "default" values for numeric fields
    buffer[0] = 0x0; // empty byte
    buffer[1] = 0x0; // track number
    buffer[2] = 0x0; // genre

    // write track
    if (!m_trackPos.isEmpty()) {
        try {
            const auto position(m_trackPos.toPositionInSet().position());
            if (position < 0x00 || position > 0xFF) {
                throw ConversionException();
            }
            buffer[1] = static_cast<char>(position);
        } catch (const ConversionException &) {
            diag.emplace_back(
                DiagLevel::Warning, "Track position field can not be set because given value can not be converted appropriately.", context);
        }
    }

    // write genre
    try {
        const auto genreIndex(m_genre.toStandardGenreIndex());
        if (genreIndex < 0x00 || genreIndex > 0xFF) {
            throw ConversionException();
        }
        buffer[2] = static_cast<char>(genreIndex);
    } catch (const ConversionException &) {
        diag.emplace_back(DiagLevel::Warning,
            "Genre field can not be set because given value can not be converted to a standard genre number supported by ID3v1.", context);
    }

    stream.write(buffer, 3);
    stream.flush();
}

const TagValue &Id3v1Tag::value(KnownField field) const
{
    switch (field) {
    case KnownField::Title:
        return m_title;
    case KnownField::Artist:
        return m_artist;
    case KnownField::Album:
        return m_album;
    case KnownField::RecordDate:
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
    switch (field) {
    case KnownField::Title:
        m_title = value;
        break;
    case KnownField::Artist:
        m_artist = value;
        break;
    case KnownField::Album:
        m_album = value;
        break;
    case KnownField::RecordDate:
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
    switch (field) {
    case KnownField::Title:
        return !m_title.isEmpty();
    case KnownField::Artist:
        return !m_artist.isEmpty();
    case KnownField::Album:
        return !m_album.isEmpty();
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

std::size_t Id3v1Tag::fieldCount() const
{
    auto count = std::size_t(0);
    for (const auto &value : std::initializer_list<const TagValue *>{ &m_title, &m_artist, &m_album, &m_year, &m_comment, &m_trackPos, &m_genre }) {
        if (!value->isEmpty()) {
            ++count;
        }
    }
    return count;
}

bool Id3v1Tag::supportsField(KnownField field) const
{
    switch (field) {
    case KnownField::Title:
    case KnownField::Artist:
    case KnownField::Album:
    case KnownField::RecordDate:
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
    for (auto *value : initializer_list<TagValue *>{ &m_title, &m_artist, &m_album, &m_year, &m_comment, &m_trackPos, &m_genre }) {
        // convert UTF-16 to UTF-8
        switch (value->dataEncoding()) {
        case TagTextEncoding::Latin1:
        case TagTextEncoding::Utf8:
        case TagTextEncoding::Unspecified:
            break;
        default:
            value->convertDataEncoding(TagTextEncoding::Utf8);
        }
    }
}

/*!
 * \brief Internally used to read values with the specified \a maxLength from the specified \a buffer.
 */
void Id3v1Tag::readValue(TagValue &value, size_t maxLength, const char *buffer)
{
    const char *end = buffer + maxLength - 1;
    while ((*end == 0x0 || *end == ' ') && end >= buffer) {
        --end;
        --maxLength;
    }
    if (buffer == end) {
        return;
    }
    if (maxLength >= 3 && BE::toUInt24(buffer) == 0x00EFBBBF) {
        value.assignData(buffer + 3, maxLength - 3, TagDataType::Text, TagTextEncoding::Utf8);
    } else {
        value.assignData(buffer, maxLength, TagDataType::Text, TagTextEncoding::Latin1);
    }
}

/*!
 * \brief Internally used to write values.
 */
void Id3v1Tag::writeValue(const TagValue &value, size_t length, char *buffer, ostream &targetStream, Diagnostics &diag)
{
    // initialize buffer with zeroes
    memset(buffer, 0, length);

    // stringify value
    string valueAsString;
    try {
        valueAsString = value.toString();
    } catch (const ConversionException &) {
        diag.emplace_back(
            DiagLevel::Warning, "Field can not be set because given value can not be converted appropriately.", "making ID3v1 tag field");
    }

    // handle encoding
    auto *valueStart = buffer;
    auto valueLength = length;
    auto hasProblematicEncoding = false;
    switch (value.dataEncoding()) {
    case TagTextEncoding::Latin1:
        break;
    case TagTextEncoding::Utf8:
        // write UTF-8 BOM if the value contains non-ASCII characters
        for (const auto c : valueAsString) {
            if ((c & 0x80) == 0) {
                continue;
            }
            buffer[0] = static_cast<char>(0xEF);
            buffer[1] = static_cast<char>(0xBB);
            buffer[2] = static_cast<char>(0xBF);
            valueStart += 3;
            valueLength -= 3;
            hasProblematicEncoding = true;
            break;
        }
        break;
    default:
        hasProblematicEncoding = true;
    }
    if (hasProblematicEncoding) {
        diag.emplace_back(DiagLevel::Warning, "The used encoding is unlikely to be supported by other software.", "making ID3v1 tag field");
    }

    // copy the string
    if (valueAsString.size() > length) {
        diag.emplace_back(
            DiagLevel::Warning, argsToString("Value has been truncated. Max. ", length, " characters supported."), "making ID3v1 tag field");
    }
    valueAsString.copy(valueStart, valueLength);

    targetStream.write(buffer, static_cast<streamsize>(length));
}

} // namespace TagParser
