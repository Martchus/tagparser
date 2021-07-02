#include "./matroskaseekinfo.h"
#include "./matroskaid.h"

#include "../diagnostics.h"
#include "../exceptions.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringbuilder.h>

#include <string>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::MatroskaSeekInfo
 * \brief The MatroskaSeekInfo class helps parsing and making "SeekHead"-elements.
 */

/*!
 * \brief Shifts all offsets greater or equal than \a start by \a amount bytes.
 */
void MatroskaSeekInfo::shift(std::uint64_t start, std::int64_t amount)
{
    for (auto &info : m_info) {
        if (get<1>(info) >= start) {
            if (amount > 0) {
                get<1>(info) += static_cast<std::uint64_t>(amount);
            } else {
                get<1>(info) -= static_cast<std::uint64_t>(-amount);
            }
        }
    }
}

/*!
 * \brief Parses the specified \a seekHeadElement and populates info() with the gathered information.
 * \throws Throws ios_base::failure when an IO error occurs.
 * \throws Throws Failure or a derived exception when a parsing error occurs.
 * \remarks
 * - The object does not take ownership over the specified \a seekHeadElement.
 * - Possibly previously parsed info() is not cleared. So subsequent calls can be used to gather seek
 *   information from multiple seek head elements. Use clear() manually if that is not wanted.
 * - If the specified \a seekHeadElement references another seek head element the referenced seek head
 *   element is parsed as well. One can set \a maxIndirection to 0 to prevent that or even increase the value
 *   to allow following references even more deeply. References to elements which have already been visited
 *   are never followed, though.
 */
void MatroskaSeekInfo::parse(EbmlElement *seekHeadElement, Diagnostics &diag, size_t maxIndirection)
{
    static const string context("parsing \"SeekHead\"-element");

    m_seekHeadElements.emplace_back(seekHeadElement);

    for (EbmlElement *seekElement = seekHeadElement->firstChild(), *seekIdElement, *seekPositionElement; seekElement;
         seekElement = seekElement->nextSibling()) {
        seekElement->parse(diag);
        switch (seekElement->id()) {
        case MatroskaIds::Seek:
            seekIdElement = seekPositionElement = nullptr;
            for (auto *seekElementChild = seekElement->firstChild(); seekElementChild; seekElementChild = seekElementChild->nextSibling()) {
                seekElementChild->parse(diag);
                switch (seekElementChild->id()) {
                case MatroskaIds::SeekID:
                    if (seekIdElement) {
                        diag.emplace_back(DiagLevel::Warning,
                            "The \"Seek\"-element contains multiple \"SeekID\"-elements. Surplus elements will be ignored.", context);
                    }
                    seekIdElement = seekElementChild;
                    break;
                case MatroskaIds::SeekPosition:
                    if (seekPositionElement) {
                        diag.emplace_back(DiagLevel::Warning,
                            "The \"Seek\"-element contains multiple \"SeekPosition\"-elements. Surplus elements will be ignored.", context);
                    }
                    seekPositionElement = seekElementChild;
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning,
                        "The element \"" % seekElementChild->idToString()
                            + "\" within the \"Seek\" element is not a \"SeekID\"-element nor a \"SeekPosition\"-element and will be ignored.",
                        context);
                }
            }

            if (!seekIdElement || !seekPositionElement) {
                diag.emplace_back(DiagLevel::Warning, "The \"Seek\"-element does not contain a \"SeekID\"- and a \"SeekPosition\"-element.", context);
                break;
            }

            m_info.emplace_back(seekIdElement->readUInteger(), seekPositionElement->readUInteger());

            // follow possibly referenced seek head element
            if (m_info.back().first == MatroskaIds::SeekHead) {
                const auto startOffset = m_info.back().second;
                if (!maxIndirection) {
                    diag.emplace_back(DiagLevel::Warning,
                        argsToString("Not following reference by \"Seek\"-element at ", seekElement->startOffset(),
                            " which points to another \"SeekHead\"-element at ", startOffset, '.'),
                        context);
                    break;
                }

                auto visited = false;
                for (const auto *const visitedSeekHeadElement : m_seekHeadElements) {
                    if (visitedSeekHeadElement->startOffset() == startOffset) {
                        diag.emplace_back(DiagLevel::Warning,
                            argsToString("The \"Seek\"-element at ", seekElement->startOffset(), " contains a loop to the \"SeekHead\"-element at ",
                                visitedSeekHeadElement->startOffset(), '.'),
                            context);
                        visited = true;
                        break;
                    }
                }
                if (visited) {
                    break;
                }
                m_additionalSeekHeadElements.emplace_back(make_unique<EbmlElement>(seekHeadElement->container(), startOffset));
                parse(m_additionalSeekHeadElements.back().get(), diag, maxIndirection - 1);
            }

            break;
        case EbmlIds::Crc32:
        case EbmlIds::Void:
            break;
        default:
            diag.emplace_back(
                DiagLevel::Warning, "The element " % seekElement->idToString() + " is not a seek element and will be ignored.", context);
        }
    }
    if (m_info.empty()) {
        diag.emplace_back(DiagLevel::Warning, "No seek information found.", context);
    }
}

/*!
 * \brief Writes a "SeekHead" element for the current instance to the specified \a stream.
 * \param stream Specifies the stream to write the "SeekHead" element to.
 * \throws Throws ios_base::failure when an IO error occurs.
 * \throws Throws Failure or a derived exception when a making error occurs.
 */
void MatroskaSeekInfo::make(ostream &stream, Diagnostics &diag)
{
    CPP_UTILITIES_UNUSED(diag)

    std::uint64_t totalSize = 0;
    char buff0[8];
    char buff1[8];
    char buff2[2];
    std::uint8_t sizeLength0, sizeLength1;
    // calculate size
    for (const auto &info : m_info) {
        // "Seek" element + "SeekID" element + "SeekPosition" element
        totalSize += 2u + 1u + (2u + 1u + EbmlElement::calculateIdLength(info.first)) + (2u + 1u + EbmlElement::calculateUIntegerLength(info.second));
    }
    // write ID and size
    BE::getBytes(static_cast<std::uint32_t>(MatroskaIds::SeekHead), buff0);
    stream.write(buff0, 4);
    sizeLength0 = EbmlElement::makeSizeDenotation(totalSize, buff0);
    stream.write(buff0, sizeLength0);
    // write entries
    for (const auto &info : m_info) {
        // make values
        sizeLength0 = EbmlElement::makeId(info.first, buff0);
        sizeLength1 = EbmlElement::makeUInteger(info.second, buff1);
        // "Seek" header
        BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::Seek), buff2);
        stream.write(buff2, 2);
        stream.put(static_cast<char>(0x80 | (2 + 1 + sizeLength0 + 2 + 1 + sizeLength1)));
        // "SeekID" element
        BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::SeekID), buff2);
        stream.write(buff2, 2);
        stream.put(static_cast<char>(0x80 | sizeLength0));
        stream.write(buff0, sizeLength0);
        // "SeekPosition" element
        BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::SeekPosition), buff2);
        stream.write(buff2, 2);
        stream.put(static_cast<char>(0x80 | sizeLength1));
        stream.write(buff1, sizeLength1);
    }
}

/*!
 * \brief Returns the minimal number of bytes written when calling the make() method.
 * \remarks The returned value gets invalidated when the object is mutated.
 */
std::uint64_t MatroskaSeekInfo::minSize() const
{
    std::uint64_t maxTotalSize = m_info.size() * (2 + 1 + 2 + 1 + 1 + 2 + 1 + 1);
    return 4 + EbmlElement::calculateSizeDenotationLength(maxTotalSize) + maxTotalSize;
}

/*!
 * \brief Returns the maximal number of bytes written when calling the make() method.
 * \remarks The returned value gets invalidated when the object is mutated.
 */
std::uint64_t MatroskaSeekInfo::maxSize() const
{
    std::uint64_t maxTotalSize = m_info.size() * (2 + 1 + 2 + 1 + 4 + 2 + 1 + 8);
    return 4 + EbmlElement::calculateSizeDenotationLength(maxTotalSize) + maxTotalSize;
}

/*!
 * \brief Returns the number of bytes which will be written when calling the make() method.
 * \remarks The returned value gets invalidated when the object is mutated.
 */
std::uint64_t MatroskaSeekInfo::actualSize() const
{
    std::uint64_t totalSize = 0;
    for (const auto &info : m_info) {
        // "Seek" element + "SeekID" element + "SeekPosition" element
        totalSize
            += 2u + 1u + (2 + 1 + EbmlElement::calculateIdLength(get<0>(info))) + (2u + 1u + EbmlElement::calculateUIntegerLength(get<1>(info)));
    }
    return totalSize += 4u + EbmlElement::calculateSizeDenotationLength(totalSize);
}

/*!
 * \brief Pushes the specified \a offset of an element with the specified \a id to the info.
 *
 * If there is an existing entry with the same \a id and \a index the existing entry will be
 * updated and no new entry created.
 *
 * \returns Returns an indication whether the actualSize() has changed.
 */
bool MatroskaSeekInfo::push(unsigned int index, EbmlElement::IdentifierType id, std::uint64_t offset)
{
    unsigned int currentIndex = 0;
    for (auto &entry : info()) {
        if (get<0>(entry) == id) {
            if (index == currentIndex) {
                bool sizeUpdated = EbmlElement::calculateUIntegerLength(get<1>(entry)) != EbmlElement::calculateUIntegerLength(offset);
                get<1>(entry) = offset;
                return sizeUpdated;
            }
            ++currentIndex;
        }
    }
    info().emplace_back(id, offset);
    return true;
}

/*!
 * \brief Resets the object to its initial state.
 */
void MatroskaSeekInfo::clear()
{
    m_seekHeadElements.clear();
    m_additionalSeekHeadElements.clear();
    m_info.clear();
}

} // namespace TagParser
