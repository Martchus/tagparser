#include "ebmlelement.h"
#include "matroskatrack.h"
#include "matroskacontainer.h"
#include "matroskaid.h"

#include "../mediaformat.h"
#include "../exceptions.h"

using namespace std;

namespace Media {

/*!
 * \class Media::MatroskaTrack
 * \brief Implementation of Media::AbstractTrack for the Matroska container.
 */

/*!
 * \brief Constructs a new track for the specified \a trackElement.
 *
 * Each track element (ID: MatroskaId::TrackEntry) holds header information
 * for one track in the Matroska file.
 */
MatroskaTrack::MatroskaTrack(EbmlElement &trackElement) :
    AbstractTrack(trackElement.stream(), trackElement.startOffset()),
    m_trackElement(&trackElement)
{}

/*!
 * \brief Destroys the track.
 */
MatroskaTrack::~MatroskaTrack()
{}

TrackType MatroskaTrack::type() const
{
    return TrackType::MatroskaTrack;
}

void MatroskaTrack::internalParseHeader()
{
    const string context("parsing header of Matroska track");

    try {
    m_trackElement->parse();
    } catch(Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse track element.", context);
        throw;
    }

    EbmlElement *trackInfoElement = m_trackElement->firstChild(), *subElement = nullptr;
    while(trackInfoElement) {
        try {
            trackInfoElement->parse();
        } catch (Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse track information element.", context);
            break;
        }
        uint32 defaultDuration = 0;
        switch(trackInfoElement->id()) {
        case MatroskaIds::TrackType:
            switch(trackInfoElement->readUInteger()) {
            case MatroskaTrackType::Video:
                m_mediaType = MediaType::Visual;
                break;
            case MatroskaTrackType::Audio:
                m_mediaType = MediaType::Acoustic;
                break;
            case MatroskaTrackType::Subtitle:
                m_mediaType = MediaType::Textual;
                break;
            default:
                m_mediaType = MediaType::Unknown;
            }
            break;
        case MatroskaIds::TrackVideo:
            subElement = trackInfoElement->firstChild();
            while(subElement) {
                try {
                    subElement->parse();
                } catch (Failure &) {
                    addNotification(NotificationType::Critical, "Unable to parse video track element.", context);
                    break;
                }
                switch(subElement->id()) {
                case MatroskaIds::DisplayWidth:
                    m_displaySize.setWidth(subElement->readUInteger());
                    break;
                case MatroskaIds::DisplayHeight:
                    m_displaySize.setHeight(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelWidth:
                    m_pixelSize.setWidth(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelHeight:
                    m_pixelSize.setHeight(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelCropTop:
                    m_cropping.setTop(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelCropLeft:
                    m_cropping.setLeft(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelCropBottom:
                    m_cropping.setBottom(subElement->readUInteger());
                    break;
                case MatroskaIds::PixelCropRight:
                    m_cropping.setRight(subElement->readUInteger());
                    break;
                case MatroskaIds::FrameRate:
                    m_fps = subElement->readFloat();
                    break;
                case MatroskaIds::FlagInterlaced:
                    m_interlaced = subElement->readUInteger();
                    break;
                case MatroskaIds::ColorSpace:
                    m_colorSpace = subElement->readUInteger();
                    break;
                default: ;
                }
                subElement = subElement->nextSibling();
            }
            break;
        case MatroskaIds::TrackAudio:
            subElement = trackInfoElement->firstChild();
            while(subElement) {
                try {
                    subElement->parse();
                } catch (Failure &) {
                    addNotification(NotificationType::Critical, "Unable to parse audio track element.", context);
                    break;
                }
                switch(subElement->id()) {
                case MatroskaIds::BitDepth:
                    m_bitsPerSample = subElement->readUInteger();
                    break;
                case MatroskaIds::Channels:
                    m_channelCount = subElement->readUInteger();
                    break;
                case MatroskaIds::SamplingFrequency:
                    m_samplesPerSecond = subElement->readFloat();
                    break;
                default: ;
                }
                subElement = subElement->nextSibling();
            }
            break;
        case MatroskaIds::TrackNumber:
            m_trackNumber = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackUID:
            m_id = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackName:
            m_name = trackInfoElement->readString();
            break;
        case MatroskaIds::TrackLanguage:
            m_language = trackInfoElement->readString();
            break;
        case MatroskaIds::CodecID:
            m_formatId = trackInfoElement->readString();
            break;
        case MatroskaIds::CodecName:
            m_formatName = trackInfoElement->readString();
            break;
        case MatroskaIds::TrackFlagEnabled:
            m_enabled = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackFlagDefault:
            m_default = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackFlagForced:
            m_forced = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::TrackFlagLacing:
            m_lacing = trackInfoElement->readUInteger();
            break;
        case MatroskaIds::DefaultDuration:
            defaultDuration = trackInfoElement->readUInteger();
            break;
        default: ;
        }
        switch(m_mediaType) {
        case MediaType::Visual:
            if(m_fps == 0 && defaultDuration != 0) {
                m_fps = 1000000000.0 / static_cast<double>(defaultDuration);
            }
            break;
        default: ;
        }

        trackInfoElement = trackInfoElement->nextSibling();
    }
}

}
