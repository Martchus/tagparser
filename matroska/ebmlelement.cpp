#include "./ebmlelement.h"
#include "./ebmlid.h"
#include "./matroskacontainer.h"
#include "./matroskaid.h"

#include "../exceptions.h"
#include "../mediafileinfo.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <cstdint>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::EbmlElement
 * \brief The EbmlElement class helps to parse EBML files such as Matroska files.
 */

/*!
 * \brief Specifies the number of bytes to be skipped till a valid EBML element is found in the stream.
 */
std::uint64_t EbmlElement::bytesToBeSkipped = 0x4000;

/*!
 * \brief Constructs a new top level element with the specified \a container at the specified \a startOffset.
 */
EbmlElement::EbmlElement(MatroskaContainer &container, std::uint64_t startOffset)
    : GenericFileElement<EbmlElement>(container, startOffset)
{
}

/*!
 * \brief Constructs a new top level element with the specified \a container at the specified \a startOffset.
 */
EbmlElement::EbmlElement(MatroskaContainer &container, std::uint64_t startOffset, std::uint64_t maxSize)
    : GenericFileElement<EbmlElement>(container, startOffset, maxSize)
{
}

/*!
 * \brief Constructs a new sub level element with the specified \a parent at the specified \a startOffset.
 */
EbmlElement::EbmlElement(EbmlElement &parent, std::uint64_t startOffset)
    : GenericFileElement<EbmlElement>(parent, startOffset)
{
}

/*!
 * \brief Returns the parsing context.
 */
string EbmlElement::parsingContext() const
{
    return ("parsing header of EBML element " % idToString() % " at ") + startOffset();
}

/*!
 * \brief Parses the EBML element.
 */
void EbmlElement::internalParse(Diagnostics &diag)
{
    static const string context("parsing EBML element header");

    for (std::uint64_t skipped = 0; skipped < bytesToBeSkipped; ++m_startOffset, --m_maxSize, ++skipped) {
        // check whether max size is valid
        if (maxTotalSize() < 2) {
            diag.emplace_back(DiagLevel::Critical, argsToString("The EBML element at ", startOffset(), " is truncated or does not exist."), context);
            throw TruncatedDataException();
        }
        stream().seekg(static_cast<streamoff>(startOffset()));

        // read ID
        char buf[maximumIdLengthSupported() > maximumSizeLengthSupported() ? maximumIdLengthSupported() : maximumSizeLengthSupported()] = { 0 };
        std::uint8_t beg = static_cast<std::uint8_t>(stream().peek()), mask = 0x80;
        m_idLength = 1;
        while (m_idLength <= maximumIdLengthSupported() && (beg & mask) == 0) {
            ++m_idLength;
            mask >>= 1;
        }
        if (m_idLength > maximumIdLengthSupported()) {
            if (!skipped) {
                diag.emplace_back(
                    DiagLevel::Critical, argsToString("EBML ID length at ", startOffset(), " is not supported, trying to skip."), context);
            }
            continue; // try again
        }
        if (m_idLength > container().maxIdLength()) {
            if (!skipped) {
                diag.emplace_back(DiagLevel::Critical, argsToString("EBML ID length at ", startOffset(), " is invalid, trying to skip."), context);
            }
            continue; // try again
        }
        reader().read(buf + (maximumIdLengthSupported() - m_idLength), m_idLength);
        m_id = BE::toInt<std::uint32_t>(buf);

        // check whether this element is actually a sibling of one of its parents rather then a child
        // (might be the case if the parent's size is unknown and hence assumed to be the max file size)
        if (m_parent && m_parent->m_sizeUnknown) {
            // check at which level in the hierarchy the element is supposed to occur using its ID
            // (the only chance to find out whether the element belongs higher up in the hierarchy)
            const MatroskaElementLevel supposedLevel = matroskaIdLevel(m_id);
            const std::uint8_t actualLevel = level();
            if (actualLevel > supposedLevel) {
                // the file belongs higher up in the hierarchy so find a better parent
                if (EbmlElement *betterParent = m_parent->parent(actualLevel - static_cast<std::uint8_t>(supposedLevel))) {
                    // recompute the parent size (assumption - which was rest of the available space - was wrong)
                    m_parent->m_dataSize = m_startOffset - m_parent->m_startOffset - m_parent->headerSize();
                    m_parent->m_sizeUnknown = false;
                    // detach from ...
                    if (m_parent->firstChild() == this) {
                        // ... parent
                        m_parent->m_firstChild.release();
                        m_parent->m_firstChild = std::move(m_nextSibling);
                    } else {
                        // ... previous sibling
                        for (EbmlElement *sibling = m_parent->firstChild(); sibling; sibling = sibling->nextSibling()) {
                            if (sibling->nextSibling() == this) {
                                sibling->m_nextSibling.release();
                                sibling->m_nextSibling = std::move(m_nextSibling);
                                break;
                            }
                        }
                    }
                    // insert as child of better parent
                    if (EbmlElement *previousSibling = betterParent->lastChild()) {
                        previousSibling->m_nextSibling.reset(this);
                    } else {
                        betterParent->m_firstChild.reset(this);
                    }
                    // update own reference to parent
                    m_parent = betterParent;
                }
            }
        }

        // read size
        beg = static_cast<std::uint8_t>(stream().peek());
        mask = 0x80;
        m_sizeLength = 1;
        if ((m_sizeUnknown = (beg == 0xFF))) {
            // this indicates that the element size is unknown
            // -> just assume the element takes the maximum available size
            m_dataSize = maxTotalSize() - headerSize();
        } else {
            while (m_sizeLength <= maximumSizeLengthSupported() && (beg & mask) == 0) {
                ++m_sizeLength;
                mask >>= 1;
            }
            if (m_sizeLength > maximumSizeLengthSupported()) {
                if (!skipped) {
                    diag.emplace_back(DiagLevel::Critical, "EBML size length is not supported.", parsingContext());
                }
                continue; // try again
            }
            if (m_sizeLength > container().maxSizeLength()) {
                if (!skipped) {
                    diag.emplace_back(DiagLevel::Critical, "EBML size length is invalid.", parsingContext());
                }
                continue; // try again
            }
            // read size into buffer
            memset(buf, 0, sizeof(DataSizeType)); // reset buffer
            reader().read(buf + (maximumSizeLengthSupported() - m_sizeLength), m_sizeLength);
            // xor the first byte in buffer which has been read from the file with mask
            *(buf + (maximumSizeLengthSupported() - m_sizeLength)) ^= static_cast<char>(mask);
            m_dataSize = BE::toInt<std::uint64_t>(buf);
            // check if element is truncated
            if (totalSize() > maxTotalSize()) {
                if (m_idLength + m_sizeLength > maxTotalSize()) { // header truncated
                    if (!skipped) {
                        diag.emplace_back(DiagLevel::Critical, "EBML header seems to be truncated.", parsingContext());
                    }
                    continue; // try again
                } else { // data truncated
                    diag.emplace_back(DiagLevel::Warning, "Data of EBML element seems to be truncated; unable to parse siblings of that element.",
                        parsingContext());
                    m_dataSize = maxTotalSize() - m_idLength - m_sizeLength; // using max size instead
                }
            }
        }

        // check if there's a first child
        const std::uint64_t firstChildOffset = this->firstChildOffset();
        if (firstChildOffset && firstChildOffset < totalSize()) {
            m_firstChild.reset(new EbmlElement(static_cast<EbmlElement &>(*this), startOffset() + firstChildOffset));
        } else {
            m_firstChild.reset();
        }

        // check if there's a sibling
        if (totalSize() < maxTotalSize()) {
            if (parent()) {
                m_nextSibling.reset(new EbmlElement(*(parent()), startOffset() + totalSize()));
            } else {
                m_nextSibling.reset(new EbmlElement(container(), startOffset() + totalSize(), maxTotalSize() - totalSize()));
            }
        } else {
            m_nextSibling.reset();
        }

        // no critical errors occurred
        // -> add a warning if bytes have been skipped
        if (skipped) {
            diag.emplace_back(DiagLevel::Warning, argsToString(skipped, " bytes have been skipped"), parsingContext());
        }
        // -> don't need another try, return here
        return;
    }

    // critical errors occurred and skipping some bytes wasn't successful
    throw InvalidDataException();
}

/*!
 * \brief Reads the content of the element as string.
 */
std::string EbmlElement::readString()
{
    stream().seekg(static_cast<streamoff>(dataOffset()));
    return reader().readString(dataSize());
}

/*!
 * \brief Reads the content of the element as unsigned integer.
 *
 * Reads up to 8 bytes. If the element stores more data the
 * additional bytes are ignored.
 */
std::uint64_t EbmlElement::readUInteger()
{
    constexpr DataSizeType maxBytesToRead = 8;
    char buff[maxBytesToRead] = { 0 };
    const auto bytesToSkip = maxBytesToRead - min(dataSize(), maxBytesToRead);
    stream().seekg(static_cast<streamoff>(dataOffset()), ios_base::beg);
    stream().read(buff + bytesToSkip, static_cast<streamoff>(sizeof(buff) - bytesToSkip));
    return BE::toInt<std::uint64_t>(buff);
}

/*!
 * \brief Reads the content of the element as float.
 * \remarks Reads exactly 4 or 8 bytes. If the element stores more or less data zero is returned.
 */
double EbmlElement::readFloat()
{
    stream().seekg(static_cast<streamoff>(dataOffset()));
    switch (dataSize()) {
    case sizeof(float):
        return static_cast<double>(reader().readFloat32BE());
    case sizeof(double):
        return reader().readFloat64BE();
    default:
        return 0.0;
    }
}

/*!
 * \brief Returns the length of the specified \a id in byte.
 * \throws Throws InvalidDataException() if \a id can not be represented.
 */
std::uint8_t EbmlElement::calculateIdLength(GenericFileElement::IdentifierType id)
{
    if (id <= 0xFF) {
        return 1;
    } else if (id <= 0x7FFF) {
        return 2;
    } else if (id <= 0x3FFFFF) {
        return 3;
    } else if (id <= 0x1FFFFFFF) {
        return 4;
    } else {
        throw InvalidDataException();
    }
}

/*!
 * \brief Returns the length of the size denotation for the specified \a size in byte.
 * \throws Throws InvalidDataException() if \a size can not be represented.
 */
std::uint8_t EbmlElement::calculateSizeDenotationLength(std::uint64_t size)
{
    if (size < 126) {
        return 1;
    } else if (size <= 16382ul) {
        return 2;
    } else if (size <= 2097150ul) {
        return 3;
    } else if (size <= 268435454ul) {
        return 4;
    } else if (size <= 34359738366ul) {
        return 5;
    } else if (size <= 4398046511102ul) {
        return 6;
    } else if (size <= 562949953421310ul) {
        return 7;
    } else if (size <= 72057594037927934ul) {
        return 8;
    } else {
        throw InvalidDataException();
    }
}

/*!
 * \brief Stores the specified \a id in the specified buffer
 *        which must be at least 8 bytes long.
 * \returns Returns the number of bytes written to \a buff.
 * \throws Throws InvalidDataException() if \a id can not be represented.
 */
std::uint8_t EbmlElement::makeId(GenericFileElement::IdentifierType id, char *buff)
{
    if (id <= 0xFF) {
        *buff = static_cast<char>(id);
        return 1;
    } else if (id <= 0x7FFF) {
        BE::getBytes(static_cast<std::uint16_t>(id), buff);
        return 2;
    } else if (id <= 0x3FFFFF) {
        BE::getBytes(static_cast<std::uint32_t>(id << 0x8), buff);
        return 3;
    } else if (id <= 0x1FFFFFFF) {
        BE::getBytes(static_cast<std::uint32_t>(id), buff);
        return 4;
    } else {
        throw InvalidDataException();
    }
}

/*!
 * \brief Makes the size denotation for the specified \a size and stores it to \a buff.
 * \param size Specifies the size to be denoted.
 * \param buff Specifies the buffer to store the denotation. Must be at least 8 bytes long.
 * \returns Returns the number of bytes written to \a buff.
 * \throws Throws InvalidDataException() if \a size can not be represented.
 */
std::uint8_t EbmlElement::makeSizeDenotation(std::uint64_t size, char *buff)
{
    if (size < 126) {
        *buff = static_cast<char>(size | 0x80);
        return 1;
    } else if (size <= 16382ul) {
        BE::getBytes(static_cast<std::uint16_t>(size | 0x4000), buff);
        return 2;
    } else if (size <= 2097150ul) {
        BE::getBytes(static_cast<std::uint32_t>((size | 0x200000) << 0x08), buff);
        return 3;
    } else if (size <= 268435454ul) {
        BE::getBytes(static_cast<std::uint32_t>(size | 0x10000000), buff);
        return 4;
    } else if (size <= 34359738366ul) {
        BE::getBytes(static_cast<std::uint64_t>((size | 0x800000000) << 0x18), buff);
        return 5;
    } else if (size <= 4398046511102ul) {
        BE::getBytes(static_cast<std::uint64_t>((size | 0x40000000000) << 0x10), buff);
        return 6;
    } else if (size <= 562949953421310ul) {
        BE::getBytes(static_cast<std::uint64_t>((size | 0x2000000000000) << 0x08), buff);
        return 7;
    } else if (size <= 72057594037927934ul) {
        BE::getBytes(static_cast<std::uint64_t>(size | 0x100000000000000), buff);
        return 8;
    }
    throw InvalidDataException();
}

/*!
 * \brief Makes the size denotation for the specified \a size and stores it to \a buff.
 * \param size Specifies the size to be denoted.
 * \param buff Specifies the buffer to store the denotation. Must be at least 8 bytes long.
 * \param minBytes Specifies the minimum number of bytes to use. Might be use allow subsequent element growth.
 * \returns Returns the number of bytes written to \a buff. Always in the range of \a minBytes and 8.
 * \throws Throws InvalidDataException() if \a size can not be represented.
 */
std::uint8_t EbmlElement::makeSizeDenotation(std::uint64_t size, char *buff, std::uint8_t minBytes)
{
    if (minBytes <= 1 && size < 126) {
        *buff = static_cast<char>(size | 0x80);
        return 1;
    } else if (minBytes <= 2 && size <= 16382ul) {
        BE::getBytes(static_cast<std::uint16_t>(size | 0x4000), buff);
        return 2;
    } else if (minBytes <= 3 && size <= 2097150ul) {
        BE::getBytes(static_cast<std::uint32_t>((size | 0x200000) << 0x08), buff);
        return 3;
    } else if (minBytes <= 4 && size <= 268435454ul) {
        BE::getBytes(static_cast<std::uint32_t>(size | 0x10000000), buff);
        return 4;
    } else if (minBytes <= 5 && size <= 34359738366ul) {
        BE::getBytes(static_cast<std::uint64_t>((size | 0x800000000) << 0x18), buff);
        return 5;
    } else if (minBytes <= 6 && size <= 4398046511102ul) {
        BE::getBytes(static_cast<std::uint64_t>((size | 0x40000000000) << 0x10), buff);
        return 6;
    } else if (minBytes <= 7 && size <= 562949953421310ul) {
        BE::getBytes(static_cast<std::uint64_t>((size | 0x2000000000000) << 0x08), buff);
        return 7;
    } else if (minBytes <= 8 && size <= 72057594037927934ul) {
        BE::getBytes(static_cast<std::uint64_t>(size | 0x100000000000000), buff);
        return 8;
    }
    throw InvalidDataException();
}

/*!
 * \brief Returns the length of the specified unsigned \a integer in byte.
 * \throws Throws InvalidDataException() if \a integer can not be represented.
 */
std::uint8_t EbmlElement::calculateUIntegerLength(std::uint64_t integer)
{
    if (integer <= 0xFFul) {
        return 1;
    } else if (integer <= 0xFFFFul) {
        return 2;
    } else if (integer <= 0xFFFFFFul) {
        return 3;
    } else if (integer <= 0xFFFFFFFFul) {
        return 4;
    } else if (integer <= 0xFFFFFFFFFFul) {
        return 5;
    } else if (integer <= 0xFFFFFFFFFFFFul) {
        return 6;
    } else if (integer <= 0xFFFFFFFFFFFFFFul) {
        return 7;
    } else {
        return 8;
    }
}

/*!
 * \brief Writes \a value to \a buff.
 * \returns Returns the number of bytes written to \a buff.
 */
std::uint8_t EbmlElement::makeUInteger(std::uint64_t value, char *buff)
{
    if (value <= 0xFFul) {
        *buff = static_cast<char>(value);
        return 1;
    } else if (value <= 0xFFFFul) {
        BE::getBytes(static_cast<std::uint16_t>(value), buff);
        return 2;
    } else if (value <= 0xFFFFFFul) {
        BE::getBytes(static_cast<std::uint32_t>(value << 0x08), buff);
        return 3;
    } else if (value <= 0xFFFFFFFFul) {
        BE::getBytes(static_cast<std::uint32_t>(value), buff);
        return 4;
    } else if (value <= 0xFFFFFFFFFFul) {
        BE::getBytes(static_cast<std::uint64_t>(value << 0x18), buff);
        return 5;
    } else if (value <= 0xFFFFFFFFFFFFul) {
        BE::getBytes(static_cast<std::uint64_t>(value << 0x10), buff);
        return 6;
    } else if (value <= 0xFFFFFFFFFFFFFFul) {
        BE::getBytes(static_cast<std::uint64_t>(value << 0x08), buff);
        return 7;
    } else {
        BE::getBytes(static_cast<std::uint64_t>(value), buff);
        return 8;
    }
}

/*!
 * \brief Writes \a value to \a buff.
 * \returns Returns the number of bytes written to \a buff.
 * \param value Specifies the value to be written.
 * \param buff Specifies the buffer to write to.
 * \param minBytes Specifies the minimum number of bytes to use.
 * \remarks Regardless of \a minBytes, this function will never make
 *          more than 8 bytes.
 */
std::uint8_t EbmlElement::makeUInteger(std::uint64_t value, char *buff, std::uint8_t minBytes)
{
    if (minBytes <= 1 && value <= 0xFFul) {
        *buff = static_cast<char>(value);
        return 1;
    } else if (minBytes <= 2 && value <= 0xFFFFul) {
        BE::getBytes(static_cast<std::uint16_t>(value), buff);
        return 2;
    } else if (minBytes <= 3 && value <= 0xFFFFFFul) {
        BE::getBytes(static_cast<std::uint32_t>(value << 0x08), buff);
        return 3;
    } else if (minBytes <= 4 && value <= 0xFFFFFFFFul) {
        BE::getBytes(static_cast<std::uint32_t>(value), buff);
        return 4;
    } else if (minBytes <= 5 && value <= 0xFFFFFFFFFFul) {
        BE::getBytes(static_cast<std::uint64_t>(value << 0x18), buff);
        return 5;
    } else if (minBytes <= 6 && value <= 0xFFFFFFFFFFFFul) {
        BE::getBytes(static_cast<std::uint64_t>(value << 0x10), buff);
        return 6;
    } else if (minBytes <= 7 && value <= 0xFFFFFFFFFFFFFFul) {
        BE::getBytes(static_cast<std::uint64_t>(value << 0x08), buff);
        return 7;
    } else {
        BE::getBytes(static_cast<std::uint64_t>(value), buff);
        return 8;
    }
}

/*!
 * \brief Makes a simple EBML element.
 * \param stream Specifies the stream to write the data to.
 * \param id Specifies the element ID.
 * \param content Specifies the value of the element as unsigned integer.
 */
void EbmlElement::makeSimpleElement(ostream &stream, IdentifierType id, std::uint64_t content)
{
    char buff1[8];
    char buff2[8];
    std::uint8_t sizeLength = EbmlElement::makeId(id, buff1);
    stream.write(buff1, sizeLength);
    std::uint8_t elementSize = EbmlElement::makeUInteger(content, buff2);
    sizeLength = EbmlElement::makeSizeDenotation(elementSize, buff1);
    stream.write(buff1, sizeLength);
    stream.write(buff2, elementSize);
}

/*!
 * \brief Makes a simple EBML element.
 * \param stream Specifies the stream to write the data to.
 * \param id Specifies the element ID.
 * \param content Specifies the value of the element as string.
 */
void EbmlElement::makeSimpleElement(std::ostream &stream, GenericFileElement::IdentifierType id, string_view content)
{
    char buff1[8];
    std::uint8_t sizeLength = EbmlElement::makeId(id, buff1);
    stream.write(buff1, sizeLength);
    sizeLength = EbmlElement::makeSizeDenotation(content.size(), buff1);
    stream.write(buff1, sizeLength);
    stream.write(content.data(), static_cast<std::streamsize>(content.size()));
}

} // namespace TagParser
