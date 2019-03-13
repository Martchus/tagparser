#include "./avcconfiguration.h"

#include "../diagnostics.h"
#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/io/binaryreader.h>

#include <limits>

using namespace std;
using namespace IoUtilities;

namespace TagParser {

/*!
 * \class AvcConfiguration
 * \brief The AvcConfiguration struct provides a parser for AVC configuration.
 */

/*!
 * \brief Parses the AVC configuration using the specified \a reader.
 * \throws Throws TruncatedDataException() when the config size exceeds the specified \a maxSize.
 * \todo Implement logging/reporting parsing errors.
 */
void AvcConfiguration::parse(BinaryReader &reader, std::uint64_t maxSize, Diagnostics &diag)
{
    VAR_UNUSED(diag)
    if (maxSize < 7) {
        throw TruncatedDataException();
    }
    maxSize -= 7;

    reader.stream()->seekg(1, ios_base::cur); // always 1
    profileIndication = reader.readByte();
    profileCompat = reader.readByte();
    levelIndication = reader.readByte();
    naluSizeLength = (reader.readByte() & 0x03) + 1;

    // read SPS info entries
    std::uint8_t entryCount = reader.readByte() & 0x0f;
    spsInfos.reserve(entryCount);
    for (; entryCount; --entryCount) {
        if (maxSize < 2) {
            throw TruncatedDataException();
        }
        spsInfos.emplace_back();
        try {
            spsInfos.back().parse(
                reader, maxSize > numeric_limits<std::uint32_t>::max() ? numeric_limits<std::uint32_t>::max() : static_cast<std::uint32_t>(maxSize));
        } catch (const TruncatedDataException &) {
            if (spsInfos.back().size > maxSize - 2) {
                throw;
            }
            spsInfos.pop_back();
        } catch (const Failure &) {
            spsInfos.pop_back();
            // TODO: log parsing error
        }
        maxSize -= spsInfos.back().size;
    }

    // read PPS info entries
    entryCount = reader.readByte();
    ppsInfos.reserve(entryCount);
    for (; entryCount; --entryCount) {
        if (maxSize < 2) {
            throw TruncatedDataException();
        }
        ppsInfos.emplace_back();
        try {
            ppsInfos.back().parse(
                reader, maxSize > numeric_limits<std::uint32_t>::max() ? numeric_limits<std::uint32_t>::max() : static_cast<std::uint32_t>(maxSize));
        } catch (const TruncatedDataException &) {
            if (ppsInfos.back().size > maxSize - 2) {
                throw;
            }
            ppsInfos.pop_back();
        } catch (const Failure &) {
            ppsInfos.pop_back();
            // TODO: log parsing error
        }
        maxSize -= ppsInfos.back().size;
    }

    // ignore remaining data
}

} // namespace TagParser
