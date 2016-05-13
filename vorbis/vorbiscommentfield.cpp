#include "./vorbiscommentfield.h"
#include "./vorbiscommentids.h"

#include "../flac/flacmetadata.h"

#include "../ogg/oggiterator.h"

#include "../id3/id3v2frame.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/misc/memory.h>

#include <iostream>

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
VorbisCommentField::VorbisCommentField(const identifierType &id, const TagValue &value) :
    TagField<VorbisCommentField>(id, value)
{}

/*!
 * \brief Parses a field from the stream read using the specified \a reader.
 *
 * The position of the current character in the input stream is expected to be
 * at the beginning of the field to be parsed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisCommentField::parse(OggIterator &iterator)
{
    static const string context("parsing Vorbis comment  field");
    char buff[4];
    iterator.read(buff, 4);
    if(auto size = LE::toUInt32(buff)) { // read size
        if(iterator.currentCharacterOffset() + size <= iterator.streamSize()) {
            // read data
            auto data = make_unique<char []>(size);
            iterator.read(data.get(), size);
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
                    pictureBlock.parse(bufferStream);
                    setTypeInfo(pictureBlock.pictureType());
                } catch (const ios_base::failure &) {
                    addNotification(NotificationType::Critical, "An IO error occured when reading the METADATA_BLOCK_PICTURE struct.", context);
                    throw Failure();
                } catch (const ConversionException &) {
                    addNotification(NotificationType::Critical, "Base64 data from METADATA_BLOCK_PICTURE is invalid.", context);
                    throw InvalidDataException();
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
 * \brief Writes the field to a stream using the specified \a writer.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void VorbisCommentField::make(BinaryWriter &writer)
{
    static const string context("making Vorbis comment  field");
    if(id().empty()) {
        addNotification(NotificationType::Critical, "The field ID is empty.", context);
    }
    try {
        // try to convert value to string
        string valueString;
        if(id() == VorbisCommentIds::cover()) {
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
            } catch (const ios_base::failure &) {
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
    } catch(ConversionException &) {
        addNotification(NotificationType::Critical, "Assigned value can not be converted appropriately.", context);
        throw InvalidDataException();
    }
}

}
