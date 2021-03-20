#include "./mpeg4descriptor.h"
#include "./mp4container.h"
#include "./mp4ids.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/binaryreader.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::Mpeg4Descriptor
 * \brief The Mpeg4Descriptor class helps to parse MPEG-4 descriptors.
 */

/*!
 * \brief Constructs a new top level descriptor with the specified \a container at the specified \a startOffset
 *        and with the specified \a maxSize.
 */
Mpeg4Descriptor::Mpeg4Descriptor(ContainerType &container, std::uint64_t startOffset, std::uint64_t maxSize)
    : GenericFileElement<Mpeg4Descriptor>(container, startOffset, maxSize)
{
}

/*!
 * \brief Constructs a new sub level descriptor with the specified \a parent at the specified \a startOffset.
 */
Mpeg4Descriptor::Mpeg4Descriptor(Mpeg4Descriptor &parent, std::uint64_t startOffset)
    : GenericFileElement<Mpeg4Descriptor>(parent, startOffset)
{
}

/*!
 * \brief Returns the parsing context.
 */
string Mpeg4Descriptor::parsingContext() const
{
    return "parsing " % idToString() % " descriptor at " + startOffset();
}

/*!
 * \brief Converts the specified atom \a ID to a printable string.
 */
std::string Mpeg4Descriptor::idToString() const
{
    return "0x" + numberToString(id(), static_cast<std::uint8_t>(16));
}

/*!
 * \brief Parses the MPEG-4 descriptor.
 * \remarks Does not detect the first child.
 */
void Mpeg4Descriptor::internalParse(Diagnostics &diag)
{
    if (maxTotalSize() < minimumElementSize()) {
        diag.emplace_back(DiagLevel::Critical,
            "Descriptor is smaller than 2 byte and hence invalid. The maximum size within the encloding element is " % numberToString(maxTotalSize())
                + '.',
            "parsing MPEG-4 descriptor");
        throw TruncatedDataException();
    }
    stream().seekg(static_cast<streamoff>(startOffset()));
    // read ID
    m_idLength = m_sizeLength = 1;
    m_id = reader().readByte();
    // read data size
    std::uint8_t tmp = reader().readByte();
    m_dataSize = tmp & 0x7F;
    while (tmp & 0x80) {
        m_dataSize = (m_dataSize << 7) | ((tmp = reader().readByte()) & 0x7F);
        ++m_sizeLength;
    }
    // check whether the denoted data size exceeds the available data size
    if (maxTotalSize() < totalSize()) {
        diag.emplace_back(DiagLevel::Warning, "The descriptor seems to be truncated; unable to parse siblings of that ", parsingContext());
        m_dataSize = static_cast<std::uint32_t>(maxTotalSize()); // using max size instead
    }
    m_firstChild.reset();

    // check for siblings
    if (totalSize() >= maxTotalSize()) {
        m_nextSibling.reset();
        return;
    }
    if (parent()) {
        m_nextSibling.reset(new Mpeg4Descriptor(*(parent()), startOffset() + totalSize()));
    } else {
        m_nextSibling.reset(new Mpeg4Descriptor(container(), startOffset() + totalSize(), maxTotalSize() - totalSize()));
    }
}

} // namespace TagParser
