#ifndef TAG_PARSER_OGGCONTAINER_H
#define TAG_PARSER_OGGCONTAINER_H

#include "./oggiterator.h"
#include "./oggpage.h"
#include "./oggstream.h"

#include "../vorbis/vorbiscomment.h"

#include "../genericcontainer.h"

#include <tuple>
#include <unordered_map>

/// \cond
namespace CppUtilities {
template <std::size_t bufferSize> class CopyHelper;
}
/// \endcond

namespace TagParser {

class MediaFileInfo;
class OggContainer;

/*!
 * \brief The OggParameter struct holds the OGG parameter for a VorbisComment.
 */
struct TAG_PARSER_EXPORT OggParameter {
    constexpr OggParameter();
    void set(std::size_t pageIndex, std::size_t segmentIndex, bool lastMetaDataBlock, GeneralMediaFormat streamFormat = GeneralMediaFormat::Vorbis);

    std::size_t firstPageIndex;
    std::size_t firstSegmentIndex;
    std::size_t lastPageIndex;
    std::size_t lastSegmentIndex;
    GeneralMediaFormat streamFormat;
    bool lastMetaDataBlock;
    bool removed;
};

/*!
 * \brief Creates new parameters.
 * \remarks The OggContainer class is responsible for assigning sane values.
 */
constexpr OggParameter::OggParameter()
    : firstPageIndex(0)
    , firstSegmentIndex(0)
    , lastPageIndex(0)
    , lastSegmentIndex(0)
    , streamFormat(GeneralMediaFormat::Vorbis)
    , lastMetaDataBlock(false)
    , removed(false)
{
}

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

class TAG_PARSER_EXPORT OggVorbisComment final : public VorbisComment {
    friend class OggContainer;

public:
    OggVorbisComment();

    static constexpr TagType tagType = TagType::OggVorbisComment;
    static constexpr std::string_view tagName = "OGG Vorbis comment";
    TagType type() const override;
    std::string_view typeName() const override;
    bool supportsTarget() const override;

    OggParameter &oggParams();
    const OggParameter &oggParams() const;

private:
    OggParameter m_oggParams;
};

/*!
 * \brief Constructs a new OGG Vorbis comment.
 */
inline OggVorbisComment::OggVorbisComment()
{
}

inline TagType OggVorbisComment::type() const
{
    return TagType::OggVorbisComment;
}

/*!
 * \brief Returns true; the target is used to specify the stream.
 * \remarks At this point, one cannot move a tag from one stream to another by changing the target. So
 *          the target is only evaluated when invoking createTag() and added to parsed tags for informative
 *          purposes.
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

class TAG_PARSER_EXPORT OggContainer final : public GenericContainer<MediaFileInfo, OggVorbisComment, OggStream, OggPage> {
    friend class OggStream;

public:
    OggContainer(MediaFileInfo &fileInfo, std::uint64_t startOffset);
    ~OggContainer() override;

    bool isChecksumValidationEnabled() const;
    void setChecksumValidationEnabled(bool enabled);
    void reset() override;

    OggVorbisComment *createTag(const TagTarget &target) override;
    OggVorbisComment *tag(std::size_t index) override;
    std::size_t tagCount() const override;
    bool removeTag(Tag *tag) override;
    void removeAllTags() override;

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;
    void internalParseTags(Diagnostics &diag, AbortableProgressFeedback &progress) override;
    void internalParseTracks(Diagnostics &diag, AbortableProgressFeedback &progress) override;
    void internalMakeFile(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    void announceComment(
        std::size_t pageIndex, std::size_t segmentIndex, bool lastMetaDataBlock, GeneralMediaFormat mediaFormat = GeneralMediaFormat::Vorbis);
    void makeVorbisCommentSegment(std::stringstream &buffer, CppUtilities::CopyHelper<65307> &copyHelper, std::vector<std::uint32_t> &newSegmentSizes,
        VorbisComment *comment, OggParameter *params, Diagnostics &diag);

    std::unordered_map<std::uint32_t, std::vector<std::unique_ptr<OggStream>>::size_type> m_streamsBySerialNo;

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

} // namespace TagParser

#endif // TAG_PARSER_OGGCONTAINER_H
