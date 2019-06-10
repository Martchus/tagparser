#include "./abstractattachment.h"

#include "./exceptions.h"
#include "./mediafileinfo.h"

#include <c++utilities/io/copy.h>

#include <memory>
#include <sstream>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

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
StreamDataBlock::StreamDataBlock(const std::function<std::istream &()> &stream, std::istream::off_type startOffset, std::ios_base::seekdir startDir,
    std::istream::off_type endOffset, std::ios_base::seekdir endDir)
    : m_stream(stream)
{
    auto &s = stream();
    auto currentPos = s.tellg();
    s.seekg(startOffset, startDir);
    m_startOffset = s.tellg();
    s.seekg(endOffset, endDir);
    m_endOffset = s.tellg();
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
    stream().seekg(startOffset());
    stream().read(m_buffer.get(), size());
}

/*!
 * \brief Copies the data to the specified \a stream.
 * \remarks Makes use of the buffer allocated with makeBuffer() if this method has been called before.
 */
void StreamDataBlock::copyTo(ostream &stream) const
{
    if (buffer()) {
        stream.write(buffer().get(), size());
    } else {
        CopyHelper<0x2000> copyHelper;
        m_stream().seekg(startOffset());
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
FileDataBlock::FileDataBlock(const string &path, Diagnostics &diag)
    : m_fileInfo(make_unique<MediaFileInfo>())
{
    m_fileInfo->setPath(path);
    m_fileInfo->open(true);
    m_fileInfo->parseContainerFormat(diag);
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
void AbstractAttachment::setFile(const std::string &path, Diagnostics &diag)
{
    m_data.reset();
    auto file = make_unique<FileDataBlock>(path, diag);
    const auto fileName = file->fileInfo()->fileName();
    if (!fileName.empty()) {
        m_name = fileName;
    }
    const char *mimeType = file->fileInfo()->mimeType();
    if (*mimeType) {
        m_mimeType = mimeType;
    }
    m_data = move(file);
    m_isDataFromFile = true;
}

} // namespace TagParser
