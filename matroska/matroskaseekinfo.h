#ifndef MEDIA_MATROSKASEEKINFO_H
#define MEDIA_MATROSKASEEKINFO_H

#include "./ebmlelement.h"

#include "../statusprovider.h"

#include <utility>

namespace Media {

class TAG_PARSER_EXPORT MatroskaSeekInfo : public StatusProvider
{
public:
    MatroskaSeekInfo();

    EbmlElement *seekHeadElement() const;
    const std::vector<std::pair<EbmlElement::IdentifierType, uint64> > &info() const;
    std::vector<std::pair<EbmlElement::IdentifierType, uint64> > &info();
    void shift(uint64 start, int64 amount);
    void parse(EbmlElement *seekHeadElement);
    void make(std::ostream &stream);
    uint64 minSize() const;
    uint64 maxSize() const;
    uint64 actualSize() const;
    bool push(unsigned int index, EbmlElement::IdentifierType id, uint64 offset);
    void clear();

    // these methods seem to be not needed anymore
    static std::pair<EbmlElement::IdentifierType, uint64> *findSeekInfo(std::vector<MatroskaSeekInfo> &seekInfos, uint64 offset);
    static bool updateSeekInfo(const std::vector<MatroskaSeekInfo> &oldSeekInfos, std::vector<MatroskaSeekInfo> &newSeekInfos, uint64 oldOffset, uint64 newOffset);
    static bool updateSeekInfo(std::vector<MatroskaSeekInfo> &newSeekInfos, uint64 oldOffset, uint64 newOffset);

private:
    EbmlElement *m_seekHeadElement;
    std::vector<std::pair<EbmlElement::IdentifierType, uint64> > m_info;
};

/*!
 * \brief Constructs a new MatroskaSeekInfo.
 */
inline MatroskaSeekInfo::MatroskaSeekInfo() :
    m_seekHeadElement(nullptr)
{}

/*!
 * \brief Returns a pointer to the \a seekHeadElement specified when the parse() method was called.
 */
inline EbmlElement *MatroskaSeekInfo::seekHeadElement() const
{
    return m_seekHeadElement;
}

/*!
 * \brief Returns the seek information gathered when the parse() method was called.
 * \returns Returns the seek information as pairs of element IDs and the associated offsets (relative to the beginning of the file).
 */
inline const std::vector<std::pair<EbmlElement::IdentifierType, uint64> > &MatroskaSeekInfo::info() const
{
    return m_info;
}

/*!
 * \brief Returns a mutable version of the seek information gathered when the parse() method was called.
 * \returns Returns the seek information as pairs of element IDs and the associated offsets (relative to the beginning of the file).
 */
inline std::vector<std::pair<EbmlElement::IdentifierType, uint64> > &MatroskaSeekInfo::info()
{
    return m_info;
}

}

#endif // MEDIA_MATROSKASEEKINFO_H
