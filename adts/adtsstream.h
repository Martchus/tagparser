#ifndef TAG_PARSER_ADTSSTREAM_H
#define TAG_PARSER_ADTSSTREAM_H

#include "./adtsframe.h"

#include "../abstracttrack.h"

namespace TagParser {

class TAG_PARSER_EXPORT AdtsStream final : public AbstractTrack {
public:
    AdtsStream(std::iostream &stream, std::uint64_t startOffset);
    ~AdtsStream() override;

    TrackType type() const override;

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    AdtsFrame m_firstFrame;
};

/*!
 * \brief Constructs a new track for the \a stream at the specified \a startOffset.
 */
inline AdtsStream::AdtsStream(std::iostream &stream, std::uint64_t startOffset)
    : AbstractTrack(stream, startOffset)
{
    m_mediaType = MediaType::Audio;
}

inline AdtsStream::~AdtsStream()
{
}

inline TrackType AdtsStream::type() const
{
    return TrackType::AdtsStream;
}

} // namespace TagParser

#endif // TAG_PARSER_ADTSSTREAM_H
