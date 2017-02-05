#include "./matroskachapter.h"
#include "./ebmlelement.h"
#include "./matroskaid.h"

#include <c++utilities/conversion/stringbuilder.h>

#include <memory>

using namespace std;
using namespace ChronoUtilities;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class MatroskaChapter
 * \brief The MatroskaChapter class provides an implementation of AbstractAttachment for Matroska files.
 */

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
 * \remarks
 * - Fetches nested chapters but does not parse them.
 * - Clears all previous parsing results.
 */

void MatroskaChapter::internalParse()
{
    // clear previous values and status
    static const string context("parsing \"ChapterAtom\"-element");
    invalidateStatus();
    clear();
    // iterate through childs of "ChapterAtom"-element
    for(EbmlElement *chapterAtomChild = m_chapterAtomElement->firstChild(); chapterAtomChild; chapterAtomChild = chapterAtomChild->nextSibling()) {
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
            for(EbmlElement *chapterTrackElement = chapterAtomChild->firstChild(); chapterTrackElement; chapterTrackElement = chapterTrackElement->nextSibling()) {
                chapterTrackElement->parse();
                switch(chapterTrackElement->id()) {
                case MatroskaIds::ChapterTrack:
                    m_tracks.emplace_back(chapterTrackElement->readUInteger());
                    break;
                default:
                    addNotification(NotificationType::Warning, "\"ChapterTrack\"-element contains unknown child element \"" % chapterAtomChild->idToString() + "\". It will be ignored.", context);
                }
            }
            break;
        case MatroskaIds::ChapterDisplay:
            m_names.emplace_back();
            for(EbmlElement *chapterDisplayElement = chapterAtomChild->firstChild(); chapterDisplayElement; chapterDisplayElement = chapterDisplayElement->nextSibling()) {
                chapterDisplayElement->parse();
                switch(chapterDisplayElement->id()) {
                case MatroskaIds::ChapString:
                    if(m_names.back().empty()) {
                        m_names.back().assign(chapterDisplayElement->readString());
                    } else {
                        addNotification(NotificationType::Warning, "\"ChapterDisplay\"-element contains multiple \"ChapString\"-elements. Surplus occurrences will be ignored.", context);
                    }
                    break;
                case MatroskaIds::ChapLanguage:
                    m_names.back().languages().emplace_back(chapterDisplayElement->readString());
                    break;
                case MatroskaIds::ChapCountry:
                    m_names.back().countries().emplace_back(chapterDisplayElement->readString());
                    break;
                }
            }
            break;
        case MatroskaIds::ChapProcess:
            break;
        case MatroskaIds::ChapterAtom:
            m_nestedChapters.emplace_back(make_unique<MatroskaChapter>(chapterAtomChild));
        default:
            addNotification(NotificationType::Warning, "\"ChapterAtom\"-element contains unknown child element \"" % chapterAtomChild->idToString() + "\". It will be ignored.", context);
        }
    }
    // "eng" is default language
    for(LocaleAwareString &name : m_names) {
        if(name.languages().empty()) {
            name.languages().emplace_back("eng");
        }
    }
}

void MatroskaChapter::clear()
{
    AbstractChapter::clear();
    m_nestedChapters.clear();
}

} // namespace Media

