#ifndef TAG_PARSER_FLACSTREAM_H
#define TAG_PARSER_FLACSTREAM_H

#include "../abstracttrack.h"

#include <iosfwd>
#include <memory>

namespace TagParser {

class MediaFileInfo;
class VorbisComment;

class TAG_PARSER_EXPORT FlacStream : public AbstractTrack {
public:
    FlacStream(MediaFileInfo &mediaFileInfo, uint64 startOffset);
    ~FlacStream() override;

    TrackType type() const override;
    VorbisComment *vorbisComment() const;
    VorbisComment *createVorbisComment();
    bool removeVorbisComment();
    uint32 paddingSize() const;
    uint32 streamOffset() const;

    std::streamoff makeHeader(std::ostream &stream, Diagnostics &diag);
    static void makePadding(std::ostream &stream, uint32 size, bool isLast, Diagnostics &diag);

protected:
    void internalParseHeader(Diagnostics &diag) override;

private:
    MediaFileInfo &m_mediaFileInfo;
    std::unique_ptr<VorbisComment> m_vorbisComment;
    uint32 m_paddingSize;
    uint32 m_streamOffset;
};

inline FlacStream::~FlacStream()
{
}

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

} // namespace TagParser

#endif // TAG_PARSER_FLACSTREAM_H
