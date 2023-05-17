#ifndef TAG_PARSER_ID3V2FRAME_H
#define TAG_PARSER_ID3V2FRAME_H

#include "./id3v2frameids.h"

#include "../generictagfield.h"
#include "../tagvalue.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <iosfwd>
#include <string>
#include <vector>

namespace TagParser {

class Id3v2Frame;
class Diagnostics;

class TAG_PARSER_EXPORT Id3v2FrameMaker {
    friend class Id3v2Frame;

public:
    void make(CppUtilities::BinaryWriter &writer);
    const Id3v2Frame &field() const;
    const std::unique_ptr<char[]> &data() const;
    std::uint32_t dataSize() const;
    std::uint32_t requiredSize() const;

private:
    Id3v2FrameMaker(Id3v2Frame &frame, std::uint8_t version, Diagnostics &diag);
    void makeSubstring(const TagValue &value, Diagnostics &diag, const std::string &context);

    Id3v2Frame &m_frame;
    std::uint32_t m_frameId;
    const std::uint8_t m_version;
    std::unique_ptr<char[]> m_data;
    std::uint32_t m_dataSize;
    std::uint32_t m_decompressedSize;
    std::uint32_t m_requiredSize;
};

/*!
 * \brief Returns the associated frame.
 */
inline const Id3v2Frame &Id3v2FrameMaker::field() const
{
    return m_frame;
}

/*!
 * \brief Returns the frame data.
 */
inline const std::unique_ptr<char[]> &Id3v2FrameMaker::data() const
{
    return m_data;
}

/*!
 * \brief Returns the size of the array returned by data().
 */
inline std::uint32_t Id3v2FrameMaker::dataSize() const
{
    return m_dataSize;
}

/*!
 * \brief Returns number of bytes which will be written when making the frame.
 */
inline std::uint32_t Id3v2FrameMaker::requiredSize() const
{
    return m_requiredSize;
}

/*!
 * \brief Defines traits for the TagField implementation of the Id3v2Frame class.
 */
template <> class TAG_PARSER_EXPORT TagFieldTraits<Id3v2Frame> {
public:
    using IdentifierType = std::uint32_t;
    using TypeInfoType = std::uint8_t;
};

class TAG_PARSER_EXPORT Id3v2Frame : public TagField<Id3v2Frame> {
    friend class TagField<Id3v2Frame>;
    friend class Id3v2FrameMaker;

public:
    Id3v2Frame();
    Id3v2Frame(const IdentifierType &id, const TagValue &value, std::uint8_t group = 0, std::uint16_t flag = 0);

    // parsing/making
    void parse(CppUtilities::BinaryReader &reader, std::uint32_t version, std::uint32_t maximalSize, Diagnostics &diag);
    Id3v2FrameMaker prepareMaking(std::uint8_t version, Diagnostics &diag);
    void make(CppUtilities::BinaryWriter &writer, std::uint8_t version, Diagnostics &diag);

    // member access
    const std::vector<TagValue> &additionalValues() const;
    std::vector<TagValue> &additionalValues();
    bool isAdditionalTypeInfoUsed() const;
    bool isValid() const;
    bool hasPaddingReached() const;
    std::uint16_t flag() const;
    void setFlag(std::uint16_t value);
    std::uint32_t totalSize() const;
    std::uint32_t dataSize() const;
    bool toDiscardWhenUnknownAndTagIsAltered() const;
    bool toDiscardWhenUnknownAndFileIsAltered() const;
    bool isReadOnly() const;
    bool isCompressed() const;
    bool isEncrypted() const;
    bool hasGroupInformation() const;
    bool isUnsynchronized() const;
    bool hasDataLengthIndicator() const;
    std::uint8_t group() const;
    void setGroup(std::uint8_t value);
    std::uint32_t parsedVersion() const;
    bool supportsNestedFields() const;

    // parsing helper
    TagTextEncoding parseTextEncodingByte(std::uint8_t textEncodingByte, Diagnostics &diag);
    std::tuple<const char *, std::size_t, const char *> parseSubstring(
        const char *buffer, std::size_t maxSize, TagTextEncoding &encoding, bool addWarnings, Diagnostics &diag);
    std::string parseString(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding, bool addWarnings, Diagnostics &diag);
    std::u16string parseWideString(const char *buffer, std::size_t dataSize, TagTextEncoding &encoding, bool addWarnings, Diagnostics &diag);
    void parseLegacyPicture(const char *buffer, std::size_t maxSize, TagValue &tagValue, std::uint8_t &typeInfo, Diagnostics &diag);
    void parsePicture(const char *buffer, std::size_t maxSize, TagValue &tagValue, std::uint8_t &typeInfo, Diagnostics &diag);
    void parseComment(const char *buffer, std::size_t maxSize, TagValue &tagValue, Diagnostics &diag);
    void parseBom(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding, Diagnostics &diag);

    // making helper
    static std::uint8_t makeTextEncodingByte(TagTextEncoding textEncoding);
    static std::size_t makeBom(char *buffer, TagTextEncoding encoding);
    static void makeLegacyPicture(
        std::unique_ptr<char[]> &buffer, std::uint32_t &bufferSize, const TagValue &picture, std::uint8_t typeInfo, Diagnostics &diag);
    static void makePicture(std::unique_ptr<char[]> &buffer, std::uint32_t &bufferSize, const TagValue &picture, std::uint8_t typeInfo,
        std::uint8_t version, Diagnostics &diag);
    static void makeComment(
        std::unique_ptr<char[]> &buffer, std::uint32_t &bufferSize, const TagValue &comment, std::uint8_t version, Diagnostics &diag);

    static IdentifierType fieldIdFromString(std::string_view idString);
    static std::string fieldIdToString(IdentifierType id);

private:
    void internallyClearValue();
    void internallyClearFurtherData();
    std::string ignoreAdditionalValuesDiagMsg() const;

    std::vector<TagValue> m_additionalValues;
    std::uint32_t m_parsedVersion;
    std::uint32_t m_dataSize;
    std::uint32_t m_totalSize;
    std::uint16_t m_flag;
    std::uint8_t m_group;
    bool m_padding;
};

/*!
 * \brief Returns additional values.
 * \remarks Frames might allow to store multiple values, eg. ID3v2.4 text frames allow to store multiple strings.
 */
inline const std::vector<TagValue> &Id3v2Frame::additionalValues() const
{
    return m_additionalValues;
}

/*!
 * \brief Returns additional values.
 * \remarks Frames might allow to store multiple values, eg. ID3v2.4 text frames allow to store multiple strings.
 */
inline std::vector<TagValue> &Id3v2Frame::additionalValues()
{
    return m_additionalValues;
}

/*!
 * \brief Returns whether the instance uses the additional type info.
 */
inline bool Id3v2Frame::isAdditionalTypeInfoUsed() const
{
    return id() == Id3v2FrameIds::lCover || id() == Id3v2FrameIds::sCover;
}

/*!
 * \brief Returns whether the frame is valid.
 */
inline bool Id3v2Frame::isValid() const
{
    return !(id() == 0 || value().isEmpty() || m_padding);
}

/*!
 * \brief Returns whether the padding has reached.
 */
inline bool Id3v2Frame::hasPaddingReached() const
{
    return m_padding;
}

/*!
 * \brief Returns the flags.
 */
inline std::uint16_t Id3v2Frame::flag() const
{
    return m_flag;
}

/*!
 * \brief Sets the flags.
 */
inline void Id3v2Frame::setFlag(std::uint16_t value)
{
    m_flag = value;
}

/*!
 * \brief Returns the total size of the frame in bytes.
 */
inline std::uint32_t Id3v2Frame::totalSize() const
{
    return m_totalSize;
}

/*!
 * \brief Returns the size of the data stored in the frame in bytes.
 */
inline std::uint32_t Id3v2Frame::dataSize() const
{
    return m_dataSize;
}

/*!
 * \brief Returns whether the frame is flagged to be discarded when it is unknown and the tag is altered.
 */
inline bool Id3v2Frame::toDiscardWhenUnknownAndTagIsAltered() const
{
    return m_flag & 0x8000;
}

/*!
 * \brief Returns whether the frame is flagged to be discarded when it is unknown and the file (but NOT the tag) is altered.
 */
inline bool Id3v2Frame::toDiscardWhenUnknownAndFileIsAltered() const
{
    return m_flag & 0x4000;
}

/*!
 * \brief Returns whether the frame is flagged as read-only.
 */
inline bool Id3v2Frame::isReadOnly() const
{
    return m_flag & 0x2000;
}

/*!
 * \brief Returns whether the frame is compressed.
 */
inline bool Id3v2Frame::isCompressed() const
{
    return m_parsedVersion >= 4 ? m_flag & 0x8 : m_flag & 0x80;
}

/*!
 * \brief Returns whether the frame is encrypted.
 * \remarks Reading encrypted frames is not supported.
 */
inline bool Id3v2Frame::isEncrypted() const
{
    return m_parsedVersion >= 4 ? m_flag & 0x4 : m_flag & 0x40;
}

/*!
 * \brief Returns whether the frame contains group information.
 */
inline bool Id3v2Frame::hasGroupInformation() const
{
    return m_parsedVersion >= 4 ? m_flag & 0x40 : m_flag & 0x20;
}

/*!
 * \brief Returns whether the frame is unsynchronized.
 */
inline bool Id3v2Frame::isUnsynchronized() const
{
    return m_parsedVersion >= 4 ? m_flag & 0x2 : false;
}

/*!
 * \brief Returns whether the frame has a data length indicator.
 */
inline bool Id3v2Frame::hasDataLengthIndicator() const
{
    return m_parsedVersion >= 4 ? m_flag & 0x1 : isCompressed();
}

/*!
 * \brief Returns the group.
 * \sa hasGroupInformation()
 */
inline std::uint8_t Id3v2Frame::group() const
{
    return m_group;
}

/*!
 * \brief Sets the group information.
 */
inline void Id3v2Frame::setGroup(std::uint8_t value)
{
    m_group = value;
}

/*!
 * \brief Returns the version of the frame (read when parsing the frame).
 */
inline std::uint32_t Id3v2Frame::parsedVersion() const
{
    return m_parsedVersion;
}

/*!
 * \brief Returns whether nested fields are supported.
 */
inline bool Id3v2Frame::supportsNestedFields() const
{
    return true;
}

/*!
 * \brief Converts the specified ID string representation to an actual ID.
 */
inline Id3v2Frame::IdentifierType Id3v2Frame::fieldIdFromString(std::string_view idString)
{
    switch (idString.size()) {
    case 3:
        return CppUtilities::BE::toUInt24(idString.data());
    case 4:
        return CppUtilities::BE::toInt<std::uint32_t>(idString.data());
    default:
        throw CppUtilities::ConversionException("ID3v2 ID must be 3 or 4 chars");
    }
}

/*!
 * \brief Returns the string representation for the specified \a id.
 */
inline std::string Id3v2Frame::fieldIdToString(Id3v2Frame::IdentifierType id)
{
    return CppUtilities::interpretIntegerAsString<std::uint32_t>(id, Id3v2FrameIds::isLongId(id) ? 0 : 1);
}

} // namespace TagParser

#endif // TAG_PARSER_ID3V2FRAME_H
