#ifndef MEDIA_MP4CONTAINER_H
#define MEDIA_MP4CONTAINER_H

#include "./mp4atom.h"
#include "./mp4tag.h"
#include "./mp4track.h"

#include "../genericcontainer.h"

#include <c++utilities/conversion/types.h>

#include <memory>
#include <vector>

namespace Media {

class MediaFileInfo;

class LIB_EXPORT Mp4Container : public GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom>
{
public:
    Mp4Container(MediaFileInfo &fileInfo, uint64 startOffset);
    ~Mp4Container();

    bool supportsTrackModifications() const;
    bool isFragmented() const;
    void reset();

protected:
    void internalParseHeader();
    void internalParseTags();
    void internalParseTracks();
    void internalMakeFile();

private:
    void updateOffsets(const std::vector<int64> &oldMdatOffsets, const std::vector<int64> &newMdatOffsets);

    bool m_fragmented;
};

inline bool Mp4Container::supportsTrackModifications() const
{
    return true;
}

/*!
 * \brief Returns whether the file is fragmented.
 * Track information needs to be parsed to detect fragmentation.
 */
inline bool Mp4Container::isFragmented() const
{
    return m_fragmented;
}

}

#endif // MEDIA_MP4CONTAINER_H
