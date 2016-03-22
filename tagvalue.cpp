#include "./tagvalue.h"

#include "./id3/id3genres.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/conversion/conversionexception.h>
#include <c++utilities/misc/memory.h>

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
 * \brief Constructs an empty TagValue.
 */
TagValue::TagValue() :
    m_size(0),
    m_type(TagDataType::Undefined),
    m_labeledAsReadonly(false),
    m_encoding(TagTextEncoding::Latin1),
    m_descEncoding(TagTextEncoding::Latin1)
{}

/*!
 * \brief Constructs a new TagValue holding a copy of the given \a text.
 * \param text Specifies the text.
 * \param encoding Specifies the encoding of the given string as TagTextEncoding.
 */
TagValue::TagValue(const string &text, TagTextEncoding encoding) :
    m_size(text.size()),
    m_type(TagDataType::Text),
    m_labeledAsReadonly(false),
    m_encoding(encoding),
    m_descEncoding(TagTextEncoding::Latin1)
{
    if(m_size) {
        m_ptr = make_unique<char []>(m_size);
        text.copy(m_ptr.get(), m_size);
    }
}

/*!
 * \brief Constructs a new TagValue holding the given integer \a value.
 */
TagValue::TagValue(int value) :
    TagValue(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::Integer)
{}

/*!
 * \brief Constructs a new TagValue with a copy of the given \a data.
 *
 * \param data Specifies a pointer to the data.
 * \param length Specifies the length of the data.
 * \param type Specifies the type of the data as TagDataType.
 * \param encoding Specifies the encoding of the data as TagTextEncoding. The
 *                 encoding will only be considered if a text is assigned.
 */
TagValue::TagValue(const char *data, size_t length, TagDataType type, TagTextEncoding encoding) :
    m_size(length),
    m_type(type),
    m_labeledAsReadonly(false),
    m_encoding(encoding),
    m_descEncoding(TagTextEncoding::Latin1)
{
    if(length) {
        m_ptr = make_unique<char []>(m_size);
        std::copy(data, data + length, m_ptr.get());
    }
}

/*!
 * \brief Constructs a new TagValue holding with the given \a data.
 *
 * The data is not copied. It is moved.
 *
 * \param data Specifies a pointer to the data.
 * \param length Specifies the length of the data.
 * \param type Specifies the type of the data as TagDataType.
 * \param encoding Specifies the encoding of the data as TagTextEncoding. The
 *                 encoding will only be considered if a text is assigned.
 */
TagValue::TagValue(std::unique_ptr<char[]> &&data, size_t length, TagDataType type, TagTextEncoding encoding) :
    m_size(length),
    m_type(type),
    m_labeledAsReadonly(false),
    m_encoding(encoding),
    m_descEncoding(TagTextEncoding::Latin1)
{
    if(length) {
        m_ptr = move(data);
    }
}

/*!
 * \brief Constructs a new TagValue holding a copy of the given PositionInSet \a value.
 * \param value Specifies the PositionInSet.
 */
TagValue::TagValue(const PositionInSet &value) :
    TagValue(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::PositionInSet)
{}

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
 * Both instances are only considered equal, if the data type, encodings (if relevant for the type) and meta data are equal.
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
 * \throws Throws ConversionException an failure.
 */
int32 TagValue::toInteger() const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            return ConversionUtilities::stringToNumber<int32>(string(m_ptr.get(), m_size));
        case TagDataType::Integer:
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
 * \throws Throws ConversionException an failure.
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
 * \throws Throws ConversionException an failure.
 */
PositionInSet TagValue::toPositionInSet() const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            return PositionInSet(string(m_ptr.get(), m_size));
        case TagDataType::Integer:
        case TagDataType::PositionInSet:
            switch(m_size) {
            case sizeof(int32):
                return PositionInSet(*(reinterpret_cast<int *>(m_ptr.get())));
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
 * \throws Throws ConversionException an failure.
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
 * \throws Throws ConversionException an failure.
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
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::string representation.
 * \throws Throws ConversionException an failure.
 */
string TagValue::toString() const
{
    string res;
    toString(res);
    return res;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::string representation.
 * \throws Throws ConversionException on failure.
 */
void TagValue::toString(string &result) const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            result.assign(m_ptr.get(), m_size);
            return;
        case TagDataType::Integer:
            result = ConversionUtilities::numberToString(toInteger());
            return;
        case TagDataType::PositionInSet:
            result = toPositionInSet().toString();
            return;
        case TagDataType::StandardGenreIndex:
            if(const char *genreName = Id3Genres::stringFromIndex(toInteger())) {
                result.assign(genreName);
                return;
            } else {
                throw ConversionException("No string representation for the assigned standard genre index available.");
            }
            break;
        case TagDataType::TimeSpan:
            result = toTimeSpan().toString();
            return;
        default:
            throw ConversionException("Can not convert binary data/picture to string.");
        }
    }
    result.clear();
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::wstring representation.
 * \throws Throws ConversionException on failure.
 * \remarks Use this only, if UTF-16 text is assigned.
 */
u16string TagValue::toWString() const
{
    u16string res;
    toWString(res);
    return res;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::u16string representation.
 * \throws Throws ConversionException on failure.
 * \remarks Use this only, if UTF-16 text is assigned.
 */
void TagValue::toWString(u16string &result) const
{
    if(!isEmpty()) {
        switch(m_type) {
        case TagDataType::Text:
            result.assign(reinterpret_cast<typename u16string::value_type *>(m_ptr.get()), m_size / sizeof(typename u16string::value_type));
            return;
        case TagDataType::Integer:
            result = ConversionUtilities::numberToString<int32, u16string>(toInteger());
            return;
        case TagDataType::PositionInSet:
            result = toPositionInSet().toString<u16string>();
            return;
        case TagDataType::StandardGenreIndex:
            if(const char *genreName = Id3Genres::stringFromIndex(toInteger())) {
                // TODO: implement this
                throw ConversionException("Wide default genre strings are currently not supported.");
            } else {
                throw ConversionException("No string representation for the assigned standard genre index available.");
            }
            break;
        case TagDataType::TimeSpan:
            // TODO: implement this
            throw ConversionException("Wide time span string representations are currently not supported.");
        default:
            throw ConversionException("Can not convert binary data/picture to string.");
        }
    }
    result.clear();
}

/*!
 * \brief Assigns a copy of the given \a text.
 * \param text Specifies the text to be assigned.
 * \param encoding Specifies the encoding of the given \a text.
 */
void TagValue::assignText(const string &text, TagTextEncoding encoding)
{
    m_size = text.length();
    m_type = TagDataType::Text;
    m_encoding = encoding;
    if(m_size > 0) {
        m_ptr = make_unique<char []>(m_size);
        text.copy(m_ptr.get(), m_size);
    } else {
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
