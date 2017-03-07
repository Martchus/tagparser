#include "./vorbiscomment.h"
#include "./vorbiscommentids.h"

#include "../ogg/oggiterator.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/io/copy.h>

#include <map>
#include <memory>

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
        return FieldMapBasedTag<VorbisComment>::value(field);
    }
}

bool VorbisComment::setValue(KnownField field, const TagValue &value)
{
    switch(field) {
    case KnownField::Vendor:
        setVendor(value);
        return true;
    default:
        return FieldMapBasedTag<VorbisComment>::setValue(field, value);
    }
}

VorbisComment::IdentifierType VorbisComment::internallyGetFieldId(KnownField field) const
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

KnownField VorbisComment::internallyGetKnownField(const IdentifierType &id) const
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
 * \brief Internal implementation for parsing.
 */
template<class StreamType>
void VorbisComment::internalParse(StreamType &stream, uint64 maxSize, VorbisCommentFlags flags)
{
    // prepare parsing
    invalidateStatus();
    static const string context("parsing Vorbis comment");
    uint64 startOffset = static_cast<uint64>(stream.tellg());
    try {
        // read signature: 0x3 + "vorbis"
        char sig[8];
        bool skipSignature = flags & VorbisCommentFlags::NoSignature;
        if(!skipSignature) {
            CHECK_MAX_SIZE(7);
            stream.read(sig, 7);
            skipSignature = (ConversionUtilities::BE::toUInt64(sig) & 0xffffffffffffff00u) == 0x03766F7262697300u;
        }
        if(skipSignature) {
            // read vendor (length prefixed string)
            {
                CHECK_MAX_SIZE(4);
                stream.read(sig, 4);
                const auto vendorSize = LE::toUInt32(sig);
                if(vendorSize <= maxSize) {
                    auto buff = make_unique<char []>(vendorSize);
                    stream.read(buff.get(), vendorSize);
                    m_vendor.assignData(move(buff), vendorSize, TagDataType::Text, TagTextEncoding::Utf8);
                    // TODO: Is the vendor string actually UTF-8 (like the field values)?
                } else {
                    addNotification(NotificationType::Critical, "Vendor information is truncated.", context);
                    throw TruncatedDataException();
                }
                maxSize -= vendorSize;
            }
            // read field count
            CHECK_MAX_SIZE(4);
            stream.read(sig, 4);
            uint32 fieldCount = LE::toUInt32(sig);
            VorbisCommentField field;
            const string &fieldId = field.id();
            for(uint32 i = 0; i < fieldCount; ++i) {
                // read fields
                try {
                    field.parse(stream, maxSize);
                    fields().emplace(fieldId, field);
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
                stream.ignore(); // skip framing byte
            }
            m_size = static_cast<uint32>(static_cast<uint64>(stream.tellg()) - startOffset);
        } else {
            addNotification(NotificationType::Critical, "Signature is invalid.", context);
            throw InvalidDataException();
        }
    } catch(const TruncatedDataException &) {
        m_size = static_cast<uint32>(static_cast<uint64>(stream.tellg()) - startOffset);
        addNotification(NotificationType::Critical, "Vorbis comment is truncated.", context);
        throw;
    }
}

/*!
 * \brief Parses tag information using the specified OGG \a iterator.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisComment::parse(OggIterator &iterator, VorbisCommentFlags flags)
{
    internalParse(iterator, iterator.streamSize(), flags);
}

/*!
 * \brief Parses tag information using the specified OGG \a iterator.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void VorbisComment::parse(istream &stream, uint64 maxSize, VorbisCommentFlags flags)
{
    internalParse(stream, maxSize, flags);
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
    // write field count later
    const auto fieldCountOffset = stream.tellp();
    writer.writeUInt32LE(0);
    // write fields
    uint32 fieldsWritten = 0;
    for(auto i : fields()) {
        VorbisCommentField &field = i.second;
        if(!field.value().isEmpty()) {
            try {
                if(field.make(writer, flags)) {
                    ++fieldsWritten;
                }
            } catch(const Failure &) {
                // nothing to do here since notifications will be added anyways
            }
            // add making notifications
            addNotifications(context, field);
            field.invalidateNotifications();
        }
    }
    // write field count
    const auto framingByteOffset = stream.tellp();
    stream.seekp(fieldCountOffset);
    writer.writeUInt32LE(fieldsWritten);
    stream.seekp(framingByteOffset);
    // write framing byte
    if(!(flags & VorbisCommentFlags::NoFramingByte)) {
        stream.put(0x01);
    }
}

}
