#include "./id3v2frame.h"
#include "./id3v2frameids.h"

#include "../diagnostics.h"
#include "../exceptions.h"
#include "../tagtype.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>

#include <zlib.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>

using namespace std;
using namespace CppUtilities;
namespace TagParser {

/// \cond
namespace Id3v2TextEncodingBytes {
enum Id3v2TextEncodingByte : std::uint8_t { Ascii, Utf16WithBom, Utf16BigEndianWithoutBom, Utf8 };
}
/// \endcond

/// \brief The maximum (supported) size of an ID3v2Frame.
constexpr auto maxId3v2FrameDataSize(numeric_limits<std::uint32_t>::max() - 15);

/*!
 * \class TagParser::Id3v2Frame
 * \brief The Id3v2Frame class is used by Id3v2Tag to store the fields.
 */

/*!
 * \brief Constructs a new Id3v2Frame.
 */
Id3v2Frame::Id3v2Frame()
    : m_parsedVersion(0)
    , m_dataSize(0)
    , m_totalSize(0)
    , m_flag(0)
    , m_group(0)
    , m_padding(false)
{
}

/*!
 * \brief Constructs a new Id3v2Frame with the specified \a id, \a value, \a group and \a flag.
 */
Id3v2Frame::Id3v2Frame(const IdentifierType &id, const TagValue &value, std::uint8_t group, std::uint16_t flag)
    : TagField<Id3v2Frame>(id, value)
    , m_parsedVersion(0)
    , m_dataSize(0)
    , m_totalSize(0)
    , m_flag(flag)
    , m_group(group)
    , m_padding(false)
{
}

/*!
 * \brief Helper function to parse the genre index.
 * \returns Returns the genre index or -1 if the specified string does not denote a genre index.
 */
template <class stringtype> static int parseGenreIndex(const stringtype &denotation)
{
    auto index = -1;
    for (auto c : denotation) {
        if (index == -1) {
            switch (c) {
            case ' ':
                break;
            case '(':
                index = 0;
                break;
            case '\0':
                return -1;
            default:
                if (c >= '0' && c <= '9') {
                    index = c - '0';
                } else {
                    return -1;
                }
            }
        } else {
            switch (c) {
            case ')':
                return index;
            case '\0':
                return index;
            default:
                if (c >= '0' && c <= '9') {
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
 * \brief Returns an std::string instance for the substring parsed using parseSubstring().
 */
static std::string stringFromSubstring(std::tuple<const char *, std::size_t, const char *> substr)
{
    return std::string(std::get<0>(substr), std::get<1>(substr));
}

/*!
 * \brief Returns an std::u16string instance for the substring parsed using parseSubstring().
 */
static std::u16string wideStringFromSubstring(std::tuple<const char *, std::size_t, const char *> substr, TagTextEncoding encoding)
{
    std::u16string res(reinterpret_cast<u16string::const_pointer>(std::get<0>(substr)), std::get<1>(substr) / 2);
    TagValue::ensureHostByteOrder(res, encoding);
    return res;
}

/*!
 * \brief Reads the play counter from the specified range.
 */
static std::uint64_t readPlayCounter(const char *begin, const char *end, const std::string &context, Diagnostics &diag)
{
    auto res = std::uint64_t();
    auto pos = end - 1;
    if (end - begin > 8) {
        diag.emplace_back(DiagLevel::Critical, "Play counter is bigger than eight bytes and therefore not supported.", context);
        return res;
    }
    for (auto shift = 0; pos >= begin; shift += 8, --pos) {
        res += static_cast<std::uint64_t>(static_cast<std::uint8_t>(*pos)) << shift;
    }
    return res;
}

/*!
 * \brief Parses a frame from the stream read using the specified \a reader.
 *
 * The position of the current character in the input stream is expected to be
 * at the beginning of the frame to be parsed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Id3v2Frame::parse(BinaryReader &reader, std::uint32_t version, std::uint32_t maximalSize, Diagnostics &diag)
{
    static const string defaultContext("parsing ID3v2 frame");
    string context;

    // parse header
    if (version < 3) {
        // parse header for ID3v2.1 and ID3v2.2
        // -> read ID
        setId(reader.readUInt24BE());
        if (id() & 0xFFFF0000u) {
            m_padding = false;
        } else {
            // padding reached
            m_padding = true;
            throw NoDataFoundException();
        }

        // -> update context
        context = "parsing " % idToString() + " frame";

        // -> read size, check whether frame is truncated
        m_dataSize = reader.readUInt24BE();
        m_totalSize = m_dataSize + 6;
        if (m_totalSize > maximalSize) {
            diag.emplace_back(DiagLevel::Warning, "The frame is truncated and will be ignored.", context);
            throw TruncatedDataException();
        }

        // -> no flags/group in ID3v2.2
        m_flag = 0;
        m_group = 0;

    } else {
        // parse header for ID3v2.3 and ID3v2.4
        // -> read ID
        setId(reader.readUInt32BE());
        if (id() & 0xFF000000u) {
            m_padding = false;
        } else {
            // padding reached
            m_padding = true;
            throw NoDataFoundException();
        }

        // -> update context
        context = "parsing " % idToString() + " frame";

        // -> read size, check whether frame is truncated
        m_dataSize = version >= 4 ? reader.readSynchsafeUInt32BE() : reader.readUInt32BE();
        m_totalSize = m_dataSize + 10;
        if (m_totalSize > maximalSize) {
            diag.emplace_back(DiagLevel::Warning, "The frame is truncated and will be ignored.", context);
            throw TruncatedDataException();
        }

        // -> read flags and group
        m_flag = reader.readUInt16BE();
        m_group = hasGroupInformation() ? reader.readByte() : 0;
        if (isEncrypted()) {
            // encryption is not implemented
            diag.emplace_back(DiagLevel::Critical, "Encrypted frames aren't supported.", context);
            throw VersionNotSupportedException();
        }
    }

    // add a warning if a frame appears in an ID3v2 tag known not to support it
    if (version <= 3 && Id3v2FrameIds::isOnlyId3v24Id(version < 3 ? Id3v2FrameIds::convertToLongId(id()) : id())) {
        diag.emplace_back(DiagLevel::Warning,
            argsToString("The frame is only supported in ID3v2.4 and newer but the tag's version is ID3v2.", version, '.'), context);
    } else if (version > 3 && Id3v2FrameIds::isPreId3v24Id(id())) {
        diag.emplace_back(DiagLevel::Warning,
            argsToString("The frame is only supported in ID3v2.3 and older but the tag's version is ID3v2.", version, '.'), context);
    }

    // frame size mustn't be 0
    if (m_dataSize <= 0) {
        diag.emplace_back(DiagLevel::Warning, "The frame size is 0.", context);
        throw InvalidDataException();
    }

    // parse the data
    unique_ptr<char[]> buffer;

    // -> decompress data if compressed; otherwise just read it
    if (isCompressed()) {
        uLongf decompressedSize = version >= 4 ? reader.readSynchsafeUInt32BE() : reader.readUInt32BE();
        if (decompressedSize < m_dataSize) {
            diag.emplace_back(DiagLevel::Critical, "The decompressed size is smaller than the compressed size.", context);
            throw InvalidDataException();
        }
        const auto bufferCompressed = make_unique<char[]>(m_dataSize);
        reader.read(bufferCompressed.get(), m_dataSize);
        buffer = make_unique<char[]>(decompressedSize);
        switch (
            uncompress(reinterpret_cast<Bytef *>(buffer.get()), &decompressedSize, reinterpret_cast<Bytef *>(bufferCompressed.get()), m_dataSize)) {
        case Z_MEM_ERROR:
            diag.emplace_back(DiagLevel::Critical, "Decompressing failed. The source buffer was too small.", context);
            throw InvalidDataException();
        case Z_BUF_ERROR:
            diag.emplace_back(DiagLevel::Critical, "Decompressing failed. The destination buffer was too small.", context);
            throw InvalidDataException();
        case Z_DATA_ERROR:
            diag.emplace_back(DiagLevel::Critical, "Decompressing failed. The input data was corrupted or incomplete.", context);
            throw InvalidDataException();
        case Z_OK:
            break;
        default:
            diag.emplace_back(DiagLevel::Critical, "Decompressing failed (unknown reason).", context);
            throw InvalidDataException();
        }
        if (decompressedSize > maxId3v2FrameDataSize) {
            diag.emplace_back(DiagLevel::Critical, "The decompressed data exceeds the maximum supported frame size.", context);
            throw InvalidDataException();
        }
        m_dataSize = static_cast<std::uint32_t>(decompressedSize);
    } else {
        buffer = make_unique<char[]>(m_dataSize);
        reader.read(buffer.get(), m_dataSize);
    }

    // read tag value depending on frame ID/type
    if (Id3v2FrameIds::isTextFrame(id())) {
        // parse text encoding byte
        TagTextEncoding dataEncoding = parseTextEncodingByte(static_cast<std::uint8_t>(*buffer.get()), diag);

        // parse string values (since ID3v2.4 a text frame may contain multiple strings)
        const char *currentOffset = buffer.get() + 1;
        for (size_t currentIndex = 1; currentIndex < m_dataSize;) {
            // determine the next substring
            const auto substr(parseSubstring(currentOffset, m_dataSize - currentIndex, dataEncoding, false, diag));

            // handle case when string is empty
            if (!get<1>(substr)) {
                if (currentIndex == 1) {
                    value().clearDataAndMetadata();
                }
                currentIndex = static_cast<size_t>(get<2>(substr) - buffer.get());
                currentOffset = get<2>(substr);
                continue;
            }

            // determine the TagValue instance to store the value
            TagValue *const value = [&] {
                if (this->value().isEmpty()) {
                    return &this->value();
                }
                m_additionalValues.emplace_back();
                return &m_additionalValues.back();
            }();

            // apply further parsing for some text frame types (eg. convert track number to PositionInSet)
            if ((version >= 3 && (id() == Id3v2FrameIds::lTrackPosition || id() == Id3v2FrameIds::lDiskPosition))
                || (version < 3 && (id() == Id3v2FrameIds::sTrackPosition || id() == Id3v2FrameIds::sDiskPosition))) {
                // parse the track number or the disk number frame
                try {
                    if (characterSize(dataEncoding) > 1) {
                        value->assignPosition(PositionInSet(wideStringFromSubstring(substr, dataEncoding)));
                    } else {
                        value->assignPosition(PositionInSet(stringFromSubstring(substr)));
                    }
                } catch (const ConversionException &) {
                    diag.emplace_back(DiagLevel::Warning, "The value of track/disk position frame is not numeric and will be ignored.", context);
                }

            } else if ((version >= 3 && id() == Id3v2FrameIds::lLength) || (version < 3 && id() == Id3v2FrameIds::sLength)) {
                // parse frame contains length
                try {
                    const auto milliseconds = [&] {
                        if (dataEncoding == TagTextEncoding::Utf16BigEndian || dataEncoding == TagTextEncoding::Utf16LittleEndian) {
                            const auto parsedStringRef = parseSubstring(buffer.get() + 1, m_dataSize - 1, dataEncoding, false, diag);
                            const auto convertedStringData = dataEncoding == TagTextEncoding::Utf16BigEndian
                                ? convertUtf16BEToUtf8(get<0>(parsedStringRef), get<1>(parsedStringRef))
                                : convertUtf16LEToUtf8(get<0>(parsedStringRef), get<1>(parsedStringRef));
                            return string(convertedStringData.first.get(), convertedStringData.second);
                        } else { // Latin-1 or UTF-8
                            return stringFromSubstring(substr);
                        }
                    }();
                    value->assignTimeSpan(TimeSpan::fromMilliseconds(stringToNumber<double>(milliseconds)));
                } catch (const ConversionException &) {
                    diag.emplace_back(DiagLevel::Warning, "The value of the length frame is not numeric and will be ignored.", context);
                }

            } else if ((version >= 3 && id() == Id3v2FrameIds::lGenre) || (version < 3 && id() == Id3v2FrameIds::sGenre)) {
                // parse genre/content type
                const auto genreIndex = [&] {
                    if (characterSize(dataEncoding) > 1) {
                        return parseGenreIndex(wideStringFromSubstring(substr, dataEncoding));
                    } else {
                        return parseGenreIndex(stringFromSubstring(substr));
                    }
                }();
                if (genreIndex != -1) {
                    // genre is specified as ID3 genre number
                    value->assignStandardGenreIndex(genreIndex);
                } else {
                    // genre is specified as string
                    value->assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
                }
            } else {
                // store any other text frames as-is
                value->assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
            }

            currentIndex = static_cast<size_t>(get<2>(substr) - buffer.get());
            currentOffset = get<2>(substr);
        }

        // add warning about additional values
        if (version < 4 && !m_additionalValues.empty()) {
            diag.emplace_back(
                DiagLevel::Warning, "Multiple strings found though the tag is pre-ID3v2.4. " + ignoreAdditionalValuesDiagMsg(), context);
        }

    } else if (version >= 3 && id() == Id3v2FrameIds::lCover) {
        // parse picture frame
        std::uint8_t type;
        parsePicture(buffer.get(), m_dataSize, value(), type, diag);
        setTypeInfo(type);

    } else if (version < 3 && id() == Id3v2FrameIds::sCover) {
        // parse legacy picutre
        std::uint8_t type;
        parseLegacyPicture(buffer.get(), m_dataSize, value(), type, diag);
        setTypeInfo(type);

    } else if (((version >= 3 && id() == Id3v2FrameIds::lComment) || (version < 3 && id() == Id3v2FrameIds::sComment))
        || ((version >= 3 && id() == Id3v2FrameIds::lUnsynchronizedLyrics) || (version < 3 && id() == Id3v2FrameIds::sUnsynchronizedLyrics))) {
        // parse comment frame or unsynchronized lyrics frame (these two frame types have the same structure)
        parseComment(buffer.get(), m_dataSize, value(), diag);

    } else if (((version >= 3 && id() == Id3v2FrameIds::lPlayCounter) || (version < 3 && id() == Id3v2FrameIds::sPlayCounter))) {
        // parse play counter frame
        value().assignUnsignedInteger(readPlayCounter(buffer.get(), buffer.get() + m_dataSize, context, diag));

    } else if (((version >= 3 && id() == Id3v2FrameIds::lRating) || (version < 3 && id() == Id3v2FrameIds::sRating))) {
        // parse popularimeter frame
        auto popularity = Popularity{ .scale = TagType::Id3v2Tag };
        auto userEncoding = TagTextEncoding::Latin1;
        auto substr = parseSubstring(buffer.get(), m_dataSize, userEncoding, true, diag);
        auto end = buffer.get() + m_dataSize;
        if (std::get<1>(substr)) {
            popularity.user.assign(std::get<0>(substr), std::get<1>(substr));
        }
        auto ratingPos = std::get<2>(substr);
        if (ratingPos >= end) {
            diag.emplace_back(DiagLevel::Critical, "Popularimeter frame is incomplete (rating is missing).", context);
            throw TruncatedDataException();
        }
        popularity.rating = static_cast<std::uint8_t>(*ratingPos);
        popularity.playCounter = readPlayCounter(ratingPos + 1, end, context, diag);
        value().assignPopularity(popularity);

    } else {
        // parse unknown/unsupported frame
        value().assignData(buffer.get(), m_dataSize, TagDataType::Undefined);
    }
}

/*!
 * \brief Prepares making.
 * \returns Returns a Id3v2FrameMaker object which can be used to actually make the frame.
 * \remarks The field must NOT be mutated after making is prepared when it is intended to actually
 *          make the field using the make method of the returned object.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 *
 * This method might be useful when it is necessary to know the size of the field before making it.
 */
Id3v2FrameMaker Id3v2Frame::prepareMaking(std::uint8_t version, Diagnostics &diag)
{
    return Id3v2FrameMaker(*this, version, diag);
}

/*!
 * \brief Writes the frame to a stream using the specified \a writer and the
 *        specified ID3v2 \a version.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 */
void Id3v2Frame::make(BinaryWriter &writer, std::uint8_t version, Diagnostics &diag)
{
    prepareMaking(version, diag).make(writer);
}

/*!
 * \brief Clears ID3v2-specific values. Called via clear() and clearValue().
 */
void Id3v2Frame::internallyClearValue()
{
    value().clearDataAndMetadata();
    m_additionalValues.clear();
}

/*!
 * \brief Clears ID3v2-specific values. Called via clear().
 */
void Id3v2Frame::internallyClearFurtherData()
{
    m_flag = 0;
    m_group = 0;
    m_parsedVersion = 0;
    m_dataSize = 0;
    m_totalSize = 0;
    m_padding = false;
}

/*!
 * \brief Returns a diag message that additional values are ignored.
 */
std::string Id3v2Frame::ignoreAdditionalValuesDiagMsg() const
{
    if (m_additionalValues.size() == 1) {
        return argsToString("Additional value \"", m_additionalValues.front().toString(TagTextEncoding::Utf8), "\" is supposed to be ignored.");
    }
    return argsToString("Additional values ", DiagMessage::formatList(TagValue::toStrings(m_additionalValues)), " are supposed to be ignored.");
}

/*!
 * \brief Computes the size required to serialize the specified \a playCounter value.
 */
static std::uint32_t computePlayCounterSize(std::uint64_t playCounter)
{
    auto res = 4u;
    for (playCounter >>= 32; playCounter; playCounter >>= 8, ++res)
        ; // additional bytes for play counter into account when it is > 0xFFFFFFFF
    return res;
}

/*!
 * \brief Writes the specified \a playCounter with the specified \a playCounterSize to the buffer specified
 *        by the \a last address to write the data to.
 */
static void writePlayCounter(char *last, std::uint32_t playCounterSize, std::uint64_t playCounter)
{
    for (; playCounter || playCounterSize; playCounter >>= 8, --playCounterSize, --last) {
        *last = static_cast<char>(playCounter & 0xFF);
    }
}

/*!
 * \class TagParser::Id3v2FrameMaker
 * \brief The Id3v2FrameMaker class helps making ID3v2 frames.
 *        It allows to calculate the required size.
 * \sa See Id3v2FrameMaker::prepareMaking() for more information.
 */

/*!
 * \brief Prepares making the specified \a frame.
 * \sa See Id3v2Frame::prepareMaking() for more information.
 */
Id3v2FrameMaker::Id3v2FrameMaker(Id3v2Frame &frame, std::uint8_t version, Diagnostics &diag)
    : m_frame(frame)
    , m_frameId(m_frame.id())
    , m_version(version)
{
    const string context("making " % m_frame.idToString() + " frame");

    // validate frame's configuration
    if (m_frame.isEncrypted()) {
        diag.emplace_back(DiagLevel::Critical, "Cannot make an encrypted frame (isn't supported by this tagging library).", context);
        throw InvalidDataException();
    }
    if (m_frame.hasPaddingReached()) {
        diag.emplace_back(DiagLevel::Critical, "Cannot make a frame which is marked as padding.", context);
        throw InvalidDataException();
    }
    if (version < 3 && m_frame.isCompressed()) {
        diag.emplace_back(DiagLevel::Warning, "Compression is not supported by the version of ID3v2 and won't be applied.", context);
    }
    if (version < 3 && (m_frame.flag() || m_frame.group())) {
        diag.emplace_back(DiagLevel::Warning,
            "The existing flag and group information is not supported by the version of ID3v2 and will be ignored/discarted.", context);
    }

    // get non-empty, assigned values
    vector<const TagValue *> values;
    values.reserve(1 + frame.additionalValues().size());
    if (!frame.value().isEmpty()) {
        values.emplace_back(&frame.value());
    }
    for (const auto &value : frame.additionalValues()) {
        if (!value.isEmpty()) {
            values.emplace_back(&value);
        }
    }

    // validate assigned values
    if (values.empty()) {
        throw NoDataProvidedException();
        // note: This is not really an issue because in the case we're not provided with any value here just means that the field
        //       is supposed to be removed. So don't add any diagnostic messages here.
    }
    const bool isTextFrame = Id3v2FrameIds::isTextFrame(m_frameId);
    if (values.size() != 1) {
        if (!isTextFrame) {
            diag.emplace_back(DiagLevel::Critical, "Multiple values are not supported for non-text-frames.", context);
            throw InvalidDataException();
        } else if (version < 4) {
            diag.emplace_back(
                DiagLevel::Warning, "Multiple strings assigned to pre-ID3v2.4 text frame. " + frame.ignoreAdditionalValuesDiagMsg(), context);
        }
    }

    // convert frame ID if necessary
    if (version >= 3) {
        if (Id3v2FrameIds::isShortId(m_frameId)) {
            // try to convert the short frame ID to its long equivalent
            if (!(m_frameId = Id3v2FrameIds::convertToLongId(m_frameId))) {
                diag.emplace_back(DiagLevel::Critical,
                    "The short frame ID can't be converted to its long equivalent which is needed to use the frame in a newer version of ID3v2.",
                    context);
                throw InvalidDataException();
            }
        }
    } else {
        if (Id3v2FrameIds::isLongId(m_frameId)) {
            // try to convert the long frame ID to its short equivalent
            if (!(m_frameId = Id3v2FrameIds::convertToShortId(m_frameId))) {
                diag.emplace_back(DiagLevel::Critical,
                    "The long frame ID can't be converted to its short equivalent which is needed to use the frame in the old version of ID3v2.",
                    context);
                throw InvalidDataException();
            }
        }
    }

    // add a warning if we're writing the frame for an ID3v2 tag known not to support it
    if (version <= 3 && Id3v2FrameIds::isOnlyId3v24Id(version < 3 ? Id3v2FrameIds::convertToLongId(m_frameId) : m_frameId)) {
        diag.emplace_back(DiagLevel::Warning,
            argsToString("The frame is only supported in ID3v2.4 and newer but version of the tag being written is ID3v2.", version,
                ". The frame is written nevertheless but other tools might not be able to deal with it."),
            context);
    } else if (version > 3 && Id3v2FrameIds::isPreId3v24Id(m_frameId)) {
        diag.emplace_back(DiagLevel::Warning,
            argsToString("The frame is only supported in ID3v2.3 and older but version of the tag being written is ID3v2.", version,
                ". The frame is written nevertheless but other tools might not be able to deal with it."),
            context);
    }

    // make actual data depending on the frame ID
    try {
        if (isTextFrame) {
            // make text frame
            vector<string> substrings;
            substrings.reserve(1 + frame.additionalValues().size());
            TagTextEncoding encoding = TagTextEncoding::Unspecified;

            if ((version >= 3 && (m_frameId == Id3v2FrameIds::lTrackPosition || m_frameId == Id3v2FrameIds::lDiskPosition))
                || (version < 3 && (m_frameId == Id3v2FrameIds::sTrackPosition || m_frameId == Id3v2FrameIds::sDiskPosition))) {
                // make track number or disk number frame
                encoding = version >= 4 ? TagTextEncoding::Utf8 : TagTextEncoding::Latin1;
                for (const auto *const value : values) {
                    // convert the position to string
                    substrings.emplace_back(value->toString(encoding));
                    // warn if value is no valid position (although we just store a string after all)
                    if (value->type() == TagDataType::PositionInSet) {
                        continue;
                    }
                    try {
                        value->toPositionInSet();
                    } catch (const ConversionException &) {
                        diag.emplace_back(DiagLevel::Warning,
                            argsToString("The track/disk number \"", substrings.back(), "\" is not of the expected form, eg. \"4/10\"."), context);
                    }
                }

            } else if ((version >= 3 && m_frameId == Id3v2FrameIds::lLength) || (version < 3 && m_frameId == Id3v2FrameIds::sLength)) {
                // make length frame
                encoding = TagTextEncoding::Latin1;
                for (const auto *const value : values) {
                    const auto duration(value->toTimeSpan());
                    if (duration.isNegative()) {
                        diag.emplace_back(DiagLevel::Critical, argsToString("Assigned duration \"", duration.toString(), "\" is negative."), context);
                        throw InvalidDataException();
                    }
                    substrings.emplace_back(numberToString(static_cast<std::uint64_t>(duration.totalMilliseconds())));
                }

            } else {
                // make standard genre index and other text frames
                // -> find text encoding suitable for all assigned values
                for (const auto *const value : values) {
                    switch (encoding) {
                    case TagTextEncoding::Unspecified:
                        switch (value->type()) {
                        case TagDataType::StandardGenreIndex:
                            encoding = TagTextEncoding::Latin1;
                            break;
                        default:
                            encoding = value->dataEncoding();
                        }
                        break;
                    case TagTextEncoding::Latin1:
                        switch (value->dataEncoding()) {
                        case TagTextEncoding::Latin1:
                            break;
                        default:
                            encoding = value->dataEncoding();
                        }
                        break;
                    default:;
                    }
                }
                if (version <= 3 && encoding == TagTextEncoding::Utf8) {
                    encoding = TagTextEncoding::Utf16LittleEndian;
                }
                // -> format values
                for (const auto *const value : values) {
                    if ((value->type() == TagDataType::StandardGenreIndex)
                        && ((version >= 3 && m_frameId == Id3v2FrameIds::lGenre) || (version < 3 && m_frameId == Id3v2FrameIds::sGenre))) {
                        // make standard genere index
                        substrings.emplace_back(numberToString(value->toStandardGenreIndex()));

                    } else {
                        // make other text frame
                        substrings.emplace_back(value->toString(encoding));
                    }
                }
            }

            // concatenate substrings using encoding specific byte order mark and termination
            const auto terminationLength = (encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian) ? 2u : 1u;
            const auto byteOrderMark = [&] {
                switch (encoding) {
                case TagTextEncoding::Utf16LittleEndian:
                    return string({ '\xFF', '\xFE' });
                case TagTextEncoding::Utf16BigEndian:
                    return string({ '\xFE', '\xFF' });
                default:
                    return string();
                }
            }();
            const auto concatenatedSubstrings = joinStrings(substrings, string(), false, byteOrderMark, string(terminationLength, '\0'));

            // write text encoding byte and concatenated strings to data buffer
            m_data = make_unique<char[]>(m_decompressedSize = static_cast<std::uint32_t>(1 + concatenatedSubstrings.size()));
            m_data[0] = static_cast<char>(Id3v2Frame::makeTextEncodingByte(encoding));
            concatenatedSubstrings.copy(&m_data[1], concatenatedSubstrings.size());

        } else if ((version >= 3 && m_frameId == Id3v2FrameIds::lCover) || (version < 3 && m_frameId == Id3v2FrameIds::sCover)) {
            // make picture frame
            m_frame.makePicture(m_data, m_decompressedSize, *values.front(), m_frame.isTypeInfoAssigned() ? m_frame.typeInfo() : 0, version, diag);

        } else if (((version >= 3 && m_frameId == Id3v2FrameIds::lComment) || (version < 3 && m_frameId == Id3v2FrameIds::sComment))
            || ((version >= 3 && m_frameId == Id3v2FrameIds::lUnsynchronizedLyrics)
                || (version < 3 && m_frameId == Id3v2FrameIds::sUnsynchronizedLyrics))) {
            // make comment frame or the unsynchronized lyrics frame
            m_frame.makeComment(m_data, m_decompressedSize, *values.front(), version, diag);

        } else if (((version >= 3 && m_frameId == Id3v2FrameIds::lPlayCounter) || (version < 3 && m_frameId == Id3v2FrameIds::sPlayCounter))) {
            // make play counter frame
            auto playCounter = std::uint64_t();
            try {
                playCounter = values.front()->toUnsignedInteger();
            } catch (const ConversionException &) {
                diag.emplace_back(DiagLevel::Warning,
                    argsToString("The play counter \"", values.front()->toDisplayString(), "\" is not an unsigned integer."), context);
            }
            m_decompressedSize = computePlayCounterSize(playCounter);
            m_data = make_unique<char[]>(m_decompressedSize);
            writePlayCounter(m_data.get() + m_decompressedSize - 1, m_decompressedSize, playCounter);

        } else if (((version >= 3 && m_frameId == Id3v2FrameIds::lRating) || (version < 3 && m_frameId == Id3v2FrameIds::sRating))) {
            // make popularimeter frame
            auto popularity = Popularity();
            try {
                popularity = values.front()->toScaledPopularity(TagType::Id3v2Tag);
            } catch (const ConversionException &) {
                diag.emplace_back(DiagLevel::Warning,
                    argsToString(
                        "The popularity \"", values.front()->toDisplayString(), "\" is not of the expected form, eg. \"user|rating|counter\"."),
                    context);
            }
            // -> clamp rating
            if (popularity.rating > 0xFF) {
                popularity.rating = 0xFF;
                diag.emplace_back(DiagLevel::Warning, argsToString("The rating has been clamped to 255."), context);
            } else if (popularity.rating < 0x00) {
                popularity.rating = 0x00;
                diag.emplace_back(DiagLevel::Warning, argsToString("The rating has been clamped to 0."), context);
            }
            // -> compute size: user name length + termination + rating byte
            m_decompressedSize = static_cast<std::uint32_t>(popularity.user.size() + 2);
            const auto playCounterSize = computePlayCounterSize(popularity.playCounter);
            m_decompressedSize += playCounterSize;
            // -> copy data into buffer
            m_data = make_unique<char[]>(m_decompressedSize);
            auto pos = popularity.user.size() + 1;
            std::memcpy(m_data.get(), popularity.user.data(), pos);
            m_data[pos] = static_cast<char>(popularity.rating);
            writePlayCounter(m_data.get() + pos + playCounterSize, playCounterSize, popularity.playCounter);

        } else {
            // make unknown frame
            const auto &value(*values.front());
            if (value.dataSize() > maxId3v2FrameDataSize) {
                diag.emplace_back(DiagLevel::Critical, "Assigned value exceeds maximum size.", context);
                throw InvalidDataException();
            }
            m_data = make_unique<char[]>(m_decompressedSize = static_cast<std::uint32_t>(value.dataSize()));
            std::memcpy(m_data.get(), value.dataPointer(), m_decompressedSize);
        }
    } catch (const ConversionException &) {
        try {
            const auto valuesAsString = TagValue::toStrings(values);
            diag.emplace_back(DiagLevel::Critical,
                argsToString("Assigned value(s) \"", DiagMessage::formatList(valuesAsString), "\" can not be converted appropriately."), context);
        } catch (const ConversionException &) {
            diag.emplace_back(DiagLevel::Critical, "Assigned value(s) can not be converted appropriately.", context);
        }
        throw InvalidDataException();
    }

    // apply compression if frame should be compressed
    if (version >= 3 && m_frame.isCompressed()) {
        auto compressedSize = compressBound(m_decompressedSize);
        auto compressedData = make_unique<char[]>(compressedSize);
        switch (compress(reinterpret_cast<Bytef *>(compressedData.get()), reinterpret_cast<uLongf *>(&compressedSize),
            reinterpret_cast<Bytef *>(m_data.get()), m_decompressedSize)) {
        case Z_MEM_ERROR:
            diag.emplace_back(DiagLevel::Critical, "Decompressing failed. The source buffer was too small.", context);
            throw InvalidDataException();
        case Z_BUF_ERROR:
            diag.emplace_back(DiagLevel::Critical, "Decompressing failed. The destination buffer was too small.", context);
            throw InvalidDataException();
        case Z_OK:;
        }
        if (compressedSize > maxId3v2FrameDataSize) {
            diag.emplace_back(DiagLevel::Critical, "Compressed size exceeds maximum data size.", context);
            throw InvalidDataException();
        }
        m_data.swap(compressedData);
        m_dataSize = static_cast<std::uint32_t>(compressedSize);
    } else {
        m_dataSize = m_decompressedSize;
    }

    // calculate required size
    // -> data size
    m_requiredSize = m_dataSize;
    if (version < 3) {
        // -> header size
        m_requiredSize += 6;
    } else {
        // -> header size
        m_requiredSize += 10;
        // -> group byte
        if (m_frame.hasGroupInformation()) {
            m_requiredSize += 1;
        }
        // -> decompressed size
        if (version >= 3 && m_frame.isCompressed()) {
            m_requiredSize += 4;
        }
    }
}

/*!
 * \brief Saves the frame (specified when constructing the object) using
 *        the specified \a writer.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw TagParser::Failure or a derived exception.
 */
void Id3v2FrameMaker::make(BinaryWriter &writer)
{
    if (m_version < 3) {
        writer.writeUInt24BE(m_frameId);
        writer.writeUInt24BE(m_dataSize);
    } else {
        writer.writeUInt32BE(m_frameId);
        if (m_version >= 4) {
            writer.writeSynchsafeUInt32BE(m_dataSize);
        } else {
            writer.writeUInt32BE(m_dataSize);
        }
        writer.writeUInt16BE(m_frame.flag());
        if (m_frame.hasGroupInformation()) {
            writer.writeByte(m_frame.group());
        }
        if (m_version >= 3 && m_frame.isCompressed()) {
            if (m_version >= 4) {
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
TagTextEncoding Id3v2Frame::parseTextEncodingByte(std::uint8_t textEncodingByte, Diagnostics &diag)
{
    switch (textEncodingByte) {
    case Id3v2TextEncodingBytes::Ascii:
        return TagTextEncoding::Latin1;
    case Id3v2TextEncodingBytes::Utf16WithBom:
        return TagTextEncoding::Utf16LittleEndian;
    case Id3v2TextEncodingBytes::Utf16BigEndianWithoutBom:
        return TagTextEncoding::Utf16BigEndian;
    case Id3v2TextEncodingBytes::Utf8:
        return TagTextEncoding::Utf8;
    default:
        diag.emplace_back(
            DiagLevel::Warning, "The charset of the frame is invalid. Latin-1 will be used.", "parsing encoding of frame " + idToString());
        return TagTextEncoding::Latin1;
    }
}

/*!
 * \brief Returns a text encoding byte for the specified \a textEncoding.
 */
std::uint8_t Id3v2Frame::makeTextEncodingByte(TagTextEncoding textEncoding)
{
    switch (textEncoding) {
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
 * \brief Parses a substring from the specified \a buffer.
 *
 * This method ensures that byte order marks and termination characters for the specified \a encoding are omitted.
 * It might add a warning if the substring is not terminated.
 *
 * \param buffer Specifies a pointer to the possibly terminated string.
 * \param bufferSize Specifies the size of the string in byte.
 * \param encoding Specifies the encoding of the string. Might be adjusted if a byte order marks is found.
 * \param addWarnings Specifies whether warnings should be added if the string is not terminated.
 * \returns Returns the start offset, the length of the string (without termination) and the end offset (after termination).
 * \remarks The length is always returned as the number of bytes, not as the number of characters (makes a difference for
 *          Unicode encodings).
 */
tuple<const char *, size_t, const char *> Id3v2Frame::parseSubstring(
    const char *buffer, std::size_t bufferSize, TagTextEncoding &encoding, bool addWarnings, Diagnostics &diag)
{
    tuple<const char *, size_t, const char *> res(buffer, 0, buffer + bufferSize);
    switch (encoding) {
    case TagTextEncoding::Unspecified:
    case TagTextEncoding::Latin1:
    case TagTextEncoding::Utf8: {
        if ((bufferSize >= 3) && (BE::toUInt24(buffer) == 0x00EFBBBF)) {
            if (encoding == TagTextEncoding::Latin1) {
                diag.emplace_back(DiagLevel::Critical, "Denoted character set is Latin-1 but an UTF-8 BOM is present - assuming UTF-8.",
                    "parsing frame " + idToString());
                encoding = TagTextEncoding::Utf8;
            }
            get<0>(res) += 3;
        }
        const char *pos = get<0>(res);
        for (; *pos != 0x00; ++pos) {
            if (pos < get<2>(res)) {
                ++get<1>(res);
            } else {
                if (addWarnings) {
                    diag.emplace_back(
                        DiagLevel::Warning, "String in frame is not terminated properly.", "parsing termination of frame " + idToString());
                }
                break;
            }
        }
        get<2>(res) = pos + 1;
        break;
    }
    case TagTextEncoding::Utf16BigEndian:
    case TagTextEncoding::Utf16LittleEndian: {
        if (bufferSize >= 2) {
            switch (LE::toInt<std::uint16_t>(buffer)) {
            case 0xFEFF:
                if (encoding == TagTextEncoding::Utf16BigEndian) {
                    diag.emplace_back(DiagLevel::Critical,
                        "Denoted character set is UTF-16 Big Endian but UTF-16 Little Endian BOM is present - assuming UTF-16 LE.",
                        "parsing frame " + idToString());
                    encoding = TagTextEncoding::Utf16LittleEndian;
                }
                get<0>(res) += 2;
                break;
            case 0xFFFE:
                encoding = TagTextEncoding::Utf16BigEndian;
                get<0>(res) += 2;
            }
        }
        const std::uint16_t *pos = reinterpret_cast<const std::uint16_t *>(get<0>(res));
        for (; *pos != 0x0000; ++pos) {
            if (pos < reinterpret_cast<const std::uint16_t *>(get<2>(res))) {
                get<1>(res) += 2;
            } else {
                if (addWarnings) {
                    diag.emplace_back(
                        DiagLevel::Warning, "Wide string in frame is not terminated properly.", "parsing termination of frame " + idToString());
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
 * \brief Parses a substring from the specified \a buffer.
 *
 * Same as Id3v2Frame::parseSubstring() but returns the substring as string object.
 */
string Id3v2Frame::parseString(const char *buffer, size_t dataSize, TagTextEncoding &encoding, bool addWarnings, Diagnostics &diag)
{
    return stringFromSubstring(parseSubstring(buffer, dataSize, encoding, addWarnings, diag));
}

/*!
 * \brief Parses a substring from the specified \a buffer.
 *
 * Same as Id3v2Frame::parseSubstring() but returns the substring as u16string object
 *
 * \remarks Converts byte order to match host byte order (otherwise it wouldn't make much sense to use the resulting u16string).
 */
u16string Id3v2Frame::parseWideString(const char *buffer, size_t dataSize, TagTextEncoding &encoding, bool addWarnings, Diagnostics &diag)
{
    return wideStringFromSubstring(parseSubstring(buffer, dataSize, encoding, addWarnings, diag), encoding);
}

/*!
 * \brief Parses a byte order mark from the specified \a buffer.
 * \param buffer Specifies the buffer holding the byte order mark.
 * \param maxSize Specifies the maximal number of bytes to read from the buffer.
 * \param encoding Specifies the encoding of the string. Might be reset if a byte order mark is found.
 * \remarks This method is not used anymore and might be deleted.
 */
void Id3v2Frame::parseBom(const char *buffer, std::size_t maxSize, TagTextEncoding &encoding, Diagnostics &diag)
{
    switch (encoding) {
    case TagTextEncoding::Utf16BigEndian:
    case TagTextEncoding::Utf16LittleEndian:
        if ((maxSize >= 2) && (BE::toInt<std::uint16_t>(buffer) == 0xFFFE)) {
            encoding = TagTextEncoding::Utf16LittleEndian;
        } else if ((maxSize >= 2) && (BE::toInt<std::uint16_t>(buffer) == 0xFEFF)) {
            encoding = TagTextEncoding::Utf16BigEndian;
        }
        break;
    default:
        if ((maxSize >= 3) && (BE::toUInt24(buffer) == 0x00EFBBBF)) {
            encoding = TagTextEncoding::Utf8;
            diag.emplace_back(DiagLevel::Warning, "UTF-8 byte order mark found in text frame.", "parsing byte order mark of frame " + idToString());
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
void Id3v2Frame::parseLegacyPicture(const char *buffer, std::size_t maxSize, TagValue &tagValue, std::uint8_t &typeInfo, Diagnostics &diag)
{
    static const string context("parsing ID3v2.2 picture frame");
    if (maxSize < 6) {
        diag.emplace_back(DiagLevel::Critical, "Picture frame is incomplete.", context);
        throw TruncatedDataException();
    }
    const char *end = buffer + maxSize;
    auto dataEncoding = parseTextEncodingByte(static_cast<std::uint8_t>(*buffer), diag); // the first byte stores the encoding
    typeInfo = static_cast<unsigned char>(*(buffer + 4));
    auto substr = parseSubstring(buffer + 5, static_cast<size_t>(end - 5 - buffer), dataEncoding, true, diag);
    tagValue.setDescription(string(get<0>(substr), get<1>(substr)), dataEncoding);
    if (get<2>(substr) >= end) {
        diag.emplace_back(DiagLevel::Critical, "Picture frame is incomplete (actual data is missing).", context);
        throw TruncatedDataException();
    }
    tagValue.assignData(get<2>(substr), static_cast<size_t>(end - get<2>(substr)), TagDataType::Picture, dataEncoding);
}

/*!
 * \brief Parses the ID3v2.3 picture from the specified \a buffer.
 * \param buffer Specifies the buffer holding the picture.
 * \param maxSize Specifies the maximal number of bytes to read from the buffer.
 * \param tagValue Specifies the tag value used to store the results.
 * \param typeInfo Specifies a byte used to store the type info.
 */
void Id3v2Frame::parsePicture(const char *buffer, std::size_t maxSize, TagValue &tagValue, std::uint8_t &typeInfo, Diagnostics &diag)
{
    static const string context("parsing ID3v2.3 picture frame");
    const char *end = buffer + maxSize;
    auto dataEncoding = parseTextEncodingByte(static_cast<std::uint8_t>(*buffer), diag); // the first byte stores the encoding
    auto mimeTypeEncoding = TagTextEncoding::Latin1;
    auto substr = parseSubstring(buffer + 1, maxSize - 1, mimeTypeEncoding, true, diag);
    if (get<1>(substr)) {
        tagValue.setMimeType(string(get<0>(substr), get<1>(substr)));
    }
    if (get<2>(substr) >= end) {
        diag.emplace_back(DiagLevel::Critical, "Picture frame is incomplete (type info, description and actual data are missing).", context);
        throw TruncatedDataException();
    }
    typeInfo = static_cast<unsigned char>(*get<2>(substr));
    if (++get<2>(substr) >= end) {
        diag.emplace_back(DiagLevel::Critical, "Picture frame is incomplete (description and actual data are missing).", context);
        throw TruncatedDataException();
    }
    substr = parseSubstring(get<2>(substr), static_cast<size_t>(end - get<2>(substr)), dataEncoding, true, diag);
    tagValue.setDescription(string(get<0>(substr), get<1>(substr)), dataEncoding);
    if (get<2>(substr) >= end) {
        diag.emplace_back(DiagLevel::Critical, "Picture frame is incomplete (actual data is missing).", context);
        throw TruncatedDataException();
    }
    tagValue.assignData(get<2>(substr), static_cast<size_t>(end - get<2>(substr)), TagDataType::Picture, dataEncoding);
}

/*!
 * \brief Parses the comment/unsynchronized lyrics from the specified \a buffer.
 * \param buffer Specifies the buffer holding the picture.
 * \param dataSize Specifies the maximal number of bytes to read from the buffer.
 * \param tagValue Specifies the tag value used to store the results.
 */
void Id3v2Frame::parseComment(const char *buffer, std::size_t dataSize, TagValue &tagValue, Diagnostics &diag)
{
    static const string context("parsing comment/unsynchronized lyrics frame");
    const char *end = buffer + dataSize;
    if (dataSize < 5) {
        diag.emplace_back(DiagLevel::Critical, "Comment frame is incomplete.", context);
        throw TruncatedDataException();
    }
    TagTextEncoding dataEncoding = parseTextEncodingByte(static_cast<std::uint8_t>(*buffer), diag);
    if (*(++buffer)) {
        tagValue.setLocale(Locale(std::string(buffer, 3), LocaleFormat::ISO_639_2_B)); // does standard say whether T or B?
    }
    auto substr = parseSubstring(buffer += 3, dataSize -= 4, dataEncoding, true, diag);
    tagValue.setDescription(string(get<0>(substr), get<1>(substr)), dataEncoding);
    if (get<2>(substr) > end) {
        diag.emplace_back(DiagLevel::Critical, "Comment frame is incomplete (description not terminated?).", context);
        throw TruncatedDataException();
    }
    substr = parseSubstring(get<2>(substr), static_cast<size_t>(end - get<2>(substr)), dataEncoding, false, diag);
    tagValue.assignData(get<0>(substr), get<1>(substr), TagDataType::Text, dataEncoding);
}

/*!
 * \brief Writes the BOM for the specified \a encoding to the specified \a buffer.
 * \returns Returns the number of bytes written to the buffer.
 */
size_t Id3v2Frame::makeBom(char *buffer, TagTextEncoding encoding)
{
    switch (encoding) {
    case TagTextEncoding::Utf16LittleEndian:
        LE::getBytes(static_cast<std::uint16_t>(0xFEFF), buffer);
        return 2;
    case TagTextEncoding::Utf16BigEndian:
        BE::getBytes(static_cast<std::uint16_t>(0xFEFF), buffer);
        return 2;
    default:
        return 0;
    }
}

/*!
 * \brief Writes the specified picture to the specified buffer (ID3v2.2 compatible).
 */
void Id3v2Frame::makeLegacyPicture(
    unique_ptr<char[]> &buffer, std::uint32_t &bufferSize, const TagValue &picture, std::uint8_t typeInfo, Diagnostics &diag)
{
    // determine description
    TagTextEncoding descriptionEncoding = picture.descriptionEncoding();
    StringData convertedDescription;
    string::size_type descriptionSize = picture.description().find(
        "\0\0", 0, descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 2 : 1);
    if (descriptionSize == string::npos) {
        descriptionSize = picture.description().size();
    }
    if (descriptionEncoding == TagTextEncoding::Utf8) {
        // UTF-8 is only supported by ID3v2.4, so convert back to UTF-16
        descriptionEncoding = TagTextEncoding::Utf16LittleEndian;
        convertedDescription = convertUtf8ToUtf16LE(picture.description().data(), descriptionSize);
        descriptionSize = convertedDescription.second;
    }

    // calculate needed buffer size and create buffer
    // note: encoding byte + image format + picture type byte + description size + 1 or 2 null bytes (depends on encoding)                                                                                       + data size
    const auto requiredBufferSize = 1 + 3 + 1 + descriptionSize
        + (descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 4 : 1)
        + picture.dataSize();
    if (requiredBufferSize > numeric_limits<std::uint32_t>::max()) {
        diag.emplace_back(DiagLevel::Critical, "Required size exceeds maximum.", "making legacy picture frame");
        throw InvalidDataException();
    }
    buffer = make_unique<char[]>(bufferSize = static_cast<std::uint32_t>(requiredBufferSize));
    char *offset = buffer.get();

    // write encoding byte
    *offset = static_cast<char>(makeTextEncodingByte(descriptionEncoding));

    // write mime type
    const char *imageFormat;
    if (picture.mimeType() == "image/jpeg") {
        imageFormat = "JPG";
    } else if (picture.mimeType() == "image/png") {
        imageFormat = "PNG";
    } else if (picture.mimeType() == "image/gif") {
        imageFormat = "GIF";
    } else if (picture.mimeType() == "-->") {
        imageFormat = picture.mimeType().data();
    } else {
        imageFormat = "UND";
    }
    std::strncpy(++offset, imageFormat, 3);

    // write picture type
    *(offset += 3) = static_cast<char>(typeInfo);

    // write description
    offset += makeBom(offset + 1, descriptionEncoding);
    if (convertedDescription.first) {
        copy(convertedDescription.first.get(), convertedDescription.first.get() + descriptionSize, ++offset);
    } else {
        picture.description().copy(++offset, descriptionSize);
    }
    *(offset += descriptionSize) = 0x00; // terminate description and increase data offset
    if (descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian) {
        *(++offset) = 0x00;
    }

    // write actual data
    copy(picture.dataPointer(), picture.dataPointer() + picture.dataSize(), ++offset);
}

/*!
 * \brief Writes the specified picture to the specified buffer.
 */
void Id3v2Frame::makePicture(std::unique_ptr<char[]> &buffer, std::uint32_t &bufferSize, const TagValue &picture, std::uint8_t typeInfo,
    std::uint8_t version, Diagnostics &diag)
{
    if (version < 3) {
        makeLegacyPicture(buffer, bufferSize, picture, typeInfo, diag);
        return;
    }

    // determine description
    TagTextEncoding descriptionEncoding = picture.descriptionEncoding();
    StringData convertedDescription;
    string::size_type descriptionSize = picture.description().find(
        "\0\0", 0, descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 2 : 1);
    if (descriptionSize == string::npos) {
        descriptionSize = picture.description().size();
    }
    if (version < 4 && descriptionEncoding == TagTextEncoding::Utf8) {
        // UTF-8 is only supported by ID3v2.4, so convert back to UTF-16
        descriptionEncoding = TagTextEncoding::Utf16LittleEndian;
        convertedDescription = convertUtf8ToUtf16LE(picture.description().data(), descriptionSize);
        descriptionSize = convertedDescription.second;
    }
    // determine mime-type
    string::size_type mimeTypeSize = picture.mimeType().find('\0');
    if (mimeTypeSize == string::npos) {
        mimeTypeSize = picture.mimeType().length();
    }

    // calculate needed buffer size and create buffer
    // note: encoding byte + mime type size + 0 byte + picture type byte + description size + 1 or 4 null bytes (depends on encoding)                                                                                       + data size
    const auto requiredBufferSize = 1 + mimeTypeSize + 1 + 1 + descriptionSize
        + (descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian ? 4 : 1)
        + picture.dataSize();
    if (requiredBufferSize > numeric_limits<uint32_t>::max()) {
        diag.emplace_back(DiagLevel::Critical, "Required size exceeds maximum.", "making picture frame");
        throw InvalidDataException();
    }
    buffer = make_unique<char[]>(bufferSize = static_cast<uint32_t>(requiredBufferSize));
    char *offset = buffer.get();

    // write encoding byte
    *offset = static_cast<char>(makeTextEncodingByte(descriptionEncoding));

    // write mime type
    picture.mimeType().copy(++offset, mimeTypeSize);

    *(offset += mimeTypeSize) = 0x00; // terminate mime type
    // write picture type
    *(++offset) = static_cast<char>(typeInfo);

    // write description
    offset += makeBom(offset + 1, descriptionEncoding);
    if (convertedDescription.first) {
        copy(convertedDescription.first.get(), convertedDescription.first.get() + descriptionSize, ++offset);
    } else {
        picture.description().copy(++offset, descriptionSize);
    }
    *(offset += descriptionSize) = 0x00; // terminate description and increase data offset
    if (descriptionEncoding == TagTextEncoding::Utf16BigEndian || descriptionEncoding == TagTextEncoding::Utf16LittleEndian) {
        *(++offset) = 0x00;
    }

    // write actual data
    copy(picture.dataPointer(), picture.dataPointer() + picture.dataSize(), ++offset);
}

/*!
 * \brief Writes the specified comment to the specified buffer.
 */
void Id3v2Frame::makeComment(unique_ptr<char[]> &buffer, std::uint32_t &bufferSize, const TagValue &comment, std::uint8_t version, Diagnostics &diag)
{
    static const string context("making comment frame");

    // check whether type and other values are valid
    TagTextEncoding encoding = comment.dataEncoding();
    if (!comment.description().empty() && encoding != comment.descriptionEncoding()) {
        diag.emplace_back(DiagLevel::Critical, "Data encoding and description encoding aren't equal.", context);
        throw InvalidDataException();
    }
    const string &language = comment.locale().abbreviatedName(LocaleFormat::ISO_639_2_B, LocaleFormat::ISO_639_2_T, LocaleFormat::Unknown);
    if (language.length() > 3) {
        diag.emplace_back(DiagLevel::Critical, "The language must be 3 bytes long (ISO-639-2).", context);
        throw InvalidDataException();
    }
    StringData convertedDescription;
    string::size_type descriptionSize = comment.description().find(
        "\0\0", 0, encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian ? 2 : 1);
    if (descriptionSize == string::npos) {
        descriptionSize = comment.description().size();
    }
    if (version < 4 && encoding == TagTextEncoding::Utf8) {
        // UTF-8 is only supported by ID3v2.4, so convert back to UTF-16
        encoding = TagTextEncoding::Utf16LittleEndian;
        convertedDescription = convertUtf8ToUtf16LE(comment.description().data(), descriptionSize);
        descriptionSize = convertedDescription.second;
    }

    // calculate needed buffer size and create buffer
    // note: encoding byte + language + description size + actual data size + BOMs and termination
    const auto data = comment.toString(encoding);
    const auto requiredBufferSize = 1 + 3 + descriptionSize + data.size()
        + (encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian ? 6 : 1) + data.size();
    if (requiredBufferSize > numeric_limits<uint32_t>::max()) {
        diag.emplace_back(DiagLevel::Critical, "Required size exceeds maximum.", context);
        throw InvalidDataException();
    }
    buffer = make_unique<char[]>(bufferSize = static_cast<uint32_t>(requiredBufferSize));
    char *offset = buffer.get();

    // write encoding
    *offset = static_cast<char>(makeTextEncodingByte(encoding));

    // write language
    for (unsigned int i = 0; i < 3; ++i) {
        *(++offset) = (language.length() > i) ? language[i] : 0x00;
    }

    // write description
    offset += makeBom(offset + 1, encoding);
    if (convertedDescription.first) {
        copy(convertedDescription.first.get(), convertedDescription.first.get() + descriptionSize, ++offset);
    } else {
        comment.description().copy(++offset, descriptionSize);
    }
    offset += descriptionSize;
    *offset = 0x00; // terminate description and increase data offset
    if (encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian) {
        *(++offset) = 0x00;
    }

    // write actual data
    offset += makeBom(offset + 1, encoding);
    data.copy(++offset, data.size());
}

} // namespace TagParser
