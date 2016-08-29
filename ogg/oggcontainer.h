#ifndef MEDIA_OGGCONTAINER_H
#define MEDIA_OGGCONTAINER_H

#include "./oggpage.h"
#include "./oggstream.h"
#include "./oggiterator.h"

#include "../vorbis/vorbiscomment.h"

#include "../genericcontainer.h"

#include <unordered_map>
#include <tuple>

namespace IoUtilities {
template<std::size_t bufferSize>
class CopyHelper;
}

namespace Media {

class MediaFileInfo;
class OggContainer;

/*!
 * \brief The OggParameter struct holds the OGG parameter for a VorbisComment.
 */
struct TAG_PARSER_EXPORT OggParameter
{
    OggParameter();
    void set(std::size_t pageIndex, std::size_t segmentIndex, bool lastMetaDataBlock, GeneralMediaFormat streamFormat = GeneralMediaFormat::Vorbis);

    std::size_t firstPageIndex;
    std::size_t firstSegmentIndex;
    std::size_t lastPageIndex;
    std::size_t lastSegmentIndex;
    bool lastMetaDataBlock;
    GeneralMediaFormat streamFormat;
    bool removed;
};

/*!
 * \brief Creates new parameters.
 * \remarks The OggContainer class is responsible for assigning sane values.
 */
inline OggParameter::OggParameter() :
    firstPageIndex(0),
    firstSegmentIndex(0),
    lastPageIndex(0),
    lastSegmentIndex(0),
    lastMetaDataBlock(false),
    streamFormat(GeneralMediaFormat::Vorbis),
    removed(false)
{}

/*!
 * \brief Sets the firstPageIndex/lastPageIndex, the firstSegmentIndex/lastSegmentIndex, whether the associated meta data block is the last one and the streamFormat.
 * \remarks Whether the associated meta data block is the last one is only relevant for FLAC streams.
 */
inline void OggParameter::set(std::size_t pageIndex, std::size_t segmentIndex, bool lastMetaDataBlock, GeneralMediaFormat streamFormat)
{
    firstPageIndex = lastPageIndex = pageIndex;
    firstSegmentIndex = lastSegmentIndex = segmentIndex;
    this->lastMetaDataBlock = lastMetaDataBlock;
    this->streamFormat = streamFormat;
}

class TAG_PARSER_EXPORT OggVorbisComment : public VorbisComment
{
    friend class OggContainer;

public:
    OggVorbisComment();
    TagType type() const;
    const char *typeName() const;
    bool supportsTarget() const;

    OggParameter &oggParams();
    const OggParameter &oggParams() const;

private:
    OggParameter m_oggParams;
};

/*!
 * \brief Constructs a new OGG Vorbis comment.
 */
inline OggVorbisComment::OggVorbisComment()
{}

inline TagType OggVorbisComment::type() const
{
    return TagType::OggVorbisComment;
}

/*!
 * \brief Returns true; the target is used to specifiy the stream.
 * \sa OggContainer::createTag(), TagTarget
 */
inline bool OggVorbisComment::supportsTarget() const
{
    return true;
}

/*!
 * \brief Returns the OGG parameter for the comment.
 *
 * Consists of first page index, first segment index, last page index, last segment index and tag index (in this order).
 * These values are used and managed by the OggContainer class and do not affect the behavior of the VorbisComment instance.
 */
inline OggParameter &OggVorbisComment::oggParams()
{
    return m_oggParams;
}

/*!
 * \brief Returns the OGG parameter for the comment.
 *
 * Consists of first page index, first segment index, last page index, last segment index and tag index (in this order).
 * These values are used and managed by the OggContainer class and do not affect the behavior of the VorbisComment instance.
 */
inline const OggParameter &OggVorbisComment::oggParams() const
{
    return m_oggParams;
}

class TAG_PARSER_EXPORT OggContainer : public GenericContainer<MediaFileInfo, OggVorbisComment, OggStream, OggPage>
{
    friend class OggStream;

public:
    OggContainer(MediaFileInfo &fileInfo, uint64 startOffset);
    ~OggContainer();

    bool isChecksumValidationEnabled() const;
    void setChecksumValidationEnabled(bool enabled);
    void reset();

    OggVorbisComment *createTag(const TagTarget &target);
    OggVorbisComment *tag(std::size_t index);
    std::size_t tagCount() const;
    bool removeTag(Tag *tag);
    void removeAllTags();

protected:
    void internalParseHeader();
    void internalParseTags();
    void internalParseTracks();
    void internalMakeFile();

private:
    void announceComment(std::size_t pageIndex, std::size_t segmentIndex, bool lastMetaDataBlock, GeneralMediaFormat mediaFormat = GeneralMediaFormat::Vorbis);
    void makeVorbisCommentSegment(std::stringstream &buffer, IoUtilities::CopyHelper<65307> &copyHelper, std::vector<uint32> &newSegmentSizes, VorbisComment *comment, OggParameter *params);

    std::unordered_map<uint32, std::vector<std::unique_ptr<OggStream> >::size_type> m_streamsBySerialNo;

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
