#ifndef MEDIA_FLACMETADATAHEADER_H
#define MEDIA_FLACMETADATAHEADER_H

#include "../global.h"

#include <c++utilities/conversion/types.h>

#include <iostream>

namespace Media {

class TagValue;

/*!
 * \brief The FlacMetaDataBlockType enum specifies the type of FlacMetaDataBlockHeader.
 */
enum class FlacMetaDataBlockType : byte
{
    StreamInfo = 0,
    Padding,
    Application,
    SeekTable,
    VorbisComment,
    CuseSheet,
    Picture
};

constexpr bool operator ==(byte lhs, FlacMetaDataBlockType type)
{
    return lhs == static_cast<byte>(type);
}

constexpr bool operator !=(byte lhs, FlacMetaDataBlockType type)
{
    return lhs != static_cast<byte>(type);
}

class TAG_PARSER_EXPORT FlacMetaDataBlockHeader
{
public:
    FlacMetaDataBlockHeader();

    void parseHeader(const char *buffer);
    void makeHeader(std::ostream &outputStream);

    byte isLast() const;
    void setLast(byte last);
    byte type() const;
    void setType(FlacMetaDataBlockType type);
    uint32 dataSize() const;
    void setDataSize(uint32 dataSize);

private:
    byte m_last;
    byte m_type;
    uint32 m_dataSize;
};

/*!
 * \brief Constructs a new FLAC "METADATA_BLOCK_HEADER".
 */
inline FlacMetaDataBlockHeader::FlacMetaDataBlockHeader() :
    m_last(0),
    m_type(0),
    m_dataSize(0)
{}

/*!
 * \brief Returns whether this is the last metadata block before the audio blocks.
 * \remarks The default value is 0/false.
 */
inline byte FlacMetaDataBlockHeader::isLast() const
{
    return m_last;
}

/*!
 * \brief Sets whether this is the last metadata block before the audio blocks.
 */
inline void FlacMetaDataBlockHeader::setLast(byte last)
{
    m_last = last;
}

/*!
 * \brief Returns the block type.
 * \sa FlacMetaDataBlockType
 */
inline byte FlacMetaDataBlockHeader::type() const
{
    return m_type;
}

/*!
 * \brief Sets the block type.
 */
inline void FlacMetaDataBlockHeader::setType(FlacMetaDataBlockType type)
{
    m_type = static_cast<byte>(type);
}

/*!
 * \brief Returns the length in bytes of the meta data (excluding the size of the header itself).
 */
inline uint32 FlacMetaDataBlockHeader::dataSize() const
{
    return m_dataSize;
}

/*!
 * \brief Sets the length in bytes of the meta data (excluding the size of the header itself).
 * \remarks Max value is (2^24 - 1).
 */
inline void FlacMetaDataBlockHeader::setDataSize(uint32 dataSize)
{
    m_dataSize = dataSize;
}

class TAG_PARSER_EXPORT FlacMetaDataBlockStreamInfo
{
public:
    FlacMetaDataBlockStreamInfo();

    void parse(const char *buffer);

    uint16 minBlockSize() const;
    uint16 maxBlockSize() const;
    uint32 minFrameSize() const;
    uint32 maxFrameSize() const;
    uint32 samplingFrequency() const;
    byte channelCount() const;
    byte bitsPerSample() const;
    uint64 totalSampleCount() const;
    const char *md5Sum() const;

private:
    uint16 m_minBlockSize;
    uint16 m_maxBlockSize;
    uint32 m_minFrameSize;
    uint32 m_maxFrameSize;
    uint32 m_samplingFrequency;
    byte m_channelCount;
    byte m_bitsPerSample;
    uint64 m_totalSampleCount;
    char m_md5Sum[16];
};

/*!
 * \brief Constructs a new FLAC "METADATA_BLOCK_STREAMINFO".
 */
inline FlacMetaDataBlockStreamInfo::FlacMetaDataBlockStreamInfo() :
    m_minBlockSize(0),
    m_maxBlockSize(0),
    m_minFrameSize(0),
    m_maxFrameSize(0),
    m_samplingFrequency(0),
    m_channelCount(0),
    m_bitsPerSample(0),
    m_totalSampleCount(0),
    m_md5Sum{0}
{}

/*!
 * \brief Returns the minimum block size (in samples) used in the stream.
 */
inline uint16 FlacMetaDataBlockStreamInfo::minBlockSize() const
{
    return m_minBlockSize;
}

/*!
 * \brief Returns the maximum block size (in samples) used in the stream.
 *
 * (Minimum blocksize == maximum blocksize) implies a fixed-blocksize stream.
 */
inline uint16 FlacMetaDataBlockStreamInfo::maxBlockSize() const
{
    return m_maxBlockSize;
}

/*!
 * \brief Returns the minimum frame size (in bytes) used in the stream.
 *
 * May be 0 to imply the value is not known.
 */
inline uint32 FlacMetaDataBlockStreamInfo::minFrameSize() const
{
    return m_minFrameSize;
}

/*!
 * \brief The maximum frame size (in bytes) used in the stream.
 *
 * May be 0 to imply the value is not known.
 */
inline uint32 FlacMetaDataBlockStreamInfo::maxFrameSize() const
{
    return m_maxFrameSize;
}

/*!
 * \brief Returns the sampling frequency in Hz.
 *
 * Though 20 bits are available, the maximum sample rate is limited by the
 * structure of frame headers to 655350Hz. Also, a value of 0 is invalid.
 */
inline uint32 FlacMetaDataBlockStreamInfo::samplingFrequency() const
{
    return m_samplingFrequency;
}

/*!
 * \brief Returns the number of channels.
 *
 * FLAC supports from 1 to 8 channels .
 */
inline byte FlacMetaDataBlockStreamInfo::channelCount() const
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
inline byte FlacMetaDataBlockStreamInfo::bitsPerSample() const
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
inline uint64 FlacMetaDataBlockStreamInfo::totalSampleCount() const
{
    return m_totalSampleCount;
}

/*!
 * \brief Returns the MD5 signature of the unencoded audio data.
 *
 * This allows the decoder to determine if an error exists in the
 * audio data even when the error does not result in an invalid bitstream.
 */
inline const char *FlacMetaDataBlockStreamInfo::md5Sum() const
{
    return m_md5Sum;
}

class TAG_PARSER_EXPORT FlacMetaDataBlockPicture
{
public:
    FlacMetaDataBlockPicture(TagValue &tagValue);

    void parse(std::istream &inputStream, uint32 maxSize);
    uint32 requiredSize() const;
    void make(std::ostream &outputStream);

    uint32 pictureType() const;
    void setPictureType(uint32 pictureType);
    TagValue &value();

private:
    uint32 m_pictureType;
    TagValue &m_value;
};

/*!
 * \brief Constructs a new FLAC "METADATA_BLOCK_PICTURE".
 *
 * The picture is read from/stored to the specified \a tagValue.
 * The FlacMetaDataBlockPicture does not take ownership over
 * the specified \a tagValue.
 */
inline FlacMetaDataBlockPicture::FlacMetaDataBlockPicture(TagValue &tagValue) :
    m_pictureType(0),
    m_value(tagValue)
{}

/*!
 * \brief Returns the picture type according to the ID3v2 APIC frame.
 */
inline uint32 FlacMetaDataBlockPicture::pictureType() const
{
    return m_pictureType;
}

/*!
 * \brief Sets the picture type according to the ID3v2 APIC frame.
 */
inline void FlacMetaDataBlockPicture::setPictureType(uint32 pictureType)
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

}

#endif // MEDIA_FLACMETADATAHEADER_H
