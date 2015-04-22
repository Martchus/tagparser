#ifndef WAVEAUDIOSTREAM_H
#define WAVEAUDIOSTREAM_H

#include "../abstracttrack.h"

#include <c++utilities/io/binaryreader.h>

#include <fstream>

namespace Media
{

class LIB_EXPORT WaveAudioStream : public AbstractTrack
{
public:
    WaveAudioStream(std::iostream &stream, uint64 startOffset);
    virtual ~WaveAudioStream();

    virtual TrackType type() const;

protected:
    virtual void internalParseHeader();

private:
    uint64 m_dataOffset;
};

}

#endif // WAVEAUDIOSTREAM_H
