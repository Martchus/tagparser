#ifndef TAG_PARSER_MP3FRAMEAUDIOSTREAM_H
#define TAG_PARSER_MP3FRAMEAUDIOSTREAM_H

#include "../diagnostics.h"

#include <c++utilities/conversion/types.h>

#include <iostream>

namespace IoUtilities {
class BinaryReader;
}

namespace TagParser {

/*!
 * \brief Specifies the channel mode.
 */
enum class MpegChannelMode {
    Stereo, /**< stereo */
    JointStereo, /**< joint stereo */
    DualChannel, /**< dual channel */
    SingleChannel, /**< single channel/mono */
    Unspecifed /**< used to indicate that the channel mode is unknown */
};

TAG_PARSER_EXPORT const char *mpegChannelModeString(MpegChannelMode channelMode);

enum class XingHeaderFlags {
    None = 0x0u, /**< No Xing frames are present  */
    HasFramesField = 0x1u, /**< Xing frames field is present */
    HasBytesField = 0x2u, /**< Xing bytes field is present */
    HasTocField = 0x4u, /**< Xing TOC field is present */
    HasQualityIndicator = 0x8u /**< Xing quality indicator is present */
};

class TAG_PARSER_EXPORT MpegAudioFrame {
public:
    constexpr MpegAudioFrame();

    void parseHeader(IoUtilities::BinaryReader &reader, Diagnostics &diag);

    constexpr bool isValid() const;
    double mpegVersion() const;
    int layer() const;
    constexpr bool isProtectedByCrc() const;
    uint16 bitrate() const;
    uint32 samplingFrequency() const;
    constexpr uint32 paddingSize() const;
    MpegChannelMode channelMode() const;
    constexpr bool hasCopyright() const;
    constexpr bool isOriginal() const;
    uint32 sampleCount() const;
    uint32 size() const;
    constexpr bool isXingHeaderAvailable() const;
    constexpr XingHeaderFlags xingHeaderFlags() const;
    constexpr bool isXingFramefieldPresent() const;
    constexpr bool isXingBytesfieldPresent() const;
    constexpr bool isXingTocFieldPresent() const;
    constexpr bool isXingQualityIndicatorFieldPresent() const;
    constexpr uint32 xingFrameCount() const;
    constexpr uint32 xingBytesfield() const;
    constexpr uint32 xingQualityIndicator() const;

private:
    static constexpr uint64 s_xingHeaderOffset = 0x24;
    static constexpr uint32 s_sync = 0xFFE00000u;
    static const uint16 s_bitrateTable[0x2][0x3][0xF];
    uint32 m_header;
    uint64 m_xingHeader;
    XingHeaderFlags m_xingHeaderFlags;
    uint32 m_xingFramefield;
    uint32 m_xingBytesfield;
    uint32 m_xingQualityIndicator;
};

/*!
 * \brief Constructs a new frame.
 */
constexpr MpegAudioFrame::MpegAudioFrame()
    : m_header(0)
    , m_xingHeader(0)
    , m_xingHeaderFlags(XingHeaderFlags::None)
    , m_xingFramefield(0)
    , m_xingBytesfield(0)
    , m_xingQualityIndicator(0)
{
}

/*!
 * \brief Returns an indication whether the frame is valid.
 */
constexpr bool MpegAudioFrame::isValid() const
{
    return (m_header & s_sync) == s_sync;
}

/*!
 * \brief Returns an indication whether the frame is protected by CRC.
 */
constexpr bool MpegAudioFrame::isProtectedByCrc() const
{
    return (m_header & 0x10000u) != 0x10000u;
}

/*!
 * \brief Returns the bitrate of the frame if known; otherwise returns 0.
 */
inline uint16 MpegAudioFrame::bitrate() const
{
    if (mpegVersion() > 0.0 && layer() > 0) {
        return s_bitrateTable[mpegVersion() == 1.0 ? 0 : 1][layer() - 1][(m_header & 0xf000u) >> 12];
    } else {
        return 0;
    }
}

/*!
 * \brief Returns the padding size if known; otherwise returns 0.
 */
constexpr uint32 MpegAudioFrame::paddingSize() const
{
    if (isValid()) {
        return (m_header & 0x60000u) == 0x60000u ? 4u : 1u * (m_header & 0x200u);
    } else {
        return 0;
    }
}

/*!
 * \brief Returns an indication whether the frame is copyrighted.
 */
constexpr bool MpegAudioFrame::hasCopyright() const
{
    return (m_header & 0x8u) == 0x8u;
}

/*!
 * \brief Returns an indication whether the frame labeled as original.
 */
constexpr bool MpegAudioFrame::isOriginal() const
{
    return (m_header & 0x4u) == 0x4u;
}

constexpr XingHeaderFlags operator|(XingHeaderFlags lhs, XingHeaderFlags rhs)
{
    return static_cast<XingHeaderFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

constexpr XingHeaderFlags operator&(XingHeaderFlags lhs, XingHeaderFlags rhs)
{
    return static_cast<XingHeaderFlags>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

/*!
 * \brief Returns an indication whether a Xing header is present.
 */
constexpr bool MpegAudioFrame::isXingHeaderAvailable() const
{
    return ((m_xingHeader & 0x58696e6700000000uL) == 0x58696e6700000000uL) || ((m_xingHeader & 0x496e666f00000000uL) == 0x496e666f00000000uL);
}

/*!
 * \brief Returns the Xing header flags.
 */
constexpr XingHeaderFlags MpegAudioFrame::xingHeaderFlags() const
{
    return m_xingHeaderFlags;
}

/*!
 * \brief Returns an indication whether the Xing frame field is present.
 */
constexpr bool MpegAudioFrame::isXingFramefieldPresent() const
{
    return (isXingHeaderAvailable()) ? ((m_xingHeaderFlags & XingHeaderFlags::HasFramesField) == XingHeaderFlags::HasFramesField) : false;
}

/*!
 * \brief Returns an indication whether the Xing bytes field is present.
 */
constexpr bool MpegAudioFrame::isXingBytesfieldPresent() const
{
    return (isXingHeaderAvailable()) ? ((m_xingHeaderFlags & XingHeaderFlags::HasFramesField) == XingHeaderFlags::HasFramesField) : false;
}

/*!
 * \brief Returns an indication whether the Xing TOC is present.
 */
constexpr bool MpegAudioFrame::isXingTocFieldPresent() const
{
    return (isXingHeaderAvailable()) ? ((m_xingHeaderFlags & XingHeaderFlags::HasTocField) == XingHeaderFlags::HasTocField) : false;
}

/*!
 * \brief Returns an indication whether the Xing quality indicator field is present.
 */
constexpr bool MpegAudioFrame::isXingQualityIndicatorFieldPresent() const
{
    return (isXingHeaderAvailable()) ? ((m_xingHeaderFlags & XingHeaderFlags::HasQualityIndicator) == XingHeaderFlags::HasQualityIndicator) : false;
}

/*!
 * \brief Returns an indication whether the Xing frame count is present.
 */
constexpr uint32 MpegAudioFrame::xingFrameCount() const
{
    return m_xingFramefield;
}

/*!
 * \brief Returns the Xing bytes field if known; otherwise returns 0.
 */
constexpr uint32 MpegAudioFrame::xingBytesfield() const
{
    return m_xingBytesfield;
}

/*!
 * \brief Returns the Xing quality indicator if known; otherwise returns 0.
 */
constexpr uint32 MpegAudioFrame::xingQualityIndicator() const
{
    return m_xingQualityIndicator;
}

} // namespace TagParser

#endif // TAG_PARSER_MP3FRAMEAUDIOSTREAM_H
