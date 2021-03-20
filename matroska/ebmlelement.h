#ifndef TAG_PARSER_EBMLELEMENT_H
#define TAG_PARSER_EBMLELEMENT_H

#include "./ebmlid.h"
#include "./matroskaid.h"

#include "../genericfileelement.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string_view>

namespace TagParser {

class EbmlElement;
class MatroskaContainer;

/*!
 * \brief Defines traits for the GenericFileElement implementation EbmlElement.
 */
template <> class TAG_PARSER_EXPORT FileElementTraits<EbmlElement> {
public:
    using ContainerType = MatroskaContainer;
    using IdentifierType = std::uint32_t;
    using DataSizeType = std::uint64_t;
};

class TAG_PARSER_EXPORT EbmlElement : public GenericFileElement<EbmlElement> {
    friend class GenericFileElement<EbmlElement>;

public:
    EbmlElement(MatroskaContainer &container, std::uint64_t startOffset);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    std::uint64_t firstChildOffset() const;
    std::string readString();
    std::uint64_t readUInteger();
    double readFloat();

    static std::uint8_t calculateIdLength(IdentifierType id);
    static std::uint8_t calculateSizeDenotationLength(std::uint64_t size);
    static std::uint8_t makeId(IdentifierType id, char *buff);
    static std::uint8_t makeSizeDenotation(std::uint64_t size, char *buff);
    static std::uint8_t makeSizeDenotation(std::uint64_t size, char *buff, std::uint8_t minBytes);
    static std::uint8_t calculateUIntegerLength(std::uint64_t integer);
    static std::uint8_t makeUInteger(std::uint64_t value, char *buff);
    static std::uint8_t makeUInteger(std::uint64_t value, char *buff, std::uint8_t minBytes);
    static void makeSimpleElement(std::ostream &stream, IdentifierType id, std::uint64_t content);
    static void makeSimpleElement(std::ostream &stream, IdentifierType id, std::string_view content);
    static std::uint64_t bytesToBeSkipped;

protected:
    EbmlElement(EbmlElement &parent, std::uint64_t startOffset);
    EbmlElement(MatroskaContainer &container, std::uint64_t startOffset, std::uint64_t maxSize);

    void internalParse(Diagnostics &diag);

private:
    std::string parsingContext() const;
};

/*!
 * \brief Converts the specified EBML \a ID to a printable string.
 */
inline std::string EbmlElement::idToString() const
{
    using namespace CppUtilities;
    if (const auto name = matroskaIdName(id()); !name.empty()) {
        return argsToString('0', 'x', numberToString(id(), 16u), ' ', '\"', name, '\"');
    } else {
        return "0x" + numberToString(id(), 16u);
    }
}

/*!
 * \brief Returns an indication whether the element is a parent element.
 * \remarks This information is not read from the element header. Some
 *          elements are simply known to be parents whereas all other
 *          are considered as non-parents.
 */
inline bool EbmlElement::isParent() const
{
    using namespace EbmlIds;
    using namespace MatroskaIds;
    switch (id()) {
    case Header:
    case SignatureSlot:
    case SignatureElements:
    case SignatureElementList:
    case Segment:
    case SeekHead:
    case Seek:
    case SegmentInfo:
    case ChapterTranslate:
    case Cluster:
    case SilentTracks:
    case BlockGroup:
    case BlockAdditions:
    case BlockMore:
    case Slices:
    case TimeSlice:
    case ReferenceFrame:
    case Tracks:
    case TrackEntry:
    case TrackTranslate:
    case TrackVideo:
    case TrackAudio:
    case TrackOperation:
    case TrackCombinePlanes:
    case TrackPlane:
    case TrackJoinBlocks:
    case ContentEncodings:
    case ContentEncoding:
    case ContentCompression:
    case ContentEncryption:
    case Cues:
    case CuePoint:
    case CueTrackPositions:
    case CueReference:
    case Attachments:
    case AttachedFile:
    case Chapters:
    case EditionEntry:
    case ChapterAtom:
    case ChapterTrack:
    case ChapterDisplay:
    case ChapProcess:
    case ChapProcessCommand:
    case Tags:
    case MatroskaIds::Tag:
    case Targets:
    case SimpleTag:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns an indication whether the element is considered as padding.
 */
inline bool EbmlElement::isPadding() const
{
    return id() == EbmlIds::Void;
}

/*!
 * \brief Returns the offset of the first child of the element.
 * \remarks The returned offset is relative to the start offset if this element.
 */
inline std::uint64_t EbmlElement::firstChildOffset() const
{
    return isParent() ? (idLength() + sizeLength()) : 0;
}

} // namespace TagParser

#endif // TAG_PARSER_EBMLELEMENT_H
