#ifndef MEDIA_MATROSKATAG_H
#define MEDIA_MATROSKATAG_H

#include "matroskatagfield.h"
#include "../fieldbasedtag.h"

namespace Media {

class EbmlElement;
class MatroskaTag;

class LIB_EXPORT MatroskaTagMaker
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
    std::vector<MatroskaTagFieldMaker> m_makers;
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

class LIB_EXPORT MatroskaTag : public FieldMapBasedTag<MatroskaTagField>
{
public:
    MatroskaTag();
    ~MatroskaTag();

    TagType type() const;
    const char *typeName() const;
    TagTextEncoding proposedTextEncoding() const;
    bool canEncodingBeUsed(TagTextEncoding encoding) const;
    bool supportsTarget() const;
//    bool supportsNestedTags() const;

    std::string fieldId(KnownField field) const;
    KnownField knownField(const std::string &id) const;

    void parse(EbmlElement &tagElement);
    MatroskaTagMaker prepareMaking();
    void make(std::ostream &stream);

private:
    void parseTargets(EbmlElement &targetsElement);
};

inline bool MatroskaTag::supportsTarget() const
{
    return true;
}

//inline bool MatroskaTag::supportsNestedTags() const
//{
//    return true;
//}

/*!
 * \brief Constructs a new tag.
 */
inline MatroskaTag::MatroskaTag()
{}

/*!
 * \brief Destroys the tag.
 */
inline MatroskaTag::~MatroskaTag()
{}

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
