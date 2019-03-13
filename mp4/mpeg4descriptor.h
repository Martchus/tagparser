#ifndef TAG_PARSER_MPEG4DESCRIPTOR_H
#define TAG_PARSER_MPEG4DESCRIPTOR_H

#include "../genericfileelement.h"

#include <memory>

namespace TagParser {

class Mp4Container;
class Mpeg4Descriptor;

/*!
 * \brief Defines traits for the GenericFileElement implementation Mpeg4Descriptor.
 */
template <> class TAG_PARSER_EXPORT FileElementTraits<Mpeg4Descriptor> {
public:
    using ContainerType = Mp4Container;
    using IdentifierType = std::uint8_t;
    using DataSizeType = std::uint32_t;

    /*!
     * \brief Returns the minimal descriptor size which is 2 byte.
     */
    static constexpr std::uint8_t minimumElementSize()
    {
        return 2;
    }
};

class TAG_PARSER_EXPORT Mpeg4Descriptor : public GenericFileElement<Mpeg4Descriptor> {
    friend class GenericFileElement<Mpeg4Descriptor>;

public:
    Mpeg4Descriptor(ContainerType &container, std::uint64_t startOffset, std::uint64_t maxSize);

    std::string idToString() const;
    bool isParent() const;
    bool isPadding() const;
    std::uint64_t firstChildOffset() const;

protected:
    Mpeg4Descriptor(Mpeg4Descriptor &parent, std::uint64_t startOffset);

    void internalParse(Diagnostics &diag);

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
inline std::uint64_t Mpeg4Descriptor::firstChildOffset() const
{
    return firstChild() ? firstChild()->startOffset() - startOffset() : 0;
}

} // namespace TagParser

#endif // TAG_PARSER_MPEG4DESCRIPTOR_H
