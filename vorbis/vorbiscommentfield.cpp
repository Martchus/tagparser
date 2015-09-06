#include "vorbiscommentfield.h"
#include "vorbiscommentids.h"

#include "tagparser/ogg/oggiterator.h"

#include "tagparser/id3/id3v2frame.h"

#include "tagparser/exceptions.h"

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
                stringstream ss(ios_base::in | ios_base::out | ios_base::binary);
                ss.exceptions(ios_base::failbit | ios_base::badbit);
                ss.rdbuf()->pubsetbuf(reinterpret_cast<char *>(decoded.first.get()), decoded.second);
                BinaryReader reader(&ss);
                setTypeInfo(reader.readUInt32BE());
                auto size = reader.readUInt32BE();
                value().setMimeType(reader.readString(size));
                size = reader.readUInt32BE();
                value().setDescription(reader.readString(size));
                // skip width, height, color depth, number of colors used
                ss.seekg(4 * 4, ios_base::cur);
                size = reader.readUInt32BE();
                auto data = make_unique<char[]>(size);
                ss.read(data.get(), size);
                value().assignData(move(data), size, TagDataType::Picture);
            } catch (const ios_base::failure &) {
                addNotification(NotificationType::Critical, "An IO error occured when reading the METADATA_BLOCK_PICTURE struct.", context);
                throw Failure();
            } catch (const ConversionException &) {
                addNotification(NotificationType::Critical, "Base64 data from METADATA_BLOCK_PICTURE is invalid.", context);
                throw InvalidDataException();
            }
        } else if(id().size() + 1 < size) {
            // extract other values (as string)
            setValue(string(data.get() + idSize + 1, size - idSize - 1));
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
                uint32 dataSize = 32 + value().mimeType().size() + value().description().size() + value().dataSize();
                auto buffer = make_unique<char[]>(dataSize);
                stringstream ss(ios_base::in | ios_base::out | ios_base::binary);
                ss.exceptions(ios_base::failbit | ios_base::badbit);
                ss.rdbuf()->pubsetbuf(buffer.get(), dataSize);
                BinaryWriter writer(&ss);
                writer.writeUInt32BE(typeInfo());
                writer.writeUInt32BE(value().mimeType().size());
                writer.writeString(value().mimeType());
                writer.writeUInt32BE(value().description().size());
                writer.writeString(value().description());
                writer.writeUInt32BE(0); // skip width
                writer.writeUInt32BE(0); // skip height
                writer.writeUInt32BE(0); // skip color depth
                writer.writeUInt32BE(0); // skip number of colors used
                writer.writeUInt32BE(value().dataSize());
                writer.write(value().dataPointer(), value().dataSize());
                valueString = encodeBase64(reinterpret_cast<byte *>(buffer.get()), dataSize);
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
