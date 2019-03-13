#ifndef TAG_PARSER_VORBISIDENTIFICATIONHEADER_H
#define TAG_PARSER_VORBISIDENTIFICATIONHEADER_H

#include "../global.h"

#include <cstdint>

namespace TagParser {

class OggIterator;

class TAG_PARSER_EXPORT VorbisIdentificationHeader {
public:
    constexpr VorbisIdentificationHeader();

    void parseHeader(OggIterator &iterator);

    constexpr std::uint32_t version() const;
    constexpr std::uint8_t channels() const;
    constexpr std::uint32_t sampleRate() const;
    constexpr std::uint32_t maxBitrate() const;
    constexpr std::uint32_t nominalBitrate() const;
    constexpr std::uint32_t minBitrate() const;
    constexpr std::uint8_t blockSize() const;
    constexpr std::uint8_t framingFlag() const;

private:
    std::uint32_t m_version;
    std::uint8_t m_channels;
    std::uint32_t m_sampleRate;
    std::uint32_t m_maxBitrate;
    std::uint32_t m_nominalBitrate;
    std::uint32_t m_minBitrate;
    std::uint8_t m_blockSize;
    std::uint8_t m_framingFlag;
};

/*!
 * \brief Constructs a new Vorbis identification header.
 */
constexpr VorbisIdentificationHeader::VorbisIdentificationHeader()
    : m_version(0)
    , m_channels(0)
    , m_sampleRate(0)
    , m_maxBitrate(0)
    , m_nominalBitrate(0)
    , m_minBitrate(0)
    , m_blockSize(0)
    , m_framingFlag(0)
{
}

constexpr std::uint32_t VorbisIdentificationHeader::version() const
{
    return m_version;
}

constexpr std::uint8_t VorbisIdentificationHeader::channels() const
{
    return m_channels;
}

constexpr std::uint32_t VorbisIdentificationHeader::sampleRate() const
{
    return m_sampleRate;
}

constexpr std::uint32_t VorbisIdentificationHeader::maxBitrate() const
{
    return m_maxBitrate;
}

constexpr std::uint32_t VorbisIdentificationHeader::nominalBitrate() const
{
    return m_nominalBitrate;
}

constexpr std::uint32_t VorbisIdentificationHeader::minBitrate() const
{
    return m_minBitrate;
}

constexpr std::uint8_t VorbisIdentificationHeader::blockSize() const
{
    return m_blockSize;
}

constexpr std::uint8_t VorbisIdentificationHeader::framingFlag() const
{
    return m_framingFlag;
}

} // namespace TagParser

#endif // TAG_PARSER_VORBISIDENTIFICATIONHEADER_H
