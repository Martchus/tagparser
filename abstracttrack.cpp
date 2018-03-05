#include "./abstracttrack.h"
#include "./exceptions.h"
#include "./mediaformat.h"

#include "./mp4/mp4ids.h"

#include "mpegaudio/mpegaudioframe.h"

using namespace std;
using namespace ConversionUtilities;
using namespace ChronoUtilities;
using namespace IoUtilities;

namespace Media {

/*!
 * \class Media::AbstractTrack
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
AbstractTrack::AbstractTrack(istream &inputStream, ostream &outputStream, uint64 startOffset) :
    m_istream(&inputStream),
    m_ostream(&outputStream),
    m_reader(BinaryReader(&inputStream)),
    m_writer(BinaryWriter(&outputStream)),
    m_startOffset(startOffset),
    m_headerValid(false),
    m_format(),
    m_mediaType(MediaType::Unknown),
    m_version(0.0),
    m_size(0),
    m_trackNumber(0),
    m_id(0),
    m_bitrate(0.0),
    m_maxBitrate(0.0),
    m_samplingFrequency(0),
    m_extensionSamplingFrequency(0),
    m_bitsPerSample(0),
    m_bytesPerSecond(0),
    m_channelCount(0),
    m_channelConfig(0),
    m_extensionChannelConfig(0),
    m_sampleCount(0),
    m_quality(0),
    m_depth(0),
    m_fps(0),
    m_chromaFormat(nullptr),
    m_interlaced(false),
    m_timeScale(0),
    m_enabled(true),
    m_default(false),
    m_forced(false),
    m_lacing(false),
    m_encrypted(false),
    m_usedInPresentation(true),
    m_usedWhenPreviewing(true),
    m_colorSpace(0)
{}

/*!
 * \brief Constructs a new track.
 * \param stream Specifies the stream the track will read from or write
 *               to to perform particular operations such as reading header
 *               information.
 * \param startOffset The start offset of the track in the specified \a stream.
 */
AbstractTrack::AbstractTrack(std::iostream &stream, uint64 startOffset) :
    AbstractTrack(stream, stream, startOffset)
{}

/*!
 * \brief Destroys the track.
 */
AbstractTrack::~AbstractTrack()
{}

/*!
 * \brief Returns a string with the channel configuration if available; otherwise returns nullptr.
 */
const char *AbstractTrack::channelConfigString() const
{
    switch(m_format.general) {
    case GeneralMediaFormat::Aac:
        return m_channelConfig ? Mpeg4ChannelConfigs::channelConfigString(m_channelConfig) : nullptr;
    case GeneralMediaFormat::Mpeg1Audio: case GeneralMediaFormat::Mpeg2Audio:
        return mpegChannelModeString(static_cast<MpegChannelMode>(m_channelConfig));
    default:
        return nullptr;
    }
}

/*!
 * \brief Returns the extension channel configuration if available; otherwise returns nullptr.
 */
byte AbstractTrack::extensionChannelConfig() const
{
    return m_extensionChannelConfig;
}

/*!
 * \brief Returns a string with the extension channel configuration if available; otherwise returns nullptr.
 */
const char *AbstractTrack::extensionChannelConfigString() const
{
    switch(m_format.general) {
    case GeneralMediaFormat::Aac:
        return m_extensionChannelConfig ? Mpeg4ChannelConfigs::channelConfigString(m_extensionChannelConfig) : nullptr;
    default:
        return nullptr;
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
    if(!name().empty()) {
        ss << ", name: \"" << name() << "\"";
    }
    if(!language().empty() && language() != "und") {
        ss << ", language: \"" << language() << "\"";
    }
    return ss.str();
}

/*!
 * \brief Returns a short description about the track.
 *
 * The description contains the abbreviated format and further information depending on the media
 * type (eg. display size in case of video, language in case of audio/text). It is intended to be joined
 * with descriptions of other tracks to get a short technical description about the file.
 *
 * Examples (exact format might change in the future!):
 * - H.264-720p
 * - HE-AAC-6ch-eng
 */
string AbstractTrack::description() const
{
    // use abbreviated format
    const char *format = m_format.shortAbbreviation();
    if(!format || !*format) {
        // fall back to media type name if no abbreviation available
        format = mediaTypeName();
    }

    // find additional info
    const char *additionalInfo = nullptr;
    switch(m_mediaType) {
    case MediaType::Video:
        if(!displaySize().isNull()) {
            additionalInfo = displaySize().abbreviation();
        } else if(!pixelSize().isNull()) {
            additionalInfo = pixelSize().abbreviation();
        }
        break;
    case MediaType::Audio:
    case MediaType::Text:
        if(channelCount()) {
            if(!language().empty() && language() != "und") {
                return argsToString(format, '-', channelCount(), "ch-", language());
            } else {
                return argsToString(format, '-', channelCount(), 'c', 'h');
            }
        } else if(!language().empty() && language() != "und") {
            additionalInfo = language().data();
        }
        break;
    default:
        ;
    }

    if(additionalInfo) {
        return argsToString(format, '-', additionalInfo);
    }
    return format;
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
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void AbstractTrack::parseHeader(Diagnostics &diag)
{
    m_headerValid = false;
    m_istream->seekg(m_startOffset, ios_base::beg);
    try {
        internalParseHeader(diag);
        m_headerValid = true;
    } catch(Failure &) {
        throw;
    }
}

/*!
 * \fn AbstractTrack::internalParseHeader()
 * \brief This method is internally called to parse header information.
 *        It needs to be implemented when subclassing this class.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */

}
