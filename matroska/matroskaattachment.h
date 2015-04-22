#ifndef MEDIA_MATROSKAATTACHMENT_H
#define MEDIA_MATROSKAATTACHMENT_H

#include "../abstractattachment.h"

namespace Media {

class EbmlElement;
class MatroskaAttachment;

class LIB_EXPORT MatroskaAttachmentMaker
{
    friend class MatroskaAttachment;

public:
    void make(std::ostream &stream) const;
    const MatroskaAttachment &attachment() const;
    uint64 requiredSize() const;

private:
    MatroskaAttachmentMaker(MatroskaAttachment &attachment);

    MatroskaAttachment &m_attachment;
    uint64 m_attachedFileElementSize;
    uint64 m_totalSize;
};

/*!
 * \brief Returns the associated attachment.
 */
inline const MatroskaAttachment &MatroskaAttachmentMaker::attachment() const
{
    return m_attachment;
}

/*!
 * \brief Returns the number of bytes which will be written when making the attachment.
 */
inline uint64 MatroskaAttachmentMaker::requiredSize() const
{
    return m_totalSize;
}

class LIB_EXPORT MatroskaAttachment : public AbstractAttachment
{
public:
    MatroskaAttachment();

    void parse(EbmlElement *attachedFileElement);
    MatroskaAttachmentMaker prepareMaking();
    void make(std::ostream &stream);

    EbmlElement *attachedFileElement() const;

private:
    EbmlElement *m_attachedFileElement;

};

/*!
 * \brief Constructs a new Matroska attachment.
 */
inline MatroskaAttachment::MatroskaAttachment() :
    m_attachedFileElement(nullptr)
{}

/*!
 * \brief Returns the "AttachedFile"-element which has been specified when the parse() method has been called.
 */
inline EbmlElement *MatroskaAttachment::attachedFileElement() const
{
    return m_attachedFileElement;
}

} // namespace Media

#endif // MEDIA_MATROSKAATTACHMENT_H
