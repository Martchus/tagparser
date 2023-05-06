#ifndef TAG_PARSER_ABSTRACTTRACK_H
#define TAG_PARSER_ABSTRACTTRACK_H

#include "./aspectratio.h"
#include "./diagnostics.h"
#include "./localehelper.h"
#include "./margin.h"
#include "./mediaformat.h"
#include "./size.h"

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/misc/flagenumclass.h>

#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>

namespace TagParser {

class AbortableProgressFeedback;
class MpegAudioFrameStream;
class WaveAudioStream;
class Mp4Track;

/*!
 * \brief The TrackType enum specifies the underlying file type of a track and the concrete class of the track object.
 */
enum class TrackType {
    Unspecified, /**< The track type is not specified. */
    MatroskaTrack, /**< The track is a TagParser::MatroskaTrack. */
    MpegAudioFrameStream, /**< The track is a TagParser::MpegAudioFrameStream. */
    Mp4Track, /**< The track is a TagParser::Mp4Track. */
    WaveAudioStream, /**< The track is a TagParser::WaveAudioStream. */
    OggStream, /**< The track is a TagParser::OggStream. */
    AdtsStream, /**< The track is a TagParser::AdtsStream. */
    FlacStream, /**< The track is a TagParser::FlacStream. */
    IvfStream, /**< The track is a TagParser::IvfStream. */
};

/*!
 * \brief The TrackFlags enum specifies miscellaneous boolean properties of a track.
 */
enum class TrackFlags : std::uint64_t {
    None = 0, /**< No flags present */
    HeaderValid = (1 << 0), /**< The track header is valid. Set by AbstractTrack::parseHeader() on success. */
    Enabled = (1 << 2), /**< The track is marked as enabled. */
    Default = (1 << 3), /**< The track is marked as default. */
    Forced = (1 << 4), /**< The track is marked as forced. */
    Lacing = (1 << 5), /**< The track has lacing. */
    Encrypted = (1 << 6), /**< The track is encrypted. */
    UsedInPresentation = (1 << 7), /**< The track is supposed to be used in presentation. */
    UsedWhenPreviewing = (1 << 8), /**< The track is supposed to be used when previewing. */
    Interlaced = (1 << 9), /**< The video is interlaced. */
};

/*!
 * \brief The FieldOrder enum declares the field ordering of the video.
 */
enum class FieldOrder : std::uint64_t {
    Progressive = 0,
    Tff = 1,
    Undetermined = 2,
    Bff = 6,
    BffSwapped = 9,
    TffSwapped = 14,
};

/*!
 * \brief The StereoMode enum specifies the Stereo-3D video mode.
 * \sa https://matroska.org/technical/notes.html#multi-planar-and-3d-videos
 */
enum class StereoMode : std::uint64_t {
    Unknown = std::numeric_limits<std::uint64_t>::max(),
};

/*!
 * \brief The AlphaMode enum specifies the alpha video mode.
 */
enum class AlphaMode : std::uint64_t {
    Unknown = std::numeric_limits<std::uint64_t>::max(),
};

/*!
 * \brief The DisplayUnit enum specifies how display width and heigh are interpreted.
 */
enum class DisplayUnit : std::uint64_t { Pixels, Centimeters, Inches, DisplayAspectRatio, Unknown };

/*!
 * \brief The AspectRatioType enum specifies the possible modifications to the aspect ratio.
 */
enum class AspectRatioType : std::uint64_t {
    FreeResizing,
    KeepAspectRatio,
    Fixed,
    Unknown = std::numeric_limits<std::uint64_t>::max(),
};

} // namespace TagParser

CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(TagParser, TagParser::TrackFlags)

namespace TagParser {

struct AbstractTrackPrivate;

class TAG_PARSER_EXPORT AbstractTrack {
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
    CppUtilities::BinaryReader &reader();
    CppUtilities::BinaryWriter &writer();
    std::uint64_t startOffset() const;
    TrackFlags flags() const;
    MediaFormat format() const;
    double version() const;
    std::string_view formatName() const;
    std::string_view formatAbbreviation() const;
    const std::string &formatId() const;
    MediaType mediaType() const;
    std::string_view mediaTypeName() const;
    std::uint64_t size() const;
    void setSize(std::uint64_t size);
    std::uint32_t trackNumber() const;
    void setTrackNumber(std::uint32_t trackNumber);
    std::uint64_t id() const;
    void setId(std::uint64_t id);
    const std::string name() const;
    void setName(std::string_view name);
    const CppUtilities::TimeSpan &duration() const;
    double bitrate() const;
    double maxBitrate() const;
    const CppUtilities::DateTime &creationTime() const;
    const CppUtilities::DateTime &modificationTime() const;
    const Locale &locale() const;
    void setLocale(const Locale &locale);
    std::uint32_t samplingFrequency() const;
    std::uint32_t extensionSamplingFrequency() const;
    std::uint16_t bitsPerSample() const;
    std::uint16_t channelCount() const;
    std::uint8_t channelConfig() const;
    std::string_view channelConfigString() const;
    std::uint8_t extensionChannelConfig() const;
    std::string_view extensionChannelConfigString() const;
    std::uint64_t sampleCount() const;
    int quality() const;
    const Size &pixelSize() const;
    const Size &displaySize() const;
    const Size &resolution() const;
    const std::string &compressorName() const;
    void setCompressorName(std::string_view compressorName);
    std::uint16_t depth() const;
    std::uint32_t fps() const;
    std::string_view chromaFormat() const;
    const AspectRatio &pixelAspectRatio() const;
    bool isInterlaced() const;
    std::uint32_t timeScale() const;
    bool isEnabled() const;
    void setEnabled(bool enabled);
    bool isDefault() const;
    void setDefault(bool isDefault);
    bool isForced() const;
    void setForced(bool forced);
    bool hasLacing() const;
    bool isEncrypted() const;
    std::uint32_t colorSpace() const;
    const Margin &cropping() const;
    std::string label() const;
    std::string description() const;
    std::string shortDescription() const;

    void parseHeader(Diagnostics &diag, AbortableProgressFeedback &progress);
    bool isHeaderValid() const;

protected:
    AbstractTrack(std::istream &inputStream, std::ostream &outputStream, std::uint64_t startOffset);
    AbstractTrack(std::iostream &stream, std::uint64_t startOffset);
    virtual void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) = 0;

    std::istream *m_istream;
    std::ostream *m_ostream;
    CppUtilities::BinaryReader m_reader;
    CppUtilities::BinaryWriter m_writer;
    std::uint64_t m_startOffset;
    TrackFlags m_flags;
    MediaFormat m_format;
    std::string m_formatId;
    std::string m_formatName;
    MediaType m_mediaType;
    double m_version;
    std::uint64_t m_size;
    std::uint32_t m_trackNumber;
    std::uint64_t m_id;
    std::string m_name;
    CppUtilities::TimeSpan m_duration;
    double m_bitrate;
    double m_maxBitrate;
    CppUtilities::DateTime m_creationTime;
    CppUtilities::DateTime m_modificationTime;
    Locale m_locale;
    std::uint32_t m_samplingFrequency;
    std::uint32_t m_extensionSamplingFrequency;
    std::uint16_t m_bitsPerSample;
    std::uint32_t m_bytesPerSecond;
    std::uint16_t m_channelCount;
    std::uint8_t m_channelConfig;
    std::uint8_t m_extensionChannelConfig;
    std::uint16_t m_chunkSize;
    std::uint64_t m_sampleCount;
    int m_quality;
    Size m_pixelSize;
    Size m_displaySize;
    Size m_resolution;
    std::string m_compressorName;
    std::uint16_t m_depth;
    std::uint32_t m_fps;
    std::string_view m_chromaFormat;
    AspectRatio m_pixelAspectRatio;
    std::uint32_t m_timeScale;
    std::uint32_t m_colorSpace;
    Margin m_cropping;
    FieldOrder m_fieldOrder;
    StereoMode m_stereoMode;
    AlphaMode m_alphaMode;
    DisplayUnit m_displayUnit;
    AspectRatioType m_aspectRatioType;
    std::unique_ptr<AbstractTrackPrivate> m_p;

private:
    std::string makeDescription(bool verbose) const;
};

/*!
 * \brief Returns the associated input stream.
 */
inline std::istream &AbstractTrack::inputStream()
{
    return *m_istream;
}

/*!
 * \brief Assigns another input stream.
 *
 * The track will read from the \a stream to perform
 * particular operations such as reading header information.
 */
inline void AbstractTrack::setInputStream(std::istream &stream)
{
    m_reader.setStream(m_istream = &stream);
}

/*!
 * \brief Returns the associated output stream.
 */
inline std::ostream &AbstractTrack::outputStream()
{
    return *m_ostream;
}

/*!
 * \brief Assigns another output stream.
 *
 * The track will write to the \a stream to perform
 * particular operations such as updating or making header information.
 */
inline void AbstractTrack::setOutputStream(std::ostream &stream)
{
    m_writer.setStream(m_ostream = &stream);
}

/*!
 * \brief Returns a binary reader for the associated stream.
 *
 * \remarks The returned reader is internally used when reading binary data
 *          from the associated stream. Do not change its byte order.
 */
inline CppUtilities::BinaryReader &AbstractTrack::reader()
{
    return m_reader;
}

/*!
 * \brief Returns a binary writer for the associated stream.
 *
 * \remarks The returned writer is internally used when writing binary data
 *          to the associated stream. Do not change its byte order.
 */
inline CppUtilities::BinaryWriter &AbstractTrack::writer()
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
inline std::uint64_t AbstractTrack::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns flags (various boolean properties) of this track.
 * \sa TrackFlags
 */
inline TrackFlags AbstractTrack::flags() const
{
    return m_flags;
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
 *  - The string might get invalidated when the track is (re)parsed.
 */
inline std::string_view AbstractTrack::formatName() const
{
    return m_format || m_formatName.empty() ? m_format.name() : m_formatName;
}

/*!
 * \brief Returns the a more or less common abbreviation for the format of the track
 *        if known; otherwise returns an empty string.
 */
inline std::string_view AbstractTrack::formatAbbreviation() const
{
    const auto abbr = m_format.abbreviation();
    return !abbr.empty() || m_formatId.empty() ? abbr : m_formatId;
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
inline std::string_view AbstractTrack::mediaTypeName() const
{
    return ::TagParser::mediaTypeName(m_mediaType);
}

/*!
 * \brief Returns the size in bytes if known; otherwise returns 0.
 */
inline std::uint64_t AbstractTrack::size() const
{
    return m_size;
}

/*!
 * \brief Sets the size in bytes.
 * \remarks
 * This is used by MediaFileInfo to set the track size for certain types of tracks before invoking the parsing.
 * If you use this a class derived from AbstractTrack directly you may want to do the same if not the entire
 * input stream is supposed to be considered part of the track and the parser would otherwise assume that (like
 * the parser of MpegAudioFrameStream might do).
 */
inline void AbstractTrack::setSize(std::uint64_t size)
{
    m_size = size;
}

/*!
 * \brief Returns the track number if known; otherwise returns 0.
 */
inline std::uint32_t AbstractTrack::trackNumber() const
{
    return m_trackNumber;
}

/*!
 * \brief Sets the track number.
 * \remarks Whether the new value is applied when saving changes depends on the implementation.
 */
inline void AbstractTrack::setTrackNumber(std::uint32_t trackNumber)
{
    m_trackNumber = trackNumber;
}

/*!
 * \brief Returns the track ID if known; otherwise returns 0.
 */
inline std::uint64_t AbstractTrack::id() const
{
    return m_id;
}

/*!
 * \brief Sets the track ID.
 * \remarks Whether the new value is applied when saving changes depends on the implementation.
 */
inline void AbstractTrack::setId(std::uint64_t id)
{
    m_id = id;
}

/*!
 * \brief Returns the track name if known; otherwise returns an empty string.
 */
inline const std::string AbstractTrack::name() const
{
    return m_name;
}

/*!
 * \brief Sets the name.
 * \remarks Whether the new value is applied when saving changes depends on the implementation.
 */
inline void AbstractTrack::setName(std::string_view name)
{
    m_name = name;
}

/*!
 * \brief Returns the duration if known; otherwise returns a TimeSpan of zero ticks.
 */
inline const CppUtilities::TimeSpan &AbstractTrack::duration() const
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
inline const CppUtilities::DateTime &AbstractTrack::creationTime() const
{
    return m_creationTime;
}

/*!
 * \brief Returns the time of the last modification if known; otherwise returns a DateTime of zero ticks.
 */
inline const CppUtilities::DateTime &AbstractTrack::modificationTime() const
{
    return m_modificationTime;
}

/*!
 * \brief Returns the locale of the track if known; otherwise returns an empty locale.
 *
 * The format of the locale depends on the particular format/implementation.
 */
inline const Locale &AbstractTrack::locale() const
{
    return m_locale;
}

/*!
 * \brief Sets the locale of the track.
 * \remarks Whether the new value is applied when saving changes depends on the format/implementation.
 */
inline void AbstractTrack::setLocale(const Locale &locale)
{
    m_locale = locale;
}

/*!
 * \brief Returns the number of samples per second if known; otherwise returns 0.
 */
inline std::uint32_t AbstractTrack::samplingFrequency() const
{
    return m_samplingFrequency;
}

/*!
 * \brief Returns the number of samples per second if known; otherwise returns 0.
 * \remarks This sample rate value takes extensions like SBR into account.
 */
inline std::uint32_t AbstractTrack::extensionSamplingFrequency() const
{
    return m_extensionSamplingFrequency;
}

/*!
 * \brief Returns the number of bits per sample; otherwise returns 0.
 */
inline std::uint16_t AbstractTrack::bitsPerSample() const
{
    return m_bitsPerSample;
}

/*!
 * \brief Returns the number of channels if known; otherwise returns 0.
 *
 * This value only makes sense for audio tracks.
 */
inline std::uint16_t AbstractTrack::channelCount() const
{
    return m_channelCount;
}

/*!
 * \brief Returns the channel configuration.
 *
 * This is the MPEG-4 channel config for MPEG-4 audio.
 * \sa Mpeg4ChannelConfigs::channelConfigString()
 */
inline std::uint8_t AbstractTrack::channelConfig() const
{
    return m_channelConfig;
}

/*!
 * \brief Returns the number of samples/frames if known; otherwise returns 0.
 */
inline std::uint64_t AbstractTrack::sampleCount() const
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
 * \brief Returns the compressor name if known; otherwise returns an empty string.
 * \remarks Whether the new value is applied when saving changes depends on the implementation.
 */
inline void AbstractTrack::setCompressorName(std::string_view compressorName)
{
    m_compressorName = compressorName;
}

/*!
 * \brief Returns the bit depth if known; otherwise returns 0.
 */
inline std::uint16_t AbstractTrack::depth() const
{
    return m_depth;
}

/*!
 * \brief Returns the number of frames per second if known; otherwise returns 0.
 *
 * This value only makes sense for video tracks.
 */
inline std::uint32_t AbstractTrack::fps() const
{
    return m_fps;
}

/*!
 * \brief Returns the chroma subsampling format if known; otherwise returns nullptr.
 *
 * This value only makes sense for video tracks.
 */
inline std::string_view AbstractTrack::chromaFormat() const
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
 * \brief Returns true if the video is interlaced; otherwise returns false.
 *
 * This value only makes sense for video tracks.
 */
inline bool AbstractTrack::isInterlaced() const
{
    return m_flags & TrackFlags::Interlaced;
}

/*!
 * \brief Returns the time scale if known; otherwise returns 0.
 *
 * The time scale depends on the format.
 */
inline std::uint32_t AbstractTrack::timeScale() const
{
    return m_timeScale;
}

/*!
 * \brief Returns true if the track is marked as enabled; otherwise returns false.
 */
inline bool AbstractTrack::isEnabled() const
{
    return m_flags & TrackFlags::Enabled;
}

/*!
 * \brief Sets whether the track is enabled.
 * \remarks Whether the new value is applied when saving changes depends on the implementation.
 */
inline void AbstractTrack::setEnabled(bool enabled)
{
    CppUtilities::modFlagEnum(m_flags, TrackFlags::Enabled, enabled);
}

/*!
 * \brief Returns true if the track is marked as default; otherwise returns false.
 */
inline bool AbstractTrack::isDefault() const
{
    return m_flags & TrackFlags::Default;
}

/*!
 * \brief Sets whether the track is a default track.
 * \remarks Whether the new value is applied when saving changes depends on the implementation.
 */
inline void AbstractTrack::setDefault(bool isDefault)
{
    CppUtilities::modFlagEnum(m_flags, TrackFlags::Default, isDefault);
}

/*!
 * \brief Returns true if the track is marked as forced; otherwise returns false.
 */
inline bool AbstractTrack::isForced() const
{
    return m_flags & TrackFlags::Forced;
}

/*!
 * \brief Sets whether the track is forced.
 * \remarks Whether the new value is applied when saving changes depends on the implementation.
 */
inline void AbstractTrack::setForced(bool forced)
{
    CppUtilities::modFlagEnum(m_flags, TrackFlags::Forced, forced);
}

/*!
 * \brief Returns true if the track has lacing; otherwise returns false.
 */
inline bool AbstractTrack::hasLacing() const
{
    return m_flags & TrackFlags::Lacing;
}

/*!
 * \brief Returns true if the track is marked as encrypted; otherwise returns false.
 */
inline bool AbstractTrack::isEncrypted() const
{
    return m_flags & TrackFlags::Encrypted;
}

/*!
 * \brief Returns the color space if known; otherwise returns 0.
 */
inline std::uint32_t AbstractTrack::colorSpace() const
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
    return m_flags & TrackFlags::HeaderValid;
}

} // namespace TagParser

#endif // TAG_PARSER_ABSTRACTTRACK_H
