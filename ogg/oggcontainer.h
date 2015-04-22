#ifndef MEDIA_OGGCONTAINER_H
#define MEDIA_OGGCONTAINER_H

#include "oggpage.h"
#include "oggstream.h"
#include "oggiterator.h"

#include "../vorbis/vorbiscomment.h"

#include "../genericcontainer.h"

#include <unordered_map>
#include <tuple>

namespace Media {

class MediaFileInfo;

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
    std::list<std::tuple<std::vector<OggPage>::size_type, std::vector<uint32>::size_type, std::vector<std::unique_ptr<VorbisComment> >::size_type> > m_commentTable;
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
