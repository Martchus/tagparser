#include "./abstractchapter.h"
#include "./progressfeedback.h"

#include <sstream>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/// \brief The AbstractChapterPrivate struct contains private fields of the AbstractChapter class.
struct AbstractChapterPrivate {};

/*!
 * \class TagParser::AbstractChapter
 * \brief The AbstractChapter class parses chapter information.
 */

/*!
 * \brief Constructs a new chapter.
 */
AbstractChapter::AbstractChapter()
    : m_id(0)
    , m_startTime(-1)
    , m_endTime(-1)
    , m_hidden(false)
    , m_enabled(true)
{
}

/*!
 * \brief Destroys the chapter.
 */
AbstractChapter::~AbstractChapter()
{
}

/*!
 * \brief Returns a label for the chapter.
 */
string AbstractChapter::label() const
{
    stringstream ss;
    ss << "ID: " << id();
    if (!names().empty()) {
        ss << ", name: \"" << names().front() << "\"";
    }
    if (!startTime().isNegative()) {
        ss << ", start: " << startTime().toString(TimeSpanOutputFormat::WithMeasures);
    }
    return ss.str();
}

/*!
 * \brief Resets the object to its initial state.
 */
void AbstractChapter::clear()
{
    m_id = 0;
    m_names.clear();
    m_startTime = m_endTime = TimeSpan(-1);
    m_tracks.clear();
    m_hidden = false;
    m_enabled = true;
}

/*!
 * \brief Parses the chapter.
 *
 * Fetches nested chapters but does not parse them.
 *
 * Clears all previous parsing results.
 */
void AbstractChapter::parse(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    clear();
    internalParse(diag, progress);
}

/*!
 * \brief Parses the chapter and nested chapters recursively.
 *
 * Clears all previous parsing results.
 */
void AbstractChapter::parseNested(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    progress.stopIfAborted();
    clear();
    internalParse(diag, progress);
    for (size_t i = 0, count = nestedChapterCount(); i < count; ++i) {
        nestedChapter(i)->parseNested(diag, progress);
    }
}

/*!
 * \fn AbstractChapter::internalParse()
 * \brief Internally called to parse the chapter.
 *
 * Must be implemented when subclassing.
 *
 * \throws Throws Failure or a derived class when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */

} // namespace TagParser
