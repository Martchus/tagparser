#include "matroskatagid.h"

#include <string>

using namespace std;

namespace Media {

namespace MatroskaTagIds {

}

/*!
 * \brief Returns a string for the specified \a targetTypeValue if
 *        known; otherwise returns an empty string.
 */
const char *matroskaTargetTypeName(uint32 targetTypeValue)
{
    if(targetTypeValue >= 70) {
        return "collection";
    } else if(targetTypeValue >= 60) {
        return "edition, issue, volume, opus, season, sequel";
    } else if(targetTypeValue >= 50) {
        return "album, opera, concert, movie, episode";
    } else if(targetTypeValue >= 40) {
        return "part, session";
    } else if(targetTypeValue >= 30) {
        return "track, song, chapter";
    } else if(targetTypeValue >= 20) {
        return "subtrack, part, movement, scene";
    } else if(targetTypeValue >= 10) {
        return "shot";
    } else {
        return "";
    }
}

} // namespace Media
