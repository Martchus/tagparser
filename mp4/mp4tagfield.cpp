#include "./mp4tagfield.h"
#include "./mp4atom.h"
#include "./mp4container.h"
#include "./mp4ids.h"

#include "../exceptions.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <algorithm>
#include <limits>
#include <memory>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::Mp4TagField
 * \brief The Mp4TagField class is used by Mp4Tag to store the fields.
 */

/*!
 * \brief Constructs a new Mp4TagField.
 */
Mp4TagField::Mp4TagField()
    : m_parsedRawDataType(RawDataType::Reserved)
    , m_countryIndicator(0)
    , m_langIndicator(0)
{
}

/*!
 * \brief Constructs a new Mp4TagField with the specified \a id and \a value.
 */
Mp4TagField::Mp4TagField(IdentifierType id, const TagValue &value)
    : TagField<Mp4TagField>(id, value)
    , m_parsedRawDataType(RawDataType::Reserved)
    , m_countryIndicator(0)
    , m_langIndicator(0)
{
}

/*!
 * \brief Constructs a new Mp4TagField with the specified \a mean, \a name and \a value.
 *
 * The ID will be set to Mp4TagAtomIds::Extended indicating an tag field using the
 * reverse DNS style.
 *
 * \sa The last paragraph of <a href="http://atomicparsley.sourceforge.net/mpeg-4files.html">Known iTunes Metadata Atoms</a>
 *     gives additional information about this form of MP4 tag fields.
 */
Mp4TagField::Mp4TagField(std::string_view mean, std::string_view name, const TagValue &value)
    : Mp4TagField(Mp4TagAtomIds::Extended, value)
{
    m_name = name;
    m_mean = mean;
}

/*!
 * \brief Parses field information from the specified Mp4Atom.
 *
 * The specified atom should be a child atom of the "ilst" atom.
 * Each child of the "ilst" atom holds one field of the Mp4Tag.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void Mp4TagField::reparse(Mp4Atom &ilstChild, Diagnostics &diag)
{
    // prepare reparsing
    using namespace Mp4AtomIds;
    using namespace Mp4TagAtomIds;
    string context("parsing MP4 tag field");
    ilstChild.parse(diag); // ensure child has been parsed
    setId(ilstChild.id());
    context = "parsing MP4 tag field " + ilstChild.idToString();
    iostream &stream = ilstChild.stream();
    BinaryReader &reader = ilstChild.container().reader();
    int dataAtomFound = 0, meanAtomFound = 0, nameAtomFound = 0;
    for (Mp4Atom *dataAtom = ilstChild.firstChild(); dataAtom; dataAtom = dataAtom->nextSibling()) {
        try {
            dataAtom->parse(diag);
            if (dataAtom->id() == Mp4AtomIds::Data) {
                if (dataAtom->dataSize() < 8) {
                    diag.emplace_back(DiagLevel::Warning,
                        "Truncated child atom \"data\" in tag atom (ilst child) found. It will be ignored and discarded when applying changes.",
                        context);
                    continue;
                }
                auto *val = &value();
                auto *rawDataType = &m_parsedRawDataType;
                auto *countryIndicator = &m_countryIndicator;
                auto *languageIndicator = &m_langIndicator;
                if (++dataAtomFound > 1) {
                    if (dataAtomFound == 2) {
                        diag.emplace_back(DiagLevel::Warning,
                            "Multiple \"data\" child atom in tag atom (ilst child) found. It will be ignored but preserved when applying changes.",
                            context);
                    }
                    auto &additionalData = m_additionalData.emplace_back();
                    val = &additionalData.value;
                    rawDataType = &additionalData.rawDataType;
                    countryIndicator = &additionalData.countryIndicator;
                    languageIndicator = &additionalData.languageIndicator;
                }
                stream.seekg(static_cast<streamoff>(dataAtom->dataOffset()));
                if (reader.readByte() != 0) {
                    diag.emplace_back(DiagLevel::Warning,
                        "The version indicator byte is not zero, the tag atom might be unsupported and hence not be parsed correctly.", context);
                }
                setTypeInfo(*rawDataType = reader.readUInt24BE());
                try { // try to show warning if parsed raw data type differs from expected raw data type for this atom id
                    const vector<std::uint32_t> expectedRawDataTypes = this->expectedRawDataTypes();
                    if (find(expectedRawDataTypes.cbegin(), expectedRawDataTypes.cend(), m_parsedRawDataType) == expectedRawDataTypes.cend()) {
                        diag.emplace_back(DiagLevel::Warning, "Unexpected data type indicator found.", context);
                    }
                } catch (const Failure &) {
                    // tag id is unknown, it is not possible to validate parsed data type
                }
                *countryIndicator = reader.readUInt16BE(); // FIXME: use locale within the tag value
                *languageIndicator = reader.readUInt16BE(); // FIXME: use locale within the tag value
                switch (*rawDataType) {
                case RawDataType::Utf8:
                case RawDataType::Utf16:
                    stream.seekg(static_cast<streamoff>(dataAtom->dataOffset() + 8));
                    val->assignText(reader.readString(dataAtom->dataSize() - 8),
                        (m_parsedRawDataType == RawDataType::Utf16) ? TagTextEncoding::Utf16BigEndian : TagTextEncoding::Utf8);
                    break;
                case RawDataType::Gif:
                case RawDataType::Jpeg:
                case RawDataType::Png:
                case RawDataType::Bmp: {
                    switch (m_parsedRawDataType) {
                    case RawDataType::Gif:
                        val->setMimeType("image/gif");
                        break;
                    case RawDataType::Jpeg:
                        val->setMimeType("image/jpeg");
                        break;
                    case RawDataType::Png:
                        val->setMimeType("image/png");
                        break;
                    case RawDataType::Bmp:
                        val->setMimeType("image/bmp");
                        break;
                    default:;
                    }
                    const auto coverSize = static_cast<streamoff>(dataAtom->dataSize() - 8);
                    auto coverData = make_unique<char[]>(static_cast<size_t>(coverSize));
                    stream.read(coverData.get(), coverSize);
                    val->assignData(std::move(coverData), static_cast<size_t>(coverSize), TagDataType::Picture);
                    break;
                }
                case RawDataType::BeSignedInt: {
                    int number = 0;
                    if (dataAtom->dataSize() > (8 + 4)) {
                        diag.emplace_back(DiagLevel::Warning, "Data atom stores integer of invalid size. Trying to read data anyways.", context);
                    }
                    if (dataAtom->dataSize() >= (8 + 4)) {
                        number = reader.readInt32BE();
                    } else if (dataAtom->dataSize() == (8 + 2)) {
                        number = reader.readInt16BE();
                    } else if (dataAtom->dataSize() == (8 + 1)) {
                        number = reader.readChar();
                    }
                    switch (ilstChild.id()) {
                    case PreDefinedGenre: // consider number as standard genre index
                        val->assignStandardGenreIndex(number);
                        break;
                    default:
                        val->assignInteger(number);
                    }
                    break;
                }
                case RawDataType::BeUnsignedInt: {
                    int number = 0;
                    if (dataAtom->dataSize() > (8 + 4)) {
                        diag.emplace_back(DiagLevel::Warning, "Data atom stores integer of invalid size. Trying to read data anyways.", context);
                    }
                    if (dataAtom->dataSize() >= (8 + 4)) {
                        number = static_cast<int>(reader.readUInt32BE());
                    } else if (dataAtom->dataSize() == (8 + 2)) {
                        number = static_cast<int>(reader.readUInt16BE());
                    } else if (dataAtom->dataSize() == (8 + 1)) {
                        number = static_cast<int>(reader.readByte());
                    }
                    switch (ilstChild.id()) {
                    case PreDefinedGenre: // consider number as standard genre index
                        val->assignStandardGenreIndex(number - 1);
                        break;
                    default:
                        val->assignInteger(number);
                    }
                    break;
                }
                default:
                    switch (ilstChild.id()) {
                    // track number, disk number and genre have no specific data type id
                    case TrackPosition:
                    case DiskPosition: {
                        if (dataAtom->dataSize() < (8 + 6)) {
                            diag.emplace_back(DiagLevel::Warning, "Track/disk position is truncated. Trying to read data anyways.", context);
                        }
                        std::uint16_t pos = 0, total = 0;
                        if (dataAtom->dataSize() >= (8 + 4)) {
                            stream.seekg(2, ios_base::cur);
                            pos = reader.readUInt16BE();
                        }
                        if (dataAtom->dataSize() >= (8 + 6)) {
                            total = reader.readUInt16BE();
                        }
                        val->assignPosition(PositionInSet(pos, total));
                        break;
                    }
                    case PreDefinedGenre:
                        if (dataAtom->dataSize() < (8 + 2)) {
                            diag.emplace_back(DiagLevel::Warning, "Genre index is truncated.", context);
                        } else {
                            val->assignStandardGenreIndex(reader.readUInt16BE() - 1);
                        }
                        break;
                    default: // no supported data type, read raw data
                        const auto dataSize = static_cast<streamsize>(dataAtom->dataSize() - 8);
                        auto data = make_unique<char[]>(static_cast<size_t>(dataSize));
                        stream.read(data.get(), dataSize);
                        if (ilstChild.id() == Mp4TagAtomIds::Cover) {
                            val->assignData(std::move(data), static_cast<size_t>(dataSize), TagDataType::Picture);
                        } else {
                            val->assignData(std::move(data), static_cast<size_t>(dataSize), TagDataType::Undefined);
                        }
                    }
                }
            } else if (dataAtom->id() == Mp4AtomIds::Mean) {
                if (dataAtom->dataSize() < 8) {
                    diag.emplace_back(DiagLevel::Warning,
                        "Truncated child atom \"mean\" in tag atom (ilst child) found. It will be ignored and discarded when applying changes.",
                        context);
                    continue;
                }
                if (++meanAtomFound > 1) {
                    if (meanAtomFound == 2) {
                        diag.emplace_back(DiagLevel::Warning,
                            "Tag atom contains more than one mean atom. The additional mean atoms will be ignored and discarded when applying "
                            "changes.",
                            context);
                    }
                    continue;
                }
                stream.seekg(static_cast<streamoff>(dataAtom->dataOffset() + 4));
                m_mean = reader.readString(dataAtom->dataSize() - 4);
            } else if (dataAtom->id() == Mp4AtomIds::Name) {
                if (dataAtom->dataSize() < 4) {
                    diag.emplace_back(DiagLevel::Warning,
                        "Truncated child atom \"name\" in tag atom (ilst child) found. It will be ignored and discarded when applying changes.",
                        context);
                    continue;
                }
                if (++nameAtomFound > 1) {
                    if (nameAtomFound == 2) {
                        diag.emplace_back(DiagLevel::Warning,
                            "Tag atom contains more than one name atom. The addiational name atoms will be ignored and discarded when applying "
                            "changes.",
                            context);
                    }
                    continue;
                }
                stream.seekg(static_cast<streamoff>(dataAtom->dataOffset() + 4));
                m_name = reader.readString(dataAtom->dataSize() - 4);
            } else {
                diag.emplace_back(DiagLevel::Warning,
                    "Unknown child atom \"" % dataAtom->idToString()
                        + "\" in tag atom (ilst child) found. It will be ignored and discarded when applying changes.",
                    context);
            }
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Warning,
                "Unable to parse all children atom in tag atom (ilst child) found. Invalid children will be ignored and discarded when applying "
                "changes.",
                context);
        }
    }
    if (value().isEmpty()) {
        diag.emplace_back(DiagLevel::Warning, "The field value is empty.", context);
    }
}

/*!
 * \brief Prepares making.
 * \returns Returns a Mp4TagFieldMaker object which can be used to actually make the field.
 * \remarks The field must NOT be mutated after making is prepared when it is intended to actually
 *          make the field using the make method of the returned object.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 *
 * This method might be useful when it is necessary to know the size of the field before making it.
 */
Mp4TagFieldMaker Mp4TagField::prepareMaking(Diagnostics &diag)
{
    return Mp4TagFieldMaker(*this, diag);
}

/*!
 * \brief Saves the field to the specified \a stream.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 */
void Mp4TagField::make(ostream &stream, Diagnostics &diag)
{
    prepareMaking(diag).make(stream);
}

/*!
 * \brief Returns the expected raw data types for the ID of the field.
 */
std::vector<std::uint32_t> Mp4TagField::expectedRawDataTypes() const
{
    using namespace Mp4TagAtomIds;
    std::vector<std::uint32_t> res;
    switch (id()) {
    case Album:
    case Artist:
    case Comment:
    case Year:
    case Title:
    case Genre:
    case Composer:
    case Encoder:
    case Grouping:
    case Description:
    case Lyrics:
    case RecordLabel:
    case Performers:
    case Lyricist:
        res.push_back(RawDataType::Utf8);
        res.push_back(RawDataType::Utf16);
        break;
    case PreDefinedGenre:
    case TrackPosition:
    case DiskPosition:
        res.push_back(RawDataType::Reserved);
        break;
    case Bpm:
    case Rating: // 0 = None, 1 = Explicit, 2 = Clean
        res.push_back(RawDataType::BeSignedInt);
        res.push_back(RawDataType::BeUnsignedInt);
        break;
    case Cover:
        res.push_back(RawDataType::Gif);
        res.push_back(RawDataType::Jpeg);
        res.push_back(RawDataType::Png);
        res.push_back(RawDataType::Bmp);
        break;
    case Extended:
        if (mean() != Mp4TagExtendedMeanIds::iTunes) {
            throw Failure();
        }
        // assumption that extended "iTunes" tags always use Unicode correct?
        res.push_back(RawDataType::Utf8);
        res.push_back(RawDataType::Utf16);
        break;
    default:
        throw Failure();
    }
    return res;
}

/*!
 * \brief Returns an appropriate raw data type.
 * \return
 * Returns the type info if assigned; otherwise returns a raw data type considered as appropriate for the
 * ID of the field and its value.
 * \sa See Mp4TagField::appropriateRawDataTypeForValue() for the behavior if no type info is assigned.
 */
std::uint32_t Mp4TagField::appropriateRawDataType() const
{
    if (isTypeInfoAssigned()) {
        // obtain raw data type from tag field if present
        return typeInfo();
    }

    // there is no raw data type assigned (tag field was not present in original file and
    // has been inserted by the library's user without type)
    // -> try to derive appropriate raw data type from atom ID
    return appropriateRawDataTypeForValue(value());
}

/*!
 * \brief Returns an appropriate raw data type.
 * \returns
 * Returns a raw data type considered as appropriate for the ID of the field and the specified \a value.
 * \throws
 * Throws TagParser::Failure if an appropriate raw data type can not be determined. It is possible to determine
 * the raw data type for all supported tag field IDs (those where a conversion to KnownField via Mp4Tag exists).
 */
std::uint32_t Mp4TagField::appropriateRawDataTypeForValue(const TagValue &value) const
{
    using namespace Mp4TagAtomIds;
    switch (id()) {
    case Album:
    case Artist:
    case Comment:
    case Year:
    case Title:
    case Genre:
    case Composer:
    case Encoder:
    case Grouping:
    case Description:
    case Lyrics:
    case RecordLabel:
    case Performers:
    case Lyricist:
    case AlbumArtist:
        switch (value.dataEncoding()) {
        case TagTextEncoding::Utf8:
            return RawDataType::Utf8;
        case TagTextEncoding::Utf16BigEndian:
            return RawDataType::Utf16;
        default:;
        }
        break;
    case TrackPosition:
    case DiskPosition:
        return RawDataType::Reserved;
    case PreDefinedGenre:
    case Bpm:
    case Rating:
        return RawDataType::BeSignedInt;
    case Cover: {
        const string &mimeType = value.mimeType();
        if (mimeType == "image/jpg" || mimeType == "image/jpeg") { // "well-known" type
            return RawDataType::Jpeg;
        } else if (mimeType == "image/png") {
            return RawDataType::Png;
        } else if (mimeType == "image/bmp") {
            return RawDataType::Bmp;
        }
    } break;
    case Extended:
        if (mean() != Mp4TagExtendedMeanIds::iTunes) {
            throw Failure();
        }
        switch (value.dataEncoding()) {
        case TagTextEncoding::Utf8:
            return RawDataType::Utf8;
        case TagTextEncoding::Utf16BigEndian:
            return RawDataType::Utf16;
        default:;
        }
        break;
    default:;
    }

    // do not forget to extend Mp4Tag::internallyGetFieldId() and Mp4Tag::internallyGetKnownField() as well

    throw Failure();
}

/*!
 * \brief Clears MP4-specific values. Called via clear() and clearValue().
 */
void Mp4TagField::internallyClearValue()
{
    value().clearDataAndMetadata();
    m_additionalData.clear();
    m_countryIndicator = 0;
    m_langIndicator = 0;
}

/*!
 * \brief Clears MP4-specific values. Called via clear() and clearValue().
 */
void Mp4TagField::internallyClearFurtherData()
{
    m_name.clear();
    m_mean.clear();
    m_parsedRawDataType = RawDataType::Reserved;
}

/// \cond
Mp4TagFieldMaker::Data::Data()
    : convertedData(stringstream::in | stringstream::out | stringstream::binary)
{
}
/// \endcond

/*!
 * \class TagParser::Mp4TagFieldMaker
 * \brief The Mp4TagFieldMaker class helps making tag fields.
 *        It allows to calculate the required size.
 * \sa See Mp4TagFieldMaker::prepareMaking() for more information.
 */

/*!
 * \brief Prepares making the specified \a field.
 * \sa See Mp4TagField::prepareMaking() for more information.
 */
Mp4TagFieldMaker::Mp4TagFieldMaker(Mp4TagField &field, Diagnostics &diag)
    : m_field(field)
    , m_writer(nullptr)
    , m_totalSize(0)
{
    if (!m_field.id()) {
        diag.emplace_back(DiagLevel::Warning, "Invalid tag atom ID.", "making MP4 tag field");
        throw InvalidDataException();
    }
    const string context("making MP4 tag field " + Mp4TagField::fieldIdToString(m_field.id()));
    if (m_field.value().isEmpty() && (!m_field.mean().empty() || !m_field.name().empty())) {
        diag.emplace_back(DiagLevel::Critical, "No tag value assigned.", context);
        throw InvalidDataException();
    }

    // calculate size for name and mean
    m_totalSize = 8 + (m_field.name().empty() ? 0 : (12 + m_field.name().size())) + (m_field.mean().empty() ? 0 : (12 + m_field.mean().size()));

    // prepare making data atom and calculate the expected size
    m_totalSize += prepareDataAtom(field.value(), field.countryIndicator(), field.languageIndicator(), context, diag);
    for (const auto &additionalData : m_field.additionalData()) {
        m_totalSize += prepareDataAtom(additionalData.value, additionalData.countryIndicator, additionalData.languageIndicator, context, diag);
    }

    if (m_totalSize > numeric_limits<std::uint32_t>::max()) {
        diag.emplace_back(DiagLevel::Critical, "Making a such big MP4 tag field is not possible.", context);
        throw NotImplementedException();
    }
}

/*!
 * \brief Prepares making a data atom for the specified \a value.
 */
std::uint64_t Mp4TagFieldMaker::prepareDataAtom(
    const TagValue &value, std::uint16_t countryIndicator, std::uint16_t languageIndicator, const std::string &context, Diagnostics &diag)
{
    // add new data entry
    auto &data = m_data.emplace_back();
    m_writer.setStream(&data.convertedData);

    // assign local info
    // FIXME: use locale within the tag value instead of just passing through current values
    data.countryIndicator = countryIndicator;
    data.languageIndicator = languageIndicator;

    try {
        // try to use appropriate raw data type
        data.rawType = m_field.isTypeInfoAssigned() ? m_field.typeInfo() : m_field.appropriateRawDataTypeForValue(value);
    } catch (const Failure &) {
        // unable to obtain appropriate raw data type
        if (m_field.id() == Mp4TagAtomIds::Cover) {
            // assume JPEG image
            data.rawType = RawDataType::Jpeg;
            diag.emplace_back(
                DiagLevel::Warning, "It was not possible to find an appropriate raw data type id. JPEG image will be assumed.", context);
        } else {
            // assume UTF-8 text
            data.rawType = RawDataType::Utf8;
            diag.emplace_back(DiagLevel::Warning, "It was not possible to find an appropriate raw data type id. UTF-8 will be assumed.", context);
        }
    }

    try {
        if (!value.isEmpty()) { // there might be only mean and name info, but no data
            data.convertedData.exceptions(std::stringstream::failbit | std::stringstream::badbit);
            switch (data.rawType) {
            case RawDataType::Utf8:
                if (value.type() != TagDataType::Text || value.dataEncoding() != TagTextEncoding::Utf8) {
                    m_writer.writeString(value.toString(TagTextEncoding::Utf8));
                }
                break;
            case RawDataType::Utf16:
                if (value.type() != TagDataType::Text || value.dataEncoding() != TagTextEncoding::Utf16LittleEndian) {
                    m_writer.writeString(value.toString(TagTextEncoding::Utf16LittleEndian));
                }
                break;
            case RawDataType::BeSignedInt: {
                int number = value.toInteger();
                if (number <= numeric_limits<std::int16_t>::max() && number >= numeric_limits<std::int16_t>::min()) {
                    m_writer.writeInt16BE(static_cast<std::int16_t>(number));
                } else {
                    m_writer.writeInt32BE(number);
                }
                break;
            }
            case RawDataType::BeUnsignedInt: {
                int number = value.toInteger();
                if (number <= numeric_limits<std::uint16_t>::max() && number >= numeric_limits<std::uint16_t>::min()) {
                    m_writer.writeUInt16BE(static_cast<std::uint16_t>(number));
                } else if (number > 0) {
                    m_writer.writeUInt32BE(static_cast<std::uint32_t>(number));
                } else {
                    throw ConversionException(
                        "Negative integer can not be assigned to the field with the ID \"" % interpretIntegerAsString<std::uint32_t>(m_field.id())
                        + "\".");
                }
                break;
            }
            case RawDataType::Bmp:
            case RawDataType::Jpeg:
            case RawDataType::Png:
                break; // leave converted data empty to write original data later
            default:
                switch (m_field.id()) {
                // track number and disk number are exceptions
                // raw data type 0 is used, information is stored as pair of unsigned integers
                case Mp4TagAtomIds::TrackPosition:
                case Mp4TagAtomIds::DiskPosition: {
                    PositionInSet pos = value.toPositionInSet();
                    m_writer.writeInt32BE(pos.position());
                    if (pos.total() <= numeric_limits<std::int16_t>::max()) {
                        m_writer.writeInt16BE(static_cast<std::int16_t>(pos.total()));
                    } else {
                        throw ConversionException(
                            "Integer can not be assigned to the field with the id \"" % interpretIntegerAsString<std::uint32_t>(m_field.id())
                            + "\" because it is to big.");
                    }
                    m_writer.writeUInt16BE(0);
                    break;
                }
                case Mp4TagAtomIds::PreDefinedGenre:
                    m_writer.writeUInt16BE(static_cast<std::uint16_t>(value.toStandardGenreIndex()));
                    break;
                default:; // leave converted data empty to write original data later
                }
            }
        }
    } catch (const ConversionException &e) {
        // it was not possible to perform required conversions
        if (char_traits<char>::length(e.what())) {
            diag.emplace_back(DiagLevel::Critical, e.what(), context);
        } else {
            diag.emplace_back(DiagLevel::Critical, "The assigned tag value can not be converted to be written appropriately.", context);
        }
        throw InvalidDataException();
    }

    // calculate data size; assign raw data
    if (value.isEmpty()) {
        return data.size = 0;
    } else if (data.convertedData.tellp()) {
        data.size = static_cast<std::size_t>(data.convertedData.tellp());
    } else {
        data.rawData = std::string_view(value.dataPointer(), data.size = value.dataSize());
    }
    return data.size += 16;
}

/*!
 * \brief Saves the field (specified when constructing the object) to the
 *        specified \a stream.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw TagParser::Failure or a derived exception.
 */
void Mp4TagFieldMaker::make(ostream &stream)
{
    m_writer.setStream(&stream);
    // size of entire tag atom
    m_writer.writeUInt32BE(static_cast<std::uint32_t>(m_totalSize));
    // id of tag atom
    m_writer.writeUInt32BE(m_field.id());
    // write "mean" atom
    if (!m_field.mean().empty()) {
        m_writer.writeUInt32BE(static_cast<std::uint32_t>(12 + m_field.mean().size()));
        m_writer.writeUInt32BE(Mp4AtomIds::Mean);
        m_writer.writeUInt32BE(0);
        m_writer.writeString(m_field.mean());
    }
    // write "name" atom
    if (!m_field.name().empty()) {
        m_writer.writeUInt32BE(static_cast<std::uint32_t>(12 + m_field.name().length()));
        m_writer.writeUInt32BE(Mp4AtomIds::Name);
        m_writer.writeUInt32BE(0);
        m_writer.writeString(m_field.name());
    }
    // write "data" atoms
    for (auto &data : m_data) {
        if (!data.size) {
            continue;
        }
        m_writer.writeUInt32BE(static_cast<std::uint32_t>(data.size)); // size of data atom
        m_writer.writeUInt32BE(Mp4AtomIds::Data); // id of data atom
        m_writer.writeByte(0); // version
        m_writer.writeUInt24BE(data.rawType);
        m_writer.writeUInt16BE(data.countryIndicator);
        m_writer.writeUInt16BE(data.languageIndicator);
        if (data.convertedData.tellp()) {
            // write converted data
            stream << data.convertedData.rdbuf();
        } else {
            // no conversion was needed, write data directly from tag value
            stream.write(data.rawData.data(), static_cast<std::streamoff>(data.rawData.size()));
        }
    }
}

} // namespace TagParser
