#ifndef MP4TAGATOM_H
#define MP4TAGATOM_H

#include "../generictagfield.h"
#include "../statusprovider.h"

#include <c++utilities/io/binarywriter.h>
#include <c++utilities/conversion/stringconversion.h>

#include <vector>
#include <sstream>

namespace Media
{

/*!
 * \brief Encapsulates the most common data type IDs of MP4 tag fields.
 */
namespace RawDataType {
enum KnownValue : uint32
{
    Reserved = 0, /**< reserved for use where no type needs to be indicated */
    Utf8 = 1, /**< without any count or NULL terminator */
    Utf16 = 2, /**< also known as UTF-16BE */
    Sjis = 3, /**< S/JIS: deprecated unless it is needed for special Japanese characters */
    Utf8Sort = 4, /**< variant storage of a string for sorting only */
    Utf16Sort = 5, /**< variant storage of a string for sorting only */
    Html = 6, /**< the HTML file header specifies which HTML version */
    Xml = 7, /**< the XML header must identify the DTD or schemas */
    Uuid = 8, /**< also known as GUID; stored as 16 bytes in binary (valid as an ID) */
    Isrc = 9, /**< stored as UTF-8 text (valid as an ID) */
    Mi3p = 10, /**< stored as UTF-8 text (valid as an ID) */
    Gif = 12, /**< (deprecated) a GIF image */
    Jpeg = 13, /**< in a JFIF wrapper */
    Png = 14, /**< in a PNG wrapper */
    Url = 15, /**< absolute, in UTF-8 characters */
    Duration = 16, /**< in milliseconds, 32-bit integer */
    DateTime = 17, /**< in UTC, counting seconds since midnight, January 1, 1904; 32 or 64-bits */
    Genred = 18, /**< a list of enumerated values */
    BeSignedInt = 21, /**< the size of the integer is derived from the container size (max 4 byte assumed) */
    BeUnsignedInt = 22, /**< the size of the integer is derived from the container size (max 4 byte assumed) */
    BeFloat32 = 23, /**< IEEE754 */
    BeFloat64 = 24, /**< IEEE754 */
    Upc = 25, /**< Universal Product Code, in text UTF-8 format (valid as an ID) */
    Bmp = 27, /**< windows bitmap format graphics */
    QuickTimeMetadataAtom = 28, /**< a block of data having the structure of the Metadata atom defined in this specification */
    Undefined = 255 /**< a undefined */
};
}

class Mp4TagField;

/*!
 * \brief Defines traits for the TagField implementation of the Mp4TagField class.
 */
template <>
class TAG_PARSER_EXPORT TagFieldTraits<Mp4TagField>
{
public:
    typedef uint32 IdentifierType;
    typedef uint32 TypeInfoType;
};

class Mp4Atom;

class TAG_PARSER_EXPORT Mp4TagFieldMaker
{
    friend class Mp4TagField;

public:
    void make(std::ostream &stream);
    const Mp4TagField &field() const;
    uint64 requiredSize() const;

private:
    Mp4TagFieldMaker(Mp4TagField &field);

    Mp4TagField &m_field;
    std::stringstream m_convertedData;
    IoUtilities::BinaryWriter m_writer;
    uint32 m_rawDataType;
    uint64 m_dataSize;
    uint64 m_totalSize;
};

/*!
 * \brief Returns the associated field.
 */
inline const Mp4TagField &Mp4TagFieldMaker::field() const
{
    return m_field;
}

/*!
 * \brief Returns number of bytes which will be written when making the field.
 */
inline uint64 Mp4TagFieldMaker::requiredSize() const
{
   return m_totalSize;
}

class TAG_PARSER_EXPORT Mp4TagField : public TagField<Mp4TagField>, public StatusProvider
{
    friend class TagField<Mp4TagField>;

public:
    Mp4TagField();
    Mp4TagField(IdentifierType id, const TagValue &value);
    Mp4TagField(const std::string &mean, const std::string &name, const TagValue &value);

    void reparse(Mp4Atom &ilstChild);
    Mp4TagFieldMaker prepareMaking();
    void make(std::ostream &stream);

    bool isAdditionalTypeInfoUsed() const;
    const std::string &name() const;
    void setName(const std::string &name);
    const std::string &mean() const;
    void setMean(const std::string &mean);
    uint32 parsedRawDataType() const;
    uint16 countryIndicator() const;
    uint16 languageIndicator() const;
    bool supportsNestedFields() const;
    std::vector<uint32> expectedRawDataTypes() const;
    uint32 appropriateRawDataType() const;

    static IdentifierType fieldIdFromString(const char *idString, std::size_t idStringSize = std::string::npos);
    static std::string fieldIdToString(IdentifierType id);

protected:
    void cleared();

private:
    std::string m_name;
    std::string m_mean;
    uint32 m_parsedRawDataType;
    uint16 m_countryIndicator;
    uint16 m_langIndicator;
};

/*!
 * \brief Returns whether the additional type info is used.
 */
inline bool Mp4TagField::isAdditionalTypeInfoUsed() const
{
    return false;
}

/*!
 * \brief Returns the "name" for "extended" fields.
 */
inline const std::string &Mp4TagField::name() const
{
    return m_name;
}

/*!
 * \brief Sets the "name" for the "extended" field.
 */
inline void Mp4TagField::setName(const std::string &name)
{
    m_name = name;
}

/*!
 * \brief Returns the "mean" for "extended" fields.
 */
inline const std::string &Mp4TagField::mean() const
{
    return m_mean;
}

/*!
 * \brief Sets the "mean" for the "extended" field.
 */
inline void Mp4TagField::setMean(const std::string &mean)
{
    m_mean = mean;
}

/*!
 * \brief Returns the raw data type which has been determined when parsing.
 */
inline uint32 Mp4TagField::parsedRawDataType() const
{
    return m_parsedRawDataType;
}

/*!
 * \brief Returns the country indicator.
 */
inline uint16 Mp4TagField::countryIndicator() const
{
    return m_countryIndicator;
}

/*!
 * \brief Returns the language indicator.
 */
inline uint16 Mp4TagField::languageIndicator() const
{
    return m_langIndicator;
}

/*!
 * \brief Returns whether nested fields are supported.
 */
inline bool Mp4TagField::supportsNestedFields() const
{
    return false;
}

/*!
 * \brief Converts the specified ID string representation to an actual ID.
 * \remarks The specified \a idString is assumed to be UTF-8 encoded. In order to get the ©-sign
 *          correctly, it is converted to Latin-1.
 */
inline Mp4TagField::IdentifierType Mp4TagField::fieldIdFromString(const char *idString, std::size_t idStringSize)
{
    const auto latin1 = ConversionUtilities::convertUtf8ToLatin1(idString, idStringSize != std::string::npos ? idStringSize : std::strlen(idString));
    switch(latin1.second) {
    case 4:
        return ConversionUtilities::BE::toUInt32(latin1.first.get());
    default:
        throw ConversionUtilities::ConversionException("MP4 ID must be exactly 4 chars");
    }
}

/*!
 * \brief Returns the string representation for the specified \a id.
 * \remarks The specified \a id is considered Latin-1 encoded. In order to get the ©-sign
 *          correctly, it is converted to UTF-8.
 */
inline std::string Mp4TagField::fieldIdToString(Mp4TagField::IdentifierType id)
{
    const auto utf8 = ConversionUtilities::convertLatin1ToUtf8(ConversionUtilities::interpretIntegerAsString<uint32>(id).data(), 4);
    return std::string(utf8.first.get(), utf8.second);
}

}

#endif // MP4TAGATOM_H
