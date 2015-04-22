#ifndef AVCCONFIGURATION_H
#define AVCCONFIGURATION_H

#include "avcinfo.h"

#include <vector>

namespace Media {

struct LIB_EXPORT AvcConfiguration
{
    AvcConfiguration();
    byte profileIdc;
    byte profileCompat;
    byte levelIdc;
    byte naluSizeLength;
    std::vector<SpsInfo> spsInfos;
    std::vector<PpsInfo> ppsInfos;
};

inline AvcConfiguration::AvcConfiguration() :
    profileIdc(0),
    profileCompat(0),
    levelIdc(0),
    naluSizeLength(0)
{}

}

#endif // AVCCONFIGURATION_H
