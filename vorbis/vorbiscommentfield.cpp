#include "./vorbiscommentfield.h"
#include "./vorbiscommentids.h"

#include "../flac/flacmetadata.h"

#include "../ogg/oggiterator.h"

#include "../diagnostics.h"
#include "../exceptions.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <iostream>
#include <memory>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::VorbisCommentField
 * \brief The VorbisCommentField class is used by VorbisComment to store the fields.
 */

/*!
 * \brief Constructs a new Vorbis comment field.
 */
VorbisCommentField::VorbisCommentField()
{
}

/*!
 * \brief Constructs a new Vorbis comment with the specified \a id and \a value.
 */
VorbisCommentField::VorbisCommentField(const IdentifierType &id, const TagValue &value)
    : TagField<VorbisCommentField>(id, value)
{
}

/*!
 * \brief Internal implementation for parsing.
 */
template <class StreamType> void VorbisCommentField::internalParse(StreamType &stream, std::uint64_t &maxSize, Diagnostics &diag)
{
    static const string context("parsing Vorbis comment  field");
    char buff[4];
    if (maxSize < 4) {
        diag.emplace_back(DiagLevel::Critical, argsToString("Field expected at ", static_cast<std::streamoff>(stream.tellg()), '.'), context);
        throw TruncatedDataException();
    } else {
        maxSize -= 4;
    }
    stream.read(buff, 4);
    if (const auto size = LE::toUInt32(buff)) { // read size
        if (size <= maxSize) {
            maxSize -= size;
            // read data
            auto data = make_unique<char[]>(size);
            stream.read(data.get(), size);
            std::uint32_t idSize = 0;
            for (const char *i = data.get(), *end = data.get() + size; i != end && *i != '='; ++i, ++idSize)
                ;
            // extract id
            setId(string(data.get(), idSize));
            if (!idSize) {
                // empty field ID
                diag.emplace_back(
                    DiagLevel::Critical, argsToString("The field ID at ", static_cast<std::streamoff>(stream.tellg()), " is empty."), context);
                throw InvalidDataException();
            } else if (id() == VorbisCommentIds::cover()) {
                // extract cover value
                try {
                    auto decoded = decodeBase64(data.get() + idSize + 1, size - idSize - 1);
                    stringstream bufferStream(ios_base::in | ios_base::out | ios_base::binary);
                    bufferStream.exceptions(ios_base::failbit | ios_base::badbit);
#if defined(__GLIBCXX__) && !defined(_LIBCPP_VERSION)
                    bufferStream.rdbuf()->pubsetbuf(reinterpret_cast<char *>(decoded.first.get()), decoded.second);
#else
                    bufferStream.write(reinterpret_cast<const char *>(decoded.first.get()), decoded.second);
#endif
                    FlacMetaDataBlockPicture pictureBlock(value());
                    pictureBlock.parse(bufferStream, decoded.second);
                    setTypeInfo(pictureBlock.pictureType());
                } catch (const TruncatedDataException &) {
                    diag.emplace_back(DiagLevel::Critical, "METADATA_BLOCK_PICTURE is truncated.", context);
                    throw;
                } catch (const ConversionException &) {
                    diag.emplace_back(DiagLevel::Critical, "Base64 coding of METADATA_BLOCK_PICTURE is invalid.", context);
                    throw InvalidDataException();
                } catch (const std::ios_base::failure &failure) {
                    diag.emplace_back(DiagLevel::Critical,
                        argsToString("An IO error occurred when reading the METADATA_BLOCK_PICTURE struct: ", failure.what()), context);
                    throw Failure();
                }
            } else if (id().size() + 1 < size) {
                const auto str = std::string_view(data.get() + idSize + 1, size - idSize - 1);
                if (id() == VorbisCommentIds::rating()) {
                    try {
                        // set rating as Popularity to preserve the scale information
                        value().assignPopularity(Popularity{ .rating = stringToNumber<double>(str), .scale = TagType::VorbisComment });
                    } catch (const ConversionException &) {
                        // fallback to text
                        value().assignText(str, TagTextEncoding::Utf8);
                        diag.emplace_back(DiagLevel::Warning, argsToString("The rating is not a number."), context);
                    }
                } else {
                    // extract other values (as string)
                    value().assignText(str, TagTextEncoding::Utf8);
                }
            }
        } else {
            diag.emplace_back(DiagLevel::Critical, argsToString("Field at ", static_cast<std::streamoff>(stream.tellg()), " is truncated."), context);
            throw TruncatedDataException();
        }
    }
}

/*!
 * \brief Parses a field using the specified \a iterator.
 *
 * The currentCharacterOffset() of the iterator is expected to be
 * at the beginning of the field to be parsed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisCommentField::parse(OggIterator &iterator, Diagnostics &diag)
{
    std::uint64_t maxSize = iterator.streamSize() - iterator.currentCharacterOffset();
    internalParse(iterator, maxSize, diag);
}

/*!
 * \brief Parses a field using the specified \a iterator.
 *
 * The currentCharacterOffset() of the iterator is expected to be
 * at the beginning of the field to be parsed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisCommentField::parse(OggIterator &iterator, std::uint64_t &maxSize, Diagnostics &diag)
{
    internalParse(iterator, maxSize, diag);
}

/*!
 * \brief Parses a field from the specified \a stream.
 *
 * The position of the current character in the input stream is expected to be
 * at the beginning of the field to be parsed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisCommentField::parse(istream &stream, std::uint64_t &maxSize, Diagnostics &diag)
{
    internalParse(stream, maxSize, diag);
}

/*!
 * \brief Writes the field to a stream using the specified \a writer.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 * \returns Returns whether the field has been written. (Some fields might be skipped
 *          when specific \a flags are set.)
 */
bool VorbisCommentField::make(BinaryWriter &writer, VorbisCommentFlags flags, Diagnostics &diag)
{
    static const string context("making Vorbis comment  field");
    if (id().empty()) {
        diag.emplace_back(DiagLevel::Critical, "The field ID is empty.", context);
    }
    try {
        // try to convert value to string
        string valueString;
        if (id() == VorbisCommentIds::cover()) {
            if (flags & VorbisCommentFlags::NoCovers) {
                return false;
            }
            // make cover
            if (value().type() != TagDataType::Picture) {
                diag.emplace_back(DiagLevel::Critical, "Assigned value of cover field is not picture data.", context);
                throw InvalidDataException();
            }
            try {
                FlacMetaDataBlockPicture pictureBlock(value());
                pictureBlock.setPictureType(typeInfo());

                const auto requiredSize = pictureBlock.requiredSize();
                auto buffer = make_unique<char[]>(requiredSize);
                stringstream bufferStream(ios_base::in | ios_base::out | ios_base::binary);
                bufferStream.exceptions(ios_base::failbit | ios_base::badbit);
#if defined(__GLIBCXX__) && !defined(_LIBCPP_VERSION)
                bufferStream.rdbuf()->pubsetbuf(buffer.get(), requiredSize);
#endif
                pictureBlock.make(bufferStream);
#if defined(__GLIBCXX__) && !defined(_LIBCPP_VERSION)
                bufferStream.read(buffer.get(), static_cast<std::streamsize>(requiredSize));
#endif
                valueString = encodeBase64(reinterpret_cast<std::uint8_t *>(buffer.get()), requiredSize);

            } catch (const Failure &) {
                diag.emplace_back(DiagLevel::Critical, "Unable to make METADATA_BLOCK_PICTURE struct from the assigned value.", context);
                throw;
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical,
                    argsToString("An IO error occurred when writing the METADATA_BLOCK_PICTURE struct: ", failure.what()), context);
                throw Failure();
            }
        } else if (value().type() == TagDataType::Popularity) {
            valueString = value().toScaledPopularity(TagType::VorbisComment).toString();
        } else {
            // make normal string value
            valueString = value().toString(TagTextEncoding::Utf8);
        }
        const auto size(valueString.size() + id().size() + 1);
        if (size > numeric_limits<std::uint32_t>::max()) {
            diag.emplace_back(DiagLevel::Critical, "Assigned value exceeds the maximum size.", context);
            throw InvalidDataException();
        }
        writer.writeUInt32LE(static_cast<std::uint32_t>(size));
        writer.writeString(id());
        writer.writeChar('=');
        writer.writeString(valueString);
    } catch (const ConversionException &) {
        diag.emplace_back(DiagLevel::Critical, "Assigned value can not be converted appropriately.", context);
        throw InvalidDataException();
    }
    return true;
}

} // namespace TagParser
