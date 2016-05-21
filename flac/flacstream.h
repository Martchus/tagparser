#ifndef FLACSTREAM_H
#define FLACSTREAM_H

#include "../abstracttrack.h"

#include <c++utilities/misc/memory.h>

#include <iosfwd>

namespace Media {

class MediaFileInfo;
class VorbisComment;

class LIB_EXPORT FlacStream : public AbstractTrack
{
public:
    FlacStream(MediaFileInfo &mediaFileInfo, uint64 startOffset);
    ~FlacStream();

    TrackType type() const;
    VorbisComment *vorbisComment() const;
    VorbisComment *createVorbisComment();
    bool removeVorbisComment();
    uint32 paddingSize() const;
    uint32 streamOffset() const;

    uint32 makeHeader(std::ostream &stream);
    static void makePadding(std::ostream &stream, uint32 size, bool isLast);

protected:
    void internalParseHeader();

private:
    MediaFileInfo &m_mediaFileInfo;
    std::unique_ptr<VorbisComment> m_vorbisComment;
    uint32 m_paddingSize;
    uint32 m_streamOffset;
};

inline FlacStream::~FlacStream()
{}

inline TrackType FlacStream::type() const
{
    return TrackType::FlacStream;
}

/*!
 * \brief Returns the Vorbis comment if one is present in the stream.
 */
inline VorbisComment *FlacStream::vorbisComment() const
{
    return m_vorbisComment.get();
}

/*!
 * \brief Returns the padding size.
 */
inline uint32 FlacStream::paddingSize() const
{
    return m_paddingSize;
}

/*!
 * \brief Returns the start offset of the actual FLAC frames.
 * \remarks This equals the size of the metadata header blocks.
 */
inline uint32 FlacStream::streamOffset() const
{
    return m_streamOffset;
}

}

#endif // FLACSTREAM_H
