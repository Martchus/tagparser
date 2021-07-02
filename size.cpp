#include "./size.h"

namespace TagParser {

/// \cond
constexpr Size fromHeightAndAspectRatio(std::uint32_t height, std::uint32_t numerator = 4, std::uint32_t denominator = 3)
{
    return Size(height * numerator / denominator, height);
}
/// \endcond

/*!
 * \brief Returns an abbreviation for the current instance, eg. 720p for sizes greater than 960×720
 *        and 1080p for sizes greater than 1440×1080.
 * \remarks The width thresolds are for 4:3 resolutions so both, 4:3 and 16:9 "720p" is considered as such.
 */
std::string_view Size::abbreviation() const
{
    if (*this >= fromHeightAndAspectRatio(4320)) {
        return "8k";
    } else if (*this >= fromHeightAndAspectRatio(2160)) {
        return "4k";
    } else if (*this >= fromHeightAndAspectRatio(1080)) {
        return "1080p";
    } else if (*this >= fromHeightAndAspectRatio(720)) {
        return "720p";
    } else if (*this >= fromHeightAndAspectRatio(576)) {
        return "576p";
    } else if (*this >= fromHeightAndAspectRatio(480)) {
        return "480p";
    } else if (*this >= fromHeightAndAspectRatio(320)) {
        return "320p";
    } else if (*this >= fromHeightAndAspectRatio(240)) {
        return "240p";
    }
    return "<240p";
}

} // namespace TagParser
