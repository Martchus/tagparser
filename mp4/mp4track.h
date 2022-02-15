#ifndef TAG_PARSER_MP4TRACK_H
#define TAG_PARSER_MP4TRACK_H

#include "../abstracttrack.h"

#include <memory>
#include <vector>

namespace TagParser {

class Mp4Atom;
class Mpeg4Descriptor;
struct AvcConfiguration;
struct Av1Configuration;
struct TrackHeaderInfo;
struct Mp4Timings;

class TAG_PARSER_EXPORT Mpeg4AudioSpecificConfig {
public:
    Mpeg4AudioSpecificConfig();

    std::uint8_t audioObjectType;
    std::uint8_t sampleFrequencyIndex;
    std::uint32_t sampleFrequency;
    std::uint8_t channelConfiguration;
    std::uint8_t extensionAudioObjectType;
    bool sbrPresent;
    bool psPresent;
    std::uint8_t extensionSampleFrequencyIndex;
    std::uint32_t extensionSampleFrequency;
    std::uint8_t extensionChannelConfiguration;
    bool frameLengthFlag;
    bool dependsOnCoreCoder;
    std::uint16_t coreCoderDelay;
    std::uint8_t extensionFlag;
    std::uint8_t layerNr;
    std::uint8_t numOfSubFrame;
    std::uint16_t layerLength;
    std::uint8_t resilienceFlags;
    std::uint8_t epConfig;
};

class TAG_PARSER_EXPORT Mpeg4VideoSpecificConfig {
public:
    Mpeg4VideoSpecificConfig();

    std::uint8_t profile;
    std::string userData;
};

class TAG_PARSER_EXPORT Mpeg4ElementaryStreamInfo {
public:
    Mpeg4ElementaryStreamInfo();

    bool dependencyFlag() const;
    bool urlFlag() const;
    bool ocrFlag() const;
    std::uint8_t priority() const;
    std::uint8_t streamTypeId() const;
    bool upstream() const;

    std::uint16_t id;
    std::uint8_t esDescFlags;
    std::uint16_t dependsOnId;
    std::string url;
    std::uint16_t ocrId;
    std::uint8_t objectTypeId;
    std::uint8_t decCfgDescFlags;
    std::uint32_t bufferSize;
    std::uint32_t maxBitrate;
    std::uint32_t averageBitrate;
    std::unique_ptr<Mpeg4AudioSpecificConfig> audioSpecificConfig;
    std::unique_ptr<Mpeg4VideoSpecificConfig> videoSpecificConfig;
};

inline Mpeg4ElementaryStreamInfo::Mpeg4ElementaryStreamInfo()
    : id(0)
    , esDescFlags(0)
    , dependsOnId(0)
    , ocrId(0)
    , objectTypeId(0)
    , decCfgDescFlags(0)
    , bufferSize(0)
    , maxBitrate(0)
    , averageBitrate(0)
{
}

inline bool Mpeg4ElementaryStreamInfo::dependencyFlag() const
{
    return esDescFlags & 0x80;
}

inline bool Mpeg4ElementaryStreamInfo::urlFlag() const
{
    return esDescFlags & 0x40;
}

inline bool Mpeg4ElementaryStreamInfo::ocrFlag() const
{
    return esDescFlags & 0x20;
}

inline std::uint8_t Mpeg4ElementaryStreamInfo::priority() const
{
    return esDescFlags & 0x1F;
}

inline std::uint8_t Mpeg4ElementaryStreamInfo::streamTypeId() const
{
    return decCfgDescFlags >> 2;
}

inline bool Mpeg4ElementaryStreamInfo::upstream() const
{
    return decCfgDescFlags & 0x02;
}

class TAG_PARSER_EXPORT Mp4Track final : public AbstractTrack {
public:
    Mp4Track(Mp4Atom &trakAtom);
    ~Mp4Track() override;
    TrackType type() const override;

    // getter methods specific for MP4 tracks
    Mp4Atom &trakAtom();
    const std::vector<std::uint32_t> &sampleSizes() const;
    unsigned int chunkOffsetSize() const;
    std::uint32_t chunkCount() const;
    std::uint32_t sampleToChunkEntryCount() const;
    const Mpeg4ElementaryStreamInfo *mpeg4ElementaryStreamInfo() const;
    const AvcConfiguration *avcConfiguration() const;
    const Av1Configuration *av1Configuration() const;

    // methods to parse configuration details from the track header
    static std::unique_ptr<Mpeg4ElementaryStreamInfo> parseMpeg4ElementaryStreamInfo(
        CppUtilities::BinaryReader &reader, Mp4Atom *esDescAtom, Diagnostics &diag);
    static std::unique_ptr<Mpeg4AudioSpecificConfig> parseAudioSpecificConfig(
        std::istream &stream, std::uint64_t startOffset, std::uint64_t size, Diagnostics &diag);
    static std::unique_ptr<Mpeg4VideoSpecificConfig> parseVideoSpecificConfig(
        CppUtilities::BinaryReader &reader, std::uint64_t startOffset, std::uint64_t size, Diagnostics &diag);

    // methods to read the "index" (chunk offsets and sizes)
    std::vector<std::uint64_t> readChunkOffsets(bool parseFragments, Diagnostics &diag);
    std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>> readSampleToChunkTable(Diagnostics &diag);
    std::vector<std::uint64_t> readChunkSizes(TagParser::Diagnostics &diag);

    // methods to make the track header
    void bufferTrackAtoms(Diagnostics &diag);
    std::uint64_t requiredSize(Diagnostics &diag) const;
    void makeTrack(Diagnostics &diag);
    void makeTrackHeader(Diagnostics &diag);
    void makeMedia(Diagnostics &diag);
    void makeMediaInfo(Diagnostics &diag);
    void makeSampleTable(Diagnostics &diag);

    // methods to update chunk offsets
    void updateChunkOffsets(const std::vector<std::int64_t> &oldMdatOffsets, const std::vector<std::int64_t> &newMdatOffsets);
    void updateChunkOffsets(const std::vector<std::uint64_t> &chunkOffsets);
    void updateChunkOffset(std::uint32_t chunkIndex, std::uint64_t offset);

    static void addInfo(const AvcConfiguration &avcConfig, AbstractTrack &track);
    static void addInfo(const Av1Configuration &av1Config, AbstractTrack &track);

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    // private helper methods
    std::uint64_t accumulateSampleSizes(std::size_t &sampleIndex, std::size_t count, Diagnostics &diag);
    void addChunkSizeEntries(
        std::vector<std::uint64_t> &chunkSizeTable, std::size_t count, std::size_t &sampleIndex, std::uint32_t sampleCount, Diagnostics &diag);
    const TrackHeaderInfo &verifyPresentTrackHeader() const;
    Mp4Timings computeTimings() const;

    Mp4Atom *m_trakAtom;
    Mp4Atom *m_tkhdAtom;
    Mp4Atom *m_mdiaAtom;
    Mp4Atom *m_mdhdAtom;
    Mp4Atom *m_hdlrAtom;
    Mp4Atom *m_minfAtom;
    Mp4Atom *m_stblAtom;
    Mp4Atom *m_stsdAtom;
    Mp4Atom *m_stscAtom;
    Mp4Atom *m_stcoAtom;
    Mp4Atom *m_stszAtom;
    std::uint32_t m_rawMediaType;
    std::uint16_t m_framesPerSample;
    std::vector<std::uint32_t> m_sampleSizes;
    unsigned int m_chunkOffsetSize;
    std::uint32_t m_chunkCount;
    std::uint32_t m_sampleToChunkEntryCount;
    std::uint64_t m_rawTkhdCreationTime;
    std::uint64_t m_rawMdhdCreationTime;
    std::uint64_t m_rawTkhdModificationTime;
    std::uint64_t m_rawMdhdModificationTime;
    std::uint64_t m_rawTkhdDuration;
    std::uint64_t m_rawMdhdDuration;
    std::unique_ptr<Mpeg4ElementaryStreamInfo> m_esInfo;
    std::unique_ptr<AvcConfiguration> m_avcConfig;
    std::unique_ptr<Av1Configuration> m_av1Config;
    mutable std::unique_ptr<TrackHeaderInfo> m_trackHeaderInfo;
};

/*!
 * \brief Returns the trak atom for the current instance.
 */
inline Mp4Atom &Mp4Track::trakAtom()
{
    return *m_trakAtom;
}

/*!
 * \brief Returns the sample size table for the track.
 * \remarks If the table contains only one size this is the constant
 *          sample size.
 * \remarks The table is empty if the track denotes 64-bit sample sizes.
 * \sa sampleSizes64()
 */
inline const std::vector<std::uint32_t> &Mp4Track::sampleSizes() const
{
    return m_sampleSizes;
}

/*!
 * \brief Returns the size of a single chunk offset denotation within the stco atom.
 *
 * Valid values are 4 and 8 bytes.
 */
inline unsigned int Mp4Track::chunkOffsetSize() const
{
    return m_chunkOffsetSize;
}

/*!
 * \brief Returns the number of chunks denoted by the stco atom.
 */
inline std::uint32_t Mp4Track::chunkCount() const
{
    return m_chunkCount;
}

/*!
 * \brief Returns the number of "sample to chunk" entries within the stsc atom.
 */
inline std::uint32_t Mp4Track::sampleToChunkEntryCount() const
{
    return m_sampleToChunkEntryCount;
}

/*!
 * \brief Returns information about the MPEG-4 elementary stream.
 * \remarks
 *  - The track must be parsed before this information becomes available.
 *  - The information is only available, if the track has an MPEG-4 elementary stream descriptor atom.
 *  - The track keeps ownership over the returned object.
 */
inline const Mpeg4ElementaryStreamInfo *Mp4Track::mpeg4ElementaryStreamInfo() const
{
    return m_esInfo.get();
}

/*!
 * \brief Returns the AVC configuration.
 * \remarks
 *  - The track must be parsed before this information becomes available.
 *  - The track keeps ownership over the returned object.
 */
inline const AvcConfiguration *Mp4Track::avcConfiguration() const
{
    return m_avcConfig.get();
}

/*!
 * \brief Returns the AV1 configuration.
 * \remarks
 *  - The track must be parsed before this information becomes available.
 *  - The track keeps ownership over the returned object.
 */
inline const Av1Configuration *Mp4Track::av1Configuration() const
{
    return m_av1Config.get();
}

} // namespace TagParser

#endif // TAG_PARSER_MP4TRACK_H
