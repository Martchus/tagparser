#ifndef ABSTRACTTRACK_H
#define ABSTRACTTRACK_H

#include "./statusprovider.h"
#include "./size.h"
#include "./margin.h"
#include "./aspectratio.h"
#include "./mediaformat.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/timespan.h>

#include <iosfwd>
#include <string>

namespace Media {

class MpegAudioFrameStream;
class WaveAudioStream;
class Mp4Track;

/*!
 * \brief Specifies the track type.
 */
enum class TrackType
{
    Unspecified, /**< The track type is not specified. */
    MatroskaTrack, /**< The track is a Media::MatroskaTrack. */
    MpegAudioFrameStream, /**< The track is a Media::MpegAudioFrameStream. */
    Mp4Track, /**< The track is a Media::Mp4Track. */
    WaveAudioStream, /**< The track is a Media::WaveAudioStream. */
    OggStream, /**< The track is a Media::OggStream. */
    AdtsStream, /**< The track is a Media::AdtsStream. */
    FlacStream, /**< The track is a Media::FlacStream. */
};

class TAG_PARSER_EXPORT AbstractTrack : public StatusProvider
{
    friend class MpegAudioFrameStream;
    friend class WaveAudioStream;
    friend class Mp4Track;

public:
    virtual ~AbstractTrack();

    virtual TrackType type() const;
    std::istream &inputStream();
    void setInputStream(std::istream &stream);
    std::ostream &outputStream();
    void setOutputStream(std::ostream &stream);
    IoUtilities::BinaryReader &reader();
    IoUtilities::BinaryWriter &writer();
    uint64 startOffset() const;
    MediaFormat format() const;
    double version() const;
    const char *formatName() const;
    const char *formatAbbreviation() const;
    const std::string &formatId() const;
    MediaType mediaType() const;
    const char *mediaTypeName() const;
    uint64 size() const;
    uint32 trackNumber() const;
    uint64 id() const;
    const std::string name() const;
    const ChronoUtilities::TimeSpan &duration() const;
    double bitrate() const;
    double maxBitrate() const;
    const ChronoUtilities::DateTime &creationTime() const;
    const ChronoUtilities::DateTime &modificationTime() const;
    const std::string &language() const;
    uint32 samplingFrequency() const;
    uint32 extensionSamplingFrequency() const;
    uint16 bitsPerSample() const;
    uint16 channelCount() const;
    byte channelConfig() const;
    const char *channelConfigString() const;
    byte extensionChannelConfig() const;
    const char *extensionChannelConfigString() const;
    uint64 sampleCount() const;
    int quality() const;
    const Size &pixelSize() const;
    const Size &displaySize() const;
    const Size &resolution() const;
    const std::string &compressorName() const;
    uint16 depth() const;
    uint32 fps() const;
    const char *chromaFormat() const;
    const AspectRatio &pixelAspectRatio() const;
    bool isInterlaced() const;
    uint32 timeScale() const;
    bool isEnabled() const;
    bool isDefault() const;
    bool isForced() const;
    bool hasLacing() const;
    bool isEncrypted() const;
    uint32 colorSpace() const;
    const Margin &cropping() const;
    std::string label() const;

    void parseHeader();
    bool isHeaderValid() const;

protected:
    AbstractTrack(std::istream &inputStream, std::ostream &outputStream, uint64 startOffset);
    AbstractTrack(std::iostream &stream, uint64 startOffset);
    virtual void internalParseHeader() = 0;

    std::istream *m_istream;
    std::ostream *m_ostream;
    IoUtilities::BinaryReader m_reader;
    IoUtilities::BinaryWriter m_writer;
    uint64 m_startOffset;
    bool m_headerValid;
    MediaFormat m_format;
    std::string m_formatId;
    std::string m_formatName;
    MediaType m_mediaType;
    double m_version;
    uint64 m_size;
    uint32 m_trackNumber;
    uint64 m_id;
    std::string m_name;
    ChronoUtilities::TimeSpan m_duration;
    double m_bitrate;
    double m_maxBitrate;
    ChronoUtilities::DateTime m_creationTime;
    ChronoUtilities::DateTime m_modificationTime;
    std::string m_language;
    uint32 m_samplingFrequency;
    uint32 m_extensionSamplingFrequency;
    uint16 m_bitsPerSample;
    uint32 m_bytesPerSecond;
    uint16 m_channelCount;
    byte m_channelConfig;
    byte m_extensionChannelConfig;
    uint16 m_chunkSize;
    uint64 m_sampleCount;
    int m_quality;
    Size m_pixelSize;
    Size m_displaySize;
    Size m_resolution;
    std::string m_compressorName;
    uint16 m_depth;
    uint32 m_fps;
    const char *m_chromaFormat;
    AspectRatio m_pixelAspectRatio;
    bool m_interlaced;
    uint32 m_timeScale;
    bool m_enabled;
    bool m_default;
    bool m_forced;
    bool m_lacing;
    bool m_encrypted;
    bool m_usedInPresentation;
    bool m_usedWhenPreviewing;
    uint32 m_colorSpace;
    Margin m_cropping;
};

/*!
 * \brief Returns the associated input stream.
 */
inline std::istream &AbstractTrack::inputStream()
{
    return *m_istream;
}

/*!
 * \brief Assigns an other input stream.
 *
 * The track will read from the \a stream to perform
 * particular operations such as reading header information.
 */
inline void AbstractTrack::setInputStream(std::istream &stream)
{
    m_istream = &stream;
    m_reader.setStream(m_istream);
}

/*!
 * \brief Returns the associated input stream.
 */
inline std::ostream &AbstractTrack::outputStream()
{
    return *m_ostream;
}

/*!
 * \brief Assigns an other output stream.
 *
 * The track will write to the \a stream to perform
 * particular operations such as updating or making header information.
 */
inline void AbstractTrack::setOutputStream(std::ostream &stream)
{
    m_ostream = &stream;
    m_writer.setStream(m_ostream);
}

/*!
 * \brief Returns a binary reader for the associated stream.
 *
 * \remarks The returned reader is internally used when reading binary data
 *          from the associated stream. Do not change its byte order.
 */
inline IoUtilities::BinaryReader &AbstractTrack::reader()
{
    return m_reader;
}

/*!
 * \brief Returns a binary writer for the associated stream.
 *
 * \remarks The returned writer is internally used when writing binary data
 *          to the associated stream. Do not change its byte order.
 */
inline IoUtilities::BinaryWriter &AbstractTrack::writer()
{
    return m_writer;
}

/*!
 * \brief Returns the type of the track if known; otherwise returns TrackType::Unspecified.
 */
inline TrackType AbstractTrack::type() const
{
    return TrackType::Unspecified;
}

/*!
 * \brief Returns the start offset of the track in the associated stream.
 */
inline uint64 AbstractTrack::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the format of the track if known; otherwise returns MediaFormat::Unknown.
 */
inline MediaFormat AbstractTrack::format() const
{
    return m_format;
}

/*!
 * \brief Returns the version/level of the track if known; otherwise returns 0.
 */
inline double AbstractTrack::version() const
{
    return m_version;
}

/*!
 * \brief Returns the format of the track as C-style string if known; otherwise
 *        returns the format abbreviation or an empty string.
 * \remarks
 *  - The caller must not free the returned string.
 *  - The string might get invalidated when the track is (re)parsed.
 */
inline const char *AbstractTrack::formatName() const
{
    return m_format || m_formatName.empty() ? m_format.name() : m_formatName.c_str();
}

/*!
 * \brief Returns the a more or less common abbreviation for the format of the track
 *        as C-style string if known; otherwise returns an empty string.
 */
inline const char *AbstractTrack::formatAbbreviation() const
{
    const char *abbr = m_format.abbreviation();
    return *abbr || m_formatId.empty() ? m_format.abbreviation() : m_formatId.c_str();
}

/*!
 * \brief Returns the format/codec ID. This is usually the raw format identifier
 *        extracted from the container) if known; otherwise returns an empty string.
 */
inline const std::string &AbstractTrack::formatId() const
{
    return m_formatId;
}

/*!
 * \brief Returns the media type if known; otherwise returns MediaType::Other.
 */
inline MediaType AbstractTrack::mediaType() const
{
    return m_mediaType;
}

/*!
 * \brief Returns the string representation of the media type of the track.
 */
inline const char *AbstractTrack::mediaTypeName() const
{
    return ::Media::mediaTypeName(m_mediaType);
}

/*!
 * \brief Returns the size in bytes if known; otherwise returns 0.
 */
inline uint64 AbstractTrack::size() const
{
    return m_size;
}

/*!
 * \brief Returns the track number if known; otherwise returns 0.
 */
inline uint32 AbstractTrack::trackNumber() const
{
    return m_trackNumber;
}

/*!
 * \brief Returns the track ID if known; otherwise returns 0.
 */
inline uint64 AbstractTrack::id() const
{
    return m_id;
}

/*!
 * \brief Returns the track name if known; otherwise returns an empty string.
 */
inline const std::string AbstractTrack::name() const
{
    return m_name;
}

/*!
 * \brief Returns the duration if known; otherwise returns a TimeSpan of zero ticks.
 */
inline const ChronoUtilities::TimeSpan &AbstractTrack::duration() const
{
    return m_duration;
}

/*!
 * \brief Returns the average bitrate in kbit/s if known; otherwise returns zero.
 */
inline double AbstractTrack::bitrate() const
{
    return m_bitrate;
}

/*!
 * \brief Returns the maximum bitrate in kbit/s if known; otherwise returns zero.
 */
inline double AbstractTrack::maxBitrate() const
{
    return m_maxBitrate;
}

/*!
 * \brief Returns the creation time if known; otherwise returns a DateTime of zero ticks.
 */
inline const ChronoUtilities::DateTime &AbstractTrack::creationTime() const
{
    return m_creationTime;
}

/*!
 * \brief Returns the time of the last modification if known; otherwise returns a DateTime of zero ticks.
 */
inline const ChronoUtilities::DateTime &AbstractTrack::modificationTime() const
{
    return m_modificationTime;
}

/*!
 * \brief Returns the language of the track if known; otherwise returns an empty string.
 *
 * The format of the language denotation depends on the particular implementation.
 */
inline const std::string &AbstractTrack::language() const
{
    return m_language;
}

/*!
 * \brief Returns the number of samples per second if known; otherwise returns 0.
 */
inline uint32 AbstractTrack::samplingFrequency() const
{
    return m_samplingFrequency;
}

/*!
 * \brief Returns the number of samples per second if known; otherwise returns 0.
 * \remarks This sample rate value takes extensions like SBR into account.
 */
inline uint32 AbstractTrack::extensionSamplingFrequency() const
{
    return m_extensionSamplingFrequency;
}

/*!
 * \brief Returns the number of bits per sample; otherwise returns 0.
 */
inline uint16 AbstractTrack::bitsPerSample() const
{
    return m_bitsPerSample;
}

/*!
 * \brief Returns the number of channels if known; otherwise returns 0.
 *
 * This value only makes sense for audio tracks.
 */
inline uint16 AbstractTrack::channelCount() const
{
    return m_channelCount;
}

/*!
 * \brief Returns the channel configuration.
 *
 * This is the MPEG-4 channel config for MPEG-4 audio.
 * \sa Mpeg4ChannelConfigs::channelConfigString()
 */
inline byte AbstractTrack::channelConfig() const
{
    return m_channelConfig;
}

/*!
 * \brief Returns the number of samples if known; otherwise returns 0.
 */
inline uint64 AbstractTrack::sampleCount() const
{
    return m_sampleCount;
}

/*!
 * \brief Returns the quality if known; otherwise returns 0.
 *
 * The scale depends on the format.
 */
inline int AbstractTrack::quality() const
{
    return m_quality;
}

/*!
 * \brief Returns the size of the encoded video frames if known; otherwise returns a zero size.
 *
 * This value only makes sense for video tracks.
 */
inline const Size &AbstractTrack::pixelSize() const
{
    return m_pixelSize;
}

/*!
 * \brief Returns the size of the video frames to display if known; otherwise returns a zero size.
 *
 * This value only makes sense for video tracks.
 */
inline const Size &AbstractTrack::displaySize() const
{
    return m_displaySize;
}

/*!
 * \brief Returns the resolution if known; otherwise returns a zero size.
 *
 * This value only makes sense for video tracks.
 */
inline const Size &AbstractTrack::resolution() const
{
    return m_resolution;
}

/*!
 * \brief Returns the compressor name if known; otherwise returns an empty string.
 */
inline const std::string &AbstractTrack::compressorName() const
{
    return m_compressorName;
}

/*!
 * \brief Returns the bit depth if known; otherwise returns 0.
 */
inline uint16 AbstractTrack::depth() const
{
    return m_depth;
}

/*!
 * \brief Returns the number of frames per second if known; otherwise returns 0.
 *
 * This value only makes sense for video tracks.
 */
inline uint32 AbstractTrack::fps() const
{
    return m_fps;
}

/*!
 * \brief Returns the chroma subsampling format if known; otherwise returns nullptr.
 *
 * This value only makes sense for video tracks.
 */
inline const char *AbstractTrack::chromaFormat() const
{
    return m_chromaFormat;
}

/*!
 * \brief Returns the pixel aspect ratio (PAR).
 */
inline const AspectRatio &AbstractTrack::pixelAspectRatio() const
{
    return m_pixelAspectRatio;
}

/*!
 * \brief Returns true if the video is denoted as interlaced; otherwise returns false.
 *
 * This value only makes sense for video tracks.
 */
inline bool AbstractTrack::isInterlaced() const
{
    return m_interlaced;
}

/*!
 * \brief Returns the time scale if known; otherwise returns 0.
 *
 * The time scale depends on the format.
 */
inline uint32 AbstractTrack::timeScale() const
{
    return m_timeScale;
}

/*!
 * \brief Returns true if the track is denoted as enabled; otherwise returns false.
 */
inline bool AbstractTrack::isEnabled() const
{
    return m_enabled;
}

/*!
 * \brief Returns true if the track is denoted as default; otherwise returns false.
 */
inline bool AbstractTrack::isDefault() const
{
    return m_default;
}

/*!
 * \brief Returns true if the track is denoted as forced; otherwise returns false.
 */
inline bool AbstractTrack::isForced() const
{
    return m_forced;
}

/*!
 * \brief Returns true if the track has lacing; otherwise returns false.
 */
inline bool AbstractTrack::hasLacing() const
{
    return m_lacing;
}

/*!
 * \brief Returns true if the track is denoted as encrypted; otherwise returns false.
 */
inline bool AbstractTrack::isEncrypted() const
{
    return m_encrypted;
}

/*!
 * \brief Returns the color space if known; otherwise returns 0.
 */
inline uint32 AbstractTrack::colorSpace() const
{
    return m_colorSpace;
}

/*!
 * \brief Returns the cropping if known; otherwise returns zero margins.
 */
inline const Margin &AbstractTrack::cropping() const
{
    return m_cropping;
}

/*!
 * \brief Returns an indication whether the track header is valid.
 */
inline bool AbstractTrack::isHeaderValid() const
{
    return m_headerValid;
}

}

#endif // ABSTRACTTRACK_H
