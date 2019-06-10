#ifndef TAG_PARSER_WAVEAUDIOSTREAM_H
#define TAG_PARSER_WAVEAUDIOSTREAM_H

#include "../abstracttrack.h"

namespace TagParser {

class TAG_PARSER_EXPORT WaveFormatHeader {
public:
    constexpr WaveFormatHeader();

    void parse(CppUtilities::BinaryReader &reader);
    std::pair<MediaFormat, std::uint64_t> parseExt(CppUtilities::BinaryReader &reader, std::uint64_t maxSize, Diagnostics &diag);
    MediaFormat format() const;
    constexpr std::uint32_t bitrate() const;

    std::uint16_t formatTag;
    std::uint16_t channelCount;
    std::uint16_t sampleRate;
    std::uint16_t bytesPerSecond;
    std::uint16_t chunkSize;
    std::uint16_t bitsPerSample;
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
constexpr std::uint32_t WaveFormatHeader::bitrate() const
{
    return bitsPerSample * sampleRate * channelCount;
}

class TAG_PARSER_EXPORT WaveAudioStream : public AbstractTrack {
public:
    WaveAudioStream(std::iostream &stream, std::uint64_t startOffset);
    ~WaveAudioStream() override;

    TrackType type() const override;

    static void addInfo(const WaveFormatHeader &waveHeader, AbstractTrack &track);

protected:
    void internalParseHeader(Diagnostics &diag) override;

private:
    std::uint64_t m_dataOffset;
};

} // namespace TagParser

#endif // TAG_PARSER_WAVEAUDIOSTREAM_H
