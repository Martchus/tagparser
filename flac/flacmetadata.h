#ifndef TAG_PARSER_FLACMETADATAHEADER_H
#define TAG_PARSER_FLACMETADATAHEADER_H

#include "../global.h"

#include <cstdint>
#include <iostream>

namespace TagParser {

class TagValue;

/*!
 * \brief The FlacMetaDataBlockType enum specifies the type of FlacMetaDataBlockHeader.
 */
enum class FlacMetaDataBlockType : std::uint8_t { StreamInfo = 0, Padding, Application, SeekTable, VorbisComment, CuseSheet, Picture };

constexpr bool operator==(std::uint8_t lhs, FlacMetaDataBlockType type)
{
    return lhs == static_cast<std::uint8_t>(type);
}

constexpr bool operator!=(std::uint8_t lhs, FlacMetaDataBlockType type)
{
    return lhs != static_cast<std::uint8_t>(type);
}

class TAG_PARSER_EXPORT FlacMetaDataBlockHeader {
public:
    constexpr FlacMetaDataBlockHeader();

    void parseHeader(std::string_view buffer);
    void makeHeader(std::ostream &outputStream);

    constexpr std::uint8_t isLast() const;
    void setLast(std::uint8_t last);
    constexpr std::uint8_t type() const;
    void setType(FlacMetaDataBlockType type);
    constexpr std::uint32_t dataSize() const;
    void setDataSize(std::uint32_t dataSize);

private:
    std::uint8_t m_last;
    std::uint8_t m_type;
    std::uint32_t m_dataSize;
};

/*!
 * \brief Constructs a new FLAC "METADATA_BLOCK_HEADER".
 */
constexpr FlacMetaDataBlockHeader::FlacMetaDataBlockHeader()
    : m_last(0)
    , m_type(0)
    , m_dataSize(0)
{
}

/*!
 * \brief Returns whether this is the last metadata block before the audio blocks.
 * \remarks The default value is 0/false.
 */
constexpr std::uint8_t FlacMetaDataBlockHeader::isLast() const
{
    return m_last;
}

/*!
 * \brief Sets whether this is the last metadata block before the audio blocks.
 */
inline void FlacMetaDataBlockHeader::setLast(std::uint8_t last)
{
    m_last = last;
}

/*!
 * \brief Returns the block type.
 * \sa FlacMetaDataBlockType
 */
constexpr std::uint8_t FlacMetaDataBlockHeader::type() const
{
    return m_type;
}

/*!
 * \brief Sets the block type.
 */
inline void FlacMetaDataBlockHeader::setType(FlacMetaDataBlockType type)
{
    m_type = static_cast<std::uint8_t>(type);
}

/*!
 * \brief Returns the length in bytes of the meta data (excluding the size of the header itself).
 */
constexpr std::uint32_t FlacMetaDataBlockHeader::dataSize() const
{
    return m_dataSize;
}

/*!
 * \brief Sets the length in bytes of the meta data (excluding the size of the header itself).
 * \remarks Max value is (2^24 - 1).
 */
inline void FlacMetaDataBlockHeader::setDataSize(std::uint32_t dataSize)
{
    m_dataSize = dataSize;
}

class TAG_PARSER_EXPORT FlacMetaDataBlockStreamInfo {
public:
    constexpr FlacMetaDataBlockStreamInfo();

    void parse(std::string_view buffer);

    constexpr std::uint16_t minBlockSize() const;
    constexpr std::uint16_t maxBlockSize() const;
    constexpr std::uint32_t minFrameSize() const;
    constexpr std::uint32_t maxFrameSize() const;
    constexpr std::uint32_t samplingFrequency() const;
    constexpr std::uint8_t channelCount() const;
    constexpr std::uint8_t bitsPerSample() const;
    constexpr std::uint64_t totalSampleCount() const;
    constexpr const char *md5Sum() const;

private:
    std::uint16_t m_minBlockSize;
    std::uint16_t m_maxBlockSize;
    std::uint32_t m_minFrameSize;
    std::uint32_t m_maxFrameSize;
    std::uint32_t m_samplingFrequency;
    std::uint8_t m_channelCount;
    std::uint8_t m_bitsPerSample;
    std::uint64_t m_totalSampleCount;
    char m_md5Sum[16];
};

/*!
 * \brief Constructs a new FLAC "METADATA_BLOCK_STREAMINFO".
 */
constexpr FlacMetaDataBlockStreamInfo::FlacMetaDataBlockStreamInfo()
    : m_minBlockSize(0)
    , m_maxBlockSize(0)
    , m_minFrameSize(0)
    , m_maxFrameSize(0)
    , m_samplingFrequency(0)
    , m_channelCount(0)
    , m_bitsPerSample(0)
    , m_totalSampleCount(0)
    , m_md5Sum{ 0 }
{
}

/*!
 * \brief Returns the minimum block size (in samples) used in the stream.
 */
constexpr std::uint16_t FlacMetaDataBlockStreamInfo::minBlockSize() const
{
    return m_minBlockSize;
}

/*!
 * \brief Returns the maximum block size (in samples) used in the stream.
 *
 * (Minimum blocksize == maximum blocksize) implies a fixed-blocksize stream.
 */
constexpr std::uint16_t FlacMetaDataBlockStreamInfo::maxBlockSize() const
{
    return m_maxBlockSize;
}

/*!
 * \brief Returns the minimum frame size (in bytes) used in the stream.
 *
 * May be 0 to imply the value is not known.
 */
constexpr std::uint32_t FlacMetaDataBlockStreamInfo::minFrameSize() const
{
    return m_minFrameSize;
}

/*!
 * \brief The maximum frame size (in bytes) used in the stream.
 *
 * May be 0 to imply the value is not known.
 */
constexpr std::uint32_t FlacMetaDataBlockStreamInfo::maxFrameSize() const
{
    return m_maxFrameSize;
}

/*!
 * \brief Returns the sampling frequency in Hz.
 *
 * Though 20 bits are available, the maximum sample rate is limited by the
 * structure of frame headers to 655350Hz. Also, a value of 0 is invalid.
 */
constexpr std::uint32_t FlacMetaDataBlockStreamInfo::samplingFrequency() const
{
    return m_samplingFrequency;
}

/*!
 * \brief Returns the number of channels.
 *
 * FLAC supports from 1 to 8 channels .
 */
constexpr std::uint8_t FlacMetaDataBlockStreamInfo::channelCount() const
{
    return m_channelCount;
}

/*!
 * \brief Returns the bits per sample.
 *
 * FLAC supports from 4 to 32 bits per sample.
 * Currently the reference encoder and decoders only support up
 * to 24 bits per sample.
 */
constexpr std::uint8_t FlacMetaDataBlockStreamInfo::bitsPerSample() const
{
    return m_bitsPerSample;
}

/*!
 * \brief Returns the total samples in stream.
 *
 * 'Samples' means inter-channel sample, i.e. one second of 44.1Khz audio
 * will have 44100 samples regardless of the number of channels.
 *
 * A value of zero here means the number of total samples is unknown.
 */
constexpr std::uint64_t FlacMetaDataBlockStreamInfo::totalSampleCount() const
{
    return m_totalSampleCount;
}

/*!
 * \brief Returns the MD5 signature of the unencoded audio data.
 *
 * This allows the decoder to determine if an error exists in the
 * audio data even when the error does not result in an invalid bitstream.
 */
constexpr const char *FlacMetaDataBlockStreamInfo::md5Sum() const
{
    return m_md5Sum;
}

class TAG_PARSER_EXPORT FlacMetaDataBlockPicture {
public:
    FlacMetaDataBlockPicture(TagValue &tagValue);

    void parse(std::istream &inputStream, std::uint32_t maxSize);
    std::uint32_t requiredSize() const;
    void make(std::ostream &outputStream);

    std::uint32_t pictureType() const;
    void setPictureType(std::uint32_t pictureType);
    TagValue &value();

private:
    std::uint32_t m_pictureType;
    TagValue &m_value;
};

/*!
 * \brief Constructs a new FLAC "METADATA_BLOCK_PICTURE".
 *
 * The picture is read from/stored to the specified \a tagValue.
 * The FlacMetaDataBlockPicture does not take ownership over
 * the specified \a tagValue.
 */
inline FlacMetaDataBlockPicture::FlacMetaDataBlockPicture(TagValue &tagValue)
    : m_pictureType(0)
    , m_value(tagValue)
{
}

/*!
 * \brief Returns the picture type according to the ID3v2 APIC frame.
 */
inline std::uint32_t FlacMetaDataBlockPicture::pictureType() const
{
    return m_pictureType;
}

/*!
 * \brief Sets the picture type according to the ID3v2 APIC frame.
 */
inline void FlacMetaDataBlockPicture::setPictureType(std::uint32_t pictureType)
{
    m_pictureType = pictureType;
}

/*!
 * \brief Returns the tag value the picture is read from/stored to.
 */
inline TagValue &FlacMetaDataBlockPicture::value()
{
    return m_value;
}

} // namespace TagParser

#endif // TAG_PARSER_FLACMETADATAHEADER_H
