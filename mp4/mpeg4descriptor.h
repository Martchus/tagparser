#ifndef MPEG4DESCRIPTOR_H
#define MPEG4DESCRIPTOR_H

#include "../genericfileelement.h"

#include <memory>

namespace Media {

class Mp4Container;
class Mpeg4Descriptor;

/*!
 * \brief Defines traits for the GenericFileElement implementation Mpeg4Descriptor.
 */
template <>
class TAG_PARSER_EXPORT FileElementTraits<Mpeg4Descriptor>
{
public:
    typedef Mp4Container ContainerType;
    typedef byte IdentifierType;
    typedef uint32 DataSizeType;

    /*!
     * \brief Returns the minimal descriptor size which is 2 byte.
     */
    static constexpr byte minimumElementSize()
    {
        return 2;
    }
};

class TAG_PARSER_EXPORT Mpeg4Descriptor : public GenericFileElement<Mpeg4Descriptor>
{
    friend class GenericFileElement<Mpeg4Descriptor>;

public:
    Mpeg4Descriptor(ContainerType &container, uint64 startOffset, uint64 maxSize);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    uint64 firstChildOffset() const;

protected:
    Mpeg4Descriptor(Mpeg4Descriptor &parent, uint64 startOffset);

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

}

#endif // MPEG4DESCRIPTOR_H
