#include "./matroskatagid.h"

using namespace std;

namespace TagParser {

/*!
 * \brief Encapsulates Matroska tag IDs.
 * \sa See https://matroska.org/technical/specs/tagging/index.html for a complete list of the officially supported Matroska tag names.
 */
namespace MatroskaTagIds {

/*!
 * \brief Encapsulates track-specific Matroska tag IDs written by mkvmerge 7.0.0 or newer.
 * \sa https://github.com/mbunkus/mkvtoolnix/wiki/Automatic-tag-generation
 */
namespace TrackSpecific {
}
} // namespace MatroskaTagIds

} // namespace TagParser
