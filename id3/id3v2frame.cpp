#include "id3v2frame.h"
#include "id3genres.h"
#include "id3v2frameids.h"

#include "tagparser/exceptions.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/misc/memory.h>

#include <zlib.h>

#include <algorithm>
#include <cstring>

using namespace std;
using namespace ConversionUtilities;
using namespace ChronoUtilities;
using namespace IoUtilities;

namespace Media {

/*!
 * \class Media::Id3v2Frame
 * \brief The Id3v2Frame class is used by Id3v2Tag to store the fields.
 */

/*!
 * \brief Constructs a new Id3v2Frame.
 */
Id3v2Frame::Id3v2Frame() :
    m_flag(0),
    m_group(0),
    m_parsedVersion(0),
    m_dataSize(0),
    m_frameSize(0),
    m_padding(false)
{}

/*!
 * \brief Constructs a new Id3v2Frame with the specified \a id, \a value, \a group and \a flag.
 */
Id3v2Frame::Id3v2Frame(const identifierType &id, const TagValue &value, byte group, int16 flag) :
    TagField<Id3v2Frame>(id, value),
    m_flag(flag),
    m_group(group),
    m_parsedVersion(0),
    m_dataSize(0),
    m_frameSize(0),
    m_padding(false)
{}

/*!
 * \brief Parses a frame from the stream read using the specified \a reader.
 *
 * The position of the current character in the input stream is expected to be
 * at the beginning of the frame to be parsed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Id3v2Frame::parse(BinaryReader &reader, int32 version, uint32 maximalSize)
{
    invalidateStatus();
    string context("parsing ID3v2 frame");
    Id3v2FrameHelper helper(frameIdString(), *this);
    if(version < 3) {
        // parse header for ID3v2.1 and ID3v2.2
        setId(reader.readUInt24BE());
        if((id() & 0xFFFF0000u) == 0) {
            m_padding = true;
            addNotification(NotificationType::Debug, "Frame ID starts with null-byte -> padding reached.", context);
            throw NoDataFoundException();
        } else {
            m_padding = false;
        }
        context = "parsing " + helper.id() + " frame";
        m_dataSize = reader.readUInt24BE();
        m_frameSize = m_dataSize + 6;
        if(m_frameSize > maximalSize) {
            addNotification(NotificationType::Warning, "The frame is truncated and will be ignored.", "parsing " + frameIdString() + " frame");
            throw TruncatedDataException();
        }
        m_flag = 0;
        m_group = 0;
    } else {
        // parse header for ID3v2.3 and ID3v2.4
        setId(reader.readUInt32BE());
        if((id() & 0xFF000000u) == 0) {
            m_padding = true;
            addNotification(NotificationType::Debug, "Frame ID starts with null-byte -> padding reached.", context);
            throw NoDataFoundException();
        } else {
            m_padding = false;
        }
        context = "parsing " + helper.id() + " frame";
        m_dataSize = version >= 4
                ? reader.readSynchsafeUInt32BE()
                : reader.readUInt32BE();
        m_frameSize = m_dataSize + 10;
        if(m_frameSize > maximalSize) {
            addNotification(NotificationType::Warning, "The frame is truncated and will be ignored.", context);
            throw TruncatedDataException();
        }
        m_flag = reader.readUInt16BE();
        m_group = hasGroupInformation() ? reader.readByte() : 0;
        if(isEncrypted()) {
            addNotification(NotificationType::Critical, "Encrypted frames aren't supported.", context);
            throw VersionNotSupportedException();
        }
    }
    if(m_dataSize <= 0) {
        addNotification(NotificationType::Critical, "The frame size is 0.", context);
        throw InvalidDataException();
    }
    // parse the data
    vector<char> buffer;
    if(isCompressed()) {
        // decompress compressed data
        uLongf decompressedSize = version >= 4 ? reader.readSynchsafeUInt32BE() : reader.readUInt32BE();
        if(decompressedSize < m_dataSize) {
            addNotification(NotificationType::Critical, "The decompressed size is smaller then the compressed size.", context);
            throw InvalidDataException();
        }
        vector<char> bufferCompressed;
        bufferCompressed.resize(m_dataSize);
        reader.read(bufferCompressed.data(), m_dataSize);
        buffer.resize(decompressedSize);
        switch(uncompress(reinterpret_cast<Bytef *>(buffer.data()), &decompressedSize, reinterpret_cast<Bytef *>(bufferCompressed.data()), m_dataSize)) {
        case Z_MEM_ERROR:
            addNotification(NotificationType::Critical, "Decompressing failed. The source buffer was too small.", context);
            throw InvalidDataException();
        case Z_BUF_ERROR:
            addNotification(NotificationType::Critical, "Decompressing failed. The destination buffer was too small.", context);
            throw InvalidDataException();
        case Z_DATA_ERROR:
            addNotification(NotificationType::Critical, "Decompressing failed. The input data was corrupted or incomplete.", context);
            throw InvalidDataException();
        case Z_OK:
            ;
        }
        m_dataSize = decompressedSize;
    } else {
        buffer.resize(m_dataSize);
        reader.read(buffer.data(), m_dataSize);
    }
    if(Id3v2FrameIds::isTextfield(id())) {
        // frame contains text
        TagTextEncoding dataEncoding = helper.parseTextEncodingByte(buffer.front()); // the first byte stores the encoding
        // the track number or the disk number frame
        if((version >= 3 &&
            (id() == Id3v2FrameIds::lTrackPosition || id() == Id3v2FrameIds::lDiskPosition))
                || (version < 3 && id() == Id3v2FrameIds::sTrackPosition)) {
            try {
                PositionInSet position;
                if(characterSize(dataEncoding) > 1) {
                    position = PositionInSet(helper.parseWideString(buffer.data() + 1, m_dataSize - 1, dataEncoding));
                } else {
                    position = PositionInSet(helper.parseString(buffer.data() + 1, m_dataSize - 1, dataEncoding));
                }
                value().assignPosition(position);
            } catch(ConversionException &) {
                addNotification(NotificationType::Warning, "The value of track/disk position frame is not numeric and will be ignored.", context);
            }
        // frame contains length
        } else if((version >= 3 && id() == Id3v2FrameIds::lLength) || (version < 3 && id() == Id3v2FrameIds::sLength)) {
            double milliseconds;
            try {
                if(characterSize(dataEncoding) > 1) {
                    wstring millisecondsStr = helper.parseWideString(buffer.data() + 1, m_dataSize - 1, dataEncoding);
                    milliseconds = ConversionUtilities::stringToNumber<double, wstring>(millisecondsStr, 10);
                } else {
                    milliseconds = ConversionUtilities::stringToNumber<double>(helper.parseString(buffer.data() + 1, m_dataSize - 1, dataEncoding), 10);
                }
                value().assignTimeSpan(TimeSpan::fromMilliseconds(milliseconds));
            } catch (ConversionException &) {
                addNotification(NotificationType::Warning, "The value of the length frame is not numeric and will be ignored.", context);
            }
        // genre/content type
        } else if((version >= 3 && id() == Id3v2FrameIds::lGenre) || (version < 3 && id() == Id3v2FrameIds::sGenre)) {
            int genreIndex;
            try {
                if(characterSize(dataEncoding) > 1) {
                    wstring indexStr = helper.parseWideString(buffer.data() + 1, m_dataSize - 1, dataEncoding);
                    if(indexStr.front() == L'(' && indexStr.back() == L')') {
                        indexStr = indexStr.substr(1, indexStr.length() - 2);
                    }
                    genreIndex = ConversionUtilities::stringToNumber<int, wstring>(indexStr, 10);
                } else {
                    string indexStr = helper.parseString(buffer.data() + 1, m_dataSize - 1, dataEncoding);
                    if(indexStr.front() == '(' && indexStr.back() == ')') {
                        indexStr = indexStr.substr(1, indexStr.length() - 2);
                    }
                    genreIndex = ConversionUtilities::stringToNumber<int>(indexStr, 10);
                }
                value().assignStandardGenreIndex(genreIndex); // genre is specified as ID3 genre number
            } catch(ConversionException &) {
                // genre is specified as string
                // string might be null terminated
                auto substr = helper.parseSubstring(buffer.data() + 1, m_dataSize - 1, dataEncoding);
                value().assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
            }
        } else { // any other text frame
            // string might be null terminated
            auto substr = helper.parseSubstring(buffer.data() + 1, m_dataSize - 1, dataEncoding);
            value().assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
        }
    // frame stores picture
    } else if((version >= 3 && id() == Id3v2FrameIds::lCover) || (version < 3 && id() == Id3v2FrameIds::sCover)) {
        byte type;
        helper.parsePicture(buffer.data(), m_dataSize, value(), type);
        setTypeInfo(type);
    // comment frame or unsynchronized lyrics frame (these two frame types have the same structure)
    } else if(((version >= 3 && id() == Id3v2FrameIds::lComment) || (version < 3 && id() == Id3v2FrameIds::sComment))
              || ((version >= 3 && id() == Id3v2FrameIds::lUnsynchronizedLyrics) || (version < 3 && id() == Id3v2FrameIds::sUnsynchronizedLyrics))) {
        helper.parseComment(buffer.data(), m_dataSize, value());
    // unknown frame
    } else {
        value().assignData(buffer.data(), m_dataSize, TagDataType::Undefined);
    }
}

/*!
 * \brief Writes the frame to a stream using the specified \a writer and the
 *        specified ID3v2 version.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void Id3v2Frame::make(IoUtilities::BinaryWriter &writer, int32 version)
{
    invalidateStatus();
    Id3v2FrameHelper helper(frameIdString(), *this);
    const string context("making " + helper.id() + " frame");
    // check if a valid frame can be build from the data
    if(value().isEmpty()) {
        addNotification(NotificationType::Critical, "Cannot make an empty frame.", context);
        throw InvalidDataException();
    }
    if(isEncrypted()) {
        addNotification(NotificationType::Critical, "Cannot make an encrypted frame (isn't supported by this tagging library).", context);
        throw InvalidDataException();
    }
    if(m_padding) {
        addNotification(NotificationType::Critical, "Cannot make a frame which is marked as padding.", context);
        throw InvalidDataException();
    }
    uint32 frameId = id();
    if(version >= 3) {
        if(Id3v2FrameIds::isShortId(frameId)) {
            // try to convert the short frame id to its long equivalent
            frameId = Id3v2FrameIds::convertToLongId(frameId);
            if(frameId == 0) {
                addNotification(NotificationType::Critical, "The short frame id can't be converted to its long equivalent which is needed to use the frame in a newer version of ID3v2.", context);
                throw InvalidDataException();
            }
        }
    } else {
        if(Id3v2FrameIds::isLongId(frameId)) {
            // try to convert the long frame id to its short equivalent
            frameId = Id3v2FrameIds::convertToShortId(frameId);
            if(frameId == 0) {
                addNotification(NotificationType::Critical, "The long frame id can't be converted to its short equivalent which is needed to use the frame in the old version of ID3v2.", context);
                throw InvalidDataException();
            }
        }
    }
    if(version < 3 && (m_flag != 0 || m_group != 0)) {
        addNotification(NotificationType::Warning, "The existing flag and group information is not supported by the version of ID3v2 and will be ignored/discarted.", context);
    }
    // create actual data, depending on the frame type
    unique_ptr<char[]> buffer;
    uint32 decompressedSize;
    // check if the frame to be written is a text frame
    try {
        if(Id3v2FrameIds::isTextfield(frameId)) {
            if((version >= 3 && (frameId == Id3v2FrameIds::lTrackPosition || frameId == Id3v2FrameIds::lDiskPosition))
                    || (version < 3 && frameId == Id3v2FrameIds::sTrackPosition)) {
                // the track number or the disk number frame
                helper.makeString(buffer, decompressedSize, value().toString(), TagTextEncoding::Latin1);
            } else if((version >= 3 && frameId == Id3v2FrameIds::lLength)
                      || (version < 3 && frameId == Id3v2FrameIds::sLength)) {
                // the length
                    helper.makeString(buffer, decompressedSize, ConversionUtilities::numberToString(value().toTimeSpan().totalMilliseconds()), TagTextEncoding::Latin1);
            } else if(value().type() == TagDataType::StandardGenreIndex && ((version >= 3 && frameId == Id3v2FrameIds::lGenre)
                      || (version < 3 && frameId == Id3v2FrameIds::sGenre))) {
                // genre/content type as standard genre index
                helper.makeString(buffer, decompressedSize, ConversionUtilities::numberToString(value().toStandardGenreIndex()), TagTextEncoding::Latin1);
            } else {
                // any other text frame
                helper.makeString(buffer, decompressedSize, value().toString(), value().dataEncoding()); // the same as a normal text frame
            }
        } else if((version >= 3 && frameId == Id3v2FrameIds::lCover)
                  || (version < 3 && frameId == Id3v2FrameIds::sCover)) {
            // picture frame
            helper.makePicture(buffer, decompressedSize, value(), isTypeInfoAssigned() ? typeInfo() : 0);
        } else if(((version >= 3 && id() == Id3v2FrameIds::lComment)
                   || (version < 3 && id() == Id3v2FrameIds::sComment))
                  || ((version >= 3 && id() == Id3v2FrameIds::lUnsynchronizedLyrics)
                      || (version < 3 && id() == Id3v2FrameIds::sUnsynchronizedLyrics))) {
            // the comment frame or the unsynchronized lyrics frame
            helper.makeComment(buffer, decompressedSize, value());
        } else  {
            // an unknown frame
            // create buffer
            buffer = make_unique<char[]>(decompressedSize = value().dataSize());
            // just write the data
            copy(value().dataPointer(), value().dataPointer() + value().dataSize(), buffer.get());
        }
    } catch(ConversionException &) {
        addNotification(NotificationType::Critical, "Assigned value can not be converted appropriately.", context);
        throw InvalidDataException();
    }
    unsigned long actualSize;
    if(version >= 3 && isCompressed()) {
        actualSize = compressBound(decompressedSize);
        auto compressedBuffer = make_unique<char[]>(actualSize);
        switch(compress(reinterpret_cast<Bytef *>(compressedBuffer.get()), &actualSize, reinterpret_cast<Bytef *>(buffer.get()), decompressedSize)) {
        case Z_MEM_ERROR:
            addNotification(NotificationType::Critical, "Decompressing failed. The source buffer was too small.", context);
            throw InvalidDataException();
        case Z_BUF_ERROR:
            addNotification(NotificationType::Critical, "Decompressing failed. The destination buffer was too small.", context);
            throw InvalidDataException();
        case Z_OK:
            ;
        }
        buffer.swap(compressedBuffer);
    } else {
        actualSize = decompressedSize;
    }
    if(version < 3) {
        writer.writeUInt24BE(frameId);
        writer.writeUInt24BE(actualSize);
    } else {
        writer.writeUInt32BE(frameId);
        if(version >= 4) {
            writer.writeSynchsafeUInt32BE(actualSize);
        } else {
            writer.writeUInt32BE(actualSize);
        }
        writer.writeUInt16BE(m_flag);
        if(hasGroupInformation()) {
            writer.writeByte(m_group);
        }
        if(isCompressed()) {
            if(version >= 4) {
                writer.writeSynchsafeUInt32BE(decompressedSize);
            } else {
                writer.writeUInt32BE(decompressedSize);
            }
        }
    }
    writer.write(buffer.get(), actualSize);
}

/*!
 * \brief Ensures the field is cleared.
 */
void Id3v2Frame::cleared()
{
    m_flag = 0;
    m_group = 0;
    m_parsedVersion = 0;
    m_dataSize = 0;
    m_frameSize = 0;
    m_padding = false;
}

/*!
 * \class Media::Id3v2FrameHelper
 * \brief The Id3v2FrameHelper class helps parsing and making ID3v2 frames.
 */

/*!
 * \brief The Id3v2FrameHelper class helps parsing and making ID3v2 frames.
 * \param id Specifies the identifier of the current frame (used to print warnings).
 * \param provider Specifies the status provider to store warnings.
 */
Id3v2FrameHelper::Id3v2FrameHelper(const std::string &id, StatusProvider &provider) :
    m_id(id),
    m_statusProvider(provider)
{}

/*!
 * \brief Returns the text encoding for the specified \a textEncodingByte.
 *
 * If the \a textEncodingByte doesn't match any encoding TagTextEncoding::Latin1 is
 * returned and a parsing notification is added.
 */
TagTextEncoding Id3v2FrameHelper::parseTextEncodingByte(byte textEncodingByte)
{
    switch(textEncodingByte) {
    case 0: // Ascii
        return TagTextEncoding::Latin1;
    case 1: // Utf 16 with bom
        return TagTextEncoding::Utf16LittleEndian;
    case 2: // Utf 16 without bom
        return TagTextEncoding::Utf16BigEndian;
    case 3: // Utf 8
        return TagTextEncoding::Utf8;
    default:
        m_statusProvider.addNotification(NotificationType::Warning, "The charset of the frame is invalid. Latin-1 will be used.", "parsing encoding of frame " + m_id);
        return TagTextEncoding::Latin1;
    }
}

/*!
 * \brief Returns a text encoding byte for the specified \a textEncoding.
 */
byte Id3v2FrameHelper::makeTextEncodingByte(TagTextEncoding textEncoding)
{
    switch(textEncoding) {
    case TagTextEncoding::Latin1:
        return 0;
    case TagTextEncoding::Utf8:
        return 3;
    case TagTextEncoding::Utf16LittleEndian:
        return 1;
    case TagTextEncoding::Utf16BigEndian:
        return 2;
    default:
        return 0;
    }
}

/*!
 * \brief Parses a substring in the specified \a buffer.
 *
 * This method ensures that byte order marks and termination characters for the specified \a encoding are omitted.
 * It might add a waring if the substring is not terminated.
 *
 * \param buffer Specifies a pointer to the possibly terminated string.
 * \param bufferSize Specifies the size of the string in byte.
 * \param encoding Specifies the encoding of the string. Might be adjusted if a byte order marks is found.
 * \param addWarnings Specifies whether warnings should be added to the status provider if the string is not terminated.
 * \returns Returns the start offset, the length of the string (without termination) and the end offset (after termination).
 * \remarks The length is always returned as the number of bytes, not as the number of characters (makes a difference for
 *          UTF-16 encodings).
 */
tuple<const char *, size_t, const char *> Id3v2FrameHelper::parseSubstring(const char *buffer, size_t bufferSize, TagTextEncoding &encoding, bool addWarnings)
{
    tuple<const char *, size_t, const char *> res(buffer, 0, buffer + bufferSize);
    switch(encoding) {
    case TagTextEncoding::Utf16BigEndian:
    case TagTextEncoding::Utf16LittleEndian: {
            if(bufferSize >= 2) {
                if(ConversionUtilities::LE::toUInt16(buffer) == 0xFEFF) {
                    encoding = TagTextEncoding::Utf16LittleEndian;
                    get<0>(res) += 2;
                } else if(ConversionUtilities::BE::toUInt16(buffer) == 0xFEFF) {
                    encoding = TagTextEncoding::Utf16BigEndian;
                    get<0>(res) += 2;
                }
            }
            const uint16 *pos = reinterpret_cast<const uint16 *>(get<0>(res));
            for(; *pos != 0x0000; ++pos) {
                if(pos < reinterpret_cast<const uint16 *>(get<2>(res))) {
                    get<1>(res) += 2;
                } else {
                    if(addWarnings) {
                        m_statusProvider.addNotification(NotificationType::Warning, "Wide string in frame is not terminated proberly.", "parsing termination of frame " + m_id);
                    }
                    break;
                }
            }
            get<2>(res) = reinterpret_cast<const char *>(++pos);
            break;
        }
    default: {
            if((bufferSize >= 3) && (ConversionUtilities::BE::toUInt24(buffer) == 0x00EFBBBF)) {
                get<0>(res) += 3;
                encoding = TagTextEncoding::Utf8;
            }
            const char *pos = get<0>(res);
            for(; *pos != 0x00; ++pos) {
                if(pos < get<2>(res)) {
                    ++get<1>(res);
                } else {
                    if(addWarnings) {
                        m_statusProvider.addNotification(NotificationType::Warning, "String in frame is not terminated proberly.", "parsing termination of frame " + m_id);
                    }
                    break;
                }
            }
            get<2>(res) = ++pos;
            break;
        }
    }
    return res;
}

/*!
 * \brief Parses a substring in the specified \a buffer.
 *
 * Same as Id3v2FrameHelper::parseSubstring() but returns the substring as string object.
 */
string Id3v2FrameHelper::parseString(const char *buffer, size_t dataSize, TagTextEncoding &encoding, bool addWarnings)
{
    auto substr = parseSubstring(buffer, dataSize, encoding, addWarnings);
    return string(get<0>(substr), get<1>(substr));
}

/*!
 * \brief Parses a substring in the specified \a buffer.
 *
 * Same as Id3v2FrameHelper::parseSubstring() but returns the substring as wstring object.
 */
wstring Id3v2FrameHelper::parseWideString(const char *buffer, size_t dataSize, TagTextEncoding &encoding, bool addWarnings)
{
    auto substr = parseSubstring(buffer, dataSize, encoding, addWarnings);
    return wstring(reinterpret_cast<wstring::const_pointer>(get<0>(substr)), get<1>(substr) / 2);
}

/*!
 * \brief Parses a byte order mark from the specified \a buffer.
 *
 * \param buffer Specifies the buffer holding the byte order mark.
 * \param maxSize Specifies the maximal number of bytes to read from the buffer.
 * \param encoding Specifies the encoding of the string. Might be reset if a byte order mark is found.
 *
 * \remarks This method is not used anymore and might be deleted.
 */
void Id3v2FrameHelper::parseBom(const char *buffer, size_t maxSize, TagTextEncoding &encoding)
{
    switch(encoding) {
    case TagTextEncoding::Utf16BigEndian:
    case TagTextEncoding::Utf16LittleEndian:
        if((maxSize >= 2) && (ConversionUtilities::BE::toUInt16(buffer) == 0xFFFE)) {
            encoding = TagTextEncoding::Utf16LittleEndian;
        } else if((maxSize >= 2) && (ConversionUtilities::BE::toUInt16(buffer) == 0xFEFF)) {
            encoding = TagTextEncoding::Utf16BigEndian;
        }
        break;
    default:
        if((maxSize >= 3) && (ConversionUtilities::BE::toUInt24(buffer) == 0x00EFBBBF)) {
            encoding = TagTextEncoding::Utf8;
            m_statusProvider.addNotification(NotificationType::Warning, "UTF-8 byte order mark found in text frame.", "parsing byte oder mark of frame " + m_id);
        }
    }
}

/*!
 * \brief Parses the picture from the specified \a buffer.
 * \param buffer Specifies the buffer holding the picture.
 * \param maxSize Specifies the maximal number of bytes to read from the buffer.
 * \param tagValue Specifies the tag value used to store the results.
 * \param typeInfo Specifies a byte used to store the type info.
 */
void Id3v2FrameHelper::parsePicture(const char *buffer, size_t maxSize, TagValue &tagValue, byte &typeInfo)
{
    static const string context("parsing ID3v2 picture frame");
    const char *end = buffer + maxSize;
    auto dataEncoding = parseTextEncodingByte(*buffer); // the first byte stores the encoding
    auto mimeTypeEnc = TagTextEncoding::Latin1; // MIME type shoud be encoded in Latin-1
    auto substr = parseSubstring(buffer + 1, maxSize - 1, mimeTypeEnc, true);
    if(get<1>(substr)) {
        tagValue.setMimeType(string(get<0>(substr), get<1>(substr)));
    }
    if(get<2>(substr) >= end) {
        m_statusProvider.addNotification(NotificationType::Critical, "Picture frame is incomplete (type info, description and actual data are missing).", context);
        throw TruncatedDataException();
    }
    typeInfo = static_cast<unsigned char>(*get<2>(substr));
    if(++get<2>(substr) >= end) {
        m_statusProvider.addNotification(NotificationType::Critical, "Picture frame is incomplete (description and actual data are missing).", context);
        throw TruncatedDataException();
    }
    substr = parseSubstring(get<2>(substr), end - get<2>(substr), dataEncoding, true);
    tagValue.setDescription(string(get<0>(substr), get<1>(substr)), dataEncoding);
    if(get<2>(substr) >= end) {
        m_statusProvider.addNotification(NotificationType::Critical, "Picture frame is incomplete (actual data is missing).", context);
        throw TruncatedDataException();
    }
    tagValue.assignData(get<2>(substr), end - get<2>(substr), TagDataType::Picture, dataEncoding);
}

/*!
 * \brief Parses the comment from the specified \a buffer.
 * \param buffer Specifies the buffer holding the picture.
 * \param dataSize Specifies the maximal number of bytes to read from the buffer.
 * \param tagValue Specifies the tag value used to store the results.
 */
void Id3v2FrameHelper::parseComment(const char *buffer, size_t dataSize, TagValue &tagValue)
{
    static const string context("parsing comment frame");
    const char *end = buffer + dataSize;
    if(dataSize < 6) {
        m_statusProvider.addNotification(NotificationType::Critical, "Comment frame is incomplete.", context);
        throw TruncatedDataException();
    }
    TagTextEncoding dataEncoding = parseTextEncodingByte(*buffer);
    tagValue.setLanguage(string(++buffer, 3));
    auto substr = parseSubstring(buffer += 3, dataSize -= 4, dataEncoding, true);
    tagValue.setDescription(string(get<0>(substr), get<1>(substr)), dataEncoding);
    if(get<2>(substr) >= end) {
        m_statusProvider.addNotification(NotificationType::Critical, "Comment frame is incomplete (description not terminated?).", context);
        throw TruncatedDataException();
    }
    substr = parseSubstring(get<2>(substr), end - get<2>(substr), dataEncoding, false);
    tagValue.assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
}

/*!
 * \brief Writes an encoding denoation and the specified string \a value to a \a buffer.
 * \param buffer Specifies the buffer.
 * \param value Specifies the string to make.
 * \param encoding Specifies the encoding of the string to make.
 */
void Id3v2FrameHelper::makeString(unique_ptr<char[]> &buffer, uint32 &bufferSize, const string &value, TagTextEncoding encoding)
{
    makeEncodingAndData(buffer, bufferSize, encoding, value.c_str(), value.length());
}

/*!
 * \brief Writes an encoding denoation and the specified \a data to a \a buffer.
 * \param buffer Specifies the buffer.
 * \param encoding Specifies the data encoding.
 * \param data Specifies the data.
 * \param dataSize Specifies the data size.
 */
void Id3v2FrameHelper::makeEncodingAndData(unique_ptr<char[]> &buffer, uint32 &bufferSize, TagTextEncoding encoding, const char *data, size_t dataSize)
{
    // calculate buffer size
    if(!data) {
        dataSize = 0;
    }
    switch(encoding) {
    case TagTextEncoding::Latin1:
    case TagTextEncoding::Utf8:
    case TagTextEncoding::Unspecified: // assumption
         // allocate buffer
        buffer = make_unique<char[]>(bufferSize = 1 + dataSize + 1);
        break;
    case TagTextEncoding::Utf16LittleEndian:
    case TagTextEncoding::Utf16BigEndian:
        // allocate buffer
        buffer = make_unique<char[]>(bufferSize = 1 + dataSize + 2);
        break;
    }
    buffer[0] = makeTextEncodingByte(encoding); // set text encoding byte
    if(dataSize > 0) {
        copy(data, data + dataSize, buffer.get() + 1); // write string data
    }
}

/*!
 * \brief Writes the specified picture to the specified buffer.
 */
void Id3v2FrameHelper::makePicture(unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &picture, byte typeInfo)
{
    // calculate needed buffer size and create buffer
    TagTextEncoding descriptionEncoding = picture.descriptionEncoding();
    uint32 dataSize = picture.dataSize();
    string::size_type mimeTypeLength = picture.mimeType().find('\0');
    if(mimeTypeLength == string::npos) {
        mimeTypeLength = picture.mimeType().length();
    }
    string::size_type descriptionLength = picture.description().find('\0');
    if(descriptionLength == string::npos) {
        descriptionLength = picture.description().length();
    }
    buffer = make_unique<char[]>(bufferSize = 1             + mimeTypeLength   + 1      + 1                 + descriptionLength  + (descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 2 : 1) + dataSize);
    // note:                                  encoding byte + mime type length + 0 byte + picture type byte + description length + 1 or 2 null bytes (depends on encoding)                                                                                       + data size
    char *offset = buffer.get();
    // write encoding byte
    *offset = makeTextEncodingByte(descriptionEncoding);
    // write mime type
    picture.mimeType().copy(++offset, mimeTypeLength);
    offset += mimeTypeLength;
    *offset = 0x00; // terminate mime type
    // write picture type
    *(++offset) = typeInfo;
    // write description
    picture.description().copy(++offset, descriptionLength);
    offset += descriptionLength;
    *offset = 0x00; // terminate description and increase data offset
    if(descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian) {
        *(++offset) = 0x00;
    }
    // write actual data
    copy(picture.dataPointer(), picture.dataPointer() + picture.dataSize(), ++offset);
}

/*!
 * \brief Writes the specified comment to the specified buffer.
 */
void Id3v2FrameHelper::makeComment(unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &comment)
{
    static const string context("making comment frame");
    // check type and other values are valid
    TagTextEncoding encoding = comment.dataEncoding();
    if(!comment.description().empty() && encoding != comment.descriptionEncoding()) {
        m_statusProvider.addNotification(NotificationType::Critical, "Data enoding and description encoding aren't equal.", context);
        throw InvalidDataException();
    }
    const string &lng = comment.language();
    if(lng.length() > 3) {
        m_statusProvider.addNotification(NotificationType::Critical, "The language must be 3 bytes long (ISO-639-2).", context);
        throw InvalidDataException();
    }
    // calculate needed buffer size and create buffer
    string::size_type descriptionLength = comment.description().find('\0');
    if(descriptionLength == string::npos)
        descriptionLength = comment.description().length();
    uint32 dataSize = comment.dataSize();
    buffer = make_unique<char[]>(bufferSize = 1             + 3        + descriptionLength  + (encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian ? 2 : 1) + dataSize);
    // note:                     encoding byte + language + description length + 1 or 2 null bytes                                                                                       + data size
    char *offset = buffer.get();
    // write encoding
    *offset = makeTextEncodingByte(encoding);
    // write language
    for(unsigned int i = 0; i < 3; ++i) {
        *(++offset) = (lng.length() > i) ? lng.at(i) : 0x00;
    }
    // write description
    comment.description().copy(++offset, descriptionLength);
    offset += descriptionLength;
    *offset = 0x00; // terminate description and increase data offset
    if(encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian) {
        *(++offset) = 0x00;
    }
    // write actual data
    const auto data = comment.toString();
    data.copy(++offset, data.length());
}

}
