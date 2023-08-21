#include "./mpegaudioframestream.h"

#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/conversion/stringbuilder.h>

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

void MpegAudioFrameStream::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing MPEG audio frame header");
    if (!m_istream) {
        throw NoDataFoundException();
    }
    // parse frames until the first valid, non-empty frame is reached
    m_istream->seekg(static_cast<std::streamoff>(m_startOffset), ios_base::beg);
    for (size_t invalidByteskipped = 0; m_frames.size() < 200 && invalidByteskipped <= 0x600u;) {
        MpegAudioFrame &frame = invalidByteskipped > 0 ? m_frames.back() : m_frames.emplace_back();
        try {
            frame.parseHeader(m_reader, diag);
        } catch (const InvalidDataException &) {
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
        const auto xingSize = frame.xingBytesfield();
        if (!m_size) {
            m_size = xingSize;
        } else if (xingSize != m_size) {
            diag.emplace_back(DiagLevel::Warning,
                argsToString("Real size of MPEG audio frames (", m_size, " byte) is not in accordance with value provided by Xing header (", xingSize,
                    " byte). The real size will be used."),
                context);
        }
    }
    if (frame.isXingFramefieldPresent()) {
        const auto duration = static_cast<double>(frame.xingFrameCount() * frame.sampleCount()) / static_cast<double>(frame.samplingFrequency());
        m_bitrate = static_cast<double>(m_size) / duration / 125.0;
        m_duration = TimeSpan::fromSeconds(duration);
    } else {
        m_bitrate = frame.bitrate();
        m_duration = TimeSpan::fromSeconds(static_cast<double>(m_size) / (m_bytesPerSecond = static_cast<std::uint32_t>(m_bitrate * 125)));
    }
}

} // namespace TagParser
