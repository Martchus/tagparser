#include "./matroskaseekinfo.h"
#include "./matroskaid.h"

#include "../exceptions.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/binaryconversion.h>

#include <string>

using namespace std;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::MatroskaSeekInfo
 * \brief The MatroskaSeekInfo class helps parsing and making "SeekHead"-elements.
 */

/*!
 * \brief Shifts all offsets greather or equal than \a start by \a amount bytes.
 */
void MatroskaSeekInfo::shift(uint64 start, int64 amount)
{
    for(auto &info : m_info) {
        if(get<1>(info) >= start) {
            get<1>(info) += amount;
        }
    }
}

/*!
 * \brief Parses the specified \a seekHeadElement.
 * \throws Throws ios_base::failure when an IO error occurs.
 * \throws Throws Failure or a derived exception when a parsing error occurs.
 * \remarks The object does not take ownership over the specified \a seekHeadElement.
 */
void MatroskaSeekInfo::parse(EbmlElement *seekHeadElement)
{
    static const string context("parsing \"SeekHead\"-element");
    m_seekHeadElement = seekHeadElement;
    m_info.clear();
    EbmlElement *seekElement = seekHeadElement->firstChild();
    EbmlElement *seekElementChild, *seekIdElement, *seekPositionElement;
    while(seekElement) {
        seekElement->parse();
        switch(seekElement->id()) {
        case MatroskaIds::Seek:
            seekElementChild = seekElement->firstChild();
            seekIdElement = seekPositionElement = nullptr;
            while(seekElementChild) {
                seekElementChild->parse();
                switch(seekElementChild->id()) {
                case MatroskaIds::SeekID:
                    if(seekIdElement) {
                        addNotification(NotificationType::Warning, "The \"Seek\"-element contains multiple \"SeekID\"-elements. Surplus elements will be ignored.", context);
                    }
                    seekIdElement = seekElementChild;
                    break;
                case MatroskaIds::SeekPosition:
                    if(seekPositionElement) {
                        addNotification(NotificationType::Warning, "The \"Seek\"-element contains multiple \"SeekPosition\"-elements. Surplus elements will be ignored.", context);
                    }
                    seekPositionElement = seekElementChild;
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    addNotification(NotificationType::Warning, "The element \""
                                    % seekElementChild->idToString()
                                    + "\" within the \"Seek\" element is not a \"SeekID\"-element nor a \"SeekPosition\"-element and will be ignored.", context);
                }
                seekElementChild = seekElementChild->nextSibling();
            }
            if(seekIdElement && seekPositionElement) {
                m_info.emplace_back(seekIdElement->readUInteger(), seekPositionElement->readUInteger());
            } else {
                addNotification(NotificationType::Warning, "The \"Seek\"-element does not contain a \"SeekID\"- and a \"SeekPosition\"-element.", context);
            }
            break;
        case EbmlIds::Crc32:
        case EbmlIds::Void:
            break;
        default:
            addNotification(NotificationType::Warning, "The element " % seekElement->idToString() + " is not a seek element and will be ignored.", context);
        }
        seekElement = seekElement->nextSibling();
    }
    if(m_info.empty()) {
        addNotification(NotificationType::Warning, "No seek information found.", context);
    }
}

/*!
 * \brief Writes a "SeekHead" element for the current instance to the specified \a stream.
 * \param stream Specifies the stream to write the "SeekHead" element to.
 * \throws Throws ios_base::failure when an IO error occurs.
 * \throws Throws Failure or a derived exception when a making error occurs.
 */
void MatroskaSeekInfo::make(ostream &stream)
{
    uint64 totalSize = 0;
    char buff0[8];
    char buff1[8];
    char buff2[2];
    byte sizeLength0, sizeLength1;
    // calculate size
    for(const auto &info : m_info) {
        // "Seek" element + "SeekID" element + "SeekPosition" element
        totalSize += 2 + 1 + (2 + 1 + EbmlElement::calculateIdLength(get<0>(info))) + (2 + 1 + EbmlElement::calculateUIntegerLength(get<1>(info)));
    }
    // write ID and size
    BE::getBytes(static_cast<uint32>(MatroskaIds::SeekHead), buff0);
    stream.write(buff0, 4);
    sizeLength0 = EbmlElement::makeSizeDenotation(totalSize, buff0);
    stream.write(buff0, sizeLength0);
    // write entries
    for(const auto &info : m_info) {
        // make values
        sizeLength0 = EbmlElement::makeId(get<0>(info), buff0);
        sizeLength1 = EbmlElement::makeUInteger(get<1>(info), buff1);
        // "Seek" header
        BE::getBytes(static_cast<uint16>(MatroskaIds::Seek), buff2);
        stream.write(buff2, 2);
        stream.put(0x80 | (2 + 1 + sizeLength0 + 2 + 1 + sizeLength1));
        // "SeekID" element
        BE::getBytes(static_cast<uint16>(MatroskaIds::SeekID), buff2);
        stream.write(buff2, 2);
        stream.put(0x80 | sizeLength0);
        stream.write(buff0, sizeLength0);
        // "SeekPosition" element
        BE::getBytes(static_cast<uint16>(MatroskaIds::SeekPosition), buff2);
        stream.write(buff2, 2);
        stream.put(0x80 | sizeLength1);
        stream.write(buff1, sizeLength1);
    }
}

/*!
 * \brief Returns the minimal number of bytes written when calling the make() method.
 * \remarks The returned value gets invalidated when the object is mutated.
 */
uint64 MatroskaSeekInfo::minSize() const
{
    uint64 maxTotalSize = m_info.size() * (2 + 1 + 2 + 1 + 1 + 2 + 1 + 1);
    return 4 + EbmlElement::calculateSizeDenotationLength(maxTotalSize) + maxTotalSize;
}

/*!
 * \brief Returns the maximal number of bytes written when calling the make() method.
 * \remarks The returned value gets invalidated when the object is mutated.
 */
uint64 MatroskaSeekInfo::maxSize() const
{
    uint64 maxTotalSize = m_info.size() * (2 + 1 + 2 + 1 + 4 + 2 + 1 + 8);
    return 4 + EbmlElement::calculateSizeDenotationLength(maxTotalSize) + maxTotalSize;
}

/*!
 * \brief Returns the number of bytes which will be written when calling the make() method.
 * \remarks The returned value gets invalidated when the object is mutated.
 */
uint64 MatroskaSeekInfo::actualSize() const
{
    uint64 totalSize = 0;
    for(const auto &info : m_info) {
        // "Seek" element + "SeekID" element + "SeekPosition" element
        totalSize += 2 + 1 + (2 + 1 + EbmlElement::calculateIdLength(get<0>(info))) + (2 + 1 + EbmlElement::calculateUIntegerLength(get<1>(info)));
    }
    return totalSize += 4 + EbmlElement::calculateSizeDenotationLength(totalSize);
}

/*!
 * \brief Pushes the specified \a offset of an element with the specified \a id to the info.
 *
 * If there is an existing entry with the same \a id and \a index the existing entry will be
 * updated and no new entry created.
 *
 * \returns Returns an indication whether the actualSize() has changed.
 */
bool MatroskaSeekInfo::push(unsigned int index, EbmlElement::IdentifierType id, uint64 offset)
{
    unsigned int currentIndex = 0;
    for(auto &entry : info()) {
        if(get<0>(entry) == id) {
            if(index == currentIndex) {
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
    m_seekHeadElement = nullptr;
    m_info.clear();
}

/*!
 * \brief Returns a pointer to the first pair with the specified \a offset or nullptr if no such pair could be found.
 */
std::pair<EbmlElement::IdentifierType, uint64> *MatroskaSeekInfo::findSeekInfo(std::vector<MatroskaSeekInfo> &seekInfos, uint64 offset)
{
    for(auto &seekInfo : seekInfos) {
        for(auto &entry : seekInfo.info()) {
            if(get<1>(entry) == offset) {
                return &entry;
            }
        }
    }
    return nullptr;
}

/*!
 * \brief Sets the offset of all entires in \a newSeekInfos to \a newOffset where the corresponding entry in \a oldSeekInfos has the offset \a oldOffset.
 * \returns Returns an indication whether the update altered the offset length.
 */
bool MatroskaSeekInfo::updateSeekInfo(const std::vector<MatroskaSeekInfo> &oldSeekInfos, std::vector<MatroskaSeekInfo> &newSeekInfos, uint64 oldOffset, uint64 newOffset)
{
    bool updated = false;
    auto oldIterator0 = oldSeekInfos.cbegin(), oldEnd0 = oldSeekInfos.cend();
    auto newIterator0 = newSeekInfos.begin(), newEnd0 = newSeekInfos.end();
    for(; oldIterator0 != oldEnd0 && newIterator0 != newEnd0; ++oldIterator0, ++newIterator0) {
        auto oldIterator1 = oldIterator0->info().cbegin(), oldEnd1 = oldIterator0->info().cend();
        auto newIterator1 = newIterator0->info().begin(), newEnd1 = newIterator0->info().end();
        for(; oldIterator1 != oldEnd1 && newIterator1 != newEnd1; ++oldIterator1, ++newIterator1) {
            if(get<1>(*oldIterator1) == oldOffset) {
                if(get<1>(*newIterator1) != newOffset) {
                    updated = updated || (EbmlElement::calculateUIntegerLength(newOffset) != EbmlElement::calculateUIntegerLength(get<1>(*newIterator1)));
                    get<1>(*newIterator1) = newOffset;
                }
            }
        }
    }
    return updated;
}

/*!
 * \brief Sets the offset of all entires in \a newSeekInfos to \a newOffset where the offset is \a oldOffset.
 * \returns Returns an whether at least one offset has been updated.
 */
bool MatroskaSeekInfo::updateSeekInfo(std::vector<MatroskaSeekInfo> &newSeekInfos, uint64 oldOffset, uint64 newOffset)
{
    if(oldOffset == newOffset) {
        return false;
    }
    bool updated = false;
    for(auto &seekInfo : newSeekInfos) {
        for(auto &info : seekInfo.info()) {
            if(get<1>(info) == oldOffset) {
                get<1>(info) = newOffset;
                updated = true;
            }
        }
    }
    return updated;
}

}
