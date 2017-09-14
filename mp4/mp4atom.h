#ifndef MP4ATOM_H
#define MP4ATOM_H

#include "./mp4ids.h"

#include "../genericfileelement.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/conversion/stringconversion.h>

#include <list>
#include <iostream>
#include <memory>

namespace Media
{

class Mp4Atom;
class Mp4Container;

/*!
 * \brief Defines traits for the GenericFileElement implementation Mp4Atom.
 */
template <>
class TAG_PARSER_EXPORT FileElementTraits<Mp4Atom>
{
public:
    /*!
     * \brief The container type used to store such elements is Mp4Container.
     */
    typedef Mp4Container containerType;

    /*!
     * \brief The type used to store atom IDs is an unsigned 32-bit integer.
     */
    typedef uint32 identifierType;

    /*!
     * \brief The type used to store element sizes is an unsigned 64-bit integer.
     */
    typedef uint64 dataSizeType;

    /*!
     * \brief The implementation type is Mp4Atom.
     */
    typedef Mp4Atom implementationType;

    /*!
     * \brief Returns the minimal atom size which is 8 byte.
     */
    static constexpr byte minimumElementSize()
    {
        return 8;
    }
};

class TAG_PARSER_EXPORT Mp4Atom : public GenericFileElement<Mp4Atom>
{
    friend class GenericFileElement<Mp4Atom>;

public:
    Mp4Atom(containerType& container, uint64 startOffset);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    uint64 firstChildOffset() const;

    static void seekBackAndWriteAtomSize(std::ostream &stream, const std::ostream::pos_type &startOffset);
    static void seekBackAndWriteAtomSize64(std::ostream &stream, const std::ostream::pos_type &startOffset);
    static void addHeaderSize(uint64 &dataSize);
    static void makeHeader(uint64 size, uint32 id, IoUtilities::BinaryWriter &writer);

protected:
    Mp4Atom(containerType& container, uint64 startOffset, uint64 maxSize);
    Mp4Atom(implementationType &parent, uint64 startOffset);

    void internalParse();

private:
    std::string parsingContext() const;
};

/*!
 * \brief Converts the specified atom \a ID to a printable string.
 */
inline std::string Mp4Atom::idToString() const
{
    auto idString = ConversionUtilities::interpretIntegerAsString<identifierType>(id());
    for(char &c : idString) {
        if(c < ' ') {
            c = '?';
        }
    }
    return idString;
}

/*!
 * \brief Adds the header size to the specified \a data size.
 */
inline void Mp4Atom::addHeaderSize(uint64 &dataSize)
{
    dataSize += (dataSize < 0xFFFFFFF7 ? 8 : 16);
}

}

#endif // MP4ATOM_H
