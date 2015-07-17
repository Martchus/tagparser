#include "adtsframe.h"
#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>

using namespace std;
using namespace IoUtilities;

namespace Media {

/*!
 * \class Media::AdtsFrame
 * \brief The AdtsFrame class is used to parse "Audio Data Transport Stream" frames.
 */

/*!
 * \brief Parses the header read using the specified \a reader.
 * \throws Throws InvalidDataException if the data read from the stream is
 *         no valid frame header.
 */
void AdtsFrame::parseHeader(IoUtilities::BinaryReader &reader)
{
    m_header1 = reader.readUInt16BE();
    if(!isValid()) {
        throw InvalidDataException();
    }
    m_header2 = hasCrc() ? reader.readUInt56BE() : (reader.readUInt40BE() << 2);
}

} // namespace Media

