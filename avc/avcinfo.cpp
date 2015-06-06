#include "avcinfo.h"

#include <unordered_map>

namespace Media {

/*!
 * \struct Media::SpsInfo
 * \brief The SpsInfo struct holds the sequence parameter set.
 */

/*!
 * \brief SpsInfo::parse
 * \param stream
 */
void SpsInfo::parse(std::istream &stream)
{
    static auto highLevelProfileIds = std::unordered_map<unsigned int, bool> {
        { 44, true }, { 83, true }, { 86, true }, { 100, true }, { 110, true },
        { 118, true }, { 122, true }, { 128, true }, { 244, true }
    };
    int const addSpace = 100;

    parNum = 1;
    parDen = 1;
    arFound = false;

    uint32 unitsInTick = 0;
    uint32 timeScale = 0;


}



}
