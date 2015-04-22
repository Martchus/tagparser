#ifndef MEDIA_VORBISCOMMENT_H
#define MEDIA_VORBISCOMMENT_H

#include "vorbiscommentfield.h"

#include "../fieldbasedtag.h"

namespace Media {

class OggIterator;

class LIB_EXPORT VorbisComment : public FieldMapBasedTag<VorbisCommentField>
{
public:
    VorbisComment();
    ~VorbisComment();

    TagType type() const;
    const char *typeName() const;
    TagTextEncoding proposedTextEncoding() const;
    bool canEncodingBeUsed(TagTextEncoding encoding) const;

    std::string fieldId(KnownField field) const;
    KnownField knownField(const std::string &id) const;

    void parse(OggIterator &iterator);
    void make(std::ostream &stream);

    const std::string &vendor() const;
    void setVendor(const std::string &vendor);

private:
    std::string m_vendor;

};

/*!
 * \brief Constructs a new Vorbis comment.
 */
inline VorbisComment::VorbisComment()
{}

/*!
 * \brief Destroys the Vorbis comment.
 */
inline VorbisComment::~VorbisComment()
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

inline const std::string &VorbisComment::vendor() const
{
    return m_vendor;
}

inline void VorbisComment::setVendor(const std::string &vendor)
{
    m_vendor = vendor;
}

}

#endif // MEDIA_VORBISCOMMENT_H
