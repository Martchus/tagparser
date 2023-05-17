#include "./opusidentificationheader.h"

#include "../ogg/oggiterator.h"

#include "../exceptions.h"

#include <c++utilities/conversion/binaryconversion.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::OpusIdentificationHeader
 * \brief The OpusIdentificationHeader class is an Opus identification header parser.
 * \sa https://wiki.xiph.org/OggOpus
 */

/*!
 * \brief Parses the Opus identification header which is read using the specified \a iterator.
 * \remarks The header is assumed to start at the current position of \a iterator.
 */
void OpusIdentificationHeader::parseHeader(OggIterator &iterator)
{
    char buff[19 - 8];
    iterator.read(buff, 8);
    if (BE::toInt<std::uint64_t>(buff) != 0x4F70757348656164u) {
        throw InvalidDataException(); // not Opus identification header
    }
    iterator.read(buff, sizeof(buff));
    m_version = static_cast<std::uint8_t>(*(buff));
    m_channels = static_cast<std::uint8_t>(*(buff + 1));
    m_preSkip = LE::toInt<std::uint16_t>(buff + 2);
    m_sampleRate = LE::toUInt32(buff + 4);
    m_outputGain = LE::toInt<std::uint16_t>(buff + 8);
    m_channelMap = static_cast<std::uint8_t>(*(buff + 10));
}

} // namespace TagParser
