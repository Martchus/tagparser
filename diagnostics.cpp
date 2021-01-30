#include "./diagnostics.h"

using namespace std;

namespace TagParser {

/*!
 * \class DiagMessage
 * \brief The DiagMessage class holds an information, warning or error gathered during parsing or making.
 */

/*!
 * \class Diagnostics
 * \brief The Diagnostics class is a container for DiagMessage.
 * \remarks A lot of methods in this library take such a container as argument. The method will add additional
 *          information, warnings or errors to it.
 */

/*!
 * \brief Returns the string representation of the specified \a diagLevel.
 */
std::string_view diagLevelName(DiagLevel diagLevel)
{
    switch (diagLevel) {
    case DiagLevel::Information:
        return "information";
    case DiagLevel::Warning:
        return "warning";
    case DiagLevel::Critical:
        return "critical";
    case DiagLevel::Debug:
        return "debug";
    case DiagLevel::None:
    default:
        return std::string_view();
    }
}

/*!
 * \brief Returns whether there's at least one DiagMessage which is at least as worse as \a level.
 */
bool Diagnostics::has(DiagLevel level) const
{
    for (const auto &msg : *this) {
        if (msg.level() >= level) {
            return true;
        }
    }
    return false;
}

/*!
 * \brief Returns the worst diag level present in the container.
 */
DiagLevel Diagnostics::level() const
{
    auto level = DiagLevel::None;
    for (const auto &msg : *this) {
        if ((level |= msg.level()) >= worstDiagLevel) {
            return level;
        }
    }
    return level;
}

/*!
 * \brief Concatenates the specified string \a values to a list.
 */
string DiagMessage::formatList(const std::vector<string> &values)
{
    auto size = values.size() * 4;
    for (const auto &str : values) {
        size += str.size();
    }
    std::string res;
    res.reserve(size);
    for (auto value = values.cbegin(), end = values.cend(), last = values.cend() - 1; value != end; ++value) {
        if (value == last) {
            res += " and ";
        } else if (!res.empty()) {
            res += ", ";
        }
        res += '\"';
        res += *value;
        res += '\"';
    }
    return res;
}

} // namespace TagParser
