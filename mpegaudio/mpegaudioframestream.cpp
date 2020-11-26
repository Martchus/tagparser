#include "./mpegaudioframestream.h"

#include "../exceptions.h"
#include "../mediaformat.h"

#include <sstream>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::MpegAudioFrameStream
 * \brief Implementation of TagParser::AbstractTrack MPEG audio streams.
 */

/*!
 * \brief Adds the information from the specified \a frame to the specified \a track.
 */
void MpegAudioFrameStream::addInfo(const MpegAudioFrame &frame, AbstractTrack &track)
{
    track.m_version = frame.mpegVersion();
    track.m_format = MediaFormat(GeneralMediaFormat::Mpeg1Audio, static_cast<unsigned char>(frame.layer()));
    track.m_channelCount = frame.channelMode() == MpegChannelMode::SingleChannel ? 1 : 2;
    track.m_channelConfig = static_cast<std::uint8_t>(frame.channelMode());
    track.m_samplingFrequency = frame.samplingFrequency();
}

void MpegAudioFrameStream::internalParseHeader(Diagnostics &diag)
{
    static const string context("parsing MPEG audio frame header");
    if (!m_istream) {
        throw NoDataFoundException();
    }
    // get size
    m_istream->seekg(-128, ios_base::end);
    if (m_reader.readUInt24BE() == 0x544147) {
        m_size = static_cast<std::uint64_t>(m_istream->tellg()) - 3u - m_startOffset;
    } else {
        m_size = static_cast<std::uint64_t>(m_istream->tellg()) + 125u - m_startOffset;
    }
    m_istream->seekg(static_cast<streamoff>(m_startOffset), ios_base::beg);
    // parse frames until the first valid, non-empty frame is reached
    for (size_t invalidByteskipped = 0; m_frames.size() < 200 && invalidByteskipped <= 0x600u;) {
        MpegAudioFrame &frame = invalidByteskipped > 0 ? m_frames.back() : m_frames.emplace_back();
        try {
            frame.parseHeader(m_reader, diag);
        } catch (const InvalidDataException &e) {
            if (++invalidByteskipped > 1) {
                diag.pop_back();
            }
            m_istream->seekg(-3, ios_base::cur);
            continue;
        }
        if (invalidByteskipped > 1) {
            diag.emplace_back(DiagLevel::Critical, argsToString("The next ", invalidByteskipped, " bytes are junk as well."), context);
        }
        if (!frame.size()) {
            continue; // likely just junk, check further frames
        }
        invalidByteskipped = 0;
        if (frame.isProtectedByCrc()) {
            m_istream->seekg(2, ios_base::cur);
        }
        break;
    }
    if (!m_frames.back().isValid()) {
        return;
    }
    const MpegAudioFrame &frame = m_frames.back();
    addInfo(frame, *this);
    if (frame.isXingBytesfieldPresent()) {
        std::uint32_t xingSize = frame.xingBytesfield();
        if (m_size && xingSize != m_size) {
            diag.emplace_back(DiagLevel::Warning,
                "Real length of MPEG audio frames is not in accordance with value provided by Xing header. The Xing header value will be used.",
                context);
            m_size = xingSize;
        }
    }
    m_bitrate = frame.isXingFramefieldPresent() ? ((static_cast<double>(m_size) * 8.0)
                    / (static_cast<double>(frame.xingFrameCount() * frame.sampleCount()) / static_cast<double>(frame.samplingFrequency())) / 1024.0)
                                                : frame.bitrate();
    m_duration = TimeSpan::fromSeconds(static_cast<double>(m_size) / (m_bytesPerSecond = static_cast<std::uint32_t>(m_bitrate * 125)));
}

} // namespace TagParser
