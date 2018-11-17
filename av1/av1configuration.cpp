#include "./av1configuration.h"

#include "../diagnostics.h"
#include "../exceptions.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/io/binaryreader.h>

using namespace std;
using namespace ConversionUtilities;
using namespace IoUtilities;

namespace TagParser {

/*!
 * \class Av1Configuration
 * \brief The Av1Configuration struct provides a parser for AV1 configuration found in ISOBMFF files.
 */

/*!
 * \brief Parses the AV1 configuration using the specified \a reader.
 * \throws Throws TruncatedDataException() when the config size exceeds the specified \a maxSize.
 * \remarks Logging/reporting parsing errors is not implemented yet.
 * \todo Read all fields.
 */
void Av1Configuration::parse(BinaryReader &reader, uint64 maxSize, Diagnostics &diag)
{
    if (maxSize < 4) {
        throw TruncatedDataException();
    }
    markerAndVersion = reader.readUInt32BE();
    if (marker() != 1) {
        diag.emplace_back(DiagLevel::Warning, argsToString("Marker is ", marker(), " (and not 1)."), "parsing AV1 config");
    }
    if (version() != 1) {
        diag.emplace_back(DiagLevel::Warning, argsToString("Version ", version(), " is not supported (only 1)."), "parsing AV1 config");
    }
}

} // namespace TagParser
