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
 * \class TagParser::WaveFormatHeader
 * \brief The WaveFormatHeader class parses the WAVEFORMATEX structure defined by MS.
 */

/*!
 * \brief Parses the WAVE "fmt " header segment using the specified \a reader.
 * \remarks Reads 16 bytes from the associated stream.
 */
void WaveFormatHeader::parse(IoUtilities::BinaryReader &reader)
{
    Diagnostics diag;
    parseExt(reader, 16, diag);
}

/*!
 * \brief Parses the WAVE "fmt " header segment using the specified \a reader.
 * \returns Returns the detected media format and the number of bytes read.
 * \todo
 * - Make sampleRate and bytesPerSecond 32-bit in v9.
 * - Make GUID a field of WaveFormatHeader instead of returning the MediaFormat in v9.
 * - Replace parse() function with this.
 */
pair<MediaFormat, std::uint64_t> WaveFormatHeader::parseExt(IoUtilities::BinaryReader &reader, std::uint64_t maxSize, Diagnostics &diag)
{
    auto result = make_pair<MediaFormat, std::uint64_t>(GeneralMediaFormat::Unknown, 0);
    if (maxSize < 16) {
        diag.emplace_back(DiagLevel::Warning, "\"fmt \" segment is truncated.", "parsing WAVE format header");
        return result;
    }

    formatTag = reader.readUInt16LE();
    result.first = format();
    channelCount = reader.readUInt16LE();
    sampleRate = reader.readUInt32LE();
    bytesPerSecond = reader.readUInt32LE();
    chunkSize = reader.readUInt16LE();
    bitsPerSample = reader.readUInt16LE();
    result.second = 16;

    // read extended header unless format is PCM
    if (result.first.general == GeneralMediaFormat::Pcm) {
        return result;
    }
    if ((maxSize -= 16) < 2) {
        diag.emplace_back(DiagLevel::Warning, "\"fmt \" segment is truncated (extended header missing).", "parsing WAVE format header");
        return result;
    }
    const auto extensionSize = reader.readUInt16LE();
    result.second += 2;
    if ((maxSize -= 2) < 2) {
        diag.emplace_back(DiagLevel::Warning, "\"fmt \" segment is truncated (extended header truncated).", "parsing WAVE format header");
        return result;
    }

    // skip extended header unless format is "WAVE_FORMAT_EXTENSIBLE"
    if (formatTag != 65534) {
        reader.stream()->seekg(extensionSize, ios_base::cur);
        result.second += extensionSize;
        return result;
    }

    // read extended header for "WAVE_FORMAT_EXTENSIBLE"
    if (extensionSize != 22) {
        diag.emplace_back(DiagLevel::Warning, "\"fmt \" extended header has unexptected size.", "parsing WAVE format header");
        return result;
    }
    bitsPerSample = reader.readUInt16LE();
    reader.stream()->seekg(4, ios_base::cur); // skip channel mask
    const auto guid1 = reader.readUInt64BE();
    const auto guid2 = reader.readUInt64BE();
    result.second += 22;
    switch (guid2) {
    case 0x000800000aa00389b71:
        switch (guid1) {
        case 0x0100000000001000ul:
            result.first = MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmIntLe);
            break;
        case 0x0300000000001000ul:
            result.first = MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmFloatIeee);
            break;
        }
        break;
    }
    return result;
}

/*!
 * \brief Returns the media format denoted by the format tag.
 */
MediaFormat WaveFormatHeader::format() const
{
    switch (formatTag) {
    case 0x0001u:
        return MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmIntLe);
    case 0x0003u:
        return MediaFormat(GeneralMediaFormat::Pcm, SubFormats::PcmFloatIeee);
    case 0x0050u:
        return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer2);
    case 0x0055u:
        return MediaFormat(GeneralMediaFormat::Mpeg1Audio, SubFormats::Mpeg1Layer3);
    default:
        return GeneralMediaFormat::Unknown;
    }
}

/*!
 * \class TagParser::WaveAudioStream
 * \brief Implementation of TagParser::AbstractTrack for the
 *        RIFF WAVE container format.
 */

/*!
 * \brief Constructs a new track for the \a stream at the specified \a startOffset.
 */
WaveAudioStream::WaveAudioStream(iostream &stream, std::uint64_t startOffset)
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
    m_istream->seekg(static_cast<streamoff>(m_startOffset + 8));
    if (m_reader.readUInt32BE() != 0x57415645u) {
        throw NoDataFoundException();
    }
    while (!m_dataOffset) {
        const auto segmentId = m_reader.readUInt32BE();
        auto restHeaderLen = m_reader.readUInt32LE();
        switch (segmentId) {
        case 0x666D7420u: { // format segment
            WaveFormatHeader waveHeader;
            std::uint64_t bytesRead;
            tie(m_format, bytesRead) = waveHeader.parseExt(m_reader, restHeaderLen, diag);
            addInfo(waveHeader, *this);
            restHeaderLen -= bytesRead;
        } break;
        case 0x64617461u: // data segment
            m_dataOffset = static_cast<std::uint64_t>(m_istream->tellg());
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
    m_istream->seekg(static_cast<streamoff>(m_dataOffset));
    MpegAudioFrame frame;
    frame.parseHeader(m_reader, diag);
    MpegAudioFrameStream::addInfo(frame, *this);
    m_bitrate = frame.isXingFramefieldPresent() ? ((static_cast<double>(m_size) * 8.0)
                    / (static_cast<double>(frame.xingFrameCount() * frame.sampleCount()) / static_cast<double>(frame.samplingFrequency())) / 1024.0)
                                                : frame.bitrate();
    m_bytesPerSecond = static_cast<std::uint32_t>(m_bitrate * 125);
    m_duration = TimeSpan::fromSeconds(static_cast<double>(m_size) / (m_bitrate * 128.0));
}

} // namespace TagParser
