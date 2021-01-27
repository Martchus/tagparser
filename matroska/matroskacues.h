#ifndef TAG_PARSER_MATROSKACUES_H
#define TAG_PARSER_MATROSKACUES_H

#include "./ebmlelement.h"

#include <ostream>
#include <unordered_map>

namespace TagParser {

class TAG_PARSER_EXPORT MatroskaOffsetStates {
public:
    constexpr MatroskaOffsetStates(std::uint64_t initialValue);
    constexpr std::uint64_t currentValue() const;
    void update(std::uint64_t newValue);
    constexpr std::uint64_t initialValue() const;

private:
    std::uint64_t m_initialValue;
    std::uint64_t m_currentValue;
};

constexpr MatroskaOffsetStates::MatroskaOffsetStates(std::uint64_t initialValue)
    : m_initialValue(initialValue)
    , m_currentValue(initialValue)
{
}

constexpr std::uint64_t MatroskaOffsetStates::currentValue() const
{
    return m_currentValue;
}

inline void MatroskaOffsetStates::update(std::uint64_t newValue)
{
    m_currentValue = newValue;
}

constexpr std::uint64_t MatroskaOffsetStates::initialValue() const
{
    return m_initialValue;
}

class TAG_PARSER_EXPORT MatroskaReferenceOffsetPair : public MatroskaOffsetStates {
public:
    constexpr MatroskaReferenceOffsetPair(std::uint64_t referenceOffset, std::uint64_t initialValue);
    constexpr std::uint64_t referenceOffset() const;

private:
    std::uint64_t m_referenceOffset;
};

constexpr MatroskaReferenceOffsetPair::MatroskaReferenceOffsetPair(std::uint64_t referenceOffset, std::uint64_t initialValue)
    : MatroskaOffsetStates(initialValue)
    , m_referenceOffset(referenceOffset)
{
}

constexpr std::uint64_t MatroskaReferenceOffsetPair::referenceOffset() const
{
    return m_referenceOffset;
}

class TAG_PARSER_EXPORT MatroskaCuePositionUpdater {
public:
    MatroskaCuePositionUpdater();

    EbmlElement *cuesElement() const;
    std::uint64_t totalSize() const;

    void parse(EbmlElement *cuesElement, Diagnostics &diag);
    bool updateOffsets(std::uint64_t originalOffset, std::uint64_t newOffset);
    bool updateRelativeOffsets(std::uint64_t referenceOffset, std::uint64_t originalRelativeOffset, std::uint64_t newRelativeOffset);
    void make(std::ostream &stream, Diagnostics &diag);
    void clear();

private:
    struct PairHash {
        template <class T1, class T2> inline std::size_t operator()(const std::pair<T1, T2> &pair) const
        {
            std::size_t seed = 0;
            seed ^= std::hash<T1>()(pair.first) + 0x9e3779b9;
            seed ^= std::hash<T2>()(pair.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    bool updateSize(EbmlElement *element, int shift);

    EbmlElement *m_cuesElement;
    std::unordered_map<EbmlElement *, MatroskaOffsetStates> m_offsets;
    std::unordered_multimap<std::uint64_t, EbmlElement *> m_cueElementByOriginalOffset;
    std::unordered_map<EbmlElement *, MatroskaReferenceOffsetPair> m_relativeOffsets;
    std::unordered_multimap<std::pair<std::uint64_t, std::uint64_t>, EbmlElement *, PairHash> m_cueRelativePositionElementByOriginalOffsets;
    std::unordered_map<EbmlElement *, std::uint64_t> m_sizes;
};

/*!
 * \brief Creates a new MatroskaCuePositionUpdater.
 *
 * The parse() method should be called to do further initialization.
 */
inline MatroskaCuePositionUpdater::MatroskaCuePositionUpdater()
    : m_cuesElement(nullptr)
{
}

/*!
 * \brief Returns the "Cues"-element specified when calling the parse() method.
 *
 * Returns nullptr if no "Cues"-element is set.
 */
inline EbmlElement *MatroskaCuePositionUpdater::cuesElement() const
{
    return m_cuesElement;
}

/*!
 * \brief Resets the object to its initial state. Parsing results and updates are cleared.
 */
inline void MatroskaCuePositionUpdater::clear()
{
    m_cuesElement = nullptr;
    m_offsets.clear();
    m_sizes.clear();
}

} // namespace TagParser

#endif // TAG_PARSER_MATROSKACUES_H
