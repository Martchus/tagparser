#include "abstractattachment.h"

#include "mediafileinfo.h"
#include "exceptions.h"

#include <sstream>

using namespace std;

namespace Media {

/*!
 * \class Media::StreamDataBlock
 * \brief The StreamDataBlock class is a reference to a certain data block of a stream.
 */

/*!
 * \brief Constructs a new StreamDataBlock.
 *
 * The derived is responsible for the prober initialization of the object.
 */
StreamDataBlock::StreamDataBlock() :
    m_stream(nullptr),
    m_startOffset(0),
    m_endOffset(0)
{}

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
StreamDataBlock::StreamDataBlock(const std::function<std::istream & ()> &stream, istream::off_type startOffset, ios_base::seekdir startDir, istream::off_type endOffset, ios_base::seekdir endDir) :
    m_stream(stream)
{
    auto &s = stream();
    auto currentPos = s.tellg();
    s.seekg(startOffset, startDir);
    m_startOffset = s.tellg();
    s.seekg(endOffset, endDir);
    m_endOffset = s.tellg();
    s.seekg(currentPos);
    if(m_endOffset < m_startOffset) {
        throw ios_base::failure("End offset is less then start offset.");
    }
}

/*!
 * \class Media::FileDataBlock
 * \brief The FileDataBlock class is a reference to a certain data block of a file stream.
 */

/*!
 * \brief Constructs a new FileDataBlock with the specified \a path.
 *
 * Opens a file stream with the specified \a path.
 *
 * \throws Throws ios_base::failure when an IO error occurs.
 */
FileDataBlock::FileDataBlock(const string &path) :
    m_fileInfo(new MediaFileInfo)
{
    m_fileInfo->setPath(path);
    m_fileInfo->open(true);
    m_fileInfo->parseContainerFormat();
    m_startOffset = 0;
    m_endOffset = m_fileInfo->size();
    m_stream = [this] () -> std::istream & {
        return this->m_fileInfo->stream();
    };
}

/*!
 * \class Media::AbstractAttachment
 * \brief The AbstractAttachment class parses and stores attachment information.
 */

/*!
 * \brief Returns a label for the track.
 */
string AbstractAttachment::label() const
{
    stringstream ss;
    ss << "ID: " << id();
    if(!name().empty()) {
        ss << ", name: \"" << name() << "\"";
    }
    if(!mimeType().empty()) {
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
 * \throws Throws Media::Failure or a derived class when a parsing
 *         error occurs.
 *
 * When such an exception is thrown, the attachment remains unchanged.
 */
void AbstractAttachment::setFile(const std::string &path)
{
    m_data.reset(new FileDataBlock(path));
    FileDataBlock &data = *static_cast<FileDataBlock *>(m_data.get());
    string fileName = data.fileInfo()->fileName();
    if(!fileName.empty()) {
        m_name = fileName;
    }
    const char *mimeType = data.fileInfo()->mimeType();
    if(*mimeType) {
        m_mimeType = mimeType;
    }
}

} // namespace Media

