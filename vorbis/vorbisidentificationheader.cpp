#include "./vorbisidentificationheader.h"
#include "./vorbispackagetypes.h"

#include "../ogg/oggiterator.h"

#include "../exceptions.h"

#include <c++utilities/conversion/binaryconversion.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::VorbisIdentificationHeader
 * \brief The VorbisIdentificationHeader class is a Vorbis identification header parser.
 */

/*!
 * \brief Parses the Vorbis identification header which is read using the specified \a iterator.
 * \remarks The header is assumed to start at the current position of \a iterator.
 */
void VorbisIdentificationHeader::parseHeader(OggIterator &iterator)
{
    char buff[30 - 7];
    iterator.read(buff, 7);
    if ((BE::toInt<std::uint64_t>(buff) & 0xffffffffffffff00u) != 0x01766F7262697300u) {
        throw InvalidDataException(); // not Vorbis identification header
    }
    iterator.read(buff, sizeof(buff));
    m_version = LE::toUInt32(buff);
    m_channels = static_cast<std::uint8_t>(*(buff + 4));
    m_sampleRate = LE::toUInt32(buff + 5);
    m_maxBitrate = LE::toUInt32(buff + 9);
    m_nominalBitrate = LE::toUInt32(buff + 13);
    m_minBitrate = LE::toUInt32(buff + 17);
    m_blockSize = static_cast<std::uint8_t>(*(buff + 21));
    m_framingFlag = static_cast<std::uint8_t>(*(buff + 22));
}

} // namespace TagParser
