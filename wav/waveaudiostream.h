#ifndef TAG_PARSER_WAVEAUDIOSTREAM_H
#define TAG_PARSER_WAVEAUDIOSTREAM_H

#include "../abstracttrack.h"

namespace TagParser {

class TAG_PARSER_EXPORT WaveFormatHeader {
public:
    constexpr WaveFormatHeader();

    std::uint64_t parse(CppUtilities::BinaryReader &reader, std::uint64_t maxSize, Diagnostics &diag);
    MediaFormat format() const;
    constexpr std::uint32_t bitrate() const;

    std::uint64_t guid1;
    std::uint64_t guid2;
    std::uint16_t formatTag;
    std::uint16_t channelCount;
    std::uint32_t sampleRate;
    std::uint32_t bytesPerSecond;
    std::uint16_t chunkSize;
    std::uint16_t bitsPerSample;
    std::uint32_t channelMask;
};

/*!
 * \brief Constructs a new WaveFormatHeader.
 */
constexpr WaveFormatHeader::WaveFormatHeader()
    : guid1(0)
    , guid2(0)
    , formatTag(0)
    , channelCount(0)
    , sampleRate(0)
    , bytesPerSecond(0)
    , chunkSize(0)
    , bitsPerSample(0)
    , channelMask(0)
{
}

/*!
 * \brief Calculates the bitrate from the header data.
 */
constexpr std::uint32_t WaveFormatHeader::bitrate() const
{
    return bitsPerSample * sampleRate * channelCount;
}

class TAG_PARSER_EXPORT WaveAudioStream final : public AbstractTrack {
public:
    WaveAudioStream(std::iostream &stream, std::uint64_t startOffset);
    ~WaveAudioStream() override;

    TrackType type() const override;

    static void addInfo(const WaveFormatHeader &waveHeader, AbstractTrack &track);

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    std::uint64_t m_dataOffset;
};

} // namespace TagParser

#endif // TAG_PARSER_WAVEAUDIOSTREAM_H
