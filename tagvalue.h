#ifndef TAGVALUE_H
#define TAGVALUE_H

#include "./positioninset.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/chrono/datetime.h>

#include <iosfwd>
#include <string>
#include <memory>

namespace Media {

class Tag;
class Id3v2Frame;

/*!
 * \brief Specifies the text encoding.
 */
enum class TagTextEncoding : unsigned int
{
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
inline int characterSize(TagTextEncoding encoding) {
    switch(encoding) {
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
enum class TagDataType : unsigned int
{
    Text, /**< text/string */
    Integer, /**< integer, up to 64-bit */
    PositionInSet, /**< position in set, see Media::PositionInSet */
    StandardGenreIndex, /**< pre-defined genre name denoted by numerical code */
    TimeSpan, /**< time span, see ChronoUtils::TimeSpan */
    DateTime, /**< date time, see ChronoUtils::DateTime */
    Picture, /**< picture file */
    Binary, /**< unspecified binary data */
    Undefined, /**< undefined/invalid data type */
    Float, /**< IEEE float, 32- or 64-bit */
};

class TAG_PARSER_EXPORT TagValue
{
    friend class Id3v2Frame; // FIXME: make ensureHostByteOrder() public in next minor release

public:
    // constructor, destructor
    TagValue();
    TagValue(const char *text, std::size_t textSize, TagTextEncoding textEncoding = TagTextEncoding::Latin1, TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    TagValue(const std::string &text, TagTextEncoding textEncoding = TagTextEncoding::Latin1, TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    TagValue(int value);
    TagValue(const char *data, size_t length, TagDataType type = TagDataType::Undefined, TagTextEncoding encoding = TagTextEncoding::Latin1);
    TagValue(std::unique_ptr<char[]> &&data, size_t length, TagDataType type = TagDataType::Binary, TagTextEncoding encoding = TagTextEncoding::Latin1);
    TagValue(const PositionInSet &value);
    TagValue(const TagValue &other);
    TagValue(TagValue &&other) = default;
    ~TagValue();

    // operators
    TagValue &operator=(const TagValue &other);
    TagValue &operator=(TagValue &&other) = default;
    bool operator==(const TagValue &other) const;
    bool operator!=(const TagValue &other) const;

    // methods
    bool isEmpty() const;
    void clearData();
    void clearMetadata();
    void clearDataAndMetadata();
    TagDataType type() const;
    std::string toString(TagTextEncoding encoding = TagTextEncoding::Unspecified) const;
    void toString(std::string &result, TagTextEncoding encoding = TagTextEncoding::Unspecified) const;
    std::u16string toWString(TagTextEncoding encoding = TagTextEncoding::Unspecified) const;
    void toWString(std::u16string &result, TagTextEncoding encoding = TagTextEncoding::Unspecified) const;
    int32 toInteger() const;
    template<typename NumberType>
    NumberType toNumber() const;
    int toStandardGenreIndex() const;
    PositionInSet toPositionInSet() const;
    ChronoUtilities::TimeSpan toTimeSpan() const;
    ChronoUtilities::DateTime toDateTime() const;
    size_t dataSize() const;
    char *dataPointer() const;
    const std::string &description() const;
    void setDescription(const std::string &value, TagTextEncoding encoding = TagTextEncoding::Latin1);
    const std::string &mimeType() const;
    void setMimeType(const std::string &value);
    const std::string &language() const;
    void setLanguage(const std::string &value);
    bool isLabeledAsReadonly() const;
    void setReadonly(bool value);
    TagTextEncoding dataEncoding() const;
    void convertDataEncoding(TagTextEncoding encoding);
    void convertDataEncodingForTag(const Tag *tag);
    TagTextEncoding descriptionEncoding() const;
    static const TagValue &empty();

    void assignText(const char *text, std::size_t textSize, TagTextEncoding textEncoding = TagTextEncoding::Latin1, TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    void assignText(const std::string &text, TagTextEncoding textEncoding = TagTextEncoding::Latin1, TagTextEncoding convertTo = TagTextEncoding::Unspecified);
    void assignInteger(int value);
    void assignStandardGenreIndex(int index);
    void assignData(const char *data, size_t length, TagDataType type = TagDataType::Binary, TagTextEncoding encoding = TagTextEncoding::Latin1);
    void assignData(std::unique_ptr<char[]> &&data, size_t length, TagDataType type = TagDataType::Binary, TagTextEncoding encoding = TagTextEncoding::Latin1);
    void assignPosition(PositionInSet value);
    void assignTimeSpan(ChronoUtilities::TimeSpan value);
    void assignDateTime(ChronoUtilities::DateTime value);


private:
    static void stripBom(const char *&text, size_t &length, TagTextEncoding encoding);
    static void ensureHostByteOrder(std::u16string &u16str, TagTextEncoding currentEncoding);

    std::unique_ptr<char[]> m_ptr;
    std::string::size_type m_size;
    TagDataType m_type;
    std::string m_desc;
    std::string m_mimeType;
    std::string m_lng;
    bool m_labeledAsReadonly;
    TagTextEncoding m_encoding;
    TagTextEncoding m_descEncoding;
};

/*!
 * \brief Constructs an empty TagValue.
 */
inline TagValue::TagValue() :
    m_size(0),
    m_type(TagDataType::Undefined),
    m_labeledAsReadonly(false),
    m_encoding(TagTextEncoding::Latin1),
    m_descEncoding(TagTextEncoding::Latin1)
{}

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
inline TagValue::TagValue(const char *text, std::size_t textSize, TagTextEncoding textEncoding, TagTextEncoding convertTo) :
    m_labeledAsReadonly(false),
    m_descEncoding(TagTextEncoding::Latin1)
{
    assignText(text, textSize, textEncoding, convertTo);
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
inline TagValue::TagValue(const std::string &text, TagTextEncoding textEncoding, TagTextEncoding convertTo) :
    m_labeledAsReadonly(false),
    m_descEncoding(TagTextEncoding::Latin1)
{
    assignText(text, textEncoding, convertTo);
}

/*!
 * \brief Constructs a new TagValue holding the given integer \a value.
 */
inline TagValue::TagValue(int value) :
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
 * \remarks Strips the BOM of the specified \a data if \a type is TagDataType::Text.
 */
inline TagValue::TagValue(const char *data, size_t length, TagDataType type, TagTextEncoding encoding) :
    m_size(length),
    m_type(type),
    m_labeledAsReadonly(false),
    m_encoding(encoding),
    m_descEncoding(TagTextEncoding::Latin1)
{
    if(length) {
        if(type == TagDataType::Text) {
            stripBom(data, m_size, encoding);
        }
        m_ptr = std::make_unique<char []>(m_size);
        std::copy(data, data + m_size, m_ptr.get());
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
 * \remarks Does not strip the BOM so for consistency the caller must ensure there is no BOM present.
 */
inline TagValue::TagValue(std::unique_ptr<char[]> &&data, size_t length, TagDataType type, TagTextEncoding encoding) :
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
inline TagValue::TagValue(const PositionInSet &value) :
    TagValue(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::PositionInSet)
{}

/*!
 * \brief Returns whether both instances are not equal.
 * \remarks Simply the negation of operator==() so check there for details.
 */
inline bool TagValue::operator!=(const TagValue &other) const
{
    return !(*this == other);
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
    if(value.isNull()) {
        m_type = TagDataType::PositionInSet;
        clearData();
    } else {
        assignData(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::PositionInSet);
    }
}

/*!
 * \brief Assigns the given TimeSpan \a value.
 */
inline void TagValue::assignTimeSpan(ChronoUtilities::TimeSpan value)
{
    assignData(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::TimeSpan);
}

/*!
 * \brief Assigns the given DateTime \a value.
 */
inline void TagValue::assignDateTime(ChronoUtilities::DateTime value)
{
    assignData(reinterpret_cast<const char *>(&value), sizeof(value), TagDataType::DateTime);
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
 * \param encoding Specifies the encoding to to be used; set to TagTextEncoding::Unspecified to use the
 *        present encoding without any character set conversion.
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
 * \brief Returns an indication whether an value is assigned.
 * \remarks Meta data such as description and MIME type is not considered as an assigned value.
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
 * \brief Returns the size of the assigned value in bytes.
 * \remarks Meta data such as description and MIME type is not considered as part of the assigned value.
 */
inline size_t TagValue::dataSize() const
{
    return m_size;
}

/*!
 * \brief Returns a pointer to the raw data assigned to the current instance.
 * \remarks The instance keeps ownership over the data which will be invalidated when the
 *          it gets destroyed or an other value is assigned.
 * \remarks The raw data is not null terminated. See dataSize().
 */
inline char *TagValue::dataPointer() const
{
    return m_ptr.get();
}

/*!
 * \brief Returns the description.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa descriptionEncoding()
 * \sa setDescription()
 */
inline const std::string &TagValue::description() const
{
    return m_desc;
}

/*!
 * \brief Sets the description.
 * \param value Specifies the description.
 * \param encoding Specifies the encoding used to provide the description.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa description()
 * \sa descriptionEncoding()
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
 * \param value Specifies the mime type.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa mimeType()
 */
inline void TagValue::setMimeType(const std::string &value)
{
    m_mimeType = value;
}

/*!
 * \brief Returns the language.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa setLanguage()
 */
inline const std::string &TagValue::language() const
{
    return m_lng;
}

/*!
 * \brief Sets the language.
 * \param value Specifies the language.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa language()
 */
inline void TagValue::setLanguage(const std::string &value)
{
    m_lng = value;
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
inline void TagValue::setReadonly(bool value)
{
    m_labeledAsReadonly = value;
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

}

#endif // TAGVALUE_H
