#ifndef WAVEAUDIOSTREAM_H
#define WAVEAUDIOSTREAM_H

#include "../abstracttrack.h"

#include <fstream>

namespace Media
{

class LIB_EXPORT WaveFormatHeader
{
public:
    WaveFormatHeader();

    void parse(IoUtilities::BinaryReader &reader);
    MediaFormat format() const;
    uint32 bitrate() const;

    uint16 formatTag;
    uint16 channelCount;
    uint16 sampleRate;
    uint16 bytesPerSecond;
    uint16 chunkSize;
    uint16 bitsPerSample;
};

/*!
 * \brief Calculates the bitrate from the header data.
 */
inline uint32 WaveFormatHeader::bitrate() const
{
    return bitsPerSample * sampleRate * channelCount;
}

class LIB_EXPORT WaveAudioStream : public AbstractTrack
{
public:
    WaveAudioStream(std::iostream &stream, uint64 startOffset);
    virtual ~WaveAudioStream();

    virtual TrackType type() const;

    static void addInfo(const WaveFormatHeader &waveHeader, AbstractTrack &track);

protected:
    virtual void internalParseHeader();

private:
    uint64 m_dataOffset;
};

}

#endif // WAVEAUDIOSTREAM_H
