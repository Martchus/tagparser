#include "mp4atom.h"
#include "mp4container.h"
#include "mp4track.h"
#include "mp4ids.h"
#include "mpeg4descriptor.h"

#include "../mpegaudio/mpegaudioframe.h"
#include "../mpegaudio/mpegaudioframestream.h"

#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/io/bitreader.h>

#include <locale>
#include <cmath>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

namespace Media {

DateTime startDate = DateTime::fromDate(1904, 1, 1);

uint32 sampleRateTable[] = {
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000
};

MediaFormat fmtTable[] = {
    GeneralMediaFormat::Unknown,
    MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4MainProfile),
    MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LowComplexityProfile),
    MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4ScalableSamplingRateProfile),
    MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LongTermPrediction),
    MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4SpectralBandReplicationProfile)
};

Mpeg4AudioSpecificConfig::Mpeg4AudioSpecificConfig() :
    audioObjectType(0),
    sampleFrequencyIndex(0xF),
    sampleFrequency(0),
    channelConfiguration(0),
    extensionAudioObjectType(0),
    sbrPresent(false),
    psPresent(false),
    extensionSampleFrequencyIndex(0xF),
    extensionSampleFrequency(0),
    extensionChannelConfiguration(0),
    frameLengthFlag(false),
    dependsOnCoreCoder(false),
    coreCoderDelay(0),
    extensionFlag(0),
    layerNr(0),
    numOfSubFrame(0),
    layerLength(0),
    resilienceFlags(0),
    epConfig(0)
{}

Mpeg4VideoSpecificConfig::Mpeg4VideoSpecificConfig()
{}

/*!
 * \class Media::Mp4Track
 * \brief Implementation of Media::AbstractTrack for the MP4 container.
 */

/*!
 * \brief Constructs a new track for the specified \a trakAtom.
 *
 * "trak"-atoms are stored in the top-level atom "move". Each "trak"-atom holds
 * header information for one track in the MP4 file.
 */
Mp4Track::Mp4Track(Mp4Atom &trakAtom) :
    AbstractTrack(trakAtom.stream(), trakAtom.startOffset()),
    m_trakAtom(&trakAtom),
    m_tkhdAtom(nullptr),
    m_mdiaAtom(nullptr),
    m_mdhdAtom(nullptr),
    m_hdlrAtom(nullptr),
    m_minfAtom(nullptr),
    m_stblAtom(nullptr),
    m_stsdAtom(nullptr),
    m_stscAtom(nullptr),
    m_stcoAtom(nullptr),
    m_stszAtom(nullptr),
    //m_codecConfigAtom(nullptr),
    //m_esDescAtom(nullptr),
    m_framesPerSample(1),
    m_chunkOffsetSize(4),
    m_chunkCount(0),
    m_sampleToChunkEntryCount(0)
{}

/*!
 * \brief Destroys the track.
 */
Mp4Track::~Mp4Track()
{}

TrackType Mp4Track::type() const
{
    return TrackType::Mp4Track;
}

/*!
 * \brief Reads the chunk offsets from the stco atom.
 * \returns Returns the chunk offset table for the track.
 * \throws Throws InvalidDataException when
 *          - there is no stream assigned.
 *          - the header has been considered as invalid when parsing the header information.
 *          - the determined chunk offset size is invalid.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 *
 * \sa readChunkSizes();
 */
vector<uint64> Mp4Track::readChunkOffsets()
{
    static const string context("reading chunk offset table of MP4 track");
    if(!isHeaderValid() || !m_istream) {
        addNotification(NotificationType::Critical, "Track has not been parsed.", context);
        throw InvalidDataException();
    }
    vector<uint64> offsets;
    if(m_stcoAtom) {
        // verify integrity of the chunk offset table
        uint64 actualTableSize = m_stcoAtom->dataSize();
        if(actualTableSize < (8 + chunkOffsetSize())) {
            addNotification(NotificationType::Critical, "The stco atom is truncated. There are no chunk offsets present.", context);
            throw InvalidDataException();
        } else {
            actualTableSize -= 8;
        }
        uint32 actualChunkCount = chunkCount();
        uint64 calculatedTableSize = chunkCount() * chunkOffsetSize();
        if(calculatedTableSize < actualTableSize) {
            addNotification(NotificationType::Critical, "The stco atom stores more chunk offsets as denoted. The additional chunk offsets will be ignored.", context);
        } else if(calculatedTableSize > actualTableSize) {
            addNotification(NotificationType::Critical, "The stco atom is truncated. It stores less chunk offsets as denoted.", context);
            actualChunkCount = floor(static_cast<double>(actualTableSize) / static_cast<double>(chunkOffsetSize()));
        }
        // read the table
        offsets.reserve(actualChunkCount);
        m_istream->seekg(m_stcoAtom->dataOffset() + 8);
        switch(chunkOffsetSize()) {
        case 4:
            for(uint32 i = 0; i < actualChunkCount; ++i) {
                offsets.push_back(reader().readUInt32BE());
            }
            break;
        case 8:
            for(uint32 i = 0; i < actualChunkCount; ++i) {
                offsets.push_back(reader().readUInt64BE());
            }
            break;
        default:
            addNotification(NotificationType::Critical, "The determined chunk offset size is invalid.", context);
            throw InvalidDataException();
        }
    }
    // read sample offsets of fragments
    //    Mp4Atom *moofAtom = m_trakAtom->container().firstElement()->siblingById(moof, true);
    //    uint64 totalDuration = 0;
    //    while(moofAtom) {
    //        moofAtom->parse();
    //        Mp4Atom *trafAtom = moofAtom->childById(traf);
    //        while(trafAtom) {
    //            trafAtom->parse();
    //            Mp4Atom *tfhdAtom = trafAtom->childById(tfhd);
    //            while(tfhdAtom) {
    //                tfhdAtom->parse();
    //                uint32 calculatedDataSize = 0;
    //                if(tfhdAtom->dataSize() < calculatedDataSize) {
    //                    addNotification(NotificationType::Critical, "tfhd atom is truncated.", context);
    //                } else {
    //                    m_stream->seekg(tfhdAtom->dataOffset() + 1);
    //                    uint32 flags = reader.readUInt24();
    //                    if(m_id == reader.readUInt32()) { // check track ID
    //                        if(flags & 0x000001) { // base-data-offset present
    //                            calculatedDataSize += 8;
    //                        }
    //                        if(flags & 0x000002) { // sample-description-index present
    //                            calculatedDataSize += 4;
    //                        }
    //                        if(flags & 0x000008) { // default-sample-duration present
    //                            calculatedDataSize += 4;
    //                        }
    //                        if(flags & 0x000010) { // default-sample-size present
    //                            calculatedDataSize += 4;
    //                        }
    //                        if(flags & 0x000020) { // default-sample-flags present
    //                            calculatedDataSize += 4;
    //                        }
    //                        //uint64 baseDataOffset = moofAtom->startOffset();
    //                        //uint32 defaultSampleDescriptionIndex = 0;
    //                        uint32 defaultSampleDuration = 0;
    //                        uint32 defaultSampleSize = 0;
    //                        uint32 defaultSampleFlags = 0;
    //                        if(tfhdAtom->dataSize() < calculatedDataSize) {
    //                            addNotification(NotificationType::Critical, "tfhd atom is truncated (presence of fields denoted).", context);
    //                        } else {
    //                            if(flags & 0x000001) { // base-data-offset present
    //                                //baseDataOffset = reader.readUInt64();
    //                                m_stream->seekg(8, ios_base::cur);
    //                            }
    //                            if(flags & 0x000002) { // sample-description-index present
    //                                //defaultSampleDescriptionIndex = reader.readUInt32();
    //                                m_stream->seekg(4, ios_base::cur);
    //                            }
    //                            if(flags & 0x000008) { // default-sample-duration present
    //                                defaultSampleDuration = reader.readUInt32();
    //                                //m_stream->seekg(4, ios_base::cur);
    //                            }
    //                            if(flags & 0x000010) { // default-sample-size present
    //                                defaultSampleSize = reader.readUInt32();
    //                            }
    //                            if(flags & 0x000020) { // default-sample-flags present
    //                                defaultSampleFlags = reader.readUInt32();
    //                                //m_stream->seekg(4, ios_base::cur);
    //                            }
    //                        }
    //                        Mp4Atom *trunAtom = trafAtom->childById(trun);
    //                        while(trunAtom) {
    //                            uint32 calculatedDataSize = 8;
    //                            if(trunAtom->dataSize() < calculatedDataSize) {
    //                                addNotification(NotificationType::Critical, "trun atom is truncated.", context);
    //                            } else {
    //                                m_stream->seekg(trunAtom->dataOffset() + 1);
    //                                uint32 flags = reader.readUInt24();
    //                                uint32 sampleCount = reader.readUInt32();
    //                                m_sampleCount += sampleCount;
    //                                if(flags & 0x000001) { // data offset present
    //                                    calculatedDataSize += 4;
    //                                }
    //                                if(flags & 0x000004) { // first-sample-flags present
    //                                    calculatedDataSize += 4;
    //                                }
    //                                uint32 entrySize = 0;
    //                                if(flags & 0x000100) { // sample-duration present
    //                                    entrySize += 4;
    //                                }
    //                                if(flags & 0x000200) { // sample-size present
    //                                    entrySize += 4;
    //                                }
    //                                if(flags & 0x000400) { // sample-flags present
    //                                    entrySize += 4;
    //                                }
    //                                if(flags & 0x000800) { // sample-composition-time-offsets present
    //                                    entrySize += 4;
    //                                }
    //                                calculatedDataSize += entrySize * sampleCount;
    //                                if(trunAtom->dataSize() < calculatedDataSize) {
    //                                    addNotification(NotificationType::Critical, "trun atom is truncated (presence of fields denoted).", context);
    //                                } else {
    //                                    if(flags & 0x000001) { // data offset present
    //                                        m_stream->seekg(4, ios_base::cur);
    //                                        //int32 dataOffset = reader.readInt32();
    //                                    }
    //                                    if(flags & 0x000004) { // first-sample-flags present
    //                                        m_stream->seekg(4, ios_base::cur);
    //                                    }
    //                                    for(uint32 i = 0; i < sampleCount; ++i) {
    //                                        if(flags & 0x000100) { // sample-duration present
    //                                            totalDuration += reader.readUInt32();
    //                                        } else {
    //                                            totalDuration += defaultSampleDuration;
    //                                        }
    //                                        if(flags & 0x000200) { // sample-size present
    //                                            m_sampleSizes.push_back(reader.readUInt32());
    //                                            m_size += m_sampleSizes.back();
    //                                        } else {
    //                                            m_size += defaultSampleSize;
    //                                        }
    //                                        if(flags & 0x000400) { // sample-flags present
    //                                            m_stream->seekg(4, ios_base::cur);
    //                                        }
    //                                        if(flags & 0x000800) { // sample-composition-time-offsets present
    //                                            m_stream->seekg(4, ios_base::cur);
    //                                        }
    //                                    }
    //                                }
    //                            }
    //                            trunAtom = trunAtom->siblingById(trun, false);
    //                        }
    //                        if(m_sampleSizes.empty() && defaultSampleSize) {
    //                            m_sampleSizes.push_back(defaultSampleSize);
    //                        }
    //                    }
    //                }
    //                tfhdAtom = tfhdAtom->siblingById(tfhd, false);
    //            }
    //            trafAtom = trafAtom->siblingById(traf, false);
    //        }
    //        moofAtom = moofAtom->siblingById(moof, false);
    //    }
    return offsets;
}

/*!
 * \brief Accumulates \a count sample sizes from the specified \a sampleSizeTable starting at the specified \a sampleIndex.
 * \remarks This helper function is used by the addChunkSizeEntries() method.
 */
uint64 Mp4Track::accumulateSampleSizes(size_t &sampleIndex, size_t count)
{
    if(sampleIndex + count <= m_sampleSizes.size()) {
        uint64 sum = 0;
        for(size_t end = sampleIndex + count; sampleIndex < end; ++sampleIndex) {
            sum += m_sampleSizes[sampleIndex];
        }
        return sum;
    } else if(m_sampleSizes.size() == 1) {
        sampleIndex += count;
        return static_cast<uint64>(m_sampleSizes.front()) * count;
    } else {
        addNotification(NotificationType::Critical, "There are not as many sample size entries as samples.", "reading chunk sizes of MP4 track");
        throw InvalidDataException();
    }
}

/*!
 * \brief Adds chunks size entries to the specified \a chunkSizeTable.
 * \param chunkSizeTable Specifies the chunk size table. The chunks sizes will be added to this table.
 * \param count Specifies the number of chunks to be added. The size of \a chunkSizeTable is increased this value.
 * \param sampleIndex Specifies the index of the first sample in the \a sampleSizeTable; is increased by \a count * \a sampleCount.
 * \param sampleSizeTable Specifies the table holding the sample sizes.
 * \remarks This helper function is used by the readChunkSize() method.
 */
void Mp4Track::addChunkSizeEntries(std::vector<uint64> &chunkSizeTable, size_t count, size_t &sampleIndex, uint32 sampleCount)
{
    for(size_t i = 0; i < count; ++i) {
        chunkSizeTable.push_back(accumulateSampleSizes(sampleIndex, sampleCount));
    }
}

/*!
 * \brief Reads the sample to chunk table.
 * \returns Returns a vector with the table entries wrapped using the tuple container. The first value
 *          is an integer that gives the first chunk that share the same samples count and sample description index.
 *          The second value is sample cound and the third value the sample description index.
 * \remarks The table is not validated.
 */
vector<tuple<uint32, uint32, uint32> > Mp4Track::readSampleToChunkTable()
{
    static const string context("reading sample to chunk table of MP4 track");
    if(!isHeaderValid() || !m_istream || !m_stscAtom) {
        addNotification(NotificationType::Critical, "Track has not been parsed or is invalid.", context);
        throw InvalidDataException();
    }
    // verify integrity of the sample to chunk table
    uint64 actualTableSize = m_stscAtom->dataSize();
    if(actualTableSize < 20) {
        addNotification(NotificationType::Critical, "The stsc atom is truncated. There are no \"sample to chunk\" entries present.", context);
        throw InvalidDataException();
    } else {
        actualTableSize -= 8;
    }
    uint32 actualSampleToChunkEntryCount = sampleToChunkEntryCount();
    uint64 calculatedTableSize = actualSampleToChunkEntryCount * 12;
    if(calculatedTableSize < actualTableSize) {
        addNotification(NotificationType::Critical, "The stsc atom stores more entries as denoted. The additional entries will be ignored.", context);
    } else if(calculatedTableSize > actualTableSize) {
        addNotification(NotificationType::Critical, "The stsc atom is truncated. It stores less entries as denoted.", context);
        actualSampleToChunkEntryCount = floor(static_cast<double>(actualTableSize) / 12.0);
    }
    // prepare reading
    vector<tuple<uint32, uint32, uint32> > sampleToChunkTable;
    sampleToChunkTable.reserve(actualSampleToChunkEntryCount);
    m_istream->seekg(m_stscAtom->dataOffset() + 8);
    for(uint32 i = 0; i < actualSampleToChunkEntryCount; ++i) {
        // read entry
        uint32 firstChunk = reader().readUInt32BE();
        uint32 samplesPerChunk = reader().readUInt32BE();
        uint32 sampleDescriptionIndex = reader().readUInt32BE();
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
vector<uint64> Mp4Track::readChunkSizes()
{
    static const string context("reading chunk sizes of MP4 track");
    if(!isHeaderValid() || !m_istream || !m_stcoAtom) {
        addNotification(NotificationType::Critical, "Track has not been parsed or is invalid.", context);
        throw InvalidDataException();
    }
    // read sample to chunk table
    const auto sampleToChunkTable = readSampleToChunkTable();
    // accumulate chunk sizes from the table
    vector<uint64> chunkSizes;
    if(!sampleToChunkTable.empty()) {
        // prepare reading
        auto tableIterator = sampleToChunkTable.cbegin();
        chunkSizes.reserve(m_chunkCount);
        // read first entry
        size_t sampleIndex = 0;
        uint32 previousChunkIndex = get<0>(*tableIterator); // the first chunk has the index 1 and not zero!
        if(previousChunkIndex != 1) {
            addNotification(NotificationType::Critical, "The first chunk of the first \"sample to chunk\" entry must be 1.", context);
            previousChunkIndex = 1; // try to read the entry anyway
        }
        uint32 samplesPerChunk = get<1>(*tableIterator);
        // read the following entries
        ++tableIterator;
        for(const auto tableEnd = sampleToChunkTable.cend(); tableIterator != tableEnd; ++tableIterator) {
            uint32 firstChunkIndex = get<0>(*tableIterator);
            if(firstChunkIndex > previousChunkIndex && firstChunkIndex <= m_chunkCount) {
                addChunkSizeEntries(chunkSizes, firstChunkIndex - previousChunkIndex, sampleIndex, samplesPerChunk);
            } else {
                addNotification(NotificationType::Critical,
                                "The first chunk index of a \"sample to chunk\" entry must be greather then the first chunk of the previous entry and not greather then the chunk count.", context);
                throw InvalidDataException();
            }
            previousChunkIndex = firstChunkIndex;
            samplesPerChunk = get<1>(*tableIterator);
        }
        if(m_chunkCount >= previousChunkIndex) {
            addChunkSizeEntries(chunkSizes, m_chunkCount + 1 - previousChunkIndex, sampleIndex, samplesPerChunk);
        }
    }
    return chunkSizes;
}

/*!
 * \brief Reads the AVC configuration for the track.
 * \remarks
 *  - Returns an empty configuration for non-AVC tracks.
 *  - Notifications might be added.
 */
AvcConfiguration Mp4Track::parseAvcConfiguration(Mp4Atom *avcConfigAtom)
{
    AvcConfiguration config;
    if(avcConfigAtom) {
        try {
            auto configSize = avcConfigAtom->dataSize();
            if(avcConfigAtom && configSize >= 5) {
                // skip first byte (is always 1)
                m_istream->seekg(avcConfigAtom->dataOffset() + 1);
                // read profile, IDC level, NALU size length
                config.profileIdc = m_reader.readByte();
                config.profileCompat = m_reader.readByte();
                config.levelIdc = m_reader.readByte();
                config.naluSizeLength = m_reader.readByte() & 0x03;
                // read SPS infos
                if((configSize -= 5) >= 3) {
                    byte entryCount = m_reader.readByte() & 0x0f;
                    uint16 entrySize;
                    while(entryCount && configSize) {
                        if((entrySize = m_reader.readUInt16BE()) <= configSize) {
                            // TODO: read entry
                            configSize -= entrySize;
                        } else {
                            throw TruncatedDataException();
                        }
                        --entryCount;
                    }
                    // read PPS infos
                    if((configSize -= 5) >= 3) {
                        entryCount = m_reader.readByte();
                        while(entryCount && configSize) {
                            if((entrySize = m_reader.readUInt16BE()) <= configSize) {
                                // TODO: read entry
                                configSize -= entrySize;
                            } else {
                                throw TruncatedDataException();
                            }
                            --entryCount;
                        }
                        // TODO: read trailer
                        return config;
                    }
                }
            }
            throw TruncatedDataException();
        } catch (TruncatedDataException &) {
            addNotification(NotificationType::Critical, "AVC configuration is truncated.", "parsing AVC configuration");
        }
    }
    return config;
}

/*!
 * \brief Reads the MPEG-4 elementary stream descriptor for the track.
 * \remarks
 *  - Notifications might be added.
 * \sa mpeg4ElementaryStreamInfo()
 */
std::unique_ptr<Mpeg4ElementaryStreamInfo> Mp4Track::parseMpeg4ElementaryStreamInfo(Mp4Atom *esDescAtom)
{
    static const string context("parsing MPEG-4 elementary stream descriptor");
    using namespace Mpeg4ElementaryStreamObjectIds;
    unique_ptr<Mpeg4ElementaryStreamInfo> esInfo;
    if(esDescAtom->dataSize() >= 12) {
        m_istream->seekg(esDescAtom->dataOffset());
        // read version/flags
        if(m_reader.readUInt32BE() != 0) {
            addNotification(NotificationType::Warning, "Unknown version/flags.", context);
        }
        // read extended descriptor
        Mpeg4Descriptor esDesc(esDescAtom->container(), m_istream->tellg(), esDescAtom->dataSize() - 4);
        try {
            esDesc.parse();
            // check ID
            if(esDesc.id() != Mpeg4DescriptorIds::ElementaryStreamDescr) {
                addNotification(NotificationType::Critical, "Invalid descriptor found.", context);
                throw Failure();
            }
            // read stream info
            m_istream->seekg(esDesc.dataOffset());
            esInfo = make_unique<Mpeg4ElementaryStreamInfo>();
            esInfo->id = m_reader.readUInt16BE();
            esInfo->esDescFlags = m_reader.readByte();
            if(esInfo->dependencyFlag()) {
                esInfo->dependsOnId = m_reader.readUInt16BE();
            }
            if(esInfo->urlFlag()) {
                esInfo->url = m_reader.readString(m_reader.readByte());
            }
            if(esInfo->ocrFlag()) {
                esInfo->ocrId = m_reader.readUInt16BE();
            }
            for(Mpeg4Descriptor *esDescChild = esDesc.denoteFirstChild(static_cast<uint64>(m_istream->tellg()) - esDesc.startOffset()); esDescChild; esDescChild = esDescChild->nextSibling()) {
                esDescChild->parse();
                switch(esDescChild->id()) {
                case Mpeg4DescriptorIds::DecoderConfigDescr:
                    // read decoder config descriptor
                    m_istream->seekg(esDescChild->dataOffset());
                    esInfo->objectTypeId = m_reader.readByte();
                    esInfo->decCfgDescFlags = m_reader.readByte();
                    esInfo->bufferSize = m_reader.readUInt24BE();
                    esInfo->maxBitrate = m_reader.readUInt32BE();
                    esInfo->averageBitrate = m_reader.readUInt32BE();
                    for(Mpeg4Descriptor *decCfgDescChild = esDescChild->denoteFirstChild(esDescChild->headerSize() + 13); decCfgDescChild; decCfgDescChild = decCfgDescChild->nextSibling()) {
                        decCfgDescChild->parse();
                        switch(decCfgDescChild->id()) {
                        case Mpeg4DescriptorIds::DecoderSpecificInfo:
                            // read decoder specific info
                            switch(esInfo->objectTypeId) {
                            case Aac: case Mpeg2AacMainProfile: case Mpeg2AacLowComplexityProfile:
                            case Mpeg2AacScaleableSamplingRateProfile: case Mpeg2Audio: case Mpeg1Audio:
                                esInfo->audioSpecificConfig = parseAudioSpecificConfig(decCfgDescChild);
                                break;
                            case Mpeg4Visual:
                                esInfo->videoSpecificConfig = parseVideoSpecificConfig(decCfgDescChild);
                                break;
                            default:
                                ; // TODO: covering more object types
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
        } catch (Failure &) {
            addNotification(NotificationType::Critical, "The MPEG-4 descriptor element structure is invalid.", context);
        }
    } else {
        addNotification(NotificationType::Warning, "Elementary stream descriptor atom (esds) is truncated.", context);
    }
    return esInfo;
}

/*!
 * \brief Parses the audio specific configuration for the track.
 * \remarks
 *  - Notifications might be added.
 * \sa mpeg4ElementaryStreamInfo()
 */
unique_ptr<Mpeg4AudioSpecificConfig> Mp4Track::parseAudioSpecificConfig(Mpeg4Descriptor *decSpecInfoDesc)
{
    static const string context("parsing MPEG-4 audio specific config from elementary stream descriptor");
    using namespace Mpeg4AudioObjectIds;
    // read config into buffer and construct BitReader for bitwise reading
    m_istream->seekg(decSpecInfoDesc->dataOffset());
    //cout << "audio cfg @" << decSpecInfoDesc->dataOffset() << endl;
    auto buff = make_unique<char []>(decSpecInfoDesc->dataSize());
    m_istream->read(buff.get(), decSpecInfoDesc->dataSize());
    BitReader bitReader(buff.get(), decSpecInfoDesc->dataSize());
    auto audioCfg = make_unique<Mpeg4AudioSpecificConfig>();
    try {
        // read audio object type
        auto getAudioObjectType = [&audioCfg, &bitReader] {
            byte objType = bitReader.readBits<byte>(5);
            if(objType == 31) {
                objType = 32 + bitReader.readBits<byte>(6);
            }
            return objType;
        };
        audioCfg->audioObjectType = getAudioObjectType();
        // read sampling frequency
        if((audioCfg->sampleFrequencyIndex = bitReader.readBits<byte>(4)) == 0xF) {
            audioCfg->sampleFrequency = bitReader.readBits<uint32>(24);
        }
        // read channel config
        audioCfg->channelConfiguration = bitReader.readBits<byte>(4);
        // read extension header
        switch(audioCfg->audioObjectType) {
        case Sbr:
        case Ps:
            audioCfg->extensionAudioObjectType = Sbr;
            audioCfg->sbrPresent = true;
            if((audioCfg->extensionSampleFrequencyIndex = bitReader.readBits<byte>(4)) == 0xF) {
                audioCfg->extensionSampleFrequency = bitReader.readBits<uint32>(24);
            }
            if((audioCfg->audioObjectType = getAudioObjectType()) == ErBsac) {
                audioCfg->extensionChannelConfiguration = bitReader.readBits<byte>(4);
            }
            break;
        }
        switch(audioCfg->audioObjectType) {
        case Ps:
            audioCfg->psPresent = true;
            break;
        }
        // read GA specific config
        switch(audioCfg->audioObjectType) {
        case AacMain: case AacLc: case AacLtp: case AacScalable:
        case TwinVq: case ErAacLc: case ErAacLtp: case ErAacScalable:
        case ErTwinVq: case ErBsac: case ErAacLd:
            audioCfg->frameLengthFlag = bitReader.readBits<byte>(1);
            if((audioCfg->dependsOnCoreCoder = bitReader.readBits<byte>(1))) {
                audioCfg->coreCoderDelay = bitReader.readBits<byte>(14);
            }
            audioCfg->extensionFlag = bitReader.readBits<byte>(1);
            if(audioCfg->channelConfiguration == 0) {
                throw NotImplementedException(); // TODO: parse program_config_element
            }
            switch(audioCfg->audioObjectType) {
            case AacScalable: case ErAacScalable:
                audioCfg->layerNr = bitReader.readBits<byte>(3);
                break;
            default:
                ;
            }
            if(audioCfg->extensionFlag == 1) {
                switch(audioCfg->audioObjectType) {
                case ErBsac:
                    audioCfg->numOfSubFrame = bitReader.readBits<byte>(5);
                    audioCfg->layerLength = bitReader.readBits<uint16>(11);
                    break;
                case ErAacLc: case ErAacLtp: case ErAacScalable: case ErAacLd:
                    audioCfg->resilienceFlags = bitReader.readBits<byte>(3);
                    break;
                default:
                    ;
                }
                if(bitReader.readBits<byte>(1) == 1) { // extension flag 3
                    throw NotImplementedException(); // TODO
                }
            }
            break;
        default:
            throw NotImplementedException(); // TODO: cover remaining object types
        }
        // read error specific config
        switch(audioCfg->audioObjectType) {
        case ErAacLc: case ErAacLtp: case ErAacScalable: case ErTwinVq:
        case ErBsac: case ErAacLd: case ErCelp: case ErHvxc: case ErHiln:
        case ErParametric: case ErAacEld:
            switch(audioCfg->epConfig = bitReader.readBits<byte>(2)) {
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
        if(audioCfg->extensionAudioObjectType != 5 && bitReader.bitsAvailable() >= 16) {
            uint16 syncExtensionType = bitReader.readBits<uint16>(11);
            if(syncExtensionType == 0x2B7) {
                if((audioCfg->extensionAudioObjectType = getAudioObjectType()) == Sbr) {
                    if((audioCfg->sbrPresent = bitReader.readBits<byte>(1))) {
                        if((audioCfg->extensionSampleFrequencyIndex = bitReader.readBits<byte>(4)) == 0xF) {
                            audioCfg->extensionSampleFrequency = bitReader.readBits<uint32>(24);
                        }
                        if(bitReader.bitsAvailable() >= 12) {
                            if((syncExtensionType = bitReader.readBits<uint16>(11)) == 0x548) {
                                audioCfg->psPresent = bitReader.readBits<byte>(1);
                            }
                        }
                    }
                }
            } else if (syncExtensionType == 0x548) {
                audioCfg->psPresent = bitReader.readBits<byte>(1);
            }
        }
    } catch(ios_base::failure &) {
        if(m_istream->fail()) {
            // IO error caused by input stream
            throw;
        } else {
            // IO error caused by bitReader
            addNotification(NotificationType::Critical, "Audio specific configuration is truncated.", context);
        }
    } catch(NotImplementedException &) {
        addNotification(NotificationType::Information, "Not implemented for the format of audio track.", context);
    }
    return audioCfg;
}

/*!
 * \brief Parses the video specific configuration for the track.
 * \remarks
 *  - Notifications might be added.
 * \sa mpeg4ElementaryStreamInfo()
 */
std::unique_ptr<Mpeg4VideoSpecificConfig> Mp4Track::parseVideoSpecificConfig(Mpeg4Descriptor *decSpecInfoDesc)
{
    static const string context("parsing MPEG-4 video specific config from elementary stream descriptor");
    using namespace Mpeg4AudioObjectIds;
    auto videoCfg = make_unique<Mpeg4VideoSpecificConfig>();
    // seek to start
    m_istream->seekg(decSpecInfoDesc->dataOffset());
    uint64 bytesRemaining = decSpecInfoDesc->dataSize();
    if(bytesRemaining > 3 && (m_reader.readUInt24BE() == 1)) {
        bytesRemaining -= 3;
        uint32 buff1;
        while(bytesRemaining) {
            --bytesRemaining;
            switch(m_reader.readByte()) { // read start code
            case Mpeg4VideoCodes::VisualObjectSequenceStart:
                if(bytesRemaining) {
                    videoCfg->profile = m_reader.readByte();
                    --bytesRemaining;
                }
                break;
            case Mpeg4VideoCodes::VideoObjectLayerStart:

                break;
            case Mpeg4VideoCodes::UserDataStart:
                buff1 = 0;
                while(bytesRemaining >= 3) {
                    if((buff1 = m_reader.readUInt24BE()) != 1) {
                        m_istream->seekg(-2, ios_base::cur);
                        videoCfg->userData.push_back(buff1 >> 16);
                        --bytesRemaining;
                    } else {
                        bytesRemaining -= 3;
                        break;
                    }
                }
                if(buff1 != 1 && bytesRemaining > 0) {
                    videoCfg->userData += m_reader.readString(bytesRemaining);
                    bytesRemaining = 0;
                }
                break;
            default:
                ;
            }
            // skip stuff we're not interested in to get the start of the
            // next video object
            while(bytesRemaining >= 3) {
                if(m_reader.readUInt24BE() != 1) {
                    m_istream->seekg(-2, ios_base::cur);
                    --bytesRemaining;
                } else {
                    bytesRemaining -= 3;
                    break;
                }
            }
        }
    } else {
        addNotification(NotificationType::Critical, "\"Visual Object Sequence Header\" not found.", context);
    }
    return videoCfg;
}

/*!
 * \brief Updates the chunk offsets of the track. This is necessary when the mdat atom (which contains
 *        the actual chunk data) is moved.
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
 */
void Mp4Track::updateChunkOffsets(const vector<int64> &oldMdatOffsets, const vector<int64> &newMdatOffsets)
{
    if(!isHeaderValid() || !m_ostream || !m_istream || !m_stcoAtom) {
        throw InvalidDataException();
    }
    if(oldMdatOffsets.size() == 0 || oldMdatOffsets.size() != newMdatOffsets.size()) {
        throw InvalidDataException();
    }
    static const unsigned int stcoDataBegin = 8;
    uint64 startPos = m_stcoAtom->dataOffset() + stcoDataBegin;
    uint64 endPos = startPos + m_stcoAtom->totalSize() - stcoDataBegin;
    m_istream->seekg(startPos);
    m_ostream->seekp(startPos);
    vector<int64>::size_type i;
    vector<int64>::size_type size;
    uint64 currentPos = m_istream->tellg();
    switch(m_stcoAtom->id()) {
    case Mp4AtomIds::ChunkOffset: {
        uint32 off;
        while((currentPos + 4) <= endPos) {
            off = m_reader.readUInt32BE();
            for(i = 0, size = oldMdatOffsets.size(); i < size; ++i) {
                if(off > static_cast<uint64>(oldMdatOffsets[i])) {
                    off += (newMdatOffsets[i] - oldMdatOffsets[i]);
                    break;
                }
            }
            m_ostream->seekp(currentPos);
            m_writer.writeUInt32BE(off);
            currentPos += m_istream->gcount();
        }
        break;
    } case Mp4AtomIds::ChunkOffset64: {
        uint64 off;
        while((currentPos + 8) <= endPos) {
            off = m_reader.readUInt64BE();
            for(i = 0, size = oldMdatOffsets.size(); i < size; ++i) {
                if(off > static_cast<uint64>(oldMdatOffsets[i])) {
                    off += (newMdatOffsets[i] - oldMdatOffsets[i]);
                    break;
                }
            }
            m_ostream->seekp(currentPos);
            m_writer.writeUInt64BE(off);
            currentPos += m_istream->gcount();
        }
        break;
    }
    default:
        throw InvalidDataException();
    }
}

/*!
 * \brief Updates a particular chunk offset.
 * \param chunkIndex Specifies the index of the chunk offset to be updated.
 * \param offset Specifies the new chunk offset.
 */
void Mp4Track::updateChunkOffset(uint32 chunkIndex, uint64 offset)
{
    if(!isHeaderValid() || !m_istream || !m_stcoAtom || chunkIndex >= m_chunkCount) {
        throw InvalidDataException();
    }
    m_ostream->seekp(m_stcoAtom->dataOffset() + 8 + chunkOffsetSize() * chunkIndex);
    switch(chunkOffsetSize()) {
    case 4:
        writer().writeUInt32BE(offset);
        break;
    case 8:
        writer().writeUInt64BE(offset);
        break;
    default:
        throw InvalidDataException();
    }
}

/*!
 * \brief Makes the track entry ("trak"-atom) for the track. The data is written to the assigned output stream
 *        at the current position.
 * \remarks Currently the "trak"-atom from the source file is just copied to the output stream.
 */
void Mp4Track::makeTrack()
{
    /*
    // write header
    ostream::pos_type trakStartOffset = outputStream().tellp();
    writer.writeUInt32(0); // write size later
    writer.writeUInt32(Mp4AtomIds::Track);
    // write tkhd atom
    makeTrackHeader();
    // write tref atom (if one exists)
    if(Mp4Atom *trefAtom = trakAtom().childById(Mp4AtomIds::TrackReference)) {
        trefAtom->copyEntireAtomToStream(outputStream());
    }
    // write edts atom (if one exists)
    if(Mp4Atom *edtsAtom = trakAtom().childById(Mp4AtomIds::Edit)) {
        edtsAtom->copyEntireAtomToStream(outputStream());
    }
    // write mdia atom
    makeMedia();
    // write size (of trak atom)
    Mp4Atom::seekBackAndWriteAtomSize(outputStream(), trakStartOffset, false);
    */
    trakAtom().copyEntirely(outputStream());
}

/*!
 * \brief Makes the track header (tkhd atom) for the track. The data is written to the assigned output stream
 *        at the current position.
 */
void Mp4Track::makeTrackHeader()
{
    writer().writeUInt32BE(100); // size
    writer().writeUInt32BE(Mp4AtomIds::TrackHeader);
    writer().writeByte(1); // version
    uint32 flags = 0;
    if(m_enabled) {
        flags |= 0x000001;
    }
    if(m_usedInPresentation) {
        flags |= 0x000002;
    }
    if(m_usedWhenPreviewing) {
        flags |= 0x000004;
    }
    writer().writeUInt24BE(flags);
    writer().writeUInt64BE(static_cast<uint64>((m_creationTime - startDate).totalSeconds()));
    writer().writeUInt64BE(static_cast<uint64>((m_modificationTime - startDate).totalSeconds()));
    writer().writeUInt32BE(m_id);
    writer().writeUInt32BE(0); // reserved
    writer().writeUInt64BE(static_cast<uint64>(m_duration.totalSeconds() * m_timeScale));
    writer().writeUInt32BE(0); // reserved
    writer().writeUInt32BE(0); // reserved
    if(m_tkhdAtom) {
        // use existing values
        char buffer[48];
        m_istream->seekg(m_tkhdAtom->startOffset() + 52);
        m_istream->read(buffer, sizeof(buffer));
        m_ostream->write(buffer, sizeof(buffer));
    } else {
        // write default values
        writer().writeInt16BE(0); // layer
        writer().writeInt16BE(0); // alternate group
        writer().writeFixed8BE(1.0); // volume
        writer().writeUInt16BE(0); // reserved
        for(int32 value : {0x00010000,0,0,0,0x00010000,0,0,0,0x40000000}) { // unity matrix
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
void Mp4Track::makeMedia()
{
    ostream::pos_type mdiaStartOffset = outputStream().tellp();
    writer().writeUInt32BE(0); // write size later
    writer().writeUInt32BE(Mp4AtomIds::Media);
    // write mdhd atom
    writer().writeUInt32BE(36); // size
    writer().writeByte(1); // version
    writer().writeUInt24BE(0); // flags
    writer().writeUInt64BE(static_cast<uint64>((m_creationTime - startDate).totalSeconds()));
    writer().writeUInt64BE(static_cast<uint64>((m_modificationTime - startDate).totalSeconds()));
    writer().writeUInt32BE(m_timeScale);
    writer().writeUInt64BE(static_cast<uint64>(m_duration.totalSeconds() * m_timeScale));
    // convert and write language
    uint16 language = 0;
    for(size_t charIndex = 0; charIndex < m_language.length() && charIndex < 3; ++charIndex) {
        if(m_language[charIndex] >= 'a' && m_language[charIndex] <= 'z') {
            language |= static_cast<uint16>(m_language[charIndex]) << (0xA - charIndex * 0x5);
        } else { // invalid character
            addNotification(NotificationType::Warning, "Assigned language \"" + m_language + "\" is of an invalid format and will be ignored.", "making mdhd atom");
            language = 0x55C4; // und
            break;
        }
    }
    writer().writeUInt16BE(language);
    writer().writeUInt16BE(0); // pre defined
    // write hdlr atom
    writer().writeUInt32BE(33 + m_name.length()); // size
    writer().writeUInt32BE(Mp4AtomIds::HandlerReference);
    writer().writeUInt64BE(0); // version, flags, pre defined
    switch(m_mediaType) {
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
        outputStream().write("meta", 4);
        break;
    default:
        addNotification(NotificationType::Critical, "Media type is invalid; The media type video is assumed.", "making hdlr atom");
        outputStream().write("vide", 4);
        break;
    }
    for(int i = 0; i < 3; ++i) writer().writeUInt32BE(0); // reserved
    writer().writeTerminatedString(m_name);
    // write minf atom
    makeMediaInfo();
    // write size (of mdia atom)
    Mp4Atom::seekBackAndWriteAtomSize(outputStream(), mdiaStartOffset, false);
}

/*!
 * \brief Makes a media information (minf atom) for the track. The data is written to the assigned output stream
 *        at the current position.
 */
void Mp4Track::makeMediaInfo()
{
    ostream::pos_type minfStartOffset = outputStream().tellp();
    writer().writeUInt32BE(0); // write size later
    writer().writeUInt32BE(Mp4AtomIds::MediaInformation);
    bool dinfAtomWritten = false;
    if(m_minfAtom) {
        // copy existing vmhd atom
        if(Mp4Atom *vmhdAtom = m_minfAtom->childById(Mp4AtomIds::VideoMediaHeader)) {
            vmhdAtom->copyEntirely(outputStream());
        }
        // copy existing smhd atom
        if(Mp4Atom *smhdAtom = m_minfAtom->childById(Mp4AtomIds::SoundMediaHeader)) {
            smhdAtom->copyEntirely(outputStream());
        }
        // copy existing hmhd atom
        if(Mp4Atom *hmhdAtom = m_minfAtom->childById(Mp4AtomIds::HintMediaHeader)) {
            hmhdAtom->copyEntirely(outputStream());
        }
        // copy existing nmhd atom
        if(Mp4Atom *nmhdAtom = m_minfAtom->childById(Mp4AtomIds::NullMediaHeaderBox)) {
            nmhdAtom->copyEntirely(outputStream());
        }
        // copy existing dinf atom
        if(Mp4Atom *dinfAtom = m_minfAtom->childById(Mp4AtomIds::DataInformation)) {
            dinfAtom->copyEntirely(outputStream());
            dinfAtomWritten = true;
        }
    }
    // write dinf atom if not written yet
    if(!dinfAtomWritten) {
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
    makeSampleTable();
    // write size (of minf atom)
    Mp4Atom::seekBackAndWriteAtomSize(outputStream(), minfStartOffset, false);
}

/*!
 * \brief Makes the sample table (stbl atom) for the track. The data is written to the assigned output stream
 *        at the current position.
 * \remarks Not fully implemented yet.
 */
void Mp4Track::makeSampleTable()
{
    ostream::pos_type stblStartOffset = outputStream().tellp();
    writer().writeUInt32BE(0); // write size later
    writer().writeUInt32BE(Mp4AtomIds::SampleTable);
    Mp4Atom *stblAtom = m_minfAtom ? m_minfAtom->childById(Mp4AtomIds::SampleTable) : nullptr;
    // write stsd atom
    if(m_stsdAtom) {
        // copy existing stsd atom
        m_stsdAtom->copyEntirely(outputStream());
    } else {
        addNotification(NotificationType::Critical, "Unable to make stsd atom from scratch.", "making stsd atom");
        throw NotImplementedException();
    }
    // write stts and ctts atoms
    Mp4Atom *sttsAtom = stblAtom ? stblAtom->childById(Mp4AtomIds::DecodingTimeToSample) : nullptr;
    if(sttsAtom) {
        // copy existing stts atom
        sttsAtom->copyEntirely(outputStream());
    } else {
        addNotification(NotificationType::Critical, "Unable to make stts atom from scratch.", "making stts atom");
        throw NotImplementedException();
    }
    Mp4Atom *cttsAtom = stblAtom ? stblAtom->childById(Mp4AtomIds::CompositionTimeToSample) : nullptr;
    if(cttsAtom) {
        // copy existing ctts atom
        cttsAtom->copyEntirely(outputStream());
    }
    // write stsc atom (sample-to-chunk table)

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

    // write size (of stbl atom)
    Mp4Atom::seekBackAndWriteAtomSize(outputStream(), stblStartOffset, false);
}

void Mp4Track::internalParseHeader()
{
    static const string context("parsing MP4 track");
    using namespace Mp4AtomIds;
    if(!m_trakAtom) {
        addNotification(NotificationType::Critical, "\"trak\"-atom is null.", context);
        throw InvalidDataException();
    }
    // get atoms
    try {
        if(!(m_tkhdAtom = m_trakAtom->childById(TrackHeader))) {
            addNotification(NotificationType::Critical, "No \"tkhd\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_mdiaAtom = m_trakAtom->childById(Media))) {
            addNotification(NotificationType::Critical, "No \"mdia\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_mdhdAtom = m_mdiaAtom->childById(MediaHeader))) {
            addNotification(NotificationType::Critical, "No \"mdhd\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_hdlrAtom = m_mdiaAtom->childById(HandlerReference))) {
            addNotification(NotificationType::Critical, "No \"hdlr\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_minfAtom = m_mdiaAtom->childById(MediaInformation))) {
            addNotification(NotificationType::Critical, "No \"minf\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_stblAtom = m_minfAtom->childById(SampleTable))) {
            addNotification(NotificationType::Critical, "No \"stbl\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_stsdAtom = m_stblAtom->childById(SampleDescription))) {
            addNotification(NotificationType::Critical, "No \"stsd\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_stcoAtom = m_stblAtom->childById(ChunkOffset)) && !(m_stcoAtom = m_stblAtom->childById(ChunkOffset64))) {
            addNotification(NotificationType::Critical, "No \"stco\"/\"co64\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_stscAtom = m_stblAtom->childById(SampleToChunk))) {
            addNotification(NotificationType::Critical, "No \"stsc\"-atom found.", context);
            throw InvalidDataException();
        }
        if(!(m_stszAtom = m_stblAtom->childById(SampleSize)) && !(m_stszAtom = m_stblAtom->childById(CompactSampleSize))) {
            addNotification(NotificationType::Critical, "No \"stsz\"/\"stz2\"-atom found.", context);
            throw InvalidDataException();
        }
    } catch(Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse relevant atoms.", context);
        throw InvalidDataException();
    }
    BinaryReader &reader = m_trakAtom->reader();
    // read tkhd atom
    m_istream->seekg(m_tkhdAtom->startOffset() + 8); // seek to beg, skip size and name
    byte atomVersion = reader.readByte(); // read version
    uint32 flags = reader.readUInt24BE();
    m_enabled = flags & 0x000001;
    m_usedInPresentation = flags & 0x000002;
    m_usedWhenPreviewing = flags & 0x000004;
    switch(atomVersion) {
    case 0:
        m_creationTime = startDate + TimeSpan::fromSeconds(reader.readUInt32BE());
        m_modificationTime = startDate + TimeSpan::fromSeconds(reader.readUInt32BE());
        m_id = reader.readUInt32BE();
        break;
    case 1:
        m_creationTime = startDate + TimeSpan::fromSeconds(reader.readUInt64BE());
        m_modificationTime = startDate + TimeSpan::fromSeconds(reader.readUInt64BE());
        m_id = reader.readUInt32BE();
        break;
    default:
        addNotification(NotificationType::Critical, "Version of \"tkhd\"-atom not supported. It will be ignored. Track ID, creation time and modification time might not be be determined.", context);
        m_creationTime = DateTime();
        m_modificationTime = DateTime();
        m_id = 0;
    }
    // read mdhd atom
    m_istream->seekg(m_mdhdAtom->startOffset() + 8); // seek to beg, skip size and name
    atomVersion = reader.readByte(); // read version
    m_istream->seekg(3, ios_base::cur); // skip flags
    switch(atomVersion) {
    case 0:
        m_creationTime = startDate + TimeSpan::fromSeconds(reader.readUInt32BE());
        m_modificationTime = startDate + TimeSpan::fromSeconds(reader.readUInt32BE());
        m_timeScale = reader.readUInt32BE();
        m_duration = TimeSpan::fromSeconds(static_cast<double>(reader.readUInt32BE()) / static_cast<double>(m_timeScale));
        break;
    case 1:
        m_creationTime = startDate + TimeSpan::fromSeconds(reader.readUInt64BE());
        m_modificationTime = startDate + TimeSpan::fromSeconds(reader.readUInt64BE());
        m_timeScale = reader.readUInt32BE();
        m_duration = TimeSpan::fromSeconds(static_cast<double>(reader.readUInt64BE()) / static_cast<double>(m_timeScale));
        break;
    default:
        addNotification(NotificationType::Warning, "Version of \"mdhd\"-atom not supported. It will be ignored. Creation time, modification time, time scale and duration might not be determined.", context);
        m_timeScale = 0;
        m_duration = TimeSpan();
    }
    uint16 rawLanguage = reader.readUInt16BE();
    char buff[3];
    buff[0] = ((rawLanguage & 0x7C00) >> 0xA) + 0x60;
    buff[1] = ((rawLanguage & 0x03E0) >> 0x5) + 0x60;
    buff[2] = ((rawLanguage & 0x001F) >> 0x0) + 0x60;
    m_language = string(buff, 3);
    // read hdlr atom
    //  track type
    m_istream->seekg(m_hdlrAtom->startOffset() + 16); // seek to beg, skip size, name, version, flags and reserved bytes
    string trackTypeStr = reader.readString(4);
    if(trackTypeStr == "soun") {
        m_mediaType = MediaType::Audio;
    } else if(trackTypeStr == "vide") {
        m_mediaType = MediaType::Video;
    } else if(trackTypeStr == "hint") {
        m_mediaType = MediaType::Hint;
    } else if(trackTypeStr == "meta") {
        m_mediaType = MediaType::Text;
    } else {
        m_mediaType = MediaType::Unknown;
    }
    //  name
    m_istream->seekg(12, ios_base::cur); // skip reserved bytes
    //name = reader.readString(hdlrAtom->size - 16 - 4 - 12);
    m_name = reader.readTerminatedString(m_hdlrAtom->totalSize() - 12 - 4 - 12, 0);
    // read stco atom (only chunk count)
    m_chunkOffsetSize = (m_stcoAtom->id() == Mp4AtomIds::ChunkOffset64) ? 8 : 4;
    m_istream->seekg(m_stcoAtom->dataOffset() + 4);
    m_chunkCount = reader.readUInt32BE();
    // read stsd atom
    m_istream->seekg(m_stsdAtom->dataOffset() + 4); // seek to beg, skip size, name, version and flags
    uint32 entryCount = reader.readUInt32BE();
    Mp4Atom *esDescParentAtom = nullptr;
    uint16 tmp;
    if(entryCount > 0) {
        try {
            for(Mp4Atom *codecConfigContainerAtom = m_stsdAtom->firstChild(); codecConfigContainerAtom; codecConfigContainerAtom = codecConfigContainerAtom->nextSibling()) {
                codecConfigContainerAtom->parse();
                // parse FOURCC
                m_formatId = interpretIntegerAsString<uint32>(codecConfigContainerAtom->id());
                m_format = FourccIds::fourccToMediaFormat(codecConfigContainerAtom->id());
                // parse codecConfigContainerAtom
                m_istream->seekg(codecConfigContainerAtom->dataOffset());
                switch(codecConfigContainerAtom->id()) {
                case FourccIds::Mpeg4Audio: case FourccIds::AmrNarrowband: case FourccIds::Amr:
                case FourccIds::Drms: case FourccIds::Alac: case FourccIds::WindowsMediaAudio:
                case FourccIds::Ac3: case FourccIds::EAc3: case FourccIds::DolbyMpl:
                case FourccIds::Dts: case FourccIds::DtsH: case FourccIds::DtsE:
                    m_istream->seekg(6 + 2, ios_base::cur); // skip reserved bytes, data reference index
                    tmp = reader.readUInt16BE(); // read sound version
                    m_istream->seekg(6, ios_base::cur);
                    m_channelCount = reader.readUInt16BE();
                    m_bitsPerSample = reader.readUInt16BE();
                    m_istream->seekg(4, ios_base::cur); // skip reserved bytes (again)
                    if(!m_sampleRate) {
                        m_sampleRate = reader.readUInt32BE() >> 16;
                        if(codecConfigContainerAtom->id() != FourccIds::DolbyMpl) {
                            m_sampleRate >>= 16;
                        }
                    } else {
                        m_istream->seekg(4, ios_base::cur);
                    }
                    if(codecConfigContainerAtom->id() != FourccIds::WindowsMediaAudio) {
                        switch(tmp) {
                        case 1:
                            codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 28 + 16);
                            break;
                        case 2:
                            codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 28 + 32);
                            break;
                        default:
                            codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 28);
                        }
                        if(!esDescParentAtom) {
                            esDescParentAtom = codecConfigContainerAtom;
                        }
                    }
                    break;
                case FourccIds::Mpeg4Video: case FourccIds::H263Quicktime: case FourccIds::H2633GPP:
                case FourccIds::Avc1: case FourccIds::Avc2: case FourccIds::Avc3: case FourccIds::Avc4:
                case FourccIds::Drmi: case FourccIds::Hevc1: case FourccIds::Hevc2:
                    m_istream->seekg(6 + 2 + 16, ios_base::cur); // skip reserved bytes, data reference index, and reserved bytes (again)
                    m_pixelSize.setWidth(reader.readUInt16BE());
                    m_pixelSize.setHeight(reader.readUInt16BE());
                    m_resolution.setWidth(static_cast<uint32>(reader.readFixed16BE()));
                    m_resolution.setHeight(static_cast<uint32>(reader.readFixed16BE()));
                    m_istream->seekg(4, ios_base::cur); // skip reserved bytes
                    m_framesPerSample = reader.readUInt16BE();
                    tmp = reader.readByte();
                    m_compressorName = reader.readString(31);
                    if(tmp == 0) {
                        m_compressorName.clear();
                    } else if(tmp < 32) {
                        m_compressorName.resize(tmp);
                    }
                    m_depth = reader.readUInt16BE(); // 24: color without alpha
                    codecConfigContainerAtom->denoteFirstChild(codecConfigContainerAtom->headerSize() + 78);
                    if(!esDescParentAtom) {
                        esDescParentAtom = codecConfigContainerAtom;
                    }
                    break;
                case Mp4AtomIds::PixalAspectRatio:
                    break; // TODO
                case Mp4AtomIds::CleanAperature:
                    break; // TODO
                default:
                    ;
                }
            }
            // parse AVC configuration
            //codecConfigContainerAtom->childById(Mp4AtomIds::AvcConfiguration);
            // parse MPEG-4 elementary stream descriptor
            if(esDescParentAtom) {
                Mp4Atom *esDescAtom = esDescParentAtom->childById(Mp4FormatExtensionIds::Mpeg4ElementaryStreamDescriptor);
                if(!esDescAtom) {
                    esDescAtom = esDescParentAtom->childById(Mp4FormatExtensionIds::Mpeg4ElementaryStreamDescriptor2);
                }
                if(esDescAtom) {
                    try {
                        if((m_esInfo = parseMpeg4ElementaryStreamInfo(esDescAtom))) {
                            m_format += Mpeg4ElementaryStreamObjectIds::streamObjectTypeFormat(m_esInfo->objectTypeId);
                            m_bitrate = static_cast<double>(m_esInfo->averageBitrate) / 1000;
                            m_maxBitrate = static_cast<double>(m_esInfo->maxBitrate) / 1000;
                            if(m_esInfo->audioSpecificConfig) {
                                // check the audio specific config for useful information
                                m_format += Mpeg4AudioObjectIds::idToMediaFormat(m_esInfo->audioSpecificConfig->audioObjectType, m_esInfo->audioSpecificConfig->sbrPresent, m_esInfo->audioSpecificConfig->psPresent);
                                if(m_esInfo->audioSpecificConfig->sampleFrequencyIndex == 0xF) {
                                    m_sampleRate = m_esInfo->audioSpecificConfig->sampleFrequency;
                                } else if(m_esInfo->audioSpecificConfig->sampleFrequencyIndex < sizeof(sampleRateTable)) {
                                    m_sampleRate = sampleRateTable[m_esInfo->audioSpecificConfig->sampleFrequencyIndex];
                                } else {
                                    addNotification(NotificationType::Warning, "Audio specific config has invalid sample frequency index.", context);
                                }
                                if(m_esInfo->audioSpecificConfig->extensionSampleFrequencyIndex == 0xF) {
                                    m_extensionSampleRate = m_esInfo->audioSpecificConfig->extensionSampleFrequency;
                                } else if(m_esInfo->audioSpecificConfig->extensionSampleFrequencyIndex < sizeof(sampleRateTable)) {
                                    m_extensionSampleRate = sampleRateTable[m_esInfo->audioSpecificConfig->extensionSampleFrequencyIndex];
                                } else {
                                    addNotification(NotificationType::Warning, "Audio specific config has invalid extension sample frequency index.", context);
                                }
                            }
                            if(m_esInfo->videoSpecificConfig) {
                                // check the video specific config for useful information
                                if(m_format.general == GeneralMediaFormat::Mpeg4Video && m_esInfo->videoSpecificConfig->profile) {
                                    m_format.sub = m_esInfo->videoSpecificConfig->profile;
                                    if(!m_esInfo->videoSpecificConfig->userData.empty()) {
                                        m_formatId += " / " + m_esInfo->videoSpecificConfig->userData;
                                    }
                                }
                            }
                            // check the stream data for missing information
                            switch(m_format.general) {
                            case GeneralMediaFormat::Mpeg1Audio: case GeneralMediaFormat::Mpeg2Audio: {
                                MpegAudioFrame frame;
                                m_istream->seekg(m_stcoAtom->dataOffset() + 8);
                                m_istream->seekg(m_chunkOffsetSize == 8 ? reader.readUInt64BE() : reader.readUInt32BE());
                                frame.parseHeader(*m_istream);
                                MpegAudioFrameStream::addInfo(frame, *this);
                                break;
                            } default:
                                ;
                            }
                        }
                    } catch(Failure &) {
                    }
                }
            }
        } catch (Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse child atoms of \"stsd\"-atom.", context);
        }
    }
    // read stsz atom which holds the sample size table
    m_sampleSizes.clear();
    m_size = m_sampleCount = 0;
    uint64 actualSampleSizeTableSize = m_stszAtom->dataSize();
    if(actualSampleSizeTableSize < 12) {
        addNotification(NotificationType::Critical, "The stsz atom is truncated. There are no sample sizes present. The size of the track can not be determined.", context);
    } else {
        actualSampleSizeTableSize -= 12; // subtract size of version and flags
        m_istream->seekg(m_stszAtom->dataOffset() + 4); // seek to beg, skip size, name, version and flags
        uint32 fieldSize;
        uint32 constantSize;
        if(m_stszAtom->id() == Mp4AtomIds::CompactSampleSize) {
            constantSize = 0;
            m_istream->seekg(3, ios_base::cur); // seek reserved bytes
            fieldSize = reader.readByte();
            m_sampleCount = reader.readUInt32BE();
        } else {
            constantSize = reader.readUInt32BE();
            m_sampleCount = reader.readUInt32BE();
            fieldSize = 32;
        }
        if(constantSize) {
            m_sampleSizes.push_back(constantSize);
            m_size = constantSize * m_sampleCount;
        } else {
            uint64 actualSampleCount = m_sampleCount;
            uint64 calculatedSampleSizeTableSize = ceil((0.125 * fieldSize) * m_sampleCount);
            if(calculatedSampleSizeTableSize < actualSampleSizeTableSize) {
                addNotification(NotificationType::Critical, "The stsz atom stores more entries as denoted. The additional entries will be ignored.", context);
            } else if(calculatedSampleSizeTableSize > actualSampleSizeTableSize) {
                addNotification(NotificationType::Critical, "The stsz atom is truncated. It stores less entries as denoted.", context);
                actualSampleCount = floor(static_cast<double>(actualSampleSizeTableSize) / (0.125 * fieldSize));
            }
            m_sampleSizes.reserve(actualSampleCount);
            uint32 i = 1;
            switch(fieldSize) {
            case 4:
                for(; i <= actualSampleCount; i += 2) {
                    byte val = reader.readByte();
                    m_sampleSizes.push_back(val >> 4);
                    m_sampleSizes.push_back(val & 0xF0);
                    m_size += (val >> 4) + (val & 0xF0);
                }
                if(i <= actualSampleCount + 1) {
                    m_sampleSizes.push_back(reader.readByte() >> 4);
                    m_size += m_sampleSizes.back();
                }
                break;
            case 8:
                for(; i <= actualSampleCount; ++i) {
                    m_sampleSizes.push_back(reader.readByte());
                    m_size += m_sampleSizes.back();
                }
                break;
            case 16:
                for(; i <= actualSampleCount; ++i) {
                    m_sampleSizes.push_back(reader.readUInt16BE());
                    m_size += m_sampleSizes.back();
                }
                break;
            case 32:
                for(; i <= actualSampleCount; ++i) {
                    m_sampleSizes.push_back(reader.readUInt32BE());
                    m_size += m_sampleSizes.back();
                }
                break;
            default:
                addNotification(NotificationType::Critical, "The fieldsize used to store the sample sizes is not supported. The sample count and size of the track can not be determined.", context);
            }
        }
    }
    // no sample sizes found, search for trun atoms
    uint64 totalDuration = 0;
    for(Mp4Atom *moofAtom = m_trakAtom->container().firstElement()->siblingById(MovieFragment, true); moofAtom; moofAtom = moofAtom->siblingById(MovieFragment, false)) {
        moofAtom->parse();
        for(Mp4Atom *trafAtom = moofAtom->childById(TrackFragment); trafAtom; trafAtom = trafAtom->siblingById(TrackFragment, false)) {
            trafAtom->parse();
            for(Mp4Atom *tfhdAtom = trafAtom->childById(TrackFragmentHeader); tfhdAtom; tfhdAtom = tfhdAtom->siblingById(TrackFragmentHeader, false)) {
                tfhdAtom->parse();
                uint32 calculatedDataSize = 0;
                if(tfhdAtom->dataSize() < calculatedDataSize) {
                    addNotification(NotificationType::Critical, "tfhd atom is truncated.", context);
                } else {
                    m_istream->seekg(tfhdAtom->dataOffset() + 1);
                    uint32 flags = reader.readUInt24BE();
                    if(m_id == reader.readUInt32BE()) { // check track ID
                        if(flags & 0x000001) { // base-data-offset present
                            calculatedDataSize += 8;
                        }
                        if(flags & 0x000002) { // sample-description-index present
                            calculatedDataSize += 4;
                        }
                        if(flags & 0x000008) { // default-sample-duration present
                            calculatedDataSize += 4;
                        }
                        if(flags & 0x000010) { // default-sample-size present
                            calculatedDataSize += 4;
                        }
                        if(flags & 0x000020) { // default-sample-flags present
                            calculatedDataSize += 4;
                        }
                        //uint64 baseDataOffset = moofAtom->startOffset();
                        //uint32 defaultSampleDescriptionIndex = 0;
                        uint32 defaultSampleDuration = 0;
                        uint32 defaultSampleSize = 0;
                        //uint32 defaultSampleFlags = 0;
                        if(tfhdAtom->dataSize() < calculatedDataSize) {
                            addNotification(NotificationType::Critical, "tfhd atom is truncated (presence of fields denoted).", context);
                        } else {
                            if(flags & 0x000001) { // base-data-offset present
                                //baseDataOffset = reader.readUInt64();
                                m_istream->seekg(8, ios_base::cur);
                            }
                            if(flags & 0x000002) { // sample-description-index present
                                //defaultSampleDescriptionIndex = reader.readUInt32();
                                m_istream->seekg(4, ios_base::cur);
                            }
                            if(flags & 0x000008) { // default-sample-duration present
                                defaultSampleDuration = reader.readUInt32BE();
                                //m_istream->seekg(4, ios_base::cur);
                            }
                            if(flags & 0x000010) { // default-sample-size present
                                defaultSampleSize = reader.readUInt32BE();
                            }
                            if(flags & 0x000020) { // default-sample-flags present
                                //defaultSampleFlags = reader.readUInt32BE();
                                m_istream->seekg(4, ios_base::cur);
                            }
                        }
                        for(Mp4Atom *trunAtom = trafAtom->childById(TrackFragmentRun); trunAtom; trunAtom = trunAtom->siblingById(TrackFragmentRun, false)) {
                            uint32 calculatedDataSize = 8;
                            if(trunAtom->dataSize() < calculatedDataSize) {
                                addNotification(NotificationType::Critical, "trun atom is truncated.", context);
                            } else {
                                m_istream->seekg(trunAtom->dataOffset() + 1);
                                uint32 flags = reader.readUInt24BE();
                                uint32 sampleCount = reader.readUInt32BE();
                                m_sampleCount += sampleCount;
                                if(flags & 0x000001) { // data offset present
                                    calculatedDataSize += 4;
                                }
                                if(flags & 0x000004) { // first-sample-flags present
                                    calculatedDataSize += 4;
                                }
                                uint32 entrySize = 0;
                                if(flags & 0x000100) { // sample-duration present
                                    entrySize += 4;
                                }
                                if(flags & 0x000200) { // sample-size present
                                    entrySize += 4;
                                }
                                if(flags & 0x000400) { // sample-flags present
                                    entrySize += 4;
                                }
                                if(flags & 0x000800) { // sample-composition-time-offsets present
                                    entrySize += 4;
                                }
                                calculatedDataSize += entrySize * sampleCount;
                                if(trunAtom->dataSize() < calculatedDataSize) {
                                    addNotification(NotificationType::Critical, "trun atom is truncated (presence of fields denoted).", context);
                                } else {
                                    if(flags & 0x000001) { // data offset present
                                        m_istream->seekg(4, ios_base::cur);
                                        //int32 dataOffset = reader.readInt32();
                                    }
                                    if(flags & 0x000004) { // first-sample-flags present
                                        m_istream->seekg(4, ios_base::cur);
                                    }
                                    for(uint32 i = 0; i < sampleCount; ++i) {
                                        if(flags & 0x000100) { // sample-duration present
                                            totalDuration += reader.readUInt32BE();
                                        } else {
                                            totalDuration += defaultSampleDuration;
                                        }
                                        if(flags & 0x000200) { // sample-size present
                                            m_sampleSizes.push_back(reader.readUInt32BE());
                                            m_size += m_sampleSizes.back();
                                        } else {
                                            m_size += defaultSampleSize;
                                        }
                                        if(flags & 0x000400) { // sample-flags present
                                            m_istream->seekg(4, ios_base::cur);
                                        }
                                        if(flags & 0x000800) { // sample-composition-time-offsets present
                                            m_istream->seekg(4, ios_base::cur);
                                        }
                                    }
                                }
                            }
                        }
                        if(m_sampleSizes.empty() && defaultSampleSize) {
                            m_sampleSizes.push_back(defaultSampleSize);
                        }
                    }
                }
            }
        }
    }
    // set duration from "trun-information" if the duration has not been determined yet
    if(m_duration.isNull() && totalDuration) {
        uint32 timeScale = m_timeScale;
        if(!timeScale) {
            timeScale = trakAtom().container().timeScale();
        }
        if(timeScale) {
            m_duration = TimeSpan::fromSeconds(static_cast<double>(totalDuration) / static_cast<double>(timeScale));
        }
    }
    // caluculate average bitrate
    if(m_bitrate < 0.01 && m_bitrate > -0.01) {
        m_bitrate = (static_cast<double>(m_size) * 0.0078125) / m_duration.totalSeconds();
    }
    // read stsc atom (only number of entries)
    m_istream->seekg(m_stscAtom->dataOffset() + 4);
    m_sampleToChunkEntryCount = reader.readUInt32BE();
}

}
