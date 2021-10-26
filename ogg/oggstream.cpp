#include "./oggstream.h"
#include "./oggcontainer.h"

#include "../vorbis/vorbisidentificationheader.h"
#include "../vorbis/vorbispackagetypes.h"

#include "../opus/opusidentificationheader.h"

#include "../flac/flactooggmappingheader.h"

#include "../exceptions.h"
#include "../mediafileinfo.h"
#include "../mediaformat.h"

#include <c++utilities/chrono/timespan.h>

#include <functional>
#include <iostream>

using namespace std;
using namespace std::placeholders;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::OggStream
 * \brief Implementation of TagParser::AbstractTrack for OGG streams.
 */

/*!
 * \brief Constructs a new track for the \a stream at the specified \a startOffset.
 */
OggStream::OggStream(OggContainer &container, vector<OggPage>::size_type startPage)
    : AbstractTrack(container.stream(), container.m_iterator.pages()[startPage].startOffset())
    , m_startPage(startPage)
    , m_container(container)
    , m_currentSequenceNumber(0)
{
}

/*!
 * \brief Destroys the track.
 */
OggStream::~OggStream()
{
}

void OggStream::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing OGG page header");

    // read basic information from first page
    OggIterator &iterator = m_container.m_iterator;
    const OggPage &firstPage = iterator.pages()[m_startPage];
    m_version = firstPage.streamStructureVersion();
    m_id = firstPage.streamSerialNumber();

    // ensure iterator is setup properly
    iterator.setFilter(firstPage.streamSerialNumber());
    iterator.setPageIndex(m_startPage);

    // iterate through segments using OggIterator
    for (bool hasIdentificationHeader = false, hasCommentHeader = false; iterator && (!hasIdentificationHeader || !hasCommentHeader); ++iterator) {
        const std::uint32_t currentSize = iterator.currentSegmentSize();
        if (currentSize >= 8) {
            // determine stream format
            inputStream().seekg(static_cast<streamoff>(iterator.currentSegmentOffset()));
            const std::uint64_t sig = reader().readUInt64BE();

            if ((sig & 0x00ffffffffffff00u) == 0x00766F7262697300u) {
                // Vorbis header detected
                switch (m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Vorbis;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Vorbis:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning, "Stream format is inconsistent.", context);
                    continue;
                }

                // check header type
                switch (sig >> 56) {
                case VorbisPackageTypes::Identification:
                    if (!hasIdentificationHeader) {
                        // parse identification header
                        VorbisIdentificationHeader ind;
                        ind.parseHeader(iterator);
                        m_version = ind.version();
                        m_channelCount = ind.channels();
                        m_samplingFrequency = ind.sampleRate();
                        if (ind.nominalBitrate()) {
                            m_bitrate = ind.nominalBitrate();
                        } else if (ind.maxBitrate() == ind.minBitrate()) {
                            m_bitrate = ind.maxBitrate();
                        }
                        if (m_bitrate != 0.0) {
                            m_bitrate /= 1000.0;
                        }
                        calculateDurationViaSampleCount();
                        hasIdentificationHeader = true;
                    } else {
                        diag.emplace_back(DiagLevel::Critical,
                            "Vorbis identification header appears more than once. Oversupplied occurrence will be ignored.", context);
                    }
                    break;
                case VorbisPackageTypes::Comments:
                    // Vorbis comment found -> notify container about comment
                    if (!hasCommentHeader) {
                        m_container.announceComment(iterator.currentPageIndex(), iterator.currentSegmentIndex(), false, GeneralMediaFormat::Vorbis);
                        hasCommentHeader = true;
                    } else {
                        diag.emplace_back(
                            DiagLevel::Critical, "Vorbis comment header appears more than once. Oversupplied occurrence will be ignored.", context);
                    }
                    break;
                case VorbisPackageTypes::Setup:
                    break; // TODO
                default:;
                }

            } else if (sig == 0x4F70757348656164u) {
                // Opus header detected
                switch (m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Opus;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Opus:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning, "Stream format is inconsistent.", context);
                    continue;
                }
                if (!hasIdentificationHeader) {
                    // parse identification header
                    OpusIdentificationHeader ind;
                    ind.parseHeader(iterator);
                    m_version = ind.version();
                    m_channelCount = ind.channels();
                    m_samplingFrequency = ind.sampleRate();
                    calculateDurationViaSampleCount(ind.preSkip());
                    hasIdentificationHeader = true;
                } else {
                    diag.emplace_back(
                        DiagLevel::Critical, "Opus identification header appears more than once. Oversupplied occurrence will be ignored.", context);
                }

            } else if (sig == 0x4F70757354616773u) {
                // Opus comment detected
                switch (m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Opus;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Opus:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning, "Stream format is inconsistent.", context);
                    continue;
                }

                // notify container about comment
                if (!hasCommentHeader) {
                    m_container.announceComment(iterator.currentPageIndex(), iterator.currentSegmentIndex(), false, GeneralMediaFormat::Opus);
                    hasCommentHeader = true;
                } else {
                    diag.emplace_back(
                        DiagLevel::Critical, "Opus tags/comment header appears more than once. Oversupplied occurrence will be ignored.", context);
                }

            } else if ((sig & 0xFFFFFFFFFF000000u) == 0x7F464C4143000000u) {
                // FLAC header detected
                switch (m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Flac;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Flac:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning, "Stream format is inconsistent.", context);
                    continue;
                }

                if (!hasIdentificationHeader) {
                    // parse FLAC-to-Ogg mapping header
                    FlacToOggMappingHeader mapping;
                    const FlacMetaDataBlockStreamInfo &streamInfo = mapping.streamInfo();
                    mapping.parseHeader(iterator);
                    m_bitsPerSample = streamInfo.bitsPerSample();
                    m_channelCount = streamInfo.channelCount();
                    m_samplingFrequency = streamInfo.samplingFrequency();
                    m_sampleCount = streamInfo.totalSampleCount();
                    calculateDurationViaSampleCount();
                    hasIdentificationHeader = true;
                } else {
                    diag.emplace_back(
                        DiagLevel::Critical, "FLAC-to-Ogg mapping header appears more than once. Oversupplied occurrence will be ignored.", context);
                }

                if (!hasCommentHeader) {
                    // a Vorbis comment should be following
                    if (++iterator) {
                        constexpr auto headerSize = 4;
                        char buff[headerSize];
                        iterator.read(buff, headerSize);
                        FlacMetaDataBlockHeader header;
                        header.parseHeader(std::string_view(buff, headerSize));
                        if (header.type() == FlacMetaDataBlockType::VorbisComment) {
                            m_container.announceComment(
                                iterator.currentPageIndex(), iterator.currentSegmentIndex(), header.isLast(), GeneralMediaFormat::Flac);
                            hasCommentHeader = true;
                        } else {
                            diag.emplace_back(
                                DiagLevel::Critical, "OGG page after FLAC-to-Ogg mapping header doesn't contain Vorbis comment.", context);
                        }
                    } else {
                        diag.emplace_back(
                            DiagLevel::Critical, "No more OGG pages after FLAC-to-Ogg mapping header (Vorbis comment expected).", context);
                    }
                }

            } else if ((sig & 0x00ffffffffffff00u) == 0x007468656F726100u) {
                // Theora header detected
                switch (m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Theora;
                    m_mediaType = MediaType::Video;
                    break;
                case GeneralMediaFormat::Theora:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning, "Stream format is inconsistent.", context);
                    continue;
                }
                // TODO: read more information about Theora stream

            } else if ((sig & 0xFFFFFFFFFFFF0000u) == 0x5370656578200000u) {
                // Speex header detected
                switch (m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::Speex;
                    m_mediaType = MediaType::Audio;
                    break;
                case GeneralMediaFormat::Speex:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning, "Stream format is inconsistent.", context);
                    continue;
                }
                // TODO: read more information about Speex stream
            } else if (sig == 0x595556344D504547u) {
                // YUV4MPEG header detected
                switch (m_format.general) {
                case GeneralMediaFormat::Unknown:
                    m_format = GeneralMediaFormat::UncompressedVideoFrames;
                    m_mediaType = MediaType::Video;
                    m_chromaFormat = "YUV";
                    break;
                case GeneralMediaFormat::UncompressedVideoFrames:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning, "Stream format is inconsistent.", context);
                    continue;
                }
                // TODO: read more information about YUV4MPEG stream
            }
            // currently only Vorbis, Opus, Theora, Speex and YUV4MPEG can be detected, TODO: detect more formats

        } else {
            // just ignore segments of only 8 byte or even less
            // TODO: print warning?
        }

        // TODO: reduce code duplication
    }

    // estimate duration from size and bitrate if sample count and sample rate could not be determined
    if (m_duration.isNull() && m_size && m_bitrate != 0.0) {
        // calculate duration from stream size and bitrate, assuming 1 % overhead
        m_duration = TimeSpan::fromSeconds(static_cast<double>(m_size) / (m_bitrate * 125.0) * 1.1);
    }
}

void OggStream::calculateDurationViaSampleCount(std::uint16_t preSkip)
{
    // define predicate for finding pages of this stream by its stream serial number
    const auto pred = bind(&OggPage::matchesStreamSerialNumber, _1, m_id);

    // determine sample count
    const auto &iterator = m_container.m_iterator;
    if (!m_sampleCount && iterator.isLastPageFetched()) {
        const auto &pages = iterator.pages();
        const auto firstPage = find_if(pages.cbegin(), pages.cend(), pred);
        const auto lastPage = find_if(pages.crbegin(), pages.crend(), pred);
        if (firstPage != pages.cend() && lastPage != pages.crend()) {
            m_sampleCount = lastPage->absoluteGranulePosition() - firstPage->absoluteGranulePosition();
            // must apply "pre-skip" here to calculate effective sample count and duration?
            if (m_sampleCount > preSkip) {
                m_sampleCount -= preSkip;
            } else {
                m_sampleCount = 0;
            }
        }
    }

    // actually calculate the duration
    if (m_sampleCount && m_samplingFrequency != 0.0) {
        m_duration = TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / m_samplingFrequency);
    }
}

} // namespace TagParser
