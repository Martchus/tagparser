#ifndef TAGVALUE_H
#define TAGVALUE_H

#include "positioninset.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/chrono/datetime.h>

#include <istream>
#include <string>
#include <memory>

namespace Media
{

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
    Integer, /**< integer */
    PositionInSet, /**< position in set, see Media::PositionInSet */
    StandardGenreIndex, /**< pre-defined genre name denoted by numerical code */
    TimeSpan, /**< time span, see ChronoUtils::TimeSpan */
    DateTime, /**< date time, see ChronoUtils::DateTime */
    Picture, /**< picture file */
    Binary, /**< unspecified binary data */
    Undefined /**< undefined/invalid data type */
};

class LIB_EXPORT TagValue
{
public:
    // constructor, destructor
    TagValue();
    TagValue(const std::string &text, TagTextEncoding encoding = TagTextEncoding::Latin1);
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

    // methods
    bool isEmpty() const;
    void clearData();
    void clearMetadata();
    void clearDataAndMetadata();
    TagDataType type() const;
    std::string toString() const;
    void toString(std::string &result) const;
    int32 toInteger() const;
    int toStandardGenreIndex() const;
    PositionInSet toPositionIntSet() const;
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
    TagTextEncoding descriptionEncoding() const;
    static const TagValue &empty();

    void assignText(const std::string &text, TagTextEncoding encoding = TagTextEncoding::Latin1);
    void assignInteger(int value);
    void assignStandardGenreIndex(int index);
    void assignData(const char *data, size_t length, TagDataType type = TagDataType::Binary, TagTextEncoding encoding = TagTextEncoding::Latin1);
    void assignData(std::unique_ptr<char[]> &&data, size_t length, TagDataType type = TagDataType::Binary, TagTextEncoding encoding = TagTextEncoding::Latin1);
    void assignPosition(PositionInSet value);
    void assignTimeSpan(ChronoUtilities::TimeSpan value);
    void assignDateTime(ChronoUtilities::DateTime value);


private:
    //char *_ptr;
    std::unique_ptr<char[]> m_ptr;
    std::string::size_type m_size;
    TagDataType m_type;
    std::string m_dec;
    std::string m_mimeType;
    std::string m_lng;
    bool m_labeledAsReadonly;
    TagTextEncoding m_encoding;
    TagTextEncoding m_descEncoding;
};

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
 * \brief Returns an indication whether an value is assigned.
 * \remarks Meta data such as description and mime type is not considered as an assigned value.
 */
inline bool TagValue::isEmpty() const
{
    return m_ptr == nullptr || m_size == 0;
}

/*!
 * \brief Clears the assigned data.
 * \remarks Meta data such as description and mime type remains unaffected.
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
 * \remarks Meta data such as description and mime type is not considered as part of the assigned value.
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
    return m_dec;
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
    m_dec = value;
    m_descEncoding = encoding;
}

/*!
 * \brief Returns the mime type.
 * \remarks The usage of this meta information depends on the tag implementation.
 * \sa setMimeType()
 */
inline const std::string &TagValue::mimeType() const
{
    return m_mimeType;
}

/*!
 * \brief Sets the mime type.
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
 * \remarks This method is only useful when a text is assigned.
 * \sa assignText()
 */
inline TagTextEncoding TagValue::dataEncoding() const
{
    return m_encoding;
}

/*!
 * \brief Returns the description encoding.
 * \remarks This method is only useful when a description is assigned.
 * \sa description()
 * \sa setDescription()
 */
inline TagTextEncoding TagValue::descriptionEncoding() const
{
    return m_descEncoding;
}

}

#endif // TAGVALUE_H
