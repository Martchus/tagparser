#include "./matroskatagfield.h"
#include "./ebmlelement.h"
#include "./matroskacontainer.h"

#include "../exceptions.h"

#include <c++utilities/io/binarywriter.h>

#include <memory>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::MatroskaTagField
 * \brief The MatroskaTagField class is used by MatroskaTag to store the fields.
 */

/*!
 * \brief Constructs a new MatroskaTagField.
 */
MatroskaTagField::MatroskaTagField()
{}

/*!
 * \brief Constructs a new MatroskaTagField with the specified \a id and \a value.
 */
MatroskaTagField::MatroskaTagField(const string &id, const TagValue &value) :
    TagField<MatroskaTagField>(id, value)
{}

/*!
 * \brief Parses field information from the specified EbmlElement.
 *
 * The specified atom should be a simple tag element. These elements
 * represents the fields of a MatroskaTag.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaTagField::reparse(EbmlElement &simpleTagElement, bool parseNestedFields)
{
    string context("parsing Matroska tag field");
    clear();
    simpleTagElement.parse();
    bool tagDefaultFound = false;
    for(EbmlElement *child = simpleTagElement.firstChild(); child; child = child = child->nextSibling()) {
        try {
            child->parse();
        } catch (const Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse children of \"SimpleTag\"-element.", context);
            break;
        }
        switch(child->id()) {
        case MatroskaIds::TagName:
            if(id().empty()) {
                setId(child->readString());
                context = "parsing Matroska tag field " + id();
            } else {
                addNotification(NotificationType::Warning, "\"SimpleTag\"-element contains multiple \"TagName\"-elements. Surplus TagName elements will be ignored.", context);
            }
            break;
        case MatroskaIds::TagString:
        case MatroskaIds::TagBinary:
            if(value().isEmpty()) {
                unique_ptr<char[]> buffer = make_unique<char []>(child->dataSize());
                child->stream().seekg(child->dataOffset());
                child->stream().read(buffer.get(), child->dataSize());
                switch(child->id()) {
                case MatroskaIds::TagString:
                    value().assignData(move(buffer), child->dataSize(), TagDataType::Text, TagTextEncoding::Utf8);
                    break;
                case MatroskaIds::TagBinary:
                    value().assignData(move(buffer), child->dataSize(), TagDataType::Undefined);
                    break;
                }
            } else {
                addNotification(NotificationType::Warning, "\"SimpleTag\"-element contains multiple \"TagString\"/\"TagBinary\"-elements. Surplus \"TagName\"/\"TagBinary\"-elements will be ignored.", context);
            }
            break;
        case MatroskaIds::TagLanguage:
            if(value().language().empty() || value().language() == "und") {
                string lng = child->readString();
                if(lng != "und") {
                    value().setLanguage(lng);
                }
            } else {
                addNotification(NotificationType::Warning, "\"SimpleTag\"-element contains multiple \"TagLanguage\"-elements. Surplus \"TagLanguage\"-elements will be ignored.", context);
            }
            break;
        case MatroskaIds::TagDefault:
            if(!tagDefaultFound) {
                setDefault(child->readUInteger() > 0);
                tagDefaultFound = true;
            } else {
                addNotification(NotificationType::Warning, "\"SimpleTag\"-element contains multiple \"TagDefault\" elements. Surplus \"TagDefault\"-elements will be ignored.", context);
            }
            break;
        case MatroskaIds::SimpleTag:
            if(parseNestedFields) {
                nestedFields().emplace_back();
                nestedFields().back().reparse(*child, true);
            } else {
                addNotification(NotificationType::Warning, "Nested fields are currently not supported. Nested tags can not be displayed and will be discarded when rewriting the file.", context);
            }
            break;
        case EbmlIds::Crc32:
        case EbmlIds::Void:
            break;
        default:
            addNotification(NotificationType::Warning, argsToString("\"SimpleTag\"-element contains unknown element ", child->idToString(), " at ", child->startOffset(), ". It will be ignored."), context);
        }
    }
}

/*!
 * \brief Prepares making.
 * \returns Returns a MatroskaTagFieldMaker object which can be used to actually make the field.
 * \remarks The field must NOT be mutated after making is prepared when it is intended to actually
 *          make the field using the make method of the returned object.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 *
 * This method might be useful when it is necessary to know the size of the field before making it.
 */
MatroskaTagFieldMaker MatroskaTagField::prepareMaking()
{
    static const string context("making Matroska \"SimpleTag\" element.");
    // check whether ID is empty
    if(id().empty()) {
        addNotification(NotificationType::Critical, "Can not make \"SimpleTag\" element with empty \"TagName\".", context);
        throw InvalidDataException();
    }
    try {
        return MatroskaTagFieldMaker(*this);
    } catch(const ConversionException &) {
        addNotification(NotificationType::Critical, "The assigned tag value can not be converted to be written appropriately.", context);
        throw InvalidDataException();
    }
}

/*!
 * \brief Saves the field to the specified \a stream (makes a "SimpleTag" element). *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void MatroskaTagField::make(ostream &stream)
{
    prepareMaking().make(stream);
}

/*!
 * \class Media::MatroskaTagFieldMaker
 * \brief The MatroskaTagFieldMaker class helps making tag fields.
 *        It allows to calculate the required size.
 * \sa See MatroskaTagField::prepareMaking() for more information.
 */

/*!
 * \brief Prepares making the specified \a field.
 * \sa See MatroskaTagField::prepareMaking() for more information.
 */
MatroskaTagFieldMaker::MatroskaTagFieldMaker(MatroskaTagField &field) :
    m_field(field),
    m_isBinary(false)
{
    try {
        m_stringValue = m_field.value().toString();
    } catch(const ConversionException &) {
        m_field.addNotification(NotificationType::Warning, "The assigned tag value can not be converted to a string and is treated as binary value (which is likely not what you want since official Matroska specifiecation doesn't list any binary fields).", "making Matroska \"SimpleTag\" element.");
        m_isBinary = true;
    }
    size_t languageSize = m_field.value().language().size();
    if(!languageSize) {
        languageSize = 3; // if there's no language set, the 3 byte long value "und" is used
    }
    m_simpleTagSize =
            // "TagName" element
            + 2 + EbmlElement::calculateSizeDenotationLength(m_field.id().size()) + m_field.id().size()
            // "TagLanguage" element
            + 2 + EbmlElement::calculateSizeDenotationLength(languageSize)  + languageSize
            // "TagDefault" element
            + 2 + 1 + 1
            // "TagString" element
            + 2 + EbmlElement::calculateSizeDenotationLength(m_stringValue.size()) + m_stringValue.size();
    // nested tags
    for(auto &nestedField : field.nestedFields()) {
        m_nestedMaker.emplace_back(nestedField.prepareMaking());
        m_simpleTagSize += m_nestedMaker.back().m_totalSize;
    }
    m_totalSize = 2 + EbmlElement::calculateSizeDenotationLength(m_simpleTagSize) + m_simpleTagSize;
}

/*!
 * \brief Saves the field (specified when constructing the object) to the
 *        specified \a stream (makes a "SimpleTag" element). *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw Media::Failure or a derived exception.
 */
void MatroskaTagFieldMaker::make(ostream &stream) const
{
    BinaryWriter writer(&stream);
    char buff[8];
    // write header of "SimpleTag" element
    writer.writeUInt16BE(MatroskaIds::SimpleTag);
    byte sizeDenotationLen = EbmlElement::makeSizeDenotation(m_simpleTagSize, buff);
    stream.write(buff, sizeDenotationLen);
    // write header of "TagName" element
    writer.writeUInt16BE(MatroskaIds::TagName);
    sizeDenotationLen = EbmlElement::makeSizeDenotation(m_field.id().size(), buff);
    stream.write(buff, sizeDenotationLen);
    stream.write(m_field.id().c_str(), m_field.id().size());
    // write header of "TagLanguage" element
    writer.writeUInt16BE(MatroskaIds::TagLanguage);
    if(m_field.value().language().empty()) {
        stream.put(static_cast<ostream::char_type>(0x80 | 3));
        stream.write("und", 3);
    } else {
        sizeDenotationLen = EbmlElement::makeSizeDenotation(m_field.value().language().size(), buff);
        stream.write(buff, sizeDenotationLen);
        stream.write(m_field.value().language().c_str(), m_field.value().language().size());
    }
    // write header of "TagDefault" element
    writer.writeUInt16BE(MatroskaIds::TagDefault);
    stream.put(static_cast<ostream::char_type>(0x80 | 1));
    stream.put(m_field.isDefault() ? 1 : 0);
    // write header of "TagString"/"TagBinary" element
    if(m_isBinary) {
        writer.writeUInt16BE(MatroskaIds::TagBinary);
        sizeDenotationLen = EbmlElement::makeSizeDenotation(m_field.value().dataSize(), buff);
        stream.write(buff, sizeDenotationLen);
        stream.write(m_field.value().dataPointer(), m_field.value().dataSize());
    } else {
        writer.writeUInt16BE(MatroskaIds::TagString);
        sizeDenotationLen = EbmlElement::makeSizeDenotation(m_stringValue.size(), buff);
        stream.write(buff, sizeDenotationLen);
        stream.write(m_stringValue.data(), m_stringValue.size());
    }
    // make nested tags
    for(const auto &maker : m_nestedMaker) {
        maker.make(stream);
    }
}

}
