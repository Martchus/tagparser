#include "waveaudiostream.h"

#include "../exceptions.h"
#include "../mediaformat.h"

using namespace std;

namespace Media {

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
                switch(m_reader.readUInt16LE()) {
                case 0x0001u:
                    m_format = GeneralMediaFormat::Pcm;
                    break;
                case 0x0050u:
                    m_format = MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer2);
                    break;
                case 0x0055u:
                    m_format = MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
                    break;
                default:
                    m_format = GeneralMediaFormat::Unknown;
                }
                m_channelCount = m_reader.readUInt16LE();
                m_samplesPerSecond = m_reader.readUInt32LE();
                m_bytesPerSecond = m_reader.readUInt32LE();
                m_chunkSize = m_reader.readUInt16LE();
                m_bitsPerSample = m_reader.readUInt16LE();
                m_bitrate = m_bitsPerSample * m_samplesPerSecond * m_channelCount;
            } else {
                m_format = GeneralMediaFormat::Unknown;
            }
            if(restHeaderLen > 16u) {
                m_istream->seekg(m_dataOffset, ios_base::beg);
            }
            if(m_reader.readUInt32BE() == 0x64617461u) {
                m_size = m_reader.readUInt32LE();
                m_sampleCount = m_size / m_chunkSize;
                m_duration = ChronoUtilities::TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / static_cast<double>(m_samplesPerSecond));
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
