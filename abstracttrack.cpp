#include "./abstracttrack.h"
#include "./exceptions.h"
#include "./mediaformat.h"

#include "./mp4/mp4ids.h"

#include "mpegaudio/mpegaudioframe.h"

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/// \brief The AbstractTrackPrivate struct contains private fields of the AbstractTrack class.
struct AbstractTrackPrivate {};

/*!
 * \class TagParser::AbstractTrack
 * \brief The AbstractTrack class parses and stores technical information about
 *        video, audio and other kinds of media tracks.
 *
 * The tag class only provides the interface and common functionality. It is meant to be subclassed.
 */

/*!
 * \brief Constructs a new track.
 * \param inputStream Specifies the stream the track will read from to perform particular operations such
 *                    as reading header information.
 * \param outputStream Specifies the stream the track will write to to perform particular operations such
 *                     as updating or making header information.
 * \param startOffset The start offset of the track in the specified \a stream.
 */
AbstractTrack::AbstractTrack(istream &inputStream, ostream &outputStream, std::uint64_t startOffset)
    : m_istream(&inputStream)
    , m_ostream(&outputStream)
    , m_reader(BinaryReader(&inputStream))
    , m_writer(BinaryWriter(&outputStream))
    , m_startOffset(startOffset)
    , m_flags(TrackFlags::Enabled | TrackFlags::UsedInPresentation | TrackFlags::UsedWhenPreviewing)
    , m_format()
    , m_mediaType(MediaType::Unknown)
    , m_version(0.0)
    , m_size(0)
    , m_trackNumber(0)
    , m_id(0)
    , m_bitrate(0.0)
    , m_maxBitrate(0.0)
    , m_samplingFrequency(0)
    , m_extensionSamplingFrequency(0)
    , m_bitsPerSample(0)
    , m_bytesPerSecond(0)
    , m_channelCount(0)
    , m_channelConfig(0)
    , m_extensionChannelConfig(0)
    , m_sampleCount(0)
    , m_quality(0)
    , m_depth(0)
    , m_fps(0)
    , m_timeScale(0)
    , m_colorSpace(0)
    , m_fieldOrder(FieldOrder::Undetermined)
    , m_stereoMode(StereoMode::Unknown)
    , m_alphaMode(AlphaMode::Unknown)
    , m_displayUnit(DisplayUnit::Unknown)
    , m_aspectRatioType(AspectRatioType::Unknown)
{
}

/*!
 * \brief Constructs a new track.
 * \param stream Specifies the stream the track will read from or write
 *               to to perform particular operations such as reading header
 *               information.
 * \param startOffset The start offset of the track in the specified \a stream.
 */
AbstractTrack::AbstractTrack(std::iostream &stream, std::uint64_t startOffset)
    : AbstractTrack(stream, stream, startOffset)
{
}

/*!
 * \brief Destroys the track.
 */
AbstractTrack::~AbstractTrack()
{
}

/*!
 * \brief Returns a string with the channel configuration if available; otherwise returns nullptr.
 */
std::string_view AbstractTrack::channelConfigString() const
{
    switch (m_format.general) {
    case GeneralMediaFormat::Aac:
        return m_channelConfig ? Mpeg4ChannelConfigs::channelConfigString(m_channelConfig) : std::string_view();
    case GeneralMediaFormat::Mpeg1Audio:
    case GeneralMediaFormat::Mpeg2Audio:
        return mpegChannelModeString(static_cast<MpegChannelMode>(m_channelConfig));
    default:
        return std::string_view();
    }
}

/*!
 * \brief Returns the extension channel configuration if available; otherwise returns nullptr.
 */
std::uint8_t AbstractTrack::extensionChannelConfig() const
{
    return m_extensionChannelConfig;
}

/*!
 * \brief Returns a string with the extension channel configuration if available; otherwise returns nullptr.
 */
std::string_view AbstractTrack::extensionChannelConfigString() const
{
    switch (m_format.general) {
    case GeneralMediaFormat::Aac:
        return m_extensionChannelConfig ? Mpeg4ChannelConfigs::channelConfigString(m_extensionChannelConfig) : std::string_view();
    default:
        return std::string_view();
    }
}

/*!
 * \brief Returns a label for the track.
 *
 * The label contains the ID, type, name and language of the track. It is intended to be used in a menu for
 * selecting a track from the file.
 */
string AbstractTrack::label() const
{
    stringstream ss;
    ss << "ID: " << id();
    ss << ", type: " << mediaTypeName();
    if (!name().empty()) {
        ss << ", name: \"" << name() << "\"";
    }
    if (const auto &language = locale().fullOrSomeAbbreviatedName(); !language.empty()) {
        ss << ", language: " << language << "";
    }
    return ss.str();
}

/// \cond
string AbstractTrack::makeDescription(bool verbose) const
{
    // use abbreviated format
    const auto format = MediaFormat(m_format.general, verbose ? m_format.sub : 0, verbose ? m_format.extension : 0);
    auto formatName = format.shortAbbreviation();
    if (formatName.empty()) {
        // fall back to media type name if no abbreviation available
        formatName = mediaTypeName();
    }

    // find additional info and level
    auto additionalInfoRef = std::string_view();
    auto level = std::string();
    switch (m_mediaType) {
    case MediaType::Video:
        if (!pixelSize().isNull()) {
            additionalInfoRef = pixelSize().abbreviation();
        } else if (!displaySize().isNull()) {
            additionalInfoRef = displaySize().abbreviation();
        }
        if (verbose) {
            switch (format.general) {
            case GeneralMediaFormat::Mpeg4Video:
            case GeneralMediaFormat::Avc:
            case GeneralMediaFormat::Hevc:
                if (version() != 0.0) {
                    level = "@L" + numberToString(version());
                }
                break;
            default:;
            }
        }
        break;
    case MediaType::Audio:
    case MediaType::Text:
        if (channelCount()) {
            if (const auto &localeName = locale().someAbbreviatedName(); !localeName.empty()) {
                return argsToString(formatName, '-', channelCount(), "ch-", localeName);
            } else {
                return argsToString(formatName, '-', channelCount(), 'c', 'h');
            }
        } else if (const auto &localeName = locale().someAbbreviatedName(); !localeName.empty()) {
            additionalInfoRef = localeName;
        }
        break;
    default:;
    }

    if (!additionalInfoRef.empty()) {
        return argsToString(formatName, level, '-', additionalInfoRef);
    }
    return argsToString(formatName, level);
}
/// \endcond

/*!
 * \brief Returns a description about the track.
 *
 * The description contains the abbreviated format and further information depending on the media
 * type (eg. display size in case of video, language in case of audio/text). It is intended to be joined
 * with descriptions of other tracks to get a short technical description about the file.
 *
 * Examples (exact format might change in the future!):
 * - H.264-High-10@5.1-720p
 * - HE-AAC-6ch-eng
 */
string AbstractTrack::description() const
{
    return makeDescription(true);
}

/*!
 * \brief Returns a short description about the track.
 *
 * See description() for details.
 *
 * Examples (exact format might change in the future!):
 * - H.264-720p
 * - HE-AAC-6ch-eng
 */
string AbstractTrack::shortDescription() const
{
    return makeDescription(false);
}

/*!
 * \brief Parses technical information about the track from the header.
 *
 * The information will be read from the associated stream. The stream and the
 * start offset of the track have been specified when constructing the Track.
 *
 * The parsed information can be accessed using the corresponding methods.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void AbstractTrack::parseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    m_flags -= TrackFlags::HeaderValid;
    m_istream->seekg(static_cast<streamoff>(m_startOffset), ios_base::beg);
    try {
        internalParseHeader(diag, progress);
        m_flags += TrackFlags::HeaderValid;
    } catch (const Failure &) {
        throw;
    }
}

/*!
 * \fn AbstractTrack::internalParseHeader()
 * \brief This method is internally called to parse header information.
 *        It needs to be implemented when subclassing this class.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */

} // namespace TagParser
