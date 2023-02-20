#include "./mp4tag.h"
#include "./mp4atom.h"
#include "./mp4container.h"
#include "./mp4ids.h"

#include "../exceptions.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/binarywriter.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::Mp4ExtendedFieldId
 * \brief The Mp4ExtendedFieldId specifies parameter for an extended field denoted via Mp4TagAtomIds::Extended.
 */

/*!
 * \brief Constructs a new instance for the specified \a field.
 * \remarks The instance will be invalid if no extended field parameter for \a field are known.
 */
Mp4ExtendedFieldId::Mp4ExtendedFieldId(KnownField field)
{
    switch (field) {
    case KnownField::EncoderSettings:
        mean = Mp4TagExtendedMeanIds::iTunes;
        name = Mp4TagExtendedNameIds::cdec;
        updateOnly = false;
        break;
    case KnownField::RecordLabel:
        mean = Mp4TagExtendedMeanIds::iTunes;
        name = Mp4TagExtendedNameIds::label;
        updateOnly = true; // set record label via extended field only if extended field is already present
        break;
    default:;
    }
}

/*!
 * \class TagParser::Mp4Tag
 * \brief Implementation of TagParser::Tag for the MP4 container.
 */

bool Mp4Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    switch (encoding) {
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
    switch (field) {
    case KnownField::Genre: {
        const TagValue &value = FieldMapBasedTag<Mp4Tag>::value(Mp4TagAtomIds::Genre);
        if (!value.isEmpty()) {
            return value;
        } else {
            return FieldMapBasedTag<Mp4Tag>::value(Mp4TagAtomIds::PreDefinedGenre);
        }
    }
    case KnownField::EncoderSettings:
        return this->value(Mp4TagExtendedMeanIds::iTunes, Mp4TagExtendedNameIds::cdec);
    case KnownField::RecordLabel: {
        const TagValue &value = FieldMapBasedTag<Mp4Tag>::value(Mp4TagAtomIds::RecordLabel);
        if (!value.isEmpty()) {
            return value;
        } else {
            return this->value(Mp4TagExtendedMeanIds::iTunes, Mp4TagExtendedNameIds::label);
        }
    }
    default:
        return FieldMapBasedTag<Mp4Tag>::value(field);
    }
}

std::vector<const TagValue *> Mp4Tag::values(KnownField field) const
{
    auto values = FieldMapBasedTag<Mp4Tag>::values(field);
    if (const auto extendedId = Mp4ExtendedFieldId(field)) {
        auto range = fields().equal_range(Mp4TagAtomIds::Extended);
        for (auto i = range.first; i != range.second; ++i) {
            const auto &extendedField = i->second;
            if (extendedId.matches(extendedField)) {
                values.emplace_back(&extendedField.value());
                for (const auto &additionalData : extendedField.additionalData()) {
                    values.emplace_back(&additionalData.value);
                }
            }
        }
    }
    return values;
}

/*!
 * \brief Returns the value of the field with the specified \a mean and \a name attributes.
 * \remarks
 * - If there are multiple fields with specified \a mean and \a name only the first value will be returned.
 */
const TagValue &Mp4Tag::value(std::string_view mean, std::string_view name) const
{
    auto range = fields().equal_range(Mp4TagAtomIds::Extended);
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second.mean() == mean && i->second.name() == name) {
            return i->second.value();
        }
    }
    return TagValue::empty();
}

Mp4Tag::IdentifierType Mp4Tag::internallyGetFieldId(KnownField field) const
{
    using namespace Mp4TagAtomIds;
    switch (field) {
    case KnownField::Album:
        return Album;
    case KnownField::Artist:
        return Artist;
    case KnownField::Comment:
        return Comment;
    case KnownField::RecordDate:
        return Year;
    case KnownField::Title:
        return Title;
    case KnownField::Genre:
        return Genre;
    case KnownField::TrackPosition:
        return TrackPosition;
    case KnownField::DiskPosition:
        return DiskPosition;
    case KnownField::Composer:
        return Composer;
    case KnownField::Encoder:
        return Encoder;
    case KnownField::Bpm:
        return Bpm;
    case KnownField::Cover:
        return Cover;
    case KnownField::LawRating:
        return Rating;
    case KnownField::Grouping:
        return Grouping;
    case KnownField::Description:
        return Description;
    case KnownField::Lyrics:
        return Lyrics;
    case KnownField::RecordLabel:
        return RecordLabel;
    case KnownField::Performers:
        return Performers;
    case KnownField::Lyricist:
        return Lyricist;
    case KnownField::AlbumArtist:
        return AlbumArtist;
    case KnownField::Copyright:
        return Copyright;
    case KnownField::Conductor:
        return Conductor;
    case KnownField::Director:
        return Director;
    case KnownField::Publisher:
        return Publisher;
    case KnownField::SoundEngineer:
        return SoundEngineer;
    case KnownField::Producer:
        return Producer;
    case KnownField::ExecutiveProducer:
        return ExecutiveProducer;
    case KnownField::ArtDirector:
        return ArtDirector;
    case KnownField::Arranger:
        return Arranger;
    default:
        return 0;
    }
    // do not forget to extend Mp4Tag::internallyGetKnownField() and Mp4TagField::appropriateRawDataType() as well
}

KnownField Mp4Tag::internallyGetKnownField(const IdentifierType &id) const
{
    using namespace Mp4TagAtomIds;
    switch (id) {
    case Album:
        return KnownField::Album;
    case Artist:
        return KnownField::Artist;
    case Comment:
        return KnownField::Comment;
    case Year:
        return KnownField::RecordDate;
    case Title:
        return KnownField::Title;
    case PreDefinedGenre:
    case Genre:
        return KnownField::Genre;
    case TrackPosition:
        return KnownField::TrackPosition;
    case DiskPosition:
        return KnownField::DiskPosition;
    case Composer:
        return KnownField::Composer;
    case Encoder:
        return KnownField::Encoder;
    case Bpm:
        return KnownField::Bpm;
    case Cover:
        return KnownField::Cover;
    case Rating:
        return KnownField::LawRating;
    case Grouping:
        return KnownField::Grouping;
    case Description:
        return KnownField::Description;
    case Lyrics:
        return KnownField::Lyrics;
    case RecordLabel:
        return KnownField::RecordLabel;
    case Performers:
        return KnownField::Performers;
    case Lyricist:
        return KnownField::Lyricist;
    case AlbumArtist:
        return KnownField::AlbumArtist;
    case Copyright:
        return KnownField::Copyright;
    case Conductor:
        return KnownField::Conductor;
    case Director:
        return KnownField::Director;
    case Publisher:
        return KnownField::Publisher;
    case SoundEngineer:
        return KnownField::SoundEngineer;
    case Producer:
        return KnownField::Producer;
    case ExecutiveProducer:
        return KnownField::ExecutiveProducer;
    case ArtDirector:
        return KnownField::ArtDirector;
    case Arranger:
        return KnownField::Arranger;
    default:
        return KnownField::Invalid;
    }
    // do not forget to extend Mp4Tag::internallyGetFieldId() and Mp4TagField::appropriateRawDataType() as well
}

/*!
 * \brief Adds values from additional data atoms as well.
 */
void Mp4Tag::internallyGetValuesFromField(const Mp4Tag::FieldType &field, std::vector<const TagValue *> &values) const
{
    if (!field.value().isEmpty()) {
        values.emplace_back(&field.value());
    }
    for (const auto &value : field.additionalData()) {
        if (!value.value.isEmpty()) {
            values.emplace_back(&value.value);
        }
    }
}

bool Mp4Tag::setValue(KnownField field, const TagValue &value)
{
    switch (field) {
    case KnownField::Genre:
        switch (value.type()) {
        case TagDataType::StandardGenreIndex:
            fields().erase(Mp4TagAtomIds::Genre);
            return FieldMapBasedTag<Mp4Tag>::setValue(Mp4TagAtomIds::PreDefinedGenre, value);
        default:
            fields().erase(Mp4TagAtomIds::PreDefinedGenre);
            return FieldMapBasedTag<Mp4Tag>::setValue(Mp4TagAtomIds::Genre, value);
        }
    case KnownField::EncoderSettings:
        return setValue(Mp4TagExtendedMeanIds::iTunes, Mp4TagExtendedNameIds::cdec, value);
    case KnownField::RecordLabel:
        if (!this->value(Mp4TagExtendedMeanIds::iTunes, Mp4TagExtendedNameIds::label).isEmpty()) {
            setValue(Mp4TagExtendedMeanIds::iTunes, Mp4TagExtendedNameIds::label, value);
        }
        [[fallthrough]];
    default:
        return FieldMapBasedTag<Mp4Tag>::setValue(field, value);
    }
}

bool Mp4Tag::setValues(KnownField field, const std::vector<TagValue> &values)
{
    if (const auto extendedId = Mp4ExtendedFieldId(field)) {
        auto valuesIterator = values.cbegin();
        auto range = fields().equal_range(Mp4TagAtomIds::Extended);
        for (; valuesIterator != values.cend() && range.first != range.second;) {
            if (!valuesIterator->isEmpty()) {
                auto &extendedField = range.first->second;
                if (extendedId.matches(extendedField) && (!extendedId.updateOnly || !extendedField.value().isEmpty())) {
                    extendedField.clearValue();
                    extendedField.setValue(*valuesIterator);
                    // note: Not sure which extended tag fields support multiple data atoms and which don't. Let's simply use
                    //       only one data atom per extended field here and get rid of any possibly assigned additional data
                    //       atoms.
                    ++valuesIterator;
                }
                ++range.first;
            } else {
                ++valuesIterator;
            }
        }
        for (; valuesIterator != values.cend(); ++valuesIterator) {
            if (valuesIterator->isEmpty()) {
                fields().emplace(std::piecewise_construct, std::forward_as_tuple(Mp4TagAtomIds::Extended),
                    std::forward_as_tuple(extendedId.mean, extendedId.name, *valuesIterator));
            }
        }
        for (; range.first != range.second; ++range.first) {
            range.first->second.clearValue();
        }
    }
    return FieldMapBasedTag<Mp4Tag>::setValues(field, values);
}

/*!
 * \brief Assigns the given \a value to the field with the specified \a mean and \a name attributes.
 * \remarks
 * - If there are multiple fields with specified \a mean and \a name only the first will be altered.
 * - If no field is present, a new one will be created.
 * - If \a value is empty, the field will be removed.
 */
bool Mp4Tag::setValue(std::string_view mean, std::string_view name, const TagValue &value)
{
    auto range = fields().equal_range(Mp4TagAtomIds::Extended);
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second.mean() == mean && i->second.name() == name) {
            i->second.setValue(value);
            return true;
        }
    }
    fields().insert(make_pair(Mp4TagAtomIds::Extended, FieldType(mean, name, value)));
    return true;
}

bool Mp4Tag::hasField(KnownField field) const
{
    switch (field) {
    case KnownField::Genre:
        return FieldMapBasedTag<Mp4Tag>::hasField(Mp4TagAtomIds::PreDefinedGenre) || FieldMapBasedTag<Mp4Tag>::hasField(Mp4TagAtomIds::Genre);
    default:
        return FieldMapBasedTag<Mp4Tag>::hasField(field);
    }
}

/*!
 * \brief Parses tag information from the specified \a metaAtom.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Mp4Tag::parse(Mp4Atom &metaAtom, Diagnostics &diag)
{
    static const string context("parsing MP4 tag");
    m_size = metaAtom.totalSize();
    istream &stream = metaAtom.container().stream();
    BinaryReader &reader = metaAtom.container().reader();
    if (metaAtom.totalSize() > numeric_limits<std::uint32_t>::max()) {
        diag.emplace_back(DiagLevel::Critical, "Can't handle such big \"meta\" atoms.", context);
        throw NotImplementedException();
    }
    Mp4Atom *subAtom = nullptr;
    try {
        metaAtom.childById(Mp4AtomIds::HandlerReference, diag);
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Critical, "Unable to parse child atoms of meta atom (stores hdlr and ilst atoms).", context);
    }
    if (subAtom) {
        stream.seekg(static_cast<streamoff>(subAtom->startOffset() + subAtom->headerSize()));
        int versionByte = reader.readByte();
        if (versionByte != 0) {
            diag.emplace_back(DiagLevel::Warning, "Version is unknown.", context);
        }
        if (reader.readUInt24BE()) {
            diag.emplace_back(DiagLevel::Warning, "Flags (hdlr atom) aren't set to 0.", context);
        }
        if (reader.readInt32BE()) {
            diag.emplace_back(DiagLevel::Warning, "Predefined 32-bit integer (hdlr atom) isn't set to 0.", context);
        }
        std::uint64_t handlerType = reader.readUInt64BE();
        if (/*(((handlerType & 0xFFFFFFFF00000000) >> 32) != 0x6D647461) && */ (handlerType != 0x6d6469726170706c)) {
            diag.emplace_back(DiagLevel::Warning, "Handler type (value in hdlr atom) is unknown. Trying to parse meta information anyhow.", context);
        }
        m_version = numberToString(versionByte);
    } else {
        m_version.clear();
    }
    try {
        subAtom = metaAtom.childById(Mp4AtomIds::ItunesList, diag);
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Critical, "Unable to parse child atoms of meta atom (stores hdlr and ilst atoms).", context);
    }
    if (!subAtom) {
        diag.emplace_back(DiagLevel::Warning, "No ilst atom found (stores attached meta information).", context);
        throw NoDataFoundException();
    }
    for (auto *child = subAtom->firstChild(); child; child = child->nextSibling()) {
        Mp4TagField tagField;
        try {
            child->parse(diag);
            tagField.reparse(*child, diag);
            fields().emplace(child->id(), std::move(tagField));
        } catch (const Failure &) {
        }
    }
}

/*!
 * \brief Prepares making.
 * \returns Returns a Mp4TagMaker object which can be used to actually make the tag.
 * \remarks The tag must NOT be mutated after making is prepared when it is intended to actually
 *          make the tag using the make method of the returned object.
 * \throws Throws TagParser::Failure or a derived exception when a making error occurs.
 *
 * This method might be useful when it is necessary to know the size of the tag before making it.
 * \sa make()
 */
Mp4TagMaker Mp4Tag::prepareMaking(Diagnostics &diag)
{
    return Mp4TagMaker(*this, diag);
}

/*!
 * \brief Writes tag information to the specified \a stream.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 */
void Mp4Tag::make(ostream &stream, Diagnostics &diag)
{
    prepareMaking(diag).make(stream, diag);
}

/*!
 * \class TagParser::Mp4TagMaker
 * \brief The Mp4TagMaker class helps writing MP4 tags.
 *
 * An instance can be obtained using the Mp4Tag::prepareMaking() method.
 */

/*!
 * \brief Prepares making the specified \a tag.
 * \sa See Mp4Tag::prepareMaking() for more information.
 */
Mp4TagMaker::Mp4TagMaker(Mp4Tag &tag, Diagnostics &diag)
    : m_tag(tag)
    ,
    // meta head, hdlr atom
    m_metaSize(8 + 37)
    ,
    // ilst head
    m_ilstSize(8)
    ,
    // ensure there only one genre atom is written (prefer genre as string)
    m_omitPreDefinedGenre(m_tag.fields().count(m_tag.hasField(Mp4TagAtomIds::Genre)))
{
    m_maker.reserve(m_tag.fields().size());
    for (auto &field : m_tag.fields()) {
        if (!field.second.value().isEmpty() && (!m_omitPreDefinedGenre || field.first != Mp4TagAtomIds::PreDefinedGenre)) {
            try {
                m_ilstSize += m_maker.emplace_back(field.second.prepareMaking(diag)).requiredSize();
            } catch (const Failure &) {
            }
        }
    }
    if (m_ilstSize != 8) {
        m_metaSize += m_ilstSize;
    }
    if (m_metaSize >= numeric_limits<std::uint32_t>::max()) {
        diag.emplace_back(DiagLevel::Critical, "Making such big tags is not implemented.", "making MP4 tag");
        throw NotImplementedException();
    }
}

/*!
 * \brief Saves the tag (specified when constructing the object) to the
 *        specified \a stream.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw TagParser::Failure or a derived exception.
 */
void Mp4TagMaker::make(ostream &stream, Diagnostics &diag)
{
    // write meta head
    BinaryWriter writer(&stream);
    writer.writeUInt32BE(static_cast<std::uint32_t>(m_metaSize));
    writer.writeUInt32BE(Mp4AtomIds::Meta);
    // write hdlr atom
    static const std::uint8_t hdlrData[37] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x68, 0x64, 0x6C, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x6D, 0x64, 0x69, 0x72, 0x61, 0x70, 0x70, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    stream.write(reinterpret_cast<const char *>(hdlrData), sizeof(hdlrData));
    if (m_ilstSize != 8) {
        // write ilst head
        writer.writeUInt32BE(static_cast<std::uint32_t>(m_ilstSize));
        writer.writeUInt32BE(Mp4AtomIds::ItunesList);
        // write fields
        for (auto &maker : m_maker) {
            maker.make(stream);
        }
    } else {
        // no fields to be written -> no ilst to be written
        diag.emplace_back(DiagLevel::Warning, "Tag is empty.", "making MP4 tag");
    }
}

} // namespace TagParser
