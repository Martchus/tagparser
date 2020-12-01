#include "./avcconfiguration.h"

#include "../diagnostics.h"
#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/io/binaryreader.h>

#include <limits>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class AvcConfiguration
 * \brief The AvcConfiguration struct provides a parser for AVC configuration.
 */

/*!
 * \brief Parses the AVC configuration using the specified \a reader.
 * \throws Throws TruncatedDataException() when the config size exceeds the specified \a maxSize.
 */
void AvcConfiguration::parse(BinaryReader &reader, std::uint64_t maxSize, Diagnostics &diag)
{
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
    std::uint8_t spsEntryCount = reader.readByte() & 0x0f;
    std::uint8_t ignoredSpsEntries = 0;
    spsInfos.reserve(spsEntryCount);
    for (; spsEntryCount; --spsEntryCount) {
        if (maxSize < SpsInfo::minSize) {
            throw TruncatedDataException();
        }
        auto error = false;
        try {
            spsInfos.emplace_back().parse(
                reader, maxSize > numeric_limits<std::uint32_t>::max() ? numeric_limits<std::uint32_t>::max() : static_cast<std::uint32_t>(maxSize));
        } catch (const TruncatedDataException &) {
            if (spsInfos.back().size > (maxSize - SpsInfo::minSize)) {
                throw; // sps info looks bigger than bytes to read
            }
            error = true; // sps info exceeds denoted size
        } catch (const Failure &) {
            error = true;
        }
        maxSize -= spsInfos.back().size;
        if (error) {
            spsInfos.pop_back();
            ++ignoredSpsEntries;
        }
    }

    // read PPS info entries
    std::uint8_t ppsEntryCount = reader.readByte();
    std::uint8_t ignoredPpsEntries = 0;
    ppsInfos.reserve(ppsEntryCount);
    for (; ppsEntryCount; --ppsEntryCount) {
        if (maxSize < PpsInfo::minSize) {
            throw TruncatedDataException();
        }
        auto error = false;
        try {
            ppsInfos.emplace_back().parse(
                reader, maxSize > numeric_limits<std::uint32_t>::max() ? numeric_limits<std::uint32_t>::max() : static_cast<std::uint32_t>(maxSize));
        } catch (const TruncatedDataException &) {
            if (ppsInfos.back().size > (maxSize - PpsInfo::minSize)) {
                throw; // pps info looks bigger than bytes to read
            }
            error = true; // pps info exceeds denoted size
        } catch (const Failure &) {
            error = true;
        }
        maxSize -= ppsInfos.back().size;
        if (error) {
            ppsInfos.pop_back();
            ++ignoredPpsEntries;
        }
    }

    // log parsing errors
    if (ignoredSpsEntries || ignoredPpsEntries) {
        diag.emplace_back(DiagLevel::Debug,
            argsToString(
                "Ignored ", ignoredSpsEntries, " SPS entries and ", ignoredPpsEntries, " PPS entries. This AVC config is likely just not supported."),
            "parsing AVC config");
    }

    // ignore remaining data
}

} // namespace TagParser
