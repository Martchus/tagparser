#ifndef MPEG4DESCRIPTOR_H
#define MPEG4DESCRIPTOR_H

#include "../genericfileelement.h"

#include <c++utilities/misc/memory.h>

namespace Media {

class Mp4Container;
class Mpeg4Descriptor;

/*!
 * \brief Defines traits for the GenericFileElement implementation Mpeg4Descriptor.
 */
template <>
class LIB_EXPORT FileElementTraits<Mpeg4Descriptor>
{
public:
    /*!
     * \brief The container type used to store such elements is Mp4Container.
     */
    typedef Mp4Container containerType;

    /*!
     * \brief The type used to store atom IDs is an unsigned 32-bit integer.
     */
    typedef byte identifierType;

    /*!
     * \brief The type used to store element sizes is an unsigned 32-bit integer.
     */
    typedef uint32 dataSizeType;

    /*!
     * \brief The implementation type is Mp4Atom.
     */
    typedef Mpeg4Descriptor implementationType;
};

class LIB_EXPORT Mpeg4Descriptor : public GenericFileElement<Mpeg4Descriptor>
{
    friend class GenericFileElement<Mpeg4Descriptor>;

public:
    Mpeg4Descriptor(containerType& container, uint64 startOffset, uint64 maxSize);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    uint64 firstChildOffset() const;
    implementationType *denoteFirstChild(uint32 offset);

protected:
    Mpeg4Descriptor(implementationType &parent, uint64 startOffset);

    void internalParse();

private:
    std::string parsingContext() const;
};

/*!
 * \brief Returns an indication whether the descriptor contains sub descriptors.
 *
 * \remarks Returns true if a first child has been denoted (via denoteFirstChild()).
 */
inline bool Mpeg4Descriptor::isParent() const
{
    return m_firstChild != nullptr;
}

/*!
 * \brief Returns always false for MPEG-4 descriptors.
 */
inline bool Mpeg4Descriptor::isPadding() const
{
    return false;
}

/*!
 * \brief Returns the offset of the first child (relative to the start offset of this descriptor).
 *
 * \remarks The first child must be denoted (via denoteFirstChild()).
 */
inline uint64 Mpeg4Descriptor::firstChildOffset() const
{
    return firstChild() ? firstChild()->startOffset() - startOffset() : 0;
}

/*!
 * \brief Denotes the first child to start at the specified \a offset (relative to the start offset of this descriptor).
 * \remarks A new first child is constructed. A possibly existing subtree is invalidated.
 */
inline Mpeg4Descriptor::implementationType *Mpeg4Descriptor::denoteFirstChild(uint32 relativeFirstChildOffset)
{
    m_firstChild.reset(new implementationType(static_cast<implementationType &>(*this), startOffset() + relativeFirstChildOffset));
    return m_firstChild.get();
}

}

#endif // MPEG4DESCRIPTOR_H
