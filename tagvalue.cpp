#include "./tagvalue.h"
#include "./tag.h"

#include "./id3/id3genres.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/conversion/conversionexception.h>

#include <algorithm>
#include <utility>
#include <cstring>

using namespace std;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

namespace Media {

/*!
 * \class Media::TagValue
 * \brief The TagValue class wraps values of different types. It is meant to be assigned to a tag field.
 *
 * For a list of supported types see Media::TagDataType.
 */

/*!
 * \brief Constructs a new TagValue holding a copy of the given TagValue instance.
 * \param other Specifies another TagValue instance.
 */
TagValue::TagValue(const TagValue &other) :
    m_size(other.m_size),
    m_type(other.m_type),
    m_desc(other.m_desc),
    m_mimeType(other.m_mimeType),
    m_lng(other.m_lng),
    m_labeledAsReadonly(other.m_labeledAsReadonly),
    m_encoding(other.m_encoding),
    m_descEncoding(other.m_descEncoding)
{
    if(!other.isEmpty()) {
        m_ptr = make_unique<char []>(m_size);
        std::copy(other.m_ptr.get(), other.m_ptr.get() + other.m_size, m_ptr.get());
    }
}

/*!
 * \brief Assigns the value of another TagValue to the current instance.
 */
TagValue &TagValue::operator=(const TagValue &other)
{
    if(this != &other) {
        m_size = other.m_size;
        m_type = other.m_type;
        m_desc = other.m_desc;
        m_mimeType = other.m_mimeType;
        m_lng = other.m_lng;
        m_labeledAsReadonly = other.m_labeledAsReadonly;
        m_encoding = other.m_encoding;
        m_descEncoding = other.m_descEncoding;
        if(other.isEmpty()) {
            m_ptr.reset();
        } else {
            m_ptr = make_unique<char[]>(m_size);
            std::copy(other.m_ptr.get(), other.m_ptr.get() + other.m_size, m_ptr.get());
        }
    }
    return *this;
}

/*!
 * \brief Returns whether both instances are equal.
 *
 * If the data types are not equal, two instances are still considered equal if the string representation
 * is identical. The encoding and meta data must be equal as well (if relevant for the data type).
 */
bool TagValue::operator==(const TagValue &other) const
{
    if(m_desc != other.m_desc || (!m_desc.empty() && m_descEncoding != other.m_descEncoding)
            || m_mimeType != other.m_mimeType || m_lng != other.m_lng || m_labeledAsReadonly != other.m_labeledAsReadonly) {
        return false;
    }
    if(m_type == other.m_type) {
        switch(m_type) {
        case TagDataType::Text:
            if(m_size != other.m_size && m_encoding != other.m_encoding) {
                return false;
            }
            return strncmp(m_ptr.get(), other.m_ptr.get(), m_size) == 0;
        case TagDataType::PositionInSet:
            return toPositionInSet() == other.toPositionInSet();
        case TagDataType::Integer:
            return toInteger() == other.toInteger();
        case TagDataType::StandardGenreIndex:
            return toStandardGenreIndex() == other.toStandardGenreIndex();
        case TagDataType::TimeSpan:
            return toTimeSpan() == other.toTimeSpan();
        case TagDataType::DateTime:
            return toDateTime() == other.toDateTime();
        case TagDataType::Picture:
        case TagDataType::Binary:
        case TagDataType::Undefined:
            if(m_size != other.m_size) {
                return false;
            }
            return strncmp(m_ptr.get(), other.m_ptr.get(), m_size) == 0;
        default:
            return false;
        }
    } else {
        // different types
        try {
            // try to convert both values to string
            // if the string representations are equal, both values can also be considered equal
            return toString() == other.toString();
        } catch(const ConversionException &) {
            return false;
        }
    }
}

/*!
 * \brief Destroys the TagValue.
 */
TagValue::~TagValue()
{}

/*!
 * \brief Wipes assigned meta data.
 *  - Clears description, mime type and language.
 *  - Resets the read-only flag to false.
 *  - Resets the encoding to TagTextEncoding::Latin1.
 *  - Resets the data type to TagDataType::Undefined.
 */
void TagValue::clearMetadata()
{
    m_desc.clear();
    m_mimeType.clear();
    m_lng.clear();
    m_labeledAsReadonly = false;
    m_encoding = TagTextEncoding::Latin1;
    m_type = TagDataType::Undefined;
}

/*!
 * \brief Wipes assigned data including meta data.
 * \sa clearData()
 * \sa clearMetadata()
 */
void TagValue::clearDataAndMetadata()
{
    clearData();
    clearMetadata();
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        integer representation.
 * \throws Throws ConversionException on failure.
 */
int32 TagValue::toInteger() const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            return ConversionUtilities::stringToNumber<int32>(string(m_ptr.get(), m_size));
        case TagDataType::Integer:
        case TagDataType::PositionInSet:
        case TagDataType::StandardGenreIndex:
            if(m_size == sizeof(int32)) {
                auto res = *reinterpret_cast<int32 *>(m_ptr.get());
                return res;
            } else {
                throw ConversionException("Can not convert assigned data to integer because the data size is not appropriate.");
            }
            break;
        default:
            throw ConversionException("Can not convert binary data/picture/time span/date time to integer.");
        }
    }
    return 0;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        standard genre index.
 * \throws Throws ConversionException on failure.
 */
int TagValue::toStandardGenreIndex() const
{
    if(!isEmpty()) {
        int index = 0;
        switch(m_type) {
        case TagDataType::Text: {
            string s(m_ptr.get(), m_size);
            try {
                index = ConversionUtilities::stringToNumber<int32>(s);
            } catch (ConversionException &) {
                index = Id3Genres::indexFromString(s);
            }
            break;
        } case TagDataType::StandardGenreIndex:
        case TagDataType::Integer:
            if(m_size == sizeof(int)) {
                index = *reinterpret_cast<int *>(m_ptr.get());
            } else {
                throw ConversionException("The assigned data is of unappropriate size.");
            }
            break;
        default:
            throw ConversionException("It is not possible to convert assigned data to a number because of its incompatible type.");
        }
        if(Id3Genres::isIndexSupported(index)) {
            return index;
        } else {
            throw ConversionException("The assigned number is not a valid standard genre index.");
        }
    }
    return 0;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        PositionInSet representation.
 * \throws Throws ConversionException on failure.
 */
PositionInSet TagValue::toPositionInSet() const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            switch(m_encoding) {
            case TagTextEncoding::Unspecified:
            case TagTextEncoding::Latin1:
            case TagTextEncoding::Utf8:
                return PositionInSet(string(m_ptr.get(), m_size));
            case TagTextEncoding::Utf16LittleEndian:
            case TagTextEncoding::Utf16BigEndian:
                // FIXME: Ensure endianness is correctly
                return PositionInSet(u16string(reinterpret_cast<char16_t *>(m_ptr.get()), m_size / 2));
            }
        case TagDataType::Integer:
        case TagDataType::PositionInSet:
            switch(m_size) {
            case sizeof(int32):
                return PositionInSet(*(reinterpret_cast<int32 *>(m_ptr.get())));
            case 2 * sizeof(int32):
                return PositionInSet(*(reinterpret_cast<int32 *>(m_ptr.get())), *(reinterpret_cast<int32 *>(m_ptr.get() + sizeof(int32))));
            default:
                throw ConversionException("The size of the assigned data is not appropriate.");
            }
        default:
            throw ConversionException("Can not convert binary data/genre index/picture to \"position in set\".");
        }
    }
    return PositionInSet();
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        TimeSpan representation.
 * \throws Throws ConversionException on failure.
 */
TimeSpan TagValue::toTimeSpan() const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            return TimeSpan::fromString(string(m_ptr.get(), m_size));
        case TagDataType::Integer:
        case TagDataType::TimeSpan:
            switch(m_size) {
            case sizeof(int32):
                return TimeSpan(*(reinterpret_cast<int32 *>(m_ptr.get())));
            case sizeof(int64):
                return TimeSpan(*(reinterpret_cast<int64 *>(m_ptr.get())));
            default:
                throw ConversionException("The size of the assigned data is not appropriate.");
            }
        default:
            throw ConversionException("Can not convert binary data/genre index/position in set/picture to time span.");
        }
    }
    return TimeSpan();
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        DateTime representation.
 * \throws Throws ConversionException on failure.
 */
DateTime TagValue::toDateTime() const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            return DateTime::fromString(string(m_ptr.get(), m_size));
        case TagDataType::Integer:
        case TagDataType::TimeSpan:
            if(m_size == sizeof(int32)) {
                return DateTime(*(reinterpret_cast<int32 *>(m_ptr.get())));
            } else if(m_size == sizeof(int64)) {
                return DateTime(*(reinterpret_cast<int64 *>(m_ptr.get())));
            } else {
                throw ConversionException("The assigned data is of unappropriate size.");
            }
        default:
            throw ConversionException("Can not convert binary data/genre index/position in set/picture to date time.");
        }
    }
    return DateTime();
}

/*!
 * \brief Returns the encoding parameter (name of the character set and bytes per character) for the specified \a tagTextEncoding.
 */
pair<const char *, float> encodingParameter(TagTextEncoding tagTextEncoding)
{
    switch(tagTextEncoding) {
    case TagTextEncoding::Latin1:
        return make_pair("ISO-8859-1", 1.0f);
    case TagTextEncoding::Utf8:
        return make_pair("UTF-8", 1.0f);
    case TagTextEncoding::Utf16LittleEndian:
        return make_pair("UTF-16LE", 2.0f);
    case TagTextEncoding::Utf16BigEndian:
        return make_pair("UTF-16BE", 2.0f);
    default:
        return make_pair(nullptr, 0.0f);
    }
}

/*!
 * \brief Converts the currently assigned text value to the specified \a encoding.
 * \throws Throws ConversionUtilities::ConversionException() if the conversion fails.
 * \remarks
 * - Does nothing if dataEncoding() equals \a encoding.
 * - Sets dataEncoding() to the specified \a encoding if the conversion succeeds.
 * - Does not do any conversion if the current type() is not TagDataType::Text.
 * \sa convertDataEncodingForTag()
 */
void TagValue::convertDataEncoding(TagTextEncoding encoding)
{
    if(m_encoding != encoding) {
        if(type() == TagDataType::Text) {
            StringData encodedData;
            switch(encoding) {
            case TagTextEncoding::Utf8:
                // use pre-defined methods when encoding to UTF-8
                switch(dataEncoding()) {
                case TagTextEncoding::Latin1:
                    encodedData = convertLatin1ToUtf8(m_ptr.get(), m_size);
                    break;
                case TagTextEncoding::Utf16LittleEndian:
                    encodedData = convertUtf16LEToUtf8(m_ptr.get(), m_size);
                    break;
                case TagTextEncoding::Utf16BigEndian:
                    encodedData = convertUtf16BEToUtf8(m_ptr.get(), m_size);
                    break;
                default:
                    ;
                }
                break;
            default: {
                // otherwise, determine input and output parameter to use general covertString method
                const auto inputParameter = encodingParameter(dataEncoding());
                const auto outputParameter = encodingParameter(encoding);
                encodedData = convertString(inputParameter.first, outputParameter.first, m_ptr.get(), m_size, outputParameter.second / inputParameter.second);
            }
            }
            // can't just move the encoded data because it needs to be deleted with free
            m_ptr = make_unique<char []>(m_size = encodedData.second);
            copy(encodedData.first.get(), encodedData.first.get() + encodedData.second, m_ptr.get());
        }
        m_encoding = encoding;
    }
}

/*!
 * \brief Ensures the encoding of the currently assigned text value is supported by the specified \a tag.
 * \sa This is a convenience method for convertDataEncoding().
 */
void TagValue::convertDataEncodingForTag(const Tag *tag)
{
    if(type() == TagDataType::Text && !tag->canEncodingBeUsed(dataEncoding())) {
        convertDataEncoding(tag->proposedTextEncoding());
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::string representation.
 * \param result Specifies the string to store the result.
 * \param encoding Specifies the encoding to to be used; set to TagTextEncoding::Unspecified to use the
 *        present encoding without any character set conversion.
 * \remarks If UTF-16 is the desired output \a encoding, it makes sense to use the toWString() method instead.
 * \throws Throws ConversionException on failure.
 */
void TagValue::toString(string &result, TagTextEncoding encoding) const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            if(encoding == TagTextEncoding::Unspecified || dataEncoding() == TagTextEncoding::Unspecified || encoding == dataEncoding()) {
                result.assign(m_ptr.get(), m_size);
            } else {
                StringData encodedData;
                switch(encoding) {
                case TagTextEncoding::Utf8:
                    // use pre-defined methods when encoding to UTF-8
                    switch(dataEncoding()) {
                    case TagTextEncoding::Latin1:
                        encodedData = convertLatin1ToUtf8(m_ptr.get(), m_size);
                        break;
                    case TagTextEncoding::Utf16LittleEndian:
                        encodedData = convertUtf16LEToUtf8(m_ptr.get(), m_size);
                        break;
                    case TagTextEncoding::Utf16BigEndian:
                        encodedData = convertUtf16BEToUtf8(m_ptr.get(), m_size);
                        break;
                    default:
                        ;
                    }
                    break;
                default: {
                    // otherwise, determine input and output parameter to use general covertString method
                    const auto inputParameter = encodingParameter(dataEncoding());
                    const auto outputParameter = encodingParameter(encoding);
                    encodedData = convertString(inputParameter.first, outputParameter.first, m_ptr.get(), m_size, outputParameter.second / inputParameter.second);
                }
                }
                result.assign(encodedData.first.get(), encodedData.second);
            }
            return;
        case TagDataType::Integer:
            result = ConversionUtilities::numberToString(toInteger());
            break;
        case TagDataType::PositionInSet:
            result = toPositionInSet().toString();
            break;
        case TagDataType::StandardGenreIndex:
            if(const char *genreName = Id3Genres::stringFromIndex(toInteger())) {
                result.assign(genreName);
                break;
            } else {
                throw ConversionException("No string representation for the assigned standard genre index available.");
            }
        case TagDataType::TimeSpan:
            result = toTimeSpan().toString();
            break;
        default:
            throw ConversionException("Can not convert binary data/picture to string.");
        }
        if(encoding == TagTextEncoding::Utf16LittleEndian || encoding == TagTextEncoding::Utf16BigEndian) {
            auto encodedData = encoding == TagTextEncoding::Utf16LittleEndian
                    ? convertUtf8ToUtf16LE(result.data(), result.size())
                    : convertUtf8ToUtf16BE(result.data(), result.size());
            result.assign(encodedData.first.get(), encodedData.second);
        }
    } else {
        result.clear();
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::u16string representation.
 * \throws Throws ConversionException on failure.
 * \remarks Use this only, if \a encoding is an UTF-16 encoding.
 * \sa toString()
 */
void TagValue::toWString(std::u16string &result, TagTextEncoding encoding) const
{
    if(!isEmpty()) {
        string regularStrRes;
        switch(m_type) {
        case TagDataType::Text:
            if(encoding == TagTextEncoding::Unspecified || encoding == dataEncoding()) {
                result.assign(reinterpret_cast<const char16_t *>(m_ptr.get()), m_size / sizeof(char16_t));
            } else {
                StringData encodedData;
                switch(encoding) {
                case TagTextEncoding::Utf8:
                    // use pre-defined methods when encoding to UTF-8
                    switch(dataEncoding()) {
                    case TagTextEncoding::Latin1:
                        encodedData = convertLatin1ToUtf8(m_ptr.get(), m_size);
                        break;
                    case TagTextEncoding::Utf16LittleEndian:
                        encodedData = convertUtf16LEToUtf8(m_ptr.get(), m_size);
                        break;
                    case TagTextEncoding::Utf16BigEndian:
                        encodedData = convertUtf16BEToUtf8(m_ptr.get(), m_size);
                        break;
                    default:
                        ;
                    }
                    break;
                default: {
                    // otherwise, determine input and output parameter to use general covertString method
                    const auto inputParameter = encodingParameter(dataEncoding());
                    const auto outputParameter = encodingParameter(encoding);
                    encodedData = convertString(inputParameter.first, outputParameter.first, m_ptr.get(), m_size, outputParameter.second / inputParameter.second);
                }
                }
                result.assign(reinterpret_cast<const char16_t *>(encodedData.first.get()), encodedData.second / sizeof(char16_t));
            }
            return;
        case TagDataType::Integer:
            regularStrRes = ConversionUtilities::numberToString(toInteger());
            break;
        case TagDataType::PositionInSet:
            regularStrRes = toPositionInSet().toString();
            break;
        case TagDataType::StandardGenreIndex:
            if(const char *genreName = Id3Genres::stringFromIndex(toInteger())) {
                regularStrRes.assign(genreName);
                break;
            } else {
                throw ConversionException("No string representation for the assigned standard genre index available.");
            }
        case TagDataType::TimeSpan:
            regularStrRes = toTimeSpan().toString();
            break;
        default:
            throw ConversionException("Can not convert binary data/picture to string.");
        }
        if(encoding == TagTextEncoding::Utf16LittleEndian || encoding == TagTextEncoding::Utf16BigEndian) {
            auto encodedData = encoding == TagTextEncoding::Utf16LittleEndian
                    ? convertUtf8ToUtf16LE(regularStrRes.data(), result.size())
                    : convertUtf8ToUtf16BE(regularStrRes.data(), result.size());
            result.assign(reinterpret_cast<const char16_t *>(encodedData.first.get()), encodedData.second / sizeof(const char16_t));
        }
    } else {
        result.clear();
    }
}

/*!
 * \brief Assigns a copy of the given \a text.
 * \param text Specifies the text to be assigned.
 * \param textSize Specifies the size of \a text. (The actual number of bytes, not the number of characters.)
 * \param textEncoding Specifies the encoding of the given \a text.
 * \param convertTo Specifies the encoding to convert \a text to; set to TagTextEncoding::Unspecified to
 *                  use \a textEncoding without any character set conversions.
 * \throws Throws a ConversionException if the conversion the specified character set fails.
 */
void TagValue::assignText(const char *text, std::size_t textSize, TagTextEncoding textEncoding, TagTextEncoding convertTo)
{
    m_type = TagDataType::Text;
    m_encoding = convertTo == TagTextEncoding::Unspecified ? textEncoding : convertTo;
    if(textSize) {
        if(convertTo == TagTextEncoding::Unspecified || textEncoding == convertTo) {
            m_ptr = make_unique<char []>(m_size = textSize);
            copy(text, text + textSize, m_ptr.get());
        } else {
            StringData encodedData;
            switch(textEncoding) {
            case TagTextEncoding::Utf8:
                // use pre-defined methods when encoding to UTF-8
                switch(convertTo) {
                case TagTextEncoding::Latin1:
                    encodedData = convertUtf8ToLatin1(text, textSize);
                    break;
                case TagTextEncoding::Utf16LittleEndian:
                    encodedData = convertUtf8ToUtf16LE(text, textSize);
                    break;
                case TagTextEncoding::Utf16BigEndian:
                    encodedData = convertUtf8ToUtf16BE(text, textSize);
                    break;
                default:
                    ;
                }
                break;
            default: {
                // otherwise, determine input and output parameter to use general covertString method
                const auto inputParameter = encodingParameter(textEncoding);
                const auto outputParameter = encodingParameter(convertTo);
                encodedData = convertString(inputParameter.first, outputParameter.first, text, textSize, outputParameter.second / inputParameter.second);
            }
            }
            // can't just move the encoded data because it needs to be deleted with free
            m_ptr = make_unique<char []>(m_size = encodedData.second);
            copy(encodedData.first.get(), encodedData.first.get() + encodedData.second, m_ptr.get());
        }
    } else {
        m_size = 0;
        m_ptr.reset();
    }
}

/*!
 * \brief Assigns the given integer \a value.
 * \param value Specifies the integer to be assigned.
 */
void TagValue::assignInteger(int value)
{
    m_size = sizeof(value);
    m_ptr = make_unique<char []>(m_size);
    std::copy(reinterpret_cast<const char *>(&value), reinterpret_cast<const char *>(&value) + m_size, m_ptr.get());
    m_type = TagDataType::Integer;
    m_encoding = TagTextEncoding::Latin1;
}

/*!
 * \brief Assigns the given standard genre \a index to be assigned.
 * \param index Specifies the index to be assigned.
 * \sa <a href="http://en.wikipedia.org/wiki/ID3#List_of_genres">List of genres - Wikipedia</a>
 */
void TagValue::assignStandardGenreIndex(int index)
{
    assignInteger(index);
    m_type = TagDataType::StandardGenreIndex;
}

/*!
 * \brief Assigns a copy of the given \a data.
 * \param data Specifies the data to be assigned.
 * \param length Specifies the length of the data.
 * \param type Specifies the type of the data as TagDataType.
 * \param encoding Specifies the encoding of the data as TagTextEncoding. The
 *                 encoding will only be considered if a text is assigned.
 */
void TagValue::assignData(const char *data, size_t length, TagDataType type, TagTextEncoding encoding)
{
    if(length > m_size) {
        m_ptr = make_unique<char[]>(length);
    }
    if(length) {
        std::copy(data, data + length, m_ptr.get());
    } else {
        m_ptr.reset();
    }
    m_size = length;
    m_type = type;
    m_encoding = encoding;
}

/*!
 * \brief Assigns the given \a data. Takes ownership.
 *
 * The specified data is not copied. It is moved.
 *
 * \param data Specifies the data to be assigned.
 * \param length Specifies the length of the data.
 * \param type Specifies the type of the data as TagDataType.
 * \param encoding Specifies the encoding of the data as TagTextEncoding. The
 *                 encoding will only be considered if a text is assigned.
 */
void TagValue::assignData(unique_ptr<char[]> &&data, size_t length, TagDataType type, TagTextEncoding encoding)
{
    m_size = length;
    m_type = type;
    m_encoding = encoding;
    m_ptr = move(data);
}

/*!
 * \brief Returns an empty TagValue.
 */
const TagValue &TagValue::empty()
{
    static TagValue emptyTagValue;
    return emptyTagValue;
}

}
