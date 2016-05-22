#include "./oggstream.h"
#include "./oggcontainer.h"

#include "../vorbis/vorbispackagetypes.h"
#include "../vorbis/vorbisidentificationheader.h"

#include "../opus/opusidentificationheader.h"

#include "../flac/flactooggmappingheader.h"

#include "../mediafileinfo.h"
#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/chrono/timespan.h>

#include <iostream>
#include <functional>

using namespace std;
using namespace std::placeholders;
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
    m_container(container),
    m_currentSequenceNumber(0)
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
    iterator.setFilter(firstPage.streamSerialNumber());
    iterator.setPageIndex(m_startPage);

    // predicate for finding pages of this stream by its stream serial number
    const auto pred = bind(&OggPage::matchesStreamSerialNumber, _1, firstPage.streamSerialNumber());

    // iterate through segments using OggIterator
    // -> iterate through ALL segments to calculate the precise stream size (hence the out-commented part in the loop-condition)
    for(bool hasIdentificationHeader = false, hasCommentHeader = false; iterator /* && (!hasIdentificationHeader && !hasCommentHeader) */; ++iterator) {
        const uint32 currentSize = iterator.currentSegmentSize();
        m_size += currentSize;

        if(currentSize >= 8) {
            // determine stream format
            inputStream().seekg(iterator.currentSegmentOffset());
            const uint64 sig = reader().readUInt64BE();

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
                    continue;
                }

                // check header type
                switch(sig >> 56) {
                case VorbisPackageTypes::Identification:
                    if(!hasIdentificationHeader) {
                        // parse identification header
                        VorbisIdentificationHeader ind;
                        ind.parseHeader(iterator);
                        m_version = ind.version();
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
                        // determine sample count and duration if all pages have been fetched
                        if(iterator.areAllPagesFetched()) {
                            const auto &pages = iterator.pages();
                            const auto firstPage = find_if(pages.cbegin(), pages.cend(), pred);
                            const auto lastPage = find_if(pages.crbegin(), pages.crend(), pred);
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
                    // Vorbis comment found -> notify container about comment
                    if(!hasCommentHeader) {
                        m_container.announceComment(iterator.currentPageIndex(), iterator.currentSegmentIndex(), false, GeneralMediaFormat::Vorbis);
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

            } else if(sig == 0x4F70757348656164u) {
                // Opus header detected
                // set Opus as format
                switch(m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Opus;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Opus:
                    break;
                default:
                    addNotification(NotificationType::Warning, "Stream format is inconsistent.", context);
                    continue;
                }
                if(!hasIdentificationHeader) {
                    // parse identification header
                    OpusIdentificationHeader ind;
                    ind.parseHeader(iterator);
                    m_version = ind.version();
                    m_channelCount = ind.channels();
                    m_samplingFrequency = ind.sampleRate();
                    // determine sample count and duration if all pages have been fetched
                    if(iterator.areAllPagesFetched()) {
                        const auto &pages = iterator.pages();
                        const auto firstPage = find_if(pages.cbegin(), pages.cend(), pred);
                        const auto lastPage = find_if(pages.crbegin(), pages.crend(), pred);
                        if(firstPage != pages.cend() && lastPage != pages.crend()) {
                            m_sampleCount = lastPage->absoluteGranulePosition() - firstPage->absoluteGranulePosition();
                            // must apply "pre-skip" here do calculate effective sample count and duration?
                            if(m_sampleCount > ind.preSkip()) {
                                m_sampleCount -= ind.preSkip();
                            } else {
                                m_sampleCount = 0;
                            }
                            m_duration = TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / m_samplingFrequency);
                        }
                    }
                    hasIdentificationHeader = true;
                } else {
                    addNotification(NotificationType::Critical, "Opus identification header appears more then once. Oversupplied occurrence will be ignored.", context);
                }

            } else if(sig == 0x4F70757354616773u) {
                // Opus comment detected
                // set Opus as format
                switch(m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Opus;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Opus:
                    break;
                default:
                    addNotification(NotificationType::Warning, "Stream format is inconsistent.", context);
                    continue;
                }

                // notify container about comment
                if(!hasCommentHeader) {
                    m_container.announceComment(iterator.currentPageIndex(), iterator.currentSegmentIndex(), false, GeneralMediaFormat::Opus);
                    hasCommentHeader = true;
                } else {
                    addNotification(NotificationType::Critical, "Opus tags/comment header appears more then once. Oversupplied occurrence will be ignored.", context);
                }

            } else if((sig & 0xFFFFFFFFFF000000u) == 0x7F464C4143000000u) {
                // FLAC header detected
                // set FLAC as format
                switch(m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Flac;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Flac:
                    break;
                default:
                    addNotification(NotificationType::Warning, "Stream format is inconsistent.", context);
                    continue;
                }

                if(!hasIdentificationHeader) {
                    // parse FLAC-to-Ogg mapping header
                    FlacToOggMappingHeader mapping;
                    const FlacMetaDataBlockStreamInfo &streamInfo = mapping.streamInfo();
                    mapping.parseHeader(iterator);
                    m_bitsPerSample = streamInfo.bitsPerSample();
                    m_channelCount = streamInfo.channelCount();
                    m_samplingFrequency = streamInfo.samplingFrequency();
                    m_sampleCount = streamInfo.totalSampleCount();
                    if(!m_sampleCount && iterator.areAllPagesFetched()) {
                        const auto &pages = iterator.pages();
                        const auto firstPage = find_if(pages.cbegin(), pages.cend(), pred);
                        const auto lastPage = find_if(pages.crbegin(), pages.crend(), pred);
                        if(firstPage != pages.cend() && lastPage != pages.crend()) {
                            m_sampleCount = lastPage->absoluteGranulePosition() - firstPage->absoluteGranulePosition();
                        }
                    }
                    m_duration = TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / m_samplingFrequency);
                    hasIdentificationHeader = true;
                } else {
                    addNotification(NotificationType::Critical, "FLAC-to-Ogg mapping header appears more then once. Oversupplied occurrence will be ignored.", context);
                }

                if(!hasCommentHeader) {
                    // a Vorbis comment should be following
                    if(++iterator) {
                        char buff[4];
                        iterator.read(buff, 4);
                        FlacMetaDataBlockHeader header;
                        header.parseHeader(buff);
                        if(header.type() == FlacMetaDataBlockType::VorbisComment) {
                            m_container.announceComment(iterator.currentPageIndex(), iterator.currentSegmentIndex(), header.isLast(), GeneralMediaFormat::Flac);
                            hasCommentHeader = true;
                        } else {
                            addNotification(NotificationType::Critical, "OGG page after FLAC-to-Ogg mapping header doesn't contain Vorbis comment.", context);
                        }
                    } else {
                        addNotification(NotificationType::Critical, "No more OGG pages after FLAC-to-Ogg mapping header (Vorbis comment expected).", context);
                    }
                }

            } else if((sig & 0x00ffffffffffff00u) == 0x007468656F726100u) {
                // Theora header detected
                // set Theora as format
                switch(m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Theora;
                    m_mediaType = MediaType::Video;
                    break;
                case GeneralMediaFormat::Theora:
                    break;
                default:
                    addNotification(NotificationType::Warning, "Stream format is inconsistent.", context);
                    continue;
                }
                // TODO: read more information about Theora stream

            } // currently only Vorbis, Opus and Theora can be detected, TODO: detect more formats

        } else {
            // just ignore segments of only 8 byte or even less
            // TODO: print warning?
        }

        // TODO: reduce code duplication
    }

    if(m_duration.isNull() && m_size && m_bitrate) {
        // calculate duration from stream size and bitrate, assuming 1 % overhead
        m_duration = TimeSpan::fromSeconds(static_cast<double>(m_size) / (m_bitrate * 125.0) * 1.1);
    }
    m_headerValid = true;
}

}
