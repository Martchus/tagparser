#include "./tagtarget.h"

#include "./matroska/matroskatagid.h"

#include <c++utilities/conversion/stringconversion.h>

#include <list>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \brief Returns a string representation for the specified \a tagTargetLevel.
 */
std::string_view tagTargetLevelName(TagTargetLevel tagTargetLevel)
{
    switch (tagTargetLevel) {
    case TagTargetLevel::Shot:
        return "shot";
    case TagTargetLevel::Subtrack:
        return "subtrack, part, movement, scene";
    case TagTargetLevel::Track:
        return "track, song, chapter";
    case TagTargetLevel::Part:
        return "part, session";
    case TagTargetLevel::Album:
        return "album, opera, concert, movie, episode";
    case TagTargetLevel::Edition:
        return "edition, issue, volume, opus, season, sequel";
    case TagTargetLevel::Collection:
        return "collection";
    default:
        return std::string_view();
    }
}

/*!
 * \class TagParser::TagTarget
 * \brief The TagTarget class specifies the target of a tag.
 *
 * Tags might only target a specific track, chapter, ...
 *
 * Specifying a target is currently only fully supported by Matroska.
 *
 * Since Ogg saves tags at stream level, the stream can be specified
 * by passing a TagTarget instance to OggContainer::createTag().
 * However, only the first track in the tracks() array is considered
 * and any other values are just ignored.
 *
 * In any other tag formats, the specified target is (currently)
 * completely ignored.
 */

/*!
 * \brief Returns the string representation of the current instance.
 * \remarks Uses the specified \a tagTargetLevel if no levelName() is assigned.
 */
std::string TagTarget::toString(TagTargetLevel tagTargetLevel) const
{
    auto levelString = std::string();
    if (level()) {
        levelString += "level ";
        levelString += numberToString(level());
    }
    auto defaultLevelName = std::string_view();
    if (!levelName().empty() || !(defaultLevelName = tagTargetLevelName(tagTargetLevel)).empty()) {
        if (!levelString.empty()) {
            levelString += ' ';
        }
        levelString += '\'';
        if (!levelName().empty()) {
            levelString += levelName();
        } else {
            levelString += defaultLevelName;
        }
        levelString += '\'';
    }
    list<string> parts;
    if (levelString.empty()) {
        parts.emplace_back("undefined target");
    } else {
        parts.emplace_back(std::move(levelString));
    }
    for (auto v : tracks()) {
        parts.emplace_back("track " + numberToString(v));
    }
    for (auto v : chapters()) {
        parts.emplace_back("chapter " + numberToString(v));
    }
    for (auto v : editions()) {
        parts.emplace_back("edition " + numberToString(v));
    }
    for (auto v : attachments()) {
        parts.emplace_back("attachment  " + numberToString(v));
    }
    return joinStrings(parts, ", ");
}

} // namespace TagParser
