#ifndef TAG_PARSER_ADTSFRAME_H
#define TAG_PARSER_ADTSFRAME_H

#include "../global.h"

#include <c++utilities/conversion/types.h>

namespace IoUtilities {
class BinaryReader;
}

namespace TagParser {

class TAG_PARSER_EXPORT AdtsFrame {
public:
    constexpr AdtsFrame();

    void parseHeader(IoUtilities::BinaryReader &reader);

    constexpr bool isValid() const;
    constexpr bool isMpeg4() const;
    constexpr bool hasCrc() const;
    constexpr byte mpeg4AudioObjectId() const;
    constexpr byte mpeg4SamplingFrequencyIndex() const;
    constexpr byte mpeg4ChannelConfig() const;
    constexpr uint16 totalSize() const;
    constexpr byte headerSize() const;
    constexpr uint16 dataSize() const;
    constexpr uint16 bufferFullness() const;
    constexpr byte frameCount() const;
    constexpr uint16 crc() const;

private:
    uint16 m_header1;
    uint64 m_header2;
};

/*!
 * \brief Constructs a new frame.
 */
constexpr AdtsFrame::AdtsFrame()
    : m_header1(0)
    , m_header2(0)
{
}

/*!
 * \brief Returns an indication whether the frame is valid.
 */
constexpr bool AdtsFrame::isValid() const
{
    return ((m_header1 & 0xFFF6u) == 0xFFF0u) && (totalSize() >= headerSize());
}

/*!
 * \brief Returns whether the MPEG version is MPEG-4; otherwise the MPEG version is MPEG-2.
 */
constexpr bool AdtsFrame::isMpeg4() const
{
    return m_header1 & 0x8u;
}

/*!
 * \brief Returns whether a CRC-16 checksum is present ("protection absent" bit is NOT set).
 */
constexpr bool AdtsFrame::hasCrc() const
{
    return (m_header1 & 0x1u) == 0;
}

/*!
 * \brief Returns the MPEG-4 audio object type ID.
 * \sa TagParser::Mpeg4AudioObjectIds
 * \sa Mpeg4AudioObjectIds::idToMediaFormat()
 */
constexpr byte AdtsFrame::mpeg4AudioObjectId() const
{
    return (m_header2 >> 0x36) + 0x1u;
}

/*!
 * \brief Returns the MPEG-4 sample rate index.
 * \sa TagParser::mpeg4SampleRateTable
 */
constexpr byte AdtsFrame::mpeg4SamplingFrequencyIndex() const
{
    return (m_header2 >> 0x32) & 0xFu;
}

/*!
 * \brief Returns the MPEG-4 channel configuration.
 * \sa TagParser::Mpeg4ChannelConfigs
 * \sa TagParser::mpeg4SampleRateTable::channelConfigString()
 */
constexpr byte AdtsFrame::mpeg4ChannelConfig() const
{
    return (m_header2 >> 0x2E) & 0x7u;
}

/*!
 * \brief Returns the size of the frame (including the header) in bytes.
 */
constexpr uint16 AdtsFrame::totalSize() const
{
    return (m_header2 >> 0x1D) & 0x1FFFu;
}

/*!
 * \brief Retruns the header size in bytes (9 if CRC is present; otherwise 7).
 */
constexpr byte AdtsFrame::headerSize() const
{
    return hasCrc() ? 9 : 7;
}

/*!
 * \brief Returns the data size (total size minus header size) in bytes.
 */
constexpr uint16 AdtsFrame::dataSize() const
{
    return totalSize() - headerSize();
}

/*!
 * \brief Returns the buffer fullness.
 */
constexpr uint16 AdtsFrame::bufferFullness() const
{
    return (m_header2 >> 0x12) & 0x7FFu;
}

/*!
 * \brief Returns the number of AAC frames (RDBs) in the ADTS frame.
 */
constexpr byte AdtsFrame::frameCount() const
{
    return ((m_header2 >> 0x10) & 0x3u) + 0x1u;
}

/*!
 * \brief Returns the CRC-16 checksum of the frame.
 * \sa hasCrc()
 */
constexpr uint16 AdtsFrame::crc() const
{
    return m_header2 & 0xFFFFu;
}

} // namespace TagParser

#endif // TAG_PARSER_ADTSFRAME_H
