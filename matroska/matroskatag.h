#ifndef MEDIA_MATROSKATAG_H
#define MEDIA_MATROSKATAG_H

#include "./matroskatagfield.h"
#include "./matroskatagid.h"

#include "../fieldbasedtag.h"

namespace Media {

class EbmlElement;
class MatroskaTag;

class TAG_PARSER_EXPORT MatroskaTagMaker
{
    friend class MatroskaTag;

public:
    void make(std::ostream &stream) const;
    const MatroskaTag &tag() const;
    uint64 requiredSize() const;

private:
    MatroskaTagMaker(MatroskaTag &tag);

    MatroskaTag &m_tag;
    uint64 m_targetsSize;
    uint64 m_simpleTagsSize;
    std::vector<MatroskaTagFieldMaker> m_maker;
    uint64 m_tagSize;
    uint64 m_totalSize;
};

/*!
 * \brief Returns the associated tag.
 */
inline const MatroskaTag &MatroskaTagMaker::tag() const
{
    return m_tag;
}

/*!
 * \brief Returns the number of bytes which will be written when making the tag.
 */
inline uint64 MatroskaTagMaker::requiredSize() const
{
    return m_totalSize;
}

class TAG_PARSER_EXPORT MatroskaTag : public FieldMapBasedTag<MatroskaTagField>
{
public:
    MatroskaTag();

    static constexpr TagType tagType = TagType::MatroskaTag;
    // FIXME: implement type() and typeName() in FieldMapBasedTag
    TagType type() const;
    const char *typeName() const;
    TagTextEncoding proposedTextEncoding() const;
    bool canEncodingBeUsed(TagTextEncoding encoding) const;
    bool supportsTarget() const;
    TagTargetLevel targetLevel() const;

    std::string fieldId(KnownField field) const;
    KnownField knownField(const std::string &id) const;

    void parse(EbmlElement &tagElement);
    MatroskaTagMaker prepareMaking();
    void make(std::ostream &stream);

private:
    void parseTargets(EbmlElement &targetsElement);
};

/*!
 * \brief Constructs a new tag.
 */
inline MatroskaTag::MatroskaTag()
{}

inline bool MatroskaTag::supportsTarget() const
{
    return true;
}

inline TagTargetLevel MatroskaTag::targetLevel() const
{
    return matroskaTagTargetLevel(m_target.level());
}

inline TagType MatroskaTag::type() const
{
    return TagType::MatroskaTag;
}

inline const char *MatroskaTag::typeName() const
{
    return "Matroska tag";
}

inline TagTextEncoding MatroskaTag::proposedTextEncoding() const
{
    return TagTextEncoding::Utf8;
}

inline bool MatroskaTag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == TagTextEncoding::Utf8;
}

}

#endif // MEDIA_MATROSKATAG_H
