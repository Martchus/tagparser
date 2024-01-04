#ifndef TAG_PARSER_VORBISCOMMENT_H
#define TAG_PARSER_VORBISCOMMENT_H

#include "./vorbiscommentfield.h"

#include "../caseinsensitivecomparer.h"
#include "../fieldbasedtag.h"
#include "../mediaformat.h"

class OverallTests;

namespace TagParser {

class OggIterator;
class VorbisComment;
class Diagnostics;

/*!
 * \brief Defines traits for the TagField implementation of the VorbisComment class.
 */
template <> class TAG_PARSER_EXPORT FieldMapBasedTagTraits<VorbisComment> {
public:
    using FieldType = VorbisCommentField;
    using Compare = CaseInsensitiveStringComparer;
};

class TAG_PARSER_EXPORT VorbisComment : public FieldMapBasedTag<VorbisComment> {
    friend class FieldMapBasedTag<VorbisComment>;
    friend class ::OverallTests;

public:
    VorbisComment();

    static constexpr TagType tagType = TagType::VorbisComment;
    static constexpr std::string_view tagName = "Vorbis comment";
    static constexpr TagTextEncoding defaultTextEncoding = TagTextEncoding::Utf8;
    bool canEncodingBeUsed(TagTextEncoding encoding) const override;

    using FieldMapBasedTag<VorbisComment>::value;
    const TagValue &value(KnownField field) const override;
    using FieldMapBasedTag<VorbisComment>::setValue;
    bool setValue(KnownField field, const TagValue &value) override;

    void parse(OggIterator &iterator, VorbisCommentFlags flags, Diagnostics &diag);
    void parse(std::istream &stream, std::uint64_t maxSize, VorbisCommentFlags flags, Diagnostics &diag);
    void make(std::ostream &stream, VorbisCommentFlags flags, Diagnostics &diag);

    const TagValue &vendor() const;
    void setVendor(const TagValue &vendor);
    bool supportsMultipleValues(KnownField) const override;

protected:
    IdentifierType internallyGetFieldId(KnownField field) const;
    KnownField internallyGetKnownField(const IdentifierType &id) const;

private:
    template <class StreamType> void internalParse(StreamType &stream, std::uint64_t maxSize, VorbisCommentFlags flags, Diagnostics &diag);
    void extendPositionInSetField(std::string_view field, std::string_view totalField, const std::string &diagContext, Diagnostics &diag);
    void convertTotalFields(const std::string &diagContext, Diagnostics &diag);

private:
    TagValue m_vendor;
};

/*!
 * \brief Constructs a new Vorbis comment.
 */
inline VorbisComment::VorbisComment()
{
}

inline bool VorbisComment::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == TagTextEncoding::Utf8;
}

/*!
 * \brief Returns the vendor.
 * \remarks Also accessible via value(KnownField::Vendor).
 */
inline const TagValue &VorbisComment::vendor() const
{
    return m_vendor;
}

/*!
 * \brief Sets the vendor.
 * \remarks Also accessible via setValue(KnownField::Vendor, vendor).
 */
inline void VorbisComment::setVendor(const TagValue &vendor)
{
    m_vendor = vendor;
}

/*!
 * \brief Allows multiple values for all fields.
 * \remarks "Field names are not required to be unique (occur once) within a comment header."
 */
inline bool VorbisComment::supportsMultipleValues(KnownField) const
{
    return true;
}

} // namespace TagParser

#endif // TAG_PARSER_VORBISCOMMENT_H
