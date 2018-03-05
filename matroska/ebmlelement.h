#ifndef EBMLELEMENT_H
#define EBMLELEMENT_H

#include "./ebmlid.h"
#include "./matroskaid.h"

#include "../genericfileelement.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>

#include <iostream>
#include <memory>

namespace Media {

class EbmlElement;
class MatroskaContainer;

/*!
 * \brief Defines traits for the GenericFileElement implementation EbmlElement.
 */
template <>
class TAG_PARSER_EXPORT FileElementTraits<EbmlElement>
{
public:
    typedef MatroskaContainer ContainerType;
    typedef uint32 IdentifierType;
    typedef uint64 DataSizeType;
};

class TAG_PARSER_EXPORT EbmlElement : public GenericFileElement<EbmlElement>
{
    friend class GenericFileElement<EbmlElement>;

public:
    EbmlElement(MatroskaContainer &container, uint64 startOffset);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    uint64 firstChildOffset() const;
    std::string readString();
    uint64 readUInteger();
    float64 readFloat();

    static byte calculateIdLength(IdentifierType id);
    static byte calculateSizeDenotationLength(uint64 size);
    static byte makeId(IdentifierType id, char *buff);
    static byte makeSizeDenotation(uint64 size, char *buff);
    static byte makeSizeDenotation(uint64 size, char *buff, byte minBytes);
    static byte calculateUIntegerLength(uint64 integer);
    static byte makeUInteger(uint64 value, char *buff);
    static byte makeUInteger(uint64 value, char *buff, byte minBytes);
    static void makeSimpleElement(std::ostream &stream, IdentifierType id, uint64 content);
    static void makeSimpleElement(std::ostream &stream, IdentifierType id, const std::string &content);
    static void makeSimpleElement(std::ostream &stream, IdentifierType id, const char *data, std::size_t dataSize);
    static uint64 bytesToBeSkipped;

protected:
    EbmlElement(EbmlElement &parent, uint64 startOffset);
    EbmlElement(MatroskaContainer &container, uint64 startOffset, uint64 maxSize);

    void internalParse(Diagnostics &diag);

private:
    std::string parsingContext() const;
};

/*!
 * \brief Converts the specified EBML \a ID to a printable string.
 */
inline std::string EbmlElement::idToString() const
{
    using namespace ConversionUtilities;
    const char *const name = matroskaIdName(id());
    if(*name) {
        return argsToString('0', 'x', numberToString(id(), 16), ' ', '\"', name, '\"');
    } else {
        return "0x" + numberToString(id(), 16);
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
    switch(id()) {
    case Header:
    case SignatureSlot: case SignatureElements: case SignatureElementList:
    case Segment: case SeekHead: case Seek:
    case SegmentInfo: case ChapterTranslate:
    case Cluster: case SilentTracks: case BlockGroup: case BlockAdditions: case BlockMore: case Slices: case TimeSlice: case ReferenceFrame:
    case Tracks: case TrackEntry: case TrackTranslate: case TrackVideo: case TrackAudio: case TrackOperation: case TrackCombinePlanes: case TrackPlane: case TrackJoinBlocks:
    case ContentEncodings: case ContentEncoding: case ContentCompression: case ContentEncryption:
    case Cues: case CuePoint: case CueTrackPositions: case CueReference:
    case Attachments: case AttachedFile:
    case Chapters: case EditionEntry: case ChapterAtom: case ChapterTrack: case ChapterDisplay: case ChapProcess: case ChapProcessCommand:
    case Tags: case MatroskaIds::Tag: case Targets: case SimpleTag:
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
inline uint64 EbmlElement::firstChildOffset() const
{
    return isParent() ? (idLength() + sizeLength()) : 0;
}

}


#endif // EBMLELEMENT_H
