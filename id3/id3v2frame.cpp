#include "./id3v2frame.h"
#include "./id3genres.h"
#include "./id3v2frameids.h"

#include "../exceptions.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/conversion/stringbuilder.h>

#include <zlib.h>

#include <algorithm>
#include <cstring>
#include <memory>

using namespace std;
using namespace ConversionUtilities;
using namespace ChronoUtilities;
using namespace IoUtilities;

namespace Media {

namespace Id3v2TextEncodingBytes {
enum Id3v2TextEncodingByte : byte
{
    Ascii,
    Utf16WithBom,
    Utf16BigEndianWithoutBom,
    Utf8
};
}

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
    m_totalSize(0),
    m_padding(false)
{}

/*!
 * \brief Constructs a new Id3v2Frame with the specified \a id, \a value, \a group and \a flag.
 */
Id3v2Frame::Id3v2Frame(const IdentifierType &id, const TagValue &value, const byte group, const int16 flag) :
    TagField<Id3v2Frame>(id, value),
    m_flag(flag),
    m_group(group),
    m_parsedVersion(0),
    m_dataSize(0),
    m_totalSize(0),
    m_padding(false)
{}

/*!
 * \brief Helper function to parse the genre index.
 * \returns Returns the genre index or -1 if the specified string does not denote a genre index.
 */
template<class stringtype>
int parseGenreIndex(const stringtype &denotation)
{
    int index = -1;
    for(auto c : denotation) {
        if(index == -1) {
            switch(c) {
            case ' ':
                break;
            case '(':
                index = 0;
                break;
            case '\0':
                return -1;
            default:
                if(c >= '0' && c <= '9') {
                    index = c - '0';
                } else {
                    return -1;
                }
            }
        } else {
            switch(c) {
            case ')':
                return index;
            case '\0':
                return index;
            default:
                if(c >= '0' && c <= '9') {
                    index = index * 10 + c - '0';
                } else {
                    return -1;
                }
            }
        }
    }
    return index;
}

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
void Id3v2Frame::parse(BinaryReader &reader, const uint32 version, const uint32 maximalSize)
{
    invalidateStatus();
    clear();
    static const string defaultContext("parsing ID3v2 frame");
    string context;

    // parse header
    if(version < 3) {
        // parse header for ID3v2.1 and ID3v2.2
        // -> read ID
        setId(reader.readUInt24BE());
        if(id() & 0xFFFF0000u) {
            m_padding = false;
        } else {
            // padding reached
            m_padding = true;
            addNotification(NotificationType::Debug, "Frame ID starts with null-byte -> padding reached.", defaultContext);
            throw NoDataFoundException();
        }

        // -> update context
        context = "parsing " % frameIdString() + " frame";

        // -> read size, check whether frame is truncated
        m_dataSize = reader.readUInt24BE();
        m_totalSize = m_dataSize + 6;
        if(m_totalSize > maximalSize) {
            addNotification(NotificationType::Warning, "The frame is truncated and will be ignored.", context);
            throw TruncatedDataException();
        }

        // -> no flags/group in ID3v2.2
        m_flag = 0;
        m_group = 0;

    } else {
        // parse header for ID3v2.3 and ID3v2.4
        // -> read ID
        setId(reader.readUInt32BE());
        if(id() & 0xFF000000u) {
            m_padding = false;
        } else {
            // padding reached
            m_padding = true;
            addNotification(NotificationType::Debug, "Frame ID starts with null-byte -> padding reached.", defaultContext);
            throw NoDataFoundException();
        }

        // -> update context
        context = "parsing " % frameIdString() + " frame";

        // -> read size, check whether frame is truncated
        m_dataSize = version >= 4
                ? reader.readSynchsafeUInt32BE()
                : reader.readUInt32BE();
        m_totalSize = m_dataSize + 10;
        if(m_totalSize > maximalSize) {
            addNotification(NotificationType::Warning, "The frame is truncated and will be ignored.", context);
            throw TruncatedDataException();
        }

        // -> read flags and group
        m_flag = reader.readUInt16BE();
        m_group = hasGroupInformation() ? reader.readByte() : 0;
        if(isEncrypted()) {
            // encryption is not implemented
            addNotification(NotificationType::Critical, "Encrypted frames aren't supported.", context);
            throw VersionNotSupportedException();
        }
    }

    // frame size mustn't be 0
    if(m_dataSize <= 0) {
        addNotification(NotificationType::Critical, "The frame size is 0.", context);
        throw InvalidDataException();
    }

    // parse the data
    unique_ptr<char[]> buffer;

    // -> decompress data if compressed; otherwise just read it
    if(isCompressed()) {
        uLongf decompressedSize = version >= 4 ? reader.readSynchsafeUInt32BE() : reader.readUInt32BE();
        if(decompressedSize < m_dataSize) {
            addNotification(NotificationType::Critical, "The decompressed size is smaller than the compressed size.", context);
            throw InvalidDataException();
        }
        auto bufferCompressed = make_unique<char[]>(m_dataSize);;
        reader.read(bufferCompressed.get(), m_dataSize);
        buffer = make_unique<char[]>(decompressedSize);
        switch(uncompress(reinterpret_cast<Bytef *>(buffer.get()), &decompressedSize, reinterpret_cast<Bytef *>(bufferCompressed.get()), m_dataSize)) {
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
            break;
        default:
            addNotification(NotificationType::Critical, "Decompressing failed (unknown reason).", context);
            throw InvalidDataException();
        }
        m_dataSize = decompressedSize;
    } else {
        buffer = make_unique<char[]>(m_dataSize);
        reader.read(buffer.get(), m_dataSize);
    }

    // -> get tag value depending of field type
    if(Id3v2FrameIds::isTextFrame(id())) {
        // frame contains text
        TagTextEncoding dataEncoding = parseTextEncodingByte(*buffer.get()); // the first byte stores the encoding
        if((version >= 3 &&
            (id() == Id3v2FrameIds::lTrackPosition || id() == Id3v2FrameIds::lDiskPosition))
                || (version < 3 && id() == Id3v2FrameIds::sTrackPosition)) {
            // the track number or the disk number frame
            try {
                PositionInSet position;
                if(characterSize(dataEncoding) > 1) {
                    position = PositionInSet(parseWideString(buffer.get() + 1, m_dataSize - 1, dataEncoding));
                } else {
                    position = PositionInSet(parseString(buffer.get() + 1, m_dataSize - 1, dataEncoding));
                }
                value().assignPosition(position);
            } catch(const ConversionException &) {
                addNotification(NotificationType::Warning, "The value of track/disk position frame is not numeric and will be ignored.", context);
            }

        } else if((version >= 3 && id() == Id3v2FrameIds::lLength) || (version < 3 && id() == Id3v2FrameIds::sLength)) {
            // frame contains length
            try {
                string milliseconds;
                if(dataEncoding == TagTextEncoding::Utf16BigEndian || dataEncoding == TagTextEncoding::Utf16LittleEndian) {
                    const auto parsedStringRef = parseSubstring(buffer.get() + 1, m_dataSize - 1, dataEncoding);
                    const auto convertedStringData = dataEncoding == TagTextEncoding::Utf16BigEndian
                            ? convertUtf16BEToUtf8(get<0>(parsedStringRef), get<1>(parsedStringRef))
                            : convertUtf16LEToUtf8(get<0>(parsedStringRef), get<1>(parsedStringRef));
                    milliseconds = string(convertedStringData.first.get(), convertedStringData.second);
                } else { // Latin-1 or UTF-8
                    milliseconds = parseString(buffer.get() + 1, m_dataSize - 1, dataEncoding);
                }
                value().assignTimeSpan(TimeSpan::fromMilliseconds(stringToNumber<double>(milliseconds)));
            } catch (const ConversionException &) {
                addNotification(NotificationType::Warning, "The value of the length frame is not numeric and will be ignored.", context);
            }

        } else if((version >= 3 && id() == Id3v2FrameIds::lGenre) || (version < 3 && id() == Id3v2FrameIds::sGenre)) {
            // genre/content type
            int genreIndex;
            if(characterSize(dataEncoding) > 1) {
                auto genreDenotation = parseWideString(buffer.get() + 1, m_dataSize - 1, dataEncoding);
                genreIndex = parseGenreIndex(genreDenotation);
            } else {
                auto genreDenotation = parseString(buffer.get() + 1, m_dataSize - 1, dataEncoding);
                genreIndex = parseGenreIndex(genreDenotation);
            }
            if(genreIndex != -1) {
                // genre is specified as ID3 genre number
                value().assignStandardGenreIndex(genreIndex);
            } else {
                // genre is specified as string
                // string might be null terminated
                auto substr = parseSubstring(buffer.get() + 1, m_dataSize - 1, dataEncoding);
                value().assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
            }
        } else { // any other text frame
            // string might be null terminated
            auto substr = parseSubstring(buffer.get() + 1, m_dataSize - 1, dataEncoding);
            value().assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
        }

    } else if(version >= 3 && id() == Id3v2FrameIds::lCover) {
        // frame stores picture
        byte type;
        parsePicture(buffer.get(), m_dataSize, value(), type);
        setTypeInfo(type);

    } else if(version < 3 && id() == Id3v2FrameIds::sCover) {
        // frame stores legacy picutre
        byte type;
        parseLegacyPicture(buffer.get(), m_dataSize, value(), type);
        setTypeInfo(type);

    } else if(((version >= 3 && id() == Id3v2FrameIds::lComment) || (version < 3 && id() == Id3v2FrameIds::sComment))
              || ((version >= 3 && id() == Id3v2FrameIds::lUnsynchronizedLyrics) || (version < 3 && id() == Id3v2FrameIds::sUnsynchronizedLyrics))) {
        // comment frame or unsynchronized lyrics frame (these two frame types have the same structure)
        parseComment(buffer.get(), m_dataSize, value());

    } else {
        // unknown frame
        value().assignData(buffer.get(), m_dataSize, TagDataType::Undefined);
    }
}

/*!
 * \brief Prepares making.
 * \returns Returns a Id3v2FrameMaker object which can be used to actually make the frame.
 * \remarks The field must NOT be mutated after making is prepared when it is intended to actually
 *          make the field using the make method of the returned object.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 *
 * This method might be useful when it is necessary to know the size of the field before making it.
 */
Id3v2FrameMaker Id3v2Frame::prepareMaking(const uint32 version)
{
    return Id3v2FrameMaker(*this, version);
}

/*!
 * \brief Writes the frame to a stream using the specified \a writer and the
 *        specified ID3v2 \a version.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void Id3v2Frame::make(BinaryWriter &writer, const uint32 version)
{
    prepareMaking(version).make(writer);
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
    m_totalSize = 0;
    m_padding = false;
}

/*!
 * \class Media::Id3v2FrameMaker
 * \brief The Id3v2FrameMaker class helps making ID3v2 frames.
 *        It allows to calculate the required size.
 * \sa See Id3v2FrameMaker::prepareMaking() for more information.
 */

/*!
 * \brief Prepares making the specified \a frame.
 * \sa See Id3v2Frame::prepareMaking() for more information.
 */
Id3v2FrameMaker::Id3v2FrameMaker(Id3v2Frame &frame, const byte version) :
    m_frame(frame),
    m_frameId(m_frame.id()),
    m_version(version)
{
    m_frame.invalidateStatus();
    const string context("making " % m_frame.frameIdString() + " frame");

    // validate assigned data
    if(m_frame.value().isEmpty()) {
        m_frame.addNotification(NotificationType::Critical, "Cannot make an empty frame.", context);
        throw InvalidDataException();
    }
    if(m_frame.isEncrypted()) {
        m_frame.addNotification(NotificationType::Critical, "Cannot make an encrypted frame (isn't supported by this tagging library).", context);
        throw InvalidDataException();
    }
    if(m_frame.hasPaddingReached()) {
        m_frame.addNotification(NotificationType::Critical, "Cannot make a frame which is marked as padding.", context);
        throw InvalidDataException();
    }
    if(version < 3 && m_frame.isCompressed()) {
        m_frame.addNotification(NotificationType::Warning, "Compression is not supported by the version of ID3v2 and won't be applied.", context);
    }
    if(version < 3 && (m_frame.flag() || m_frame.group())) {
        m_frame.addNotification(NotificationType::Warning, "The existing flag and group information is not supported by the version of ID3v2 and will be ignored/discarted.", context);
    }

    // convert frame ID if necessary
    if(version >= 3) {
        if(Id3v2FrameIds::isShortId(m_frameId)) {
            // try to convert the short frame ID to its long equivalent
            if(!(m_frameId = Id3v2FrameIds::convertToLongId(m_frameId))) {
                m_frame.addNotification(NotificationType::Critical, "The short frame ID can't be converted to its long equivalent which is needed to use the frame in a newer version of ID3v2.", context);
                throw InvalidDataException();
            }
        }
    } else {
        if(Id3v2FrameIds::isLongId(m_frameId)) {
            // try to convert the long frame ID to its short equivalent
            if(!(m_frameId = Id3v2FrameIds::convertToShortId(m_frameId))) {
                m_frame.addNotification(NotificationType::Critical, "The long frame ID can't be converted to its short equivalent which is needed to use the frame in the old version of ID3v2.", context);
                throw InvalidDataException();
            }
        }
    }

    // make actual data depending on the frame ID
    try {
        if(Id3v2FrameIds::isTextFrame(m_frameId)) {
            // it is a text frame
            if((version >= 3 && (m_frameId == Id3v2FrameIds::lTrackPosition || m_frameId == Id3v2FrameIds::lDiskPosition))
                    || (version < 3 && m_frameId == Id3v2FrameIds::sTrackPosition)) {
                // track number or the disk number frame
                m_frame.makeString(m_data, m_decompressedSize, m_frame.value().toString(), TagTextEncoding::Latin1);
            } else if((version >= 3 && m_frameId == Id3v2FrameIds::lLength)
                      || (version < 3 && m_frameId == Id3v2FrameIds::sLength)) {
                // length frame
                m_frame.makeString(m_data, m_decompressedSize, ConversionUtilities::numberToString(m_frame.value().toTimeSpan().totalMilliseconds()), TagTextEncoding::Latin1);
            } else if(m_frame.value().type() == TagDataType::StandardGenreIndex && ((version >= 3 && m_frameId == Id3v2FrameIds::lGenre)
                                                                                    || (version < 3 && m_frameId == Id3v2FrameIds::sGenre))) {
                // pre-defined genre frame
                m_frame.makeString(m_data, m_decompressedSize, ConversionUtilities::numberToString(m_frame.value().toStandardGenreIndex()), TagTextEncoding::Latin1);
            } else {
                // any other text frame
                if(version <= 3 && m_frame.value().dataEncoding() == TagTextEncoding::Utf8) {
                    // UTF-8 is only supported by ID3v2.4, so convert back to UTF-16
                    m_frame.makeString(m_data, m_decompressedSize, m_frame.value().toString(TagTextEncoding::Utf16LittleEndian), TagTextEncoding::Utf16LittleEndian);
                } else {
                    // just keep encoding of the assigned value
                    m_frame.makeString(m_data, m_decompressedSize, m_frame.value().toString(), m_frame.value().dataEncoding());
                }
            }

        } else if((version >= 3 && m_frameId == Id3v2FrameIds::lCover) || (version < 3 && m_frameId == Id3v2FrameIds::sCover)) {
            // picture frame
            m_frame.makePictureConsideringVersion(m_data, m_decompressedSize, m_frame.value(), m_frame.isTypeInfoAssigned() ? m_frame.typeInfo() : 0, version);

        } else if(((version >= 3 && m_frameId == Id3v2FrameIds::lComment)
                   || (version < 3 && m_frameId == Id3v2FrameIds::sComment))
                  || ((version >= 3 && m_frameId == Id3v2FrameIds::lUnsynchronizedLyrics)
                      || (version < 3 && m_frameId == Id3v2FrameIds::sUnsynchronizedLyrics))) {
            // the comment frame or the unsynchronized lyrics frame
            m_frame.makeCommentConsideringVersion(m_data, m_decompressedSize, m_frame.value(), version);

        } else  {
            // an unknown frame
            m_data = make_unique<char[]>(m_decompressedSize = m_frame.value().dataSize());
            copy(m_frame.value().dataPointer(), m_frame.value().dataPointer() + m_decompressedSize, m_data.get());
        }
    } catch(const ConversionException &) {
        m_frame.addNotification(NotificationType::Critical, "Assigned value can not be converted appropriately.", context);
        throw InvalidDataException();
    }

    // apply compression if frame should be compressed
    if(version >= 3 && m_frame.isCompressed()) {
        m_dataSize = compressBound(m_decompressedSize);
        auto compressedData = make_unique<char[]>(m_decompressedSize);
        switch(compress(reinterpret_cast<Bytef *>(compressedData.get()), reinterpret_cast<uLongf *>(&m_dataSize), reinterpret_cast<Bytef *>(m_data.get()), m_decompressedSize)) {
        case Z_MEM_ERROR:
            m_frame.addNotification(NotificationType::Critical, "Decompressing failed. The source buffer was too small.", context);
            throw InvalidDataException();
        case Z_BUF_ERROR:
            m_frame.addNotification(NotificationType::Critical, "Decompressing failed. The destination buffer was too small.", context);
            throw InvalidDataException();
        case Z_OK:
            ;
        }
        m_data.swap(compressedData);
    } else {
        m_dataSize = m_decompressedSize;
    }

    // calculate required size
    // -> data size
    m_requiredSize = m_dataSize;
    if(version < 3) {
        // -> header size
        m_requiredSize += 6;
    } else {
        // -> header size
        m_requiredSize += 10;
        // -> group byte
        if(m_frame.hasGroupInformation()) {
            m_requiredSize += 1;
        }
        // -> decompressed size
        if(version >= 3 && m_frame.isCompressed()) {
            m_requiredSize += 4;
        }
    }
}

/*!
 * \brief Saves the frame (specified when constructing the object) using
 *        the specified \a writer.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw Media::Failure or a derived exception.
 */
void Id3v2FrameMaker::make(BinaryWriter &writer)
{
    if(m_version < 3) {
        writer.writeUInt24BE(m_frameId);
        writer.writeUInt24BE(m_dataSize);
    } else {
        writer.writeUInt32BE(m_frameId);
        if(m_version >= 4) {
            writer.writeSynchsafeUInt32BE(m_dataSize);
        } else {
            writer.writeUInt32BE(m_dataSize);
        }
        writer.writeUInt16BE(m_frame.flag());
        if(m_frame.hasGroupInformation()) {
            writer.writeByte(m_frame.group());
        }
        if(m_version >= 3 && m_frame.isCompressed()) {
            if(m_version >= 4) {
                writer.writeSynchsafeUInt32BE(m_decompressedSize);
            } else {
                writer.writeUInt32BE(m_decompressedSize);
            }
        }
    }
    writer.write(m_data.get(), m_dataSize);
}

/*!
 * \brief Returns the text encoding for the specified \a textEncodingByte.
 *
 * If the \a textEncodingByte doesn't match any encoding TagTextEncoding::Latin1 is
 * returned and a parsing notification is added.
 */
TagTextEncoding Id3v2Frame::parseTextEncodingByte(byte textEncodingByte)
{
    switch(textEncodingByte) {
    case Id3v2TextEncodingBytes::Ascii:
        return TagTextEncoding::Latin1;
    case Id3v2TextEncodingBytes::Utf16WithBom:
        return TagTextEncoding::Utf16LittleEndian;
    case Id3v2TextEncodingBytes::Utf16BigEndianWithoutBom:
        return TagTextEncoding::Utf16BigEndian;
    case Id3v2TextEncodingBytes::Utf8:
        return TagTextEncoding::Utf8;
    default:
        addNotification(NotificationType::Warning, "The charset of the frame is invalid. Latin-1 will be used.", "parsing encoding of frame " + frameIdString());
        return TagTextEncoding::Latin1;
    }
}

/*!
 * \brief Returns a text encoding byte for the specified \a textEncoding.
 */
byte Id3v2Frame::makeTextEncodingByte(TagTextEncoding textEncoding)
{
    switch(textEncoding) {
    case TagTextEncoding::Latin1:
        return Id3v2TextEncodingBytes::Ascii;
    case TagTextEncoding::Utf8:
        return Id3v2TextEncodingBytes::Utf8;
    case TagTextEncoding::Utf16LittleEndian:
        return Id3v2TextEncodingBytes::Utf16WithBom;
    case TagTextEncoding::Utf16BigEndian:
        return Id3v2TextEncodingBytes::Utf16WithBom;
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
 *          Unicode encodings).
 */
tuple<const char *, size_t, const char *> Id3v2Frame::parseSubstring(const char *buffer, std::size_t bufferSize, TagTextEncoding &encoding, bool addWarnings)
{
    tuple<const char *, size_t, const char *> res(buffer, 0, buffer + bufferSize);
    switch(encoding) {
    case TagTextEncoding::Unspecified:
    case TagTextEncoding::Latin1:
    case TagTextEncoding::Utf8: {
        if((bufferSize >= 3) && (ConversionUtilities::BE::toUInt24(buffer) == 0x00EFBBBF)) {
            if(encoding == TagTextEncoding::Latin1) {
                addNotification(NotificationType::Critical, "Denoted character set is Latin-1 but an UTF-8 BOM is present - assuming UTF-8.", "parsing frame " + frameIdString());
                encoding = TagTextEncoding::Utf8;
            }
            get<0>(res) += 3;
        }
        const char *pos = get<0>(res);
        for(; *pos != 0x00; ++pos) {
            if(pos < get<2>(res)) {
                ++get<1>(res);
            } else {
                if(addWarnings) {
                    addNotification(NotificationType::Warning, "String in frame is not terminated proberly.", "parsing termination of frame " + frameIdString());
                }
                break;
            }
        }
        get<2>(res) = pos + 1;
        break;
    }
    case TagTextEncoding::Utf16BigEndian:
    case TagTextEncoding::Utf16LittleEndian: {
        if(bufferSize >= 2) {
            switch(ConversionUtilities::LE::toUInt16(buffer)) {
            case 0xFEFF:
                if(encoding == TagTextEncoding::Utf16BigEndian) {
                    addNotification(NotificationType::Critical, "Denoted character set is UTF-16 Big Endian but UTF-16 Little Endian BOM is present - assuming UTF-16 LE.", "parsing frame " + frameIdString());
                    encoding = TagTextEncoding::Utf16LittleEndian;
                }
                get<0>(res) += 2;
                break;
            case 0xFFFE:
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
                    addNotification(NotificationType::Warning, "Wide string in frame is not terminated proberly.", "parsing termination of frame " + frameIdString());
                }
                break;
            }
        }
        get<2>(res) = reinterpret_cast<const char *>(pos + 1);
        break;
    }
    }
    return res;
}

/*!
 * \brief Parses a substring in the specified \a buffer.
 *
 * Same as Id3v2Frame::parseSubstring() but returns the substring as string object.
 */
string Id3v2Frame::parseString(const char *buffer, size_t dataSize, TagTextEncoding &encoding, bool addWarnings)
{
    auto substr = parseSubstring(buffer, dataSize, encoding, addWarnings);
    return string(get<0>(substr), get<1>(substr));
}

/*!
 * \brief Parses a substring in the specified \a buffer.
 *
 * Same as Id3v2Frame::parseSubstring() but returns the substring as u16string object
 *
 * \remarks Converts byte order to match host byte order (otherwise it wouldn't make much sense to use the resulting u16string).
 */
u16string Id3v2Frame::parseWideString(const char *buffer, size_t dataSize, TagTextEncoding &encoding, bool addWarnings)
{
    auto substr = parseSubstring(buffer, dataSize, encoding, addWarnings);
    u16string res(reinterpret_cast<u16string::const_pointer>(get<0>(substr)), get<1>(substr) / 2);
    TagValue::ensureHostByteOrder(res, encoding);
    return res;
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
void Id3v2Frame::parseBom(const char *buffer, size_t maxSize, TagTextEncoding &encoding)
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
            addNotification(NotificationType::Warning, "UTF-8 byte order mark found in text frame.", "parsing byte oder mark of frame " + frameIdString());
        }
    }
}

/*!
 * \brief Parses the ID3v2.2 picture from the specified \a buffer.
 * \param buffer Specifies the buffer holding the picture.
 * \param maxSize Specifies the maximal number of bytes to read from the buffer.
 * \param tagValue Specifies the tag value used to store the results.
 * \param typeInfo Specifies a byte used to store the type info.
 */
void Id3v2Frame::parseLegacyPicture(const char *buffer, std::size_t maxSize, TagValue &tagValue, byte &typeInfo)
{
    static const string context("parsing ID3v2.2 picture frame");
    if(maxSize < 6) {
        addNotification(NotificationType::Critical, "Picture frame is incomplete.", context);
        throw TruncatedDataException();
    }
    const char *end = buffer + maxSize;
    auto dataEncoding = parseTextEncodingByte(*buffer); // the first byte stores the encoding
    typeInfo = static_cast<unsigned char>(*(buffer + 4));
    auto substr = parseSubstring(buffer + 5, end - 5 - buffer, dataEncoding, true);
    tagValue.setDescription(string(get<0>(substr), get<1>(substr)), dataEncoding);
    if(get<2>(substr) >= end) {
        addNotification(NotificationType::Critical, "Picture frame is incomplete (actual data is missing).", context);
        throw TruncatedDataException();
    }
    tagValue.assignData(get<2>(substr), end - get<2>(substr), TagDataType::Picture, dataEncoding);
}

/*!
 * \brief Parses the ID3v2.3 picture from the specified \a buffer.
 * \param buffer Specifies the buffer holding the picture.
 * \param maxSize Specifies the maximal number of bytes to read from the buffer.
 * \param tagValue Specifies the tag value used to store the results.
 * \param typeInfo Specifies a byte used to store the type info.
 */
void Id3v2Frame::parsePicture(const char *buffer, std::size_t maxSize, TagValue &tagValue, byte &typeInfo)
{
    static const string context("parsing ID3v2.3 picture frame");
    const char *end = buffer + maxSize;
    auto dataEncoding = parseTextEncodingByte(*buffer); // the first byte stores the encoding
    auto mimeTypeEncoding = TagTextEncoding::Latin1;
    auto substr = parseSubstring(buffer + 1, maxSize - 1, mimeTypeEncoding, true);
    if(get<1>(substr)) {
        tagValue.setMimeType(string(get<0>(substr), get<1>(substr)));
    }
    if(get<2>(substr) >= end) {
        addNotification(NotificationType::Critical, "Picture frame is incomplete (type info, description and actual data are missing).", context);
        throw TruncatedDataException();
    }
    typeInfo = static_cast<unsigned char>(*get<2>(substr));
    if(++get<2>(substr) >= end) {
        addNotification(NotificationType::Critical, "Picture frame is incomplete (description and actual data are missing).", context);
        throw TruncatedDataException();
    }
    substr = parseSubstring(get<2>(substr), end - get<2>(substr), dataEncoding, true);
    tagValue.setDescription(string(get<0>(substr), get<1>(substr)), dataEncoding);
    if(get<2>(substr) >= end) {
        addNotification(NotificationType::Critical, "Picture frame is incomplete (actual data is missing).", context);
        throw TruncatedDataException();
    }
    tagValue.assignData(get<2>(substr), end - get<2>(substr), TagDataType::Picture, dataEncoding);
}

/*!
 * \brief Parses the comment/unsynchronized lyrics from the specified \a buffer.
 * \param buffer Specifies the buffer holding the picture.
 * \param dataSize Specifies the maximal number of bytes to read from the buffer.
 * \param tagValue Specifies the tag value used to store the results.
 */
void Id3v2Frame::parseComment(const char *buffer, std::size_t dataSize, TagValue &tagValue)
{
    static const string context("parsing comment/unsynchronized lyrics frame");
    const char *end = buffer + dataSize;
    if(dataSize < 5) {
        addNotification(NotificationType::Critical, "Comment frame is incomplete.", context);
        throw TruncatedDataException();
    }
    TagTextEncoding dataEncoding = parseTextEncodingByte(*buffer);
    if(*(++buffer)) {
        tagValue.setLanguage(string(buffer, 3));
    }
    auto substr = parseSubstring(buffer += 3, dataSize -= 4, dataEncoding, true);
    tagValue.setDescription(string(get<0>(substr), get<1>(substr)), dataEncoding);
    if(get<2>(substr) > end) {
        addNotification(NotificationType::Critical, "Comment frame is incomplete (description not terminated?).", context);
        throw TruncatedDataException();
    }
    substr = parseSubstring(get<2>(substr), end - get<2>(substr), dataEncoding, false);
    tagValue.assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
}

/*!
 * \brief Writes an encoding denoation and the specified string \a value to a \a buffer.
 * \param buffer Specifies the buffer.
 * \param bufferSize Specifies the size of \a buffer.
 * \param value Specifies the string to make.
 * \param encoding Specifies the encoding of the string to make.
 */
void Id3v2Frame::makeString(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const std::string &value, TagTextEncoding encoding)
{
    makeEncodingAndData(buffer, bufferSize, encoding, value.data(), value.size());
}

/*!
 * \brief Writes an encoding denoation and the specified \a data to a \a buffer.
 * \param buffer Specifies the buffer.
 * \param bufferSize Specifies the size of \a buffer.
 * \param encoding Specifies the data encoding.
 * \param data Specifies the data.
 * \param dataSize Specifies size of \a data.
 */
void Id3v2Frame::makeEncodingAndData(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, TagTextEncoding encoding, const char *data, std::size_t dataSize)
{
    // calculate buffer size and allocate buffer
    if(!data) {
        dataSize = 0;
    }
    char *bufferDataAddress;
    switch(encoding) {
    case TagTextEncoding::Latin1:
    case TagTextEncoding::Utf8:
    case TagTextEncoding::Unspecified: // assumption
        // allocate buffer
        buffer = make_unique<char[]>(bufferSize = 1 + dataSize + 1);
        buffer[0] = makeTextEncodingByte(encoding); // set text encoding byte
        bufferDataAddress = buffer.get() + 1;
        break;
    case TagTextEncoding::Utf16LittleEndian:
    case TagTextEncoding::Utf16BigEndian:
        // allocate buffer
        buffer = make_unique<char[]>(bufferSize = 1 + 2 + dataSize + 2);
        buffer[0] = makeTextEncodingByte(encoding); // set text encoding byte
        ConversionUtilities::LE::getBytes(encoding == TagTextEncoding::Utf16LittleEndian ? static_cast<uint16>(0xFEFF) : static_cast<uint16>(0xFFFE), buffer.get() + 1);
        bufferDataAddress = buffer.get() + 3;
        break;
    default:
        return;
    }

    // write string data
    if(dataSize) {
        copy(data, data + dataSize, bufferDataAddress);
    }
}

/*!
 * \brief Writes the BOM for the specified \a encoding to the specified \a buffer.
 * \returns Returns the number of bytes written to the buffer.
 */
size_t Id3v2Frame::makeBom(char *buffer, TagTextEncoding encoding)
{
    switch(encoding) {
    case TagTextEncoding::Utf16LittleEndian:
        ConversionUtilities::LE::getBytes(static_cast<uint16>(0xFEFF), buffer);
        return 2;
    case TagTextEncoding::Utf16BigEndian:
        ConversionUtilities::BE::getBytes(static_cast<uint16>(0xFEFF), buffer);
        return 2;
    default:
        return 0;
    }
}

/*!
 * \brief Writes the specified picture to the specified buffer (ID3v2.2 compatible).
 */
void Id3v2Frame::makeLegacyPicture(unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &picture, byte typeInfo)
{
    // determine description
    TagTextEncoding descriptionEncoding = picture.descriptionEncoding();
    StringData convertedDescription;
    string::size_type descriptionSize = picture.description().find("\0\0", 0, descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 2 : 1);
    if(descriptionSize == string::npos) {
        descriptionSize = picture.description().size();
    }
    if(descriptionEncoding == TagTextEncoding::Utf8) {
        // UTF-8 is only supported by ID3v2.4, so convert back to UTF-16
        descriptionEncoding = TagTextEncoding::Utf16LittleEndian;
        convertedDescription = convertUtf8ToUtf16LE(picture.description().data(), descriptionSize);
        descriptionSize = convertedDescription.second;
    }
    // calculate needed buffer size and create buffer
    const uint32 dataSize = picture.dataSize();
    buffer = make_unique<char[]>(bufferSize = 1             + 3            + 1                 + descriptionSize  + (descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 4 : 1) + dataSize);
    // note:                                  encoding byte + image format + picture type byte + description size + 1 or 2 null bytes (depends on encoding)                                                                                       + data size
    char *offset = buffer.get();
    // write encoding byte
    *offset = makeTextEncodingByte(descriptionEncoding);
    // write mime type
    const char *imageFormat;
    if(picture.mimeType() == "image/jpeg") {
        imageFormat = "JPG";
    } else if(picture.mimeType() == "image/png") {
        imageFormat = "PNG";
    } else if(picture.mimeType() == "image/gif") {
        imageFormat = "GIF";
    } else if(picture.mimeType() == "-->") {
        imageFormat = picture.mimeType().data();
    } else {
        imageFormat = "UND";
    }
    strncpy(++offset, imageFormat, 3);
    // write picture type
    *(offset += 3) = typeInfo;
    // write description
    offset += makeBom(offset + 1, descriptionEncoding);
    if(convertedDescription.first) {
        copy(convertedDescription.first.get(), convertedDescription.first.get() + descriptionSize, ++offset);
    } else {
        picture.description().copy(++offset, descriptionSize);
    }
    *(offset += descriptionSize) = 0x00; // terminate description and increase data offset
    if(descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian) {
        *(++offset) = 0x00;
    }
    // write actual data
    copy(picture.dataPointer(), picture.dataPointer() + picture.dataSize(), ++offset);
}

/*!
 * \brief Writes the specified picture to the specified buffer (ID3v2.3 compatible).
 */
void Id3v2Frame::makePicture(unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &picture, byte typeInfo)
{
    makePictureConsideringVersion(buffer, bufferSize, picture, typeInfo, 3);
}

/*!
 * \brief Writes the specified picture to the specified buffer.
 */
void Id3v2Frame::makePictureConsideringVersion(std::unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &picture, byte typeInfo, byte version)
{
    if(version < 3) {
        makeLegacyPicture(buffer, bufferSize, picture, typeInfo);
        return;
    }

    // determine description
    TagTextEncoding descriptionEncoding = picture.descriptionEncoding();
    StringData convertedDescription;
    string::size_type descriptionSize = picture.description().find("\0\0", 0, descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 2 : 1);
    if(descriptionSize == string::npos) {
        descriptionSize = picture.description().size();
    }
    if(version < 4 && descriptionEncoding == TagTextEncoding::Utf8) {
        // UTF-8 is only supported by ID3v2.4, so convert back to UTF-16
        descriptionEncoding = TagTextEncoding::Utf16LittleEndian;
        convertedDescription = convertUtf8ToUtf16LE(picture.description().data(), descriptionSize);
        descriptionSize = convertedDescription.second;
    }
    // determine mime-type
    string::size_type mimeTypeSize = picture.mimeType().find('\0');
    if(mimeTypeSize == string::npos) {
        mimeTypeSize = picture.mimeType().length();
    }
    // calculate needed buffer size and create buffer
    const uint32 dataSize = picture.dataSize();
    buffer = make_unique<char[]>(bufferSize = 1             + mimeTypeSize   + 1      + 1                 + descriptionSize  + (descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 4 : 1) + dataSize);
    // note:                                  encoding byte + mime type size + 0 byte + picture type byte + description size + 1 or 4 null bytes (depends on encoding)                                                                                       + data size
    char *offset = buffer.get();
    // write encoding byte
    *offset = makeTextEncodingByte(descriptionEncoding);
    // write mime type
    picture.mimeType().copy(++offset, mimeTypeSize);
    *(offset += mimeTypeSize) = 0x00; // terminate mime type
    // write picture type
    *(++offset) = typeInfo;
    // write description
    offset += makeBom(offset + 1, descriptionEncoding);
    if(convertedDescription.first) {
        copy(convertedDescription.first.get(), convertedDescription.first.get() + descriptionSize, ++offset);
    } else {
        picture.description().copy(++offset, descriptionSize);
    }
    *(offset += descriptionSize) = 0x00; // terminate description and increase data offset
    if(descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian) {
        *(++offset) = 0x00;
    }
    // write actual data
    copy(picture.dataPointer(), picture.dataPointer() + picture.dataSize(), ++offset);
}

/*!
 * \brief Writes the specified comment to the specified buffer.
 */
void Id3v2Frame::makeComment(unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &comment)
{
    makeCommentConsideringVersion(buffer, bufferSize, comment, 3);
}

/*!
 * \brief Writes the specified comment to the specified buffer.
 */
void Id3v2Frame::makeCommentConsideringVersion(unique_ptr<char[]> &buffer, uint32 &bufferSize, const TagValue &comment, byte version)
{
    static const string context("making comment frame");
    // check type and other values are valid
    TagTextEncoding encoding = comment.dataEncoding();
    if(!comment.description().empty() && encoding != comment.descriptionEncoding()) {
        addNotification(NotificationType::Critical, "Data enoding and description encoding aren't equal.", context);
        throw InvalidDataException();
    }
    const string &lng = comment.language();
    if(lng.length() > 3) {
        addNotification(NotificationType::Critical, "The language must be 3 bytes long (ISO-639-2).", context);
        throw InvalidDataException();
    }
    StringData convertedDescription;
    string::size_type descriptionSize = comment.description().find("\0\0", 0, encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian ? 2 : 1);
    if(descriptionSize == string::npos) {
        descriptionSize = comment.description().size();
    }
    if(version < 4 && encoding == TagTextEncoding::Utf8) {
        // UTF-8 is only supported by ID3v2.4, so convert back to UTF-16
        encoding = TagTextEncoding::Utf16LittleEndian;
        convertedDescription = convertUtf8ToUtf16LE(comment.description().data(), descriptionSize);
        descriptionSize = convertedDescription.second;
    }
    // calculate needed buffer size and create buffer
    const auto data = comment.toString(encoding);
    buffer = make_unique<char[]>(bufferSize = 1             + 3        + descriptionSize  + data.size() + (encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian ? 6 : 1) + data.size());
    // note:                     encoding byte              + language + description size + actual data size + BOMs and termination
    char *offset = buffer.get();
    // write encoding
    *offset = makeTextEncodingByte(encoding);
    // write language
    for(unsigned int i = 0; i < 3; ++i) {
        *(++offset) = (lng.length() > i) ? lng[i] : 0x00;
    }
    // write description
    offset += makeBom(offset + 1, encoding);
    if(convertedDescription.first) {
        copy(convertedDescription.first.get(), convertedDescription.first.get() + descriptionSize, ++offset);
    } else {
        comment.description().copy(++offset, descriptionSize);
    }
    offset += descriptionSize;
    *offset = 0x00; // terminate description and increase data offset
    if(encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian) {
        *(++offset) = 0x00;
    }
    // write actual data
    offset += makeBom(offset + 1, encoding);
    data.copy(++offset, data.size());
}

}
