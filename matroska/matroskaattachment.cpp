#include "./matroskaattachment.h"
#include "./matroskacontainer.h"
#include "./ebmlelement.h"
#include "./matroskaid.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringbuilder.h>

#include <memory>

using namespace std;
using namespace ConversionUtilities;
using namespace IoUtilities;

namespace Media {

/*!
 * \class Media::MatroskaAttachment
 * \brief Implementation of Media::AbstractAttachment for the Matroska container.
 */

/*!
 * \brief Parses attachment from the specified \a attachedFileElement.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaAttachment::parse(EbmlElement *attachedFileElement)
{
    clear();
    static const string context("parsing \"AttachedFile\"-element");
    m_attachedFileElement = attachedFileElement;
    EbmlElement *subElement = attachedFileElement->firstChild();
    while(subElement) {
        subElement->parse();
        switch(subElement->id()) {
        case MatroskaIds::FileDescription:
            if(description().empty()) {
                setDescription(subElement->readString());
            } else {
                addNotification(NotificationType::Warning, "Multiple \"FileDescription\"-elements found. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::FileName:
            if(name().empty()) {
                setName(subElement->readString());
            } else {
                addNotification(NotificationType::Warning, "Multiple \"FileName\"-elements found. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::FileMimeType:
            if(mimeType().empty()) {
                setMimeType(subElement->readString());
            } else {
                addNotification(NotificationType::Warning, "Multiple \"FileMimeType\"-elements found. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::FileData:
            if(data()) {
                addNotification(NotificationType::Warning, "Multiple \"FileData\"-elements found. Surplus elements will be ignored.", context);
            } else {
                setData(make_unique<StreamDataBlock>(std::bind(&EbmlElement::stream, subElement), subElement->dataOffset(), ios_base::beg, subElement->startOffset() + subElement->totalSize(), ios_base::beg));
            }
            break;
        case MatroskaIds::FileUID:
            if(id()) {
                addNotification(NotificationType::Warning, "Multiple \"FileUID\"-elements found. Surplus elements will be ignored.", context);
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
            addNotification(NotificationType::Warning, "Unknown child element \"" % subElement->idToString() + "\" found.", context);
        }
        subElement = subElement->nextSibling();
    }
}

/*!
 * \brief Writes the attachment to the specified \a stream (makes an "AttachedFile"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 * \sa prepareMaking()
 */
void MatroskaAttachment::make(std::ostream &stream)
{
    if(!data() || !data()->size()) {
        addNotification(NotificationType::Critical, "There is no data assigned.", "making Matroska attachment");
        throw InvalidDataException();
    }
    prepareMaking().make(stream);
}

/*!
 * \class Media::MatroskaAttachmentMaker
 * \brief The MatroskaAttachmentMaker class helps writing Matroska "AttachedFile"-elements which contain an attachment.
 *
 * An instance can be obtained using the MatroskaAttachment::prepareMaking() method.
 */

/*!
 * \brief Prepares making the specified \a attachment.
 * \sa See MatroskaAttachment::prepareMaking() for more information.
 */
MatroskaAttachmentMaker::MatroskaAttachmentMaker(MatroskaAttachment &attachment) :
    m_attachment(attachment)
{
    m_attachedFileElementSize = 2 + EbmlElement::calculateSizeDenotationLength(attachment.name().size()) + attachment.name().size()
            + 2 + EbmlElement::calculateSizeDenotationLength(attachment.mimeType().size()) + attachment.mimeType().size()
            + 2 + 1 + EbmlElement::calculateUIntegerLength(attachment.id());
    if(auto dataSize = attachment.data() ? attachment.data()->size() : static_cast<istream::pos_type>(0)) {
        m_attachedFileElementSize += 2 + EbmlElement::calculateSizeDenotationLength(dataSize) + dataSize;
    }
    if(!attachment.description().empty()) {
        m_attachedFileElementSize += 2 + EbmlElement::calculateSizeDenotationLength(attachment.description().size()) + attachment.description().size();
    }
    if(attachment.attachedFileElement()) {
        EbmlElement *child;
        for(auto id : initializer_list<EbmlElement::IdentifierType>{MatroskaIds::FileReferral, MatroskaIds::FileUsedStartTime, MatroskaIds::FileUsedEndTime}) {
            if((child = attachment.attachedFileElement()->childById(id))) {
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
 *                throw Media::Failure or a derived exception.
 */
void MatroskaAttachmentMaker::make(ostream &stream) const
{
    char buff[8];
    BE::getBytes(static_cast<uint16>(MatroskaIds::AttachedFile), buff);
    stream.write(buff, 2);
    byte len = EbmlElement::makeSizeDenotation(m_attachedFileElementSize, buff);
    stream.write(buff, len);
    // make elements
    EbmlElement::makeSimpleElement(stream, MatroskaIds::FileName, attachment().name());
    if(!attachment().description().empty())  {
        EbmlElement::makeSimpleElement(stream, MatroskaIds::FileDescription, attachment().description());
    }
    EbmlElement::makeSimpleElement(stream, MatroskaIds::FileMimeType, attachment().mimeType());
    EbmlElement::makeSimpleElement(stream, MatroskaIds::FileUID, attachment().id());
    if(attachment().attachedFileElement()) {
        EbmlElement *child;
        for(auto id : initializer_list<EbmlElement::IdentifierType>{MatroskaIds::FileReferral, MatroskaIds::FileUsedStartTime, MatroskaIds::FileUsedEndTime}) {
            if((child = attachment().attachedFileElement()->childById(id))) {
                if(child->buffer()) {
                    child->copyBuffer(stream);
                } else {
                    child->copyEntirely(stream);
                }
            }
        }
    }
    if(attachment().data() && attachment().data()->size()) {
        BE::getBytes(static_cast<uint16>(MatroskaIds::FileData), buff);
        stream.write(buff, 2);
        len = EbmlElement::makeSizeDenotation(attachment().data()->size(), buff);
        stream.write(buff, len);
        attachment().data()->copyTo(stream);
    }
}

void MatroskaAttachmentMaker::bufferCurrentAttachments()
{
    EbmlElement *child;
    if(attachment().attachedFileElement()) {
        for(auto id : initializer_list<EbmlElement::IdentifierType>{MatroskaIds::FileReferral, MatroskaIds::FileUsedStartTime, MatroskaIds::FileUsedEndTime}) {
            if((child = attachment().attachedFileElement()->childById(id))) {
                child->makeBuffer();
            }
        }
    }
    if(attachment().data() && attachment().data()->size() && !attachment().isDataFromFile()) {
        attachment().data()->makeBuffer();
    }
}

} // namespace Media

