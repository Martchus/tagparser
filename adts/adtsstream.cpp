#include "./adtsstream.h"

#include "../mp4/mp4ids.h"

#include "../exceptions.h"

#include <string>

using namespace std;

namespace TagParser {

/*!
 * \class Media::AdtsStream
 * \brief Implementation of Media::AbstractTrack for ADTS streams.
 */

void AdtsStream::internalParseHeader(Diagnostics &diag)
{
    //static const string context("parsing ADTS frame header");
    if (!m_istream) {
        throw NoDataFoundException();
    }
    // get size
    m_istream->seekg(-128, ios_base::end);
    if (m_reader.readUInt24BE() == 0x544147) {
        m_size = static_cast<uint64>(m_istream->tellg()) - 3u - m_startOffset;
    } else {
        m_size = static_cast<uint64>(m_istream->tellg()) + 125u - m_startOffset;
    }
    m_istream->seekg(m_startOffset, ios_base::beg);
    // parse frame header
    m_firstFrame.parseHeader(m_reader);
    m_format = Mpeg4AudioObjectIds::idToMediaFormat(m_firstFrame.mpeg4AudioObjectId());
    m_channelCount = Mpeg4ChannelConfigs::channelCount(m_channelConfig = m_firstFrame.mpeg4ChannelConfig());
    byte sampleRateIndex = m_firstFrame.mpeg4SamplingFrequencyIndex();
    m_samplingFrequency = sampleRateIndex < sizeof(mpeg4SamplingFrequencyTable) ? mpeg4SamplingFrequencyTable[sampleRateIndex] : 0;
}

} // namespace TagParser
