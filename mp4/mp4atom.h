#ifndef TAG_PARSER_MP4ATOM_H
#define TAG_PARSER_MP4ATOM_H

#include "./mp4ids.h"

#include "../genericfileelement.h"

#include <c++utilities/conversion/stringconversion.h>

#include <cstdint>
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
    using IdentifierType = std::uint32_t;
    using DataSizeType = std::uint64_t;

    /*!
     * \brief Returns the minimal atom size which is 8 byte.
     */
    static constexpr std::uint8_t minimumElementSize()
    {
        return 8;
    }
};

class TAG_PARSER_EXPORT Mp4Atom : public GenericFileElement<Mp4Atom> {
    friend class GenericFileElement<Mp4Atom>;

public:
    Mp4Atom(ContainerType &container, std::uint64_t startOffset);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    std::uint64_t firstChildOffset() const;

    static void seekBackAndWriteAtomSize(std::ostream &stream, const std::ostream::pos_type &startOffset, Diagnostics &diag);
    static void seekBackAndWriteAtomSize64(std::ostream &stream, const std::ostream::pos_type &startOffset);
    static constexpr void addHeaderSize(std::uint64_t &dataSize);
    static void makeHeader(std::uint64_t size, std::uint32_t id, CppUtilities::BinaryWriter &writer);

protected:
    Mp4Atom(ContainerType &container, std::uint64_t startOffset, std::uint64_t maxSize);
    Mp4Atom(Mp4Atom &parent, std::uint64_t startOffset);

    void internalParse(Diagnostics &diag);

private:
    std::string parsingContext() const;
};

/*!
 * \brief Converts the specified atom \a ID to a printable string.
 */
inline std::string Mp4Atom::idToString() const
{
    auto idString = CppUtilities::interpretIntegerAsString<IdentifierType>(id());
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
constexpr void Mp4Atom::addHeaderSize(std::uint64_t &dataSize)
{
    dataSize += (dataSize < 0xFFFFFFF7 ? 8 : 16);
}

} // namespace TagParser

#endif // TAG_PARSER_MP4ATOM_H
