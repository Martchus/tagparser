#ifndef TAG_PARSER_GENERICFILEELEMENT_H
#define TAG_PARSER_GENERICFILEELEMENT_H

#include "./exceptions.h"
#include "./progressfeedback.h"

#include <c++utilities/io/copy.h>

#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <list>
#include <memory>
#include <string>

namespace CppUtilities {
class BinaryReader;
class BinaryWriter;
} // namespace CppUtilities

namespace TagParser {

class Diagnostics;

/*!
 * \class TagParser::FileElementTraits
 * \brief Defines traits for the specified \a ImplementationType.
 *
 * A template specialization for each GenericFileElement should
 * be provided.
 *
 * For an example of such a specialization see FileElementTraits<Mp4Atom> or FileElementTraits<EbmlElement>.
 */
template <typename ImplementationType> class FileElementTraits {};

/*!
 * \class TagParser::GenericFileElement
 * \brief The GenericFileElement class helps to parse binary files which consist
 *        of an arboreal element structure.
 * \tparam ImplementationType Specifies the type of the actual implementation.
 * \remarks This template class is intended to be subclassed using
 *          with the "Curiously recurring template pattern".
 */
template <class ImplementationType> class TAG_PARSER_EXPORT GenericFileElement {
    friend class FileElementTraits<ImplementationType>;

public:
    /*!
     * \brief Specifies the type of the corresponding container.
     */
    using ContainerType = typename FileElementTraits<ImplementationType>::ContainerType;

    /*!
     * \brief Specifies the type used to store identifiers.
     */
    using IdentifierType = typename FileElementTraits<ImplementationType>::IdentifierType;

    /*!
     * \brief Specifies the type used to store data sizes.
     */
    using DataSizeType = typename FileElementTraits<ImplementationType>::DataSizeType;

    GenericFileElement(ContainerType &container, std::uint64_t startOffset);
    GenericFileElement(ImplementationType &parent, std::uint64_t startOffset);
    GenericFileElement(ContainerType &container, std::uint64_t startOffset, std::uint64_t maxSize);
    GenericFileElement(const GenericFileElement &other) = delete;
    GenericFileElement(GenericFileElement &other) = delete;
    GenericFileElement &operator=(const GenericFileElement &other) = delete;

    ContainerType &container();
    const ContainerType &container() const;
    std::iostream &stream();
    CppUtilities::BinaryReader &reader();
    CppUtilities::BinaryWriter &writer();
    std::uint64_t startOffset() const;
    std::uint64_t relativeStartOffset() const;
    const IdentifierType &id() const;
    std::string idToString() const;
    std::uint32_t idLength() const;
    std::uint32_t headerSize() const;
    DataSizeType dataSize() const;
    std::uint32_t sizeLength() const;
    std::uint64_t dataOffset() const;
    std::uint64_t totalSize() const;
    std::uint64_t endOffset() const;
    std::uint64_t maxTotalSize() const;
    std::uint8_t level() const;
    ImplementationType *parent();
    const ImplementationType *parent() const;
    ImplementationType *parent(std::uint8_t n);
    const ImplementationType *parent(std::uint8_t n) const;
    ImplementationType *nextSibling();
    const ImplementationType *nextSibling() const;
    ImplementationType *firstChild();
    const ImplementationType *firstChild() const;
    ImplementationType *lastChild();
    const ImplementationType *lastChild() const;
    ImplementationType *subelementByPath(Diagnostics &diag, IdentifierType item);
    ImplementationType *subelementByPath(Diagnostics &diag, IdentifierType item, IdentifierType remainingPath...);
    const ImplementationType *subelementByPath(Diagnostics &diag, IdentifierType item) const;
    const ImplementationType *subelementByPath(Diagnostics &diag, IdentifierType item, IdentifierType remainingPath...) const;
    ImplementationType *childById(const IdentifierType &id, Diagnostics &diag);
    const ImplementationType *childById(const IdentifierType &id, Diagnostics &diag) const;
    ImplementationType *siblingById(const IdentifierType &id, Diagnostics &diag);
    const ImplementationType *siblingById(const IdentifierType &id, Diagnostics &diag) const;
    ImplementationType *siblingByIdIncludingThis(const IdentifierType &id, Diagnostics &diag);
    const ImplementationType *siblingByIdIncludingThis(const IdentifierType &id, Diagnostics &diag) const;
    bool isParent() const;
    bool isPadding() const;
    std::uint64_t firstChildOffset() const;
    bool isParsed() const;
    void clear();
    void parse(Diagnostics &diag);
    void reparse(Diagnostics &diag);
    void validateSubsequentElementStructure(Diagnostics &diag, std::uint64_t *paddingSize = nullptr, AbortableProgressFeedback *progress = nullptr);
    static constexpr std::uint32_t maximumIdLengthSupported();
    static constexpr std::uint32_t maximumSizeLengthSupported();
    static constexpr std::uint8_t minimumElementSize();
    template <typename TargetStream = std::ostream>
    void copyHeader(TargetStream &targetStream, Diagnostics &diag, AbortableProgressFeedback *progress);
    template <typename TargetStream = std::ostream>
    void copyWithoutChilds(TargetStream &targetStream, Diagnostics &diag, AbortableProgressFeedback *progress);
    template <typename TargetStream = std::ostream>
    void copyEntirely(TargetStream &targetStream, Diagnostics &diag, AbortableProgressFeedback *progress);
    void makeBuffer();
    void discardBuffer();
    template <typename TargetStream = std::ostream> void copyBuffer(TargetStream &targetStream);
    template <typename TargetStream = std::ostream>
    void copyPreferablyFromBuffer(TargetStream &targetStream, Diagnostics &diag, AbortableProgressFeedback *progress);
    const std::unique_ptr<char[]> &buffer();
    ImplementationType *denoteFirstChild(std::uint32_t offset);

protected:
    IdentifierType m_id;
    std::uint64_t m_startOffset;
    std::uint64_t m_maxSize;
    DataSizeType m_dataSize;
    std::uint32_t m_idLength;
    std::uint32_t m_sizeLength;
    ImplementationType *m_parent;
    std::unique_ptr<ImplementationType> m_nextSibling;
    std::unique_ptr<ImplementationType> m_firstChild;
    std::unique_ptr<char[]> m_buffer;

private:
    template <typename TargetStream = std::ostream>
    void copyInternal(
        TargetStream &targetStream, std::uint64_t startOffset, std::uint64_t bytesToCopy, Diagnostics &diag, AbortableProgressFeedback *progress);

    ContainerType *m_container;
    bool m_parsed;

protected:
    bool m_sizeUnknown;
};

/*!
 * \brief Constructs a new top level file element with the specified \a container at the specified \a startOffset.
 * \remarks The available size is obtained using the stream of the \a container.
 */
template <class ImplementationType>
GenericFileElement<ImplementationType>::GenericFileElement(
    GenericFileElement<ImplementationType>::ContainerType &container, std::uint64_t startOffset)
    : m_id(IdentifierType())
    , m_startOffset(startOffset)
    , m_dataSize(0)
    , m_idLength(0)
    , m_sizeLength(0)
    , m_parent(nullptr)
    , m_container(&container)
    , m_parsed(false)
    , m_sizeUnknown(false)
{
    m_maxSize = container.fileInfo().size();
    if (m_maxSize > startOffset) {
        m_maxSize -= startOffset;
        stream().seekg(static_cast<std::streamoff>(startOffset), std::ios_base::beg);
    } else {
        m_maxSize = 0;
    }
}

/*!
 * \brief Constructs a new sub level file element with the specified \a parent at the specified \a startOffset.
 */
template <class ImplementationType>
GenericFileElement<ImplementationType>::GenericFileElement(ImplementationType &parent, std::uint64_t startOffset)
    : m_id(IdentifierType())
    , m_startOffset(startOffset)
    , m_maxSize(parent.startOffset() + parent.totalSize() - startOffset)
    , m_dataSize(0)
    , m_idLength(0)
    , m_sizeLength(0)
    , m_parent(&parent)
    , m_container(&parent.container())
    , m_parsed(false)
    , m_sizeUnknown(false)
{
}

/*!
 * \brief Constructs a new sub level file element with the specified \a container, \a startOffset and \a maxSize.
 */
template <class ImplementationType>
GenericFileElement<ImplementationType>::GenericFileElement(
    GenericFileElement<ImplementationType>::ContainerType &container, std::uint64_t startOffset, std::uint64_t maxSize)
    : m_id(IdentifierType())
    , m_startOffset(startOffset)
    , m_maxSize(maxSize)
    , m_dataSize(0)
    , m_idLength(0)
    , m_sizeLength(0)
    , m_parent(nullptr)
    , m_container(&container)
    , m_parsed(false)
    , m_sizeUnknown(false)
{
}

/*!
 * \brief Returns the related container.
 */
template <class ImplementationType>
inline typename GenericFileElement<ImplementationType>::ContainerType &GenericFileElement<ImplementationType>::container()
{
    return *m_container;
}

/*!
 * \brief Returns the related container.
 */
template <class ImplementationType>
inline const typename GenericFileElement<ImplementationType>::ContainerType &GenericFileElement<ImplementationType>::container() const
{
    return *m_container;
}

/*!
 * \brief Returns the related stream.
 */
template <class ImplementationType> inline std::iostream &GenericFileElement<ImplementationType>::stream()
{
    return m_container->stream();
}

/*!
 * \brief Returns the related BinaryReader.
 */
template <class ImplementationType> inline CppUtilities::BinaryReader &GenericFileElement<ImplementationType>::reader()
{
    return m_container->reader();
}

/*!
 * \brief Returns the related BinaryWriter.
 */
template <class ImplementationType> inline CppUtilities::BinaryWriter &GenericFileElement<ImplementationType>::writer()
{
    return m_container->writer();
}

/*!
 * \brief Returns the start offset in the related stream.
 */
template <class ImplementationType> inline std::uint64_t GenericFileElement<ImplementationType>::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the offset of the element in its parent or - if it is a top-level element - in the related stream.
 */
template <class ImplementationType> inline std::uint64_t GenericFileElement<ImplementationType>::relativeStartOffset() const
{
    return parent() ? startOffset() - parent()->startOffset() : startOffset();
}

/*!
 * \brief Returns the element ID.
 */
template <class ImplementationType>
inline const typename GenericFileElement<ImplementationType>::IdentifierType &GenericFileElement<ImplementationType>::id() const
{
    return m_id;
}

/*!
 * \brief Returns a printable string representation of the element ID.
 */
template <class ImplementationType> inline std::string GenericFileElement<ImplementationType>::idToString() const
{
    return static_cast<ImplementationType *>(this)->idToString();
}

/*!
 * \brief Returns the length of the id denotation in byte.
 */
template <class ImplementationType> inline std::uint32_t GenericFileElement<ImplementationType>::idLength() const
{
    return m_idLength;
}

/*!
 * \brief Returns the header size of the element in byte.
 *
 * This is the sum of the id length and the size length.
 */
template <class ImplementationType> inline std::uint32_t GenericFileElement<ImplementationType>::headerSize() const
{
    return m_idLength + m_sizeLength;
}

/*!
 * \brief Returns the data size of the element in byte.
 *
 * This is the size of the element excluding the header.
 */
template <class ImplementationType>
inline typename GenericFileElement<ImplementationType>::DataSizeType GenericFileElement<ImplementationType>::dataSize() const
{
    return m_dataSize;
}

/*!
 * \brief Returns the length of the size denotation of the element in byte.
 */
template <class ImplementationType> inline std::uint32_t GenericFileElement<ImplementationType>::sizeLength() const
{
    return m_sizeLength;
}

/*!
 * \brief Returns the data offset of the element in the related stream.
 *
 * This is the sum of start offset and header size.
 */
template <class ImplementationType> inline std::uint64_t GenericFileElement<ImplementationType>::dataOffset() const
{
    return startOffset() + headerSize();
}

/*!
 * \brief Returns the total size of the element.
 *
 * This is the sum of the header size and the data size.
 */
template <class ImplementationType> inline std::uint64_t GenericFileElement<ImplementationType>::totalSize() const
{
    return headerSize() + dataSize();
}

/*!
 * \brief Returns the offset of the first byte which doesn't belong to this element anymore.
 */
template <class ImplementationType> inline std::uint64_t GenericFileElement<ImplementationType>::endOffset() const
{
    return startOffset() + totalSize();
}

/*!
 * \brief Returns maximum total size.
 *
 * This is usually the size of the file for top-level elements and
 * the remaining size of the parent for non-top-level elements.
 */
template <class ImplementationType> inline std::uint64_t GenericFileElement<ImplementationType>::maxTotalSize() const
{
    return m_maxSize;
}

/*!
 * \brief Returns how deep the element is nested (0 for top-level elements, 1 for children of
 *        top-level elements, ...).
 */
template <class ImplementationType> std::uint8_t GenericFileElement<ImplementationType>::level() const
{
    std::uint8_t level = 0;
    for (const ImplementationType *parent = m_parent; parent; ++level, parent = parent->m_parent)
        ;
    return level;
}

/*!
 * \brief Returns the parent of the element.
 *
 * The returned element has ownership over the current instance.
 * If the current element is a top level element nullptr is returned.
 */
template <class ImplementationType> inline ImplementationType *GenericFileElement<ImplementationType>::parent()
{
    return m_parent;
}

/*!
 * \brief Returns the parent of the element.
 *
 * The returned element has ownership over the current instance.
 * If the current element is a top level element nullptr is returned.
 */
template <class ImplementationType> inline const ImplementationType *GenericFileElement<ImplementationType>::parent() const
{
    return m_parent;
}

/*!
 * \brief Returns the n-th parent of the element.
 * \remarks
 * - The returned element has ownership (at least indirect) over the current instance.
 * - Returns nullptr if level() < \a n.
 */
template <class ImplementationType> ImplementationType *GenericFileElement<ImplementationType>::parent(std::uint8_t n)
{
    ImplementationType *parent = static_cast<ImplementationType *>(this);
    for (; n && parent; --n, parent = parent->m_parent)
        ;
    return parent;
}

/*!
 * \brief Returns the n-th parent of the element.
 * \remarks
 * - The returned element has ownership (at least indirect) over the current instance.
 * - Returns nullptr if level() < \a n.
 */
template <class ImplementationType> inline const ImplementationType *GenericFileElement<ImplementationType>::parent(std::uint8_t n) const
{
    return const_cast<GenericFileElement<ImplementationType> *>(this)->parent(n);
}

/*!
 * \brief Returns the next sibling of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no next sibling is present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType> inline ImplementationType *GenericFileElement<ImplementationType>::nextSibling()
{
    return m_nextSibling.get();
}

/*!
 * \brief Returns the next sibling of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no next sibling is present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType> inline const ImplementationType *GenericFileElement<ImplementationType>::nextSibling() const
{
    return m_nextSibling.get();
}

/*!
 * \brief Returns the first child of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no children are present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType> inline ImplementationType *GenericFileElement<ImplementationType>::firstChild()
{
    return m_firstChild.get();
}

/*!
 * \brief Returns the first child of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no children are present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType> inline const ImplementationType *GenericFileElement<ImplementationType>::firstChild() const
{
    return m_firstChild.get();
}

/*!
 * \brief Returns the last child of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no children are present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType> inline ImplementationType *GenericFileElement<ImplementationType>::lastChild()
{
    for (ImplementationType *child = firstChild(); child; child = child->nextSibling()) {
        if (!child->m_nextSibling) {
            return child;
        }
    }
    return nullptr;
}

/*!
 * \brief Returns the last child of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no children are present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType> inline const ImplementationType *GenericFileElement<ImplementationType>::lastChild() const
{
    return const_cast<GenericFileElement<ImplementationType> *>(this)->lastChild();
}

/*!
 * \brief Returns the sub element for the specified path.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
ImplementationType *GenericFileElement<ImplementationType>::subelementByPath(Diagnostics &diag, IdentifierType item)
{
    // ensure element is parsed
    parse(diag);
    // return the element if it matches the current and last item in the path
    if (item == id()) {
        return static_cast<ImplementationType *>(this);
    }
    // check whether a sibling matches the item
    if (nextSibling()) {
        return nextSibling()->subelementByPath(diag, item);
    }
    return nullptr;
}

/*!
 * \brief Returns the sub element for the specified path.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
ImplementationType *GenericFileElement<ImplementationType>::subelementByPath(Diagnostics &diag, IdentifierType item, IdentifierType remainingPath...)
{
    // ensure element is parsed
    parse(diag);
    // continue with next item in path if the element matches the current item
    if (item == id()) {
        if (!firstChild()) {
            return nullptr;
        }
        return firstChild()->subelementByPath(diag, remainingPath);
    }
    // check whether a sibling matches the current item
    if (nextSibling()) {
        return nextSibling()->subelementByPath(diag, item, remainingPath);
    }
    return nullptr;
}

/*!
 * \brief Returns the sub element for the specified path.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
const ImplementationType *GenericFileElement<ImplementationType>::subelementByPath(Diagnostics &diag, IdentifierType item) const
{
    return const_cast<GenericFileElement<ImplementationType> *>(this)->subelementByPath(diag, item);
}

/*!
 * \brief Returns the sub element for the specified path.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
const ImplementationType *GenericFileElement<ImplementationType>::subelementByPath(
    Diagnostics &diag, IdentifierType item, IdentifierType remainingPath...) const
{
    return const_cast<GenericFileElement<ImplementationType> *>(this)->subelementByPath(diag, item, remainingPath);
}

/*!
 * \brief Returns the first child with the specified \a id.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType> ImplementationType *GenericFileElement<ImplementationType>::childById(const IdentifierType &id, Diagnostics &diag)
{
    parse(diag); // ensure element is parsed
    for (ImplementationType *child = firstChild(); child; child = child->nextSibling()) {
        child->parse(diag);
        if (child->id() == id) {
            return child;
        }
    }
    return nullptr;
}

/*!
 * \brief Returns the first child with the specified \a id.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
const ImplementationType *GenericFileElement<ImplementationType>::childById(const IdentifierType &id, Diagnostics &diag) const
{
    return const_cast<GenericFileElement<ImplementationType> *>(this)->childById(id, diag);
}

/*!
 * \brief Returns the first sibling with the specified \a id.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 * Possibly returns a pointer to the current instance (see \a includeThis).
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
ImplementationType *GenericFileElement<ImplementationType>::siblingById(const IdentifierType &id, Diagnostics &diag)
{
    parse(diag); // ensure element is parsed
    for (ImplementationType *sibling = nextSibling(); sibling; sibling = sibling->nextSibling()) {
        sibling->parse(diag);
        if (sibling->id() == id) {
            return sibling;
        }
    }
    return nullptr;
}

/*!
 * \brief Returns the first sibling with the specified \a id.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 * Possibly returns a pointer to the current instance (see \a includeThis).
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
const ImplementationType *GenericFileElement<ImplementationType>::siblingById(const IdentifierType &id, Diagnostics &diag) const
{
    return const_cast<GenericFileElement<ImplementationType> *>(this)->siblingById(id, diag);
}

/*!
 * \brief Returns the first sibling with the specified \a id or the current instance if its ID equals \a id.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 * Possibly returns a pointer to the current instance (see \a includeThis).
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
ImplementationType *GenericFileElement<ImplementationType>::siblingByIdIncludingThis(const IdentifierType &id, Diagnostics &diag)
{
    parse(diag); // ensure element is parsed
    for (ImplementationType *sibling = static_cast<ImplementationType *>(this); sibling; sibling = sibling->nextSibling()) {
        sibling->parse(diag);
        if (sibling->id() == id) {
            return sibling;
        }
    }
    return nullptr;
}

/*!
 * \brief Returns the first sibling with the specified \a id or the current instance if its ID equals \a id.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 * Possibly returns a pointer to the current instance (see \a includeThis).
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
const ImplementationType *GenericFileElement<ImplementationType>::siblingByIdIncludingThis(const IdentifierType &id, Diagnostics &diag) const
{
    return const_cast<GenericFileElement<ImplementationType> *>(this)->siblingByIdIncludingThis(id, diag);
}

/*!
 * \brief Returns an indication whether this instance is a parent element.
 */
template <class ImplementationType> inline bool GenericFileElement<ImplementationType>::isParent() const
{
    return static_cast<const ImplementationType *>(this)->isParent();
}

/*!
 * \brief Returns an indication whether this instance is a padding element.
 */
template <class ImplementationType> inline bool GenericFileElement<ImplementationType>::isPadding() const
{
    return static_cast<const ImplementationType *>(this)->isPadding();
}

/*!
 * \brief Returns the offset of the first child (relative to the start offset of this element).
 */
template <class ImplementationType> inline std::uint64_t GenericFileElement<ImplementationType>::firstChildOffset() const
{
    return static_cast<const ImplementationType *>(this)->firstChildOffset();
}

/*!
 * \brief Returns an indication whether this instance has been parsed yet.
 */
template <class ImplementationType> inline bool GenericFileElement<ImplementationType>::isParsed() const
{
    return m_parsed;
}

/*!
 * \brief Clears the status of the element.
 *
 * Resets id length, data size, size length to zero. Subsequent elements
 * will be deleted.
 */
template <class ImplementationType> void GenericFileElement<ImplementationType>::clear()
{
    m_id = IdentifierType();
    //m_startOffset = 0;
    m_idLength = 0;
    m_dataSize = 0;
    m_sizeLength = 0;
    m_nextSibling = nullptr;
    m_firstChild = nullptr;
    m_parsed = false;
}

/*!
 * \brief Parses the header information of the element which is read from the related
 *        stream at the start offset.
 *
 * The parsed information can accessed using the corresponding methods such as
 * id() for the element id and totalSize() for the element size.
 *
 * If the element has already been parsed (isParsed() returns true) this method
 * does nothing. To force reparsing call reparse().
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
template <class ImplementationType> void GenericFileElement<ImplementationType>::parse(Diagnostics &diag)
{
    if (!m_parsed) {
        static_cast<ImplementationType *>(this)->internalParse(diag);
        m_parsed = true;
    }
}

/*!
 * \brief Parses the header information of the element which is read from the related
 *        stream at the start offset.
 *
 * The parsed information can accessed using the corresponding methods such as
 * id() for the element id and totalSize() for the element size.
 *
 * If the element has already been parsed (isParsed() returns true) this method
 * clears the parsed information and reparses the header.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \sa parse()
 */
template <class ImplementationType> void GenericFileElement<ImplementationType>::reparse(Diagnostics &diag)
{
    clear();
    static_cast<ImplementationType *>(this)->parse(diag);
    m_parsed = true;
}

/*!
 * \brief Parses (see parse()) this and all subsequent elements.
 *
 * All diagnostic message will be stored in \a diag.
 * If padding is found its size will be set to \a paddingSize if not nullptr.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \sa parse()
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::validateSubsequentElementStructure(
    Diagnostics &diag, std::uint64_t *paddingSize, AbortableProgressFeedback *progress)
{
    if (progress) {
        progress->stopIfAborted();
    }
    // validate element itself by just parsing it
    parse(diag);
    // validate children
    if (firstChild()) {
        try {
            firstChild()->validateSubsequentElementStructure(diag, paddingSize, progress);
        } catch (const Failure &) {
            // ignore critical errors in child structure to continue validating siblings
            // (critical notifications about the errors should have already been added to diag, so nothing to do)
        }
    } else if (paddingSize && isPadding()) { // element is padding
        *paddingSize += totalSize();
    }
    // validate siblings
    if (nextSibling()) {
        nextSibling()->validateSubsequentElementStructure(diag, paddingSize, progress);
    }
}

/*!
 * \brief Writes the header information of the element to the specified \a targetStream.
 */
template <class ImplementationType>
template <typename TargetStream>
void GenericFileElement<ImplementationType>::copyHeader(TargetStream &targetStream, Diagnostics &diag, AbortableProgressFeedback *progress)
{
    copyInternal(targetStream, startOffset(), headerSize(), diag, progress);
}

/*!
 * \brief Writes the element without its children to the specified \a targetStream.
 */
template <class ImplementationType>
template <typename TargetStream>
void GenericFileElement<ImplementationType>::copyWithoutChilds(TargetStream &targetStream, Diagnostics &diag, AbortableProgressFeedback *progress)
{
    if (std::uint32_t firstChildOffset = this->firstChildOffset()) {
        copyInternal(targetStream, startOffset(), firstChildOffset, diag, progress);
    } else {
        copyInternal(targetStream, startOffset(), totalSize(), diag, progress);
    }
}

/*!
 * \brief Writes the entire element including all children to the specified \a targetStream.
 */
template <class ImplementationType>
template <typename TargetStream>
void GenericFileElement<ImplementationType>::copyEntirely(TargetStream &targetStream, Diagnostics &diag, AbortableProgressFeedback *progress)
{
    copyInternal(targetStream, startOffset(), totalSize(), diag, progress);
}

/*!
 * \brief Buffers the element (header and data).
 * \remarks The element must have been parsed.
 */
template <class ImplementationType> void GenericFileElement<ImplementationType>::makeBuffer()
{
    m_buffer = std::make_unique<char[]>(totalSize());
    container().stream().seekg(static_cast<std::streamoff>(startOffset()));
    container().stream().read(m_buffer.get(), static_cast<std::streamsize>(totalSize()));
}

/*!
 * \brief Discards buffered data.
 */
template <class ImplementationType> inline void GenericFileElement<ImplementationType>::discardBuffer()
{
    m_buffer.reset();
}

/*!
 * \brief Copies buffered data to \a targetStream.
 * \remarks Data must have been buffered using the makeBuffer() method.
 */
template <class ImplementationType>
template <typename TargetStream>
inline void GenericFileElement<ImplementationType>::copyBuffer(TargetStream &targetStream)
{
    targetStream.write(m_buffer.get(), static_cast<std::streamsize>(totalSize()));
}

/*!
 * \brief Copies buffered data to \a targetStream if data has been buffered; copies from input stream otherwise.
 * \remarks So this is copyBuffer() with a fallback to copyEntirely().
 */
template <class ImplementationType>
template <typename TargetStream>
inline void GenericFileElement<ImplementationType>::copyPreferablyFromBuffer(
    TargetStream &targetStream, Diagnostics &diag, AbortableProgressFeedback *progress)
{
    m_buffer ? copyBuffer(targetStream) : copyEntirely(targetStream, diag, progress);
}

/*!
 * \brief Returns buffered data. The returned array is totalSize() bytes long.
 * \remarks Data must have been buffered using the makeBuffer() method.
 */
template <class ImplementationType> inline const std::unique_ptr<char[]> &GenericFileElement<ImplementationType>::buffer()
{
    return m_buffer;
}

/*!
 * \brief Internally used to perform copies of the atom.
 *
 * \sa copyHeaderToStream()
 * \sa copyAtomWithoutChildsToStream()
 * \sa copyEntireAtomToStream()
 */
template <class ImplementationType>
template <typename TargetStream>
void GenericFileElement<ImplementationType>::copyInternal(
    TargetStream &targetStream, std::uint64_t startOffset, std::uint64_t bytesToCopy, Diagnostics &diag, AbortableProgressFeedback *progress)
{
    // ensure the header has been parsed correctly
    try {
        parse(diag);
    } catch (const Failure &) {
        throw InvalidDataException();
    }
    auto &stream = container().stream();
    stream.seekg(static_cast<std::streamoff>(startOffset), std::ios_base::beg);
    CppUtilities::CopyHelper<0x10000> copyHelper;
    if (progress) {
        copyHelper.callbackCopy(stream, targetStream, bytesToCopy, std::bind(&AbortableProgressFeedback::isAborted, std::ref(progress)),
            std::bind(&AbortableProgressFeedback::updateStepPercentageFromFraction, std::ref(progress), std::placeholders::_1));
    } else {
        copyHelper.copy(stream, targetStream, bytesToCopy);
    }
}

/*!
 * \brief Denotes the first child to start at the specified \a offset (relative to the start offset of this descriptor).
 * \remarks A new first child is constructed. A possibly existing subtree is invalidated.
 */
template <class ImplementationType>
ImplementationType *GenericFileElement<ImplementationType>::denoteFirstChild(std::uint32_t relativeFirstChildOffset)
{
    if (relativeFirstChildOffset + minimumElementSize() <= totalSize()) {
        m_firstChild.reset(new ImplementationType(static_cast<ImplementationType &>(*this), startOffset() + relativeFirstChildOffset));
    } else {
        m_firstChild.reset();
    }
    return m_firstChild.get();
}

/*!
 * \brief Returns the maximum id length supported by the class in byte.
 */
template <class ImplementationType> constexpr std::uint32_t GenericFileElement<ImplementationType>::maximumIdLengthSupported()
{
    return sizeof(IdentifierType);
}

/*!
 * \brief Returns the maximum size length supported by the class in byte.
 */
template <class ImplementationType> constexpr std::uint32_t GenericFileElement<ImplementationType>::maximumSizeLengthSupported()
{
    return sizeof(DataSizeType);
}

/*!
 * \brief Returns the minimum element size.
 */
template <class ImplementationType> constexpr std::uint8_t GenericFileElement<ImplementationType>::minimumElementSize()
{
    return FileElementTraits<ImplementationType>::minimumElementSize();
}

/*!
 * \fn GenericFileElement<ImplementationType>::internalParse()
 * \brief This method is called to perform parsing.
 *
 * It needs to be implemented when subclassing.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \sa parse()
 * \sa reparse()
 */

} // namespace TagParser

#endif // TAG_PARSER_GENERICFILEELEMENT_H
