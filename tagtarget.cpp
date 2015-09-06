#include "./tagtarget.h"

#include "./matroska/matroskatagid.h"

#include <c++utilities/conversion/stringconversion.h>

#include <list>

using namespace std;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \brief Returns the string representation of the current instance.
 */
string TagTarget::toString() const
{
    list<string> parts;
    parts.emplace_back();
    string &level = parts.back();
    if(this->level()) {
        level.append("level " + numberToString(this->level()));
    }
    string name;
    if(!levelName().empty()) {
        name = levelName();
    } else {
        name = matroskaTargetTypeName(this->level());
    }
    if(!name.empty()) {
        if(!level.empty()) {
            level.append(" ");
        }
        level.append("»");
        level.append(name);
        level.append("«");
    }
    if(level.empty()) {
        level.append("undefined target");
    }
    for(auto v : tracks()) {
        parts.emplace_back("track " + numberToString(v));
    }
    for(auto v : chapters()) {
        parts.emplace_back("chapter " + numberToString(v));
    }
    for(auto v : editions()) {
        parts.emplace_back("edition " + numberToString(v));
    }
    for(auto v : attachments()) {
        parts.emplace_back("attachment  " + numberToString(v));
    }
    return joinStrings(parts, ", ");
}

}
