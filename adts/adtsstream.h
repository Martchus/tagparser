#ifndef MEDIA_ADTSSTREAM_H
#define MEDIA_ADTSSTREAM_H

#include "./adtsframe.h"

#include "../abstracttrack.h"

namespace Media {

class LIB_EXPORT AdtsStream : public AbstractTrack
{
public:
    AdtsStream(std::iostream &stream, uint64 startOffset);
    ~AdtsStream();

    TrackType type() const;

protected:
    void internalParseHeader();

private:
    AdtsFrame m_firstFrame;
};

/*!
 * \brief Constructs a new track for the \a stream at the specified \a startOffset.
 */
inline AdtsStream::AdtsStream(std::iostream &stream, uint64 startOffset) :
    AbstractTrack(stream, startOffset)
{
    m_mediaType = MediaType::Audio;
}

inline AdtsStream::~AdtsStream()
{}

inline TrackType AdtsStream::type() const
{
    return TrackType::AdtsStream;
}

} // namespace Media

#endif // MEDIA_ADTSSTREAM_H
