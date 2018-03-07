#include "./waveaudiostream.h"

#include "../mpegaudio/mpegaudioframestream.h"

#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/binaryreader.h>

using namespace std;
using namespace ChronoUtilities;

namespace TagParser {

/*!
 * \class Media::WaveFormatHeader
 * \brief The WaveFormatHeader class parses the WAVEFORMATEX structure defined by MS.
 */

/*!
 * \brief Constructs a new WaveFormatHeader.
 */
WaveFormatHeader::WaveFormatHeader()
    : formatTag(0)
    , channelCount(0)
    , sampleRate(0)
    , bytesPerSecond(0)
    , chunkSize(0)
    , bitsPerSample(0)
{
}

/*!
 * \brief Parses the WAVE "fmt " header segment using the specified \a reader.
 * \remarks Reads 16 bytes from the associated stream.
 */
void WaveFormatHeader::parse(IoUtilities::BinaryReader &reader)
{
    formatTag = reader.readUInt16LE();
    channelCount = reader.readUInt16LE();
    sampleRate = reader.readUInt32LE();
    bytesPerSecond = reader.readUInt32LE();
    chunkSize = reader.readUInt16LE();
    bitsPerSample = reader.readUInt16LE();
}

/*!
 * \brief Returns the media format denoted by the format tag.
 */
MediaFormat WaveFormatHeader::format() const
{
    switch (formatTag) {
    case 0x0001u:
        return GeneralMediaFormat::Pcm;
    case 0x0050u:
        return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer2);
    case 0x0055u:
        return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
    default:
        return GeneralMediaFormat::Unknown;
    }
}

/*!
 * \class Media::WaveAudioStream
 * \brief Implementation of Media::AbstractTrack for the
 *        RIFF WAVE container format.
 */

/*!
 * \brief Constructs a new track for the \a stream at the specified \a startOffset.
 */
WaveAudioStream::WaveAudioStream(iostream &stream, uint64 startOffset)
    : AbstractTrack(stream, startOffset)
    , m_dataOffset(0)
{
    m_mediaType = MediaType::Audio;
}

/*!
 * \brief Destroys the track.
 */
WaveAudioStream::~WaveAudioStream()
{
}

TrackType WaveAudioStream::type() const
{
    return TrackType::WaveAudioStream;
}

/*!
 * \brief Adds the information from the specified \a waveHeader to the specified \a track.
 */
void WaveAudioStream::addInfo(const WaveFormatHeader &waveHeader, AbstractTrack &track)
{
    track.m_format = waveHeader.format();
    track.m_formatId = ConversionUtilities::numberToString(waveHeader.formatTag);
    track.m_channelCount = waveHeader.channelCount;
    track.m_samplingFrequency = waveHeader.sampleRate;
    track.m_bytesPerSecond = waveHeader.bytesPerSecond;
    track.m_chunkSize = waveHeader.chunkSize;
    track.m_bitsPerSample = waveHeader.bitsPerSample;
    track.m_bitrate = waveHeader.bitrate();
}

void WaveAudioStream::internalParseHeader(Diagnostics &diag)
{
    const string context("parsing RIFF/WAVE header");
    if (!m_istream) {
        throw NoDataFoundException();
    }
    if (m_reader.readUInt32BE() != 0x52494646u) {
        throw NoDataFoundException();
    }
    m_istream->seekg(m_startOffset + 8);
    if (m_reader.readUInt32BE() != 0x57415645u) {
        throw NoDataFoundException();
    }
    while (!m_dataOffset) {
        uint32 segmentId = m_reader.readUInt32BE();
        uint32 restHeaderLen = m_reader.readUInt32LE();
        switch (segmentId) {
        case 0x666D7420u:
            if (restHeaderLen >= 16u) {
                WaveFormatHeader waveHeader;
                waveHeader.parse(m_reader);
                addInfo(waveHeader, *this);
                restHeaderLen -= 16u;
            } else {
                diag.emplace_back(DiagLevel::Warning, "\"fmt \" segment is truncated.", context);
            }
            break;
        case 0x64617461u:
            m_dataOffset = m_istream->tellg();
            m_size = restHeaderLen;
            m_sampleCount = m_size / m_chunkSize;
            m_duration = TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / static_cast<double>(m_samplingFrequency));
            break;
        default:;
        }
        m_istream->seekg(restHeaderLen, ios_base::cur);
    }
    if (m_format.general != GeneralMediaFormat::Mpeg1Audio || !m_dataOffset) {
        return;
    }
    m_istream->seekg(m_dataOffset);
    MpegAudioFrame frame;
    frame.parseHeader(m_reader);
    MpegAudioFrameStream::addInfo(frame, *this);
    m_bitrate = frame.isXingFramefieldPresent()
        ? ((static_cast<double>(m_size) * 8.0)
              / (static_cast<double>(frame.xingFrameCount() * frame.sampleCount()) / static_cast<double>(frame.samplingFrequency())) / 1024.0)
        : frame.bitrate();
    m_bytesPerSecond = m_bitrate * 125;
    m_duration = TimeSpan::fromSeconds(static_cast<double>(m_size) / (m_bitrate * 128.0));
}

} // namespace TagParser
