#ifndef MPEGAUDIOFRAMESTREAM_H
#define MPEGAUDIOFRAMESTREAM_H

#include "mpegaudioframe.h"
#include "../abstracttrack.h"

#include <list>

namespace Media
{

class LIB_EXPORT MpegAudioFrameStream : public AbstractTrack
{
public:
    MpegAudioFrameStream(std::iostream &stream, uint64 startOffset);
    ~MpegAudioFrameStream();

    TrackType type() const;
protected:
    void internalParseHeader();

private:
    std::list<MpegAudioFrame> m_frames;
};

}

#endif // MPEGAUDIOFRAMESTREAM_H
