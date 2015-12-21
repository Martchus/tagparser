#ifndef MP4TAG_H
#define MP4TAG_H

#include "./mp4tagfield.h"

#include "../fieldbasedtag.h"

namespace Media
{

class Mp4Atom;
class Mp4Tag;

class LIB_EXPORT Mp4TagMaker
{
    friend class Mp4Tag;

public:
    void make(std::ostream &stream);
    const Mp4Tag &tag() const;
    uint64 requiredSize() const;

private:
    Mp4TagMaker(Mp4Tag &tag);

    Mp4Tag &m_tag;
    std::vector<Mp4TagFieldMaker> m_makers;
    uint64 m_metaSize;
    uint64 m_ilstSize;
    bool m_omitPreDefinedGenre;
};

/*!
 * \brief Returns the associated tag.
 */
inline const Mp4Tag &Mp4TagMaker::tag() const
{
    return m_tag;
}

/*!
 * \brief Returns the number of bytes which will be written when making the tag.
 */
inline uint64 Mp4TagMaker::requiredSize() const
{
    return m_metaSize;
}

class LIB_EXPORT Mp4Tag : public FieldMapBasedTag<Mp4TagField>
{
public:
    Mp4Tag();

    TagType type() const;
    const char *typeName() const;
    TagTextEncoding proposedTextEncoding() const;
    bool canEncodingBeUsed(TagTextEncoding encoding) const;

    uint32 fieldId(KnownField value) const;
    KnownField knownField(const uint32 &id) const;
    using FieldMapBasedTag<Mp4TagField>::value;
    const TagValue &value(KnownField value) const;
    const TagValue &value(const std::string mean, const std::string name) const;
    using FieldMapBasedTag<Mp4TagField>::setValue;
    bool setValue(KnownField field, const TagValue &value);
    bool setValue(const std::string mean, const std::string name, const TagValue &value);
    using FieldMapBasedTag<Mp4TagField>::hasField;
    bool hasField(KnownField value) const;

    void parse(Mp4Atom &metaAtom);
    Mp4TagMaker prepareMaking();
    void make(std::ostream &stream);
};

/*!
 * \brief Constructs a new tag.
 */
inline Mp4Tag::Mp4Tag()
{}

inline TagType Mp4Tag::type() const
{
    return TagType::Mp4Tag;
}

inline const char *Mp4Tag::typeName() const
{
    return "MP4/iTunes tag";
}

inline TagTextEncoding Mp4Tag::proposedTextEncoding() const
{
    return TagTextEncoding::Utf8;
}

}

#endif // MP4TAG_H
