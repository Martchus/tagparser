#include "./matroskaeditionentry.h"
#include "./ebmlelement.h"
#include "./matroskaid.h"

#include "../diagnostics.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>

#include <memory>
#include <string>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class MatroskaEditionEntry
 * \brief The MatroskaEditionEntry class provides a parser for edition entries in Matroska files.
 */

/*!
 * \brief Constructs a new MatroskaEditionEntry for the specified \a editionEntryElement.
 */
MatroskaEditionEntry::MatroskaEditionEntry(EbmlElement *editionEntryElement)
    : m_editionEntryElement(editionEntryElement)
    , m_id(0)
    , m_hidden(false)
    , m_default(false)
    , m_ordered(false)
{
}

/*!
 * \brief Destroys the MatroskaEditionEntry.
 */
MatroskaEditionEntry::~MatroskaEditionEntry()
{
}

/*!
 * \brief Returns a label for the entry.
 */
string MatroskaEditionEntry::label() const
{
    return argsToString("ID: ", id());
}

/*!
 * \brief Parses the "EditionEntry"-element specified when constructing the object.
 *
 * Fetches the chapters() but does not parse them.
 *
 * Clears all previous parsing results.
 */
void MatroskaEditionEntry::parse(Diagnostics &diag)
{
    // clear previous values and status
    static const string context("parsing \"EditionEntry\"-element");
    clear();
    // iterate through children of "EditionEntry"-element
    EbmlElement *entryChild = m_editionEntryElement->firstChild();
    while (entryChild) {
        entryChild->parse(diag);
        switch (entryChild->id()) {
        case MatroskaIds::EditionUID:
            m_id = entryChild->readUInteger();
            break;
        case MatroskaIds::EditionFlagHidden:
            m_hidden = entryChild->readUInteger() == 1;
            break;
        case MatroskaIds::EditionFlagDefault:
            m_default = entryChild->readUInteger() == 1;
            break;
        case MatroskaIds::EditionFlagOrdered:
            m_ordered = entryChild->readUInteger() == 1;
            break;
        case MatroskaIds::ChapterAtom:
            m_chapters.emplace_back(make_unique<MatroskaChapter>(entryChild));
            break;
        default:
            diag.emplace_back(DiagLevel::Warning,
                "\"EditionEntry\"-element contains unknown child element \"" % entryChild->idToString() + "\" which will be ignored.", context);
        }
        entryChild = entryChild->nextSibling();
    }
}

/*!
 * \brief Parses the "EditionEntry"-element specified when constructing the object.
 * \remarks
 * - Parses also fetched chapters and nested chapters.
 * - Clears all previous parsing results.
 */
void MatroskaEditionEntry::parseNested(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    progress.stopIfAborted();
    parse(diag);
    for (auto &chapter : chapters()) {
        chapter->parseNested(diag, progress);
    }
}

/*!
 * \brief Resets the object to its initial state.
 */
void MatroskaEditionEntry::clear()
{
    m_id = 0;
    m_hidden = m_default = m_ordered = false;
    m_chapters.clear();
}

} // namespace TagParser
