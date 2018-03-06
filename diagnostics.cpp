#include "./diagnostics.h"

using namespace std;

namespace TagParser {

const char *diagLevelName(DiagLevel diagLevel)
{
    switch(diagLevel) {
    case DiagLevel::Information:
        return "information";
    case DiagLevel::Warning:
        return "warning";
    case DiagLevel::Critical:
        return "critical";
    case DiagLevel::None:
    default:
        return "";
    }
}

bool Diagnostics::has(DiagLevel level) const
{
    for (const auto &msg : *this) {
        if (msg.level() >= level) {
            return true;
        }
    }
    return false;
}

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

}

