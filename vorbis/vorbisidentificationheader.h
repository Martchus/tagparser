#ifndef TAG_PARSER_VORBISIDENTIFICATIONHEADER_H
#define TAG_PARSER_VORBISIDENTIFICATIONHEADER_H

#include "../global.h"

#include <c++utilities/conversion/types.h>

namespace TagParser {

class OggIterator;

class TAG_PARSER_EXPORT VorbisIdentificationHeader
{
public:
    VorbisIdentificationHeader();

    void parseHeader(OggIterator &iterator);

    uint32 version() const;
    byte channels() const;
    uint32 sampleRate() const;
    uint32 maxBitrate() const;
    uint32 nominalBitrate() const;
    uint32 minBitrate() const;
    byte blockSize() const;
    byte framingFlag() const;

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
inline VorbisIdentificationHeader::VorbisIdentificationHeader() :
    m_version(0),
    m_channels(0),
    m_sampleRate(0),
    m_maxBitrate(0),
    m_nominalBitrate(0),
    m_minBitrate(0),
    m_blockSize(0),
    m_framingFlag(0)
{}

inline uint32 VorbisIdentificationHeader::version() const
{
    return m_version;
}

inline byte VorbisIdentificationHeader::channels() const
{
    return m_channels;
}

inline uint32 VorbisIdentificationHeader::sampleRate() const
{
    return m_sampleRate;
}

inline uint32 VorbisIdentificationHeader::maxBitrate() const
{
    return m_maxBitrate;
}

inline uint32 VorbisIdentificationHeader::nominalBitrate() const
{
    return m_nominalBitrate;
}

inline uint32 VorbisIdentificationHeader::minBitrate() const
{
    return m_minBitrate;
}

inline byte VorbisIdentificationHeader::blockSize() const
{
    return m_blockSize;
}

inline byte VorbisIdentificationHeader::framingFlag() const
{
    return m_framingFlag;
}

}

#endif // TAG_PARSER_VORBISIDENTIFICATIONHEADER_H
