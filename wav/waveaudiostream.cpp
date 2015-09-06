#include "./waveaudiostream.h"

#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/io/binaryreader.h>

using namespace std;

namespace Media {

/*!
 * \class Media::WaveFormatHeader
 * \brief The WaveFormatHeader class parses the WAVEFORMATEX structure defined by MS.
 */

/*!
 * \brief Constructs a new WaveFormatHeader.
 */
WaveFormatHeader::WaveFormatHeader() :
    formatTag(0),
    channelCount(0),
    sampleRate(0),
    bytesPerSecond(0),
    chunkSize(0),
    bitsPerSample(0)
{}

/*!
 * \brief Parses the WAVE header using the specified \a reader.
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
    switch(formatTag) {
    case 0x0001u: return GeneralMediaFormat::Pcm;
    case 0x0050u: return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer2);
    case 0x0055u: return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
    default: return GeneralMediaFormat::Unknown;
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
WaveAudioStream::WaveAudioStream(iostream &stream, uint64 startOffset) :
    AbstractTrack(stream, startOffset)
{
    m_mediaType = MediaType::Audio;
}

/*!
 * \brief Destroys the track.
 */
WaveAudioStream::~WaveAudioStream()
{}

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
    track.m_channelCount = waveHeader.channelCount;
    track.m_samplingFrequency = waveHeader.sampleRate;
    track.m_bytesPerSecond = waveHeader.bytesPerSecond;
    track.m_chunkSize = waveHeader.chunkSize;
    track.m_bitsPerSample = waveHeader.bitsPerSample;
    track.m_bitrate = waveHeader.bitrate();
}

void WaveAudioStream::internalParseHeader()
{
    if(!m_istream) {
        throw NoDataFoundException();
    }
    if(m_reader.readUInt32BE() == 0x52494646u) {
        m_istream->seekg(4, ios_base::cur);
        if(m_reader.readUInt32BE() == 0x57415645u && m_reader.readUInt32BE() == 0x666D7420u) {
            uint32 restHeaderLen = m_reader.readUInt32LE();
            m_dataOffset = static_cast<uint64>(m_istream->tellg()) + static_cast<uint64>(restHeaderLen);
            if(restHeaderLen >= 16u) {
                WaveFormatHeader waveHeader;
                waveHeader.parse(m_reader);
                addInfo(waveHeader, *this);
            }
            if(restHeaderLen > 16u) {
                m_istream->seekg(m_dataOffset, ios_base::beg);
            }
            if(m_reader.readUInt32BE() == 0x64617461u) {
                m_size = m_reader.readUInt32LE();
                m_sampleCount = m_size / m_chunkSize;
                m_duration = ChronoUtilities::TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / static_cast<double>(m_samplingFrequency));
            } else {
                throw NoDataFoundException();
            }
        } else {
            throw NoDataFoundException();
        }
    } else {
        throw NoDataFoundException();
    }
}

}
