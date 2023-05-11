#include "./abstractattachment.h"

#include "./exceptions.h"
#include "./mediafileinfo.h"
#include "./progressfeedback.h"

#include <c++utilities/io/copy.h>

#include <memory>
#include <sstream>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/// \brief The AbstractAttachmentPrivate struct contains private fields of the AbstractAttachment class.
struct AbstractAttachmentPrivate {};

/*!
 * \class TagParser::StreamDataBlock
 * \brief The StreamDataBlock class is a reference to a certain data block of a stream.
 */

/*!
 * \brief Constructs a new StreamDataBlock.
 *
 * The derived is responsible for the prober initialization of the object.
 */
StreamDataBlock::StreamDataBlock()
    : m_stream(nullptr)
    , m_startOffset(0)
    , m_endOffset(0)
{
}

/*!
 * \brief Constructs a new StreamDataBlock with the specified \a stream and offsets.
 *
 * The \a stream must be provided as function returning a reference the associated stream. This way of passing the stream
 * allows the caller to change the stream without the need to update all StreamDataBlock objects
 * referring to the stream. This is required when rewriting a file because during rewriting the original file
 * gets renamed and then reopend with another stream object.
 *
 * The object does NOT take ownership over the stream returned by the specified function.
 */
StreamDataBlock::StreamDataBlock(const std::function<std::istream &()> &stream, std::uint64_t startOffset, std::ios_base::seekdir startDir,
    std::uint64_t endOffset, std::ios_base::seekdir endDir)
    : m_stream(stream)
{
    auto &s = stream();
    auto currentPos = s.tellg();
    s.seekg(static_cast<std::istream::off_type>(startOffset), startDir);
    m_startOffset = static_cast<std::uint64_t>(s.tellg());
    s.seekg(static_cast<std::istream::off_type>(endOffset), endDir);
    m_endOffset = static_cast<std::uint64_t>(s.tellg());
    s.seekg(currentPos);
    if (m_endOffset < m_startOffset) {
        throw std::ios_base::failure("End offset is less than start offset.");
    }
}

/*!
 * \brief Discards buffered data.
 */
StreamDataBlock::~StreamDataBlock()
{
}

/*!
 * \brief Buffers the data block. Buffered data can be accessed via buffer().
 */
void StreamDataBlock::makeBuffer() const
{
    m_buffer = make_unique<char[]>(size());
    stream().seekg(static_cast<std::istream::off_type>(startOffset()));
    stream().read(m_buffer.get(), static_cast<std::streamsize>(size()));
}

/*!
 * \brief Copies the data to the specified \a stream.
 * \remarks Makes use of the buffer allocated with makeBuffer() if this method has been called before.
 */
void StreamDataBlock::copyTo(ostream &stream) const
{
    if (buffer()) {
        stream.write(buffer().get(), static_cast<std::streamsize>(size()));
    } else {
        CopyHelper<0x2000> copyHelper;
        m_stream().seekg(static_cast<std::streamsize>(startOffset()));
        copyHelper.copy(m_stream(), stream, size());
    }
}

/*!
 * \class TagParser::FileDataBlock
 * \brief The FileDataBlock class is a reference to a certain data block of a file stream.
 */

/*!
 * \brief Constructs a new FileDataBlock with the specified \a path.
 *
 * Opens a file stream with the specified \a path.
 *
 * \throws Throws ios_base::failure when an IO error occurs.
 */
FileDataBlock::FileDataBlock(std::string_view path, Diagnostics &diag, AbortableProgressFeedback &progress)
    : m_fileInfo(make_unique<MediaFileInfo>())
{
    m_fileInfo->setPath(path);
    m_fileInfo->open(true);
    m_fileInfo->parseContainerFormat(diag, progress);
    m_startOffset = 0;
    m_endOffset = m_fileInfo->size();
    m_stream = [this]() -> std::istream & { return this->m_fileInfo->stream(); };
}

/*!
 * \brief Destroys the FileDataBlock.
 * \remarks This method is needed although it is empty. Otherwise the default d'tor would be
 *          inlined where FileDataBlock is used creating a dependency to MediaFileInfo which
 *          therefore couldn't be opaque anymore.
 */
FileDataBlock::~FileDataBlock()
{
}

/*!
 * \class TagParser::AbstractAttachment
 * \brief The AbstractAttachment class parses and stores attachment information.
 */

/*!
 * \brief Constructs a new attachment.
 */
AbstractAttachment::AbstractAttachment()
    : m_id(0)
    , m_isDataFromFile(false)
    , m_ignored(false)
{
}

/*!
 * \brief Destroys the attachment.
 */
AbstractAttachment::~AbstractAttachment()
{
}

/*!
 * \brief Returns a label for the track.
 */
string AbstractAttachment::label() const
{
    stringstream ss;
    ss << "ID: " << id();
    if (!name().empty()) {
        ss << ", name: \"" << name() << "\"";
    }
    if (!mimeType().empty()) {
        ss << ", mime-type: \"" << mimeType() << "\"";
    }
    return ss.str();
}

/*!
 * \brief Resets the object to its initial state.
 */
void AbstractAttachment::clear()
{
    m_description.clear();
    m_name.clear();
    m_mimeType.clear();
    m_id = 0;
    m_data.reset();
}

/*!
 * \brief Sets the data, name and MIME-type for the specified \a path.
 *
 * A stream for the file with the specified \a path is opened (read-only).
 * This stream will be freed by the attachment if the other data is assigned
 * or the attachment gets destroyed.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived class when a parsing
 *         error occurs.
 *
 * When such an exception is thrown, the attachment remains unchanged.
 */
void AbstractAttachment::setFile(string_view path, Diagnostics &diag, AbortableProgressFeedback &progress)
{
    m_data.reset();
    auto file = make_unique<FileDataBlock>(path, diag, progress);
    const auto fileName = file->fileInfo()->fileName();
    if (!fileName.empty()) {
        m_name = fileName;
    }
    const auto mimeType = file->fileInfo()->mimeType();
    if (!mimeType.empty()) {
        m_mimeType = mimeType;
    }
    m_data = std::move(file);
    m_isDataFromFile = true;
}

} // namespace TagParser
