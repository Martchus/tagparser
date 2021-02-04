#ifndef TAG_PARSER_IVFSTREAM_H
#define TAG_PARSER_IVFSTREAM_H

#include "./ivfframe.h"

#include "../abstracttrack.h"

namespace TagParser {

class TAG_PARSER_EXPORT IvfStream final : public AbstractTrack {
public:
    IvfStream(std::iostream &stream, std::uint64_t startOffset);
    ~IvfStream() override;

    TrackType type() const override;

    void readFrame(Diagnostics &diag);

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    std::vector<IvfFrame> m_frames;
    std::uint16_t m_headerLength;
};

/*!
 * \brief Constructs a new track for the \a stream at the specified \a startOffset.
 */
inline IvfStream::IvfStream(std::iostream &stream, std::uint64_t startOffset)
    : AbstractTrack(stream, startOffset)
{
    m_mediaType = MediaType::Video;
}

inline IvfStream::~IvfStream()
{
}

inline TrackType IvfStream::type() const
{
    return TrackType::IvfStream;
}

} // namespace TagParser

#endif // TAG_PARSER_IVFSTREAM_H
