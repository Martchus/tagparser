#include "./vorbiscommentfield.h"
#include "./vorbiscommentids.h"

#include "../flac/flacmetadata.h"

#include "../ogg/oggiterator.h"

#include "../id3/id3v2frame.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/io/catchiofailure.h>
#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringconversion.h>

#include <iostream>
#include <memory>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::VorbisCommentField
 * \brief The VorbisCommentField class is used by VorbisComment to store the fields.
 */

/*!
 * \brief Constructs a new Vorbis comment field.
 */
VorbisCommentField::VorbisCommentField()
{}

/*!
 * \brief Constructs a new Vorbis comment with the specified \a id and \a value.
 */
VorbisCommentField::VorbisCommentField(const IdentifierType &id, const TagValue &value) :
    TagField<VorbisCommentField>(id, value)
{}

/*!
 * \brief Internal implementation for parsing.
 */
template<class StreamType>
void VorbisCommentField::internalParse(StreamType &stream, uint64 &maxSize)
{
    static const string context("parsing Vorbis comment  field");
    char buff[4];
    if(maxSize < 4) {
        addNotification(NotificationType::Critical, "Field expected.", context);
        throw TruncatedDataException();
    } else {
        maxSize -= 4;
    }
    stream.read(buff, 4);
    if(const auto size = LE::toUInt32(buff)) { // read size
        if(size <= maxSize) {
            maxSize -= size;
            // read data
            auto data = make_unique<char []>(size);
            stream.read(data.get(), size);
            uint32 idSize = 0;
            for(const char *i = data.get(), *end = data.get() + size; i != end && *i != '='; ++i, ++idSize);
            // extract id
            setId(string(data.get(), idSize));
            if(!idSize) {
                // empty field ID
                addNotification(NotificationType::Critical, "The field ID is empty.", context);
                throw InvalidDataException();
            } else if(id() == VorbisCommentIds::cover()) {
                // extract cover value
                try {
                    auto decoded = decodeBase64(data.get() + idSize + 1, size - idSize - 1);
                    stringstream bufferStream(ios_base::in | ios_base::out | ios_base::binary);
                    bufferStream.exceptions(ios_base::failbit | ios_base::badbit);
                    bufferStream.rdbuf()->pubsetbuf(reinterpret_cast<char *>(decoded.first.get()), decoded.second);
                    FlacMetaDataBlockPicture pictureBlock(value());
                    pictureBlock.parse(bufferStream, decoded.second);
                    setTypeInfo(pictureBlock.pictureType());
                } catch(const TruncatedDataException &) {
                    addNotification(NotificationType::Critical, "METADATA_BLOCK_PICTURE is truncated.", context);
                    throw;
                } catch(const ConversionException &) {
                    addNotification(NotificationType::Critical, "Base64 coding of METADATA_BLOCK_PICTURE is invalid.", context);
                    throw InvalidDataException();
                } catch(...) {
                    catchIoFailure();
                    addNotification(NotificationType::Critical, "An IO error occured when reading the METADATA_BLOCK_PICTURE struct.", context);
                    throw Failure();
                }
            } else if(id().size() + 1 < size) {
                // extract other values (as string)
                setValue(TagValue(string(data.get() + idSize + 1, size - idSize - 1), TagTextEncoding::Utf8));
            }
        } else {
            addNotification(NotificationType::Critical, "Field is truncated.", context);
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
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisCommentField::parse(OggIterator &iterator)
{
    uint64 maxSize = iterator.streamSize() - iterator.currentCharacterOffset();
    internalParse(iterator, maxSize);
}

/*!
 * \brief Parses a field using the specified \a iterator.
 *
 * The currentCharacterOffset() of the iterator is expected to be
 * at the beginning of the field to be parsed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisCommentField::parse(OggIterator &iterator, uint64 &maxSize)
{
    internalParse(iterator, maxSize);
}

/*!
 * \brief Parses a field from the specified \a stream.
 *
 * The position of the current character in the input stream is expected to be
 * at the beginning of the field to be parsed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisCommentField::parse(istream &stream, uint64 &maxSize)
{
    internalParse(stream, maxSize);
}

/*!
 * \brief Writes the field to a stream using the specified \a writer.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 * \returns Returns whether the field has been written. (Some fields might be skipped
 *          when specific \a flags are set.)
 */
bool VorbisCommentField::make(BinaryWriter &writer, VorbisCommentFlags flags)
{
    static const string context("making Vorbis comment  field");
    if(id().empty()) {
        addNotification(NotificationType::Critical, "The field ID is empty.", context);
    }
    try {
        // try to convert value to string
        string valueString;
        if(id() == VorbisCommentIds::cover()) {
            if(flags & VorbisCommentFlags::NoCovers) {
                return false;
            }
            // make cover
            if(value().type() != TagDataType::Picture) {
                addNotification(NotificationType::Critical, "Assigned value of cover field is not picture data.", context);
                throw InvalidDataException();
            }
            try {
                FlacMetaDataBlockPicture pictureBlock(value());
                pictureBlock.setPictureType(typeInfo());

                const auto requiredSize = pictureBlock.requiredSize();
                auto buffer = make_unique<char[]>(requiredSize);
                stringstream bufferStream(ios_base::in | ios_base::out | ios_base::binary);
                bufferStream.exceptions(ios_base::failbit | ios_base::badbit);
                bufferStream.rdbuf()->pubsetbuf(buffer.get(), requiredSize);

                pictureBlock.make(bufferStream);
                valueString = encodeBase64(reinterpret_cast<byte *>(buffer.get()), requiredSize);
            } catch(...) {
                catchIoFailure();
                addNotification(NotificationType::Critical, "An IO error occured when writing the METADATA_BLOCK_PICTURE struct.", context);
                throw Failure();
            }
        } else {
            // make normal string value
            valueString = value().toString();
        }
        writer.writeUInt32LE(id().size() + 1 + valueString.size());
        writer.writeString(id());
        writer.writeChar('=');
        writer.writeString(valueString);
    } catch(const ConversionException &) {
        addNotification(NotificationType::Critical, "Assigned value can not be converted appropriately.", context);
        throw InvalidDataException();
    }
    return true;
}

}
