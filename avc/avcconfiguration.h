#ifndef TAG_PARSER_AVCCONFIGURATION_H
#define TAG_PARSER_AVCCONFIGURATION_H

#include "./avcinfo.h"

#include <vector>

namespace TagParser {

class MediaFormat;
class Diagnostics;

struct TAG_PARSER_EXPORT AvcConfiguration {
    AvcConfiguration();
    byte profileIndication;
    byte profileCompat;
    byte levelIndication;
    byte naluSizeLength;
    std::vector<SpsInfo> spsInfos;
    std::vector<PpsInfo> ppsInfos;

    void parse(IoUtilities::BinaryReader &reader, uint64 maxSize, Diagnostics &diag);
};

/*!
 * \brief Constructs an empty AVC configuration.
 */
inline AvcConfiguration::AvcConfiguration()
    : profileIndication(0)
    , profileCompat(0)
    , levelIndication(0)
    , naluSizeLength(0)
{
}

} // namespace TagParser

#endif // TAG_PARSER_AVCCONFIGURATION_H
