#ifndef MP4TRACK_H
#define MP4TRACK_H

#include "../avc/avcconfiguration.h"

#include "../abstracttrack.h"

#include <vector>

namespace Media
{

class Mp4Atom;

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
    std::vector<uint64> readChunkOffsets();
    std::vector<std::tuple<uint32, uint32, uint32> > readSampleToChunkTable();
    std::vector<uint64> readChunkSizes();
    AvcConfiguration readAvcConfiguration();
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
    uint16 m_framesPerSample;
    std::vector<uint32> m_sampleSizes;
    unsigned int m_chunkOffsetSize;
    uint32 m_chunkCount;
    uint32 m_sampleToChunkEntryCount;
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

}

#endif // MP4TRACK_H
