#include "./ebmlelement.h"
#include "./ebmlid.h"
#include "./matroskacontainer.h"
#include "./matroskaid.h"

#include "../mediafileinfo.h"
#include "../exceptions.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <string>
#include <sstream>
#include <cstring>
#include <memory>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;


namespace Media {

/*!
 * \class Media::EbmlElement
 * \brief The EbmlElement class helps to parse EBML files such as Matroska files.
 */

/*!
 * \brief Specifies the number of bytes to be skipped till a valid EBML element is found in the stream.
 */
uint64 EbmlElement::bytesToBeSkipped = 0x4000;

/*!
 * \brief Constructs a new top level element with the specified \a container at the specified \a startOffset.
 */
EbmlElement::EbmlElement(MatroskaContainer &container, uint64 startOffset) :
    GenericFileElement<EbmlElement>(container, startOffset)
{}

/*!
 * \brief Constructs a new top level element with the specified \a container at the specified \a startOffset.
 */
EbmlElement::EbmlElement(MatroskaContainer &container, uint64 startOffset, uint64 maxSize) :
    GenericFileElement<EbmlElement>(container, startOffset, maxSize)
{}

/*!
 * \brief Constructs a new sub level element with the specified \a parent at the specified \a startOffset.
 */
EbmlElement::EbmlElement(EbmlElement &parent, uint64 startOffset) :
    GenericFileElement<EbmlElement>(parent, startOffset)
{}

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
void EbmlElement::internalParse()
{
    invalidateStatus();
    static const string context("parsing EBML element header");

    for(uint64 skipped = 0; skipped < bytesToBeSkipped; ++m_startOffset, --m_maxSize, ++skipped) {
        // check whether max size is valid
        if(maxTotalSize() < 2) {
            addNotification(NotificationType::Critical, argsToString("The EBML element at ", startOffset(), " is truncated or does not exist."), context);
            throw TruncatedDataException();
        }
        stream().seekg(startOffset());
        // read ID
        char buf[maximumIdLengthSupported() > maximumSizeLengthSupported() ? maximumIdLengthSupported() : maximumSizeLengthSupported()] = {0};
        byte beg = static_cast<byte>(stream().peek()), mask = 0x80;
        m_idLength = 1;
        while(m_idLength <= maximumIdLengthSupported() && (beg & mask) == 0) {
            ++m_idLength;
            mask >>= 1;
        }
        if(m_idLength > maximumIdLengthSupported()) {
            if(!skipped) {
                addNotification(NotificationType::Critical, argsToString("EBML ID length at ", startOffset(), " is not supported, trying to skip."), context);
            }
            continue; // try again
        }
        if(m_idLength > container().maxIdLength()) {
            if(!skipped) {
                addNotification(NotificationType::Critical, argsToString("EBML ID length at ", startOffset(), " is invalid, trying to skip."), context);
            }
            continue; // try again
        }
        reader().read(buf + (maximumIdLengthSupported() - m_idLength), m_idLength);
        m_id = BE::toUInt32(buf);

        // read size
        beg = static_cast<byte>(stream().peek()), mask = 0x80;
        m_sizeLength = 1;
        if(beg == 0xFF) {
            // this indicates that the element size is unknown
            // -> just assume the element takes the maximum available size
            m_dataSize = maxTotalSize() - headerSize();
        } else {
           while(m_sizeLength <= maximumSizeLengthSupported() && (beg & mask) == 0) {
                ++m_sizeLength;
                mask >>= 1;
            }
            if(m_sizeLength > maximumSizeLengthSupported()) {
                if(!skipped) {
                    addNotification(NotificationType::Critical, "EBML size length is not supported.", parsingContext());
                }
                continue; // try again
            }
            if(m_sizeLength > container().maxSizeLength()) {
                if(!skipped) {
                    addNotification(NotificationType::Critical, "EBML size length is invalid.", parsingContext());
                }
                continue; // try again
            }
            // read size into buffer
            memset(buf, 0, sizeof(DataSizeType)); // reset buffer
            reader().read(buf + (maximumSizeLengthSupported() - m_sizeLength), m_sizeLength);
            // xor the first byte in buffer which has been read from the file with mask
            *(buf + (maximumSizeLengthSupported() - m_sizeLength)) ^= mask;
            m_dataSize = ConversionUtilities::BE::toUInt64(buf);
            // check if element is truncated
            if(totalSize() > maxTotalSize()) {
                if(m_idLength + m_sizeLength > maxTotalSize()) { // header truncated
                    if(!skipped) {
                        addNotification(NotificationType::Critical, "EBML header seems to be truncated.", parsingContext());
                    }
                    continue; // try again
                } else { // data truncated
                    addNotification(NotificationType::Warning, "Data of EBML element seems to be truncated; unable to parse siblings of that element.", parsingContext());
                    m_dataSize = maxTotalSize() - m_idLength - m_sizeLength; // using max size instead
                }
            }
        }

        // check if there's a first child
        const uint64 firstChildOffset = this->firstChildOffset();
        if(firstChildOffset && firstChildOffset < totalSize()) {
            m_firstChild.reset(new EbmlElement(static_cast<EbmlElement &>(*this), startOffset() + firstChildOffset));
        } else {
            m_firstChild.reset();
        }

        // check if there's a sibling
        if(totalSize() < maxTotalSize()) {
            if(parent()) {
                m_nextSibling.reset(new EbmlElement(*(parent()), startOffset() + totalSize()));
            } else {
                m_nextSibling.reset(new EbmlElement(container(), startOffset() + totalSize(), maxTotalSize() - totalSize()));
            }
        } else {
            m_nextSibling.reset();
        }

        // no critical errors occured
        // -> add a warning if bytes have been skipped
        if(skipped) {
            addNotification(NotificationType::Warning, argsToString(skipped, " bytes have been skipped"), parsingContext());
        }
        // -> don't need another try, return here
        return;
    }

    // critical errors occured and skipping some bytes wasn't successful
    throw InvalidDataException();
}

/*!
 * \brief Reads the content of the element as string.
 */
std::string EbmlElement::readString()
{
    stream().seekg(dataOffset());
    return reader().readString(dataSize());
}

/*!
 * \brief Reads the content of the element as unsigned integer.
 *
 * Reads up to 8 bytes. If the element stores more data the
 * additional bytes are ignored.
 */
uint64 EbmlElement::readUInteger()
{
    char buff[sizeof(uint64)] = {0};
    int i = static_cast<int>(sizeof(buff)) - dataSize();
    if(i < 0) {
        i = 0;
    }
    stream().seekg(dataOffset(), ios_base::beg);
    stream().read(buff + i, sizeof(buff) - i);
    return BE::toUInt64(buff);
}

/*!
 * \brief Reads the content of the element as float.
 * \remarks Reads exactly 4 or 8 bytes. If the element stores more or less data zero is returned.
 */
float64 EbmlElement::readFloat()
{
    stream().seekg(dataOffset());
    switch(dataSize()) {
    case sizeof(float32):
        return reader().readFloat32BE();
    case sizeof(float64):
        return reader().readFloat64BE();
    default:
        return 0.0;
    }
}

/*!
 * \brief Returns the length of the specified \a id in byte.
 * \throws Throws InvalidDataException() if \a id can not be represented.
 */
byte EbmlElement::calculateIdLength(GenericFileElement::IdentifierType id)
{
    if(id <= 0xFF) {
        return 1;
    } else if(id <= 0x7FFF) {
        return 2;
    } else if(id <= 0x3FFFFF) {
        return 3;
    } else if(id <= 0x1FFFFFFF) {
        return 4;
    } else {
        throw InvalidDataException();
    }
}

/*!
 * \brief Returns the length of the size denotation for the specified \a size in byte.
 * \throws Throws InvalidDataException() if \a size can not be represented.
 */
byte EbmlElement::calculateSizeDenotationLength(uint64 size)
{
    if(size < 126) {
        return 1;
    } else if(size <= 16382ul) {
        return 2;
    } else if(size <= 2097150ul) {
        return 3;
    } else if(size <= 268435454ul) {
        return 4;
    } else if(size <= 34359738366ul) {
        return 5;
    } else if(size <= 4398046511102ul) {
        return 6;
    } else if(size <= 562949953421310ul) {
        return 7;
    } else if(size <= 72057594037927934ul) {
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
byte EbmlElement::makeId(GenericFileElement::IdentifierType id, char *buff)
{
    if(id <= 0xFF) {
        *buff = static_cast<byte>(id);
        return 1;
    } else if(id <= 0x7FFF) {
        BE::getBytes(static_cast<uint16>(id), buff);
        return 2;
    } else if(id <= 0x3FFFFF) {
        BE::getBytes(static_cast<uint32>(id << 0x8), buff);
        return 3;
    } else if(id <= 0x1FFFFFFF) {
        BE::getBytes(static_cast<uint32>(id), buff);
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
byte EbmlElement::makeSizeDenotation(uint64 size, char *buff)
{
    if(size < 126) {
        *buff = static_cast<byte>(size | 0x80);
        return 1;
    } else if(size <= 16382ul) {
        BE::getBytes(static_cast<uint16>(size | 0x4000), buff);
        return 2;
    } else if(size <= 2097150ul) {
        BE::getBytes(static_cast<uint32>((size | 0x200000) << 0x08), buff);
        return 3;
    } else if(size <= 268435454ul) {
        BE::getBytes(static_cast<uint32>(size | 0x10000000), buff);
        return 4;
    } else if(size <= 34359738366ul) {
        BE::getBytes(static_cast<uint64>((size | 0x800000000) << 0x18), buff);
        return 5;
    } else if(size <= 4398046511102ul) {
        BE::getBytes(static_cast<uint64>((size | 0x40000000000) << 0x10), buff);
        return 6;
    } else if(size <= 562949953421310ul) {
        BE::getBytes(static_cast<uint64>((size | 0x2000000000000) << 0x08), buff);
        return 7;
    } else if(size <= 72057594037927934ul) {
        BE::getBytes(static_cast<uint64>(size | 0x100000000000000), buff);
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
byte EbmlElement::makeSizeDenotation(uint64 size, char *buff, byte minBytes)
{
    if(minBytes <= 1 && size < 126) {
        *buff = static_cast<byte>(size | 0x80);
        return 1;
    } else if(minBytes <= 2 && size <= 16382ul) {
        BE::getBytes(static_cast<uint16>(size | 0x4000), buff);
        return 2;
    } else if(minBytes <= 3 && size <= 2097150ul) {
        BE::getBytes(static_cast<uint32>((size | 0x200000) << 0x08), buff);
        return 3;
    } else if(minBytes <= 4 && size <= 268435454ul) {
        BE::getBytes(static_cast<uint32>(size | 0x10000000), buff);
        return 4;
    } else if(minBytes <= 5 && size <= 34359738366ul) {
        BE::getBytes(static_cast<uint64>((size | 0x800000000) << 0x18), buff);
        return 5;
    } else if(minBytes <= 6 && size <= 4398046511102ul) {
        BE::getBytes(static_cast<uint64>((size | 0x40000000000) << 0x10), buff);
        return 6;
    } else if(minBytes <= 7 && size <= 562949953421310ul) {
        BE::getBytes(static_cast<uint64>((size | 0x2000000000000) << 0x08), buff);
        return 7;
    } else if(minBytes <= 8 && size <= 72057594037927934ul) {
        BE::getBytes(static_cast<uint64>(size | 0x100000000000000), buff);
        return 8;
    }
    throw InvalidDataException();
}

/*!
 * \brief Returns the length of the specified unsigned \a integer in byte.
 * \throws Throws InvalidDataException() if \a integer can not be represented.
 */
byte EbmlElement::calculateUIntegerLength(uint64 integer)
{
    if(integer <= 0xFFul) {
        return 1;
    } else if(integer <= 0xFFFFul) {
        return 2;
    } else if(integer <= 0xFFFFFFul) {
        return 3;
    } else if(integer <= 0xFFFFFFFFul) {
        return 4;
    } else if(integer <= 0xFFFFFFFFFFul) {
        return 5;
    } else if(integer <= 0xFFFFFFFFFFFFul) {
        return 6;
    } else if(integer <= 0xFFFFFFFFFFFFFFul) {
        return 7;
    } else {
        return 8;
    }
}

/*!
 * \brief Writes \a value to \a buff.
 * \returns Returns the number of bytes written to \a buff.
 */
byte EbmlElement::makeUInteger(uint64 value, char *buff)
{
    if(value <= 0xFFul) {
        *buff = static_cast<char>(value);
        return 1;
    } else if(value <= 0xFFFFul) {
        BE::getBytes(static_cast<uint16>(value), buff);
        return 2;
    } else if(value <= 0xFFFFFFul) {
        BE::getBytes(static_cast<uint32>(value << 0x08), buff);
        return 3;
    } else if(value <= 0xFFFFFFFFul) {
        BE::getBytes(static_cast<uint32>(value), buff);
        return 4;
    } else if(value <= 0xFFFFFFFFFFul) {
        BE::getBytes(static_cast<uint64>(value << 0x18), buff);
        return 5;
    } else if(value <= 0xFFFFFFFFFFFFul) {
        BE::getBytes(static_cast<uint64>(value << 0x10), buff);
        return 6;
    } else if(value <= 0xFFFFFFFFFFFFFFul) {
        BE::getBytes(static_cast<uint64>(value << 0x08), buff);
        return 7;
    } else {
        BE::getBytes(static_cast<uint64>(value), buff);
        return 8;
    }
}

/*!
 * \brief Writes \a value to \a buff.
 * \returns Returns the number of bytes written to \a buff.
 * \param value Specifies the value to be written.
 * \param buff Specifies the buffer to write to.
 * \param minBytes Specifies the minimum number of bytes to use.
 */
byte EbmlElement::makeUInteger(uint64 value, char *buff, byte minBytes)
{
    if(minBytes <= 1 && value <= 0xFFul) {
        *buff = static_cast<char>(value);
        return 1;
    } else if(minBytes <= 2 && value <= 0xFFFFul) {
        BE::getBytes(static_cast<uint16>(value), buff);
        return 2;
    } else if(minBytes <= 3 && value <= 0xFFFFFFul) {
        BE::getBytes(static_cast<uint32>(value << 0x08), buff);
        return 3;
    } else if(minBytes <= 4 && value <= 0xFFFFFFFFul) {
        BE::getBytes(static_cast<uint32>(value), buff);
        return 4;
    } else if(minBytes <= 5 && value <= 0xFFFFFFFFFFul) {
        BE::getBytes(static_cast<uint64>(value << 0x18), buff);
        return 5;
    } else if(minBytes <= 6 && value <= 0xFFFFFFFFFFFFul) {
        BE::getBytes(static_cast<uint64>(value << 0x10), buff);
        return 6;
    } else if(minBytes <= 7 && value <= 0xFFFFFFFFFFFFFFul) {
        BE::getBytes(static_cast<uint64>(value << 0x08), buff);
        return 7;
    } else {
        BE::getBytes(static_cast<uint64>(value), buff);
        return 8;
    }
}

/*!
 * \brief Makes a simple EBML element.
 * \param stream Specifies the stream to write the data to.
 * \param id Specifies the element ID.
 * \param content Specifies the value of the element as unsigned integer.
 */
void EbmlElement::makeSimpleElement(ostream &stream, IdentifierType id, uint64 content)
{
    char buff1[8];
    char buff2[8];
    byte sizeLength = EbmlElement::makeId(id, buff1);
    stream.write(buff1, sizeLength);
    byte elementSize = EbmlElement::makeUInteger(content, buff2);
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
void EbmlElement::makeSimpleElement(std::ostream &stream, GenericFileElement::IdentifierType id, const std::string &content)
{
    char buff1[8];
    byte sizeLength = EbmlElement::makeId(id, buff1);
    stream.write(buff1, sizeLength);
    sizeLength = EbmlElement::makeSizeDenotation(content.size(), buff1);
    stream.write(buff1, sizeLength);
    stream.write(content.c_str(), content.size());
}

/*!
 * \brief Makes a simple EBML element.
 * \param stream Specifies the stream to write the data to.
 * \param id Specifies the element ID.
 * \param data Specifies the data of the element.
 * \param dataSize Specifies the size of \a data.
 */
void EbmlElement::makeSimpleElement(ostream &stream, GenericFileElement::IdentifierType id, const char *data, std::size_t dataSize)
{
    char buff1[8];
    byte sizeLength = EbmlElement::makeId(id, buff1);
    stream.write(buff1, sizeLength);
    sizeLength = EbmlElement::makeSizeDenotation(dataSize, buff1);
    stream.write(buff1, sizeLength);
    stream.write(data, dataSize);
}

}



