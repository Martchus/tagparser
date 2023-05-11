#ifndef TAG_PARSER_ABSTRACTATTACHMENT_H
#define TAG_PARSER_ABSTRACTATTACHMENT_H

#include "./diagnostics.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace TagParser {

class AbortableProgressFeedback;
class MediaFileInfo;

class TAG_PARSER_EXPORT StreamDataBlock {
public:
    StreamDataBlock(const std::function<std::istream &()> &stream, uint64_t startOffset = 0, std::ios_base::seekdir startDir = std::ios_base::beg,
        uint64_t endOffset = 0, std::ios_base::seekdir endDir = std::ios_base::end);
    virtual ~StreamDataBlock();

    std::istream &stream() const;
    std::uint64_t startOffset() const;
    std::uint64_t endOffset() const;
    std::uint64_t size() const;
    const std::unique_ptr<char[]> &buffer() const;
    void makeBuffer() const;
    void discardBuffer();
    void copyTo(std::ostream &stream) const;

protected:
    StreamDataBlock();

    std::function<std::istream &()> m_stream;
    std::uint64_t m_startOffset;
    std::uint64_t m_endOffset;
    mutable std::unique_ptr<char[]> m_buffer;
};

/*!
 * \brief Returns the associated stream.
 *
 * Calling this method on invalid objects causes undefined behaviour. See isValid().
 */
inline std::istream &StreamDataBlock::stream() const
{
    return m_stream();
}

/*!
 * \brief Returns the absolute start offset of the data block in the stream.
 */
inline std::uint64_t StreamDataBlock::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the absolute end offset of the data block in the stream.
 */
inline std::uint64_t StreamDataBlock::endOffset() const
{
    return m_endOffset;
}

/*!
 * \brief Returns the size of the data block.
 */
inline std::uint64_t StreamDataBlock::size() const
{
    return m_endOffset - m_startOffset;
}

/*!
 * \brief Returns the data buffered via makeBuffer().
 */
inline const std::unique_ptr<char[]> &StreamDataBlock::buffer() const
{
    return m_buffer;
}

/*!
 * \brief Discards buffered data.
 */
inline void StreamDataBlock::discardBuffer()
{
    m_buffer.reset();
}

class TAG_PARSER_EXPORT FileDataBlock : public StreamDataBlock {
public:
    FileDataBlock(std::string_view path, Diagnostics &diag, AbortableProgressFeedback &progress);
    ~FileDataBlock();
    const MediaFileInfo *fileInfo() const;

private:
    std::unique_ptr<MediaFileInfo> m_fileInfo;
};

inline const MediaFileInfo *FileDataBlock::fileInfo() const
{
    return m_fileInfo.get();
}

struct AbstractAttachmentPrivate;

class TAG_PARSER_EXPORT AbstractAttachment {
public:
    const std::string &description() const;
    void setDescription(std::string_view description);
    const std::string &name() const;
    void setName(std::string_view name);
    const std::string &mimeType() const;
    void setMimeType(std::string_view mimeType);
    std::uint64_t id() const;
    void setId(std::uint64_t id);
    const StreamDataBlock *data() const;
    void setData(std::unique_ptr<StreamDataBlock> &&data);
    void setFile(std::string_view path, Diagnostics &diag, AbortableProgressFeedback &progress);
    bool isDataFromFile() const;
    std::string label() const;
    void clear();
    bool isIgnored() const;
    void setIgnored(bool ignored);
    bool isEmpty() const;

protected:
    explicit AbstractAttachment();
    virtual ~AbstractAttachment();

private:
    std::string m_description;
    std::string m_name;
    std::string m_mimeType;
    std::uint64_t m_id;
    std::unique_ptr<StreamDataBlock> m_data;
    std::unique_ptr<AbstractAttachmentPrivate> m_p;
    bool m_isDataFromFile;
    bool m_ignored;
};

/*!
 * \brief Returns a description of the attachment.
 */
inline const std::string &AbstractAttachment::description() const
{
    return m_description;
}

/*!
 * \brief Sets a description of the attachment.
 */
inline void AbstractAttachment::setDescription(std::string_view description)
{
    m_description = description;
}

/*!
 * \brief Returns the (file) name of the attachment.
 */
inline const std::string &AbstractAttachment::name() const
{
    return m_name;
}

/*!
 * \brief Sets the (file) name of the attachment.
 */
inline void AbstractAttachment::setName(std::string_view name)
{
    m_name = name;
}

/*!
 * \brief Returns the MIME-type of the attachment.
 */
inline const std::string &AbstractAttachment::mimeType() const
{
    return m_mimeType;
}

/*!
 * \brief Sets the MIME-type of the attachment.
 */
inline void AbstractAttachment::setMimeType(std::string_view mimeType)
{
    m_mimeType = mimeType;
}

/*!
 * \brief Returns the ID of the attachment.
 */
inline std::uint64_t AbstractAttachment::id() const
{
    return m_id;
}

/*!
 * \brief Sets the ID of the attachment.
 */
inline void AbstractAttachment::setId(uint64_t id)
{
    m_id = id;
}

/*!
 * \brief Returns a reference to the data of the attachment.
 * \remarks
 * - The reference might be nullptr if there is no data assigned.
 * - The attachment keeps ownership over the reference.
 * \sa setData(), setFile()
 */
inline const StreamDataBlock *AbstractAttachment::data() const
{
    return m_data.get();
}

/*!
 * \brief Sets the \a data for the attachment.
 * \remarks The specified \a data is moved to the attachment.
 * \sa data(), setFile()
 */
inline void AbstractAttachment::setData(std::unique_ptr<StreamDataBlock> &&data)
{
    m_data = std::move(data);
    m_isDataFromFile = false;
}

/*!
 * \brief Returns whether the assigned data has been assigned using the setFile() method.
 */
inline bool AbstractAttachment::isDataFromFile() const
{
    return m_isDataFromFile;
}

/*!
 * \brief Returns whether the attachment is ignored/omitted when rewriting the container.
 *
 * The default value is false.
 * \sa setIgnored()
 */
inline bool AbstractAttachment::isIgnored() const
{
    return m_ignored;
}

/*!
 * \brief Sets whether the attachment is ignored/omitted when rewriting the container.
 *
 * \sa isIgnored()
 */
inline void AbstractAttachment::setIgnored(bool ignored)
{
    m_ignored = ignored;
}

/*!
 * \brief Returns whether the attachment is empty (no data and no meta-data assigned).
 * \remarks Does not take into account whether an ID is set.
 */
inline bool AbstractAttachment::isEmpty() const
{
    return m_description.empty() && m_name.empty() && !m_mimeType.empty() && !m_data;
}

} // namespace TagParser

#endif // TAG_PARSER_ABSTRACTATTACHMENT_H
