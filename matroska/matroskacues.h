#ifndef MEDIA_MATROSKACUES_H
#define MEDIA_MATROSKACUES_H

#include "./ebmlelement.h"

#include <unordered_map>
#include <ostream>

namespace Media {

class TAG_PARSER_EXPORT MatroskaOffsetStates
{
public:
    MatroskaOffsetStates(uint64 initialValue);
    uint64 currentValue() const;
    void update(uint64 newValue);
    uint64 initialValue() const;
private:
    uint64 m_initialValue;
    uint64 m_currentValue;
};

inline MatroskaOffsetStates::MatroskaOffsetStates(uint64 initialValue) :
    m_initialValue(initialValue),
    m_currentValue(initialValue)
{}

inline uint64 MatroskaOffsetStates::currentValue() const
{
    return m_currentValue;
}

inline void MatroskaOffsetStates::update(uint64 newValue)
{
    m_currentValue = newValue;
}

inline uint64 MatroskaOffsetStates::initialValue() const
{
    return m_initialValue;
}

class TAG_PARSER_EXPORT MatroskaReferenceOffsetPair : public MatroskaOffsetStates
{
public:
    MatroskaReferenceOffsetPair(uint64 referenceOffset, uint64 initialValue);
    uint64 referenceOffset() const;
private:
    uint64 m_referenceOffset;
};

inline MatroskaReferenceOffsetPair::MatroskaReferenceOffsetPair(uint64 referenceOffset, uint64 initialValue) :
    MatroskaOffsetStates(initialValue),
    m_referenceOffset(referenceOffset)
{}

inline uint64 MatroskaReferenceOffsetPair::referenceOffset() const
{
    return m_referenceOffset;
}

class TAG_PARSER_EXPORT MatroskaCuePositionUpdater : public StatusProvider
{
public:
    MatroskaCuePositionUpdater();

    EbmlElement *cuesElement() const;
    uint64 totalSize() const;

    void parse(EbmlElement *cuesElement);
    bool updateOffsets(uint64 originalOffset, uint64 newOffset);
    bool updateRelativeOffsets(uint64 referenceOffset, uint64 originalRelativeOffset, uint64 newRelativeOffset);
    void make(std::ostream &stream);
    void clear();

private:
    bool updateSize(EbmlElement *element, int shift);

    EbmlElement *m_cuesElement;
    std::unordered_map<EbmlElement *, MatroskaOffsetStates> m_offsets;
    std::unordered_map<EbmlElement *, MatroskaReferenceOffsetPair> m_relativeOffsets;
    std::unordered_map<EbmlElement *, uint64> m_sizes;
};

/*!
 * \brief Creates a new MatroskaCuePositionUpdater.
 *
 * The parse() method should be called to do further initialization.
 */
inline MatroskaCuePositionUpdater::MatroskaCuePositionUpdater() :
    m_cuesElement(nullptr)
{}

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

} // namespace Media

#endif // MEDIA_MATROSKACUES_H
