#ifndef TAG_PARSER_MP4TRACK_H
#define TAG_PARSER_MP4TRACK_H

#include "../abstracttrack.h"

#include <vector>
#include <memory>

namespace TagParser
{

class Mp4Atom;
class Mpeg4Descriptor;
struct AvcConfiguration;
struct TrackHeaderInfo;

class TAG_PARSER_EXPORT Mpeg4AudioSpecificConfig
{
public:
    Mpeg4AudioSpecificConfig();

    byte audioObjectType;
    byte sampleFrequencyIndex;
    uint32 sampleFrequency;
    byte channelConfiguration;
    byte extensionAudioObjectType;
    bool sbrPresent;
    bool psPresent;
    byte extensionSampleFrequencyIndex;
    uint32 extensionSampleFrequency;
    byte extensionChannelConfiguration;
    bool frameLengthFlag;
    bool dependsOnCoreCoder;
    uint16 coreCoderDelay;
    byte extensionFlag;
    byte layerNr;
    byte numOfSubFrame;
    uint16 layerLength;
    byte resilienceFlags;
    byte epConfig;
};

class TAG_PARSER_EXPORT Mpeg4VideoSpecificConfig
{
public:
    Mpeg4VideoSpecificConfig();

    byte profile;
    std::string userData;
};

class TAG_PARSER_EXPORT Mpeg4ElementaryStreamInfo
{
public:
    Mpeg4ElementaryStreamInfo();

    bool dependencyFlag() const;
    bool urlFlag() const;
    bool ocrFlag() const;
    byte priority() const;
    byte streamTypeId() const;
    bool upstream() const;

    uint16 id;
    byte esDescFlags;
    uint16 dependsOnId;
    std::string url;
    uint16 ocrId;
    byte objectTypeId;
    byte decCfgDescFlags;
    uint32 bufferSize;
    uint32 maxBitrate;
    uint32 averageBitrate;
    std::unique_ptr<Mpeg4AudioSpecificConfig> audioSpecificConfig;
    std::unique_ptr<Mpeg4VideoSpecificConfig> videoSpecificConfig;
};

inline Mpeg4ElementaryStreamInfo::Mpeg4ElementaryStreamInfo() :
    id(0),
    esDescFlags(0),
    dependsOnId(0),
    ocrId(0),
    objectTypeId(0),
    decCfgDescFlags(0),
    bufferSize(0),
    maxBitrate(0),
    averageBitrate(0)
{}

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

inline byte Mpeg4ElementaryStreamInfo::priority() const
{
    return esDescFlags & 0x1F;
}

inline byte Mpeg4ElementaryStreamInfo::streamTypeId() const
{
    return decCfgDescFlags >> 2;
}

inline bool Mpeg4ElementaryStreamInfo::upstream() const
{
    return decCfgDescFlags & 0x02;
}

class TAG_PARSER_EXPORT Mp4Track : public AbstractTrack
{
public:
    Mp4Track(Mp4Atom &trakAtom);
    ~Mp4Track() override;
    TrackType type() const override;

    // getter methods specific for MP4 tracks
    Mp4Atom &trakAtom();
    const std::vector<uint32> &sampleSizes() const;
    unsigned int chunkOffsetSize() const;
    uint32 chunkCount() const;
    uint32 sampleToChunkEntryCount() const;
    const Mpeg4ElementaryStreamInfo *mpeg4ElementaryStreamInfo() const;
    const AvcConfiguration *avcConfiguration() const;

    // methods to parse configuration details from the track header
    static std::unique_ptr<Mpeg4ElementaryStreamInfo> parseMpeg4ElementaryStreamInfo(IoUtilities::BinaryReader &reader, Mp4Atom *esDescAtom, Diagnostics &diag);
    static std::unique_ptr<Mpeg4AudioSpecificConfig> parseAudioSpecificConfig(std::istream &stream, uint64 startOffset, uint64 size, Diagnostics &diag);
    static std::unique_ptr<Mpeg4VideoSpecificConfig> parseVideoSpecificConfig(IoUtilities::BinaryReader &reader, uint64 startOffset, uint64 size, Diagnostics &diag);

    // methods to read the "index" (chunk offsets and sizes)
    std::vector<uint64> readChunkOffsets(bool parseFragments, Diagnostics &diag);
    std::vector<std::tuple<uint32, uint32, uint32> > readSampleToChunkTable(Diagnostics &diag);
    std::vector<uint64> readChunkSizes(TagParser::Diagnostics &diag);

    // methods to make the track header
    void bufferTrackAtoms(Diagnostics &diag);
    uint64 requiredSize(Diagnostics &diag) const;
    void makeTrack(Diagnostics &diag);
    void makeTrackHeader(Diagnostics &diag);
    void makeMedia(Diagnostics &diag);
    void makeMediaInfo(Diagnostics &diag);
    void makeSampleTable(Diagnostics &diag);

    // methods to update chunk offsets
    void updateChunkOffsets(const std::vector<int64> &oldMdatOffsets, const std::vector<int64> &newMdatOffsets);
    void updateChunkOffsets(const std::vector<uint64> &chunkOffsets);
    void updateChunkOffset(uint32 chunkIndex, uint64 offset);

    static void addInfo(const AvcConfiguration &avcConfig, AbstractTrack &track);

protected:
    void internalParseHeader(Diagnostics &diag) override;

private:
    // private helper methods
    uint64 accumulateSampleSizes(size_t &sampleIndex, size_t count, Diagnostics &diag);
    void addChunkSizeEntries(std::vector<uint64> &chunkSizeTable, size_t count, size_t &sampleIndex, uint32 sampleCount, Diagnostics &diag);
    TrackHeaderInfo verifyPresentTrackHeader() const;

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
    uint16 m_framesPerSample;
    std::vector<uint32> m_sampleSizes;
    unsigned int m_chunkOffsetSize;
    uint32 m_chunkCount;
    uint32 m_sampleToChunkEntryCount;
    std::unique_ptr<Mpeg4ElementaryStreamInfo> m_esInfo;
    std::unique_ptr<AvcConfiguration> m_avcConfig;
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
inline const std::vector<uint32> &Mp4Track::sampleSizes() const
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
inline uint32 Mp4Track::chunkCount() const
{
    return m_chunkCount;
}

/*!
 * \brief Returns the number of "sample to chunk" entries within the stsc atom.
 */
inline uint32 Mp4Track::sampleToChunkEntryCount() const
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

}

#endif // TAG_PARSER_MP4TRACK_H
