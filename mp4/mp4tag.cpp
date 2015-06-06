#include "mp4container.h"
#include "mp4tag.h"
#include "mp4ids.h"
#include "mp4atom.h"
#include "../exceptions.h"

#include <c++utilities/io/binarywriter.h>
#include <c++utilities/conversion/stringconversion.h>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::Mp4Tag
 * \brief Implementation of Media::Tag for the MP4 container.
 */

bool Mp4Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    switch(encoding) {
    case TagTextEncoding::Utf8:
        return true;
    case TagTextEncoding::Utf16BigEndian:
        return true;
    default:
        return false;
    }
}

const TagValue &Mp4Tag::value(KnownField field) const
{
    switch(field) {
    case KnownField::Genre: {
        const TagValue &value = FieldMapBasedTag<fieldType>::value(Mp4TagAtomIds::Genre);
        if(!value.isEmpty()) {
            return value;
        } else {
            return FieldMapBasedTag<fieldType>::value(Mp4TagAtomIds::PreDefinedGenre);
        }
    } case KnownField::EncoderSettings:
        return value(Mp4TagExtendedMeanIds::iTunes, Mp4TagExtendedNameIds::cdec);
    default:
        return FieldMapBasedTag<fieldType>::value(field);
    }
}

/*!
 * \brief Returns the value of the field with the specified \a mean and \a name attributes.
 */
const TagValue &Mp4Tag::value(const string mean, const string name) const
{
    auto range = fields().equal_range(Mp4TagAtomIds::Extended);
    for(auto i = range.first; i != range.second; ++i) {
        if(i->second.mean() == mean && i->second.name() == name) {
            return i->second.value();
        }
    }
    return TagValue::empty();
}

uint32 Mp4Tag::fieldId(KnownField field) const
{
    using namespace Mp4TagAtomIds;
    switch(field) {
    case KnownField::Album: return Album;
    case KnownField::Artist: return Artist;
    case KnownField::Comment: return Comment;
    case KnownField::Year: return Year;
    case KnownField::Title: return Title;
    case KnownField::Genre: return Genre;
    case KnownField::TrackPosition: return TrackPosition;
    case KnownField::DiskPosition: return DiskPosition;
    case KnownField::Composer: return Composer;
    case KnownField::Encoder: return Encoder;
    case KnownField::Bpm: return Bpm;
    case KnownField::Cover: return Cover;
    case KnownField::Rating: return Rating;
    case KnownField::Grouping: return Grouping;
    case KnownField::Description: return Description;
    case KnownField::Lyrics: return Lyrics;
    case KnownField::RecordLabel: return RecordLabel;
    case KnownField::Performers: return Performers;
    case KnownField::Lyricist: return Lyricist;
    case KnownField::EncoderSettings: return Extended;
    default: return 0;
    }
}

KnownField Mp4Tag::knownField(const uint32 &id) const
{
    using namespace Mp4TagAtomIds;
    switch(id) {
    case Album: return KnownField::Album;
    case Artist: return KnownField::Artist;
    case Comment: return KnownField::Comment;
    case Year: return KnownField::Year;
    case Title: return KnownField::Title;
    case PreDefinedGenre: case Genre: return KnownField::Genre;
    case TrackPosition: return KnownField::TrackPosition;
    case DiskPosition: return KnownField::DiskPosition;
    case Composer: return KnownField::Composer;
    case Encoder: return KnownField::Encoder;
    case Bpm: return KnownField::Bpm;
    case Cover: return KnownField::Cover;
    case Rating: return KnownField::Rating;
    case Grouping: return KnownField::Grouping;
    case Description: return KnownField::Description;
    case Lyrics: return KnownField::Lyrics;
    case RecordLabel: return KnownField::RecordLabel;
    case Performers: return KnownField::Performers;
    case Lyricist: return KnownField::Lyricist;
    default: return KnownField::Invalid;
    }
}

bool Mp4Tag::setValue(KnownField field, const TagValue &value)
{
    switch(field) {
    case KnownField::Genre:
        switch(value.type()) {
        case TagDataType::StandardGenreIndex:
            if(fields().count(Mp4TagAtomIds::Genre)) {
                fields().erase(Mp4TagAtomIds::Genre);
            }
            return FieldMapBasedTag<fieldType>::setValue(Mp4TagAtomIds::PreDefinedGenre, value);
        default:
            if(fields().count(Mp4TagAtomIds::PreDefinedGenre)) {
                fields().erase(Mp4TagAtomIds::PreDefinedGenre);
            }
            return FieldMapBasedTag<fieldType>::setValue(Mp4TagAtomIds::Genre, value);
        }
    case KnownField::EncoderSettings:
        return setValue(Mp4TagExtendedMeanIds::iTunes, Mp4TagExtendedNameIds::cdec, value);
    default:
        return FieldMapBasedTag<fieldType>::setValue(field, value);
    }
}

/*!
 * \brief Assigns the given \a value to the field with the specified \a mean and \a name attributes.
 */
bool Mp4Tag::setValue(const string mean, const string name, const TagValue &value)
{
    auto range = fields().equal_range(Mp4TagAtomIds::Extended);
    for(auto i = range.first; i != range.second; ++i) {
        if(i->second.mean() == mean && i->second.name() == name) {
            i->second.setValue(value);
            return true;
        }
    }
    fields().insert(make_pair(Mp4TagAtomIds::Extended, fieldType(mean, name, value)));
    return true;
}

bool Mp4Tag::hasField(KnownField field) const
{
    switch(field) {
    case KnownField::Genre:
        return FieldMapBasedTag<fieldType>::hasField(Mp4TagAtomIds::PreDefinedGenre)
                || FieldMapBasedTag<fieldType>::hasField(Mp4TagAtomIds::Genre);
    default:
        return FieldMapBasedTag<fieldType>::hasField(field);
    }
}

/*!
 * \brief Parses tag information from the specified \a metaAtom.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Mp4Tag::parse(Mp4Atom &metaAtom)
{
    invalidateStatus();
    static const string context("parsing MP4 tag");
    istream &stream = metaAtom.container().stream();
    BinaryReader &reader = metaAtom.container().reader();
    m_size = metaAtom.totalSize();
    Mp4Atom *subAtom = nullptr;
    try {
        metaAtom.childById(Mp4AtomIds::HandlerReference);
    } catch(Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse child atoms of meta atom (stores hdlr and ilst atoms).", context);
    }
    if(subAtom) {
        stream.seekg(subAtom->startOffset() + subAtom->headerSize());
        int versionByte = reader.readByte();
        if(versionByte != 0) {
            addNotification(NotificationType::Warning, "Version is unknown.", context);
        }
        if(reader.readUInt24BE()) {
            addNotification(NotificationType::Warning, "Flags (hdlr atom) aren't set to 0.", context);
        }
        if(reader.readInt32BE()) {
            addNotification(NotificationType::Warning, "Predefined 32-bit integer (hdlr atom) ins't set to 0.", context);
        }
        uint64 handlerType = reader.readUInt64BE();
        if(/*(((handlerType & 0xFFFFFFFF00000000) >> 32) != 0x6D647461) && */(handlerType != 0x6d6469726170706c)) {
            addNotification(NotificationType::Warning, "Handler type (value in hdlr atom) is unknown. Trying to parse meta information anyhow.", context);
        }
        m_version = ConversionUtilities::numberToString(versionByte);
    } else {
        //addParsingNotification(NotificationType::Warning, "No hdlr atom found (handler of meta information). Trying to parse meta information anyhow.");
        m_version.clear();
    }
    try {
        subAtom = metaAtom.childById(Mp4AtomIds::ItunesList);
    } catch(Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse child atoms of meta atom (stores hdlr and ilst atoms).", context);
    }
    if(subAtom) {
        Mp4TagField tagField;
        for(Mp4Atom *child : *subAtom) {
            try {
                child->parse();
                tagField.invalidateNotifications();
                tagField.reparse(*child);
                fields().insert(pair<fieldType::identifierType, fieldType>(child->id(), tagField));
            } catch(Failure &) {
            }
            addNotifications(context, *child);
            addNotifications(context, tagField);
        }
    } else {
        addNotification(NotificationType::Warning, "No ilst atom found (stores attached meta information).", context);
        throw NoDataFoundException();
    }
}

/*!
 * \brief Writes tag information to the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void Mp4Tag::make(ostream &stream)
{
    invalidateStatus();
    static const string context("making MP4 tag");
    // write meta atom
    ostream::pos_type metaOff = stream.tellp();
    static const byte metaData[8] = {
        0x00, 0x00, 0x00, 0x00, 0x6D, 0x65, 0x74, 0x61
    };
    stream.write(reinterpret_cast<const char *>(metaData), sizeof(metaData));
    // write hdlr atom
    static const byte hdlrData[37] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x68, 0x64, 0x6C, 0x72, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6D, 0x64, 0x69, 0x72, 0x61, 0x70, 0x70, 0x6C,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    stream.write(reinterpret_cast<const char *>(hdlrData), sizeof(hdlrData));
    // write ilst atom
    ostream::pos_type ilstOff = stream.tellp();
    static const byte ilstData[8] = {
          0x00, 0x00, 0x00, 0x00, 0x69, 0x6C, 0x73, 0x74
    };
    stream.write(reinterpret_cast<const char *>(ilstData), sizeof(ilstData));
    // ensure there is only one genre atom (prefer genre as string)
    if(fields().count(Mp4TagAtomIds::PreDefinedGenre)
            && fields().count(Mp4TagAtomIds::Genre)) {
        fields().erase(Mp4TagAtomIds::PreDefinedGenre);
    }
    // write actual tag data
    int tagFieldsWritten = 0;
    for(auto i = fields().begin(), end = fields().end(); i != end; ++i) {
        Mp4TagField &field = i->second;
        if(field.value().isEmpty()) {
            continue;
        } else {
            field.invalidateNotifications();
            try {
                field.make(stream);
                ++tagFieldsWritten;
            } catch(Failure &) {
                // nothing to do since notifications will be added anyways
            }
            addNotifications(context, field);
        }
    }
    if(!tagFieldsWritten) {
        addNotification(NotificationType::Warning, "No tag atoms have be written.", context);
    }
    Mp4Atom::seekBackAndWriteAtomSize(stream, ilstOff);
    Mp4Atom::seekBackAndWriteAtomSize(stream, metaOff);
}

}
