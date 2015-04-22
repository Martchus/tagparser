#include "vorbiscommentfield.h"
#include "vorbiscommentids.h"

#include "../ogg/oggiterator.h"

#include "../id3/id3v2frame.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/misc/memory.h>

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
    if(uint32 size = LE::toUInt32(buff)) { // read size
        // read data
        unique_ptr<char[]> data = make_unique<char []>(size);
        iterator.read(data.get(), size);
        uint32 idSize = 0;
        for(const char *i = data.get(), *end = data.get() + size; i != end && (*i) != '='; ++i, ++idSize)
            ;
        // extract id
        setId(string(data.get(), idSize));
        if(!idSize) {
            // empty field ID
            addNotification(NotificationType::Critical, "The field ID is empty.", context);
            throw InvalidDataException();
        } else if(id() == VorbisCommentIds::cover()) {
            // extract cover value
            vector<char> buffer = decodeBase64(string(data.get() + idSize + 1, size - idSize - 1));
            Id3v2FrameHelper helper(id(), *this);
            byte type;
            helper.parsePicture(buffer.data(), buffer.size(), value(), type);
            setTypeInfo(type);
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
    // try to convert value to string
    try {
        string valueString;
        if(id() == VorbisCommentIds::cover()) {
            // make cover
            Id3v2FrameHelper helper(id(), *this);
            vector<char> buffer;
            helper.makePicture(buffer, value(), isTypeInfoAssigned() ? typeInfo() : 0);
            valueString = encodeBase64(buffer);
        } else {
            // make normal string value
            valueString = value().toString();
        }
        // write size
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
