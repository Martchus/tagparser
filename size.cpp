#include "./size.h"

namespace TagParser {

/*!
 * \brief Returns an abbreviation for the current instance, eg. 720p for sizes greather than 1280×720
 *        and 1080p for sizes greather than 1920×1080.
 */
const char *Size::abbreviation() const
{
    if (*this >= Size(7680, 4320)) {
        return "8k";
    } else if (*this >= Size(3840, 2160)) {
        return "4k";
    } else if (*this >= Size(1920, 1080)) {
        return "1080p";
    } else if (*this >= Size(1280, 720)) {
        return "720p";
    } else if (*this >= Size(704, 576)) {
        return "576p";
    } else if (*this >= Size(640, 480)) {
        return "480p";
    } else if (*this >= Size(480, 320)) {
        return "320p";
    } else if (*this >= Size(320, 240)) {
        return "240p";
    }
    return "<240p";
}

} // namespace TagParser
