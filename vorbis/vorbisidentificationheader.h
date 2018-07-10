#ifndef TAG_PARSER_VORBISIDENTIFICATIONHEADER_H
#define TAG_PARSER_VORBISIDENTIFICATIONHEADER_H

#include "../global.h"

#include <c++utilities/conversion/types.h>

namespace TagParser {

class OggIterator;

class TAG_PARSER_EXPORT VorbisIdentificationHeader {
public:
    constexpr VorbisIdentificationHeader();

    void parseHeader(OggIterator &iterator);

    constexpr uint32 version() const;
    constexpr byte channels() const;
    constexpr uint32 sampleRate() const;
    constexpr uint32 maxBitrate() const;
    constexpr uint32 nominalBitrate() const;
    constexpr uint32 minBitrate() const;
    constexpr byte blockSize() const;
    constexpr byte framingFlag() const;

private:
    uint32 m_version;
    byte m_channels;
    uint32 m_sampleRate;
    uint32 m_maxBitrate;
    uint32 m_nominalBitrate;
    uint32 m_minBitrate;
    byte m_blockSize;
    byte m_framingFlag;
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

constexpr uint32 VorbisIdentificationHeader::version() const
{
    return m_version;
}

constexpr byte VorbisIdentificationHeader::channels() const
{
    return m_channels;
}

constexpr uint32 VorbisIdentificationHeader::sampleRate() const
{
    return m_sampleRate;
}

constexpr uint32 VorbisIdentificationHeader::maxBitrate() const
{
    return m_maxBitrate;
}

constexpr uint32 VorbisIdentificationHeader::nominalBitrate() const
{
    return m_nominalBitrate;
}

constexpr uint32 VorbisIdentificationHeader::minBitrate() const
{
    return m_minBitrate;
}

constexpr byte VorbisIdentificationHeader::blockSize() const
{
    return m_blockSize;
}

constexpr byte VorbisIdentificationHeader::framingFlag() const
{
    return m_framingFlag;
}

} // namespace TagParser

#endif // TAG_PARSER_VORBISIDENTIFICATIONHEADER_H
