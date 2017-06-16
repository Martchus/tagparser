#ifndef GENERICFILEELEMENT_H
#define GENERICFILEELEMENT_H

#include "./notification.h"
#include "./exceptions.h"
#include "./statusprovider.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/io/copy.h>

#include <list>
#include <initializer_list>
#include <memory>
#include <iostream>
#include <string>

namespace IoUtilities {

class BinaryReader;
class BinaryWriter;

}

namespace Media {

template <class ImplementationType>
class GenericFileElement;

/*!
 * \class Media::FileElementIterator
 * \brief The FileElementIterator class helps iterating through the children of a FileElement.
 */
template<typename ImplementationType>
class FileElementIterator
{
public:
    FileElementIterator(ImplementationType *element = nullptr);

    ImplementationType *operator *();
    const ImplementationType *operator *() const;
    operator bool() const;
    FileElementIterator<ImplementationType> &operator ++();

private:
    ImplementationType *m_current;
};

/*!
 * \brief Constructs a new iterator for the specified \a element.
 */
template<typename ImplementationType>
inline FileElementIterator<ImplementationType>::FileElementIterator(ImplementationType *element) :
    m_current(element)
{}

/*!
 * \brief Returns a reference to the current element.
 */
template<typename ImplementationType>
inline ImplementationType *FileElementIterator<ImplementationType>::operator *()
{
    return m_current;
}

/*!
 * \brief Returns a reference the current element (constant).
 */
template<typename ImplementationType>
inline const ImplementationType *FileElementIterator<ImplementationType>::operator *() const
{
    return m_current;
}

/*!
 * \brief Moves to the next sibling.
 */
template<typename ImplementationType>
inline FileElementIterator<ImplementationType> &FileElementIterator<ImplementationType>::operator ++()
{
    m_current->parse(); // ensure the current element has been parsed
    m_current = m_current->nextSibling();
    return *this;
}

/*!
 * \brief Returns whether the iterator points to an element.
 */
template<typename ImplementationType>
inline FileElementIterator<ImplementationType>::operator bool() const
{
    return m_current != nullptr;
}

/*!
 * \class Media::FileElementTraits
 * \brief Defines traits for the specified \a ImplementationType.
 *
 * A template specialization for each GenericFileElement should
 * be provided.
 *
 * For an example of such a specialization see FileElementTraits<Mp4Atom> or FileElementTraits<EbmlElement>.
 */
template<typename ImplementationType>
class FileElementTraits
{};

/*!
 * \class Media::GenericFileElement
 * \brief The GenericFileElement class helps to parse binary files which consist
 *        of an arboreal element strucutre.
 *
 * \tparam ImplementationType Specifies the type of the actual implementation.
 */
template <class ImplementationType>
class TAG_PARSER_EXPORT GenericFileElement : public StatusProvider
{
    friend class FileElementTraits<ImplementationType>;

public:
    /*!
     * \brief Specifies the type of the corresponding container.
     */
    typedef typename FileElementTraits<ImplementationType>::containerType containerType;

    /*!
     * \brief Specifies the type used to store identifiers.
     */
    typedef typename FileElementTraits<ImplementationType>::identifierType identifierType;

    /*!
     * \brief Specifies the type used to store data sizes.
     */
    typedef typename FileElementTraits<ImplementationType>::dataSizeType dataSizeType;

    /*!
     * \brief Specifies the type of the actual implementation.
     */
    typedef typename FileElementTraits<ImplementationType>::implementationType implementationType;

    GenericFileElement(containerType &container, uint64 startOffset);
    GenericFileElement(implementationType &parent, uint64 startOffset);
    GenericFileElement(containerType &container, uint64 startOffset, uint64 maxSize);
    GenericFileElement(const GenericFileElement& other) = delete;
    GenericFileElement(GenericFileElement& other) = delete;
    GenericFileElement& operator =(const GenericFileElement& other) = delete;

    containerType& container();
    const containerType& container() const;
    std::iostream &stream();
    IoUtilities::BinaryReader &reader();
    IoUtilities::BinaryWriter &writer();
    uint64 startOffset() const;
    uint64 relativeStartOffset() const;
    const identifierType &id() const;
    std::string idToString() const;
    uint32 idLength() const;
    uint32 headerSize() const;
    dataSizeType dataSize() const;
    uint32 sizeLength() const;
    uint64 dataOffset() const;
    uint64 totalSize() const;
    uint64 endOffset() const;
    uint64 maxTotalSize() const;
    implementationType* parent();
    const implementationType* parent() const;
    implementationType* nextSibling();
    const implementationType* nextSibling() const;
    implementationType* firstChild();
    const implementationType* firstChild() const;
    implementationType* subelementByPath(const std::initializer_list<identifierType> &path);
    implementationType* subelementByPath(std::list<identifierType> &path);
    implementationType* childById(const identifierType &id);
    implementationType* siblingById(const identifierType &id, bool includeThis = false);
    FileElementIterator<implementationType> begin();
    FileElementIterator<implementationType> end();
    const FileElementIterator<implementationType> begin() const;
    const FileElementIterator<implementationType> end() const;
    bool isParent() const;
    bool isPadding() const;
    uint64 firstChildOffset() const;
    bool isParsed() const;
    void clear();
    void parse();
    void reparse();
    void validateSubsequentElementStructure(NotificationList &gatheredNotifications, uint64 *paddingSize = nullptr);
    static constexpr uint32 maximumIdLengthSupported();
    static constexpr uint32 maximumSizeLengthSupported();    
    static constexpr byte minimumElementSize();
    void copyHeader(std::ostream &targetStream);
    void copyWithoutChilds(std::ostream &targetStream);
    void copyEntirely(std::ostream &targetStream);
    void makeBuffer();
    void discardBuffer();
    void copyBuffer(std::ostream &targetStream);
    void copyPreferablyFromBuffer(std::ostream &targetStream);
    const std::unique_ptr<char[]> &buffer();
    implementationType *denoteFirstChild(uint32 offset);

protected:
    identifierType m_id;
    uint64 m_startOffset;
    uint64 m_maxSize;
    uint32 m_idLength;
    dataSizeType m_dataSize;
    uint32 m_sizeLength;
    implementationType* m_parent;
    std::unique_ptr<implementationType> m_nextSibling;
    std::unique_ptr<implementationType> m_firstChild;
    std::unique_ptr<char[]> m_buffer;

private:
    void copyInternal(std::ostream &targetStream, uint64 startOffset, uint64 bytesToCopy);

    containerType* m_container;
    bool m_parsed;
};

/*!
 * \brief Constructs a new top level file element with the specified \a container at the specified \a startOffset.
 * \remarks The available size is obtained using the stream of the \a container.
 */
template <class ImplementationType>
GenericFileElement<ImplementationType>::GenericFileElement(GenericFileElement<ImplementationType>::containerType &container, uint64 startOffset) :
    m_id(identifierType()),
    m_startOffset(startOffset),
    m_idLength(0),
    m_dataSize(0),
    m_sizeLength(0),
    m_parent(nullptr),
    m_container(&container),
    m_parsed(false)
{
    stream().seekg(0, std::ios_base::end);
    m_maxSize = static_cast<uint64>(stream().tellg());
    if(m_maxSize > startOffset) {
        m_maxSize -= startOffset;
        stream().seekg(startOffset, std::ios_base::beg);
    } else {
        m_maxSize = 0;
    }
}

/*!
 * \brief Constructs a new sub level file element with the specified \a parent at the specified \a startOffset.
 */
template <class ImplementationType>
GenericFileElement<ImplementationType>::GenericFileElement(GenericFileElement<ImplementationType>::implementationType &parent, uint64 startOffset) :
    m_id(identifierType()),
    m_startOffset(startOffset),
    m_maxSize(parent.startOffset() + parent.totalSize() - startOffset),
    m_idLength(0),
    m_dataSize(0),
    m_sizeLength(0),
    m_parent(&parent),
    m_container(&parent.container()),
    m_parsed(false)
{}

/*!
 * \brief Constructs a new sub level file element with the specified \a container, \a startOffset and \a maxSize.
 */
template <class ImplementationType>
GenericFileElement<ImplementationType>::GenericFileElement(GenericFileElement<ImplementationType>::containerType &container, uint64 startOffset, uint64 maxSize) :
    m_id(identifierType()),
    m_startOffset(startOffset),
    m_maxSize(maxSize),
    m_idLength(0),
    m_dataSize(0),
    m_sizeLength(0),
    m_parent(nullptr),
    m_container(&container),
    m_parsed(false)
{}

/*!
 * \brief Returns the related container.
 */
template <class ImplementationType>
inline typename GenericFileElement<ImplementationType>::containerType& GenericFileElement<ImplementationType>::container()
{
    return *m_container;
}

/*!
 * \brief Returns the related container.
 */
template <class ImplementationType>
inline const typename GenericFileElement<ImplementationType>::containerType &GenericFileElement<ImplementationType>::container() const
{
    return *m_container;
}

/*!
 * \brief Returns the related stream.
 */
template <class ImplementationType>
inline std::iostream &GenericFileElement<ImplementationType>::stream()
{
    return m_container->stream();
}

/*!
 * \brief Returns the related BinaryReader.
 */
template <class ImplementationType>
inline IoUtilities::BinaryReader &GenericFileElement<ImplementationType>::reader()
{
    return m_container->reader();
}

/*!
 * \brief Returns the related BinaryWriter.
 */
template <class ImplementationType>
inline IoUtilities::BinaryWriter &GenericFileElement<ImplementationType>::writer()
{
    return m_container->writer();
}

/*!
 * \brief Returns the start offset in the related stream.
 */
template <class ImplementationType>
inline uint64 GenericFileElement<ImplementationType>::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the offset of the element in its parent or - if it is a top-level element - in the related stream.
 */
template <class ImplementationType>
inline uint64 GenericFileElement<ImplementationType>::relativeStartOffset() const
{
    return parent() ? startOffset() - parent()->startOffset() : startOffset();
}

/*!
 * \brief Returns the element ID.
 */
template <class ImplementationType>
inline const typename GenericFileElement<ImplementationType>::identifierType &GenericFileElement<ImplementationType>::id() const
{
    return m_id;
}

/*!
 * \brief Returns a printable string representation of the element ID.
 */
template <class ImplementationType>
inline std::string GenericFileElement<ImplementationType>::idToString() const
{
    return static_cast<ImplementationType *>(this)->idToString();
}

/*!
 * \brief Returns the length of the id denotation in byte.
 */
template <class ImplementationType>
inline uint32 GenericFileElement<ImplementationType>::idLength() const
{
    return m_idLength;
}

/*!
 * \brief Returns the header size of the element in byte.
 *
 * This is the sum of the id length and the size length.
 */
template <class ImplementationType>
inline uint32 GenericFileElement<ImplementationType>::headerSize() const
{
    return m_idLength + m_sizeLength;
}

/*!
 * \brief Returns the data size of the element in byte.
 *
 * This is the size of the element excluding the header.
 */
template <class ImplementationType>
inline typename GenericFileElement<ImplementationType>::dataSizeType GenericFileElement<ImplementationType>::dataSize() const
{
    return m_dataSize;
}

/*!
 * \brief Returns the length of the size denotation of the element in byte.
 */
template <class ImplementationType>
inline uint32 GenericFileElement<ImplementationType>::sizeLength() const
{
    return m_sizeLength;
}

/*!
 * \brief Returns the data offset of the element in the related stream.
 *
 * This is the sum of start offset and header size.
 */
template <class ImplementationType>
inline uint64 GenericFileElement<ImplementationType>::dataOffset() const
{
    return startOffset() + headerSize();
}

/*!
 * \brief Returns the total size of the element.
 *
 * This is the sum of the header size and the data size.
 */
template <class ImplementationType>
inline uint64 GenericFileElement<ImplementationType>::totalSize() const
{
    return headerSize() + dataSize();
}

/*!
 * \brief Returns the offset of the first byte which doesn't belong to this element anymore.
 */
template <class ImplementationType>
inline uint64 GenericFileElement<ImplementationType>::endOffset() const
{
    return startOffset() + totalSize();
}

/*!
 * \brief Returns maximum total size.
 *
 * This is usually the size of the file for top-level elements and
 * the remaining size of the parent for non-top-level elements.
 */
template <class ImplementationType>
inline uint64 GenericFileElement<ImplementationType>::maxTotalSize() const
{
    return m_maxSize;
}

/*!
 * \brief Returns the parent of the element.
 *
 * The returned element has ownership over the current instance.
 * If the current element is a top level element nullptr is returned.
 */
template <class ImplementationType>
inline typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::parent()
{
    return m_parent;
}

/*!
 * \brief Returns the parent of the element.
 *
 * The returned element has ownership over the current instance.
 * If the current element is a top level element nullptr is returned.
 */
template <class ImplementationType>
inline const typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::parent() const
{
    return m_parent;
}

/*!
 * \brief Returns the next sibling of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no next sibling is present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType>
inline typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::nextSibling()
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
template <class ImplementationType>
inline const typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::nextSibling() const
{
    return m_nextSibling.get();
}

/*!
 * \brief Returns the first child of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no childs are present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType>
inline typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::firstChild()
{
    return m_firstChild.get();
}

/*!
 * \brief Returns the first child of the element.
 *
 * The current element keeps ownership over the returned element.
 * If no childs are present nullptr is returned.
 *
 * \remarks parse() needs to be called before.
 */
template <class ImplementationType>
inline const typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::firstChild() const
{
    return m_firstChild.get();
}

/*!
 * \brief Returns the sub element for the specified \a path.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
inline typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::subelementByPath(const std::initializer_list<identifierType> &path)
{
    std::list<GenericFileElement<ImplementationType>::identifierType> list(path);
    return subelementByPath(list);
}

/*!
 * \brief Returns the sub element for the specified \a path.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 * The specified \a path will modified.
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::subelementByPath(std::list<GenericFileElement<ImplementationType>::identifierType> &path)
{
    parse(); // ensure element is parsed
    if(path.size()) {
        if(path.front() == id()) {
            if(path.size() == 1) {
                return static_cast<implementationType*>(this);
            } else {
                if(firstChild()) {
                    path.pop_front();
                    return firstChild()->subelementByPath(path);
                }
            }
        } else {
            if(nextSibling()) {
                return nextSibling()->subelementByPath(path);
            }
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
typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::childById(const GenericFileElement<ImplementationType>::identifierType &id)
{
    parse(); // ensure element is parsed
    for(implementationType *child = firstChild(); child; child = child->nextSibling()) {
        child->parse();
        if(child->id() == id) {
            return child;
        }
    }
    return nullptr;
}

/*!
 * \brief Returns the first sibling with the specified \a id.
 *
 * \param id Specifies the id of the sibling to be returned.
 * \param includeThis Indicates whether this instance should be returned
 *                    if it has the specified \a id.
 *
 * The current element keeps ownership over the returned element.
 * If no element could be found nullptr is returned.
 * Possibly returns a pointer to the current instance (see \a includeThis).
 *
 * \throws Throws a parsing exception when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class ImplementationType>
typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::siblingById(const GenericFileElement<ImplementationType>::identifierType &id, bool includeThis)
{
    parse(); // ensure element is parsed
    for(implementationType *sibling = includeThis ? static_cast<implementationType*>(this) : nextSibling(); sibling; sibling = sibling->nextSibling()) {
        sibling->parse();
        if(sibling->id() == id) {
            return sibling;
        }
    }
    return nullptr;
}

/*!
 * \brief Returns an iterator for iterating over the element's childs.
 */
template <class ImplementationType>
FileElementIterator<typename GenericFileElement<ImplementationType>::implementationType> GenericFileElement<ImplementationType>::begin()
{
    return FileElementIterator<implementationType>(firstChild());
}

/*!
 * \brief Returns an iterator for iterating over the element's childs (constant).
 */
template <class ImplementationType>
const FileElementIterator<typename GenericFileElement<ImplementationType>::implementationType> GenericFileElement<ImplementationType>::begin() const
{
    return FileElementIterator<implementationType>(firstChild());
}

/*!
 * \brief Returns an invalid iterator.
 */
template <class ImplementationType>
FileElementIterator<typename GenericFileElement<ImplementationType>::implementationType> GenericFileElement<ImplementationType>::end()
{
    return FileElementIterator<ImplementationType>();
}

/*!
 * \brief Returns an invalid iterator.
 */
template <class ImplementationType>
const FileElementIterator<typename GenericFileElement<ImplementationType>::implementationType> GenericFileElement<ImplementationType>::end() const
{
    return FileElementIterator<ImplementationType>();
}

/*!
 * \brief Returns an indication whether this instance is a parent element.
 */
template <class ImplementationType>
inline bool GenericFileElement<ImplementationType>::isParent() const
{
    return static_cast<const ImplementationType *>(this)->isParent();
}

/*!
 * \brief Returns an indication whether this instance is a padding element.
 */
template <class ImplementationType>
inline bool GenericFileElement<ImplementationType>::isPadding() const
{
    return static_cast<const ImplementationType *>(this)->isPadding();
}

/*!
 * \brief Returns the offset of the first child (relative to the start offset of this element).
 */
template <class ImplementationType>
inline uint64 GenericFileElement<ImplementationType>::firstChildOffset() const
{
    return static_cast<const ImplementationType *>(this)->firstChildOffset();
}

/*!
 * \brief Returns an indication whether this instance has been parsed yet.
 */
template <class ImplementationType>
inline bool GenericFileElement<ImplementationType>::isParsed() const
{
    return m_parsed;
}

/*!
 * \brief Clears the status of the element.
 *
 * Resets id length, data size, size length to zero. Subsequent elements
 * will be deleted.
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::clear()
{
    m_id = identifierType();
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
 * id() for the elemement id and totalSize() for the element size.
 *
 * If the element has already been parsed (isParsed() returns true) this method
 * does nothing. To force reparsing call reparse().
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::parse()
{
    if(!m_parsed) {
        static_cast<ImplementationType *>(this)->internalParse();
        m_parsed = true;
    }
}

/*!
 * \brief Parses the header information of the element which is read from the related
 *        stream at the start offset.
 *
 * The parsed information can accessed using the corresponding methods such as
 * id() for the elemement id and totalSize() for the element size.
 *
 * If the element has already been parsed (isParsed() returns true) this method
 * clears the parsed information and reparses the header.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \sa parse()
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::reparse()
{
    clear();
    static_cast<ImplementationType *>(this)->parse();
    m_parsed = true;
}

/*!
 * \brief Parses (see parse()) this and all subsequent elements.
 *
 * All parsing notifications will be stored in \a gatheredNotifications.
 * If padding is found its size will be set to \a paddingSize if not nullptr.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \sa parse()
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::validateSubsequentElementStructure(NotificationList &gatheredNotifications, uint64 *paddingSize)
{
    try {
        parse();
        gatheredNotifications.insert(gatheredNotifications.end(), notifications().begin(), notifications().end());
        if(firstChild()) { // element is parent
            firstChild()->validateSubsequentElementStructure(gatheredNotifications, paddingSize);
        } else if(paddingSize && isPadding()) { // element is padding
            *paddingSize += totalSize();
        }
        if(nextSibling()) {
            nextSibling()->validateSubsequentElementStructure(gatheredNotifications, paddingSize);
        }
    } catch(Failure &) {
        gatheredNotifications.insert(gatheredNotifications.end(), notifications().begin(), notifications().end());
        throw;
    }
}

/*!
 * \brief Writes the header informaton of the element to the specified \a targetStream.
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::copyHeader(std::ostream &targetStream)
{
    copyInternal(targetStream, startOffset(), headerSize());
}

/*!
 * \brief Writes the element without its childs to the specified \a targetStream.
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::copyWithoutChilds(std::ostream &targetStream)
{
    if(uint32 firstChildOffset = this->firstChildOffset()) {
        copyInternal(targetStream, startOffset(), firstChildOffset);
    } else {
        copyInternal(targetStream, startOffset(), totalSize());
    }
}

/*!
 * \brief Writes the entire element including all childs to the specified \a targetStream.
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::copyEntirely(std::ostream &targetStream)
{
    copyInternal(targetStream, startOffset(), totalSize());
}

/*!
 * \brief Buffers the element (header and data).
 * \remarks The element must have been parsed.
 */
template <class ImplementationType>
void GenericFileElement<ImplementationType>::makeBuffer()
{
    m_buffer = std::make_unique<char[]>(totalSize());
    container().stream().seekg(startOffset());
    container().stream().read(m_buffer.get(), totalSize());
}

/*!
 * \brief Discards buffered data.
 */
template <class ImplementationType>
inline void GenericFileElement<ImplementationType>::discardBuffer()
{
    m_buffer.reset();
}

/*!
 * \brief Copies buffered data to \a targetStream.
 * \remarks Data must have been buffered using the makeBuffer() method.
 */
template <class ImplementationType>
inline void GenericFileElement<ImplementationType>::copyBuffer(std::ostream &targetStream)
{
    targetStream.write(m_buffer.get(), totalSize());
}

/*!
 * \brief Copies buffered data to \a targetStream if data has been buffered; copies from input stream otherwise.
 * \remarks So this is copyBuffer() with a fallback to copyEntirely().
 */
template <class ImplementationType>
inline void GenericFileElement<ImplementationType>::copyPreferablyFromBuffer(std::ostream &targetStream)
{
    m_buffer ? copyBuffer(targetStream) : copyEntirely(targetStream);
}

/*!
 * \brief Returns buffered data. The returned array is totalSize() bytes long.
 * \remarks Data must have been buffered using the makeBuffer() method.
 */
template <class ImplementationType>
inline const std::unique_ptr<char[]> &GenericFileElement<ImplementationType>::buffer()
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
void GenericFileElement<ImplementationType>::copyInternal(std::ostream &targetStream, uint64 startOffset, uint64 bytesToCopy)
{
    invalidateStatus();
    // ensure the header has been parsed correctly
    try {
        parse();
    } catch(Failure &) {
        throw InvalidDataException();
    }
    auto &stream = container().stream();
    stream.seekg(startOffset); // seek to start offset
    IoUtilities::CopyHelper<0x2000> copyHelper;
    copyHelper.callbackCopy(stream, targetStream, bytesToCopy, std::bind(&GenericFileElement<ImplementationType>::isAborted, this), std::bind(&GenericFileElement<ImplementationType>::updatePercentage, this, std::placeholders::_1));
    if(isAborted()) {
        throw OperationAbortedException();
    }
}

/*!
 * \brief Denotes the first child to start at the specified \a offset (relative to the start offset of this descriptor).
 * \remarks A new first child is constructed. A possibly existing subtree is invalidated.
 */
template <class ImplementationType>
typename GenericFileElement<ImplementationType>::implementationType *GenericFileElement<ImplementationType>::denoteFirstChild(uint32 relativeFirstChildOffset)
{
    if(relativeFirstChildOffset + minimumElementSize() <= totalSize()) {
        m_firstChild.reset(new implementationType(static_cast<implementationType &>(*this), startOffset() + relativeFirstChildOffset));
    } else {
        m_firstChild.reset();
    }
    return m_firstChild.get();
}

/*!
 * \brief Returns the maximum id length supported by the class in byte.
 */
template <class ImplementationType>
inline constexpr uint32 GenericFileElement<ImplementationType>::maximumIdLengthSupported()
{
    return sizeof(identifierType);
}

/*!
 * \brief Returns the maximum size length supported by the class in byte.
 */
template <class ImplementationType>
inline constexpr uint32 GenericFileElement<ImplementationType>::maximumSizeLengthSupported()
{
    return sizeof(dataSizeType);
}

/*!
 * \brief Returns the mimimum element size.
 */
template <class ImplementationType>
inline constexpr byte GenericFileElement<ImplementationType>::minimumElementSize()
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
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 *
 * \sa parse()
 * \sa reparse()
 */

}

#endif // GENERICFILEELEMENT_H
