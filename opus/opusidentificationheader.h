#ifndef TAG_PARSER_OPUSIDENTIFICATIONHEADER_H
#define TAG_PARSER_OPUSIDENTIFICATIONHEADER_H

#include "../global.h"

#include <c++utilities/conversion/types.h>

namespace TagParser {

class OggIterator;

class TAG_PARSER_EXPORT OpusIdentificationHeader {
public:
    constexpr OpusIdentificationHeader();

    void parseHeader(OggIterator &iterator);

    constexpr byte version() const;
    constexpr byte channels() const;
    constexpr uint16 preSkip() const;
    constexpr uint32 sampleRate() const;
    constexpr uint16 outputGain() const;
    constexpr byte channelMap() const;

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
constexpr OpusIdentificationHeader::OpusIdentificationHeader()
    : m_version(0)
    , m_channels(0)
    , m_preSkip(0)
    , m_sampleRate(0)
    , m_outputGain(0)
    , m_channelMap(0)
{
}

/*!
 * \brief Returns the version (which should be 1 currently).
 */
constexpr byte OpusIdentificationHeader::version() const
{
    return m_version;
}

/*!
 * \brief Returns the number of channels for the Opus stream.
 */
constexpr byte OpusIdentificationHeader::channels() const
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
constexpr uint16 OpusIdentificationHeader::preSkip() const
{
    return m_preSkip;
}

/*!
 * \brief Returns the INPUT sample rate.
 * \remarks This is not the sample rate to use for playback of the encoded data.
 * \sa https://wiki.xiph.org/OggOpus
 */
constexpr uint32 OpusIdentificationHeader::sampleRate() const
{
    return m_sampleRate;
}

/*!
 * \brief Returns the output gain.
 *
 * This is a gain to be applied by the decoder. Virtually all players and media frameworks
 * should apply it by default.
 */
constexpr uint16 OpusIdentificationHeader::outputGain() const
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
constexpr byte OpusIdentificationHeader::channelMap() const
{
    return m_channelMap;
}

} // namespace TagParser

#endif // TAG_PARSER_OPUSIDENTIFICATIONHEADER_H
