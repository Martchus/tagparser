#ifndef TAG_PARSER_WAVEAUDIOSTREAM_H
#define TAG_PARSER_WAVEAUDIOSTREAM_H

#include "../abstracttrack.h"

namespace TagParser
{

class TAG_PARSER_EXPORT WaveFormatHeader
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

class TAG_PARSER_EXPORT WaveAudioStream : public AbstractTrack
{
public:
    WaveAudioStream(std::iostream &stream, uint64 startOffset);
    ~WaveAudioStream() override;

    TrackType type() const override;

    static void addInfo(const WaveFormatHeader &waveHeader, AbstractTrack &track);

protected:
    void internalParseHeader(Diagnostics &diag) override;

private:
    uint64 m_dataOffset;
};

}

#endif // TAG_PARSER_WAVEAUDIOSTREAM_H
