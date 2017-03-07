#ifndef ID3V2FRAME_H
#define ID3V2FRAME_H

#include "./id3v2frameids.h"

#include "../generictagfield.h"
#include "../tagvalue.h"
#include "../statusprovider.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/conversion/stringconversion.h>

#include <string>
#include <iosfwd>
#include <vector>

namespace Media
{

class Id3v2Frame;

class TAG_PARSER_EXPORT Id3v2FrameMaker
{
    friend class Id3v2Frame;

public:
    void make(IoUtilities::BinaryWriter &writer);
    const Id3v2Frame &field() const;
    const std::unique_ptr<char[]> &data() const;
    uint32 dataSize() const;
    uint32 requiredSize() const;

private:
    Id3v2FrameMaker(Id3v2Frame &frame, const byte version);
    Id3v2Frame &m_frame;
    uint32 m_frameId;
    const byte m_version;
    std::unique_ptr<char[]> m_data;
    uint32 m_dataSize;
    uint32 m_decompressedSize;
    uint32 m_requiredSize;
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
inline uint32 Id3v2FrameMaker::dataSize() const
{
    return m_dataSize;
}

/*!
 * \brief Returns number of bytes which will be written when making the frame.
 */
inline uint32 Id3v2FrameMaker::requiredSize() const
{
    return m_requiredSize;
}

/*!
 * \brief Defines traits for the TagField implementation of the Id3v2Frame class.
 */
template <>
class TAG_PARSER_EXPORT TagFieldTraits<Id3v2Frame>
{
public:
    typedef uint32 IdentifierType;
    typedef byte TypeInfoType;
};

class TAG_PARSER_EXPORT Id3v2Frame : public TagField<Id3v2Frame>, public StatusProvider
{
    friend class TagField<Id3v2Frame>;

public:
    Id3v2Frame();
    Id3v2Frame(const IdentifierType &id, const TagValue &value, const byte group = 0, const int16 flag = 0);

    // parsing/making
    void parse(IoUtilities::BinaryReader &reader, const uint32 version, const uint32 maximalSize = 0);
    Id3v2FrameMaker prepareMaking(const uint32 version);
    void make(IoUtilities::BinaryWriter &writer, const uint32 version);

    // member access
    bool isAdditionalTypeInfoUsed() const;
    bool isValid() const;
    bool hasPaddingReached() const;
    std::string frameIdString() const;
    uint16 flag() const;
    void setFlag(uint16 value);
    uint32 totalSize() const;
    uint32 dataSize() const;
    bool toDiscardWhenUnknownAndTagIsAltered() const;
    bool toDiscardWhenUnknownAndFileIsAltered() const;
    bool isReadOnly() const;
    bool isCompressed() const;
    bool isEncrypted() const;
    bool hasGroupInformation() const;
    bool isUnsynchronized() const;
    bool hasDataLengthIndicator() const;
    byte group() const;
    void setGroup(byte value);
    int32 parsedVersion() const;
    bool supportsNestedFields() const;

    // parsing helper
    TagTextEncoding parseTextEncodingByte(byte textEncodingByte);
    std::tuple<const char *, size_t, const char *> parseSubstring(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding, bool addWarnings = false);
    std::string parseString(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding, bool addWarnings = false);
    std::u16string parseWideString(const char *buffer, std::size_t dataSize, TagTextEncoding &encoding, bool addWarnings = false);
    void parseLegacyPicture(const char *buffer, std::size_t maxSize, TagValue &tagValue, byte &typeInfo);
    void parsePicture(const char *buffer, std::size_t maxSize, TagValue &tagValue, byte &typeInfo);
    void parseComment(const char *buffer, std::size_t maxSize, TagValue &tagValue);
    void parseBom(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding);

    // making helper
    byte makeTextEncodingByte(TagTextEncoding textEncoding);
    std::size_t makeBom(char *buffer, TagTextEncoding encoding);
    void makeString(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const std::string &value, TagTextEncoding encoding);
    void makeEncodingAndData(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, TagTextEncoding encoding, const char *data, std::size_t m_dataSize);
    void makeLegacyPicture(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &picture, byte typeInfo);
    void makePicture(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &picture, byte typeInfo);
    void makePictureConsideringVersion(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &picture, byte typeInfo, byte version);
    void makeCommentConsideringVersion(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &comment, byte version);
    void makeComment(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &comment);

    static IdentifierType fieldIdFromString(const char *idString, std::size_t idStringSize = std::string::npos);
    static std::string fieldIdToString(IdentifierType id);

protected:
    void cleared();

private:
    uint16 m_flag;
    byte m_group;
    uint32 m_parsedVersion;
    uint32 m_dataSize;
    uint32 m_totalSize;
    bool m_padding;
};

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
 * \brief Returns the frame ID as string.
 * \deprecated Will be removed in favour of generic idToString().
 */
inline std::string Id3v2Frame::frameIdString() const
{
    return idToString();
}

/*!
 * \brief Returns the flags.
 */
inline uint16 Id3v2Frame::flag() const
{
    return m_flag;
}

/*!
 * \brief Sets the flags.
 */
inline void Id3v2Frame::setFlag(uint16 value)
{
    m_flag = value;
}

/*!
 * \brief Returns the total size of the frame in bytes.
 */
inline uint32 Id3v2Frame::totalSize() const
{
    return m_totalSize;
}

/*!
 * \brief Returns the size of the data stored in the frame in bytes.
 */
inline uint32 Id3v2Frame::dataSize() const
{
    return m_dataSize;
}

/*!
 * \brief Returns whether the frame is flaged to be discarded when it is unknown and the tag is altered.
 */
inline bool Id3v2Frame::toDiscardWhenUnknownAndTagIsAltered() const
{
    return m_flag & 0x8000;
}

/*!
 * \brief Returns whether the frame is flaged to be discarded when it is unknown and the file (but NOT the tag) is altered.
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
inline byte Id3v2Frame::group() const
{
    return m_group;
}

/*!
 * \brief Sets the group information.
 */
inline void Id3v2Frame::setGroup(byte value)
{
    m_group = value;
}

/*!
 * \brief Returns the version of the frame (read when parsing the frame).
 */
inline int32 Id3v2Frame::parsedVersion() const
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
inline Id3v2Frame::IdentifierType Id3v2Frame::fieldIdFromString(const char *idString, std::size_t idStringSize)
{
    switch(idStringSize != std::string::npos ? idStringSize : std::strlen(idString)) {
    case 3:
        return ConversionUtilities::BE::toUInt24(idString);
    case 4:
        return ConversionUtilities::BE::toUInt32(idString);
    default:
        throw ConversionUtilities::ConversionException("ID3v2 ID must be 3 or 4 chars");
    }
}

/*!
 * \brief Returns the string representation for the specified \a id.
 */
inline std::string Id3v2Frame::fieldIdToString(Id3v2Frame::IdentifierType id)
{
    return ConversionUtilities::interpretIntegerAsString<uint32>(id, Id3v2FrameIds::isLongId(id) ? 0 : 1);
}

}

#endif // ID3V2FRAME_H
