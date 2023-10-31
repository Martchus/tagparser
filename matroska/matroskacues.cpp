#include "./matroskacues.h"
#include "./matroskacontainer.h"

#include "../mediafileinfo.h"

#include <c++utilities/conversion/binaryconversion.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class MatroskaOffsetStates
 * \brief The MatroskaOffsetStates holds an offset within a Matroska file.
 *
 * The purpose of this class is to preserve the previous value when an offset
 * is updated.
 */

/*!
 * \class MatroskaReferenceOffsetPair
 * \brief The MatroskaReferenceOffsetPair holds an offset within a Matroska file plus the reference offset.
 *
 * The purpose of this class is to preserve the previous value when an offset
 * is updated.
 */

/*!
 * \class TagParser::MatroskaCuePositionUpdater
 * \brief The MatroskaCuePositionUpdater class helps to rewrite the "Cues"-element with shifted positions.
 *
 * This class is used when rewriting a Matroska file to save changed tag information.
 */

/*!
 * \brief Returns how many bytes will be written when calling the make() method.
 * \remarks The returned size might change when the object is altered (eg. by calling the updatePositions() method).
 */
std::uint64_t MatroskaCuePositionUpdater::totalSize() const
{
    if (m_cuesElement) {
        std::uint64_t size = m_sizes.at(m_cuesElement);
        return 4 + EbmlElement::calculateSizeDenotationLength(size) + size;
    } else {
        return 0;
    }
}

/*!
 * \brief Parses the specified \a cuesElement.
 * \remarks Previous parsing results and updates will be cleared.
 */
void MatroskaCuePositionUpdater::parse(EbmlElement *cuesElement, Diagnostics &diag)
{
    static const string context("parsing \"Cues\"-element");
    clear();
    std::uint64_t cuesElementSize = 0, cuePointElementSize, cueTrackPositionsElementSize, cueReferenceElementSize, pos, relPos, statePos;
    EbmlElement *cueRelativePositionElement, *cueClusterPositionElement;
    for (EbmlElement *cuePointElement = cuesElement->firstChild(); cuePointElement; cuePointElement = cuePointElement->nextSibling()) {
        // parse children of "Cues"-element which must be "CuePoint"-elements
        cuePointElement->parse(diag);
        switch (cuePointElement->id()) {
        case EbmlIds::Void:
        case EbmlIds::Crc32:
            break;
        case MatroskaIds::CuePoint:
            cuePointElementSize = 0;
            for (EbmlElement *cuePointChild = cuePointElement->firstChild(); cuePointChild; cuePointChild = cuePointChild->nextSibling()) {
                // parse children of "CuePoint"-element
                cuePointChild->parse(diag);
                switch (cuePointChild->id()) {
                case EbmlIds::Void:
                case EbmlIds::Crc32:
                    break;
                case MatroskaIds::CueTime:
                    cuePointChild->makeBuffer();
                    cuePointElementSize += cuePointChild->totalSize();
                    break;
                case MatroskaIds::CueTrackPositions:
                    cueTrackPositionsElementSize = relPos = 0;
                    cueRelativePositionElement = cueClusterPositionElement = nullptr;
                    for (EbmlElement *cueTrackPositionsChild = cuePointChild->firstChild(); cueTrackPositionsChild;
                         cueTrackPositionsChild = cueTrackPositionsChild->nextSibling()) {
                        // parse children of "CueTrackPositions"-element
                        cueTrackPositionsChild->parse(diag);
                        switch (cueTrackPositionsChild->id()) {
                        case MatroskaIds::CueTrack:
                        case MatroskaIds::CueDuration:
                        case MatroskaIds::CueBlockNumber:
                            cueTrackPositionsChild->makeBuffer();
                            cueTrackPositionsElementSize += cueTrackPositionsChild->totalSize();
                            break;
                        case MatroskaIds::CueRelativePosition:
                            relPos = (cueRelativePositionElement = cueTrackPositionsChild)->readUInteger();
                            break;
                        case MatroskaIds::CueClusterPosition:
                            pos = (cueClusterPositionElement = cueTrackPositionsChild)->readUInteger();
                            cueTrackPositionsElementSize += 2u + EbmlElement::calculateUIntegerLength(pos);
                            m_offsets.emplace(cueTrackPositionsChild, pos);
                            m_cueElementByOriginalOffset.emplace(pos, cueTrackPositionsChild);
                            break;
                        case MatroskaIds::CueCodecState:
                            statePos = cueTrackPositionsChild->readUInteger();
                            cueTrackPositionsElementSize += 2u + EbmlElement::calculateUIntegerLength(statePos);
                            m_offsets.emplace(cueTrackPositionsChild, statePos);
                            m_cueElementByOriginalOffset.emplace(statePos, cueTrackPositionsChild);
                            break;
                        case MatroskaIds::CueReference:
                            cueReferenceElementSize = 0;
                            for (EbmlElement *cueReferenceChild = cueTrackPositionsChild->firstChild(); cueReferenceChild;
                                 cueReferenceChild = cueReferenceChild->nextSibling()) {
                                // parse children of "CueReference"-element
                                cueReferenceChild->parse(diag);
                                switch (cueReferenceChild->id()) {
                                case EbmlIds::Void:
                                case EbmlIds::Crc32:
                                    break;
                                case MatroskaIds::CueRefTime:
                                case MatroskaIds::CueRefNumber:
                                    cueReferenceChild->makeBuffer();
                                    cueReferenceElementSize += cueReferenceChild->totalSize();
                                    break;
                                case MatroskaIds::CueRefCluster:
                                case MatroskaIds::CueRefCodecState:
                                    statePos = cueReferenceChild->readUInteger();
                                    cueReferenceElementSize += 2u + EbmlElement::calculateUIntegerLength(statePos);
                                    m_offsets.emplace(cueReferenceChild, statePos);
                                    m_cueElementByOriginalOffset.emplace(statePos, cueReferenceChild);
                                    break;
                                default:
                                    diag.emplace_back(DiagLevel::Warning,
                                        "\"CueReference\"-element contains a element which is not known to the parser. It will be ignored.", context);
                                }
                            }
                            cueTrackPositionsElementSize
                                += 1 + EbmlElement::calculateSizeDenotationLength(cueReferenceElementSize) + cueReferenceElementSize;
                            m_sizes.emplace(cueTrackPositionsChild, cueReferenceElementSize);
                            break;
                        default:
                            diag.emplace_back(DiagLevel::Warning,
                                "\"CueTrackPositions\"-element contains a element which is not known to the parser. It will be ignored.", context);
                        }
                    }
                    if (!cueClusterPositionElement) {
                        diag.emplace_back(
                            DiagLevel::Critical, "\"CueTrackPositions\"-element does not contain mandatory \"CueClusterPosition\"-element.", context);
                    } else if (cueRelativePositionElement) {
                        cueTrackPositionsElementSize += 2u + EbmlElement::calculateUIntegerLength(relPos);
                        m_relativeOffsets.emplace(piecewise_construct, forward_as_tuple(cueRelativePositionElement), forward_as_tuple(pos, relPos));
                        m_cueRelativePositionElementByOriginalOffsets.emplace(
                            piecewise_construct, forward_as_tuple(pos, relPos), forward_as_tuple(cueRelativePositionElement));
                    }
                    cuePointElementSize
                        += 1 + EbmlElement::calculateSizeDenotationLength(cueTrackPositionsElementSize) + cueTrackPositionsElementSize;
                    m_sizes.emplace(cuePointChild, cueTrackPositionsElementSize);
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning,
                        "\"CuePoint\"-element contains a element which is not a \"CueTime\"- or a \"CueTrackPositions\"-element. It will be ignored.",
                        context);
                }
            }
            cuesElementSize += 1 + EbmlElement::calculateSizeDenotationLength(cuePointElementSize) + cuePointElementSize;
            m_sizes.emplace(cuePointElement, cuePointElementSize);
            break;
        default:
            diag.emplace_back(
                DiagLevel::Warning, "\"Cues\"-element contains a element which is not a \"CuePoint\"-element. It will be ignored.", context);
        }
    }
    m_sizes.emplace(m_cuesElement = cuesElement, cuesElementSize);
}

/*!
 * \brief Sets the offset of the entries with the specified \a originalOffset to \a newOffset.
 * \returns Returns whether the size of the "Cues"-element has been altered.
 */
bool MatroskaCuePositionUpdater::updateOffsets(std::uint64_t originalOffset, std::uint64_t newOffset)
{
    auto updated = false;
    const auto newOffsetLength = static_cast<int>(EbmlElement::calculateUIntegerLength(newOffset));
    for (auto cueElementRange = m_cueElementByOriginalOffset.equal_range(originalOffset); cueElementRange.first != cueElementRange.second;
         ++cueElementRange.first) {
        auto *const cueElement = cueElementRange.first->second;
        const auto offsetIterator = m_offsets.find(cueElement);
        if (offsetIterator == m_offsets.end()) {
            continue;
        }
        auto &offset = offsetIterator->second;
        if (offset.currentValue() != newOffset) {
            updated
                = updateSize(cueElement->parent(), newOffsetLength - static_cast<int>(EbmlElement::calculateUIntegerLength(offset.currentValue())))
                || updated;
            offset.update(newOffset);
        }
    }
    return updated;
}

/*!
 * \brief Sets the relative offset of the entries with the specified \a originalRelativeOffset and the specified \a referenceOffset to \a newRelativeOffset.
 * \returns Returns whether the size of the "Cues"-element has been altered.
 */
bool MatroskaCuePositionUpdater::updateRelativeOffsets(
    std::uint64_t referenceOffset, std::uint64_t originalRelativeOffset, std::uint64_t newRelativeOffset)
{
    auto updated = false;
    const auto newRelativeOffsetLength = static_cast<int>(EbmlElement::calculateUIntegerLength(newRelativeOffset));
    for (auto cueElementRange = m_cueRelativePositionElementByOriginalOffsets.equal_range(std::make_pair(referenceOffset, originalRelativeOffset));
         cueElementRange.first != cueElementRange.second; ++cueElementRange.first) {
        auto *const cueRelativePositionElement = cueElementRange.first->second;
        const auto offsetIterator = m_relativeOffsets.find(cueRelativePositionElement);
        if (offsetIterator == m_relativeOffsets.end()) {
            continue;
        }
        auto &offset = offsetIterator->second;
        if (offset.currentValue() != newRelativeOffset) {
            updated = updateSize(cueRelativePositionElement->parent(),
                          newRelativeOffsetLength - static_cast<int>(EbmlElement::calculateUIntegerLength(offset.currentValue())))
                || updated;
            offset.update(newRelativeOffset);
        }
    }
    return updated;
}

/*!
 * \brief Updates the sizes for the specified \a element by adding the specified \a shift value.
 * \returns Returns whether the size of the "Cues"-element has been altered.
 */
bool MatroskaCuePositionUpdater::updateSize(EbmlElement *element, int shift)
{
    if (!shift) {
        return false; // shift is gone
    }
    if (!element) {
        // there was no parent (shouldn't happen in a normal file structure since the Segment element should
        // be parent of the Cues element)
        return shift;
    }
    // get size info
    const auto sizeIterator = m_sizes.find(element);
    if (sizeIterator == m_sizes.end()) {
        return shift; // the element is out of the scope of the cue position updater (likely the Segment element)
    }
    std::uint64_t &size = sizeIterator->second;
    // calculate new size
    const std::uint64_t newSize = shift > 0 ? size + static_cast<std::uint64_t>(shift) : size - static_cast<std::uint64_t>(-shift);
    // shift parent
    const bool updated = updateSize(element->parent(),
        shift + static_cast<int>(EbmlElement::calculateSizeDenotationLength(newSize))
            - static_cast<int>(EbmlElement::calculateSizeDenotationLength(size)));
    // apply new size
    size = newSize;
    return updated;
}

/*!
 * \brief Writes the previously parsed "Cues"-element with updated positions to the specified \a stream.
 */
void MatroskaCuePositionUpdater::make(ostream &stream, Diagnostics &diag)
{
    static const string context("making \"Cues\"-element");
    if (!m_cuesElement) {
        diag.emplace_back(DiagLevel::Warning, "No cues written; the cues of the source file could not be parsed correctly.", context);
        return;
    }
    // temporary variables
    char buff[8];
    std::uint8_t len;
    // write "Cues"-element
    try {
        BE::getBytes(static_cast<std::uint32_t>(MatroskaIds::Cues), buff);
        stream.write(buff, 4);
        len = EbmlElement::makeSizeDenotation(m_sizes[m_cuesElement], buff);
        stream.write(buff, len);
        // loop through original elements and write (a updated version) of them
        for (EbmlElement *cuePointElement = m_cuesElement->firstChild(); cuePointElement; cuePointElement = cuePointElement->nextSibling()) {
            cuePointElement->parse(diag);
            switch (cuePointElement->id()) {
            case EbmlIds::Void:
            case EbmlIds::Crc32:
                break;
            case MatroskaIds::CuePoint:
                // write "CuePoint"-element
                stream.put(static_cast<char>(MatroskaIds::CuePoint));
                len = EbmlElement::makeSizeDenotation(m_sizes[cuePointElement], buff);
                stream.write(buff, len);
                for (EbmlElement *cuePointChild = cuePointElement->firstChild(); cuePointChild; cuePointChild = cuePointChild->nextSibling()) {
                    cuePointChild->parse(diag);
                    switch (cuePointChild->id()) {
                    case EbmlIds::Void:
                    case EbmlIds::Crc32:
                        break;
                    case MatroskaIds::CueTime:
                        // write "CueTime"-element
                        cuePointChild->copyBuffer(stream);
                        cuePointChild->discardBuffer();
                        break;
                    case MatroskaIds::CueTrackPositions:
                        // write "CueTrackPositions"-element
                        stream.put(static_cast<char>(MatroskaIds::CueTrackPositions));
                        len = EbmlElement::makeSizeDenotation(m_sizes[cuePointChild], buff);
                        stream.write(buff, len);
                        for (EbmlElement *cueTrackPositionsChild = cuePointChild->firstChild(); cueTrackPositionsChild;
                             cueTrackPositionsChild = cueTrackPositionsChild->nextSibling()) {
                            cueTrackPositionsChild->parse(diag);
                            switch (cueTrackPositionsChild->id()) {
                            case MatroskaIds::CueTrack:
                            case MatroskaIds::CueDuration:
                            case MatroskaIds::CueBlockNumber:
                                // write unchanged children of "CueTrackPositions"-element
                                cueTrackPositionsChild->copyBuffer(stream);
                                cueTrackPositionsChild->discardBuffer();
                                break;
                            case MatroskaIds::CueRelativePosition:
                                if (const auto relativeOffset = m_relativeOffsets.find(cueTrackPositionsChild);
                                    relativeOffset != m_relativeOffsets.end()) {
                                    EbmlElement::makeSimpleElement(stream, cueTrackPositionsChild->id(), relativeOffset->second.currentValue());
                                }
                                // we were not able parse the relative offset because the absolute offset is missing
                                // continue anyways
                                break;
                            case MatroskaIds::CueClusterPosition:
                            case MatroskaIds::CueCodecState:
                                // write "CueClusterPosition"/"CueCodecState"-element
                                EbmlElement::makeSimpleElement(
                                    stream, cueTrackPositionsChild->id(), m_offsets.at(cueTrackPositionsChild).currentValue());
                                break;
                            case MatroskaIds::CueReference:
                                // write "CueReference"-element
                                stream.put(static_cast<char>(MatroskaIds::CueRefTime));
                                len = EbmlElement::makeSizeDenotation(m_sizes[cueTrackPositionsChild], buff);
                                stream.write(buff, len);
                                for (EbmlElement *cueReferenceChild = cueTrackPositionsChild->firstChild(); cueReferenceChild;
                                     cueReferenceChild = cueReferenceChild->nextSibling()) {
                                    cueReferenceChild->parse(diag);
                                    switch (cueReferenceChild->id()) {
                                    case EbmlIds::Void:
                                    case EbmlIds::Crc32:
                                        break;
                                    case MatroskaIds::CueRefTime:
                                    case MatroskaIds::CueRefNumber:
                                        // write unchanged children of "CueReference"-element
                                        cueReferenceChild->copyBuffer(stream);
                                        cueReferenceChild->discardBuffer();
                                        cueReferenceChild->copyEntirely(stream, diag, nullptr);
                                        break;
                                    case MatroskaIds::CueRefCluster:
                                    case MatroskaIds::CueRefCodecState:
                                        // write "CueRefCluster"/"CueRefCodecState"-element
                                        EbmlElement::makeSimpleElement(
                                            stream, cueReferenceChild->id(), m_offsets.at(cueReferenceChild).currentValue());
                                        break;
                                    default:
                                        diag.emplace_back(DiagLevel::Warning,
                                            "\"CueReference\"-element contains a element which is not known to the parser. It will be ignored.",
                                            context);
                                    }
                                }
                                break;
                            default:
                                diag.emplace_back(DiagLevel::Warning,
                                    "\"CueTrackPositions\"-element contains a element which is not known to the parser. It will be ignored.",
                                    context);
                            }
                        }
                        break;
                    default:
                        diag.emplace_back(DiagLevel::Warning,
                            "\"CuePoint\"-element contains a element which is not a \"CueTime\"- or a \"CueTrackPositions\"-element. It will be "
                            "ignored.",
                            context);
                    }
                }
                break;
            default:
                diag.emplace_back(
                    DiagLevel::Warning, "\"Cues\"-element contains a element which is not a \"CuePoint\"-element. It will be ignored.", context);
            }
        }
    } catch (const out_of_range &) {
        diag.emplace_back(
            DiagLevel::Critical, "Unable to write the file index because the index of the original file could not be parsed correctly.", context);
        throw InvalidDataException();
    }
}

} // namespace TagParser
