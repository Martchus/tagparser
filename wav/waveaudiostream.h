#ifndef TAG_PARSER_WAVEAUDIOSTREAM_H
#define TAG_PARSER_WAVEAUDIOSTREAM_H

#include "../abstracttrack.h"

namespace TagParser {

class TAG_PARSER_EXPORT WaveFormatHeader {
public:
    constexpr WaveFormatHeader();

    void parse(IoUtilities::BinaryReader &reader);
    std::pair<MediaFormat, uint64> parseExt(IoUtilities::BinaryReader &reader, uint64 maxSize, Diagnostics &diag);
    MediaFormat format() const;
    constexpr uint32 bitrate() const;

    uint16 formatTag;
    uint16 channelCount;
    uint16 sampleRate;
    uint16 bytesPerSecond;
    uint16 chunkSize;
    uint16 bitsPerSample;
};

/*!
 * \brief Constructs a new WaveFormatHeader.
 */
constexpr WaveFormatHeader::WaveFormatHeader()
    : formatTag(0)
    , channelCount(0)
    , sampleRate(0)
    , bytesPerSecond(0)
    , chunkSize(0)
    , bitsPerSample(0)
{
}

/*!
 * \brief Calculates the bitrate from the header data.
 */
constexpr uint32 WaveFormatHeader::bitrate() const
{
    return bitsPerSample * sampleRate * channelCount;
}

class TAG_PARSER_EXPORT WaveAudioStream : public AbstractTrack {
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

} // namespace TagParser

#endif // TAG_PARSER_WAVEAUDIOSTREAM_H
