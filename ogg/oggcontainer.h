#ifndef MEDIA_OGGCONTAINER_H
#define MEDIA_OGGCONTAINER_H

#include "oggpage.h"
#include "oggstream.h"
#include "oggiterator.h"

#include "tagparser/vorbis/vorbiscomment.h"

#include "tagparser/genericcontainer.h"

#include <unordered_map>
#include <tuple>

namespace Media {

class MediaFileInfo;

struct LIB_EXPORT VorbisCommentInfo
{
    VorbisCommentInfo(std::vector<OggPage>::size_type firstPageIndex, std::vector<OggPage>::size_type firstSegmentIndex, std::vector<OggPage>::size_type tagIndex);

    std::vector<OggPage>::size_type firstPageIndex;
    std::vector<OggPage>::size_type firstSegmentIndex;
    std::vector<OggPage>::size_type lastPageIndex;
    std::vector<OggPage>::size_type lastSegmentIndex;
    std::vector<std::unique_ptr<VorbisComment> >::size_type tagIndex;
};

inline VorbisCommentInfo::VorbisCommentInfo(std::vector<OggPage>::size_type firstPageIndex, std::vector<OggPage>::size_type firstSegmentIndex, std::vector<OggPage>::size_type tagIndex) :
    firstPageIndex(firstPageIndex),
    firstSegmentIndex(firstSegmentIndex),
    lastPageIndex(0),
    lastSegmentIndex(0),
    tagIndex(tagIndex)
{}

class LIB_EXPORT OggContainer : public GenericContainer<MediaFileInfo, VorbisComment, OggStream, OggPage>
{
    friend class OggStream;

public:
    OggContainer(MediaFileInfo &fileInfo, uint64 startOffset);
    ~OggContainer();

    bool isChecksumValidationEnabled() const;
    void setChecksumValidationEnabled(bool enabled);

protected:
    void internalParseHeader();
    void internalParseTags();
    void internalParseTracks();
    void internalMakeFile();

private:
    void ariseComment(std::vector<OggPage>::size_type pageIndex, std::vector<uint32>::size_type segmentIndex);

    std::unordered_map<uint32, std::vector<std::unique_ptr<OggStream> >::size_type> m_streamsBySerialNo;

    /*!
     * \brief Consists of first page index, first segment index, last page index, last segment index and tag index (in this order).
     */
    std::list<VorbisCommentInfo> m_commentTable;

    OggIterator m_iterator;
    bool m_validateChecksums;
};

/*!
 * \brief Returns whether checksum validation is enabled.
 *
 * If checksum validation is enabled, the parser will validate the OGG pages by
 * checking the CRC32 checksum.
 *
 * \sa setChecksumValidationEnabled()
 */
inline bool OggContainer::isChecksumValidationEnabled() const
{
    return m_validateChecksums;
}

/*!
 * \brief Sets whether checksum validation is enabled.
 * \sa isChecksumValidationEnabled()
 */
inline void OggContainer::setChecksumValidationEnabled(bool enabled)
{
    m_validateChecksums = enabled;
}

}

#endif // MEDIA_OGGCONTAINER_H
