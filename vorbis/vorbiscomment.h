#ifndef MEDIA_VORBISCOMMENT_H
#define MEDIA_VORBISCOMMENT_H

#include "./vorbiscommentfield.h"

#include "../caseinsensitivecomparer.h"
#include "../fieldbasedtag.h"
#include "../mediaformat.h"

namespace Media {

class OggIterator;
class VorbisComment;

/*!
 * \brief The OggParameter struct holds the OGG parameter for a VorbisComment.
 */
struct LIB_EXPORT OggParameter
{
    OggParameter();
    void set(std::size_t pageIndex, std::size_t segmentIndex, GeneralMediaFormat streamFormat = GeneralMediaFormat::Vorbis);

    std::size_t firstPageIndex;
    std::size_t firstSegmentIndex;
    std::size_t lastPageIndex;
    std::size_t lastSegmentIndex;
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
    streamFormat(GeneralMediaFormat::Vorbis), // default to Vorbis here
    removed(false)
{}

/*!
 * \brief Sets firstPageIndex/lastPageIndex, firstSegmentIndex/lastSegmentIndex and streamFormat.
 */
inline void OggParameter::set(std::size_t pageIndex, std::size_t segmentIndex, GeneralMediaFormat streamFormat)
{
    firstPageIndex = lastPageIndex = pageIndex;
    firstSegmentIndex = lastSegmentIndex = segmentIndex;
    this->streamFormat = streamFormat;
}

class LIB_EXPORT VorbisComment : public FieldMapBasedTag<VorbisCommentField, CaseInsensitiveStringComparer>
{
    friend class OggContainer;

public:
    VorbisComment();

    TagType type() const;
    const char *typeName() const;
    TagTextEncoding proposedTextEncoding() const;
    bool canEncodingBeUsed(TagTextEncoding encoding) const;

    const TagValue &value(KnownField field) const;
    bool setValue(KnownField field, const TagValue &value);
    std::string fieldId(KnownField field) const;
    KnownField knownField(const std::string &id) const;

    void parse(OggIterator &iterator, bool skipSignature = false);
    void make(std::ostream &stream, bool noSignature = false);

    const TagValue &vendor() const;
    void setVendor(const TagValue &vendor);
    OggParameter &oggParams();
    const OggParameter &oggParams() const;

private:
    TagValue m_vendor;
    OggParameter m_oggParams;
};

/*!
 * \brief Constructs a new Vorbis comment.
 */
inline VorbisComment::VorbisComment()
{}

inline TagType VorbisComment::type() const
{
    return TagType::VorbisComment;
}

inline const char *VorbisComment::typeName() const
{
    switch(m_oggParams.streamFormat) {
    case GeneralMediaFormat::Opus:
        return "Opus comment";
    case GeneralMediaFormat::Theora:
        return "Theora comment";
    default:
        // just assume Vorbis otherwise
        return "Vorbis comment";
    }
}

inline TagTextEncoding VorbisComment::proposedTextEncoding() const
{
    return TagTextEncoding::Utf8;
}

inline bool VorbisComment::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == TagTextEncoding::Utf8;
}

/*!
 * \brief Returns the vendor.
 * \remarks Also accessable via value(KnownField::Vendor).
 */
inline const TagValue &VorbisComment::vendor() const
{
    return m_vendor;
}

/*!
 * \brief Returns the vendor.
 * \remarks Also accessable via setValue(KnownField::Vendor, vendor).
 */
inline void VorbisComment::setVendor(const TagValue &vendor)
{
    m_vendor = vendor;
}

/*!
 * \brief Returns the OGG parameter for the comment.
 *
 * Consists of first page index, first segment index, last page index, last segment index and tag index (in this order).
 * These values are used and managed by the OggContainer class and do not affect the behavior of the VorbisComment instance.
 */
inline OggParameter &VorbisComment::oggParams()
{
    return m_oggParams;
}

/*!
 * \brief Returns the OGG parameter for the comment.
 *
 * Consists of first page index, first segment index, last page index, last segment index and tag index (in this order).
 * These values are used and managed by the OggContainer class and do not affect the behavior of the VorbisComment instance.
 */
inline const OggParameter &VorbisComment::oggParams() const
{
    return m_oggParams;
}

}

#endif // MEDIA_VORBISCOMMENT_H
