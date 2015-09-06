#ifndef ID3V2FRAME_H
#define ID3V2FRAME_H

#include "id3v2frameids.h"

#include "tagparser/generictagfield.h"
#include "tagparser/tagvalue.h"
#include "tagparser/statusprovider.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/conversion/stringconversion.h>

#include <string>
#include <istream>
#include <ostream>
#include <vector>

namespace Media
{

class LIB_EXPORT Id3v2FrameHelper
{
public:
    Id3v2FrameHelper(const std::string &id, StatusProvider &provider);

    const std::string &id() const;
    TagTextEncoding parseTextEncodingByte(byte textEncodingByte);
    byte makeTextEncodingByte(TagTextEncoding textEncoding);
    std::tuple<const char *, size_t, const char *> parseSubstring(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding, bool addWarnings = false);
    std::string parseString(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding, bool addWarnings = false);
    std::wstring parseWideString(const char *buffer, std::size_t dataSize, TagTextEncoding &encoding, bool addWarnings = false);
    void parsePicture(const char *buffer, size_t maxSize, TagValue &tagValue, byte &typeInfo);
    void parseComment(const char *buffer, size_t maxSize, TagValue &tagValue);
    void parseBom(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding);
    void makeString(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const std::string &value, TagTextEncoding encoding);
    void makeEncodingAndData(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, TagTextEncoding encoding, const char *data, size_t m_dataSize);
    void makePicture(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &picture, byte typeInfo);
    void makeComment(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &comment);

private:
    std::string m_id;
    StatusProvider &m_statusProvider;
};

class Id3v2Frame;

/*!
 * \brief Defines traits for the TagField implementation of the Id3v2Frame class.
 */
template <>
class LIB_EXPORT TagFieldTraits<Id3v2Frame>
{
public:
    /*!
     * \brief Fields in an ID3 tag are identified by 32-bit unsigned integers.
     */
    typedef uint32 identifierType;

    /*!
     * \brief The type info is stored using bytes.
     */
    typedef byte typeInfoType;

    /*!
     * \brief The implementation type is Id3v2Frame.
     */
    typedef Id3v2Frame implementationType;
};

/*!
 * \brief Returns the ID of the current frame.
 */
inline const std::string &Id3v2FrameHelper::id() const
{
    return m_id;
}

class LIB_EXPORT Id3v2Frame : public TagField<Id3v2Frame>, public StatusProvider
{
    friend class TagField<Id3v2Frame>;

public:
    Id3v2Frame();
    Id3v2Frame(const identifierType &id, const TagValue &value, byte group = 0, int16 flag = 0);

    void parse(IoUtilities::BinaryReader &reader, int32 version, uint32 maximalSize = 0);
    void make(IoUtilities::BinaryWriter &writer, int32 version);

    bool isAdditionalTypeInfoUsed() const;
    bool isValid() const;
    bool hasPaddingReached() const;
    std::string frameIdString() const;
    int16 flag() const;
    void setFlag(int16 value);
    uint32 frameSize() const;
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

protected:
    void cleared();

private:
    uint16 m_flag;
    byte m_group;
    int32 m_parsedVersion;
    uint32 m_dataSize;
    uint32 m_frameSize;
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
 */
inline std::string Id3v2Frame::frameIdString() const
{
    if(Id3v2FrameIds::isLongId(id())) {
        return ConversionUtilities::interpretIntegerAsString<uint32>(id());
    } else {
        return ConversionUtilities::interpretIntegerAsString<uint32>(id(), 1);
    }
}

/*!
 * \brief Returns the flags.
 */
inline int16 Id3v2Frame::flag() const
{
    return m_flag;
}

/*!
 * \brief Sets the flags.
 */
inline void Id3v2Frame::setFlag(int16 value)
{
    m_flag = value;
}

/*!
 * \brief Returns the size of the frame in bytes.
 */
inline uint32 Id3v2Frame::frameSize() const
{
    return m_frameSize;
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
    return (m_flag & 0x8000) == 0x8000;
}

/*!
 * \brief Returns whether the frame is flaged to be discarded when it is unknown and the file (but NOT the tag) is altered.
 */
inline bool Id3v2Frame::toDiscardWhenUnknownAndFileIsAltered() const
{
    return (m_flag & 0x4000) == 0x4000;
}

/*!
 * \brief Returns whether the frame is flagged as read-only.
 */
inline bool Id3v2Frame::isReadOnly() const
{
    return (m_flag & 0x2000) == 0x2000;
}

/*!
 * \brief Returns whether the frame is compressed.
 */
inline bool Id3v2Frame::isCompressed() const
{
    return m_parsedVersion >= 4 ? (m_flag & 0x8) == 0x8 : (m_flag & 0x80) == 0x80;
}

/*!
 * \brief Returns whether the frame is encrypted.
 * \remarks Reading encrypted frames is not supported.
 */
inline bool Id3v2Frame::isEncrypted() const
{
    return m_parsedVersion >= 4 ? (m_flag & 0x4) == 0x8 : (m_flag & 0x40) == 0x40;
}

/*!
 * \brief Returns whether the frame contains group information.
 */
inline bool Id3v2Frame::hasGroupInformation() const
{
    return m_parsedVersion >= 4 ? (m_flag & 0x40) == 0x40 : (m_flag & 0x20) == 0x20;
}

/*!
 * \brief Returns whether the frame is unsynchronized.
 */
inline bool Id3v2Frame::isUnsynchronized() const
{
    return m_parsedVersion >= 4 ? (m_flag & 0x2) == 0x2 : false;
}

/*!
 * \brief Returns whether the frame has a data length indicator.
 */
inline bool Id3v2Frame::hasDataLengthIndicator() const
{
    return m_parsedVersion >= 4 ? (m_flag & 0x1) == 0x1 : isCompressed();
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

}

#endif // ID3V2FRAME_H
