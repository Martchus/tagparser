#ifndef TAG_PARSER_TAGVALUE_H
#define TAG_PARSER_TAGVALUE_H

#include "./positioninset.h"

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/misc/flagenumclass.h>
#include <c++utilities/misc/traits.h>

#include <cstring>
#include <iosfwd>
#include <memory>
#include <string>

namespace TagParser {

class Tag;
class Id3v2Frame;

/*!
 * \brief Specifies the text encoding.
 */
enum class TagTextEncoding : unsigned int {
    Latin1, /**< ISO/IEC 8859-1 aka "Latin 1" */
    Utf8, /**< UTF-8 */
    Utf16LittleEndian, /**< UTF-16 (little endian) */
    Utf16BigEndian, /**< UTF-16 (big endian) */
    Unspecified /**< unspecified encoding */
};

/*!
 * \brief Returns the size of one character for the specified \a encoding in bytes.
 * \remarks For variable-width encoding the minimum size is returned.
 */
constexpr int characterSize(TagTextEncoding encoding)
{
    switch (encoding) {
    case TagTextEncoding::Latin1:
    case TagTextEncoding::Utf8:
        return 1;
    case TagTextEncoding::Utf16LittleEndian:
    case TagTextEncoding::Utf16BigEndian:
        return 2;
    default:
        return 0;
    }
}

/*!
 * \brief Specifies the data type.
 */
enum class TagDataType : unsigned int {
    Text, /**< text/string */
    Integer, /**< integer */
    PositionInSet, /**< position in set, see TagParser::PositionInSet */
    StandardGenreIndex, /**< pre-defined genre name denoted by numerical code */
    TimeSpan, /**< time span, see ChronoUtils::TimeSpan */
    DateTime, /**< date time, see ChronoUtils::DateTime */
    Picture, /**< picture file */
    Binary, /**< unspecified binary data */
    Undefined /**< undefined/invalid data type */
};

/*!
 * \brief The TagValueComparisionOption enum specifies options for TagValue::compareTo().
 */
enum class TagValueComparisionFlags : unsigned int {
    None, /**< no special behavior */
    CaseInsensitive = 0x1, /**< string-comparisions are case-insensitive (does *not* affect non-string comparisions) */
    IgnoreMetaData = 0x2, /**< do *not* take meta-data like description and MIME-types into account */
};

class TAG_PARSER_EXPORT TagValue {
public:
    // constructor, destructor
    TagValue();
    TagValue(const char *text, std::size_t textSize, TagTextEncoding textEncoding = TagTextEncoding::Latin1,
        TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    TagValue(const char *text, TagTextEncoding textEncoding = TagTextEncoding::Latin1, TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    TagValue(
        const std::string &text, TagTextEncoding textEncoding = TagTextEncoding::Latin1, TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    TagValue(int value);
    TagValue(const char *data, std::size_t length, TagDataType type = TagDataType::Undefined, TagTextEncoding encoding = TagTextEncoding::Latin1);
    TagValue(std::unique_ptr<char[]> &&data, std::size_t length, TagDataType type = TagDataType::Binary,
        TagTextEncoding encoding = TagTextEncoding::Latin1);
    TagValue(PositionInSet value);
    TagValue(CppUtilities::DateTime value);
    TagValue(CppUtilities::TimeSpan value);
    TagValue(const TagValue &other);
    TagValue(TagValue &&other) = default;
    ~TagValue();

    // operators
    TagValue &operator=(const TagValue &other);
    TagValue &operator=(TagValue &&other) = default;
    bool operator==(const TagValue &other) const;
    bool operator!=(const TagValue &other) const;
    operator bool() const;

    // methods
    bool isNull() const;
    bool isEmpty() const;
    void clearData();
    void clearMetadata();
    void clearDataAndMetadata();
    TagDataType type() const;
    std::string toString(TagTextEncoding encoding = TagTextEncoding::Unspecified) const;
    void toString(std::string &result, TagTextEncoding encoding = TagTextEncoding::Unspecified) const;
    std::u16string toWString(TagTextEncoding encoding = TagTextEncoding::Unspecified) const;
    void toWString(std::u16string &result, TagTextEncoding encoding = TagTextEncoding::Unspecified) const;
    std::int32_t toInteger() const;
    int toStandardGenreIndex() const;
    PositionInSet toPositionInSet() const;
    CppUtilities::TimeSpan toTimeSpan() const;
    CppUtilities::DateTime toDateTime() const;
    std::size_t dataSize() const;
    char *dataPointer();
    const char *dataPointer() const;
    const std::string &description() const;
    void setDescription(const std::string &value, TagTextEncoding encoding = TagTextEncoding::Latin1);
    const std::string &mimeType() const;
    void setMimeType(const std::string &mimeType);
    const std::string &language() const;
    void setLanguage(const std::string &language);
    bool isLabeledAsReadonly() const;
    void setReadonly(bool readOnly);
    TagTextEncoding dataEncoding() const;
    void convertDataEncoding(TagTextEncoding encoding);
    void convertDataEncodingForTag(const Tag *tag);
    TagTextEncoding descriptionEncoding() const;
    void convertDescriptionEncoding(TagTextEncoding encoding);
    static const TagValue &empty();

    void assignText(const char *text, std::size_t textSize, TagTextEncoding textEncoding = TagTextEncoding::Latin1,
        TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    void assignText(
        const std::string &text, TagTextEncoding textEncoding = TagTextEncoding::Latin1, TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    void assignInteger(int value);
    void assignStandardGenreIndex(int index);
    void assignData(const char *data, std::size_t length, TagDataType type = TagDataType::Binary, TagTextEncoding encoding = TagTextEncoding::Latin1);
    void assignData(std::unique_ptr<char[]> &&data, std::size_t length, TagDataType type = TagDataType::Binary,
        TagTextEncoding encoding = TagTextEncoding::Latin1);
    void assignPosition(PositionInSet value);
    void assignTimeSpan(CppUtilities::TimeSpan value);
    void assignDateTime(CppUtilities::DateTime value);

    static void stripBom(const char *&text, std::size_t &length, TagTextEncoding encoding);
    static void ensureHostByteOrder(std::u16string &u16str, TagTextEncoding currentEncoding);
    template <typename ContainerType,
        CppUtilities::Traits::EnableIf<CppUtilities::Traits::IsIteratable<ContainerType>,
            std::is_same<typename std::add_const<typename std::remove_pointer<typename ContainerType::value_type>::type>::type, const TagValue>>
            * = nullptr>
    static std::vector<std::string> toStrings(const ContainerType &values, TagTextEncoding encoding = TagTextEncoding::Utf8);
    bool compareTo(const TagValue &other, TagValueComparisionFlags options = TagValueComparisionFlags::None) const;
    bool compareData(const TagValue &other, bool ignoreCase = false) const;
    static bool compareData(const std::string &data1, const std::string &data2, bool ignoreCase = false);
    static bool compareData(const char *data1, std::size_t size1, const char *data2, std::size_t size2, bool ignoreCase = false);

private:
    std::unique_ptr<char[]> m_ptr;
    std::size_t m_size;
    std::string m_desc;
    std::string m_mimeType;
    std::string m_language;
    TagDataType m_type;
    TagTextEncoding m_encoding;
    TagTextEncoding m_descEncoding;
    bool m_labeledAsReadonly;
};

/*!
 * \brief Constructs an empty TagValue.
 */
inline TagValue::TagValue()
    : m_size(0)
    , m_type(TagDataType::Undefined)
    , m_encoding(TagTextEncoding::Latin1)
    , m_descEncoding(TagTextEncoding::Latin1)
    , m_labeledAsReadonly(false)
{
}

/*!
 * \brief Destroys the TagValue.
 */
inline TagValue::~TagValue()
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
inline TagValue::TagValue(const char *text, std::size_t textSize, TagTextEncoding textEncoding, TagTextEncoding convertTo)
    : m_descEncoding(TagTextEncoding::Latin1)
    , m_labeledAsReadonly(false)
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
inline TagValue::TagValue(const char *text, TagTextEncoding textEncoding, TagTextEncoding convertTo)
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
inline TagValue::TagValue(const std::string &text, TagTextEncoding textEncoding, TagTextEncoding convertTo)
    : m_descEncoding(TagTextEncoding::Latin1)
    , m_labeledAsReadonly(false)
{
    assignText(text, textEncoding, convertTo);
}

/*!
 * \brief Constructs a new TagValue holding the given integer \a value.
 */
inline TagValue::TagValue(int value)
    : TagValue(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::Integer)
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
inline TagValue::TagValue(const char *data, std::size_t length, TagDataType type, TagTextEncoding encoding)
    : m_size(length)
    , m_type(type)
    , m_encoding(encoding)
    , m_descEncoding(TagTextEncoding::Latin1)
    , m_labeledAsReadonly(false)
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
inline TagValue::TagValue(std::unique_ptr<char[]> &&data, std::size_t length, TagDataType type, TagTextEncoding encoding)
    : m_size(length)
    , m_type(type)
    , m_encoding(encoding)
    , m_descEncoding(TagTextEncoding::Latin1)
    , m_labeledAsReadonly(false)
{
    if (length) {
        m_ptr = move(data);
    }
}

/*!
 * \brief Constructs a new TagValue holding a copy of the given PositionInSet \a value.
 */
inline TagValue::TagValue(PositionInSet value)
    : TagValue(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::PositionInSet)
{
}

/*!
 * \brief Constructs a new TagValue holding a copy of the given DateTime \a value.
 */
inline TagValue::TagValue(CppUtilities::DateTime value)
    : TagValue(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::DateTime)
{
}

/*!
 * \brief Constructs a new TagValue holding a copy of the given TimeSpan \a value.
 */
inline TagValue::TagValue(CppUtilities::TimeSpan value)
    : TagValue(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::TimeSpan)
{
}

/*!
 * \brief Returns whether both instances are equal.
 * \sa The same as TagValue::compareTo() with TagValueComparisionOption::None so see TagValue::compareTo() for details.
 */
inline bool TagValue::operator==(const TagValue &other) const
{
    return compareTo(other, TagValueComparisionFlags::None);
}

/*!
 * \brief Returns whether both instances are not equal.
 * \sa The negation of TagValue::compareTo() with TagValueComparisionOption::None so see TagValue::compareTo() for details.
 */
inline bool TagValue::operator!=(const TagValue &other) const
{
    return !compareTo(other, TagValueComparisionFlags::None);
}

/*!
 * \brief Returns whether the value is not empty.
 * \sa See TagValue::isEmpty() for a definition on what is considered empty.
 */
inline TagParser::TagValue::operator bool() const
{
    return !isEmpty();
}

/*!
 * \brief Assigns a copy of the given \a text.
 * \param text Specifies the text to be assigned.
 * \param textEncoding Specifies the encoding of the given \a text.
 * \param convertTo Specifies the encoding to convert \a text to; set to TagTextEncoding::Unspecified to
 *                  use \a textEncoding without any character set conversions.
 * \throws Throws a ConversionException if the conversion the specified character set fails.
 * \remarks Strips the BOM of the specified \a text.
 */
inline void TagValue::assignText(const std::string &text, TagTextEncoding textEncoding, TagTextEncoding convertTo)
{
    assignText(text.data(), text.size(), textEncoding, convertTo);
}

/*!
 * \brief Assigns the given PositionInSet \a value.
 */
inline void TagValue::assignPosition(PositionInSet value)
{
    if (value.isNull()) {
        m_type = TagDataType::PositionInSet;
        clearData();
    } else {
        assignData(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::PositionInSet);
    }
}

/*!
 * \brief Assigns the given TimeSpan \a value.
 */
inline void TagValue::assignTimeSpan(CppUtilities::TimeSpan value)
{
    assignData(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::TimeSpan);
}

/*!
 * \brief Assigns the given DateTime \a value.
 */
inline void TagValue::assignDateTime(CppUtilities::DateTime value)
{
    assignData(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::DateTime);
}

/*!
 * \brief Assigns the given standard genre \a index to be assigned.
 * \param index Specifies the index to be assigned.
 * \sa <a href="http://en.wikipedia.org/wiki/ID3#List_of_genres">List of genres - Wikipedia</a>
 */
inline void TagValue::assignStandardGenreIndex(int index)
{
    assignInteger(index);
    m_type = TagDataType::StandardGenreIndex;
}

/*!
 * \brief Returns the type of the assigned value.
 */
inline TagDataType TagValue::type() const
{
    return m_type;
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
inline std::string TagValue::toString(TagTextEncoding encoding) const
{
    std::string res;
    toString(res, encoding);
    return res;
}

/*!
 * \brief Converts the value of the current TagValue object to its equivalent
 *        std::wstring representation.
 * \throws Throws ConversionException on failure.
 * \remarks Use this only, if \a encoding is an UTF-16 encoding.
 */
inline std::u16string TagValue::toWString(TagTextEncoding encoding) const
{
    std::u16string res;
    toWString(res, encoding);
    return res;
}

/*!
 * \brief Returns whether no value is assigned at all.
 *
 * Returns only true for default constructed instances or cleared instances (using TagValue::clearData()).
 * So for empty strings, the integer 0, a TimeSpan of zero length, ... this function returns true.
 *
 * \remarks Meta-data such as description and MIME-type is not considered as an assigned value.
 */
inline bool TagValue::isNull() const
{
    return m_ptr == nullptr;
}

/*!
 * \brief Returns whether no or an empty value is assigned.
 *
 * An empty string and empty binary or picture data counts as empty so this function will return
 * true for those. However, the integer 0, a TimeSpan of zero length, ... are not considered empty
 * and this function will return false.
 *
 * \remarks Meta-data such as description and MIME-type is not considered as an assigned value.
 */
inline bool TagValue::isEmpty() const
{
    return m_ptr == nullptr || m_size == 0;
}

/*!
 * \brief Clears the assigned data.
 * \remarks Meta data such as description and MIME type remains unaffected.
 * \sa clearMetadata()
 * \sa clearDataAndMetadata()
 */
inline void TagValue::clearData()
{
    m_size = 0;
    m_ptr.reset();
}

/*!
 * \brief Wipes assigned data including meta data.
 * \sa clearData()
 * \sa clearMetadata()
 */
inline void TagValue::clearDataAndMetadata()
{
    clearData();
    clearMetadata();
}

/*!
 * \brief Returns the size of the assigned value in bytes.
 * \remarks Meta data such as description and MIME type is not considered as part of the assigned value.
 */
inline std::size_t TagValue::dataSize() const
{
    return m_size;
}

/*!
 * \brief Returns a pointer to the raw data assigned to the current instance.
 * \remarks The instance keeps ownership over the data which will be invalidated when the
 *          TagValue gets destroyed or another value is assigned.
 * \remarks The raw data is not null terminated. See dataSize().
 */
inline char *TagValue::dataPointer()
{
    return m_ptr.get();
}

inline const char *TagValue::dataPointer() const
{
    return m_ptr.get();
}

/*!
 * \brief Returns the description.
 * \remarks The usage of this meta information depends on the tag implementation. It might be ignored
 *          if not supported.
 * \sa
 * - descriptionEncoding() for the encoding of the returned string
 * - convertDescriptionEncoding() to change the encoding of the description
 * - setDescription() for setting the description
 */
inline const std::string &TagValue::description() const
{
    return m_desc;
}

/*!
 * \brief Sets the description.
 * \param value Specifies the description.
 * \param encoding Specifies the encoding used to provide the description.
 * \remarks The usage of this meta information depends on the tag implementation. It might be ignored
 *          if not supported.
 * \sa
 * - description() and descriptionEncoding()
 * - convertDescriptionEncoding() to change the description encoding after assignment
 */
inline void TagValue::setDescription(const std::string &value, TagTextEncoding encoding)
{
    m_desc = value;
    m_descEncoding = encoding;
}

/*!
 * \brief Returns the MIME type.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa setMimeType()
 */
inline const std::string &TagValue::mimeType() const
{
    return m_mimeType;
}

/*!
 * \brief Sets the MIME type.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa mimeType()
 */
inline void TagValue::setMimeType(const std::string &mimeType)
{
    m_mimeType = mimeType;
}

/*!
 * \brief Returns the language.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa setLanguage()
 */
inline const std::string &TagValue::language() const
{
    return m_language;
}

/*!
 * \brief Sets the language.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa language()
 */
inline void TagValue::setLanguage(const std::string &language)
{
    m_language = language;
}

/*!
 * \brief Returns an indication whether the value is labeled as read-only.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \remarks This is just an additional information. It has no effect on the behavior
 *          of the TagValue thus assignments can still be performed (to prohibit
 *          assignments simply use the "const" keyword).
 * \sa setReadonly()
 */
inline bool TagValue::isLabeledAsReadonly() const
{
    return m_labeledAsReadonly;
}

/*!
 * \brief Sets whether the TagValue is labeled as read-only.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \remarks This is just an additional information. It has no effect on the behavior
 *          of the TagValue thus assignments can still be performed (to prohibit
 *          assignments simply use the "const" keyword).
 * \sa isLabeledAsReadonly()
 */
inline void TagValue::setReadonly(bool readOnly)
{
    m_labeledAsReadonly = readOnly;
}

/*!
 * \brief Returns the data encoding.
 * \remarks This value is only relevant if type() equals TagDataType::Text.
 * \sa assignText()
 */
inline TagTextEncoding TagValue::dataEncoding() const
{
    return m_encoding;
}

/*!
 * \brief Returns the description encoding.
 * \remarks This value is only relevant if a description is assigned.
 * \sa description(), setDescription()
 */
inline TagTextEncoding TagValue::descriptionEncoding() const
{
    return m_descEncoding;
}

/*!
 * \brief Converts the specified \a values to string using the specified \a encoding.
 * \throws Throws ConversionException on failure.
 * \sa toString()
 */
template <typename ContainerType,
    CppUtilities::Traits::EnableIf<CppUtilities::Traits::IsIteratable<ContainerType>,
        std::is_same<typename std::add_const<typename std::remove_pointer<typename ContainerType::value_type>::type>::type, const TagValue>> *>
std::vector<std::string> TagValue::toStrings(const ContainerType &values, TagTextEncoding encoding)
{
    std::vector<std::string> res;
    res.reserve(values.size());
    for (const auto &value : values) {
        res.emplace_back(CppUtilities::Traits::dereferenceMaybe(value).toString(encoding));
    }
    return res;
}

/*!
 * \brief Returns whether the raw data of the current instance equals the raw data of \a other.
 */
inline bool TagValue::compareData(const TagValue &other, bool ignoreCase) const
{
    return compareData(m_ptr.get(), m_size, other.m_ptr.get(), other.m_size, ignoreCase);
}

/*!
 * \brief Returns whether 2 data buffers are equal.
 */
inline bool TagValue::compareData(const std::string &data1, const std::string &data2, bool ignoreCase)
{
    return compareData(data1.data(), data1.size(), data2.data(), data2.size(), ignoreCase);
}

} // namespace TagParser

CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(TagParser, TagParser::TagValueComparisionFlags)

#endif // TAG_PARSER_TAGVALUE_H
