#ifndef TAG_PARSER_MP4CONTAINER_H
#define TAG_PARSER_MP4CONTAINER_H

#include "./mp4atom.h"
#include "./mp4tag.h"
#include "./mp4track.h"

#include "../genericcontainer.h"

#include <c++utilities/conversion/types.h>

#include <memory>
#include <vector>

namespace TagParser {

class MediaFileInfo;

class TAG_PARSER_EXPORT Mp4Container : public GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom>
{
public:
    Mp4Container(MediaFileInfo &fileInfo, uint64 startOffset);
    ~Mp4Container() override;

    bool supportsTrackModifications() const override;
    bool isFragmented() const;
    void reset() override;
    ElementPosition determineTagPosition(Diagnostics &diag) const override;
    ElementPosition determineIndexPosition(Diagnostics &diag) const override;

protected:
    void internalParseHeader(Diagnostics &diag) override;
    void internalParseTags(Diagnostics &diag) override;
    void internalParseTracks(Diagnostics &diag) override;
    void internalMakeFile(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    void updateOffsets(const std::vector<int64> &oldMdatOffsets, const std::vector<int64> &newMdatOffsets, Diagnostics &diag);

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

#endif // TAG_PARSER_MP4CONTAINER_H
