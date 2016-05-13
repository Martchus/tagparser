#include "./vorbiscomment.h"
#include "./vorbiscommentids.h"

#include "../ogg/oggiterator.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/io/copy.h>
#include <c++utilities/misc/memory.h>

#include <map>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::VorbisComment
 * \brief Implementation of Media::Tag for Vorbis comments.
 */

const TagValue &VorbisComment::value(KnownField field) const
{
    switch(field) {
    case KnownField::Vendor:
        return vendor();
    default:
        return FieldMapBasedTag<VorbisCommentField, CaseInsensitiveStringComparer>::value(field);
    }
}

bool VorbisComment::setValue(KnownField field, const TagValue &value)
{
    switch(field) {
    case KnownField::Vendor:
        setVendor(value);
        return true;
    default:
        return FieldMapBasedTag<VorbisCommentField, CaseInsensitiveStringComparer>::setValue(field, value);
    }
}

string VorbisComment::fieldId(KnownField field) const
{
    using namespace VorbisCommentIds;
    switch(field) {
    case KnownField::Album: return album();
    case KnownField::Artist: return artist();
    case KnownField::Comment: return comment();
    case KnownField::Cover: return cover();
    case KnownField::Year: return date();
    case KnownField::Title: return title();
    case KnownField::Genre: return genre();
    case KnownField::TrackPosition: return trackNumber();
    case KnownField::DiskPosition: return diskNumber();
    case KnownField::PartNumber: return partNumber();
    case KnownField::Composer: return composer();
    case KnownField::Encoder: return encoder();
    case KnownField::EncoderSettings: return encoderSettings();
    case KnownField::Description: return description();
    case KnownField::RecordLabel: return label();
    case KnownField::Performers: return performer();
    case KnownField::Language: return language();
    case KnownField::Lyricist: return lyricist();
    default: return string();
    }
}

KnownField VorbisComment::knownField(const string &id) const
{
    using namespace VorbisCommentIds;
    static const map<string, KnownField> fieldMap({
        {album(), KnownField::Album},
        {artist(), KnownField::Artist},
        {comment(), KnownField::Comment},
        {cover(), KnownField::Cover},
        {date(), KnownField::Year},
        {title(), KnownField::Title},
        {genre(), KnownField::Genre},
        {trackNumber(), KnownField::TrackPosition},
        {diskNumber(), KnownField::DiskPosition},
        {partNumber(), KnownField::PartNumber},
        {composer(), KnownField::Composer},
        {encoder(), KnownField::Encoder},
        {encoderSettings(), KnownField::EncoderSettings},
        {description(), KnownField::Description},
        {label(), KnownField::RecordLabel},
        {performer(), KnownField::Performers},
        {lyricist(), KnownField::Lyricist}
    });
    try {
        return fieldMap.at(id);
    } catch(out_of_range &) {
        return KnownField::Invalid;
    }
}

/*!
 * \brief Parses tag information using the specified OGG \a iterator.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisComment::parse(OggIterator &iterator, VorbisCommentFlags flags, size_t offset)
{
    // prepare parsing
    invalidateStatus();
    static const string context("parsing Vorbis comment");
    auto startOffset = iterator.currentSegmentOffset() + offset;
    iterator.seekForward(offset);
    try {
        // read signature: 0x3 + "vorbis"
        char sig[8];
        bool skipSignature = flags & VorbisCommentFlags::NoSignature;
        if(!skipSignature) {
            iterator.read(sig, 7);
            skipSignature = (ConversionUtilities::BE::toUInt64(sig) & 0xffffffffffffff00u) == 0x03766F7262697300u;
        }
        if(skipSignature) {
            // read vendor (length prefixed string)
            {
                iterator.read(sig, 4);
                const auto vendorSize = LE::toUInt32(sig);
                if(iterator.currentCharacterOffset() + vendorSize <= iterator.streamSize()) {
                    auto buff = make_unique<char []>(vendorSize);
                    iterator.read(buff.get(), vendorSize);
                    m_vendor.assignData(move(buff), vendorSize, TagDataType::Text, TagTextEncoding::Utf8);
                    // TODO: Is the vendor string actually UTF-8 (like the field values)?
                } else {
                    addNotification(NotificationType::Critical, "Vendor information is truncated.", context);
                    throw TruncatedDataException();
                }
            }
            // read field count
            iterator.read(sig, 4);
            uint32 fieldCount = LE::toUInt32(sig);
            VorbisCommentField field;
            const string &fieldId = field.id();
            for(uint32 i = 0; i < fieldCount; ++i) {
                // read fields
                try {
                    field.parse(iterator);
                    fields().insert(pair<fieldType::identifierType, fieldType>(fieldId, field));
                } catch(const TruncatedDataException &) {
                    addNotifications(field);
                    throw;
                } catch(const Failure &) {
                    // nothing to do here since notifications will be added anyways
                }
                addNotifications(field);
                field.invalidateNotifications();
            }
            if(!(flags & VorbisCommentFlags::NoFramingByte)) {
                iterator.seekForward(1); // skip framing byte
            }
            m_size = static_cast<uint32>(static_cast<uint64>(iterator.currentCharacterOffset()) - startOffset);
        } else {
            addNotification(NotificationType::Critical, "Signature is invalid.", context);
            throw InvalidDataException();
        }
    } catch(const TruncatedDataException &) {
        m_size = static_cast<uint32>(static_cast<uint64>(iterator.currentCharacterOffset()) - startOffset);
        addNotification(NotificationType::Critical, "Vorbis comment is truncated.", context);
        throw;
    }
}

/*!
 * \brief Writes tag information to the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void VorbisComment::make(std::ostream &stream, VorbisCommentFlags flags)
{
    // prepare making
    invalidateStatus();
    static const string context("making Vorbis comment");
    string vendor;
    try {
        m_vendor.toString(vendor);
    } catch(const ConversionException &) {
        addNotification(NotificationType::Warning, "Can not convert the assigned vendor to string.", context);
    }
    BinaryWriter writer(&stream);
    if(!(flags & VorbisCommentFlags::NoSignature)) {
        // write signature
        static const char sig[7] = {0x03, 0x76, 0x6F, 0x72, 0x62, 0x69, 0x73};
        stream.write(sig, sizeof(sig));
    }
    // write vendor
    writer.writeUInt32LE(vendor.size());
    writer.writeString(vendor);
    // write field count
    writer.writeUInt32LE(fieldCount());
    // write fields
    for(auto i : fields()) {
        VorbisCommentField &field = i.second;
        if(!field.value().isEmpty()) {
            try {
                field.make(writer);
            } catch(const Failure &) {
                // nothing to do here since notifications will be added anyways
            }
            // add making notifications
            addNotifications(context, field);
            field.invalidateNotifications();
        }
    }
    // write framing byte
    if(!(flags & VorbisCommentFlags::NoFramingByte)) {
        stream.put(0x01);
    }
}

}
