#include "./mp4track.h"
#include "./mp4tag.h"
#include "./mp4atom.h"
#include "./mp4ids.h"
#include "./mp4container.h"

#include "../mediafileinfo.h"
#include "../exceptions.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <sstream>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::Mp4Atom
 * \brief The Mp4Atom class helps to parse MP4 files.
 */

/*!
 * \brief Constructs a new top level atom with the specified \a container at the specified \a startOffset.
 */
Mp4Atom::Mp4Atom(GenericFileElement::ContainerType &container, uint64 startOffset) :
    GenericFileElement<Mp4Atom>(container, startOffset)
{}

/*!
 * \brief Constructs a new top level atom with the specified \a container at the specified \a startOffset.
 */
Mp4Atom::Mp4Atom(GenericFileElement::ContainerType &container, uint64 startOffset, uint64 maxSize) :
    GenericFileElement<Mp4Atom>(container, startOffset, maxSize)
{}

/*!
 * \brief Constructs a new sub level atom with the specified \a parent at the specified \a startOffset.
 */
Mp4Atom::Mp4Atom(Mp4Atom &parent, uint64 startOffset) :
    GenericFileElement<Mp4Atom>(parent, startOffset)
{}

/*!
 * \brief Returns the parsing context.
 */
string Mp4Atom::parsingContext() const
{
    return "parsing " % idToString() % " atom at " + startOffset();
}

/*!
 * \brief Parses the MP4 atom.
 */
void Mp4Atom::internalParse()
{
    invalidateStatus();
    static const string context("parsing MP4 atom");
    if(maxTotalSize() < minimumElementSize()) {
        addNotification(NotificationType::Critical, "Atom is smaller than 8 byte and hence invalid. The remaining size within the parent atom is " % numberToString(maxTotalSize()) + ".", context);
        throw TruncatedDataException();
    }
    stream().seekg(startOffset());
    m_dataSize = reader().readUInt32BE();
    if(m_dataSize == 0) {
        // atom size extends to rest of the file/enclosing container
        m_dataSize = maxTotalSize();
    }
    if(!m_dataSize) {
        addNotification(NotificationType::Critical, "No data found (only null bytes).", context);
        throw NoDataFoundException();
    }
    if(m_dataSize < 8 && m_dataSize != 1) {
        addNotification(NotificationType::Critical, "Atom is smaller than 8 byte and hence invalid.", context);
        throw TruncatedDataException();
    }
    m_id = reader().readUInt32BE();
    m_idLength = 4;
    if(m_dataSize == 1) { // atom denotes 64-bit size
        m_dataSize = reader().readUInt64BE();
        m_sizeLength = 12; // 4 bytes indicate long size denotation + 8 bytes for actual size denotation
        if(dataSize() < 16 && m_dataSize != 1) {
            addNotification(NotificationType::Critical, "Atom denoting 64-bit size is smaller than 16 byte and hence invalid.", parsingContext());
            throw TruncatedDataException();
        }
    } else {
        m_sizeLength = 4;
    }
    if(maxTotalSize() < m_dataSize) { // currently m_dataSize holds data size plus header size!
        addNotification(NotificationType::Warning, "The atom seems to be truncated; unable to parse siblings of that ", parsingContext());
        m_dataSize = maxTotalSize(); // using max size instead
    }
    // currently m_dataSize holds data size plus header size!
    m_dataSize -= headerSize();
    Mp4Atom *child = nullptr;
    if(uint64 firstChildOffset = this->firstChildOffset()) {
        if(firstChildOffset + minimumElementSize() <= totalSize()) {
            child = new Mp4Atom(static_cast<Mp4Atom &>(*this), startOffset() + firstChildOffset);
        }
    }
    m_firstChild.reset(child);
    Mp4Atom *sibling = nullptr;
    if(totalSize() < maxTotalSize()) {
        if(parent()) {
            sibling = new Mp4Atom(*(parent()), startOffset() + totalSize());
        } else {
            sibling = new Mp4Atom(container(), startOffset() + totalSize(), maxTotalSize() - totalSize());
        }
    }
    m_nextSibling.reset(sibling);
}

/*!
 * \brief This function helps to write the atom size after writing an atom to a stream.
 * \param stream Specifies the stream.
 * \param startOffset Specifies the start offset of the atom.
 *
 * This function seeks back to the start offset and writes the difference between the
 * previous offset and the start offset as 32-bit unsigned integer to the \a stream.
 * Then it seeks back to the previous offset.
 */
void Mp4Atom::seekBackAndWriteAtomSize(std::ostream &stream, const ostream::pos_type &startOffset)
{
    ostream::pos_type currentOffset = stream.tellp();
    stream.seekp(startOffset);
    BinaryWriter writer(&stream);
    writer.writeUInt32BE(currentOffset - startOffset);
    stream.seekp(currentOffset);
}

/*!
 * \brief This function helps to write the atom size after writing an atom to a stream.
 * \param stream Specifies the stream.
 * \param startOffset Specifies the start offset of the atom.
 *
 * This function seeks back to the start offset and writes the difference between the
 * previous offset and the start offset as 64-bit unsigned integer to the \a stream.
 * Then it seeks back to the previous offset.
 */
void Mp4Atom::seekBackAndWriteAtomSize64(std::ostream &stream, const ostream::pos_type &startOffset)
{
    ostream::pos_type currentOffset = stream.tellp();
    stream.seekp(startOffset);
    BinaryWriter writer(&stream);
    writer.writeUInt32BE(1);
    stream.seekp(4, ios_base::cur);
    writer.writeUInt64BE(currentOffset - startOffset);
    stream.seekp(currentOffset);
}

/*!
 * \brief Writes an MP4 atom header to the specified \a stream.
 */
void Mp4Atom::makeHeader(uint64 size, uint32 id, BinaryWriter &writer)
{
    if(size < numeric_limits<uint32>::max()) {
        writer.writeUInt32BE(static_cast<uint32>(size));
        writer.writeUInt32BE(id);
    } else {
        writer.writeUInt32BE(1);
        writer.writeUInt32BE(id);
        writer.writeUInt64BE(size);
    }
}

/*!
 * \brief Returns an indication whether the atom is a parent element.
 *
 * \remarks This information is not read from the atom header. Some
 *          atoms are simply known to be parents whereas all other
 *          are considered as non-parents.
 */
bool Mp4Atom::isParent() const
{
    using namespace Mp4AtomIds;
    // some atom ids are known to be parents
    switch(id()) {
    case Movie: case Track: case Media: case MediaInformation: case DataInformation:
    case SampleTable: case UserData: case Meta: case ItunesList: case MovieFragment:
    case TrackFragment: case MovieExtends: case DataReference: case Mp4AtomIds::AvcConfiguration:
    case FourccIds::Mpeg4Audio: case FourccIds::AmrNarrowband: case FourccIds::Amr:
    case FourccIds::Drms: case FourccIds::Alac: case FourccIds::WindowsMediaAudio:
    case FourccIds::Ac3: case FourccIds::EAc3: case FourccIds::DolbyMpl:
    case FourccIds::Dts: case FourccIds::DtsH: case FourccIds::DtsE:
        return true;
    default:
        if(parent()) {
            // some atom ids are known to contain parents
            switch(parent()->id()) {
            case ItunesList:
                return true;
            default: ;
            }
        }
    }
    return false;
}

/*!
 * \brief Returns an indication whether the atom is a padding element.
 *
 * \remarks This information is not read from the atom header. Atoms with
 *          the IDs "free" and "skip" are considered as padding.
 */
bool Mp4Atom::isPadding() const
{
    using namespace Mp4AtomIds;
    switch(id()) {
    case Free: case Skip:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns the offset of the first child (relative to the start offset of this atom).
 *
 * \remarks This information is not read from the atom header. The offsets are known
 *          for specific atoms.
 * \remarks This method returns zero for non-parent atoms which have no childs.
 * \remarks Childs with variable offset such as the "esds"-atom must be denoted!
 */
uint64 Mp4Atom::firstChildOffset() const
{
    using namespace Mp4AtomIds;
    using namespace FourccIds;
    if(isParent()) {
        switch(id()) {
        case Meta: return headerSize() + 0x4u;
        case DataReference: return headerSize() + 0x8u;
        default: return headerSize();
        }
    } else {
        switch(id()) {
        case SampleDescription: return headerSize() + 0x08u;
        default: return 0x00u;
        }
    }
}

}
