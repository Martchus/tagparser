#include "./av1configuration.h"

#include "../diagnostics.h"
#include "../exceptions.h"

#include <c++utilities/io/binaryreader.h>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class Av1Configuration
 * \brief The Av1Configuration struct provides a parser for AV1 configuration found in ISOBMFF files.
 */

/*!
 * \brief Parses the AV1 configuration using the specified \a reader.
 * \throws Throws TruncatedDataException() when the config size exceeds the specified \a maxSize.
 * \remarks Logging/reporting parsing errors is not implemented yet.
 * \todo Provide implementation
 */
void Av1Configuration::parse(BinaryReader &reader, std::uint64_t maxSize, Diagnostics &diag)
{
    CPP_UTILITIES_UNUSED(reader)
    CPP_UTILITIES_UNUSED(maxSize)
    CPP_UTILITIES_UNUSED(diag)
    throw NotImplementedException();
}

} // namespace TagParser
