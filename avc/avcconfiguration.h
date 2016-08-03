#ifndef AVCCONFIGURATION_H
#define AVCCONFIGURATION_H

#include "./avcinfo.h"

#include <vector>

namespace Media {

class MediaFormat;

struct LIB_EXPORT AvcConfiguration
{
    AvcConfiguration();
    byte profileIndication;
    byte profileCompat;
    byte levelIndication;
    byte naluSizeLength;
    std::vector<SpsInfo> spsInfos;
    std::vector<PpsInfo> ppsInfos;

    void parse(IoUtilities::BinaryReader &reader, uint64 maxSize);
};

/*!
 * \brief Constructs an empty AVC configuration.
 */
inline AvcConfiguration::AvcConfiguration() :
    profileIndication(0),
    profileCompat(0),
    levelIndication(0),
    naluSizeLength(0)
{}

}

#endif // AVCCONFIGURATION_H
