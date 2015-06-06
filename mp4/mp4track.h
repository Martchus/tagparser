#ifndef MP4TRACK_H
#define MP4TRACK_H

#include "../avc/avcconfiguration.h"

#include "../abstracttrack.h"

#include <vector>
#include <memory>

namespace Media
{

class Mp4Atom;

class LIB_EXPORT Mpeg4ElementaryStreamInfo
{
public:
    Mpeg4ElementaryStreamInfo();

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

    bool dependencyFlag() const;
    bool urlFlag() const;
    bool ocrFlag() const;
    byte priority() const;
    byte streamTypeId() const;
    bool upstream() const;
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

class LIB_EXPORT Mp4Track : public AbstractTrack
{
public:
    Mp4Track(Mp4Atom &trakAtom);
    ~Mp4Track();

    TrackType type() const;
    Mp4Atom &trakAtom();

    const std::vector<uint32> &sampleSizes() const;
    unsigned int chunkOffsetSize() const;
    uint32 chunkCount() const;
    uint32 sampleToChunkEntryCount() const;
    const Mpeg4ElementaryStreamInfo *mpeg4ElementaryStreamInfo() const;
    std::vector<uint64> readChunkOffsets();
    std::vector<std::tuple<uint32, uint32, uint32> > readSampleToChunkTable();
    std::vector<uint64> readChunkSizes();
    AvcConfiguration parseAvcConfiguration();
    bool hasMpeg4ElementaryStreamDesc() const;
    void parseMpeg4ElementaryStreamInfo();
    void updateChunkOffsets(const std::vector<int64> &oldMdatOffsets, const std::vector<int64> &newMdatOffsets);
    void updateChunkOffset(uint32 chunkIndex, uint64 offset);
    void makeTrack();
    void makeTrackHeader();
    void makeMedia();
    void makeMediaInfo();
    void makeSampleTable();

protected:
    void internalParseHeader();

private:
    uint64 accumulateSampleSizes(size_t &sampleIndex, size_t count);
    void addChunkSizeEntries(std::vector<uint64> &chunkSizeTable, size_t count, size_t &sampleIndex, uint32 sampleCount);

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
    Mp4Atom *m_codecConfigAtom;
    Mp4Atom *m_esDescAtom;
    uint16 m_framesPerSample;
    std::vector<uint32> m_sampleSizes;
    unsigned int m_chunkOffsetSize;
    uint32 m_chunkCount;
    uint32 m_sampleToChunkEntryCount;
    std::unique_ptr<Mpeg4ElementaryStreamInfo> m_esInfo;
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
 *  - The Mp4Track::readMpeg4ElementaryStreamInfo() method must be called before
 *    to parse the information. This is done when parsing the track.
 *  - The information is only available, if the track has an MPEG-4 elementary stream
 *    descriptor atom.
 *  - The track keeps ownership over the returned object.
 * \sa
 *  - readMpeg4ElementaryStreamInfo()
 *  - hasMpeg4ElementaryStreamDesc()
 */
inline const Mpeg4ElementaryStreamInfo *Mp4Track::mpeg4ElementaryStreamInfo() const
{
    return m_esInfo.get();
}

/*!
 * \brief Returns whether the track has an MPEG-4 elementary stream descriptor atom.
 */
inline bool Mp4Track::hasMpeg4ElementaryStreamDesc() const
{
    return m_esDescAtom != nullptr;
}

}

#endif // MP4TRACK_H
