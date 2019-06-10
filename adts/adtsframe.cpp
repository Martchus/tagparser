#include "./adtsframe.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>

using namespace std;

namespace TagParser {

/*!
 * \class TagParser::AdtsFrame
 * \brief The AdtsFrame class is used to parse "Audio Data Transport Stream" frames.
 */

/*!
 * \brief Parses the header read using the specified \a reader.
 * \throws Throws InvalidDataException if the data read from the stream is
 *         no valid frame header.
 */
void AdtsFrame::parseHeader(CppUtilities::BinaryReader &reader)
{
    m_header1 = reader.readUInt16BE();
    // check whether syncword is present
    if ((m_header1 & 0xFFF6u) != 0xFFF0u) {
        throw InvalidDataException();
    }
    m_header2 = hasCrc() ? reader.readUInt56BE() : (reader.readUInt40BE() << 16);
    // check whether frame length is ok
    if (totalSize() < headerSize()) {
        throw InvalidDataException();
    }
}

} // namespace TagParser
