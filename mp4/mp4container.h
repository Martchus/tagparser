#ifndef TAG_PARSER_MP4CONTAINER_H
#define TAG_PARSER_MP4CONTAINER_H

#include "./mp4atom.h"
#include "./mp4tag.h"
#include "./mp4track.h"

#include "../genericcontainer.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace TagParser {

class MediaFileInfo;

class TAG_PARSER_EXPORT Mp4Container final : public GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom> {
public:
    Mp4Container(MediaFileInfo &fileInfo, std::uint64_t startOffset);
    ~Mp4Container() override;

    bool supportsTrackModifications() const override;
    bool isFragmented() const;
    void reset() override;
    ElementPosition determineTagPosition(Diagnostics &diag) const override;
    ElementPosition determineIndexPosition(Diagnostics &diag) const override;

    static const CppUtilities::DateTime epoch;

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;
    void internalParseTags(Diagnostics &diag, AbortableProgressFeedback &progress) override;
    void internalParseTracks(Diagnostics &diag, AbortableProgressFeedback &progress) override;
    void internalMakeFile(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    void updateOffsets(const std::vector<std::int64_t> &oldMdatOffsets, const std::vector<std::int64_t> &newMdatOffsets, Diagnostics &diag,
        AbortableProgressFeedback &progress);

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

} // namespace TagParser

#endif // TAG_PARSER_MP4CONTAINER_H
