#ifndef MEDIA_ADTSFRAME_H
#define MEDIA_ADTSFRAME_H

#include <c++utilities/conversion/types.h>
#include <c++utilities/application/global.h>

namespace IoUtilities {
class BinaryReader;
}

namespace Media {

class LIB_EXPORT AdtsFrame
{
public:
    AdtsFrame();

    void parseHeader(IoUtilities::BinaryReader &reader);

    bool isValid() const;
    bool isMpeg4() const;
    bool hasCrc() const;
    byte mpeg4AudioObjectId() const;
    byte mpeg4SamplingFrequencyIndex() const;
    byte mpeg4ChannelConfig() const;
    uint16 totalSize() const;
    byte headerSize() const;
    uint16 dataSize() const;
    uint16 bufferFullness() const;
    byte frameCount() const;
    uint16 crc() const;

private:
    uint16 m_header1;
    uint64 m_header2;
};

/*!
 * \brief Constructs a new frame.
 */
inline AdtsFrame::AdtsFrame() :
    m_header1(0)
{}

/*!
 * \brief Returns an indication whether the frame is valid.
 */
inline bool AdtsFrame::isValid() const
{
    return ((m_header1 & 0xFFF6u) == 0xFFF0u) && (totalSize() >= headerSize());
}

/*!
 * \brief Returns whether the MPEG version is MPEG-4; otherwise the MPEG version is MPEG-2.
 */
inline bool AdtsFrame::isMpeg4() const
{
    return m_header1 & 0x8u;
}

/*!
 * \brief Returns whether a CRC-16 checksum is present ("protection absent" bit is NOT set).
 */
inline bool AdtsFrame::hasCrc() const
{
    return (m_header1 & 0x1u) == 0;
}

/*!
 * \brief Returns the MPEG-4 audio object type ID.
 * \sa Media::Mpeg4AudioObjectIds
 * \sa Mpeg4AudioObjectIds::idToMediaFormat()
 */
inline byte AdtsFrame::mpeg4AudioObjectId() const
{
    return (m_header2 >> 0x36) + 0x1u;
}

/*!
 * \brief Returns the MPEG-4 sample rate index.
 * \sa Media::mpeg4SampleRateTable
 */
inline byte AdtsFrame::mpeg4SamplingFrequencyIndex() const
{
    return (m_header2 >> 0x32) & 0xFu;
}

/*!
 * \brief Returns the MPEG-4 channel configuration.
 * \sa Media::Mpeg4ChannelConfigs
 * \sa Media::mpeg4SampleRateTable::channelConfigString()
 */
inline byte AdtsFrame::mpeg4ChannelConfig() const
{
    return (m_header2 >> 0x2E) & 0x7u;
}

/*!
 * \brief Returns the size of the frame (including the header) in bytes.
 */
inline uint16 AdtsFrame::totalSize() const
{
    return (m_header2 >> 0x1D) & 0x1FFFu;
}

/*!
 * \brief Retruns the header size in bytes (9 if CRC is present; otherwise 7).
 */
inline byte AdtsFrame::headerSize() const
{
    return hasCrc() ? 9 : 7;
}

/*!
 * \brief Returns the data size (total size minus header size) in bytes.
 */
inline uint16 AdtsFrame::dataSize() const
{
    return totalSize() - headerSize();
}

/*!
 * \brief Returns the buffer fullness.
 */
inline uint16 AdtsFrame::bufferFullness() const
{
    return (m_header2 >> 0x12) & 0x7FFu;
}

/*!
 * \brief Returns the number of AAC frames (RDBs) in the ADTS frame.
 */
inline byte AdtsFrame::frameCount() const
{
    return ((m_header2 >> 0x10) & 0x3u) + 0x1u;
}

/*!
 * \brief Returns the CRC-16 checksum of the frame.
 * \sa hasCrc()
 */
inline uint16 AdtsFrame::crc() const
{
    return m_header2 & 0xFFFFu;
}


} // namespace Media

#endif // MEDIA_ADTSFRAME_H
