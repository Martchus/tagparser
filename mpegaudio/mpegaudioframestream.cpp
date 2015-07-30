#include "mpegaudioframestream.h"

#include "../exceptions.h"
#include "../mediaformat.h"

#include <sstream>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

namespace Media {

/*!
 * \class Media::MpegAudioFrameStream
 * \brief Implementation of Media::AbstractTrack MPEG audio streams.
 */

/*!
 * \brief Adds the information from the specified \a frame to the specified \a track.
 */
void MpegAudioFrameStream::addInfo(const MpegAudioFrame &frame, AbstractTrack &track)
{
    track.m_version = frame.mpegVersion();
    track.m_format = MediaFormat(GeneralMediaFormat::Mpeg1Audio, frame.layer());
    track.m_channelCount = frame.channelMode() == MpegChannelMode::SingleChannel ? 1 : 2;
    track.m_channelConfig = static_cast<byte>(frame.channelMode());
    track.m_sampleRate = frame.samplingFrequency();
}

void MpegAudioFrameStream::internalParseHeader()
{
    static const string context("parsing MPEG audio frame header");
    if(!m_istream) {
        throw NoDataFoundException();
    }
    // get size
    m_istream->seekg(-128, ios_base::end);
    if(m_reader.readUInt24BE() == 0x544147) {
        m_size = static_cast<uint64>(m_istream->tellg()) - 3u - m_startOffset;
    } else {
        m_size = static_cast<uint64>(m_istream->tellg()) + 125u - m_startOffset;
    }
    m_istream->seekg(m_startOffset, ios_base::beg);
    // parse frame header
    m_frames.emplace_back();
    MpegAudioFrame &frame = m_frames.back();
    frame.parseHeader(m_reader);
    addInfo(frame, *this);
    if(frame.isXingBytesfieldPresent()) {
        uint32 xingSize = frame.xingBytesfield();
        if(m_size && xingSize != m_size) {
            addNotification(NotificationType::Warning, "Real length MPEG of audio frames is not equal with value provided by Xing header. The Xing header value will be used.", context);
            m_size = xingSize;
        }
    }
    m_bitrate = frame.isXingFramefieldPresent()
            ? ((static_cast<double>(m_size) * 8.0) / (static_cast<double>(frame.xingFrameCount() * frame.sampleCount()) / static_cast<double>(frame.samplingFrequency())) / 1024.0)
            : frame.bitrate();
    m_bytesPerSecond = m_bitrate * 125;
    m_duration = TimeSpan::fromSeconds(static_cast<double>(m_size) / (m_bitrate * 128.0));
}

}
