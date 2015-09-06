#ifndef MP4TAG_H
#define MP4TAG_H

#include "mp4tagfield.h"

#include "tagparser/fieldbasedtag.h"

namespace Media
{

class Mp4Atom;

class LIB_EXPORT Mp4Tag : public FieldMapBasedTag<Mp4TagField>
{
public:
    Mp4Tag();
    ~Mp4Tag();

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
    void make(std::ostream &stream);
};

/*!
 * \brief Constructs a new tag.
 */
inline Mp4Tag::Mp4Tag()
{}

/*!
 * \brief Destroys the tag.
 */
inline Mp4Tag::~Mp4Tag()
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
