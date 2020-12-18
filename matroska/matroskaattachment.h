#ifndef TAG_PARSER_MATROSKAATTACHMENT_H
#define TAG_PARSER_MATROSKAATTACHMENT_H

#include "../abstractattachment.h"

namespace TagParser {

class EbmlElement;
class MatroskaAttachment;

class TAG_PARSER_EXPORT MatroskaAttachmentMaker {
    friend class MatroskaAttachment;

public:
    void make(std::ostream &stream, Diagnostics &diag) const;
    const MatroskaAttachment &attachment() const;
    std::uint64_t requiredSize() const;
    void bufferCurrentAttachments(Diagnostics &diag);

private:
    MatroskaAttachmentMaker(MatroskaAttachment &attachment, Diagnostics &diag);

    MatroskaAttachment &m_attachment;
    std::uint64_t m_attachedFileElementSize;
    std::uint64_t m_totalSize;
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
inline std::uint64_t MatroskaAttachmentMaker::requiredSize() const
{
    return m_totalSize;
}

class TAG_PARSER_EXPORT MatroskaAttachment final : public AbstractAttachment {
public:
    MatroskaAttachment();

    void parse(EbmlElement *attachedFileElement, Diagnostics &diag);
    MatroskaAttachmentMaker prepareMaking(Diagnostics &diag);
    void make(std::ostream &stream, Diagnostics &diag);

    EbmlElement *attachedFileElement() const;

private:
    EbmlElement *m_attachedFileElement;
};

/*!
 * \brief Constructs a new Matroska attachment.
 */
inline MatroskaAttachment::MatroskaAttachment()
    : m_attachedFileElement(nullptr)
{
}

/*!
 * \brief Returns the "AttachedFile"-element which has been specified when the parse() method has been called.
 */
inline EbmlElement *MatroskaAttachment::attachedFileElement() const
{
    return m_attachedFileElement;
}

/*!
 * \brief Prepares making.
 * \returns Returns a MatroskaAttachmentMaker object which can be used to actually make the attachment.
 * \remarks The attachment must NOT be mutated after making is prepared when it is intended to actually
 *          make the attachment using the make method of the returned object.
 * \throws Throws TagParser::Failure or a derived exception when a making error occurs.
 *
 * This method might be useful when it is necessary to know the size of the attachment before making it.
 * \sa make()
 */
inline MatroskaAttachmentMaker MatroskaAttachment::prepareMaking(Diagnostics &diag)
{
    return MatroskaAttachmentMaker(*this, diag);
}

} // namespace TagParser

#endif // TAG_PARSER_MATROSKAATTACHMENT_H
