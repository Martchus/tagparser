#include "./avcconfiguration.h"

#include "../exceptions.h"
#include "../mediaformat.h"

#include <c++utilities/io/binaryreader.h>

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
 * \remarks Logging/reporting parsing errors is not implemented yet.
 */
void AvcConfiguration::parse(BinaryReader &reader, uint64 maxSize)
{
    if(maxSize < 7) {
        throw TruncatedDataException();
    }
    maxSize -= 7;

    reader.stream()->seekg(1, ios_base::cur); // always 1
    profileIndication = reader.readByte();
    profileCompat = reader.readByte();
    levelIndication = reader.readByte();
    naluSizeLength = (reader.readByte() & 0x03) + 1;

    // read SPS info entries
    byte entryCount = reader.readByte() & 0x0f;
    spsInfos.reserve(entryCount);
    for(; entryCount; --entryCount) {
        if(maxSize < 2) {
            throw TruncatedDataException();
        }
        spsInfos.emplace_back();
        try {
            spsInfos.back().parse(reader, maxSize);
        } catch(const TruncatedDataException &) {
            // TODO: log parsing error
            if(spsInfos.back().size > maxSize - 2) {
                throw;
            }
            spsInfos.pop_back();
        } catch(const Failure &) {
            spsInfos.pop_back();
            // TODO: log parsing error
        }
        maxSize -= spsInfos.back().size;
    }

    // read PPS info entries
    entryCount = reader.readByte();
    ppsInfos.reserve(entryCount);
    for(; entryCount; --entryCount) {
        if(maxSize < 2) {
            throw TruncatedDataException();
        }
        ppsInfos.emplace_back();
        try {
            ppsInfos.back().parse(reader, maxSize);
        } catch(const TruncatedDataException &) {
            // TODO: log parsing error
            if(ppsInfos.back().size > maxSize - 2) {
                throw;
            }
            ppsInfos.pop_back();
        } catch(const Failure &) {
            ppsInfos.pop_back();
            // TODO: log parsing error
        }
        maxSize -= ppsInfos.back().size;
    }

    // ignore remaining data
}

}
