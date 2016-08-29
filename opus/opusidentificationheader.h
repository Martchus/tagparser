#ifndef MEDIA_OPUSIDENTIFICATIONHEADER_H
#define MEDIA_OPUSIDENTIFICATIONHEADER_H

#include "../global.h"

#include <c++utilities/conversion/types.h>

namespace Media {

class OggIterator;

class TAG_PARSER_EXPORT OpusIdentificationHeader
{
public:
    OpusIdentificationHeader();

    void parseHeader(OggIterator &iterator);

    byte version() const;
    byte channels() const;
    uint16 preSkip() const;
    uint32 sampleRate() const;
    uint16 outputGain() const;
    byte channelMap() const;

private:
    byte m_version;
    byte m_channels;
    uint16 m_preSkip;
    uint32 m_sampleRate;
    uint16 m_outputGain;
    byte m_channelMap;
};

/*!
 * \brief Constructs a new Opus identification header.
 */
inline OpusIdentificationHeader::OpusIdentificationHeader() :
    m_version(0),
    m_channels(0),
    m_sampleRate(0),
    m_outputGain(0),
    m_channelMap(0)
{}

/*!
 * \brief Returns the version (which should be 1 currently).
 */
inline byte OpusIdentificationHeader::version() const
{
    return m_version;
}

/*!
 * \brief Returns the number of channels for the Opus stream.
 */
inline byte OpusIdentificationHeader::channels() const
{
    return m_channels;
}

/*!
 * \brief Returns "pre-skip" value for the Opus stream.
 *
 * This is the number of samples (at 48 kHz) to discard from the decoder
 * output when starting playback, and also the number to subtract from a
 * page's granule position to calculate its PCM sample position.
 */
inline uint16 OpusIdentificationHeader::preSkip() const
{
    return m_preSkip;
}

/*!
 * \brief Returns the INPUT sample rate.
 * \remarks This is not the sample rate to use for playback of the encoded data.
 * \sa https://wiki.xiph.org/OggOpus
 */
inline uint32 OpusIdentificationHeader::sampleRate() const
{
    return m_sampleRate;
}

/*!
 * \brief Returns the output gain.
 *
 * This is a gain to be applied by the decoder. Virtually all players and media frameworks
 * should apply it by default.
 */
inline uint16 OpusIdentificationHeader::outputGain() const
{
    return m_outputGain;
}

/*!
 * \brief Returns the channel mapping family.
 *
 * The channel mapping family indicates the order and semantic meaning of the various channels
 * encoded in each Opus packet.
 * \sa https://wiki.xiph.org/OggOpus
 */
inline byte OpusIdentificationHeader::channelMap() const
{
    return m_channelMap;
}

}

#endif // MEDIA_OPUSIDENTIFICATIONHEADER_H
