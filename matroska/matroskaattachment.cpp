#include "./matroskaattachment.h"
#include "./ebmlelement.h"
#include "./matroskacontainer.h"
#include "./matroskaid.h"

#include "../mediafileinfo.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringbuilder.h>

#include <memory>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::MatroskaAttachment
 * \brief Implementation of TagParser::AbstractAttachment for the Matroska container.
 */

/*!
 * \brief Parses attachment from the specified \a attachedFileElement.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaAttachment::parse(EbmlElement *attachedFileElement, Diagnostics &diag)
{
    clear();
    static const string context("parsing \"AttachedFile\"-element");
    m_attachedFileElement = attachedFileElement;
    EbmlElement *subElement = attachedFileElement->firstChild();
    while (subElement) {
        subElement->parse(diag);
        switch (subElement->id()) {
        case MatroskaIds::FileDescription:
            if (description().empty()) {
                setDescription(subElement->readString());
            } else {
                diag.emplace_back(DiagLevel::Warning, "Multiple \"FileDescription\"-elements found. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::FileName:
            if (name().empty()) {
                setName(subElement->readString());
            } else {
                diag.emplace_back(DiagLevel::Warning, "Multiple \"FileName\"-elements found. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::FileMimeType:
            if (mimeType().empty()) {
                setMimeType(subElement->readString());
            } else {
                diag.emplace_back(DiagLevel::Warning, "Multiple \"FileMimeType\"-elements found. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::FileData:
            if (data()) {
                diag.emplace_back(DiagLevel::Warning, "Multiple \"FileData\"-elements found. Surplus elements will be ignored.", context);
            } else {
                setData(make_unique<StreamDataBlock>(std::bind(&EbmlElement::stream, subElement), subElement->dataOffset(), ios_base::beg,
                    subElement->startOffset() + subElement->totalSize(), ios_base::beg));
            }
            break;
        case MatroskaIds::FileUID:
            if (id()) {
                diag.emplace_back(DiagLevel::Warning, "Multiple \"FileUID\"-elements found. Surplus elements will be ignored.", context);
            } else {
                setId(subElement->readUInteger());
            }
            break;
        case MatroskaIds::FileReferral:
        case MatroskaIds::FileUsedStartTime:
        case MatroskaIds::FileUsedEndTime:
        case EbmlIds::Crc32:
        case EbmlIds::Void:
            break;
        default:
            diag.emplace_back(DiagLevel::Warning, "Unknown child element \"" % subElement->idToString() + "\" found.", context);
        }
        subElement = subElement->nextSibling();
    }
}

/*!
 * \brief Writes the attachment to the specified \a stream (makes an "AttachedFile"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 * \sa prepareMaking()
 */
void MatroskaAttachment::make(std::ostream &stream, Diagnostics &diag)
{
    if (!data() || !data()->size()) {
        diag.emplace_back(DiagLevel::Critical, "There is no data assigned.", "making Matroska attachment");
        throw InvalidDataException();
    }
    prepareMaking(diag).make(stream, diag);
}

/*!
 * \class TagParser::MatroskaAttachmentMaker
 * \brief The MatroskaAttachmentMaker class helps writing Matroska "AttachedFile"-elements which contain an attachment.
 *
 * An instance can be obtained using the MatroskaAttachment::prepareMaking() method.
 */

/*!
 * \brief Prepares making the specified \a attachment.
 * \sa See MatroskaAttachment::prepareMaking() for more information.
 */
MatroskaAttachmentMaker::MatroskaAttachmentMaker(MatroskaAttachment &attachment, Diagnostics &diag)
    : m_attachment(attachment)
{
    m_attachedFileElementSize = 2 + EbmlElement::calculateSizeDenotationLength(attachment.name().size()) + attachment.name().size() + 2
        + EbmlElement::calculateSizeDenotationLength(attachment.mimeType().size()) + attachment.mimeType().size() + 2 + 1
        + EbmlElement::calculateUIntegerLength(attachment.id());
    if (auto dataSize = attachment.data() ? attachment.data()->size() : static_cast<std::uint64_t>(0)) {
        m_attachedFileElementSize += 2 + EbmlElement::calculateSizeDenotationLength(dataSize) + dataSize;
    }
    if (!attachment.description().empty()) {
        m_attachedFileElementSize
            += 2 + EbmlElement::calculateSizeDenotationLength(attachment.description().size()) + attachment.description().size();
    }
    if (attachment.attachedFileElement()) {
        EbmlElement *child;
        for (auto id : initializer_list<EbmlElement::IdentifierType>{
                 MatroskaIds::FileReferral, MatroskaIds::FileUsedStartTime, MatroskaIds::FileUsedEndTime }) {
            if ((child = attachment.attachedFileElement()->childById(id, diag))) {
                m_attachedFileElementSize += child->totalSize();
            }
        }
    }
    m_totalSize = 2 + EbmlElement::calculateSizeDenotationLength(m_attachedFileElementSize) + m_attachedFileElementSize;
}

/*!
 * \brief Saves the attachment (specified when constructing the object) to the
 *        specified \a stream (makes an "AttachedFile"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw TagParser::Failure or a derived exception.
 */
void MatroskaAttachmentMaker::make(ostream &stream, Diagnostics &diag) const
{
    char buff[8];
    BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::AttachedFile), buff);
    stream.write(buff, 2);
    std::uint8_t len = EbmlElement::makeSizeDenotation(m_attachedFileElementSize, buff);
    stream.write(buff, len);
    // make elements
    EbmlElement::makeSimpleElement(stream, MatroskaIds::FileName, attachment().name());
    if (!attachment().description().empty()) {
        EbmlElement::makeSimpleElement(stream, MatroskaIds::FileDescription, attachment().description());
    }
    EbmlElement::makeSimpleElement(stream, MatroskaIds::FileMimeType, attachment().mimeType());
    EbmlElement::makeSimpleElement(stream, MatroskaIds::FileUID, attachment().id());
    if (attachment().attachedFileElement()) {
        EbmlElement *child;
        for (auto id : initializer_list<EbmlElement::IdentifierType>{
                 MatroskaIds::FileReferral, MatroskaIds::FileUsedStartTime, MatroskaIds::FileUsedEndTime }) {
            if ((child = attachment().attachedFileElement()->childById(id, diag))) {
                if (child->buffer()) {
                    child->copyBuffer(stream);
                } else {
                    child->copyEntirely(stream, diag, nullptr);
                }
            }
        }
    }
    if (attachment().data() && attachment().data()->size()) {
        BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::FileData), buff);
        stream.write(buff, 2);
        len = EbmlElement::makeSizeDenotation(static_cast<std::uint64_t>(attachment().data()->size()), buff);
        stream.write(buff, len);
        attachment().data()->copyTo(stream);
    }
}

void MatroskaAttachmentMaker::bufferCurrentAttachments(Diagnostics &diag)
{
    EbmlElement *child;
    if (attachment().attachedFileElement()) {
        for (auto id : initializer_list<EbmlElement::IdentifierType>{
                 MatroskaIds::FileReferral, MatroskaIds::FileUsedStartTime, MatroskaIds::FileUsedEndTime }) {
            if ((child = attachment().attachedFileElement()->childById(id, diag))) {
                child->makeBuffer();
            }
        }
    }
    if (attachment().data() && attachment().data()->size() && !attachment().isDataFromFile()) {
        attachment().data()->makeBuffer();
    }
}

} // namespace TagParser
