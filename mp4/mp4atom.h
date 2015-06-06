#ifndef MP4ATOM_H
#define MP4ATOM_H

#include "mp4ids.h"

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
class LIB_EXPORT FileElementTraits<Mp4Atom>
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
};

class LIB_EXPORT Mp4Atom : public GenericFileElement<Mp4Atom>
{
    friend class GenericFileElement<Mp4Atom>;

public:
    Mp4Atom(containerType& container, uint64 startOffset);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    uint64 firstChildOffset() const;

    static void seekBackAndWriteAtomSize(std::ostream &stream, const std::ostream::pos_type &startOffset, bool denote64BitSize = false);

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
    return ConversionUtilities::interpretIntegerAsString<identifierType>(id());
}

}

#endif // MP4ATOM_H
