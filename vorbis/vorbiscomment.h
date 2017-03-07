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
 * \brief Defines traits for the TagField implementation of the VorbisComment class.
 */
template <>
class TAG_PARSER_EXPORT FieldMapBasedTagTraits<VorbisComment>
{
public:
    typedef VorbisComment implementationType;
    typedef VorbisCommentField fieldType;
    typedef CaseInsensitiveStringComparer compare;
};

class TAG_PARSER_EXPORT VorbisComment : public FieldMapBasedTag<VorbisComment>
{
    friend class FieldMapBasedTag<VorbisComment>;

public:
    VorbisComment();

    static constexpr TagType tagType = TagType::VorbisComment;
    static constexpr const char *tagName = "Vorbis comment";
    static constexpr TagTextEncoding defaultTextEncoding = TagTextEncoding::Utf8;
    bool canEncodingBeUsed(TagTextEncoding encoding) const;

    using FieldMapBasedTag<VorbisComment>::value;
    const TagValue &value(KnownField field) const;
    using FieldMapBasedTag<VorbisComment>::setValue;
    bool setValue(KnownField field, const TagValue &value);

    void parse(OggIterator &iterator, VorbisCommentFlags flags = VorbisCommentFlags::None);
    void parse(std::istream &stream, uint64 maxSize, VorbisCommentFlags flags = VorbisCommentFlags::None);
    void make(std::ostream &stream, VorbisCommentFlags flags = VorbisCommentFlags::None);

    const TagValue &vendor() const;
    void setVendor(const TagValue &vendor);

protected:
    identifierType internallyGetFieldId(KnownField field) const;
    KnownField internallyGetKnownField(const identifierType &id) const;

private:
    template<class StreamType>
    void internalParse(StreamType &stream, uint64 maxSize, VorbisCommentFlags flags);

private:
    TagValue m_vendor;
};

/*!
 * \brief Constructs a new Vorbis comment.
 */
inline VorbisComment::VorbisComment()
{}

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
 * \brief Sets the vendor.
 * \remarks Also accessable via setValue(KnownField::Vendor, vendor).
 */
inline void VorbisComment::setVendor(const TagValue &vendor)
{
    m_vendor = vendor;
}

}

#endif // MEDIA_VORBISCOMMENT_H
