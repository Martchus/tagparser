#ifndef TAG_PARSER_MATROSKASEEKINFO_H
#define TAG_PARSER_MATROSKASEEKINFO_H

#include "./ebmlelement.h"

#include <memory>
#include <utility>
#include <vector>

namespace TagParser {

class TAG_PARSER_EXPORT MatroskaSeekInfo {
public:
    MatroskaSeekInfo();

    const std::vector<EbmlElement *> &seekHeadElements() const;
    const std::vector<std::pair<EbmlElement::IdentifierType, std::uint64_t>> &info() const;
    std::vector<std::pair<EbmlElement::IdentifierType, std::uint64_t>> &info();
    void shift(std::uint64_t start, std::int64_t amount);
    void parse(EbmlElement *seekHeadElements, Diagnostics &diag, std::size_t maxIndirection = 1);
    void make(std::ostream &stream, Diagnostics &diag);
    std::uint64_t minSize() const;
    std::uint64_t maxSize() const;
    std::uint64_t actualSize() const;
    bool push(unsigned int index, EbmlElement::IdentifierType id, std::uint64_t offset);
    void clear();

private:
    std::vector<EbmlElement *> m_seekHeadElements;
    std::vector<std::unique_ptr<EbmlElement>> m_additionalSeekHeadElements;
    std::vector<std::pair<EbmlElement::IdentifierType, std::uint64_t>> m_info;
};

/*!
 * \brief Constructs a new MatroskaSeekInfo.
 */
inline MatroskaSeekInfo::MatroskaSeekInfo()
{
}

/*!
 * \brief Returns a pointer to the seek head elements the seek information is composed of.
 * \remarks This list is initially empty. When calling parse() it is at least populated with the specified seek head element (ownership remains
 * by the caller). In case that seek table references another seek table those elements are also returned (the MatroskaSeekInfo has ownership).
 */
inline const std::vector<EbmlElement *> &MatroskaSeekInfo::seekHeadElements() const
{
    return m_seekHeadElements;
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
