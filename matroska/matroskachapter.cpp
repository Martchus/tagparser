#include "./matroskachapter.h"
#include "./ebmlelement.h"
#include "./matroskaid.h"

#include <c++utilities/misc/memory.h>

using namespace std;
using namespace ChronoUtilities;

namespace Media {

/*!
 * \brief Constructs a new MatroskaChapter for the specified \a chapterAtomElement.
 */
MatroskaChapter::MatroskaChapter(EbmlElement *chapterAtomElement) :
    m_chapterAtomElement(chapterAtomElement)
{}

/*!
 * \brief Destroys the chapter.
 */
MatroskaChapter::~MatroskaChapter()
{}

/*!
 * \fn MatroskaChapter::parse()
 * \brief Parses the "ChapterAtom"-element which has been specified when constructing the object.
 *
 * Fetches nested chapters but does not parse them.
 *
 * Clears all previous parsing results.
 */

void MatroskaChapter::internalParse()
{
    // clear previous values and status
    static const string context("parsing \"ChapterAtom\"-element");
    invalidateStatus();
    clear();
    // iterate through childs of "ChapterAtom"-element
    EbmlElement *chapterAtomChild = m_chapterAtomElement->firstChild();
    EbmlElement *subElement;
    while(chapterAtomChild) {
        chapterAtomChild->parse();
        switch(chapterAtomChild->id()) {
        case MatroskaIds::ChapterUID:
            m_id = chapterAtomChild->readUInteger();
            break;
        case MatroskaIds::ChapterStringUID:
            break;
        case MatroskaIds::ChapterTimeStart:
            m_startTime = TimeSpan(chapterAtomChild->readUInteger() / 100);
            break;
        case MatroskaIds::ChapterTimeEnd:
            m_endTime = TimeSpan(chapterAtomChild->readUInteger() / 100);
            break;
        case MatroskaIds::ChapterFlagHidden:
            m_hidden = chapterAtomChild->readUInteger() == 1;
            break;
        case MatroskaIds::ChapterFlagEnabled:
            m_enabled = chapterAtomChild->readUInteger() == 1;
            break;
        case MatroskaIds::ChapterSegmentUID:
        case MatroskaIds::ChapterSegmentEditionUID:
        case MatroskaIds::ChapterPhysicalEquiv:
            break;
        case MatroskaIds::ChapterTrack:
            subElement = chapterAtomChild->firstChild();
            while(subElement) {
                subElement->parse();
                switch(subElement->id()) {
                case MatroskaIds::ChapterTrack:
                    m_tracks.emplace_back(subElement->readUInteger());
                    break;
                default:
                    addNotification(NotificationType::Warning, "\"ChapterTrack\"-element contains unknown child element \"" + chapterAtomChild->idToString() + "\". It will be ignored.", context);
                }
                subElement = subElement->nextSibling();
            }
            break;
        case MatroskaIds::ChapterDisplay:
            subElement = chapterAtomChild->firstChild();
            m_names.emplace_back();
            while(subElement) {
                subElement->parse();
                switch(subElement->id()) {
                case MatroskaIds::ChapString:
                    if(m_names.back().empty()) {
                        m_names.back().assign(subElement->readString());
                    } else {
                        addNotification(NotificationType::Warning, "\"ChapterDisplay\"-element contains multiple \"ChapString\"-elements. Surplus occurrences will be ignored.", context);
                    }
                    break;
                case MatroskaIds::ChapLanguage:
                    m_names.back().languages().emplace_back(subElement->readString());
                    break;
                case MatroskaIds::ChapCountry:
                    m_names.back().countries().emplace_back(subElement->readString());
                    break;
                }
                subElement = subElement->nextSibling();
            }
            break;
        case MatroskaIds::ChapProcess:
            break;
        case MatroskaIds::ChapterAtom:
            m_nestedChapters.emplace_back(make_unique<MatroskaChapter>(chapterAtomChild));
        default:
            addNotification(NotificationType::Warning, "\"ChapterAtom\"-element contains unknown child element \"" + chapterAtomChild->idToString() + "\". It will be ignored.", context);
        }
        chapterAtomChild = chapterAtomChild->nextSibling();
    }
    // "eng" is declared to be default language
    if(m_names.back().languages().empty()) {
        m_names.back().languages().emplace_back("eng");
    }
}

void MatroskaChapter::clear()
{
    AbstractChapter::clear();
    m_nestedChapters.clear();
}

} // namespace Media

