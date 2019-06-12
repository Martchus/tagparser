#include "./ivfframe.h"

#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::IvfFrame
 * \brief The IvfFrame class is used to parse IVF frames.
 * \sa https://wiki.multimedia.cx/index.php/IVF
 */

/*!
 * \brief Parses the header read using the specified \a reader.
 */
void IvfFrame::parseHeader(CppUtilities::BinaryReader &reader, Diagnostics &diag)
{
    CPP_UTILITIES_UNUSED(diag)
    startOffset = static_cast<std::uint64_t>(reader.stream()->tellg());
    size = reader.readUInt32BE();
    timestamp = reader.readUInt64BE();
}

} // namespace TagParser
