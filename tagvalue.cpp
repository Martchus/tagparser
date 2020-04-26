#include "./tagvalue.h"

#include "./caseinsensitivecomparer.h"
#include "./tag.h"

#include "./id3/id3genres.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/conversionexception.h>
#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>

#include <algorithm>
#include <cstring>
#include <utility>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \brief Returns the string representation of the specified \a dataType.
 */
const char *tagDataTypeString(TagDataType dataType)
{
    switch (dataType) {
    case TagDataType::Text:
        return "text";
    case TagDataType::Integer:
        return "integer";
    case TagDataType::PositionInSet:
        return "position in set";
    case TagDataType::StandardGenreIndex:
        return "genre index";
    case TagDataType::TimeSpan:
        return "time span";
    case TagDataType::DateTime:
        return "date time";
    case TagDataType::Picture:
        return "picture";
    case TagDataType::Binary:
        return "binary";
    default:
        return "undefined";
    }
}

/*!
 * \brief Returns the encoding parameter (name of the character set and bytes per character) for the specified \a tagTextEncoding.
 */
pair<const char *, float> encodingParameter(TagTextEncoding tagTextEncoding)
{
    switch (tagTextEncoding) {
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
 * \class TagParser::TagValue
 * \brief The TagValue class wraps values of different types. It is meant to be assigned to a tag field.
 *
 * For a list of supported types see TagParser::TagDataType.
 *
 * When constructing a TagValue choose the type which suites the value you want to store best. If the
 * tag format uses a different type the serializer will take care of the neccassary conversion (eg. convert
 * an integer to a string).
 *
 * When consuming a TagValue read from a tag one should not expect that a particular type is used. The
 * type depends on what the particular tag format uses. However, the conversion functions provided by the
 * TagValue class take care of neccassary conversions, eg. TagValue::toInteger() will attempt to convert a
 * string to a number (an possibly throw a ConversionException on failure).
 *
 * Values of the type TagDataType::Text can be differently encoded. See TagParser::TagTextEncoding for a
 * list of supported encodings. Be sure to use an encoding which is supported by the tag implementation.
 * To ensure that the functions Tag::canEncodingBeUsed(), Tag::proposedTextEncoding() and
 * Tag::ensureTextValuesAreProperlyEncoded() can be used.
 *
 * Values of the type TagDataType::Text are not supposed to contain Byte-Order-Marks. Before assigning text
 * which might be prepended by a Byte-Order-Mark the helper function TagValue::stripBom() can be used.
 */

/*!
 * \brief Constructs a new TagValue holding a copy of the given TagValue instance.
 * \param other Specifies another TagValue instance.
 */
TagValue::TagValue(const TagValue &other)
    : m_size(other.m_size)
    , m_desc(other.m_desc)
    , m_mimeType(other.m_mimeType)
    , m_language(other.m_language)
    , m_type(other.m_type)
    , m_encoding(other.m_encoding)
    , m_descEncoding(other.m_descEncoding)
    , m_labeledAsReadonly(other.m_labeledAsReadonly)
{
    if (!other.isEmpty()) {
        m_ptr = make_unique<char[]>(m_size);
        std::copy(other.m_ptr.get(), other.m_ptr.get() + other.m_size, m_ptr.get());
    }
}

/*!
 * \brief Assigns the value of another TagValue to the current instance.
 */
TagValue &TagValue::operator=(const TagValue &other)
{
    if (this == &other) {
        return *this;
    }
    m_size = other.m_size;
    m_type = other.m_type;
    m_desc = other.m_desc;
    m_mimeType = other.m_mimeType;
    m_language = other.m_language;
    m_labeledAsReadonly = other.m_labeledAsReadonly;
    m_encoding = other.m_encoding;
    m_descEncoding = other.m_descEncoding;
    if (other.isEmpty()) {
        m_ptr.reset();
    } else {
        m_ptr = make_unique<char[]>(m_size);
        std::copy(other.m_ptr.get(), other.m_ptr.get() + other.m_size, m_ptr.get());
    }
    return *this;
}

/// \cond
TagTextEncoding pickUtfEncoding(TagTextEncoding encoding1, TagTextEncoding encoding2)
{
    switch (encoding1) {
    case TagTextEncoding::Utf8:
    case TagTextEncoding::Utf16LittleEndian:
    case TagTextEncoding::Utf16BigEndian:
        return encoding1;
    default:
        switch (encoding2) {
        case TagTextEncoding::Utf8:
        case TagTextEncoding::Utf16LittleEndian:
        case TagTextEncoding::Utf16BigEndian:
            return encoding2;
        default:;
        }
    }
    return TagTextEncoding::Utf8;
}
/// \endcond

/*!
 * \brief Returns whether both instances are equal. Meta-data like description and MIME-type is taken into
 *        account as well.
 * \arg other Specifies the other instance.
 * \arg options Specifies options to alter the behavior. See TagValueComparisionFlags for details.
 * \remarks
 * - If the data types are not equal, two instances are still considered equal if the string representation
 *   is identical. For instance the text "2" is considered equal to the integer 2. This also means that an empty
 *   TagValue and the integer 0 are *not* considered equal.
 * - The choice to allow implicit conversions was made because different tag formats use different types and
 *   usually one does not care about those internals when comparing values.
 * - If any of the differently typed values can not be converted to a string (eg. it is binary data) the values
 *   are *not* considered equal. So the text "foo" and the binary value "foo" are not considered equal although
 *   the raw data is identical.
 * - In fact, values of the types TagDataType::DateTime, TagDataType::TimeSpan, TagDataType::Picture, TagDataType::Binary
 *   and TagDataType::Unspecified will never be considered equal with a value of another type.
 * - If the type is TagDataType::Text and the encoding differs values might still be considered equal if they
 *   represent the same characters. The same counts for the description.
 * - This might be a costly operation due to possible conversions.
 * \sa
 * - TagValue::compareData() to compare raw data without any conversions
 * - TagValueTests::testEqualityOperator() for examples
 */
bool TagValue::compareTo(const TagValue &other, TagValueComparisionFlags options) const
{
    // check whether meta-data is equal (except description)
    if (!(options & TagValueComparisionFlags::IgnoreMetaData)) {
        // check meta-data which always uses UTF-8 (everything but description)
        if (m_mimeType != other.m_mimeType || m_language != other.m_language || m_labeledAsReadonly != other.m_labeledAsReadonly) {
            return false;
        }

        // check description which might use different encodings
        if (m_descEncoding == other.m_descEncoding || m_descEncoding == TagTextEncoding::Unspecified
            || other.m_descEncoding == TagTextEncoding::Unspecified || m_desc.empty() || other.m_desc.empty()) {
            if (!compareData(m_desc, other.m_desc, options & TagValueComparisionFlags::CaseInsensitive)) {
                return false;
            }
        } else {
            const auto utfEncodingToUse = pickUtfEncoding(m_descEncoding, other.m_descEncoding);
            StringData str1, str2;
            const char *data1, *data2;
            size_t size1, size2;
            if (m_descEncoding != utfEncodingToUse) {
                const auto inputParameter = encodingParameter(m_descEncoding), outputParameter = encodingParameter(utfEncodingToUse);
                str1 = convertString(
                    inputParameter.first, outputParameter.first, m_desc.data(), m_desc.size(), outputParameter.second / inputParameter.second);
                data1 = str1.first.get();
                size1 = str1.second;
            } else {
                data1 = m_desc.data();
                size1 = m_desc.size();
            }
            if (other.m_descEncoding != utfEncodingToUse) {
                const auto inputParameter = encodingParameter(other.m_descEncoding), outputParameter = encodingParameter(utfEncodingToUse);
                str2 = convertString(inputParameter.first, outputParameter.first, other.m_desc.data(), other.m_desc.size(),
                    outputParameter.second / inputParameter.second);
                data2 = str2.first.get();
                size2 = str2.second;
            } else {
                data2 = other.m_desc.data();
                size2 = other.m_desc.size();
            }
            if (!compareData(data1, size1, data2, size2, options & TagValueComparisionFlags::CaseInsensitive)) {
                return false;
            }
        }
    }

    // check for equality if both types are identical
    if (m_type == other.m_type) {
        switch (m_type) {
        case TagDataType::Text: {
            // compare raw data directly if the encoding is the same
            if (m_size != other.m_size && m_encoding == other.m_encoding) {
                return false;
            }
            if (m_encoding == other.m_encoding || m_encoding == TagTextEncoding::Unspecified || other.m_encoding == TagTextEncoding::Unspecified) {
                return compareData(other, options & TagValueComparisionFlags::CaseInsensitive);
            }

            // compare UTF-8 or UTF-16 representation of strings avoiding unnecessary conversions
            const auto utfEncodingToUse = pickUtfEncoding(m_encoding, other.m_encoding);
            string str1, str2;
            const char *data1, *data2;
            size_t size1, size2;
            if (m_encoding != utfEncodingToUse) {
                str1 = toString(utfEncodingToUse);
                data1 = str1.data();
                size1 = str1.size();
            } else {
                data1 = m_ptr.get();
                size1 = m_size;
            }
            if (other.m_encoding != utfEncodingToUse) {
                str2 = other.toString(utfEncodingToUse);
                data2 = str2.data();
                size2 = str2.size();
            } else {
                data2 = other.m_ptr.get();
                size2 = other.m_size;
            }
            return compareData(data1, size1, data2, size2, options & TagValueComparisionFlags::CaseInsensitive);
        }
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
            return compareData(other);
        }
        return false;
    }

    // check for equality if types are different by comparing the string representation (if that makes sense)
    for (const auto dataType : { m_type, other.m_type }) {
        switch (dataType) {
        case TagDataType::TimeSpan:
        case TagDataType::DateTime:
        case TagDataType::Picture:
        case TagDataType::Binary:
        case TagDataType::Undefined:
            // do not attempt to convert these types to string because it will always fail anyways
            return false;
        default:;
        }
    }
    try {
        return compareData(toString(), other.toString(m_encoding), options & TagValueComparisionFlags::CaseInsensitive);
    } catch (const ConversionException &) {
        return false;
    }
}

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
    m_language.clear();
    m_labeledAsReadonly = false;
    m_encoding = TagTextEncoding::Latin1;
    m_descEncoding = TagTextEncoding::Latin1;
    m_type = TagDataType::Undefined;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        integer representation.
 * \throws Throws ConversionException on failure.
 */
std::int32_t TagValue::toInteger() const
{
    if (isEmpty()) {
        return 0;
    }
    switch (m_type) {
    case TagDataType::Text:
        switch (m_encoding) {
        case TagTextEncoding::Unspecified:
        case TagTextEncoding::Latin1:
        case TagTextEncoding::Utf8:
            return bufferToNumber<std::int32_t>(m_ptr.get(), m_size);
        case TagTextEncoding::Utf16LittleEndian:
        case TagTextEncoding::Utf16BigEndian:
            u16string u16str(reinterpret_cast<char16_t *>(m_ptr.get()), m_size / 2);
            ensureHostByteOrder(u16str, m_encoding);
            return stringToNumber<std::int32_t>(u16str);
        }
    case TagDataType::PositionInSet:
        if (m_size == sizeof(PositionInSet)) {
            return *reinterpret_cast<std::int32_t *>(m_ptr.get());
        }
        throw ConversionException("Can not convert assigned data to integer because the data size is not appropriate.");
    case TagDataType::Integer:
    case TagDataType::StandardGenreIndex:
        if (m_size == sizeof(std::int32_t)) {
            return *reinterpret_cast<std::int32_t *>(m_ptr.get());
        }
        throw ConversionException("Can not convert assigned data to integer because the data size is not appropriate.");
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to integer."));
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        standard genre index.
 * \throws Throws ConversionException on failure.
 */
int TagValue::toStandardGenreIndex() const
{
    if (isEmpty()) {
        return Id3Genres::emptyGenreIndex();
    }
    int index = 0;
    switch (m_type) {
    case TagDataType::Text: {
        try {
            index = toInteger();
        } catch (const ConversionException &) {
            TagTextEncoding encoding = TagTextEncoding::Utf8;
            if (m_encoding == TagTextEncoding::Latin1) {
                // no need to convert Latin-1 to UTF-8 (makes no difference in case of genre strings)
                encoding = TagTextEncoding::Unspecified;
            }
            index = Id3Genres::indexFromString(toString(encoding));
        }
        break;
    }
    case TagDataType::StandardGenreIndex:
    case TagDataType::Integer:
        if (m_size != sizeof(std::int32_t)) {
            throw ConversionException("The assigned index/integer is of unappropriate size.");
        }
        index = static_cast<int>(*reinterpret_cast<std::int32_t *>(m_ptr.get()));
        break;
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to genre index."));
    }
    if (!Id3Genres::isEmptyGenre(index) && !Id3Genres::isIndexSupported(index)) {
        throw ConversionException("The assigned number is not a valid standard genre index.");
    }
    return index;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        PositionInSet representation.
 * \throws Throws ConversionException on failure.
 */
PositionInSet TagValue::toPositionInSet() const
{
    if (isEmpty()) {
        return PositionInSet();
    }
    switch (m_type) {
    case TagDataType::Text:
        switch (m_encoding) {
        case TagTextEncoding::Unspecified:
        case TagTextEncoding::Latin1:
        case TagTextEncoding::Utf8:
            return PositionInSet(string(m_ptr.get(), m_size));
        case TagTextEncoding::Utf16LittleEndian:
        case TagTextEncoding::Utf16BigEndian:
            u16string u16str(reinterpret_cast<char16_t *>(m_ptr.get()), m_size / 2);
            ensureHostByteOrder(u16str, m_encoding);
            return PositionInSet(u16str);
        }
    case TagDataType::Integer:
    case TagDataType::PositionInSet:
        switch (m_size) {
        case sizeof(std::int32_t):
            return PositionInSet(*(reinterpret_cast<std::int32_t *>(m_ptr.get())));
        case 2 * sizeof(std::int32_t):
            return PositionInSet(
                *(reinterpret_cast<std::int32_t *>(m_ptr.get())), *(reinterpret_cast<std::int32_t *>(m_ptr.get() + sizeof(std::int32_t))));
        default:
            throw ConversionException("The size of the assigned data is not appropriate.");
        }
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to position in set."));
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        TimeSpan representation.
 * \throws Throws ConversionException on failure.
 */
TimeSpan TagValue::toTimeSpan() const
{
    if (isEmpty()) {
        return TimeSpan();
    }
    switch (m_type) {
    case TagDataType::Text:
        return TimeSpan::fromString(toString(m_encoding == TagTextEncoding::Utf8 ? TagTextEncoding::Utf8 : TagTextEncoding::Latin1));
    case TagDataType::Integer:
    case TagDataType::TimeSpan:
        switch (m_size) {
        case sizeof(std::int32_t):
            return TimeSpan(*(reinterpret_cast<std::int32_t *>(m_ptr.get())));
        case sizeof(std::int64_t):
            return TimeSpan(*(reinterpret_cast<std::int64_t *>(m_ptr.get())));
        default:
            throw ConversionException("The size of the assigned integer is not appropriate for conversion to time span.");
        }
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to time span."));
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        DateTime representation.
 * \throws Throws ConversionException on failure.
 */
DateTime TagValue::toDateTime() const
{
    if (isEmpty()) {
        return DateTime();
    }
    switch (m_type) {
    case TagDataType::Text: {
        const auto str = toString(m_encoding == TagTextEncoding::Utf8 ? TagTextEncoding::Utf8 : TagTextEncoding::Latin1);
        try {
            return DateTime::fromIsoStringGmt(str.data());
        } catch (const ConversionException &) {
            return DateTime::fromString(str);
        }
    }
    case TagDataType::Integer:
    case TagDataType::DateTime:
        if (m_size == sizeof(std::int32_t)) {
            return DateTime(*(reinterpret_cast<std::uint32_t *>(m_ptr.get())));
        } else if (m_size == sizeof(std::int64_t)) {
            return DateTime(*(reinterpret_cast<std::uint64_t *>(m_ptr.get())));
        } else {
            throw ConversionException("The size of the assigned integer is not appropriate for conversion to date time.");
        }
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to date time."));
    }
}

/*!
 * \brief Converts the currently assigned text value to the specified \a encoding.
 * \throws Throws CppUtilities::ConversionException() if the conversion fails.
 * \remarks
 * - Does nothing if dataEncoding() equals \a encoding.
 * - Sets dataEncoding() to the specified \a encoding if the conversion succeeds.
 * - Does not do any conversion if the current type() is not TagDataType::Text.
 * \sa convertDataEncodingForTag()
 */
void TagValue::convertDataEncoding(TagTextEncoding encoding)
{
    if (m_encoding == encoding) {
        return;
    }
    if (type() == TagDataType::Text) {
        StringData encodedData;
        switch (encoding) {
        case TagTextEncoding::Utf8:
            // use pre-defined methods when encoding to UTF-8
            switch (dataEncoding()) {
            case TagTextEncoding::Latin1:
                encodedData = convertLatin1ToUtf8(m_ptr.get(), m_size);
                break;
            case TagTextEncoding::Utf16LittleEndian:
                encodedData = convertUtf16LEToUtf8(m_ptr.get(), m_size);
                break;
            case TagTextEncoding::Utf16BigEndian:
                encodedData = convertUtf16BEToUtf8(m_ptr.get(), m_size);
                break;
            default:;
            }
            break;
        default: {
            // otherwise, determine input and output parameter to use general covertString method
            const auto inputParameter = encodingParameter(dataEncoding());
            const auto outputParameter = encodingParameter(encoding);
            encodedData
                = convertString(inputParameter.first, outputParameter.first, m_ptr.get(), m_size, outputParameter.second / inputParameter.second);
        }
        }
        // can't just move the encoded data because it needs to be deleted with free
        m_ptr = make_unique<char[]>(m_size = encodedData.second);
        copy(encodedData.first.get(), encodedData.first.get() + encodedData.second, m_ptr.get());
    }
    m_encoding = encoding;
}

/*!
 * \brief Ensures the encoding of the currently assigned text value is supported by the specified \a tag.
 * \sa This is a convenience method for convertDataEncoding().
 */
void TagValue::convertDataEncodingForTag(const Tag *tag)
{
    if (type() == TagDataType::Text && !tag->canEncodingBeUsed(dataEncoding())) {
        convertDataEncoding(tag->proposedTextEncoding());
    }
}

/*!
 * \brief Converts the assigned description to use the specified \a encoding.
 */
void TagValue::convertDescriptionEncoding(TagTextEncoding encoding)
{
    if (encoding == m_descEncoding) {
        return;
    }
    if (m_desc.empty()) {
        m_descEncoding = encoding;
        return;
    }
    StringData encodedData;
    switch (encoding) {
    case TagTextEncoding::Utf8:
        // use pre-defined methods when encoding to UTF-8
        switch (dataEncoding()) {
        case TagTextEncoding::Latin1:
            encodedData = convertLatin1ToUtf8(m_ptr.get(), m_size);
            break;
        case TagTextEncoding::Utf16LittleEndian:
            encodedData = convertUtf16LEToUtf8(m_ptr.get(), m_size);
            break;
        case TagTextEncoding::Utf16BigEndian:
            encodedData = convertUtf16BEToUtf8(m_ptr.get(), m_size);
            break;
        default:;
        }
        break;
    default: {
        // otherwise, determine input and output parameter to use general covertString method
        const auto inputParameter = encodingParameter(m_descEncoding);
        const auto outputParameter = encodingParameter(encoding);
        encodedData = convertString(
            inputParameter.first, outputParameter.first, m_desc.data(), m_desc.size(), outputParameter.second / inputParameter.second);
    }
    }
    m_desc.assign(encodedData.first.get(), encodedData.second);
    m_descEncoding = encoding;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::string representation.
 * \param result Specifies the string to store the result.
 * \param encoding Specifies the encoding to to be used; set to TagTextEncoding::Unspecified to use the
 *        present encoding without any character set conversion.
 * \remarks
 * - Not all types can be converted to a string, eg. TagDataType::Picture, TagDataType::Binary and
 *   TagDataType::Unspecified will always fail to convert.
 * - If UTF-16 is the desired output \a encoding, it makes sense to use the toWString() method instead.
 * \throws Throws ConversionException on failure.
 */
void TagValue::toString(string &result, TagTextEncoding encoding) const
{
    if (isEmpty()) {
        result.clear();
        return;
    }

    switch (m_type) {
    case TagDataType::Text:
        if (encoding == TagTextEncoding::Unspecified || dataEncoding() == TagTextEncoding::Unspecified || encoding == dataEncoding()) {
            result.assign(m_ptr.get(), m_size);
        } else {
            StringData encodedData;
            switch (encoding) {
            case TagTextEncoding::Utf8:
                // use pre-defined methods when encoding to UTF-8
                switch (dataEncoding()) {
                case TagTextEncoding::Latin1:
                    encodedData = convertLatin1ToUtf8(m_ptr.get(), m_size);
                    break;
                case TagTextEncoding::Utf16LittleEndian:
                    encodedData = convertUtf16LEToUtf8(m_ptr.get(), m_size);
                    break;
                case TagTextEncoding::Utf16BigEndian:
                    encodedData = convertUtf16BEToUtf8(m_ptr.get(), m_size);
                    break;
                default:;
                }
                break;
            default: {
                // otherwise, determine input and output parameter to use general covertString method
                const auto inputParameter = encodingParameter(dataEncoding());
                const auto outputParameter = encodingParameter(encoding);
                encodedData
                    = convertString(inputParameter.first, outputParameter.first, m_ptr.get(), m_size, outputParameter.second / inputParameter.second);
            }
            }
            result.assign(encodedData.first.get(), encodedData.second);
        }
        return;
    case TagDataType::Integer:
        result = numberToString(toInteger());
        break;
    case TagDataType::PositionInSet:
        result = toPositionInSet().toString();
        break;
    case TagDataType::StandardGenreIndex: {
        const auto genreIndex = toInteger();
        if (Id3Genres::isEmptyGenre(genreIndex)) {
            result.clear();
        } else if (const char *genreName = Id3Genres::stringFromIndex(genreIndex)) {
            result.assign(genreName);
        } else {
            throw ConversionException("No string representation for the assigned standard genre index available.");
        }
        break;
    }
    case TagDataType::TimeSpan:
        result = toTimeSpan().toString();
        break;
    case TagDataType::DateTime:
        result = toDateTime().toString(DateTimeOutputFormat::IsoOmittingDefaultComponents);
        break;
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to string."));
    }
    if (encoding == TagTextEncoding::Utf16LittleEndian || encoding == TagTextEncoding::Utf16BigEndian) {
        auto encodedData = encoding == TagTextEncoding::Utf16LittleEndian ? convertUtf8ToUtf16LE(result.data(), result.size())
                                                                          : convertUtf8ToUtf16BE(result.data(), result.size());
        result.assign(encodedData.first.get(), encodedData.second);
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::u16string representation.
 * \throws Throws ConversionException on failure.
 * \remarks
 * - Not all types can be converted to a string, eg. TagDataType::Picture, TagDataType::Binary and
 *   TagDataType::Unspecified will always fail to convert.
 * - Use this only, if \a encoding is an UTF-16 encoding.
 * \sa toString()
 */
void TagValue::toWString(std::u16string &result, TagTextEncoding encoding) const
{
    if (isEmpty()) {
        result.clear();
        return;
    }

    string regularStrRes;
    switch (m_type) {
    case TagDataType::Text:
        if (encoding == TagTextEncoding::Unspecified || encoding == dataEncoding()) {
            result.assign(reinterpret_cast<const char16_t *>(m_ptr.get()), m_size / sizeof(char16_t));
        } else {
            StringData encodedData;
            switch (encoding) {
            case TagTextEncoding::Utf8:
                // use pre-defined methods when encoding to UTF-8
                switch (dataEncoding()) {
                case TagTextEncoding::Latin1:
                    encodedData = convertLatin1ToUtf8(m_ptr.get(), m_size);
                    break;
                case TagTextEncoding::Utf16LittleEndian:
                    encodedData = convertUtf16LEToUtf8(m_ptr.get(), m_size);
                    break;
                case TagTextEncoding::Utf16BigEndian:
                    encodedData = convertUtf16BEToUtf8(m_ptr.get(), m_size);
                    break;
                default:;
                }
                break;
            default: {
                // otherwise, determine input and output parameter to use general covertString method
                const auto inputParameter = encodingParameter(dataEncoding());
                const auto outputParameter = encodingParameter(encoding);
                encodedData
                    = convertString(inputParameter.first, outputParameter.first, m_ptr.get(), m_size, outputParameter.second / inputParameter.second);
            }
            }
            result.assign(reinterpret_cast<const char16_t *>(encodedData.first.get()), encodedData.second / sizeof(char16_t));
        }
        return;
    case TagDataType::Integer:
        regularStrRes = numberToString(toInteger());
        break;
    case TagDataType::PositionInSet:
        regularStrRes = toPositionInSet().toString();
        break;
    case TagDataType::StandardGenreIndex: {
        const auto genreIndex = toInteger();
        if (Id3Genres::isEmptyGenre(genreIndex)) {
            regularStrRes.clear();
        } else if (const char *genreName = Id3Genres::stringFromIndex(genreIndex)) {
            regularStrRes.assign(genreName);
        } else {
            throw ConversionException("No string representation for the assigned standard genre index available.");
        }
        break;
    }
    case TagDataType::TimeSpan:
        regularStrRes = toTimeSpan().toString();
        break;
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to string."));
    }
    if (encoding == TagTextEncoding::Utf16LittleEndian || encoding == TagTextEncoding::Utf16BigEndian) {
        auto encodedData = encoding == TagTextEncoding::Utf16LittleEndian ? convertUtf8ToUtf16LE(regularStrRes.data(), result.size())
                                                                          : convertUtf8ToUtf16BE(regularStrRes.data(), result.size());
        result.assign(reinterpret_cast<const char16_t *>(encodedData.first.get()), encodedData.second / sizeof(const char16_t));
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
 * \remarks Strips the BOM of the specified \a text.
 */
void TagValue::assignText(const char *text, std::size_t textSize, TagTextEncoding textEncoding, TagTextEncoding convertTo)
{
    m_type = TagDataType::Text;
    m_encoding = convertTo == TagTextEncoding::Unspecified ? textEncoding : convertTo;

    stripBom(text, textSize, textEncoding);
    if (!textSize) {
        m_size = 0;
        m_ptr.reset();
        return;
    }

    if (convertTo == TagTextEncoding::Unspecified || textEncoding == convertTo) {
        m_ptr = make_unique<char[]>(m_size = textSize);
        copy(text, text + textSize, m_ptr.get());
        return;
    }

    StringData encodedData;
    switch (textEncoding) {
    case TagTextEncoding::Utf8:
        // use pre-defined methods when encoding to UTF-8
        switch (convertTo) {
        case TagTextEncoding::Latin1:
            encodedData = convertUtf8ToLatin1(text, textSize);
            break;
        case TagTextEncoding::Utf16LittleEndian:
            encodedData = convertUtf8ToUtf16LE(text, textSize);
            break;
        case TagTextEncoding::Utf16BigEndian:
            encodedData = convertUtf8ToUtf16BE(text, textSize);
            break;
        default:;
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
    m_ptr = make_unique<char[]>(m_size = encodedData.second);
    copy(encodedData.first.get(), encodedData.first.get() + encodedData.second, m_ptr.get());
}

/*!
 * \brief Assigns the given integer \a value.
 * \param value Specifies the integer to be assigned.
 */
void TagValue::assignInteger(int value)
{
    m_size = sizeof(value);
    m_ptr = make_unique<char[]>(m_size);
    std::copy(reinterpret_cast<const char *>(&value), reinterpret_cast<const char *>(&value) + m_size, m_ptr.get());
    m_type = TagDataType::Integer;
    m_encoding = TagTextEncoding::Latin1;
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
    if (type == TagDataType::Text) {
        stripBom(data, length, encoding);
    }
    if (length > m_size) {
        m_ptr = make_unique<char[]>(length);
    }
    if (length) {
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
 * \remarks Does not strip the BOM so for consistency the caller must ensure there is no BOM present.
 */
void TagValue::assignData(unique_ptr<char[]> &&data, size_t length, TagDataType type, TagTextEncoding encoding)
{
    m_size = length;
    m_type = type;
    m_encoding = encoding;
    m_ptr = move(data);
}

/*!
 * \brief Strips the byte order mask from the specified \a text.
 */
void TagValue::stripBom(const char *&text, size_t &length, TagTextEncoding encoding)
{
    switch (encoding) {
    case TagTextEncoding::Utf8:
        if ((length >= 3) && (BE::toUInt24(text) == 0x00EFBBBF)) {
            text += 3;
            length -= 3;
        }
        break;
    case TagTextEncoding::Utf16LittleEndian:
        if ((length >= 2) && (LE::toUInt16(text) == 0xFEFF)) {
            text += 2;
            length -= 2;
        }
        break;
    case TagTextEncoding::Utf16BigEndian:
        if ((length >= 2) && (BE::toUInt16(text) == 0xFEFF)) {
            text += 2;
            length -= 2;
        }
        break;
    default:;
    }
}

/*!
 * \brief Ensures the byte-order of the specified UTF-16 string matches the byte-order of the machine.
 * \remarks Does nothing if \a currentEncoding already matches the byte-order of the machine.
 */
void TagValue::ensureHostByteOrder(u16string &u16str, TagTextEncoding currentEncoding)
{
    if (currentEncoding !=
#if defined(CONVERSION_UTILITIES_BYTE_ORDER_LITTLE_ENDIAN)
        TagTextEncoding::Utf16LittleEndian
#elif defined(CONVERSION_UTILITIES_BYTE_ORDER_BIG_ENDIAN)
        TagTextEncoding::Utf16BigEndian
#else
#error "Host byte order not supported"
#endif
    ) {
        for (auto &c : u16str) {
            c = swapOrder(static_cast<std::uint16_t>(c));
        }
    }
}

/*!
 * \brief Returns whether 2 data buffers are equal. In case one of the sizes is zero, no pointer is dereferenced.
 */
bool TagValue::compareData(const char *data1, std::size_t size1, const char *data2, std::size_t size2, bool ignoreCase)
{
    if (size1 != size2) {
        return false;
    }
    if (!size1) {
        return true;
    }
    if (ignoreCase) {
        for (auto i1 = data1, i2 = data2, end = data1 + size1; i1 != end; ++i1, ++i2) {
            if (CaseInsensitiveCharComparer::toLower(static_cast<unsigned char>(*i1))
                != CaseInsensitiveCharComparer::toLower(static_cast<unsigned char>(*i2))) {
                return false;
            }
        }
    } else {
        for (auto i1 = data1, i2 = data2, end = data1 + size1; i1 != end; ++i1, ++i2) {
            if (*i1 != *i2) {
                return false;
            }
        }
    }
    return true;
}

/*!
 * \brief Returns a default-constructed TagValue where TagValue::isNull() and TagValue::isEmpty() both return true.
 * \remarks This is useful if one wants to return a const reference to a TagValue and a null-value is needed to indicate
 *          that the field does not exist at all.
 */
const TagValue &TagValue::empty()
{
    static TagValue emptyTagValue;
    return emptyTagValue;
}

} // namespace TagParser
