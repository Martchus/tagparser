#ifndef MEDIA_MATROSKACHAPTER_H
#define MEDIA_MATROSKACHAPTER_H

#include "tagparser/abstractchapter.h"

#include <memory>

namespace Media {

class EbmlElement;

class LIB_EXPORT MatroskaChapter : public AbstractChapter
{
public:
    MatroskaChapter(EbmlElement *chapterAtomElement);
    ~MatroskaChapter();

    MatroskaChapter *nestedChapter(std::size_t index);
    const MatroskaChapter *nestedChapter(std::size_t index) const;
    std::size_t nestedChapterCount() const;
    void clear();

protected:
    void internalParse();

private:
    EbmlElement *m_chapterAtomElement;
    std::vector<std::unique_ptr<MatroskaChapter> > m_nestedChapters;

};

inline MatroskaChapter *MatroskaChapter::nestedChapter(std::size_t index)
{
    return m_nestedChapters[index].get();
}

inline const MatroskaChapter *MatroskaChapter::nestedChapter(std::size_t index) const
{
    return m_nestedChapters[index].get();
}

inline std::size_t MatroskaChapter::nestedChapterCount() const
{
    return m_nestedChapters.size();
}


} // namespace Media

#endif // MEDIA_MATROSKACHAPTER_H
