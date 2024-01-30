#include "./mp4track.h"
#include "./mp4atom.h"
#include "./mp4container.h"
#include "./mp4ids.h"
#include "./mpeg4descriptor.h"

#include "../av1/av1configuration.h"

#include "../avc/avcconfiguration.h"

#include "../mpegaudio/mpegaudioframe.h"
#include "../mpegaudio/mpegaudioframestream.h"

#include "../exceptions.h"
#include "../mediafileinfo.h"
#include "../mediaformat.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/io/bitreader.h>

#include <cmath>
#include <locale>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \brief The Mp4Timings struct holds timing values found in multiple MP4 atoms.
 */
struct Mp4Timings {
    std::uint64_t tkhdCreationTime, mdhdCreationTime = 0;
    std::uint64_t tkhdModificationTime, mdhdModificationTime = 0;
    std::uint64_t tkhdDuration, mdhdDuration = 0;
    constexpr std::uint8_t requiredTkhdVersion() const;
    constexpr std::uint8_t requiredMdhdVersion() const;
};

/*!
 * \brief The TrackHeaderInfo struct holds information about the present track header (tkhd atom) and
 *        information for making a new track header based on it.
 * \sa TrackHeaderInfo Mp4Track::verifyPresentTrackHeader() for obtaining an instance.
 * \remarks The struct is only used internally by the Mp4Track class.
 */
struct TrackHeaderInfo {
    friend class Mp4Track;

private:
    /// \brief Specifies the size which is required for <i>making a new</i> track header based one the existing one.
    std::uint64_t requiredSize = 100;
    /// \brief Specifies whether there actually a track header exists and whether it can be used as basis for a new one.
    bool canUseExisting = false;
    /// \brief Specifies whether the existing track header is truncated.
    bool truncated = false;
    /// \brief Specifies the version of the existing track header.
    std::uint8_t version = 0;
    /// \brief Specifies the version the new track header is supposed to use.
    std::uint8_t writeVersion = 0;
    /// \brief Specifies whether the version of the existing track header is unknown (and assumed to be 1).
    bool versionUnknown = false;
    /// \brief Specifies timing values for the track.
    Mp4Timings timings;
    /// \brief Specifies the minimum required version for timings.
    std::uint8_t timingsVersion = 0;
    /// \brief Specifies the additional data offset of the existing header. Unspecified if canUseExisting is false.
    std::uint8_t additionalDataOffset = 0;
    /// \brief Specifies whether the buffered header data should be discarded when making a new track header.
    bool discardBuffer = false;
};

constexpr std::uint8_t Mp4Timings::requiredTkhdVersion() const
{
    return (tkhdCreationTime > std::numeric_limits<std::uint32_t>::max() || tkhdModificationTime > std::numeric_limits<std::uint32_t>::max()
               || tkhdDuration > std::numeric_limits<std::uint32_t>::max())
        ? 1
        : 0;
}

constexpr std::uint8_t Mp4Timings::requiredMdhdVersion() const
{
    return (mdhdCreationTime > std::numeric_limits<std::uint32_t>::max() || mdhdModificationTime > std::numeric_limits<std::uint32_t>::max()
               || mdhdDuration > std::numeric_limits<std::uint32_t>::max())
        ? 1
        : 0;
}

/*!
 * \class Mpeg4AudioSpecificConfig
 * \brief The Mpeg4AudioSpecificConfig class holds MPEG-4 audio specific config parsed using Mp4Track::parseAudioSpecificConfig().
 * \remarks Is part of Mpeg4ElementaryStreamInfo (audio streams only).
 */

Mpeg4AudioSpecificConfig::Mpeg4AudioSpecificConfig()
    : audioObjectType(0)
    , sampleFrequencyIndex(0xF)
    , sampleFrequency(0)
    , channelConfiguration(0)
    , extensionAudioObjectType(0)
    , sbrPresent(false)
    , psPresent(false)
    , extensionSampleFrequencyIndex(0xF)
    , extensionSampleFrequency(0)
    , extensionChannelConfiguration(0)
    , frameLengthFlag(false)
    , dependsOnCoreCoder(false)
    , coreCoderDelay(0)
    , extensionFlag(0)
    , layerNr(0)
    , numOfSubFrame(0)
    , layerLength(0)
    , resilienceFlags(0)
    , epConfig(0)
{
}

/*!
 * \class Mpeg4VideoSpecificConfig
 * \brief The Mpeg4VideoSpecificConfig class holds MPEG-4 video specific config parsed using Mp4Track::parseVideoSpecificConfig().
 * \remarks
 *  - Is part of Mpeg4ElementaryStreamInfo (video streams only).
 *  - AVC configuration is another thing and covered by the AvcConfiguration class.
 */

Mpeg4VideoSpecificConfig::Mpeg4VideoSpecificConfig()
    : profile(0)
{
}

/*!
 * \class Mpeg4ElementaryStreamInfo
 * \brief The Mpeg4ElementaryStreamInfo class holds MPEG-4 elementary stream info parsed using Mp4Track::parseMpeg4ElementaryStreamInfo().
 */

/*!
 * \class TagParser::Mp4Track
 * \brief Implementation of TagParser::AbstractTrack for the MP4 container.
 */

/*!
 * \brief Constructs a new track for the specified \a trakAtom.
 *
 * "trak"-atoms are stored in the top-level atom "move". Each "trak"-atom holds
 * header information for one track in the MP4 file.
 */
Mp4Track::Mp4Track(Mp4Atom &trakAtom)
    : AbstractTrack(trakAtom.stream(), trakAtom.startOffset())
    , m_trakAtom(&trakAtom)
    , m_tkhdAtom(nullptr)
    , m_mdiaAtom(nullptr)
    , m_mdhdAtom(nullptr)
    , m_hdlrAtom(nullptr)
    , m_minfAtom(nullptr)
    , m_stblAtom(nullptr)
    , m_stsdAtom(nullptr)
    , m_stscAtom(nullptr)
    , m_stcoAtom(nullptr)
    , m_stszAtom(nullptr)
    , m_rawMediaType(0)
    , m_framesPerSample(1)
    , m_chunkOffsetSize(4)
    , m_chunkCount(0)
    , m_sampleToChunkEntryCount(0)
    , m_rawTkhdCreationTime(0)
    , m_rawMdhdCreationTime(0)
    , m_rawTkhdModificationTime(0)
    , m_rawMdhdModificationTime(0)
    , m_rawTkhdDuration(0)
    , m_rawMdhdDuration(0)
{
}

/*!
 * \brief Destroys the track.
 */
Mp4Track::~Mp4Track()
{
}

TrackType Mp4Track::type() const
{
    return TrackType::Mp4Track;
}

/*!
 * \brief Reads the chunk offsets from the stco atom and fragments if \a parseFragments is true.
 * \returns Returns the chunk offset table for the track.
 * \throws Throws InvalidDataException when
 *          - there is no stream assigned.
 *          - the header has been considered as invalid when parsing the header information.
 *          - the determined chunk offset size is invalid.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \sa readChunkSizes();
 */
std::vector<std::uint64_t> Mp4Track::readChunkOffsets(bool parseFragments, Diagnostics &diag)
{
    static const string context("reading chunk offset table of MP4 track");
    if (!isHeaderValid() || !m_istream) {
        diag.emplace_back(DiagLevel::Critical, "Track has not been parsed.", context);
        throw InvalidDataException();
    }
    vector<std::uint64_t> offsets;
    if (m_stcoAtom) {
        // verify integrity of the chunk offset table
        std::uint64_t actualTableSize = m_stcoAtom->dataSize();
        if (actualTableSize < (8 + chunkOffsetSize())) {
            diag.emplace_back(DiagLevel::Critical, "The stco atom is truncated. There are no chunk offsets present.", context);
            throw InvalidDataException();
        } else {
            actualTableSize -= 8;
        }
        std::uint32_t actualChunkCount = chunkCount();
        std::uint64_t calculatedTableSize = chunkCount() * chunkOffsetSize();
        if (calculatedTableSize < actualTableSize) {
            diag.emplace_back(
                DiagLevel::Critical, "The stco atom stores more chunk offsets as denoted. The additional chunk offsets will be ignored.", context);
        } else if (calculatedTableSize > actualTableSize) {
            diag.emplace_back(DiagLevel::Critical, "The stco atom is truncated. It stores less chunk offsets as denoted.", context);
            actualChunkCount = static_cast<std::uint32_t>(floor(static_cast<double>(actualTableSize) / static_cast<double>(chunkOffsetSize())));
        }
        // read the table
        offsets.reserve(actualChunkCount);
        m_istream->seekg(static_cast<streamoff>(m_stcoAtom->dataOffset() + 8));
        switch (chunkOffsetSize()) {
        case 4:
            for (std::uint32_t i = 0; i < actualChunkCount; ++i) {
                offsets.push_back(reader().readUInt32BE());
            }
            break;
        case 8:
            for (std::uint32_t i = 0; i < actualChunkCount; ++i) {
                offsets.push_back(reader().readUInt64BE());
            }
            break;
        default:
            diag.emplace_back(DiagLevel::Critical, "The determined chunk offset size is invalid.", context);
            throw InvalidDataException();
        }
    }
    // read sample offsets of fragments
    if (parseFragments) {
        //std::uint64_t totalDuration = 0;
        for (Mp4Atom *moofAtom = m_trakAtom->container().firstElement()->siblingByIdIncludingThis(Mp4AtomIds::MovieFragment, diag); moofAtom;
             moofAtom = moofAtom->siblingById(Mp4AtomIds::MovieFragment, diag)) {
            moofAtom->parse(diag);
            for (Mp4Atom *trafAtom = moofAtom->childById(Mp4AtomIds::TrackFragment, diag); trafAtom;
                 trafAtom = trafAtom->siblingById(Mp4AtomIds::TrackFragment, diag)) {
                trafAtom->parse(diag);
                for (Mp4Atom *tfhdAtom = trafAtom->childById(Mp4AtomIds::TrackFragmentHeader, diag); tfhdAtom;
                     tfhdAtom = tfhdAtom->siblingById(Mp4AtomIds::TrackFragmentHeader, diag)) {
                    tfhdAtom->parse(diag);
                    std::uint32_t calculatedDataSize = 0;
                    if (tfhdAtom->dataSize() < calculatedDataSize) {
                        diag.emplace_back(DiagLevel::Critical, "tfhd atom is truncated.", context);
                    } else {
                        inputStream().seekg(static_cast<streamoff>(tfhdAtom->dataOffset() + 1));
                        const std::uint32_t flags = reader().readUInt24BE();
                        if (m_id == reader().readUInt32BE()) { // check track ID
                            if (flags & 0x000001) { // base-data-offset present
                                calculatedDataSize += 8;
                            }
                            if (flags & 0x000002) { // sample-description-index present
                                calculatedDataSize += 4;
                            }
                            if (flags & 0x000008) { // default-sample-duration present
                                calculatedDataSize += 4;
                            }
                            if (flags & 0x000010) { // default-sample-size present
                                calculatedDataSize += 4;
                            }
                            if (flags & 0x000020) { // default-sample-flags present
                                calculatedDataSize += 4;
                            }
                            // some variables are currently skipped because they are currently not interesting
                            //std::uint64_t baseDataOffset = moofAtom->startOffset();
                            //std::uint32_t defaultSampleDescriptionIndex = 0;
                            //std::uint32_t defaultSampleDuration = 0;
                            std::uint32_t defaultSampleSize = 0;
                            //std::uint32_t defaultSampleFlags = 0;
                            if (tfhdAtom->dataSize() < calculatedDataSize) {
                                diag.emplace_back(DiagLevel::Critical, "tfhd atom is truncated (presence of fields denoted).", context);
                            } else {
                                if (flags & 0x000001) { // base-data-offset present
                                    //baseDataOffset = reader.readUInt64();
                                    inputStream().seekg(8, ios_base::cur);
                                }
                                if (flags & 0x000002) { // sample-description-index present
                                    //defaultSampleDescriptionIndex = reader.readUInt32();
                                    inputStream().seekg(4, ios_base::cur);
                                }
                                if (flags & 0x000008) { // default-sample-duration present
                                    //defaultSampleDuration = reader().readUInt32BE();
                                    inputStream().seekg(4, ios_base::cur);
                                }
                                if (flags & 0x000010) { // default-sample-size present
                                    defaultSampleSize = reader().readUInt32BE();
                                }
                                if (flags & 0x000020) { // default-sample-flags present
                                    //defaultSampleFlags = reader().readUInt32BE();
                                    inputStream().seekg(4, ios_base::cur);
                                }
                            }
                            for (Mp4Atom *trunAtom = trafAtom->childById(Mp4AtomIds::TrackFragmentRun, diag); trunAtom;
                                 trunAtom = trunAtom->siblingById(Mp4AtomIds::TrackFragmentRun, diag)) {
                                std::uint32_t trunCalculatedDataSize = 8;
                                if (trunAtom->dataSize() < trunCalculatedDataSize) {
                                    diag.emplace_back(DiagLevel::Critical, "trun atom is truncated.", context);
                                } else {
                                    inputStream().seekg(static_cast<streamoff>(trunAtom->dataOffset() + 1));
                                    std::uint32_t trunFlags = reader().readUInt24BE();
                                    std::uint32_t sampleCount = reader().readUInt32BE();
                                    m_sampleCount += sampleCount;
                                    if (trunFlags & 0x000001) { // data offset present
                                        trunCalculatedDataSize += 4;
                                    }
                                    if (trunFlags & 0x000004) { // first-sample-flags present
                                        trunCalculatedDataSize += 4;
                                    }
                                    std::uint32_t entrySize = 0;
                                    if (trunFlags & 0x000100) { // sample-duration present
                                        entrySize += 4;
                                    }
                                    if (trunFlags & 0x000200) { // sample-size present
                                        entrySize += 4;
                                    }
                                    if (trunFlags & 0x000400) { // sample-flags present
                                        entrySize += 4;
                                    }
                                    if (trunFlags & 0x000800) { // sample-composition-time-offsets present
                                        entrySize += 4;
                                    }
                                    trunCalculatedDataSize += entrySize * sampleCount;
                                    if (trunAtom->dataSize() < trunCalculatedDataSize) {
                                        diag.emplace_back(DiagLevel::Critical, "trun atom is truncated (presence of fields denoted).", context);
                                    } else {
                                        if (trunFlags & 0x000001) { // data offset present
                                            inputStream().seekg(4, ios_base::cur);
                                            //int32 dataOffset = reader().readInt32BE();
                                        }
                                        if (trunFlags & 0x000004) { // first-sample-flags present
                                            inputStream().seekg(4, ios_base::cur);
                                        }
                                        for (std::uint32_t i = 0; i < sampleCount; ++i) {
                                            if (trunFlags & 0x000100) { // sample-duration present
                                                //totalDuration += reader().readUInt32BE();
                                                inputStream().seekg(4, ios_base::cur);
                                            } else {
                                                //totalDuration += defaultSampleDuration;
                                            }
                                            if (trunFlags & 0x000200) { // sample-size present
                                                m_sampleSizes.push_back(reader().readUInt32BE());
                                                m_size += m_sampleSizes.back();
                                            } else {
                                                m_size += defaultSampleSize;
                                            }
                                            if (trunFlags & 0x000400) { // sample-flags present
                                                inputStream().seekg(4, ios_base::cur);
                                            }
                                            if (trunFlags & 0x000800) { // sample-composition-time-offsets present
                                                inputStream().seekg(4, ios_base::cur);
                                            }
                                        }
                                    }
                                }
                            }
                            if (m_sampleSizes.empty() && defaultSampleSize) {
                                m_sampleSizes.push_back(defaultSampleSize);
                            }
                        }
                    }
                }
            }
        }
    }
    return offsets;
}

/*!
 * \brief Accumulates \a count sample sizes from the specified \a sampleSizeTable starting at the specified \a sampleIndex.
 * \remarks This helper function is used by the addChunkSizeEntries() method.
 */
std::uint64_t Mp4Track::accumulateSampleSizes(size_t &sampleIndex, size_t count, Diagnostics &diag)
{
    if (sampleIndex + count <= m_sampleSizes.size()) {
        std::uint64_t sum = 0;
        for (size_t end = sampleIndex + count; sampleIndex < end; ++sampleIndex) {
            sum += m_sampleSizes[sampleIndex];
        }
        return sum;
    } else if (m_sampleSizes.size() == 1) {
        sampleIndex += count;
        return static_cast<std::uint64_t>(m_sampleSizes.front()) * count;
    } else {
        diag.emplace_back(DiagLevel::Critical, "There are not as many sample size entries as samples.", "reading chunk sizes of MP4 track");
        throw InvalidDataException();
    }
}

/*!
 * \brief Adds chunks size entries to the specified \a chunkSizeTable.
 * \param chunkSizeTable Specifies the chunk size table. The chunks sizes will be added to this table.
 * \param count Specifies the number of chunks to be added. The size of \a chunkSizeTable is increased this value.
 * \param sampleIndex Specifies the index of the first sample in the \a sampleSizeTable; is increased by \a count * \a sampleCount.
 * \param sampleSizeTable Specifies the table holding the sample sizes.
 * \remarks This helper function is used by the readChunkSizes() method.
 */
void Mp4Track::addChunkSizeEntries(
    std::vector<std::uint64_t> &chunkSizeTable, size_t count, size_t &sampleIndex, std::uint32_t sampleCount, Diagnostics &diag)
{
    for (size_t i = 0; i < count; ++i) {
        chunkSizeTable.push_back(accumulateSampleSizes(sampleIndex, sampleCount, diag));
    }
}

/*!
 * \brief Verifies the present track header (tkhd atom) and returns relevant information for making a new track header
 *        based on it.
 */
const TrackHeaderInfo &Mp4Track::verifyPresentTrackHeader() const
{
    if (m_trackHeaderInfo) {
        return *m_trackHeaderInfo;
    }

    // return the default TrackHeaderInfo in case there is no track header prsent
    auto &info = *(m_trackHeaderInfo = std::make_unique<TrackHeaderInfo>());
    if (!m_tkhdAtom) {
        return info;
    }

    // ensure the tkhd atom is buffered but mark the buffer to be discarded again if it has not been present
    info.discardBuffer = m_tkhdAtom->buffer() == nullptr;
    if (info.discardBuffer) {
        m_tkhdAtom->makeBuffer();
    }

    // check the version of the existing tkhd atom to determine where additional data starts
    switch (info.version = static_cast<std::uint8_t>(m_tkhdAtom->buffer()[m_tkhdAtom->headerSize()])) {
    case 0:
        info.additionalDataOffset = 32;
        break;
    case 1:
        info.additionalDataOffset = 44;
        break;
    default:
        info.additionalDataOffset = 44;
        info.versionUnknown = true;
    }

    // check whether the existing tkhd atom is not truncated
    if (info.additionalDataOffset + 48u <= m_tkhdAtom->dataSize()) {
        info.canUseExisting = true;
    } else {
        info.truncated = true;
        info.canUseExisting = info.additionalDataOffset < m_tkhdAtom->dataSize();
        if (!info.canUseExisting && info.discardBuffer) {
            m_tkhdAtom->discardBuffer();
        }
    }

    // determine required size
    info.requiredSize = m_tkhdAtom->dataSize() + 8;
    info.timings = computeTimings();
    info.timingsVersion = info.timings.requiredTkhdVersion();
    if (info.version == 0) {
        info.writeVersion = info.timingsVersion;
        // add 12 byte to size if update from version 0 to version 1 is required (which needs 12 byte more)
        if (info.writeVersion != 0) {
            info.requiredSize += 12;
        }
    } else {
        info.writeVersion = info.version;
    }
    // -> add 8 byte to the size because it must be denoted using a 64-bit integer
    if (info.requiredSize > numeric_limits<std::uint32_t>::max()) {
        info.requiredSize += 8;
    }
    return info;
}

/*!
 * \brief Computes timing values for the track.
 */
Mp4Timings Mp4Track::computeTimings() const
{
    auto timings = Mp4Timings();
    if (m_trakAtom && (m_trakAtom->container().fileInfo().fileHandlingFlags() & MediaFileHandlingFlags::PreserveRawTimingValues)) {
        timings.tkhdCreationTime = m_rawTkhdCreationTime;
        timings.tkhdModificationTime = m_rawTkhdModificationTime;
        timings.tkhdDuration = m_rawTkhdDuration;
        timings.mdhdCreationTime = m_rawMdhdCreationTime;
        timings.mdhdModificationTime = m_rawMdhdModificationTime;
        timings.mdhdDuration = m_rawMdhdDuration;
    } else {
        timings.tkhdCreationTime = timings.mdhdCreationTime = static_cast<std::uint64_t>((m_creationTime - Mp4Container::epoch).totalSeconds());
        timings.tkhdModificationTime = timings.mdhdModificationTime
            = static_cast<std::uint64_t>((m_modificationTime - Mp4Container::epoch).totalSeconds());
        timings.tkhdDuration = timings.mdhdDuration = static_cast<std::uint64_t>(m_duration.totalTicks() * m_timeScale / TimeSpan::ticksPerSecond);
    }
    return timings;
}

/*!
 * \brief Reads the sample to chunk table.
 * \returns Returns a vector with the table entries wrapped using the tuple container. The first value
 *          is an integer that gives the first chunk that share the same samples count and sample description index.
 *          The second value is sample count and the third value is the sample description index.
 * \remarks The table is not validated.
 */
vector<tuple<std::uint32_t, std::uint32_t, std::uint32_t>> Mp4Track::readSampleToChunkTable(Diagnostics &diag)
{
    static const string context("reading sample to chunk table of MP4 track");
    if (!isHeaderValid() || !m_istream || !m_stscAtom) {
        diag.emplace_back(DiagLevel::Critical, "Track has not been parsed or is invalid.", context);
        throw InvalidDataException();
    }
    // verify integrity of the sample to chunk table
    std::uint64_t actualTableSize = m_stscAtom->dataSize();
    if (actualTableSize < 20) {
        diag.emplace_back(DiagLevel::Critical, "The stsc atom is truncated. There are no \"sample to chunk\" entries present.", context);
        throw InvalidDataException();
    } else {
        actualTableSize -= 8;
    }
    std::uint64_t actualSampleToChunkEntryCount = sampleToChunkEntryCount();
    std::uint64_t calculatedTableSize = actualSampleToChunkEntryCount * 12;
    if (calculatedTableSize < actualTableSize) {
        diag.emplace_back(DiagLevel::Critical, "The stsc atom stores more entries as denoted. The additional entries will be ignored.", context);
    } else if (calculatedTableSize > actualTableSize) {
        diag.emplace_back(DiagLevel::Critical, "The stsc atom is truncated. It stores less entries as denoted.", context);
        actualSampleToChunkEntryCount = actualTableSize / 12;
    }
    // prepare reading
    vector<tuple<std::uint32_t, std::uint32_t, std::uint32_t>> sampleToChunkTable;
    sampleToChunkTable.reserve(actualSampleToChunkEntryCount);
    m_istream->seekg(static_cast<streamoff>(m_stscAtom->dataOffset() + 8));
    for (std::uint32_t i = 0; i < actualSampleToChunkEntryCount; ++i) {
        // read entry
        std::uint32_t firstChunk = reader().readUInt32BE();
        std::uint32_t samplesPerChunk = reader().readUInt32BE();
        std::uint32_t sampleDescriptionIndex = reader().readUInt32BE();
        sampleToChunkTable.emplace_back(firstChunk, samplesPerChunk, sampleDescriptionIndex);
    }
    return sampleToChunkTable;
}

/*!
 * \brief Reads the chunk sizes from the stsz (sample sizes) and stsc (samples per chunk) atom.
 * \returns Returns the chunk sizes for the track.
 *
 * \throws Throws InvalidDataException when
 *          - there is no stream assigned.
 *          - the header has been considered as invalid when parsing the header information.
 *          - the determined chunk offset size is invalid.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 *
 * \sa readChunkOffsets();
 */
vector<std::uint64_t> Mp4Track::readChunkSizes(Diagnostics &diag)
{
    static const string context("reading chunk sizes of MP4 track");
    if (!isHeaderValid() || !m_istream || !m_stcoAtom) {
        diag.emplace_back(DiagLevel::Critical, "Track has not been parsed or is invalid.", context);
        throw InvalidDataException();
    }
    // read sample to chunk table
    const auto sampleToChunkTable = readSampleToChunkTable(diag);
    // accumulate chunk sizes from the table
    vector<std::uint64_t> chunkSizes;
    if (!sampleToChunkTable.empty()) {
        // prepare reading
        auto tableIterator = sampleToChunkTable.cbegin();
        chunkSizes.reserve(m_chunkCount);
        // read first entry
        size_t sampleIndex = 0;
        std::uint32_t previousChunkIndex = get<0>(*tableIterator); // the first chunk has the index 1 and not zero!
        if (previousChunkIndex != 1) {
            diag.emplace_back(DiagLevel::Critical, "The first chunk of the first \"sample to chunk\" entry must be 1.", context);
            previousChunkIndex = 1; // try to read the entry anyway
        }
        std::uint32_t samplesPerChunk = get<1>(*tableIterator);
        // read the following entries
        ++tableIterator;
        for (const auto tableEnd = sampleToChunkTable.cend(); tableIterator != tableEnd; ++tableIterator) {
            std::uint32_t firstChunkIndex = get<0>(*tableIterator);
            if (firstChunkIndex > previousChunkIndex && firstChunkIndex <= m_chunkCount) {
                addChunkSizeEntries(chunkSizes, firstChunkIndex - previousChunkIndex, sampleIndex, samplesPerChunk, diag);
            } else {
                diag.emplace_back(DiagLevel::Critical,
                    "The first chunk index of a \"sample to chunk\" entry must be greater than the first chunk of the previous entry and not "
                    "greater than the chunk count.",
                    context);
                throw InvalidDataException();
            }
            previousChunkIndex = firstChunkIndex;
            samplesPerChunk = get<1>(*tableIterator);
        }
        if (m_chunkCount >= previousChunkIndex) {
            addChunkSizeEntries(chunkSizes, m_chunkCount + 1 - previousChunkIndex, sampleIndex, samplesPerChunk, diag);
        }
    }
    return chunkSizes;
}

/*!
 * \brief Reads the MPEG-4 elementary stream descriptor for the track.
 * \sa mpeg4ElementaryStreamInfo()
 */
std::unique_ptr<Mpeg4ElementaryStreamInfo> Mp4Track::parseMpeg4ElementaryStreamInfo(
    CppUtilities::BinaryReader &reader, Mp4Atom *esDescAtom, Diagnostics &diag)
{
    static const string context("parsing MPEG-4 elementary stream descriptor");
    using namespace Mpeg4ElementaryStreamObjectIds;
    unique_ptr<Mpeg4ElementaryStreamInfo> esInfo;
    if (esDescAtom->dataSize() >= 12) {
        reader.stream()->seekg(static_cast<streamoff>(esDescAtom->dataOffset()));
        // read version/flags
        if (reader.readUInt32BE() != 0) {
            diag.emplace_back(DiagLevel::Warning, "Unknown version/flags.", context);
        }
        // read extended descriptor
        Mpeg4Descriptor esDesc(esDescAtom->container(), static_cast<std::uint64_t>(reader.stream()->tellg()), esDescAtom->dataSize() - 4);
        try {
            esDesc.parse(diag);
            // check ID
            if (esDesc.id() != Mpeg4DescriptorIds::ElementaryStreamDescr) {
                diag.emplace_back(DiagLevel::Critical, "Invalid descriptor found.", context);
                throw Failure();
            }
            // read stream info
            reader.stream()->seekg(static_cast<streamoff>(esDesc.dataOffset()));
            esInfo = make_unique<Mpeg4ElementaryStreamInfo>();
            esInfo->id = reader.readUInt16BE();
            esInfo->esDescFlags = reader.readByte();
            if (esInfo->dependencyFlag()) {
                esInfo->dependsOnId = reader.readUInt16BE();
            }
            if (esInfo->urlFlag()) {
                esInfo->url = reader.readString(reader.readByte());
            }
            if (esInfo->ocrFlag()) {
                esInfo->ocrId = reader.readUInt16BE();
            }
            for (Mpeg4Descriptor *esDescChild
                 = esDesc.denoteFirstChild(static_cast<std::uint32_t>(static_cast<std::uint64_t>(reader.stream()->tellg()) - esDesc.startOffset()));
                 esDescChild; esDescChild = esDescChild->nextSibling()) {
                esDescChild->parse(diag);
                switch (esDescChild->id()) {
                case Mpeg4DescriptorIds::DecoderConfigDescr:
                    // read decoder config descriptor
                    reader.stream()->seekg(static_cast<streamoff>(esDescChild->dataOffset()));
                    esInfo->objectTypeId = reader.readByte();
                    esInfo->decCfgDescFlags = reader.readByte();
                    esInfo->bufferSize = reader.readUInt24BE();
                    esInfo->maxBitrate = reader.readUInt32BE();
                    esInfo->averageBitrate = reader.readUInt32BE();
                    for (Mpeg4Descriptor *decCfgDescChild = esDescChild->denoteFirstChild(esDescChild->headerSize() + 13); decCfgDescChild;
                         decCfgDescChild = decCfgDescChild->nextSibling()) {
                        decCfgDescChild->parse(diag);
                        switch (decCfgDescChild->id()) {
                        case Mpeg4DescriptorIds::DecoderSpecificInfo:
                            // read decoder specific info
                            switch (esInfo->objectTypeId) {
                            case Aac:
                            case Mpeg2AacMainProfile:
                            case Mpeg2AacLowComplexityProfile:
                            case Mpeg2AacScaleableSamplingRateProfile:
                            case Mpeg2Audio:
                            case Mpeg1Audio:
                                esInfo->audioSpecificConfig
                                    = parseAudioSpecificConfig(*reader.stream(), decCfgDescChild->dataOffset(), decCfgDescChild->dataSize(), diag);
                                break;
                            case Mpeg4Visual:
                                esInfo->videoSpecificConfig
                                    = parseVideoSpecificConfig(reader, decCfgDescChild->dataOffset(), decCfgDescChild->dataSize(), diag);
                                break;
                            default:; // TODO: cover more object types
                            }
                            break;
                        }
                    }
                    break;
                case Mpeg4DescriptorIds::SlConfigDescr:
                    // uninteresting
                    break;
                }
            }
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "The MPEG-4 descriptor element structure is invalid.", context);
        }
    } else {
        diag.emplace_back(DiagLevel::Warning, "Elementary stream descriptor atom (esds) is truncated.", context);
    }
    return esInfo;
}

/*!
 * \brief Parses the audio specific configuration for the track.
 * \sa mpeg4ElementaryStreamInfo()
 */
unique_ptr<Mpeg4AudioSpecificConfig> Mp4Track::parseAudioSpecificConfig(
    istream &stream, std::uint64_t startOffset, std::uint64_t size, Diagnostics &diag)
{
    static const string context("parsing MPEG-4 audio specific config from elementary stream descriptor");
    using namespace Mpeg4AudioObjectIds;
    // read config into buffer and construct BitReader for bitwise reading
    stream.seekg(static_cast<streamoff>(startOffset));
    auto buff = make_unique<char[]>(size);
    stream.read(buff.get(), static_cast<streamoff>(size));
    BitReader bitReader(buff.get(), size);
    auto audioCfg = make_unique<Mpeg4AudioSpecificConfig>();
    try {
        // read audio object type
        auto getAudioObjectType = [&bitReader] {
            std::uint8_t objType = bitReader.readBits<std::uint8_t>(5);
            if (objType == 31) {
                objType = 32 + bitReader.readBits<std::uint8_t>(6);
            }
            return objType;
        };
        audioCfg->audioObjectType = getAudioObjectType();
        // read sampling frequency
        if ((audioCfg->sampleFrequencyIndex = bitReader.readBits<std::uint8_t>(4)) == 0xF) {
            audioCfg->sampleFrequency = bitReader.readBits<std::uint32_t>(24);
        }
        // read channel config
        audioCfg->channelConfiguration = bitReader.readBits<std::uint8_t>(4);
        // read extension header
        switch (audioCfg->audioObjectType) {
        case Sbr:
        case Ps:
            audioCfg->extensionAudioObjectType = audioCfg->audioObjectType;
            audioCfg->sbrPresent = true;
            if ((audioCfg->extensionSampleFrequencyIndex = bitReader.readBits<std::uint8_t>(4)) == 0xF) {
                audioCfg->extensionSampleFrequency = bitReader.readBits<std::uint32_t>(24);
            }
            if ((audioCfg->audioObjectType = getAudioObjectType()) == ErBsac) {
                audioCfg->extensionChannelConfiguration = bitReader.readBits<std::uint8_t>(4);
            }
            break;
        }
        switch (audioCfg->extensionAudioObjectType) {
        case Ps:
            audioCfg->psPresent = true;
            audioCfg->extensionChannelConfiguration = Mpeg4ChannelConfigs::FrontLeftFrontRight;
            break;
        }
        // read GA specific config
        switch (audioCfg->audioObjectType) {
        case AacMain:
        case AacLc:
        case AacLtp:
        case AacScalable:
        case TwinVq:
        case ErAacLc:
        case ErAacLtp:
        case ErAacScalable:
        case ErTwinVq:
        case ErBsac:
        case ErAacLd:
            audioCfg->frameLengthFlag = bitReader.readBits<std::uint8_t>(1);
            if ((audioCfg->dependsOnCoreCoder = bitReader.readBit())) {
                audioCfg->coreCoderDelay = bitReader.readBits<std::uint8_t>(14);
            }
            audioCfg->extensionFlag = bitReader.readBit();
            if (audioCfg->channelConfiguration == 0) {
                throw NotImplementedException(); // TODO: parse program_config_element
            }
            switch (audioCfg->audioObjectType) {
            case AacScalable:
            case ErAacScalable:
                audioCfg->layerNr = bitReader.readBits<std::uint8_t>(3);
                break;
            default:;
            }
            if (audioCfg->extensionFlag == 1) {
                switch (audioCfg->audioObjectType) {
                case ErBsac:
                    audioCfg->numOfSubFrame = bitReader.readBits<std::uint8_t>(5);
                    audioCfg->layerLength = bitReader.readBits<std::uint16_t>(11);
                    break;
                case ErAacLc:
                case ErAacLtp:
                case ErAacScalable:
                case ErAacLd:
                    audioCfg->resilienceFlags = bitReader.readBits<std::uint8_t>(3);
                    break;
                default:;
                }
                if (bitReader.readBit() == 1) { // extension flag 3
                    throw NotImplementedException(); // TODO
                }
            }
            break;
        default:
            throw NotImplementedException(); // TODO: cover remaining object types
        }
        // read error specific config
        switch (audioCfg->audioObjectType) {
        case ErAacLc:
        case ErAacLtp:
        case ErAacScalable:
        case ErTwinVq:
        case ErBsac:
        case ErAacLd:
        case ErCelp:
        case ErHvxc:
        case ErHiln:
        case ErParametric:
        case ErAacEld:
            switch (audioCfg->epConfig = bitReader.readBits<std::uint8_t>(2)) {
            case 2:
                break;
            case 3:
                bitReader.skipBits(1);
                break;
            default:
                throw NotImplementedException(); // TODO
            }
            break;
        }
        if (audioCfg->extensionAudioObjectType != Sbr && audioCfg->extensionAudioObjectType != Ps && bitReader.bitsAvailable() >= 16) {
            std::uint16_t syncExtensionType = bitReader.readBits<std::uint16_t>(11);
            if (syncExtensionType == 0x2B7) {
                if ((audioCfg->extensionAudioObjectType = getAudioObjectType()) == Sbr) {
                    if ((audioCfg->sbrPresent = bitReader.readBit())) {
                        if ((audioCfg->extensionSampleFrequencyIndex = bitReader.readBits<std::uint8_t>(4)) == 0xF) {
                            audioCfg->extensionSampleFrequency = bitReader.readBits<std::uint32_t>(24);
                        }
                        if (bitReader.bitsAvailable() >= 12) {
                            if ((syncExtensionType = bitReader.readBits<std::uint16_t>(11)) == 0x548) {
                                audioCfg->psPresent = bitReader.readBits<std::uint8_t>(1);
                            }
                        }
                    }
                } else if (audioCfg->extensionAudioObjectType == ErBsac) {
                    if ((audioCfg->sbrPresent = bitReader.readBit())) {
                        if ((audioCfg->extensionSampleFrequencyIndex = bitReader.readBits<std::uint8_t>(4)) == 0xF) {
                            audioCfg->extensionSampleFrequency = bitReader.readBits<std::uint32_t>(24);
                        }
                    }
                    audioCfg->extensionChannelConfiguration = bitReader.readBits<std::uint8_t>(4);
                }
            } else if (syncExtensionType == 0x548) {
                audioCfg->psPresent = bitReader.readBit();
            }
        }
    } catch (const NotImplementedException &) {
        diag.emplace_back(DiagLevel::Information, "Not implemented for the format of audio track.", context);
    } catch (const std::ios_base::failure &) {
        if (stream.fail()) {
            // IO error caused by input stream
            throw;
        } else {
            // IO error caused by bitReader
            diag.emplace_back(DiagLevel::Critical, "Audio specific configuration is truncated.", context);
        }
    }
    return audioCfg;
}

/*!
 * \brief Parses the video specific configuration for the track.
 * \sa mpeg4ElementaryStreamInfo()
 */
std::unique_ptr<Mpeg4VideoSpecificConfig> Mp4Track::parseVideoSpecificConfig(
    BinaryReader &reader, std::uint64_t startOffset, std::uint64_t size, Diagnostics &diag)
{
    static const string context("parsing MPEG-4 video specific config from elementary stream descriptor");
    using namespace Mpeg4AudioObjectIds;
    auto videoCfg = make_unique<Mpeg4VideoSpecificConfig>();
    // seek to start
    reader.stream()->seekg(static_cast<streamoff>(startOffset));
    if (size > 3 && (reader.readUInt24BE() == 1)) {
        size -= 3;
        std::uint32_t buff1;
        while (size) {
            --size;
            switch (reader.readByte()) { // read start code
            case Mpeg4VideoCodes::VisualObjectSequenceStart:
                if (size) {
                    videoCfg->profile = reader.readByte();
                    --size;
                }
                break;
            case Mpeg4VideoCodes::VideoObjectLayerStart:

                break;
            case Mpeg4VideoCodes::UserDataStart:
                buff1 = 0;
                while (size >= 3) {
                    if ((buff1 = reader.readUInt24BE()) != 1) {
                        reader.stream()->seekg(-2, ios_base::cur);
                        videoCfg->userData.push_back(static_cast<char>(buff1 >> 16));
                        --size;
                    } else {
                        size -= 3;
                        break;
                    }
                }
                if (buff1 != 1 && size > 0) {
                    videoCfg->userData += reader.readString(size);
                    size = 0;
                }
                break;
            default:;
            }
            // skip remaining values to get the start of the next video object
            while (size >= 3) {
                if (reader.readUInt24BE() != 1) {
                    reader.stream()->seekg(-2, ios_base::cur);
                    --size;
                } else {
                    size -= 3;
                    break;
                }
            }
        }
    } else {
        diag.emplace_back(DiagLevel::Critical, "\"Visual Object Sequence Header\" not found.", context);
    }
    return videoCfg;
}

/*!
 * \brief Updates the chunk offsets of the track. This is necessary when the "mdat"-atom
 *        (which contains the actual chunk data) is moved.
 * \param oldMdatOffsets Specifies a vector holding the old offsets of the "mdat"-atoms.
 * \param newMdatOffsets Specifies a vector holding the new offsets of the "mdat"-atoms.
 *
 * \throws Throws InvalidDataException when
 *          - there is no stream assigned.
 *          - the header has been considered as invalid when parsing the header information.
 *          - \a oldMdatOffsets holds not the same number of offsets as \a newMdatOffsets.
 *          - there is no atom holding these offsets.
 *          - the ID of the atom holding these offsets is not "stco" or "co64"
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 *
 * \remarks This method needs to be fixed.
 */
void Mp4Track::updateChunkOffsets(const vector<std::int64_t> &oldMdatOffsets, const vector<std::int64_t> &newMdatOffsets)
{
    if (!isHeaderValid() || !m_ostream || !m_istream || !m_stcoAtom) {
        throw InvalidDataException();
    }
    if (oldMdatOffsets.size() == 0 || oldMdatOffsets.size() != newMdatOffsets.size()) {
        throw InvalidDataException();
    }
    static const unsigned int stcoDataBegin = 8;
    std::uint64_t startPos = m_stcoAtom->dataOffset() + stcoDataBegin;
    std::uint64_t endPos = startPos + m_stcoAtom->dataSize() - stcoDataBegin;
    m_istream->seekg(static_cast<streamoff>(startPos));
    m_ostream->seekp(static_cast<streamoff>(startPos));
    vector<std::int64_t>::size_type i;
    vector<std::int64_t>::size_type size;
    auto currentPos = static_cast<std::uint64_t>(m_istream->tellg());
    switch (m_stcoAtom->id()) {
    case Mp4AtomIds::ChunkOffset: {
        std::uint32_t off;
        while ((currentPos + 4) <= endPos) {
            off = m_reader.readUInt32BE();
            for (i = 0, size = oldMdatOffsets.size(); i < size; ++i) {
                if (off > static_cast<std::uint64_t>(oldMdatOffsets[i])) {
                    off += static_cast<std::uint32_t>(newMdatOffsets[i] - oldMdatOffsets[i]);
                    break;
                }
            }
            m_ostream->seekp(static_cast<streamoff>(currentPos));
            m_writer.writeUInt32BE(off);
            currentPos += static_cast<std::uint64_t>(m_istream->gcount());
        }
        break;
    }
    case Mp4AtomIds::ChunkOffset64: {
        std::uint64_t off;
        while ((currentPos + 8) <= endPos) {
            off = m_reader.readUInt64BE();
            for (i = 0, size = oldMdatOffsets.size(); i < size; ++i) {
                if (off > static_cast<std::uint64_t>(oldMdatOffsets[i])) {
                    off += static_cast<std::uint64_t>(newMdatOffsets[i] - oldMdatOffsets[i]);
                    break;
                }
            }
            m_ostream->seekp(static_cast<streamoff>(currentPos));
            m_writer.writeUInt64BE(off);
            currentPos += static_cast<std::uint64_t>(m_istream->gcount());
        }
        break;
    }
    default:
        throw InvalidDataException();
    }
}

/*!
 * \brief Updates the chunk offsets of the track. This is necessary when the "mdat"-atom
 *        (which contains the actual chunk data) is moved.
 * \param chunkOffsets Specifies the new chunk offset table. If the "stco" atom is used the values
 *                     must fit into an 32-bit unsigned int.
 *
 * \throws Throws InvalidDataException when
 *          - there is no stream assigned.
 *          - the header has been considered as invalid when parsing the header information.
 *          - the size of \a chunkOffsets does not match chunkCount().
 *          - there is no atom holding these offsets.
 *          - the ID of the atom holding these offsets is not "stco" or "co64".
 */
void Mp4Track::updateChunkOffsets(const std::vector<std::uint64_t> &chunkOffsets)
{
    if (!isHeaderValid() || !m_ostream || !m_istream || !m_stcoAtom) {
        throw InvalidDataException();
    }
    if (chunkOffsets.size() != chunkCount()) {
        throw InvalidDataException();
    }
    m_ostream->seekp(static_cast<streamoff>(m_stcoAtom->dataOffset() + 8));
    switch (m_stcoAtom->id()) {
    case Mp4AtomIds::ChunkOffset:
        for (auto offset : chunkOffsets) {
            m_writer.writeUInt32BE(static_cast<std::uint32_t>(offset));
        }
        break;
    case Mp4AtomIds::ChunkOffset64:
        for (auto offset : chunkOffsets) {
            m_writer.writeUInt64BE(offset);
        }
        break;
    default:
        throw InvalidDataException();
    }
}

/*!
 * \brief Updates a particular chunk offset.
 * \param chunkIndex Specifies the index of the chunk offset to be updated.
 * \param offset Specifies the new chunk offset. If the "stco" atom is used the value must fit
 *               into a 32-bit unsigned int.
 * \remarks This method seems to be obsolete.
 * \throws Throws InvalidDataException when
 *          - there is no stream assigned.
 *          - the header has been considered as invalid when parsing the header information.
 *          - \a chunkIndex is not less than chunkCount().
 *          - there is no atom holding these offsets.
 *          - the ID of the atom holding these offsets is not "stco" or "co64".
 */
void Mp4Track::updateChunkOffset(std::uint32_t chunkIndex, std::uint64_t offset)
{
    if (!isHeaderValid() || !m_istream || !m_stcoAtom || chunkIndex >= m_chunkCount) {
        throw InvalidDataException();
    }
    m_ostream->seekp(static_cast<streamoff>(m_stcoAtom->dataOffset() + 8 + chunkOffsetSize() * chunkIndex));
    switch (chunkOffsetSize()) {
    case 4:
        writer().writeUInt32BE(static_cast<std::uint32_t>(offset));
        break;
    case 8:
        writer().writeUInt64BE(offset);
        break;
    default:
        throw InvalidDataException();
    }
}

/*!
 * \brief Adds the information from the specified \a avcConfig to the specified \a track.
 */
void Mp4Track::addInfo(const AvcConfiguration &avcConfig, AbstractTrack &track)
{
    if (!avcConfig.spsInfos.empty()) {
        const SpsInfo &spsInfo = avcConfig.spsInfos.back();
        track.m_format.sub = spsInfo.profileIndication;
        track.m_version = static_cast<double>(spsInfo.levelIndication) / 10;
        track.m_cropping = spsInfo.cropping;
        track.m_pixelSize = spsInfo.pictureSize;
        switch (spsInfo.chromaFormatIndication) {
        case 0:
            track.m_chromaFormat = "monochrome";
            break;
        case 1:
            track.m_chromaFormat = "YUV 4:2:0";
            break;
        case 2:
            track.m_chromaFormat = "YUV 4:2:2";
            break;
        case 3:
            track.m_chromaFormat = "YUV 4:4:4";
            break;
        default:;
        }
        track.m_pixelAspectRatio = spsInfo.pixelAspectRatio;
    } else {
        track.m_format.sub = avcConfig.profileIndication;
        track.m_version = static_cast<double>(avcConfig.levelIndication) / 10;
    }
}

/*!
 * \brief Adds the information from the specified \a av1Config to the specified \a track.
 * \todo Provide implementation
 */
void Mp4Track::addInfo(const Av1Configuration &av1Config, AbstractTrack &track)
{
    CPP_UTILITIES_UNUSED(av1Config)
    CPP_UTILITIES_UNUSED(track)
    throw NotImplementedException();
}

/*!
 * \brief Buffers all atoms required by the makeTrack() method.
 *
 * This allows to invoke makeTrack() also when the input stream is going to be
 * modified (eg. to apply changed tags without rewriting the file).
 */
void Mp4Track::bufferTrackAtoms(Diagnostics &diag)
{
    CPP_UTILITIES_UNUSED(diag)

    if (m_tkhdAtom) {
        m_tkhdAtom->makeBuffer();
    }
    for (Mp4Atom *trakChild = m_trakAtom->firstChild(); trakChild; trakChild = trakChild->nextSibling()) {
        if (trakChild->id() == Mp4AtomIds::Media) {
            continue;
        }
        trakChild->makeBuffer();
    }
    if (m_minfAtom) {
        for (Mp4Atom *childAtom = m_minfAtom->firstChild(); childAtom; childAtom = childAtom->nextSibling()) {
            childAtom->makeBuffer();
        }
    }
}

/*!
 * \brief Returns the number of bytes written when calling makeTrack().
 */
std::uint64_t Mp4Track::requiredSize(Diagnostics &diag) const
{
    CPP_UTILITIES_UNUSED(diag)

    const auto &info = verifyPresentTrackHeader();
    // add size of
    // ... trak header
    std::uint64_t size = 8;
    // ... tkhd atom
    size += info.requiredSize;
    // ... children beside tkhd and mdia
    for (Mp4Atom *trakChild = m_trakAtom->firstChild(); trakChild; trakChild = trakChild->nextSibling()) {
        if (trakChild->id() == Mp4AtomIds::Media || trakChild->id() == Mp4AtomIds::TrackHeader) {
            continue;
        }
        size += trakChild->totalSize();
    }
    // ... mdhd total size
    if (info.timingsVersion == 0) {
        // write version 0 where timing fields are 32-bit
        size += 32;
    } else {
        // write version 1 where timing fields are 64-bit
        size += 44;
    }
    // ... mdia header + hdlr total size + minf header
    size += 8 + (33 + m_name.size()) + 8;
    // ... minf children
    bool dinfAtomWritten = false;
    if (m_minfAtom) {
        for (Mp4Atom *childAtom = m_minfAtom->firstChild(); childAtom; childAtom = childAtom->nextSibling()) {
            if (childAtom->id() == Mp4AtomIds::DataInformation) {
                dinfAtomWritten = true;
            }
            size += childAtom->totalSize();
        }
    }
    if (!dinfAtomWritten) {
        // take 36 bytes for a self-made dinf atom into account if the file lacks one
        size += 36;
    }
    return size;
}

/*!
 * \brief Makes the track entry ("trak"-atom) for the track.
 *
 * The data is written to the assigned output stream at the current position. Note that this method
 * uses the assigned input stream to copy some parts from the source file. Hence the input stream must
 * still be valid when calling this method. To avoid this limitation call bufferTrackAtoms() before
 * invalidating the input stream.
 */
void Mp4Track::makeTrack(Diagnostics &diag)
{
    // write header
    ostream::pos_type trakStartOffset = outputStream().tellp();
    m_writer.writeUInt32BE(0); // write size later
    m_writer.writeUInt32BE(Mp4AtomIds::Track);

    // write tkhd atom
    makeTrackHeader(diag);

    // write children of trak atom except mdia
    for (Mp4Atom *trakChild = trakAtom().firstChild(); trakChild; trakChild = trakChild->nextSibling()) {
        if (trakChild->id() == Mp4AtomIds::Media || trakChild->id() == Mp4AtomIds::TrackHeader) {
            continue;
        }
        trakChild->copyPreferablyFromBuffer(outputStream(), diag, nullptr);
    }

    // write mdia atom
    makeMedia(diag);

    // write size (of trak atom)
    Mp4Atom::seekBackAndWriteAtomSize(outputStream(), trakStartOffset, diag);
}

/*!
 * \brief Makes the track header (tkhd atom) for the track. The data is written to the assigned output stream
 *        at the current position.
 */
void Mp4Track::makeTrackHeader(Diagnostics &diag)
{
    // verify the existing track header to make the new one based on it (if possible)
    const auto &info = verifyPresentTrackHeader();

    // add notifications in case the present track header could not be parsed
    if (info.versionUnknown) {
        diag.emplace_back(DiagLevel::Critical,
            argsToString("The version of the present \"tkhd\"-atom (", info.version, ") is unknown. Assuming version 1."),
            argsToString("making \"tkhd\"-atom of track ", m_id));
    }
    if (info.truncated) {
        diag.emplace_back(
            DiagLevel::Critical, argsToString("The present \"tkhd\"-atom is truncated."), argsToString("making \"tkhd\"-atom of track ", m_id));
    }

    // make size and element ID
    if (info.requiredSize > numeric_limits<std::uint32_t>::max()) {
        writer().writeUInt32BE(1);
        writer().writeUInt32BE(Mp4AtomIds::TrackHeader);
        writer().writeUInt64BE(info.requiredSize);
    } else {
        writer().writeUInt32BE(static_cast<std::uint32_t>(info.requiredSize));
        writer().writeUInt32BE(Mp4AtomIds::TrackHeader);
    }

    // make version and flags
    writer().writeByte(info.writeVersion);
    std::uint32_t flags = 0;
    if (isEnabled()) {
        flags |= 0x000001;
    }
    if (m_flags & TrackFlags::UsedInPresentation) {
        flags |= 0x000002;
    }
    if (m_flags & TrackFlags::UsedWhenPreviewing) {
        flags |= 0x000004;
    }
    writer().writeUInt24BE(flags);

    // make creation and modification time
    if (info.writeVersion != 0) {
        writer().writeUInt64BE(info.timings.tkhdCreationTime);
        writer().writeUInt64BE(info.timings.tkhdModificationTime);
    } else {
        writer().writeUInt32BE(static_cast<std::uint32_t>(info.timings.tkhdCreationTime));
        writer().writeUInt32BE(static_cast<std::uint32_t>(info.timings.tkhdModificationTime));
    }

    // make track ID and duration
    writer().writeUInt32BE(static_cast<std::uint32_t>(m_id));
    writer().writeUInt32BE(0); // reserved
    if (info.writeVersion != 0) {
        writer().writeUInt64BE(info.timings.tkhdDuration);
    } else {
        writer().writeUInt32BE(static_cast<std::uint32_t>(info.timings.tkhdDuration));
    }
    writer().writeUInt32BE(0); // reserved
    writer().writeUInt32BE(0); // reserved

    // make further values, either from existing tkhd atom or just some defaults
    if (info.canUseExisting) {
        // write all bytes after the previously determined additionalDataOffset
        m_ostream->write(m_tkhdAtom->buffer().get() + m_tkhdAtom->headerSize() + info.additionalDataOffset,
            static_cast<streamoff>(m_tkhdAtom->dataSize() - info.additionalDataOffset));
        // discard the buffer again if it wasn't present before
        if (info.discardBuffer) {
            m_tkhdAtom->discardBuffer();
        }
    } else {
        // write default values
        diag.emplace_back(DiagLevel::Warning, "Writing some default values because the existing tkhd atom is truncated.", "making tkhd atom");
        writer().writeInt16BE(0); // layer
        writer().writeInt16BE(0); // alternate group
        writer().writeFixed8BE(1.0); // volume (fixed 8.8 - 2 byte)
        writer().writeUInt16BE(0); // reserved
        for (const std::int32_t value : { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 }) { // unity matrix
            writer().writeInt32BE(value);
        }
        writer().writeFixed16BE(1.0); // width
        writer().writeFixed16BE(1.0); // height
    }
}

/*!
 * \brief Makes the media information (mdia atom) for the track. The data is written to the assigned output stream
 *        at the current position.
 */
void Mp4Track::makeMedia(Diagnostics &diag)
{
    ostream::pos_type mdiaStartOffset = outputStream().tellp();
    writer().writeUInt32BE(0); // write size later
    writer().writeUInt32BE(Mp4AtomIds::Media);
    // write mdhd atom
    const auto &info = verifyPresentTrackHeader();
    const auto &timings = info.timings;
    const auto timingsVersion = timings.requiredMdhdVersion();
    writer().writeUInt32BE(timingsVersion != 0 ? 44 : 32); // size
    writer().writeUInt32BE(Mp4AtomIds::MediaHeader);
    writer().writeByte(timingsVersion); // version
    writer().writeUInt24BE(0); // flags
    if (timingsVersion != 0) {
        writer().writeUInt64BE(timings.mdhdCreationTime);
        writer().writeUInt64BE(timings.mdhdModificationTime);
    } else {
        writer().writeUInt32BE(static_cast<std::uint32_t>(timings.mdhdCreationTime));
        writer().writeUInt32BE(static_cast<std::uint32_t>(timings.mdhdModificationTime));
    }
    writer().writeUInt32BE(m_timeScale);
    if (timingsVersion != 0) {
        writer().writeUInt64BE(timings.mdhdDuration);
    } else {
        writer().writeUInt32BE(static_cast<std::uint32_t>(timings.mdhdDuration));
    }
    // convert and write language
    // note: Not using m_locale.abbreviatedName() here to preserve "und" (explicitly undefined).
    const auto *language = static_cast<const std::string *>(&LocaleDetail::getEmpty());
    for (const auto &detail : m_locale) {
        if (!detail.empty() && (detail.format == LocaleFormat::ISO_639_2_T || detail.format == LocaleFormat::Unknown)) {
            language = &detail;
            break;
        }
    }
    auto codedLanguage = static_cast<std::uint16_t>(0u);
    for (auto charIndex = static_cast<std::size_t>(0); charIndex != 3; ++charIndex) {
        const char langChar = charIndex < language->size() ? (*language)[charIndex] : 0;
        if (langChar >= 'a' && langChar <= 'z') {
            codedLanguage |= static_cast<std::uint16_t>((langChar - 0x60) << (0xA - charIndex * 0x5));
            continue;
        }

        // handle invalid characters
        if (language->empty()) {
            // preserve null value (empty language field) which is not the same as "und" (explicitly undefined)
            codedLanguage = 0;
            break;
        }
        diag.emplace_back(DiagLevel::Warning, "Assigned language \"" % *language + "\" is of an invalid format. Setting language to undefined.",
            "making mdhd atom");
        codedLanguage = 0x55C4; // und(efined)
        break;
    }
    if (language->size() > 3) {
        diag.emplace_back(
            DiagLevel::Warning, "Assigned language \"" % *language + "\" is longer than 3 byte and hence will be truncated.", "making mdhd atom");
    }
    writer().writeUInt16BE(codedLanguage);
    writer().writeUInt16BE(0); // pre defined
    // write hdlr atom
    writer().writeUInt32BE(33 + static_cast<std::uint32_t>(m_name.size())); // size
    writer().writeUInt32BE(Mp4AtomIds::HandlerReference);
    writer().writeUInt64BE(0); // version, flags, pre defined
    switch (m_mediaType) {
    case MediaType::Video:
        outputStream().write("vide", 4);
        break;
    case MediaType::Audio:
        outputStream().write("soun", 4);
        break;
    case MediaType::Hint:
        outputStream().write("hint", 4);
        break;
    case MediaType::Text:
        outputStream().write("text", 4);
        break;
    case MediaType::Meta:
        outputStream().write("meta", 4);
        break;
    default:
        if (m_mediaType != MediaType::Unknown) {
            diag.emplace_back(DiagLevel::Critical, "Media type is invalid; keeping media type as-is.", "making hdlr atom");
        }
        writer().writeUInt32BE(m_rawMediaType);
        break;
    }
    for (int i = 0; i < 3; ++i)
        writer().writeUInt32BE(0); // reserved
    writer().writeTerminatedString(m_name);
    // write minf atom
    makeMediaInfo(diag);
    // write size (of mdia atom)
    Mp4Atom::seekBackAndWriteAtomSize(outputStream(), mdiaStartOffset, diag);
}

/*!
 * \brief Makes a media information (minf atom) for the track. The data is written to the assigned output stream
 *        at the current position.
 */
void Mp4Track::makeMediaInfo(Diagnostics &diag)
{
    ostream::pos_type minfStartOffset = outputStream().tellp();
    writer().writeUInt32BE(0); // write size later
    writer().writeUInt32BE(Mp4AtomIds::MediaInformation);
    bool dinfAtomWritten = false;
    if (m_minfAtom) {
        // copy existing atoms except sample table which is handled separately
        for (Mp4Atom *childAtom = m_minfAtom->firstChild(); childAtom; childAtom = childAtom->nextSibling()) {
            if (childAtom->id() == Mp4AtomIds::SampleTable) {
                continue;
            }
            if (childAtom->id() == Mp4AtomIds::DataInformation) {
                dinfAtomWritten = true;
            }
            childAtom->copyPreferablyFromBuffer(outputStream(), diag, nullptr);
        }
    }
    // write dinf atom if not written yet
    if (!dinfAtomWritten) {
        writer().writeUInt32BE(36); // size
        writer().writeUInt32BE(Mp4AtomIds::DataInformation);
        // write dref atom
        writer().writeUInt32BE(28); // size
        writer().writeUInt32BE(Mp4AtomIds::DataReference);
        writer().writeUInt32BE(0); // version and flags
        writer().writeUInt32BE(1); // entry count
        // write url  atom
        writer().writeUInt32BE(12); // size
        writer().writeUInt32BE(Mp4AtomIds::DataEntryUrl);
        writer().writeByte(0); // version
        writer().writeUInt24BE(0x000001); // flags (media data is in the same file as the movie box)
    }
    // write stbl atom
    // -> just copy existing stbl atom because makeSampleTable() is not fully implemented (yet)
    bool stblAtomWritten = false;
    if (m_minfAtom) {
        if (Mp4Atom *const stblAtom = m_minfAtom->childById(Mp4AtomIds::SampleTable, diag)) {
            stblAtom->copyPreferablyFromBuffer(outputStream(), diag, nullptr);
            stblAtomWritten = true;
        }
    }
    if (!stblAtomWritten) {
        diag.emplace_back(DiagLevel::Critical,
            "Source track does not contain mandatory stbl atom and the tagparser lib is unable to make one from scratch.", "making stbl atom");
    }
    // write size (of minf atom)
    Mp4Atom::seekBackAndWriteAtomSize(outputStream(), minfStartOffset, diag);
}

/*!
 * \brief Makes the sample table (stbl atom) for the track. The data is written to the assigned output stream
 *        at the current position.
 * \remarks Not fully implemented yet.
 */
void Mp4Track::makeSampleTable(Diagnostics &diag)
{
    // ostream::pos_type stblStartOffset = outputStream().tellp(); (enable when function is fully implemented)

    writer().writeUInt32BE(0); // write size later
    writer().writeUInt32BE(Mp4AtomIds::SampleTable);
    Mp4Atom *const stblAtom = m_minfAtom ? m_minfAtom->childById(Mp4AtomIds::SampleTable, diag) : nullptr;
    // write stsd atom
    if (m_stsdAtom) {
        // copy existing stsd atom
        m_stsdAtom->copyEntirely(outputStream(), diag, nullptr);
    } else {
        diag.emplace_back(DiagLevel::Critical, "Unable to make stsd atom from scratch.", "making stsd atom");
        throw NotImplementedException();
    }
    // write stts and ctts atoms
    Mp4Atom *const sttsAtom = stblAtom ? stblAtom->childById(Mp4AtomIds::DecodingTimeToSample, diag) : nullptr;
    if (sttsAtom) {
        // copy existing stts atom
        sttsAtom->copyEntirely(outputStream(), diag, nullptr);
    } else {
        diag.emplace_back(DiagLevel::Critical, "Unable to make stts atom from scratch.", "making stts atom");
        throw NotImplementedException();
    }
    Mp4Atom *const cttsAtom = stblAtom ? stblAtom->childById(Mp4AtomIds::CompositionTimeToSample, diag) : nullptr;
    if (cttsAtom) {
        // copy existing ctts atom
        cttsAtom->copyEntirely(outputStream(), diag, nullptr);
    }
    // write stsc atom (sample-to-chunk table)
    throw NotImplementedException();

    // write stsz atom (sample sizes)

    // write stz2 atom (compact sample sizes)

    // write stco/co64 atom (chunk offset table)

    // write stss atom (sync sample table)

    // write stsh atom (shadow sync sample table)

    // write padb atom (sample padding bits)

    // write stdp atom (sample degradation priority)

    // write sdtp atom (independent and disposable samples)

    // write sbgp atom (sample group description)

    // write sbgp atom (sample-to-group)

    // write sgpd atom (sample group description)

    // write subs atom (sub-sample information)

    // write size of stbl atom (enable when function is fully implemented)
    // Mp4Atom::seekBackAndWriteAtomSize(outputStream(), stblStartOffset, diag);
}

void Mp4Track::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing MP4 track");
    using namespace Mp4AtomIds;
    if (!m_trakAtom) {
        diag.emplace_back(DiagLevel::Critical, "\"trak\"-atom is null.", context);
        throw InvalidDataException();
    }

    // get atoms
    try {
        if (!(m_tkhdAtom = m_trakAtom->childById(TrackHeader, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"tkhd\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_mdiaAtom = m_trakAtom->childById(Media, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"mdia\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_mdhdAtom = m_mdiaAtom->childById(MediaHeader, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"mdhd\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_hdlrAtom = m_mdiaAtom->childById(HandlerReference, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"hdlr\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_minfAtom = m_mdiaAtom->childById(MediaInformation, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"minf\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_stblAtom = m_minfAtom->childById(SampleTable, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"stbl\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_stsdAtom = m_stblAtom->childById(SampleDescription, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"stsd\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_stcoAtom = m_stblAtom->childById(ChunkOffset, diag)) && !(m_stcoAtom = m_stblAtom->childById(ChunkOffset64, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"stco\"/\"co64\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_stscAtom = m_stblAtom->childById(SampleToChunk, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"stsc\"-atom found.", context);
            throw InvalidDataException();
        }
        if (!(m_stszAtom = m_stblAtom->childById(SampleSize, diag)) && !(m_stszAtom = m_stblAtom->childById(CompactSampleSize, diag))) {
            diag.emplace_back(DiagLevel::Critical, "No \"stsz\"/\"stz2\"-atom found.", context);
            throw InvalidDataException();
        }
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Critical, "Unable to parse relevant atoms.", context);
        throw InvalidDataException();
    }

    BinaryReader &reader = m_trakAtom->reader();

    // read tkhd atom
    m_istream->seekg(static_cast<streamoff>(m_tkhdAtom->startOffset() + 8)); // seek to beg, skip size and name
    auto atomVersion = reader.readByte(); // read version
    const auto flags = reader.readUInt24BE();
    modFlagEnum(m_flags, TrackFlags::Enabled, flags & 0x000001);
    modFlagEnum(m_flags, TrackFlags::UsedInPresentation, flags & 0x000002);
    modFlagEnum(m_flags, TrackFlags::UsedWhenPreviewing, flags & 0x000004);
    switch (atomVersion) {
    case 0:
        m_rawTkhdCreationTime = reader.readUInt32BE();
        m_rawTkhdModificationTime = reader.readUInt32BE();
        m_id = reader.readUInt32BE();
        m_istream->seekg(4, std::ios_base::cur);
        m_rawTkhdDuration = reader.readUInt32BE();
        break;
    case 1:
        m_rawTkhdCreationTime = reader.readUInt64BE();
        m_rawTkhdModificationTime = reader.readUInt64BE();
        m_id = reader.readUInt32BE();
        m_istream->seekg(4, std::ios_base::cur);
        m_rawTkhdDuration = reader.readUInt64BE();
        break;
    default:
        diag.emplace_back(DiagLevel::Critical,
            "Version of \"tkhd\"-atom not supported. It will be ignored. Track ID, creation time and modification time might not be be determined.",
            context);
        m_rawTkhdCreationTime = m_rawTkhdModificationTime = m_rawTkhdDuration = 0;
        m_creationTime = DateTime();
        m_modificationTime = DateTime();
        m_id = 0;
    }

    // read mdhd atom
    m_istream->seekg(static_cast<streamoff>(m_mdhdAtom->dataOffset())); // seek to beg, skip size and name
    atomVersion = reader.readByte(); // read version
    m_istream->seekg(3, ios_base::cur); // skip flags
    switch (atomVersion) {
    case 0:
        m_rawMdhdCreationTime = reader.readUInt32BE();
        m_rawMdhdModificationTime = reader.readUInt32BE();
        m_timeScale = reader.readUInt32BE();
        m_rawMdhdDuration = reader.readUInt32BE();
        break;
    case 1:
        m_rawMdhdCreationTime = reader.readUInt64BE();
        m_rawMdhdModificationTime = reader.readUInt64BE();
        m_timeScale = reader.readUInt32BE();
        m_rawMdhdDuration = reader.readUInt64BE();
        break;
    default:
        diag.emplace_back(DiagLevel::Warning,
            "Version of \"mdhd\"-atom not supported. It will be ignored. Creation time, modification time, time scale and duration might not be "
            "determined.",
            context);
        m_rawMdhdCreationTime = m_rawMdhdModificationTime = m_rawMdhdDuration = 0;
        m_timeScale = 0;
        m_duration = TimeSpan();
    }
    m_creationTime = Mp4Container::epoch + TimeSpan::fromSeconds(static_cast<TimeSpan::TickType>(m_rawMdhdCreationTime));
    m_modificationTime = Mp4Container::epoch + TimeSpan::fromSeconds(static_cast<TimeSpan::TickType>(m_rawMdhdModificationTime));
    m_duration = TimeSpan::fromSeconds(static_cast<TimeSpan::TickType>(m_rawMdhdDuration)) / static_cast<TimeSpan::TickType>(m_timeScale);

    std::uint16_t tmp = reader.readUInt16BE();
    if (tmp) {
        const char buff[] = {
            static_cast<char>(((tmp & 0x7C00) >> 0xA) + 0x60),
            static_cast<char>(((tmp & 0x03E0) >> 0x5) + 0x60),
            static_cast<char>(((tmp & 0x001F) >> 0x0) + 0x60),
        };
        m_locale.emplace_back(std::string(buff, 3), LocaleFormat::ISO_639_2_T);
    } else {
        m_locale.clear();
    }

    // read hdlr atom
    // -> seek to begin skipping size, name, version, flags and reserved bytes
    m_istream->seekg(static_cast<streamoff>(m_hdlrAtom->dataOffset() + 8));
    // -> track type
    switch (m_rawMediaType = reader.readUInt32BE()) {
    case 0x76696465:
        m_mediaType = MediaType::Video;
        break;
    case 0x736F756E:
        m_mediaType = MediaType::Audio;
        break;
    case 0x68696E74:
        m_mediaType = MediaType::Hint;
        break;
    case 0x6D657461:
        m_mediaType = MediaType::Meta;
        break;
    case 0x74657874:
        m_mediaType = MediaType::Text;
        break;
    default:
        m_mediaType = MediaType::Unknown;
    }
    // -> name
    m_istream->seekg(12, ios_base::cur); // skip reserved bytes
    if (static_cast<std::uint64_t>(tmp = static_cast<std::uint8_t>(m_istream->peek())) == m_hdlrAtom->dataSize() - 12 - 4 - 8 - 1) {
        // assume size prefixed string (seems to appear in QuickTime files)
        m_istream->seekg(1, ios_base::cur);
        m_name = reader.readString(tmp);
    } else {
        // assume null terminated string (appears in MP4 files)
        m_name = reader.readTerminatedString(m_hdlrAtom->dataSize() - 12 - 4 - 8, 0);
    }

    // read stco atom (only chunk count)
    m_chunkOffsetSize = (m_stcoAtom->id() == Mp4AtomIds::ChunkOffset64) ? 8 : 4;
    m_istream->seekg(static_cast<streamoff>(m_stcoAtom->dataOffset() + 4));
    m_chunkCount = reader.readUInt32BE();

    // read stsd atom
    m_istream->seekg(static_cast<streamoff>(m_stsdAtom->dataOffset() + 4)); // seek to beg, skip size, name, version and flags
    const auto entryCount = reader.readUInt32BE();
    Mp4Atom *esDescParentAtom = nullptr;
    if (entryCount) {
        try {
            for (Mp4Atom *codecConfigContainerAtom = m_stsdAtom->firstChild(); codecConfigContainerAtom;
                 codecConfigContainerAtom = codecConfigContainerAtom->nextSibling()) {
                codecConfigContainerAtom->parse(diag);

                // parse FOURCC
                m_formatId = interpretIntegerAsString<std::uint32_t>(codecConfigContainerAtom->id());
                m_format = FourccIds::fourccToMediaFormat(codecConfigContainerAtom->id());

                // parse codecConfigContainerAtom
                m_istream->seekg(static_cast<streamoff>(codecConfigContainerAtom->dataOffset()));
                switch (codecConfigContainerAtom->id()) {
                case FourccIds::Mpeg4Audio:
                case FourccIds::AmrNarrowband:
                case FourccIds::Amr:
                case FourccIds::Drms:
                case FourccIds::Alac:
                case FourccIds::WindowsMediaAudio:
                case FourccIds::Ac3:
                case FourccIds::EAc3:
                case FourccIds::DolbyMpl:
                case FourccIds::Dts:
                case FourccIds::DtsH:
                case FourccIds::DtsE:
                case FourccIds::Flac:
                case FourccIds::Opus:
                    m_istream->seekg(6 + 2, ios_base::cur); // skip reserved bytes, data reference index
                    tmp = reader.readUInt16BE(); // read sound version
                    m_istream->seekg(6, ios_base::cur);
                    m_channelCount = reader.readUInt16BE();
                    m_bitsPerSample = reader.readUInt16BE();
                    m_istream->seekg(4, ios_base::cur); // skip reserved bytes (again)
                    if (!m_samplingFrequency) {
                        m_samplingFrequency = reader.readUInt32BE() >> 16;
                        if (codecConfigContainerAtom->id() != FourccIds::DolbyMpl) {
                            m_samplingFrequency >>= 16;
                        }
                    } else {
                        m_istream->seekg(4, ios_base::cur);
                    }
                    if (codecConfigContainerAtom->id() != FourccIds::WindowsMediaAudio) {
                        switch (tmp) {
                        case 1:
                            codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 28 + 16);
                            break;
                        case 2:
                            codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 28 + 32);
                            break;
                        default:
                            codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 28);
                        }
                        if (!esDescParentAtom) {
                            esDescParentAtom = codecConfigContainerAtom;
                        }
                    }
                    break;
                case FourccIds::Mpeg4Video:
                case FourccIds::H263Quicktime:
                case FourccIds::H2633GPP:
                case FourccIds::Avc1:
                case FourccIds::Avc2:
                case FourccIds::Avc3:
                case FourccIds::Avc4:
                case FourccIds::Drmi:
                case FourccIds::Hevc1:
                case FourccIds::Hevc2:
                case FourccIds::Av1_IVF:
                case FourccIds::Av1_ISOBMFF:
                case FourccIds::Vp9_2:
                    m_istream->seekg(6 + 2 + 16, ios_base::cur); // skip reserved bytes, data reference index, and reserved bytes (again)
                    m_pixelSize.setWidth(reader.readUInt16BE());
                    m_pixelSize.setHeight(reader.readUInt16BE());
                    m_resolution.setWidth(static_cast<std::uint32_t>(reader.readFixed16BE()));
                    m_resolution.setHeight(static_cast<std::uint32_t>(reader.readFixed16BE()));
                    m_istream->seekg(4, ios_base::cur); // skip reserved bytes
                    m_framesPerSample = reader.readUInt16BE();
                    tmp = reader.readByte();
                    m_compressorName = reader.readString(31);
                    if (tmp == 0) {
                        m_compressorName.clear();
                    } else if (tmp < 32) {
                        m_compressorName.resize(tmp);
                    }
                    m_depth = reader.readUInt16BE(); // 24: color without alpha
                    codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 78);
                    if (!esDescParentAtom) {
                        esDescParentAtom = codecConfigContainerAtom;
                    }
                    break;
                case FourccIds::Mpeg4Sample:
                    // skip reserved bytes and data reference index
                    codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 8);
                    if (!esDescParentAtom) {
                        esDescParentAtom = codecConfigContainerAtom;
                    }
                    break;
                case Mp4AtomIds::PixalAspectRatio:
                    break; // TODO
                case Mp4AtomIds::CleanAperature:
                    break; // TODO
                default:;
                }
            }

            if (esDescParentAtom) {
                // parse AVC configuration
                if (auto *const avcConfigAtom = esDescParentAtom->childById(Mp4AtomIds::AvcConfiguration, diag)) {
                    m_istream->seekg(static_cast<streamoff>(avcConfigAtom->dataOffset()));
                    m_avcConfig = make_unique<TagParser::AvcConfiguration>();
                    try {
                        m_avcConfig->parse(reader, avcConfigAtom->dataSize(), diag);
                        addInfo(*m_avcConfig, *this);
                    } catch (const TruncatedDataException &) {
                        diag.emplace_back(DiagLevel::Critical, "AVC configuration is truncated.", context);
                    } catch (const Failure &) {
                        diag.emplace_back(DiagLevel::Critical, "AVC configuration is invalid.", context);
                    }
                }

                // parse AV1 configuration
                if (auto *const av1ConfigAtom = esDescParentAtom->childById(Mp4AtomIds::Av1Configuration, diag)) {
                    m_istream->seekg(static_cast<streamoff>(av1ConfigAtom->dataOffset()));
                    m_av1Config = make_unique<TagParser::Av1Configuration>();
                    try {
                        m_av1Config->parse(reader, av1ConfigAtom->dataSize(), diag);
                        addInfo(*m_av1Config, *this);
                    } catch (const NotImplementedException &) {
                        diag.emplace_back(DiagLevel::Information, "Parsing AV1 configuration is not supported yet.", context);
                    } catch (const TruncatedDataException &) {
                        diag.emplace_back(DiagLevel::Critical, "AV1 configuration is truncated.", context);
                    } catch (const Failure &) {
                        diag.emplace_back(DiagLevel::Critical, "AV1 configuration is invalid.", context);
                    }
                }

                // parse MPEG-4 elementary stream descriptor
                auto *esDescAtom = esDescParentAtom->childById(Mp4FormatExtensionIds::Mpeg4ElementaryStreamDescriptor, diag);
                if (!esDescAtom) {
                    esDescAtom = esDescParentAtom->childById(Mp4FormatExtensionIds::Mpeg4ElementaryStreamDescriptor2, diag);
                }
                if (esDescAtom) {
                    try {
                        if ((m_esInfo = parseMpeg4ElementaryStreamInfo(m_reader, esDescAtom, diag))) {
                            m_format += Mpeg4ElementaryStreamObjectIds::streamObjectTypeFormat(m_esInfo->objectTypeId);
                            m_bitrate = static_cast<double>(m_esInfo->averageBitrate) / 1000;
                            m_maxBitrate = static_cast<double>(m_esInfo->maxBitrate) / 1000;
                            if (m_esInfo->audioSpecificConfig) {
                                // check the audio specific config for useful information
                                m_format += Mpeg4AudioObjectIds::idToMediaFormat(m_esInfo->audioSpecificConfig->audioObjectType,
                                    m_esInfo->audioSpecificConfig->sbrPresent, m_esInfo->audioSpecificConfig->psPresent);
                                if (m_esInfo->audioSpecificConfig->sampleFrequencyIndex == 0xF) {
                                    m_samplingFrequency = m_esInfo->audioSpecificConfig->sampleFrequency;
                                } else if (m_esInfo->audioSpecificConfig->sampleFrequencyIndex < sizeof(mpeg4SamplingFrequencyTable)) {
                                    m_samplingFrequency = mpeg4SamplingFrequencyTable[m_esInfo->audioSpecificConfig->sampleFrequencyIndex];
                                } else {
                                    diag.emplace_back(DiagLevel::Warning, "Audio specific config has invalid sample frequency index.", context);
                                }
                                if (m_esInfo->audioSpecificConfig->extensionSampleFrequencyIndex == 0xF) {
                                    m_extensionSamplingFrequency = m_esInfo->audioSpecificConfig->extensionSampleFrequency;
                                } else if (m_esInfo->audioSpecificConfig->extensionSampleFrequencyIndex < sizeof(mpeg4SamplingFrequencyTable)) {
                                    m_extensionSamplingFrequency
                                        = mpeg4SamplingFrequencyTable[m_esInfo->audioSpecificConfig->extensionSampleFrequencyIndex];
                                } else {
                                    diag.emplace_back(
                                        DiagLevel::Warning, "Audio specific config has invalid extension sample frequency index.", context);
                                }
                                m_channelConfig = m_esInfo->audioSpecificConfig->channelConfiguration;
                                m_extensionChannelConfig = m_esInfo->audioSpecificConfig->extensionChannelConfiguration;
                            }
                            if (m_esInfo->videoSpecificConfig) {
                                // check the video specific config for useful information
                                if (m_format.general == GeneralMediaFormat::Mpeg4Video && m_esInfo->videoSpecificConfig->profile) {
                                    m_format.sub = m_esInfo->videoSpecificConfig->profile;
                                    if (!m_esInfo->videoSpecificConfig->userData.empty()) {
                                        m_formatId += " / ";
                                        m_formatId += m_esInfo->videoSpecificConfig->userData;
                                    }
                                }
                            }
                            // check the stream data for missing information
                            switch (m_format.general) {
                            case GeneralMediaFormat::Mpeg1Audio:
                            case GeneralMediaFormat::Mpeg2Audio: {
                                MpegAudioFrame frame;
                                m_istream->seekg(static_cast<streamoff>(m_stcoAtom->dataOffset() + 8));
                                m_istream->seekg(static_cast<streamoff>(m_chunkOffsetSize == 8 ? reader.readUInt64BE() : reader.readUInt32BE()));
                                frame.parseHeader(reader, diag);
                                MpegAudioFrameStream::addInfo(frame, *this);
                                break;
                            }
                            default:;
                            }
                        }
                    } catch (const Failure &) {
                    }
                }
            }
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Unable to parse child atoms of \"stsd\"-atom.", context);
        }
    }

    // read stsz atom which holds the sample size table
    m_sampleSizes.clear();
    m_size = m_sampleCount = 0;
    std::uint64_t actualSampleSizeTableSize = m_stszAtom->dataSize();
    if (actualSampleSizeTableSize < 12) {
        diag.emplace_back(DiagLevel::Critical,
            "The stsz atom is truncated. There are no sample sizes present. The size of the track can not be determined.", context);
    } else {
        actualSampleSizeTableSize -= 12; // subtract size of version and flags
        m_istream->seekg(static_cast<streamoff>(m_stszAtom->dataOffset() + 4)); // seek to beg, skip size, name, version and flags
        std::uint32_t fieldSize;
        std::uint32_t constantSize;
        if (m_stszAtom->id() == Mp4AtomIds::CompactSampleSize) {
            constantSize = 0;
            m_istream->seekg(3, ios_base::cur); // seek reserved bytes
            fieldSize = reader.readByte();
            m_sampleCount = reader.readUInt32BE();
        } else {
            constantSize = reader.readUInt32BE();
            m_sampleCount = reader.readUInt32BE();
            fieldSize = 32;
        }
        if (constantSize) {
            m_sampleSizes.push_back(constantSize);
            m_size = constantSize * m_sampleCount;
        } else {
            auto actualSampleCount = m_sampleCount;
            const auto calculatedSampleSizeTableSize
                = static_cast<std::uint64_t>(std::ceil((0.125 * fieldSize) * static_cast<double>(m_sampleCount)));
            if (calculatedSampleSizeTableSize < actualSampleSizeTableSize) {
                diag.emplace_back(
                    DiagLevel::Critical, "The stsz atom stores more entries as denoted. The additional entries will be ignored.", context);
            } else if (calculatedSampleSizeTableSize > actualSampleSizeTableSize) {
                diag.emplace_back(DiagLevel::Critical, "The stsz atom is truncated. It stores less entries as denoted.", context);
                actualSampleCount = static_cast<std::uint64_t>(floor(static_cast<double>(actualSampleSizeTableSize) / (0.125 * fieldSize)));
            }
            m_sampleSizes.reserve(actualSampleCount);
            std::uint32_t i = 1;
            switch (fieldSize) {
            case 4:
                for (; i <= actualSampleCount; i += 2) {
                    std::uint8_t val = reader.readByte();
                    m_sampleSizes.push_back(val >> 4);
                    m_sampleSizes.push_back(val & 0xF0);
                    m_size += (val >> 4) + (val & 0xF0);
                }
                if (i <= actualSampleCount + 1) {
                    m_sampleSizes.push_back(reader.readByte() >> 4);
                    m_size += m_sampleSizes.back();
                }
                break;
            case 8:
                for (; i <= actualSampleCount; ++i) {
                    m_sampleSizes.push_back(reader.readByte());
                    m_size += m_sampleSizes.back();
                }
                break;
            case 16:
                for (; i <= actualSampleCount; ++i) {
                    m_sampleSizes.push_back(reader.readUInt16BE());
                    m_size += m_sampleSizes.back();
                }
                break;
            case 32:
                for (; i <= actualSampleCount; ++i) {
                    m_sampleSizes.push_back(reader.readUInt32BE());
                    m_size += m_sampleSizes.back();
                }
                break;
            default:
                diag.emplace_back(DiagLevel::Critical,
                    "The fieldsize used to store the sample sizes is not supported. The sample count and size of the track can not be determined.",
                    context);
            }
        }
    }

    // no sample sizes found, search for trun atoms
    std::uint64_t totalDuration = 0;
    for (Mp4Atom *moofAtom = m_trakAtom->container().firstElement()->siblingByIdIncludingThis(MovieFragment, diag); moofAtom;
         moofAtom = moofAtom->siblingById(MovieFragment, diag)) {
        moofAtom->parse(diag);
        for (Mp4Atom *trafAtom = moofAtom->childById(TrackFragment, diag); trafAtom; trafAtom = trafAtom->siblingById(TrackFragment, diag)) {
            trafAtom->parse(diag);
            for (Mp4Atom *tfhdAtom = trafAtom->childById(TrackFragmentHeader, diag); tfhdAtom;
                 tfhdAtom = tfhdAtom->siblingById(TrackFragmentHeader, diag)) {
                tfhdAtom->parse(diag);
                std::uint32_t calculatedDataSize = 0;
                if (tfhdAtom->dataSize() < calculatedDataSize) {
                    diag.emplace_back(DiagLevel::Critical, "tfhd atom is truncated.", context);
                } else {
                    m_istream->seekg(static_cast<streamoff>(tfhdAtom->dataOffset() + 1));
                    std::uint32_t tfhdFlags = reader.readUInt24BE();
                    if (m_id == reader.readUInt32BE()) { // check track ID
                        if (tfhdFlags & 0x000001) { // base-data-offset present
                            calculatedDataSize += 8;
                        }
                        if (tfhdFlags & 0x000002) { // sample-description-index present
                            calculatedDataSize += 4;
                        }
                        if (tfhdFlags & 0x000008) { // default-sample-duration present
                            calculatedDataSize += 4;
                        }
                        if (tfhdFlags & 0x000010) { // default-sample-size present
                            calculatedDataSize += 4;
                        }
                        if (tfhdFlags & 0x000020) { // default-sample-flags present
                            calculatedDataSize += 4;
                        }
                        //uint64 baseDataOffset = moofAtom->startOffset();
                        //uint32 defaultSampleDescriptionIndex = 0;
                        std::uint32_t defaultSampleDuration = 0;
                        std::uint32_t defaultSampleSize = 0;
                        //uint32 defaultSampleFlags = 0;
                        if (tfhdAtom->dataSize() < calculatedDataSize) {
                            diag.emplace_back(DiagLevel::Critical, "tfhd atom is truncated (presence of fields denoted).", context);
                        } else {
                            if (tfhdFlags & 0x000001) { // base-data-offset present
                                //baseDataOffset = reader.readUInt64();
                                m_istream->seekg(8, ios_base::cur);
                            }
                            if (tfhdFlags & 0x000002) { // sample-description-index present
                                //defaultSampleDescriptionIndex = reader.readUInt32();
                                m_istream->seekg(4, ios_base::cur);
                            }
                            if (tfhdFlags & 0x000008) { // default-sample-duration present
                                defaultSampleDuration = reader.readUInt32BE();
                                //m_istream->seekg(4, ios_base::cur);
                            }
                            if (tfhdFlags & 0x000010) { // default-sample-size present
                                defaultSampleSize = reader.readUInt32BE();
                            }
                            if (tfhdFlags & 0x000020) { // default-sample-flags present
                                //defaultSampleFlags = reader.readUInt32BE();
                                m_istream->seekg(4, ios_base::cur);
                            }
                        }
                        for (Mp4Atom *trunAtom = trafAtom->childById(TrackFragmentRun, diag); trunAtom;
                             trunAtom = trunAtom->siblingById(TrackFragmentRun, diag)) {
                            std::uint32_t trunCalculatedDataSize = 8;
                            if (trunAtom->dataSize() < trunCalculatedDataSize) {
                                diag.emplace_back(DiagLevel::Critical, "trun atom is truncated.", context);
                            } else {
                                m_istream->seekg(static_cast<streamoff>(trunAtom->dataOffset() + 1));
                                std::uint32_t trunFlags = reader.readUInt24BE();
                                std::uint32_t sampleCount = reader.readUInt32BE();
                                m_sampleCount += sampleCount;
                                if (trunFlags & 0x000001) { // data offset present
                                    trunCalculatedDataSize += 4;
                                }
                                if (trunFlags & 0x000004) { // first-sample-flags present
                                    trunCalculatedDataSize += 4;
                                }
                                std::uint32_t entrySize = 0;
                                if (trunFlags & 0x000100) { // sample-duration present
                                    entrySize += 4;
                                }
                                if (trunFlags & 0x000200) { // sample-size present
                                    entrySize += 4;
                                }
                                if (trunFlags & 0x000400) { // sample-flags present
                                    entrySize += 4;
                                }
                                if (trunFlags & 0x000800) { // sample-composition-time-offsets present
                                    entrySize += 4;
                                }
                                trunCalculatedDataSize += entrySize * sampleCount;
                                if (trunAtom->dataSize() < trunCalculatedDataSize) {
                                    diag.emplace_back(DiagLevel::Critical, "trun atom is truncated (presence of fields denoted).", context);
                                } else {
                                    if (trunFlags & 0x000001) { // data offset present
                                        m_istream->seekg(4, ios_base::cur);
                                        //int32 dataOffset = reader.readInt32();
                                    }
                                    if (trunFlags & 0x000004) { // first-sample-flags present
                                        m_istream->seekg(4, ios_base::cur);
                                    }
                                    for (std::uint32_t i = 0; i < sampleCount; ++i) {
                                        if (trunFlags & 0x000100) { // sample-duration present
                                            totalDuration += reader.readUInt32BE();
                                        } else {
                                            totalDuration += defaultSampleDuration;
                                        }
                                        if (trunFlags & 0x000200) { // sample-size present
                                            m_sampleSizes.push_back(reader.readUInt32BE());
                                            m_size += m_sampleSizes.back();
                                        } else {
                                            m_size += defaultSampleSize;
                                        }
                                        if (trunFlags & 0x000400) { // sample-flags present
                                            m_istream->seekg(4, ios_base::cur);
                                        }
                                        if (trunFlags & 0x000800) { // sample-composition-time-offsets present
                                            m_istream->seekg(4, ios_base::cur);
                                        }
                                    }
                                }
                            }
                        }
                        if (m_sampleSizes.empty() && defaultSampleSize) {
                            m_sampleSizes.push_back(defaultSampleSize);
                        }
                    }
                }
            }
        }
    }

    // set duration from "trun-information" if the duration has not been determined yet
    if (m_duration.isNull() && totalDuration) {
        std::uint32_t timeScale = m_timeScale;
        if (!timeScale) {
            timeScale = trakAtom().container().timeScale();
        }
        if (timeScale) {
            m_duration = TimeSpan::fromSeconds(static_cast<double>(totalDuration) / static_cast<double>(timeScale));
        }
    }

    // calculate average bitrate
    if (m_bitrate < 0.01 && m_bitrate > -0.01) {
        m_bitrate = (static_cast<double>(m_size) * 0.0078125) / m_duration.totalSeconds();
    }

    // read stsc atom (only number of entries)
    m_istream->seekg(static_cast<streamoff>(m_stscAtom->dataOffset() + 4));
    m_sampleToChunkEntryCount = reader.readUInt32BE();
}

} // namespace TagParser
