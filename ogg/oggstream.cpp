#include "./oggstream.h"
#include "./oggcontainer.h"

#include "../vorbis/vorbispackagetypes.h"
#include "../vorbis/vorbisidentificationheader.h"

#include "../mediafileinfo.h"
#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/chrono/timespan.h>

#include <iostream>

using namespace std;
using namespace ChronoUtilities;

namespace Media {

/*!
 * \class Media::OggStream
 * \brief Implementation of Media::AbstractTrack for OGG streams.
 */

/*!
 * \brief Constructs a new track for the \a stream at the specified \a startOffset.
 */
OggStream::OggStream(OggContainer &container, vector<OggPage>::size_type startPage) :
    AbstractTrack(container.stream(), container.m_iterator.pages()[startPage].startOffset()),
    m_startPage(startPage),
    m_container(container)
{}

/*!
 * \brief Destroys the track.
 */
OggStream::~OggStream()
{}

void OggStream::internalParseHeader()
{
    static const string context("parsing OGG page header");
    // read basic information from first page
    OggIterator &iterator = m_container.m_iterator;
    const OggPage &firstPage = iterator.pages()[m_startPage];
    m_version = firstPage.streamStructureVersion();
    m_id = firstPage.streamSerialNumber();
    // ensure iterator is setup properly
    iterator.setFilter(m_id);
    iterator.setPageIndex(m_startPage);
    // iterate through segments using OggIterator
    bool hasIdentificationHeader = false;
    bool hasCommentHeader = false;
    for(; iterator; ++iterator) {
        uint32 currentSize = iterator.currentSegmentSize();
        m_size += currentSize;
        if(currentSize >= 8) {
            // determine stream format
            inputStream().seekg(iterator.currentSegmentOffset());
            uint64 sig = reader().readUInt64BE();
            if((sig & 0x00ffffffffffff00u) == 0x00766F7262697300u) {
                // Vorbis header detected
                // set Vorbis as format
                switch(m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Vorbis;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Vorbis:
                    break;
                default:
                    addNotification(NotificationType::Warning, "Stream format is inconsistent.", context);
                }
                // check header type
                switch(sig >> 56) {
                case VorbisPackageTypes::Identification:
                    if(!hasIdentificationHeader) {
                        VorbisIdentificationHeader ind;
                        ind.parseHeader(iterator);
                        m_channelCount = ind.channels();
                        m_samplingFrequency = ind.sampleRate();
                        if(ind.nominalBitrate()) {
                            m_bitrate = ind.nominalBitrate();
                        } else if(ind.maxBitrate() == ind.minBitrate()) {
                            m_bitrate = ind.maxBitrate();
                        }
                        if(m_bitrate) {
                            m_bitrate = static_cast<double>(m_bitrate) / 1000.0;
                        }
                        if(iterator.areAllPagesFetched()) {
                            auto pred = [this] (const OggPage &page) -> bool {
                                return page.streamSerialNumber() == this->id();
                            };
                            const auto &pages = iterator.pages();
                            auto firstPage = find_if(pages.cbegin(), pages.cend(), pred);
                            auto lastPage = find_if(pages.crbegin(), pages.crend(), pred);
                            if(firstPage != pages.cend() && lastPage != pages.crend()) {
                                m_sampleCount = lastPage->absoluteGranulePosition() - firstPage->absoluteGranulePosition();
                                m_duration = TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / m_samplingFrequency);
                            }
                        }
                        hasIdentificationHeader = true;
                    } else {
                        addNotification(NotificationType::Critical, "Vorbis identification header appears more then once. Oversupplied occurrence will be ignored.", context);
                    }
                    break;
                case VorbisPackageTypes::Comments:
                    if(!hasCommentHeader) {
                        //m_container.m_commentOffsets.push_back(iterator.currentOffset());
                        m_container.ariseComment(iterator.currentPageIndex(), iterator.currentSegmentIndex());
                        hasCommentHeader = true;
                    } else {
                        addNotification(NotificationType::Critical, "Vorbis comment header appears more then once. Oversupplied occurrence will be ignored.", context);
                    }
                    break;
                case VorbisPackageTypes::Setup:
                    break; // TODO
                default:
                    ;
                }
            } // currently only Vorbis supported
        }
    }
    if(m_duration.isNull() && m_size && m_bitrate) {
        // calculate duration from stream size and bitrate, assuming 1 % overhead
        m_duration = TimeSpan::fromSeconds(static_cast<double>(m_size) / (m_bitrate * 125.0) * 1.1);
    }
    m_headerValid = true;
}

}
