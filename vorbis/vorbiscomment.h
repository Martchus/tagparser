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
 * \brief The VorbisCommentFlags enum specifies flags which controls parsing and making of Vorbis comments.
 */
enum class VorbisCommentFlags : byte
{
    None = 0x0, /**< Regular parsing/making. */
    NoSignature = 0x1, /**< Skips the signature when parsing; does not make the signature when making. */
    NoFramingByte = 0x2 /**< Doesn't expect the framing bit to be present when parsing; does not make the framing bit when making. */
};

inline bool operator &(VorbisCommentFlags lhs, VorbisCommentFlags rhs)
{
    return static_cast<byte>(lhs) & static_cast<byte>(rhs);
}

inline VorbisCommentFlags operator |(VorbisCommentFlags lhs, VorbisCommentFlags rhs)
{
    return static_cast<VorbisCommentFlags>(static_cast<byte>(lhs) | static_cast<byte>(rhs));
}

class LIB_EXPORT VorbisComment : public FieldMapBasedTag<VorbisCommentField, CaseInsensitiveStringComparer>
{
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

    void parse(OggIterator &iterator, VorbisCommentFlags flags = VorbisCommentFlags::None, std::size_t offset = 0);
    void make(std::ostream &stream, VorbisCommentFlags flags = VorbisCommentFlags::None);

    const TagValue &vendor() const;
    void setVendor(const TagValue &vendor);

private:
    TagValue m_vendor;
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
    return "Vorbis comment";
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
 * \brief Sets the vendor.
 * \remarks Also accessable via setValue(KnownField::Vendor, vendor).
 */
inline void VorbisComment::setVendor(const TagValue &vendor)
{
    m_vendor = vendor;
}

}

#endif // MEDIA_VORBISCOMMENT_H
