#ifndef TAG_PARSER_FLACSTREAM_H
#define TAG_PARSER_FLACSTREAM_H

#include "../abstracttrack.h"

#include <iosfwd>
#include <memory>

namespace TagParser {

class MediaFileInfo;
class VorbisComment;

class TAG_PARSER_EXPORT FlacStream final : public AbstractTrack {
public:
    FlacStream(MediaFileInfo &mediaFileInfo, std::uint64_t startOffset);
    ~FlacStream() override;

    TrackType type() const override;
    VorbisComment *vorbisComment() const;
    VorbisComment *createVorbisComment();
    bool removeVorbisComment();
    std::uint32_t paddingSize() const;
    std::uint32_t streamOffset() const;

    std::streamoff makeHeader(std::ostream &stream, Diagnostics &diag);
    static void makePadding(std::ostream &stream, std::uint32_t size, bool isLast, Diagnostics &diag);

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    MediaFileInfo &m_mediaFileInfo;
    std::unique_ptr<VorbisComment> m_vorbisComment;
    std::uint32_t m_paddingSize;
    std::uint32_t m_streamOffset;
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
inline std::uint32_t FlacStream::paddingSize() const
{
    return m_paddingSize;
}

/*!
 * \brief Returns the start offset of the actual FLAC frames.
 * \remarks This equals the size of the metadata header blocks.
 */
inline std::uint32_t FlacStream::streamOffset() const
{
    return m_streamOffset;
}

} // namespace TagParser

#endif // TAG_PARSER_FLACSTREAM_H
