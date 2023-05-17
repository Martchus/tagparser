#ifndef TAG_PARSER_MP4TAGATOM_H
#define TAG_PARSER_MP4TAGATOM_H

#include "../generictagfield.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/binarywriter.h>

#include <cstdint>
#include <sstream>
#include <vector>

namespace TagParser {

/*!
 * \brief Encapsulates the most common data type IDs of MP4 tag fields.
 */
namespace RawDataType {
enum KnownValue : std::uint32_t {
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
class Diagnostics;

/*!
 * \brief Defines traits for the TagField implementation of the Mp4TagField class.
 */
template <> class TAG_PARSER_EXPORT TagFieldTraits<Mp4TagField> {
public:
    using IdentifierType = std::uint32_t;
    using TypeInfoType = std::uint32_t;
};

class Mp4Atom;

class TAG_PARSER_EXPORT Mp4TagFieldMaker {
    friend class Mp4TagField;

public:
    Mp4TagFieldMaker(Mp4TagFieldMaker &&) = default;
    void make(std::ostream &stream);
    const Mp4TagField &field() const;
    std::uint64_t requiredSize() const;

private:
    /// \cond
    struct Data {
        Data();
        Data(Data &&) = default;
        std::string_view rawData;
        std::stringstream convertedData;
        std::uint64_t size = 0;
        std::uint32_t rawType = 0;
        std::uint16_t countryIndicator = 0;
        std::uint16_t languageIndicator = 0;
    };
    /// \endcond

    Mp4TagFieldMaker(Mp4TagField &field, Diagnostics &diag);
    std::uint64_t prepareDataAtom(
        const TagValue &value, std::uint16_t countryIndicator, std::uint16_t languageIndicator, const std::string &context, Diagnostics &diag);

    Mp4TagField &m_field;
    CppUtilities::BinaryWriter m_writer;
    std::vector<Data> m_data;
    std::uint64_t m_totalSize;
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
inline std::uint64_t Mp4TagFieldMaker::requiredSize() const
{
    return m_totalSize;
}

class TAG_PARSER_EXPORT Mp4TagField : public TagField<Mp4TagField> {
    friend class TagField<Mp4TagField>;

public:
    struct AdditionalData {
        TagValue value;
        std::uint32_t rawDataType = 0;
        std::uint16_t countryIndicator = 0;
        std::uint16_t languageIndicator = 0;
    };

    Mp4TagField();
    Mp4TagField(IdentifierType id, const TagValue &value);
    Mp4TagField(std::string_view mean, std::string_view name, const TagValue &value);

    void reparse(Mp4Atom &ilstChild, Diagnostics &diag);
    Mp4TagFieldMaker prepareMaking(Diagnostics &diag);
    void make(std::ostream &stream, Diagnostics &diag);

    const std::vector<AdditionalData> &additionalData() const;
    std::vector<AdditionalData> &additionalData();
    bool isAdditionalTypeInfoUsed() const;
    const std::string &name() const;
    void setName(const std::string &name);
    const std::string &mean() const;
    void setMean(const std::string &mean);
    std::uint32_t parsedRawDataType() const;
    std::uint16_t countryIndicator() const;
    std::uint16_t languageIndicator() const;
    bool supportsNestedFields() const;
    std::vector<std::uint32_t> expectedRawDataTypes() const;
    std::uint32_t appropriateRawDataType() const;
    std::uint32_t appropriateRawDataTypeForValue(const TagValue &value) const;

    static IdentifierType fieldIdFromString(std::string_view idString);
    static std::string fieldIdToString(IdentifierType id);

private:
    void internallyClearValue();
    void internallyClearFurtherData();
    std::string m_name;
    std::string m_mean;
    std::vector<AdditionalData> m_additionalData;
    std::uint32_t m_parsedRawDataType;
    std::uint16_t m_countryIndicator;
    std::uint16_t m_langIndicator;
};

/*!
 * \brief Returns additional data (and the corresponding raw data type, country and language).
 * \remarks Some files seen in the wild have multiple data atoms. This function allows to access the data from additional atoms.
 */
inline const std::vector<Mp4TagField::AdditionalData> &Mp4TagField::additionalData() const
{
    return m_additionalData;
}

/*!
 * \brief Returns additional data (and the corresponding raw data type, country and language).
 * \remarks Some files seen in the wild have multiple data atoms. This function allows to access the data from additional atoms.
 */
inline std::vector<Mp4TagField::AdditionalData> &Mp4TagField::additionalData()
{
    return m_additionalData;
}

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
inline std::uint32_t Mp4TagField::parsedRawDataType() const
{
    return m_parsedRawDataType;
}

/*!
 * \brief Returns the country indicator.
 */
inline std::uint16_t Mp4TagField::countryIndicator() const
{
    return m_countryIndicator;
}

/*!
 * \brief Returns the language indicator.
 */
inline std::uint16_t Mp4TagField::languageIndicator() const
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
inline Mp4TagField::IdentifierType Mp4TagField::fieldIdFromString(std::string_view idString)
{
    const auto latin1 = CppUtilities::convertUtf8ToLatin1(idString.data(), idString.size());
    switch (latin1.second) {
    case 4:
        return CppUtilities::BE::toInt<std::uint32_t>(latin1.first.get());
    default:
        throw CppUtilities::ConversionException("MP4 ID must be exactly 4 chars");
    }
}

/*!
 * \brief Returns the string representation for the specified \a id.
 * \remarks The specified \a id is considered Latin-1 encoded. In order to get the ©-sign
 *          correctly, it is converted to UTF-8.
 */
inline std::string Mp4TagField::fieldIdToString(Mp4TagField::IdentifierType id)
{
    const auto utf8 = CppUtilities::convertLatin1ToUtf8(CppUtilities::interpretIntegerAsString<std::uint32_t>(id).data(), 4);
    return std::string(utf8.first.get(), utf8.second);
}

} // namespace TagParser

#endif // TAG_PARSER_MP4TAGATOM_H
