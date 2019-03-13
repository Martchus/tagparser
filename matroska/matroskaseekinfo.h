#ifndef TAG_PARSER_MATROSKASEEKINFO_H
#define TAG_PARSER_MATROSKASEEKINFO_H

#include "./ebmlelement.h"

#include <utility>

namespace TagParser {

class TAG_PARSER_EXPORT MatroskaSeekInfo {
public:
    MatroskaSeekInfo();

    EbmlElement *seekHeadElement() const;
    const std::vector<std::pair<EbmlElement::IdentifierType, std::uint64_t>> &info() const;
    std::vector<std::pair<EbmlElement::IdentifierType, std::uint64_t>> &info();
    void shift(std::uint64_t start, std::int64_t amount);
    void parse(EbmlElement *seekHeadElement, Diagnostics &diag);
    void make(std::ostream &stream, Diagnostics &diag);
    std::uint64_t minSize() const;
    std::uint64_t maxSize() const;
    std::uint64_t actualSize() const;
    bool push(unsigned int index, EbmlElement::IdentifierType id, std::uint64_t offset);
    void clear();

    // these methods seem to be not needed anymore
    static std::pair<EbmlElement::IdentifierType, std::uint64_t> *findSeekInfo(std::vector<MatroskaSeekInfo> &seekInfos, std::uint64_t offset);
    static bool updateSeekInfo(const std::vector<MatroskaSeekInfo> &oldSeekInfos, std::vector<MatroskaSeekInfo> &newSeekInfos,
        std::uint64_t oldOffset, std::uint64_t newOffset);
    static bool updateSeekInfo(std::vector<MatroskaSeekInfo> &newSeekInfos, std::uint64_t oldOffset, std::uint64_t newOffset);

private:
    EbmlElement *m_seekHeadElement;
    std::vector<std::pair<EbmlElement::IdentifierType, std::uint64_t>> m_info;
};

/*!
 * \brief Constructs a new MatroskaSeekInfo.
 */
inline MatroskaSeekInfo::MatroskaSeekInfo()
    : m_seekHeadElement(nullptr)
{
}

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
inline const std::vector<std::pair<EbmlElement::IdentifierType, std::uint64_t>> &MatroskaSeekInfo::info() const
{
    return m_info;
}

/*!
 * \brief Returns a mutable version of the seek information gathered when the parse() method was called.
 * \returns Returns the seek information as pairs of element IDs and the associated offsets (relative to the beginning of the file).
 */
inline std::vector<std::pair<EbmlElement::IdentifierType, std::uint64_t>> &MatroskaSeekInfo::info()
{
    return m_info;
}

} // namespace TagParser

#endif // TAG_PARSER_MATROSKASEEKINFO_H
