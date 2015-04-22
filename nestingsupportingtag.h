#ifndef NESTEDTAGSUPPORT
#define NESTEDTAGSUPPORT

#include "tag.h"

namespace Media {

template <class TagType>
class LIB_EXPORT NestingSupportingTag : public Tag
{
public:
    NestingSupportingTag();

    const std::vector<TagType *> &nestedTags() const;
    TagType *parent() const;
    bool setParent(Tag *tag);
    virtual TagType *nestedTag(size_t index) const;
    virtual size_t nestedTagCount() const;
    //const std::vector<Tag *> nestedTags() const;
    virtual bool supportsNestedTags() const;

private:
    std::vector<TagType *> m_nestedTags;
    TagType *m_parent;
};

inline const std::vector<TagType *> &NestingSupportingTag::nestedTags() const
{
    return m_nestedTags;
}

inline TagType *NestingSupportingTag::parent() const
{
    return m_parent;
}

bool NestingSupportingTag::setParent(Tag *tag)
{
    if(m_parent != tag) {
        if() {

        }
    }
}

inline TagType *NestingSupportingTag::nestedTag(size_t index) const
{
    return m_nestedTags.at(index);
}

inline size_t NestingSupportingTag::nestedTagCount() const
{
    return m_nestedTags.size();
}

inline bool NestingSupportingTag::supportsNestedTags() const
{
    return true;
}

}

#endif // NESTEDTAGSUPPORT

