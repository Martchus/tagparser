#ifndef TAG_PARSER_MP4ATOM_H
#define TAG_PARSER_MP4ATOM_H

#include "./mp4ids.h"

#include "../genericfileelement.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/conversion/types.h>

#include <iostream>
#include <list>
#include <memory>

namespace TagParser {

class Mp4Atom;
class Mp4Container;

/*!
 * \brief Defines traits for the GenericFileElement implementation Mp4Atom.
 */
template <> class TAG_PARSER_EXPORT FileElementTraits<Mp4Atom> {
public:
    using ContainerType = Mp4Container;
    using IdentifierType = uint32;
    using DataSizeType = uint64;

    /*!
     * \brief Returns the minimal atom size which is 8 byte.
     */
    static constexpr byte minimumElementSize()
    {
        return 8;
    }
};

class TAG_PARSER_EXPORT Mp4Atom : public GenericFileElement<Mp4Atom> {
    friend class GenericFileElement<Mp4Atom>;

public:
    Mp4Atom(ContainerType &container, uint64 startOffset);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    uint64 firstChildOffset() const;

    static void seekBackAndWriteAtomSize(std::ostream &stream, const std::ostream::pos_type &startOffset, Diagnostics &diag);
    static void seekBackAndWriteAtomSize64(std::ostream &stream, const std::ostream::pos_type &startOffset);
    static constexpr void addHeaderSize(uint64 &dataSize);
    static void makeHeader(uint64 size, uint32 id, IoUtilities::BinaryWriter &writer);

protected:
    Mp4Atom(ContainerType &container, uint64 startOffset, uint64 maxSize);
    Mp4Atom(Mp4Atom &parent, uint64 startOffset);

    void internalParse(Diagnostics &diag);

private:
    std::string parsingContext() const;
};

/*!
 * \brief Converts the specified atom \a ID to a printable string.
 */
inline std::string Mp4Atom::idToString() const
{
    auto idString = ConversionUtilities::interpretIntegerAsString<IdentifierType>(id());
    for (char &c : idString) {
        if (c < ' ') {
            c = '?';
        }
    }
    return idString;
}

/*!
 * \brief Adds the header size to the specified \a data size.
 */
constexpr void Mp4Atom::addHeaderSize(uint64 &dataSize)
{
    dataSize += (dataSize < 0xFFFFFFF7 ? 8 : 16);
}

} // namespace TagParser

#endif // TAG_PARSER_MP4ATOM_H
