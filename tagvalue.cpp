#include "./tagvalue.h"

#include "./caseinsensitivecomparer.h"
#include "./tag.h"

#include "./id3/id3genres.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/conversionexception.h>
#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <algorithm>
#include <cstring>
#include <sstream>
#include <utility>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/// \brief The TagValuePrivate struct contains private fields of the TagValue class.
struct TagValuePrivate {};

/*!
 * \brief Returns the string representation of the specified \a dataType.
 */
std::string_view tagDataTypeString(TagDataType dataType)
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
    case TagDataType::Popularity:
        return "popularity";
    case TagDataType::UnsignedInteger:
        return "unsigned integer";
    case TagDataType::DateTimeExpression:
        return "date time expression";
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
 * \class TagParser::Popularity
 * \brief The Popularity class contains a value for ID3v2's "Popularimeter" field.
 * \remarks It can also be used for other formats than ID3v2.
 * \sa See documentation of TagParser::Popularity for scaling.
 */

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
 * Values of the type TagDataType::Text can be differently encoded.
 * - See TagParser::TagTextEncoding for a list of encodings supported by this library.
 * - Tag formats usually only support a subset of these encodings. The serializers for the various tag
 *   formats provided by this library will keep the encoding if possible and otherwise convert the assigned
 *   text to an encoding supported by the tag format on the fly. Note that ID3v1 does not specify which
 *   encodings are supported (or unsupported) so the serializer will just write text data as-is.
 * - The deserializers will store text data in the encoding that is used in the tag.
 * - The functions Tag::canEncodingBeUsed() and Tag::proposedTextEncoding() can be used to check
 *   whether an encoding can be used by a certain tag format to avoid any unnecessary character set
 *   conversions.
 * - There's also the function Tag::ensureTextValuesAreProperlyEncoded() which can be used to convert all
 *   text values currently assigned to a tag to the encoding which is deemed best for the current tag format.
 *   This function is a bit more agressive than the implict conversions, e.g. it ensures no UTF-16 encoded
 *   text ends up in ID3v1 tags.
 * - If you want to use UTF-8 everywhere, simply always assign UTF-8 text and use
 *   TagValue::toString(TagTextEncoding::Utf8) when reading text.
 *
 * Values of the type TagDataType::Popularity might use different rating scales depending on the tag
 * format.
 * - You can assign a Popularity object of any scale. Tag implementations will convert it accordingly.
 * - You can use TagValue::toScaledPopularity() to retrieve a Popularity object of the desired scale.
 * - When just working with text data (via TagValue::toString() and TagValue::assignText()), no scaling
 *   of internally assigned Popularity objects is done; so you're working with raw rating values in this
 *   case.
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
    , m_locale(other.m_locale)
    , m_type(other.m_type)
    , m_encoding(other.m_encoding)
    , m_descEncoding(other.m_descEncoding)
    , m_flags(TagValueFlags::None)
{
    if (!other.isEmpty()) {
        m_ptr = make_unique<char[]>(m_size);
        std::copy(other.m_ptr.get(), other.m_ptr.get() + other.m_size, m_ptr.get());
    }
}

TagValue::TagValue(TagValue &&other) = default;

/*!
 * \brief Constructs an empty TagValue.
 */
TagValue::TagValue()
    : m_size(0)
    , m_type(TagDataType::Undefined)
    , m_encoding(TagTextEncoding::Latin1)
    , m_descEncoding(TagTextEncoding::Latin1)
    , m_flags(TagValueFlags::None)
{
}

/*!
 * \brief Constructs a new TagValue holding a copy of the given \a text.
 * \param text Specifies the text to be assigned.
 * \param textSize Specifies the size of \a text. (The actual number of bytes, not the number of characters.)
 * \param textEncoding Specifies the encoding of the given \a text.
 * \param convertTo Specifies the encoding to convert \a text to; set to TagTextEncoding::Unspecified to
 *                  use \a textEncoding without any character set conversions.
 * \throws Throws a ConversionException if the conversion the specified character set fails.
 * \remarks Strips the BOM of the specified \a text.
 */
TagValue::TagValue(const char *text, std::size_t textSize, TagTextEncoding textEncoding, TagTextEncoding convertTo)
    : m_descEncoding(TagTextEncoding::Latin1)
    , m_flags(TagValueFlags::None)
{
    assignText(text, textSize, textEncoding, convertTo);
}

/*!
 * \brief Constructs a new TagValue holding a copy of the given \a text.
 * \param text Specifies the text to be assigned. This string must be null-terminated.
 * \param textEncoding Specifies the encoding of the given \a text.
 * \param convertTo Specifies the encoding to convert \a text to; set to TagTextEncoding::Unspecified to
 *                  use \a textEncoding without any character set conversions.
 * \throws Throws a ConversionException if the conversion the specified character set fails.
 * \remarks Strips the BOM of the specified \a text.
 */
TagValue::TagValue(const char *text, TagTextEncoding textEncoding, TagTextEncoding convertTo)
{
    assignText(text, std::strlen(text), textEncoding, convertTo);
}

/*!
 * \brief Constructs a new TagValue holding a copy of the given \a text.
 * \param text Specifies the text to be assigned.
 * \param textEncoding Specifies the encoding of the given \a text.
 * \param convertTo Specifies the encoding to convert \a text to; set to TagTextEncoding::Unspecified to
 *                  use \a textEncoding without any character set conversions.
 * \throws Throws a ConversionException if the conversion the specified character set fails.
 * \remarks Strips the BOM of the specified \a text.
 */
TagValue::TagValue(const std::string &text, TagTextEncoding textEncoding, TagTextEncoding convertTo)
    : m_descEncoding(TagTextEncoding::Latin1)
    , m_flags(TagValueFlags::None)
{
    assignText(text, textEncoding, convertTo);
}

/*!
 * \brief Constructs a new TagValue holding a copy of the given \a text.
 * \param text Specifies the text to be assigned.
 * \param textEncoding Specifies the encoding of the given \a text.
 * \param convertTo Specifies the encoding to convert \a text to; set to TagTextEncoding::Unspecified to
 *                  use \a textEncoding without any character set conversions.
 * \throws Throws a ConversionException if the conversion the specified character set fails.
 * \remarks Strips the BOM of the specified \a text.
 */
TagValue::TagValue(std::string_view text, TagTextEncoding textEncoding, TagTextEncoding convertTo)
    : m_descEncoding(TagTextEncoding::Latin1)
    , m_flags(TagValueFlags::None)
{
    assignText(text, textEncoding, convertTo);
}

/*!
 * \brief Destroys the TagValue.
 */
TagValue::~TagValue()
{
}

/*!
 * \brief Constructs a new TagValue with a copy of the given \a data.
 *
 * \param data Specifies a pointer to the data.
 * \param length Specifies the length of the data.
 * \param type Specifies the type of the data as TagDataType.
 * \param encoding Specifies the encoding of the data as TagTextEncoding. The
 *                 encoding will only be considered if a text is assigned.
 * \remarks Strips the BOM of the specified \a data if \a type is TagDataType::Text.
 */
TagValue::TagValue(const char *data, std::size_t length, TagDataType type, TagTextEncoding encoding)
    : m_size(length)
    , m_type(type)
    , m_encoding(encoding)
    , m_descEncoding(TagTextEncoding::Latin1)
    , m_flags(TagValueFlags::None)
{
    if (length) {
        if (type == TagDataType::Text) {
            stripBom(data, m_size, encoding);
        }
        m_ptr = std::make_unique<char[]>(m_size);
        std::copy(data, data + m_size, m_ptr.get());
    }
}

/*!
 * \brief Constructs a new TagValue holding with the given \a data.
 *
 * The \a data is not copied. It is moved.
 *
 * \param data Specifies a pointer to the data.
 * \param length Specifies the length of the data.
 * \param type Specifies the type of the data as TagDataType.
 * \param encoding Specifies the encoding of the data as TagTextEncoding. The
 *                 encoding will only be considered if a text is assigned.
 * \remarks Does not strip the BOM so for consistency the caller must ensure there is no BOM present.
 */
TagValue::TagValue(std::unique_ptr<char[]> &&data, std::size_t length, TagDataType type, TagTextEncoding encoding)
    : m_size(length)
    , m_type(type)
    , m_encoding(encoding)
    , m_descEncoding(TagTextEncoding::Latin1)
    , m_flags(TagValueFlags::None)
{
    if (length) {
        m_ptr = std::move(data);
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
    m_locale = other.m_locale;
    m_flags = other.m_flags;
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

TagValue &TagValue::operator=(TagValue &&other) = default;

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
        if (m_mimeType != other.m_mimeType || m_locale != other.m_locale || m_flags != other.m_flags) {
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

    try {
        // check for equality if both types are identical
        if (m_type == other.m_type) {
            switch (m_type) {
            case TagDataType::Text: {
                // compare raw data directly if the encoding is the same
                if (m_size != other.m_size && m_encoding == other.m_encoding) {
                    return false;
                }
                if (m_encoding == other.m_encoding || m_encoding == TagTextEncoding::Unspecified
                    || other.m_encoding == TagTextEncoding::Unspecified) {
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
            case TagDataType::StandardGenreIndex:
                return toStandardGenreIndex() == other.toStandardGenreIndex();
            case TagDataType::TimeSpan:
                return toTimeSpan() == other.toTimeSpan();
            case TagDataType::DateTime:
                return toDateTime() == other.toDateTime();
            case TagDataType::DateTimeExpression:
                return toDateTimeExpression() == other.toDateTimeExpression();
            case TagDataType::Picture:
            case TagDataType::Binary:
            case TagDataType::Undefined:
                return compareData(other);
            default:;
            }
        }

        // do not attempt implicit conversions for certain types
        // TODO: Maybe it would actually make sense for some of these types (at least when the other type is
        //       string)?
        for (const auto dataType : { m_type, other.m_type }) {
            switch (dataType) {
            case TagDataType::TimeSpan:
            case TagDataType::DateTime:
            case TagDataType::DateTimeExpression:
            case TagDataType::Picture:
            case TagDataType::Binary:
            case TagDataType::Undefined:
                return false;
            default:;
            }
        }

        // handle types where an implicit conversion to the specific type can be done
        if (m_type == TagDataType::Integer || other.m_type == TagDataType::Integer) {
            return toInteger() == other.toInteger();
        } else if (m_type == TagDataType::UnsignedInteger || other.m_type == TagDataType::UnsignedInteger) {
            return toUnsignedInteger() == other.toUnsignedInteger();
        } else if (m_type == TagDataType::Popularity || other.m_type == TagDataType::Popularity) {
            if (options & TagValueComparisionFlags::CaseInsensitive) {
                const auto lhs = toPopularity(), rhs = other.toPopularity();
                return lhs.rating == rhs.rating && lhs.playCounter == rhs.playCounter && lhs.scale == rhs.scale
                    && compareData(lhs.user, rhs.user, true);
            } else {
                return toPopularity() == other.toPopularity();
            }
        }

        // handle other types where an implicit conversion to string can be done by comparing the string representation
        return compareData(toString(), other.toString(m_encoding), options & TagValueComparisionFlags::CaseInsensitive);

    } catch (const ConversionException &) {
        return false;
    }
}

/*!
 * \brief Wipes assigned meta data.
 *  - Clears description, mime type, language and flags.
 *  - Resets the encoding to TagTextEncoding::Latin1.
 *  - Resets the data type to TagDataType::Undefined.
 */
void TagValue::clearMetadata()
{
    m_desc.clear();
    m_mimeType.clear();
    m_locale.clear();
    m_flags = TagValueFlags::None;
    m_encoding = TagTextEncoding::Latin1;
    m_descEncoding = TagTextEncoding::Latin1;
    m_type = TagDataType::Undefined;
}

/*!
 * \brief Returns a "display string" for the specified value.
 * \returns
 * - Returns the just the type if no displayable string can be made of it, eg. "picture", otherwise
 *   returns the string representation.
 * - Returns "invalid …" if a conversion error returned when making the string representation but
 *   never throws a ConversionException (unlike toString()).
 */
std::string TagParser::TagValue::toDisplayString() const
{
    switch (m_type) {
    case TagDataType::Undefined:
    case TagDataType::Binary:
    case TagDataType::Picture:
        return std::string(tagDataTypeString(m_type));
    default:
        try {
            return toString(TagTextEncoding::Utf8);
        } catch (const ConversionException &e) {
            return argsToString("invalid ", tagDataTypeString(m_type), ':', ' ', e.what());
        }
    }
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
        case TagTextEncoding::Utf16LittleEndian:
        case TagTextEncoding::Utf16BigEndian: {
            auto u16str = u16string(reinterpret_cast<char16_t *>(m_ptr.get()), m_size / 2);
            ensureHostByteOrder(u16str, m_encoding);
            return stringToNumber<std::int32_t>(u16str);
        }
        default:
            return bufferToNumber<std::int32_t>(m_ptr.get(), m_size);
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
    case TagDataType::Popularity:
        return static_cast<std::int32_t>(toPopularity().rating);
    case TagDataType::UnsignedInteger: {
        const auto unsignedInteger = toUnsignedInteger();
        if (unsignedInteger > std::numeric_limits<std::int32_t>::max()) {
            throw ConversionException(argsToString("Unsigned integer too big for conversion to integer."));
        }
        return static_cast<std::int32_t>(unsignedInteger);
    }
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to integer."));
    }
}

std::uint64_t TagValue::toUnsignedInteger() const
{
    if (isEmpty()) {
        return 0;
    }
    switch (m_type) {
    case TagDataType::Text:
        switch (m_encoding) {
        case TagTextEncoding::Utf16LittleEndian:
        case TagTextEncoding::Utf16BigEndian: {
            auto u16str = u16string(reinterpret_cast<char16_t *>(m_ptr.get()), m_size / 2);
            ensureHostByteOrder(u16str, m_encoding);
            return stringToNumber<std::uint64_t>(u16str);
        }
        default:
            return bufferToNumber<std::uint64_t>(m_ptr.get(), m_size);
        }
    case TagDataType::PositionInSet:
    case TagDataType::Integer:
    case TagDataType::StandardGenreIndex: {
        const auto integer = toInteger();
        if (integer < 0) {
            throw ConversionException(argsToString("Can not convert negative value to unsigned integer."));
        }
        return static_cast<std::uint64_t>(integer);
    }
    case TagDataType::Popularity:
        return static_cast<std::uint64_t>(toPopularity().rating);
    case TagDataType::UnsignedInteger:
        if (m_size == sizeof(std::uint64_t)) {
            return *reinterpret_cast<std::uint64_t *>(m_ptr.get());
        }
        throw ConversionException("Can not convert assigned data to unsigned integer because the data size is not appropriate.");
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
    case TagDataType::UnsignedInteger:
        if (m_size == sizeof(std::int32_t)) {
            index = static_cast<int>(*reinterpret_cast<std::int32_t *>(m_ptr.get()));
        } else if (m_size == sizeof(std::uint64_t)) {
            const auto unsignedInt = *reinterpret_cast<std::uint64_t *>(m_ptr.get());
            if (unsignedInt <= std::numeric_limits<int>::max()) {
                index = static_cast<int>(unsignedInt);
            } else {
                index = Id3Genres::genreCount();
            }
        } else {
            throw ConversionException("The assigned index/integer is of unappropriate size.");
        }
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
        case TagTextEncoding::Utf16LittleEndian:
        case TagTextEncoding::Utf16BigEndian: {
            auto u16str = u16string(reinterpret_cast<char16_t *>(m_ptr.get()), m_size / 2);
            ensureHostByteOrder(u16str, m_encoding);
            return PositionInSet(u16str);
        }
        default:
            return PositionInSet(string(m_ptr.get(), m_size));
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
    case TagDataType::UnsignedInteger:
        switch (m_size) {
        case sizeof(std::uint64_t): {
            const auto track = *(reinterpret_cast<std::uint64_t *>(m_ptr.get()));
            if (track < std::numeric_limits<std::int32_t>::max()) {
                return PositionInSet(static_cast<std::int32_t>(track));
            }
        }
        default:;
        }
        throw ConversionException("The size of the assigned data is not appropriate.");
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
            throw ConversionException("The size of the assigned data is not appropriate for conversion to time span.");
        }
    case TagDataType::UnsignedInteger:
        switch (m_size) {
        case sizeof(std::uint64_t): {
            const auto ticks = *(reinterpret_cast<std::uint64_t *>(m_ptr.get()));
            if (ticks < static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                return TimeSpan(static_cast<std::int64_t>(ticks));
            }
        }
        default:;
        }
        throw ConversionException("The size of the assigned data is not appropriate.");
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to time span."));
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        DateTime representation (using the UTC timezone).
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
    case TagDataType::UnsignedInteger:
        if (m_size == sizeof(std::int32_t)) {
            return DateTime(*(reinterpret_cast<std::uint32_t *>(m_ptr.get())));
        } else if (m_size == sizeof(std::uint64_t)) {
            return DateTime(*(reinterpret_cast<std::uint64_t *>(m_ptr.get())));
        } else {
            throw ConversionException("The size of the assigned data is not appropriate for conversion to date time.");
        }
    case TagDataType::DateTimeExpression:
        return toDateTimeExpression().gmt();
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to date time."));
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        DateTimeExpression representation.
 * \throws Throws ConversionException on failure.
 */
CppUtilities::DateTimeExpression TagParser::TagValue::toDateTimeExpression() const
{
    if (isEmpty()) {
        return DateTimeExpression();
    }
    switch (m_type) {
    case TagDataType::Text: {
        const auto str = toString(m_encoding == TagTextEncoding::Utf8 ? TagTextEncoding::Utf8 : TagTextEncoding::Latin1);
        try {
            return DateTimeExpression::fromIsoString(str.data());
        } catch (const ConversionException &) {
            return DateTimeExpression::fromString(str.data());
        }
    }
    case TagDataType::Integer:
    case TagDataType::DateTime:
    case TagDataType::UnsignedInteger:
        return DateTimeExpression{ .value = toDateTime(), .delta = TimeSpan(), .parts = DateTimeParts::DateTime };
    case TagDataType::DateTimeExpression:
        if (m_size == sizeof(DateTimeExpression)) {
            return *reinterpret_cast<DateTimeExpression *>(m_ptr.get());
        } else {
            throw ConversionException("The size of the assigned data is not appropriate for conversion to date time expression.");
        }
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to date time."));
    }
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        Popularity representation.
 * \throws Throws ConversionException on failure.
 * \remarks
 * - If text is assigned, the returned popularity's scale will always be TagType::Unspecified
 *   as the text representation does not preserve the scale. Assign the correct scale if needed
 *   manually. Note that tag field implementations provided by this library take care to assign a
 *   popularity (and not just text) when parsing the popularity/rating fields to preserve the
 *   scale information.
 * - Use TagValue::toScaledPopularity() if you want to convert the rating to a certain scale (to
 *   use that scale consistently without having to deal with multiple scales yourself).
 */
Popularity TagValue::toPopularity() const
{
    auto popularity = Popularity();
    if (isEmpty()) {
        return popularity;
    }
    switch (m_type) {
    case TagDataType::Text:
        popularity = Popularity::fromString(std::string_view(toString(TagTextEncoding::Utf8)));
        break;
    case TagDataType::Integer:
        popularity.rating = static_cast<double>(toInteger());
        break;
    case TagDataType::Popularity: {
        auto s = std::stringstream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
        auto reader = BinaryReader(&s);
        try {
            s.exceptions(std::ios_base::failbit | std::ios_base::badbit);
#if defined(__GLIBCXX__) && !defined(_LIBCPP_VERSION)
            s.rdbuf()->pubsetbuf(m_ptr.get(), static_cast<std::streamsize>(m_size));
#else
            s.write(m_ptr.get(), static_cast<std::streamsize>(m_size));
#endif
            popularity.user = reader.readLengthPrefixedString();
            popularity.rating = reader.readFloat64LE();
            popularity.playCounter = reader.readUInt64LE();
            popularity.scale = static_cast<TagType>(reader.readUInt64LE());
        } catch (const std::ios_base::failure &) {
            throw ConversionException(argsToString("Assigned popularity is invalid"));
        }
        break;
    }
    case TagDataType::UnsignedInteger:
        popularity.rating = static_cast<double>(toUnsignedInteger());
        break;
    default:
        throw ConversionException(argsToString("Can not convert ", tagDataTypeString(m_type), " to date time."));
    }
    return popularity;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        Popularity representation using the specified \a scale.
 * \throws Throws ConversionException on failure, e.g. when Popularity::scaleTo() fails.
 * \remarks
 * 1. See Popularity::scaleTo() for details about scaling.
 * 2. If text is assigned, it is converted like with TagValue::toPopularity(). However,
 *    the specified \a a scale is *assigned* as the popularity's scale assuming that the
 *    text representation already contains a rating with the desired \a scale. That means
 *    if you assign text to a TagValue, the tag implementations (which use this function
 *    internally) will use that text as-is when serializing the popularity/rating field.
 * 3. Since TagValue::toString() also does not do any scaling the previous point means that
 *    if you only ever use TagValue::assignText() (or equivalent c'tors) and TagValue::toString()
 *    you will always work with raw rating values consistently.
 * 4. Since tag implementations provided by this library always take care to assign the
 *    popularity/rating as such (and not just as text) you do not need to care about point 2. if
 *    you want to use a certain scale consistently. Just call this function with the desired scal
 *    when reading and assign a popularity object with that scale before saving changes.
 */
Popularity TagValue::toScaledPopularity(TagType scale) const
{
    auto popularity = toPopularity();
    if (m_type == TagDataType::Text) {
        popularity.scale = scale;
    } else if (!popularity.scaleTo(scale)) {
        throw ConversionException(argsToString("Assigned popularity cannot be scaled accordingly"));
    }
    return popularity;
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
        switch (descriptionEncoding()) {
        case TagTextEncoding::Latin1:
            encodedData = convertLatin1ToUtf8(m_desc.data(), m_desc.size());
            break;
        case TagTextEncoding::Utf16LittleEndian:
            encodedData = convertUtf16LEToUtf8(m_desc.data(), m_desc.size());
            break;
        case TagTextEncoding::Utf16BigEndian:
            encodedData = convertUtf16BEToUtf8(m_desc.data(), m_desc.size());
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
 * - If UTF-16 is the desired output \a encoding, it makes sense to use TagValue::toWString() instead.
 * - If a popularity is assigned, its string representation is returned without further scaling.
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
        } else if (const auto genreName = Id3Genres::stringFromIndex(genreIndex); !genreName.empty()) {
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
        result = toDateTime().toIsoString();
        break;
    case TagDataType::Popularity:
        result = toPopularity().toString();
        break;
    case TagDataType::UnsignedInteger:
        result = numberToString(toUnsignedInteger());
        break;
    case TagDataType::DateTimeExpression:
        result = toDateTimeExpression().toIsoString();
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
        } else if (const auto genreName = Id3Genres::stringFromIndex(genreIndex); !genreName.empty()) {
            regularStrRes.assign(genreName);
        } else {
            throw ConversionException("No string representation for the assigned standard genre index available.");
        }
        break;
    }
    case TagDataType::TimeSpan:
        regularStrRes = toTimeSpan().toString();
        break;
    case TagDataType::DateTime:
        regularStrRes = toDateTime().toString(DateTimeOutputFormat::IsoOmittingDefaultComponents);
        break;
    case TagDataType::Popularity:
        regularStrRes = toPopularity().toString();
        break;
    case TagDataType::UnsignedInteger:
        regularStrRes = numberToString(toUnsignedInteger());
        break;
    case TagDataType::DateTimeExpression:
        regularStrRes = toDateTimeExpression().toIsoString();
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
 * \brief Assigns the given unsigned integer \a value.
 * \param value Specifies the unsigned integer to be assigned.
 */
void TagValue::assignUnsignedInteger(std::uint64_t value)
{
    m_size = sizeof(value);
    m_ptr = make_unique<char[]>(m_size);
    std::copy(reinterpret_cast<const char *>(&value), reinterpret_cast<const char *>(&value) + m_size, m_ptr.get());
    m_type = TagDataType::UnsignedInteger;
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
    m_ptr = std::move(data);
}

/*!
 * \brief Assigns the specified popularity \a value.
 */
void TagValue::assignPopularity(const Popularity &value)
{
    auto s = std::stringstream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    auto writer = BinaryWriter(&s);
    try {
        s.exceptions(std::ios_base::failbit | std::ios_base::badbit);
        writer.writeLengthPrefixedString(value.user);
        writer.writeFloat64LE(value.rating);
        writer.writeUInt64LE(value.playCounter);
        writer.writeUInt64LE(static_cast<std::uint64_t>(value.scale));
        auto size = static_cast<std::size_t>(s.tellp());
        auto ptr = std::make_unique<char[]>(size);
        s.read(ptr.get(), s.tellp());
        assignData(std::move(ptr), size, TagDataType::Popularity);
    } catch (const std::ios_base::failure &) {
        throw ConversionException("Unable to serialize specified Popularity");
    }
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
        if ((length >= 2) && (LE::toInt<std::uint16_t>(text) == 0xFEFF)) {
            text += 2;
            length -= 2;
        }
        break;
    case TagTextEncoding::Utf16BigEndian:
        if ((length >= 2) && (BE::toInt<std::uint16_t>(text) == 0xFEFF)) {
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

/*!
 * \brief Scales the rating from the current scale to \a targetScale.
 * \returns
 * Returns whether a conversion from the current scale to \a targetScale was possible. If no, the object stays unchanged.
 * Note that it is not validated whether the currently assigned rating is a valid value in the currently assigned scale.
 * \remarks
 * - Providing TagType::Unspecified as \a targetScale will convert to a *generic* scale where the rating is number is between
 *   1 and 5 with decimal values possible where 5 is the best possible rating and 1 the lowest. The value 0 means there's no
 *   rating.
 * - If the currently assigned scale is TagType::Unspecified than the currently assigned rating is assumed to use the *generic*
 *   scale described in the previous point.
 */
bool Popularity::scaleTo(TagType targetScale)
{
    if (scale == targetScale) {
        return true;
    }

    // convert to generic scale first
    double genericRating;
    switch (scale) {
    case TagType::Unspecified:
        genericRating = rating;
        break;
    case TagType::MatroskaTag:
        genericRating = rating / (5.0 / 4.0) + 1.0;
        break;
    case TagType::Id3v2Tag:
        genericRating = rating < 1.0 ? 0.0 : ((rating - 1.0) / (254.0 / 4.0) + 1.0);
        break;
    case TagType::VorbisComment:
    case TagType::OggVorbisComment:
        genericRating = rating / 20.0;
        break;
    default:
        return false;
    }

    // convert from the generic scale to the target scale
    switch (targetScale) {
    case TagType::Unspecified:
        rating = genericRating;
        break;
    case TagType::MatroskaTag:
        rating = (genericRating - 1.0) * (5.0 / 4.0);
        break;
    case TagType::Id3v2Tag:
        rating = genericRating < 1.0 ? 0.0 : ((genericRating - 1.0) * (254.0 / 4.0) + 1.0);
        break;
    case TagType::VorbisComment:
    case TagType::OggVorbisComment:
        rating = genericRating * 20.0;
        break;
    default:
        return false;
    }

    scale = targetScale;
    return true;
}

/*!
 * \brief Returns the popularity as string in the format "rating" if only a rating is present
 *        or in the format "user|rating|play-counter" or an empty string if the popularity isEmpty().
 */
std::string Popularity::toString() const
{
    return isEmpty() ? std::string()
                     : ((user.empty() && !playCounter) ? numberToString(rating) : (user % '|' % numberToString(rating) % '|' + playCounter));
}

/*!
 * \brief Parses the popularity from \a str assuming the same format as toString() produces and
 *        sets TagType::Unspecified as scale. So \a str is expected to contain a rating within
 *        the range of 1.0 and 5.0 or 0.0 to denote there's no rating.
 * \throws Throws ConversionException() if the format is invalid.
 */
Popularity Popularity::fromString(std::string_view str)
{
    return fromString(str, TagType::Unspecified);
}

/*!
 * \brief Parses the popularity from \a str assuming the same format as toString() produces and assigns the
 *        specified \a scale. So \a str is expected to contain a rating according to the specifications of
 *        the tag format passed via \a scale.
 * \throws Throws ConversionException() if the format is invalid.
 */
TagParser::Popularity TagParser::Popularity::fromString(std::string_view str, TagType scale)
{
    const auto parts = splitStringSimple<std::vector<std::string_view>>(str, "|");
    auto res = Popularity({ .scale = scale });
    if (parts.empty()) {
        return res;
    } else if (parts.size() > 3) {
        throw ConversionException("Wrong format, expected \"rating\" or \"user|rating|play-counter\"");
    }
    // treat a single number as rating
    if (parts.size() == 1) {
        try {
            res.rating = stringToNumber<decltype(res.rating)>(parts.front());
            return res;
        } catch (const ConversionException &) {
        }
    }
    // otherwise, read user, rating and play counter
    res.user = parts.front();
    if (parts.size() > 1) {
        res.rating = stringToNumber<decltype(res.rating)>(parts[1]);
    }
    if (parts.size() > 2) {
        res.playCounter = stringToNumber<decltype(res.playCounter)>(parts[2]);
    }
    return res;
}

} // namespace TagParser
