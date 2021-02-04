#ifndef TAG_PARSER_MPEGAUDIOFRAMESTREAM_H
#define TAG_PARSER_MPEGAUDIOFRAMESTREAM_H

#include "./mpegaudioframe.h"

#include "../abstracttrack.h"

#include <list>

namespace TagParser {

class TAG_PARSER_EXPORT MpegAudioFrameStream final : public AbstractTrack {
public:
    MpegAudioFrameStream(std::iostream &stream, std::uint64_t startOffset);
    ~MpegAudioFrameStream() override;

    TrackType type() const override;

    static void addInfo(const MpegAudioFrame &frame, AbstractTrack &track);

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    std::list<MpegAudioFrame> m_frames;
};

/*!
 * \brief Constructs a new track for the \a stream at the specified \a startOffset.
 */
inline MpegAudioFrameStream::MpegAudioFrameStream(std::iostream &stream, std::uint64_t startOffset)
    : AbstractTrack(stream, startOffset)
{
    m_mediaType = MediaType::Audio;
}

inline MpegAudioFrameStream::~MpegAudioFrameStream()
{
}

inline TrackType MpegAudioFrameStream::type() const
{
    return TrackType::MpegAudioFrameStream;
}

} // namespace TagParser

#endif // MPEGAUDIOFRAMESTREAM_H
