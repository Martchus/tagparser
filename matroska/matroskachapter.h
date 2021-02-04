#ifndef TAG_PARSER_MATROSKACHAPTER_H
#define TAG_PARSER_MATROSKACHAPTER_H

#include "../abstractchapter.h"

#include <memory>

namespace TagParser {

class EbmlElement;

class TAG_PARSER_EXPORT MatroskaChapter : public AbstractChapter {
public:
    MatroskaChapter(EbmlElement *chapterAtomElement);
    ~MatroskaChapter() override;

    MatroskaChapter *nestedChapter(std::size_t index) override;
    const MatroskaChapter *nestedChapter(std::size_t index) const override;
    std::size_t nestedChapterCount() const override;
    void clear() override;

protected:
    void internalParse(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    EbmlElement *m_chapterAtomElement;
    std::vector<std::unique_ptr<MatroskaChapter>> m_nestedChapters;
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

} // namespace TagParser

#endif // TAG_PARSER_MATROSKACHAPTER_H
