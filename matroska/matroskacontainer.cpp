#include "./matroskacontainer.h"
#include "./ebmlid.h"
#include "./matroskaid.h"
#include "./matroskacues.h"
#include "./matroskaeditionentry.h"
#include "./matroskaseekinfo.h"

#include "../mediafileinfo.h"
#include "../exceptions.h"
#include "../backuphelper.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/misc/memory.h>

#include <functional>
#include <initializer_list>
#include <unordered_set>

using namespace std;
using namespace std::placeholders;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

namespace Media {

constexpr const char appInfo[] = APP_NAME " v" APP_VERSION;
constexpr uint64 appInfoElementDataSize = sizeof(appInfo) - 1;
constexpr uint64 appInfoElementTotalSize = 2 + 1 + appInfoElementDataSize;

/*!
 * \class Media::MatroskaContainer
 * \brief Implementation of GenericContainer<MediaFileInfo, MatroskaTag, MatroskaTrack, EbmlElement>.
 */

uint64 MatroskaContainer::m_maxFullParseSize = 0x3200000;

/*!
 * \brief Constructs a new container for the specified \a fileInfo at the specified \a startOffset.
 */
MatroskaContainer::MatroskaContainer(MediaFileInfo &fileInfo, uint64 startOffset) :
    GenericContainer<MediaFileInfo, MatroskaTag, MatroskaTrack, EbmlElement>(fileInfo, startOffset),
    m_maxIdLength(4),
    m_maxSizeLength(8)
{
    m_version = 1;
    m_readVersion = 1;
    m_doctype = "matroska";
    m_doctypeVersion = 1;
    m_doctypeReadVersion = 1;
}

/*!
 * \brief Destroys the container.
 */
MatroskaContainer::~MatroskaContainer()
{}

void MatroskaContainer::reset()
{
    GenericContainer<MediaFileInfo, MatroskaTag, MatroskaTrack, EbmlElement>::reset();
    m_maxIdLength = 4;
    m_maxSizeLength = 8;
    m_version = 1;
    m_readVersion = 1;
    m_doctype = "matroska";
    m_doctypeVersion = 1;
    m_doctypeReadVersion = 1;
    m_tracksElements.clear();
    m_segmentInfoElements.clear();
    m_tagsElements.clear();
    m_chaptersElements.clear();
    m_attachmentsElements.clear();
    m_seekInfos.clear();
    m_editionEntries.clear();
    m_attachments.clear();
}

/*!
 * \brief Validates the file index (cue entries).
 * \remarks Checks only for cluster positions and missing, unknown or surplus elements.
 */
void MatroskaContainer::validateIndex()
{
    static const string context("validating Matroska file index (cues)");
    bool cuesElementsFound = false;
    if(m_firstElement) {
        unordered_set<int> ids;
        bool cueTimeFound = false, cueTrackPositionsFound = false;
        unique_ptr<EbmlElement> clusterElement;
        uint64 pos, prevClusterSize = 0, currentOffset = 0;
        // iterate throught all segments
        for(EbmlElement *segmentElement = m_firstElement->siblingById(MatroskaIds::Segment); segmentElement; segmentElement = segmentElement->siblingById(MatroskaIds::Segment)) {
            segmentElement->parse();
            // iterate throught all child elements of the segment (only "Cues"- and "Cluster"-elements are relevant for this method)
            for(EbmlElement *segmentChildElement = segmentElement->firstChild(); segmentChildElement; segmentChildElement = segmentChildElement->nextSibling()) {
                segmentChildElement->parse();
                switch(segmentChildElement->id()) {
                case EbmlIds::Void:
                case EbmlIds::Crc32:
                    break;
                case MatroskaIds::Cues:
                    cuesElementsFound = true;
                    // parse childs of "Cues"-element ("CuePoint"-elements)
                    for(EbmlElement *cuePointElement = segmentChildElement->firstChild(); cuePointElement; cuePointElement = cuePointElement->nextSibling()) {
                        cuePointElement->parse();
                        cueTimeFound = cueTrackPositionsFound = false; // to validate quantity of these elements
                        switch(cuePointElement->id()) {
                        case EbmlIds::Void:
                        case EbmlIds::Crc32:
                            break;
                        case MatroskaIds::CuePoint:
                            // parse childs of "CuePoint"-element
                            for(EbmlElement *cuePointChildElement = cuePointElement->firstChild(); cuePointChildElement; cuePointChildElement = cuePointChildElement->nextSibling()) {
                                cuePointChildElement->parse();
                                switch(cuePointChildElement->id()) {
                                case MatroskaIds::CueTime:
                                    // validate uniqueness
                                    if(cueTimeFound) {
                                        addNotification(NotificationType::Warning, "\"CuePoint\"-element contains multiple \"CueTime\" elements.", context);
                                    } else {
                                        cueTimeFound = true;
                                    }
                                    break;
                                case MatroskaIds::CueTrackPositions:
                                    cueTrackPositionsFound = true;
                                    ids.clear();
                                    clusterElement.reset();
                                    for(EbmlElement *subElement = cuePointChildElement->firstChild(); subElement; subElement = subElement->nextSibling()) {
                                        subElement->parse();
                                        switch(subElement->id()) {
                                        case MatroskaIds::CueTrack:
                                        case MatroskaIds::CueClusterPosition:
                                        case MatroskaIds::CueRelativePosition:
                                        case MatroskaIds::CueDuration:
                                        case MatroskaIds::CueBlockNumber:
                                        case MatroskaIds::CueCodecState:
                                            // validate uniqueness
                                            if(ids.count(subElement->id())) {
                                                addNotification(NotificationType::Warning, "\"CueTrackPositions\"-element contains multiple \"" + subElement->idToString() + "\" elements.", context);
                                            } else {
                                                ids.insert(subElement->id());
                                            }
                                            break;
                                        case EbmlIds::Crc32:
                                        case EbmlIds::Void:
                                        case MatroskaIds::CueReference:
                                            break;
                                        default:
                                            addNotification(NotificationType::Warning, "\"CueTrackPositions\"-element contains unknown element \"" + subElement->idToString() + "\".", context);
                                        }
                                        switch(subElement->id()) {
                                        case EbmlIds::Void:
                                        case EbmlIds::Crc32:
                                        case MatroskaIds::CueTrack:
                                            break;
                                        case MatroskaIds::CueClusterPosition:
                                            // validate "Cluster" position denoted by "CueClusterPosition"-element
                                            clusterElement = make_unique<EbmlElement>(*this, segmentElement->dataOffset() + subElement->readUInteger() - currentOffset);
                                            try {
                                                clusterElement->parse();
                                                if(clusterElement->id() != MatroskaIds::Cluster) {
                                                    addNotification(NotificationType::Critical, "\"CueClusterPosition\" element at " + numberToString(subElement->startOffset()) + " does not point to \"Cluster\"-element (points to " + numberToString(clusterElement->startOffset()) + ").", context);
                                                }
                                            } catch(Failure &) {
                                                addNotifications(context, *clusterElement);
                                            }
                                            break;
                                        case MatroskaIds::CueRelativePosition:
                                            // read "Block" position denoted by "CueRelativePosition"-element (validate later since the "Cluster"-element is needed to validate)
                                            pos = subElement->readUInteger();
                                            break;
                                        case MatroskaIds::CueDuration:
                                            break;
                                        case MatroskaIds::CueBlockNumber:
                                            break;
                                        case MatroskaIds::CueCodecState:
                                            break;
                                        case MatroskaIds::CueReference:
                                            break;
                                        default:
                                            ;
                                        }
                                    }
                                    // validate existence of mandatory elements
                                    if(!ids.count(MatroskaIds::CueTrack)) {
                                        addNotification(NotificationType::Warning, "\"CueTrackPositions\"-element does not contain mandatory element \"CueTrack\".", context);
                                    }
                                    if(!clusterElement) {
                                        addNotification(NotificationType::Warning, "\"CueTrackPositions\"-element does not contain mandatory element \"CueClusterPosition\".", context);
                                    } else {
                                        if(ids.count(MatroskaIds::CueRelativePosition)) {
                                            // validate "Block" position denoted by "CueRelativePosition"-element
                                            EbmlElement referenceElement(*this, clusterElement->dataOffset() + pos);
                                            try {
                                                referenceElement.parse();
                                                switch(referenceElement.id()) {
                                                case MatroskaIds::SimpleBlock:
                                                case MatroskaIds::Block:
                                                case MatroskaIds::BlockGroup:
                                                    break;
                                                default:
                                                    addNotification(NotificationType::Critical, "\"CueRelativePosition\" element does not point to \"Block\"-, \"BlockGroup\", or \"SimpleBlock\"-element (points to " + numberToString(referenceElement.startOffset()) + ").", context);
                                                }
                                            } catch(Failure &) {
                                                addNotifications(context, referenceElement);
                                            }
                                        }
                                    }
                                    break;
                                case EbmlIds::Crc32:
                                case EbmlIds::Void:
                                    break;
                                default:
                                    addNotification(NotificationType::Warning, "\"CuePoint\"-element contains unknown element \"" + cuePointElement->idToString() + "\".", context);
                                }
                            }
                            // validate existence of mandatory elements
                            if(!cueTimeFound) {
                                addNotification(NotificationType::Warning, "\"CuePoint\"-element does not contain mandatory element \"CueTime\".", context);
                            }
                            if(!cueTrackPositionsFound) {
                                addNotification(NotificationType::Warning, "\"CuePoint\"-element does not contain mandatory element \"CueClusterPosition\".", context);
                            }
                            break;
                        default:
                            ;
                        }
                    }
                    break;
                case MatroskaIds::Cluster:
                    // parse childs of "Cluster"-element
                    for(EbmlElement *clusterElementChild = segmentChildElement->firstChild(); clusterElementChild; clusterElementChild = clusterElementChild->nextSibling()) {
                        clusterElementChild->parse();
                        switch(clusterElementChild->id()) {
                        case EbmlIds::Void:
                        case EbmlIds::Crc32:
                            break;
                        case MatroskaIds::Position:
                            // validate position
                            if((pos = clusterElementChild->readUInteger()) > 0 && (segmentChildElement->startOffset() - segmentElement->dataOffset() + currentOffset) != pos) {
                                addNotification(NotificationType::Critical, "\"Position\"-element at " + numberToString(clusterElementChild->startOffset()) + " points to " + numberToString(pos) + " which is not the offset of the containing \"Cluster\"-element.", context);
                            }
                            break;
                        case MatroskaIds::PrevSize:
                            // validate prev size
                            if(clusterElementChild->readUInteger() != prevClusterSize) {
                                addNotification(NotificationType::Critical, "\"PrevSize\"-element at " + numberToString(clusterElementChild->startOffset()) + " has invalid value.", context);
                            }
                            break;
                        default:
                            ;
                        }
                    }
                    prevClusterSize = segmentChildElement->totalSize();
                    break;
                default:
                    ;
                }
            }
            currentOffset += segmentElement->totalSize();
        }
    }
    // add a warning when no index could be found
    if(!cuesElementsFound) {
        addNotification(NotificationType::Warning, "No \"Cues\"-elements (index) found.", context);
    }
}

/*!
 * \brief Returns an indication whether \a offset equals the start offset of \a element.
 */
bool sameOffset(uint64 offset, const EbmlElement *element) {
    return element->startOffset() == offset;
}

/*!
 * \brief Returns whether none of the specified \a elements have the specified \a offset.
 *
 * This method is used when gathering elements to avaoid adding the same element twice.
 */
inline bool excludesOffset(const vector<EbmlElement *> &elements, uint64 offset)
{
    return find_if(elements.cbegin(), elements.cend(), std::bind(sameOffset, offset, _1)) == elements.cend();
}

MatroskaChapter *MatroskaContainer::chapter(std::size_t index)
{
    size_t currentIndex = 0;
    for(auto &entry : m_editionEntries) {
        currentIndex += entry->chapters().size();
        if(index < currentIndex) {
            return entry->chapters()[index].get();
        }
    }
    return nullptr;
}

size_t MatroskaContainer::chapterCount() const
{
    size_t count = 0;
    for(const auto &entry : m_editionEntries) {
        count += entry->chapters().size();
    }
    return count;
}

MatroskaAttachment *MatroskaContainer::createAttachment()
{
    // generate unique ID
    srand (time(nullptr));
    byte tries = 0;
    uint64 attachmentId;
    generateRandomId:
    attachmentId = rand();
    if(tries < 0xFF) {
        for(const auto &attachment : m_attachments) {
            if(attachmentId == attachment->id()) {
                ++tries;
                goto generateRandomId;
            }
        }
    }
    // create new attachment, set ID
    m_attachments.emplace_back(make_unique<MatroskaAttachment>());
    auto &attachment = m_attachments.back();
    attachment->setId(attachmentId);
    return attachment.get();
}

void MatroskaContainer::internalParseHeader()
{
    static const string context("parsing header of Matroska container");
    // reset old results
    m_firstElement = make_unique<EbmlElement>(*this, startOffset());
    m_additionalElements.clear();
    m_tracksElements.clear();
    m_segmentInfoElements.clear();
    m_tagsElements.clear();
    m_seekInfos.clear();
    uint64 currentOffset = 0;
    vector<MatroskaSeekInfo>::size_type seekInfosIndex = 0;
    // loop through all top level elements
    for(EbmlElement *topLevelElement = m_firstElement.get(); topLevelElement; topLevelElement = topLevelElement->nextSibling()) {
        topLevelElement->parse();
        switch(topLevelElement->id()) {
        case EbmlIds::Header:
            for(EbmlElement *subElement = topLevelElement->firstChild(); subElement; subElement = subElement->nextSibling()) {
                try {
                    subElement->parse();
                } catch (Failure &) {
                    addNotification(NotificationType::Critical, "Unable to parse all childs of EBML header element.", context);
                    break;
                }
                switch(subElement->id()) {
                case EbmlIds::Version:
                    m_version = subElement->readUInteger();
                    break;
                case EbmlIds::ReadVersion:
                    m_readVersion = subElement->readUInteger();
                    break;
                case EbmlIds::DocType:
                    m_doctype = subElement->readString();
                    break;
                case EbmlIds::DocTypeVersion:
                    m_doctypeVersion = subElement->readUInteger();
                    break;
                case EbmlIds::DocTypeReadVersion:
                    m_doctypeReadVersion = subElement->readUInteger();
                    break;
                case EbmlIds::MaxIdLength:
                    m_maxIdLength = subElement->readUInteger();
                    if(m_maxIdLength > EbmlElement::maximumIdLengthSupported()) {
                        addNotification(NotificationType::Critical, "Maximum EBML element ID length greather then "
                                        + numberToString<uint32>(EbmlElement::maximumIdLengthSupported())
                                        + " bytes is not supported.", context);
                        throw InvalidDataException();
                    }
                    break;
                case EbmlIds::MaxSizeLength:
                    m_maxSizeLength = subElement->readUInteger();
                    if(m_maxSizeLength > EbmlElement::maximumSizeLengthSupported()) {
                        addNotification(NotificationType::Critical, "Maximum EBML element size length greather then "
                                        + numberToString<uint32>(EbmlElement::maximumSizeLengthSupported())
                                        + " bytes is not supported.", context);
                        throw InvalidDataException();
                    }
                    break;
                }
            }
            break;
        case MatroskaIds::Segment:
            for(EbmlElement *subElement = topLevelElement->firstChild(); subElement; subElement = subElement->nextSibling()) {
                try {
                    subElement->parse();
                } catch (Failure &) {
                    addNotification(NotificationType::Critical, "Unable to parse all childs of EBML segment element.", context);
                    break;
                }
                switch(subElement->id()) {
                case MatroskaIds::SeekHead:
                    m_seekInfos.emplace_back(make_unique<MatroskaSeekInfo>());
                    m_seekInfos.back()->parse(subElement);
                    addNotifications(*m_seekInfos.back());
                    break;
                case MatroskaIds::Tracks:
                    if(excludesOffset(m_tracksElements, subElement->startOffset())) {
                        m_tracksElements.push_back(subElement);
                    }
                    break;
                case MatroskaIds::SegmentInfo:
                    if(excludesOffset(m_segmentInfoElements, subElement->startOffset())) {
                        m_segmentInfoElements.push_back(subElement);
                    }
                    break;
                case MatroskaIds::Tags:
                    if(excludesOffset(m_tagsElements, subElement->startOffset())) {
                        m_tagsElements.push_back(subElement);
                    }
                    break;
                case MatroskaIds::Chapters:
                    if(excludesOffset(m_chaptersElements, subElement->startOffset())) {
                        m_chaptersElements.push_back(subElement);
                    }
                    break;
                case MatroskaIds::Attachments:
                    if(excludesOffset(m_attachmentsElements, subElement->startOffset())) {
                        m_attachmentsElements.push_back(subElement);
                    }
                    break;
                case MatroskaIds::Cluster:
                    // cluster reached
                    // stop here if all relevant information has been gathered
                    for(auto i = m_seekInfos.cbegin() + seekInfosIndex, end = m_seekInfos.cend(); i != end; ++i, ++seekInfosIndex) {
                        for(const auto &infoPair : (*i)->info()) {
                            uint64 offset = currentOffset + topLevelElement->dataOffset() + infoPair.second;
                            if(offset >= fileInfo().size()) {
                                addNotification(NotificationType::Critical, "Offset (" + numberToString(offset) + ") denoted by \"SeekHead\" element is invalid.", context);
                            } else {
                                auto element = make_unique<EbmlElement>(*this, offset);
                                try {
                                    element->parse();
                                    if(element->id() != infoPair.first) {
                                        addNotification(NotificationType::Critical, "ID of element " + element->idToString() + " at " + numberToString(offset) + " does not match the ID denoted in the \"SeekHead\" element (0x" + numberToString(infoPair.first, 16) + ").", context);
                                    }
                                    switch(element->id()) {
                                    case MatroskaIds::SegmentInfo:
                                        if(excludesOffset(m_segmentInfoElements, offset)) {
                                            m_additionalElements.emplace_back(move(element));
                                            m_segmentInfoElements.emplace_back(m_additionalElements.back().get());
                                        }
                                        break;
                                    case MatroskaIds::Tracks:
                                        if(excludesOffset(m_tracksElements, offset)) {
                                            m_additionalElements.emplace_back(move(element));
                                            m_tracksElements.emplace_back(m_additionalElements.back().get());
                                        }
                                        break;
                                   case MatroskaIds::Tags:
                                        if(excludesOffset(m_tagsElements, offset)) {
                                            m_additionalElements.emplace_back(move(element));
                                            m_tagsElements.emplace_back(m_additionalElements.back().get());
                                        }
                                        break;
                                    case MatroskaIds::Chapters:
                                        if(excludesOffset(m_chaptersElements, offset)) {
                                            m_additionalElements.emplace_back(move(element));
                                            m_chaptersElements.emplace_back(m_additionalElements.back().get());
                                        }
                                        break;
                                    case MatroskaIds::Attachments:
                                        if(excludesOffset(m_attachmentsElements, offset)) {
                                            m_additionalElements.emplace_back(move(element));
                                            m_attachmentsElements.emplace_back(m_additionalElements.back().get());
                                        }
                                        break;
                                    default:
                                        ;
                                    }
                                } catch(Failure &) {
                                    addNotification(NotificationType::Critical, "Can not parse element at " + numberToString(offset) + " (denoted using \"SeekHead\" element).", context);
                                }
                            }
                        }
                    }
                    // not checking if m_tagsElements is empty avoids long parsing times when loading big files
                    // but also has the disadvantage that the parser relies on the presence of a SeekHead element
                    // (which is not mandatory) to detect tags at the end of the segment
                    if(((!m_tracksElements.empty() && !m_tagsElements.empty()) || fileInfo().size() > m_maxFullParseSize) && !m_segmentInfoElements.empty()) {
                        goto finish;
                    }
                    break;
                }
            }
            currentOffset += topLevelElement->totalSize();
            break;
        default:
            ;
        }
    }
    // finally parse the "Info"-element and fetch "EditionEntry"-elements
    finish:
    try {
        parseSegmentInfo();
    } catch (Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse EBML (segment) \"Info\"-element.", context);
    }
}

/*!
 * \brief Parses the (segment) "Info"-element.
 *
 * This private method is called when parsing the header.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaContainer::parseSegmentInfo()
{
    if(m_segmentInfoElements.empty()) {
        throw NoDataFoundException();
    }
    m_duration = TimeSpan();
    for(EbmlElement *element : m_segmentInfoElements) {
        element->parse();
        EbmlElement *subElement = element->firstChild();
        float64 rawDuration = 0.0;
        uint64 timeScale = 0;
        bool hasTitle = false;
        while(subElement) {
            subElement->parse();
            switch(subElement->id()) {
            case MatroskaIds::Title:
                m_titles.emplace_back(subElement->readString());
                hasTitle = true;
                break;
            case MatroskaIds::Duration:
                rawDuration = subElement->readFloat();
                break;
            case MatroskaIds::TimeCodeScale:
                timeScale = subElement->readUInteger();
                break;
            }
            subElement = subElement->nextSibling();
        }
        if(!hasTitle) {
            // add empty string as title for segment if no
            // "Title"-element has been specified
            m_titles.emplace_back();
        }
        if(rawDuration > 0.0 && timeScale > 0) {
            m_duration += TimeSpan::fromSeconds(rawDuration * timeScale / 1000000000);
        }
    }
}

void MatroskaContainer::internalParseTags()
{
    static const string context("parsing tags of Matroska container");
    for(EbmlElement *element : m_tagsElements) {
        try {
            element->parse();
            for(EbmlElement *subElement = element->firstChild(); subElement; subElement = subElement->nextSibling()) {
                subElement->parse();
                switch(subElement->id()) {
                case MatroskaIds::Tag:
                    m_tags.emplace_back(make_unique<MatroskaTag>());
                    try {
                    m_tags.back()->parse(*subElement);
                } catch(NoDataFoundException &) {
                        m_tags.pop_back();
                    } catch(Failure &) {
                        addNotification(NotificationType::Critical, "Unable to parse tag " + ConversionUtilities::numberToString(m_tags.size()) + ".", context);
                    }
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    addNotification(NotificationType::Warning, "\"Tags\"-element contains unknown child. It will be ignored.", context);
                }
            }
        } catch(Failure &) {
            addNotification(NotificationType::Critical, "Element structure seems to be invalid.", context);
            throw;
        }
    }
}

void MatroskaContainer::internalParseTracks()
{
    invalidateStatus();
    static const string context("parsing tracks of Matroska container");
    for(EbmlElement *element : m_tracksElements) {
        try {
            element->parse();
            for(EbmlElement *subElement = element->firstChild(); subElement; subElement = subElement->nextSibling()) {
                subElement->parse();
                switch(subElement->id()) {
                case MatroskaIds::TrackEntry:
                    m_tracks.emplace_back(make_unique<MatroskaTrack>(*subElement));
                    try {
                        m_tracks.back()->parseHeader();
                    } catch(NoDataFoundException &) {
                        m_tracks.pop_back();
                    } catch(Failure &) {
                        addNotification(NotificationType::Critical, "Unable to parse track " + ConversionUtilities::numberToString(m_tracks.size()) + ".", context);
                    }
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    addNotification(NotificationType::Warning, "\"Tracks\"-element contains unknown child element \"" + subElement->idToString() + "\". It will be ignored.", context);
                }
            }
        } catch(Failure &) {
            addNotification(NotificationType::Critical, "Element structure seems to be invalid.", context);
            throw;
        }
    }
}

void MatroskaContainer::internalParseChapters()
{
    invalidateStatus();
    static const string context("parsing editions/chapters of Matroska container");
    for(EbmlElement *element : m_chaptersElements) {
        try {
            element->parse();
            for(EbmlElement *subElement = element->firstChild(); subElement; subElement = subElement->nextSibling()) {
                subElement->parse();
                switch(subElement->id()) {
                case MatroskaIds::EditionEntry:
                    m_editionEntries.emplace_back(make_unique<MatroskaEditionEntry>(subElement));
                    try {
                        m_editionEntries.back()->parseNested();
                    } catch(NoDataFoundException &) {
                        m_editionEntries.pop_back();
                    } catch(Failure &) {
                        addNotification(NotificationType::Critical, "Unable to parse edition entry " + ConversionUtilities::numberToString(m_editionEntries.size()) + ".", context);
                    }
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    addNotification(NotificationType::Warning, "\"Chapters\"-element contains unknown child element \"" + subElement->idToString() + "\". It will be ignored.", context);
                }
            }
        } catch(Failure &) {
            addNotification(NotificationType::Critical, "Element structure seems to be invalid.", context);
            throw;
        }
    }
}

void MatroskaContainer::internalParseAttachments()
{
    invalidateStatus();
    static const string context("parsing attachments of Matroska container");
    for(EbmlElement *element : m_attachmentsElements) {
        try {
            element->parse();
            for(EbmlElement *subElement = element->firstChild(); subElement; subElement = subElement->nextSibling()) {
                subElement->parse();
                switch(subElement->id()) {
                case MatroskaIds::AttachedFile:
                    m_attachments.emplace_back(make_unique<MatroskaAttachment>());
                    try {
                        m_attachments.back()->parse(subElement);
                    } catch(NoDataFoundException &) {
                        m_attachments.pop_back();
                    } catch(Failure &) {
                        addNotification(NotificationType::Critical, "Unable to parse attached file " + ConversionUtilities::numberToString(m_attachments.size()) + ".", context);
                    }
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    addNotification(NotificationType::Warning, "\"Attachments\"-element contains unknown child element \"" + subElement->idToString() + "\". It will be ignored.", context);
                }
            }
        } catch(Failure &) {
            addNotification(NotificationType::Critical, "Element structure seems to be invalid.", context);
            throw;
        }
    }
}

void MatroskaContainer::internalMakeFile()
{
    invalidateStatus();
    static const string context("making Matroska container");
    updateStatus("Calculating element sizes ...");
    if(!isHeaderParsed()) {
        addNotification(NotificationType::Critical, "The header has not been parsed yet.", context);
        throw InvalidDataException();
    }
    EbmlElement *level0Element = firstElement(), *level1Element, *level2Element;
    if(!level0Element) {
        addNotification(NotificationType::Critical, "No EBML elements could be found.", context);
        throw InvalidDataException();
    }
    // check whether a rewrite is required
    try {
        // calculate size of tags
        vector<MatroskaTagMaker> tagMaker;
        uint64 tagElementsSize = 0;
        for(auto &tag : tags()) {
            tag->invalidateNotifications();
            try {
                tagMaker.emplace_back(tag->prepareMaking());
                if(tagMaker.back().requiredSize() > 3) {
                    // a tag of 3 bytes size is empty and can be skipped
                    tagElementsSize += tagMaker.back().requiredSize();
                }
            } catch(Failure &) {
                // nothing to do because notifications will be added anyways
            }
            addNotifications(*tag);
        }
        uint64 tagsSize = tagElementsSize ? 4 + EbmlElement::calculateSizeDenotationLength(tagElementsSize) + tagElementsSize : 0;
        // calculate size of attachments
        vector<MatroskaAttachmentMaker> attachmentMaker;
        uint64 attachedFileElementsSize = 0;
        for(auto &attachment : m_attachments) {
            if(!attachment->isIgnored()) {
                attachment->invalidateNotifications();
                try {
                    attachmentMaker.emplace_back(attachment->prepareMaking());
                    if(attachmentMaker.back().requiredSize() > 3) {
                        // an attachment of 3 bytes size is empty and can be skipped
                        attachedFileElementsSize += attachmentMaker.back().requiredSize();
                    }
                } catch(Failure &) {
                    // nothing to do because notifications will be added anyways
                }
                addNotifications(*attachment);
            }
        }
        uint64 attachmentsSize = attachedFileElementsSize ? 4 + EbmlElement::calculateSizeDenotationLength(attachedFileElementsSize) + attachedFileElementsSize : 0;
        // check:
        //  - the number of segments to be written
        //  - current media data / first cluster offset
        //  - current padding
        //  - whether there are tags or attachments in additional segments
        // to determine whether a rewrite is required
        unsigned int lastSegmentIndex = static_cast<unsigned int>(-1);
        bool firstClusterFound = false;
        uint64 currentFirstClusterOffset = 0;
        uint64 currentPadding = 0;
        bool rewriteRequired = false;
        for(; level0Element; level0Element = level0Element->nextSibling()) {
            level0Element->parse();
            switch(level0Element->id()) {
            case EbmlIds::Void:
                if(!firstClusterFound) {
                    currentPadding += level0Element->totalSize();
                }
                break;
            case MatroskaIds::Segment:
                // check the additional segment for tags and attachments
                if(++lastSegmentIndex && !rewriteRequired) {
                    for(level1Element = level0Element->firstChild(); level1Element; level1Element = level1Element->nextSibling()) {
                        level1Element->parse();
                        if(level1Element->id() == MatroskaIds::Attachments || level1Element->id() == MatroskaIds::Tags) {
                            rewriteRequired = true;
                            break;
                        }
                    }
                }
                // check the segment for the first "Cluster"-element (if not found yet)
                if(!firstClusterFound) {
                    for(level1Element = level0Element->firstChild(); level1Element; level1Element = level1Element->nextSibling()) {
                        level1Element->parse();
                        switch(level1Element->id()) {
                        case EbmlIds::Void:
                            currentPadding += level1Element->totalSize();
                            break;
                        case MatroskaIds::Cluster:
                            currentFirstClusterOffset = level1Element->startOffset();
                            firstClusterFound = true;
                            break;
                        }
                    }
                }
                break;
            }
        }
        if(!rewriteRequired) {
            rewriteRequired = currentPadding <= fileInfo().maxPadding() && currentPadding >= fileInfo().minPadding();
        }
        // prepare rewriting the file
        //TODO: do backup stuff only when rewrite is really required
        updateStatus("Preparing for rewriting Matroska/EBML file ...");
        fileInfo().close(); // ensure the file is close before renaming it
        string backupPath;
        fstream &outputStream = fileInfo().stream();
        BinaryWriter outputWriter(&outputStream);
        fstream backupStream; // create a stream to open the backup/original file
        try {
            BackupHelper::createBackupFile(fileInfo().path(), backupPath, backupStream);
            // set backup stream as associated input stream since we need the original elements to write the new file
            setStream(backupStream);
            // recreate original file, define buffer variables
            outputStream.open(fileInfo().path(), ios_base::out | ios_base::binary | ios_base::trunc);
            // define needed variables
            uint64 elementSize; // the size of the current element
            uint64 clusterSize; // the new size the current cluster
            uint64 clusterReadOffset; // the read offset of the current cluster
            uint64 clusterReadSize; // the original size of the current cluster
            vector<uint64> clusterSizes; // the sizes of the cluster elements
            vector<uint64>::const_iterator clusterSizesIterator;
            uint64 readOffset = 0; // the current read offset to calculate positions
            uint64 currentOffset = 0; // the current write offset to calculate positions
            uint64 offset; // offset of the segment which is currently written, offset of "Cues"-element in segment
            bool cuesPresent; // whether the "Cues"-element is present in the current segment
            vector<tuple<uint64, uint64> > crc32Offsets; // holds the offsets of all CRC-32 elements and the length of the enclosing block
            bool elementHasCrc32; // whether the current segment has a CRC-32 element
            byte sizeLength; // size length used to make size denotations
            char buff[8]; // buffer used to make size denotations
            // calculate EBML header size
            uint64 ebmlHeaderSize = 2 * 7; // sub element ID sizes
            for(auto headerValue : initializer_list<uint64>{m_version, m_readVersion, m_maxIdLength, m_maxSizeLength, m_doctypeVersion, m_doctypeReadVersion}) {
                ebmlHeaderSize += sizeLength = EbmlElement::calculateUIntegerLength(headerValue);
                ebmlHeaderSize += EbmlElement::calculateSizeDenotationLength(sizeLength);
            }
            ebmlHeaderSize += m_doctype.size();
            ebmlHeaderSize += EbmlElement::calculateSizeDenotationLength(m_doctype.size());
            // prepare writing segments
            uint64 segmentInfoElementDataSize;
            MatroskaSeekInfo seekInfo;
            MatroskaCuePositionUpdater cuesUpdater;
            unsigned int segmentIndex = 0;
            unsigned int index;
            try {
                for(level0Element = firstElement(); level0Element; level0Element = level0Element->nextSibling()) {
                    switch(level0Element->id()) {
                    case EbmlIds::Header:
                        break; // header is already written; skip header here
                    case EbmlIds::Void:
                    case EbmlIds::Crc32:
                        break;
                    case MatroskaIds::Segment:
                        // write "Segment" element
                        updateStatus("Prepare writing segment ...", 0.0);
                        // prepare writing tags
                        // ensure seek info contains no old entries
                        seekInfo.clear();
                        // parse cues
                        cuesUpdater.invalidateNotifications();
                        if((level1Element = level0Element->childById(MatroskaIds::Cues))) {
                            cuesPresent = true;
                            try {
                                cuesUpdater.parse(level1Element);
                            } catch(Failure &) {
                                addNotifications(cuesUpdater);
                                throw;
                            }
                            addNotifications(cuesUpdater);
                        } else {
                            cuesPresent = false;
                        }
                        // check whether the segment has a CRC-32 element
                        elementHasCrc32 = level0Element->firstChild() && level0Element->firstChild()->id() == EbmlIds::Crc32;
                        // calculate segment size
                        calculateSegmentSize:
                        // CRC-32 element is 6 byte long
                        elementSize = elementHasCrc32 ? 6 : 0;
                        // calculate size of "SeekHead"-element
                        elementSize += seekInfo.actualSize();
                        // pretend writing elements to find out the offsets and the total segment size
                        // pretend writing "SegmentInfo"-element
                        for(level1Element = level0Element->childById(MatroskaIds::SegmentInfo), index = 0; level1Element; level1Element = level1Element->siblingById(MatroskaIds::SegmentInfo), ++index) {
                            // update offset in "SeekHead"-element
                            if(seekInfo.push(index, MatroskaIds::SegmentInfo, currentOffset + elementSize)) {
                                goto calculateSegmentSize;
                            } else {
                                // add size of "SegmentInfo"-element
                                // -> size of "MuxingApp"- and "WritingApp"-element
                                segmentInfoElementDataSize = 2 * appInfoElementTotalSize;
                                // -> add size of "Title"-element
                                if(segmentIndex < m_titles.size()) {
                                    const auto &title = m_titles[segmentIndex];
                                    if(!title.empty()) {
                                        segmentInfoElementDataSize += 2 + EbmlElement::calculateSizeDenotationLength(title.size()) + title.size();
                                    }
                                }
                                // -> add size of other childs
                                for(level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                                    level2Element->parse();
                                    switch(level2Element->id()) {
                                    case EbmlIds::Void: // skipped
                                    case EbmlIds::Crc32: // skipped
                                    case MatroskaIds::Title: // calculated separately
                                    case MatroskaIds::MuxingApp: // calculated separately
                                    case MatroskaIds::WrittingApp: // calculated separately
                                        break;
                                    default:
                                        segmentInfoElementDataSize += level2Element->totalSize();
                                    }
                                }
                                // -> calculate total size
                                elementSize += 4 + EbmlElement::calculateSizeDenotationLength(segmentInfoElementDataSize) + segmentInfoElementDataSize;
                            }
                        }
                        // pretend writing "Tracks"- and "Chapters"-element
                        for(const auto id : initializer_list<EbmlElement::identifierType>{MatroskaIds::Tracks, MatroskaIds::Chapters}) {
                            for(level1Element = level0Element->childById(id), index = 0; level1Element; level1Element = level1Element->siblingById(id), ++index) {
                                // update offset in "SeekHead"-element
                                if(seekInfo.push(index, id, currentOffset + elementSize)) {
                                    goto calculateSegmentSize;
                                } else {
                                    // add size of element
                                    elementSize += level1Element->totalSize();
                                }
                            }
                        }
                        // all "Tags"- and "Attachments"-elements are written in either the first or the last segment
                        // and either before "Cues"- and "Cluster"-elements or after these elements
                        // depending on the desired tag position (at the front/at the end)
                        if(fileInfo().tagPosition() == TagPosition::BeforeData && segmentIndex == 0) {
                            // pretend writing "Tags"-element
                            if(tagsSize) {
                                // update offsets in "SeekHead"-element
                                if(seekInfo.push(0, MatroskaIds::Tags, currentOffset + elementSize)) {
                                    goto calculateSegmentSize;
                                } else {
                                    // add size of "Tags"-element
                                    elementSize += tagsSize;
                                }
                            }
                            // pretend writing "Attachments"-element
                            if(attachmentsSize) {
                                // update offsets in "SeekHead"-element
                                if(seekInfo.push(0, MatroskaIds::Attachments, currentOffset + elementSize)) {
                                    goto calculateSegmentSize;
                                } else {
                                    // add size of "Attachments"-element
                                    elementSize += attachmentsSize;
                                }
                            }
                        }
                        // pretend writing "Cues"-element
                        if(cuesPresent) {
                            offset = elementSize; // save current offset
                            // update offset of "Cues"-element in "SeekHead"-element
                            if(seekInfo.push(0, MatroskaIds::Cues, currentOffset + elementSize)) {
                                goto calculateSegmentSize;
                            } else {
                                // add size of "Cues"-element
                                addCuesElementSize:
                                elementSize += cuesUpdater.totalSize();
                            }
                        }
                        // pretend writing "Cluster"-element
                        clusterSizes.clear();
                        for(level1Element = level0Element->childById(MatroskaIds::Cluster), index = 0; level1Element; level1Element = level1Element->siblingById(MatroskaIds::Cluster), ++index) {
                            // update offset of "Cluster"-element in "Cues"-element
                            //if(cuesPresent && cuesUpdater.updatePositions(currentOffset + level1Element->startOffset() - level0Element->dataOffset(), elementSize)) {
                            clusterReadOffset = level1Element->startOffset() - level0Element->dataOffset() + readOffset;
                            if(cuesPresent && cuesUpdater.updateOffsets(clusterReadOffset, currentOffset + elementSize)) {
                                elementSize = offset; // reset element size to previously saved offset of "Cues"-element
                                goto addCuesElementSize;
                            } else {
                                if(index == 0 && seekInfo.push(index, MatroskaIds::Cluster, currentOffset + elementSize)) {
                                    goto calculateSegmentSize;
                                } else {
                                    // add size of "Cluster"-element
                                    clusterSize = 0;
                                    clusterReadSize = 0;
                                    for(level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                                        level2Element->parse();
                                        if(cuesPresent && cuesUpdater.updateRelativeOffsets(clusterReadOffset, clusterReadSize, clusterSize)) {
                                            elementSize = offset;
                                            goto addCuesElementSize;
                                        }
                                        switch(level2Element->id()) {
                                        case EbmlIds::Void:
                                        case EbmlIds::Crc32:
                                            break;
                                        case MatroskaIds::Position:
                                            clusterSize += 1 + 1 + EbmlElement::calculateUIntegerLength(currentOffset + elementSize);
                                            break;
                                        default:
                                            clusterSize += level2Element->totalSize();
                                        }
                                        clusterReadSize += level2Element->totalSize();
                                    }
                                    clusterSizes.push_back(clusterSize);
                                    elementSize += 4 + EbmlElement::calculateSizeDenotationLength(clusterSize) + clusterSize;
                                }
                            }
                        }
                        if(fileInfo().tagPosition() == TagPosition::AfterData && segmentIndex == lastSegmentIndex) {
                            // pretend writing "Tags"-element
                            if(tagsSize) {
                                // update offsets in "SeekHead"-element
                                if(seekInfo.push(0, MatroskaIds::Tags, currentOffset + elementSize)) {
                                    goto calculateSegmentSize;
                                } else {
                                    // add size of "Tags"-element
                                    elementSize += tagsSize;
                                }
                            }
                            // pretend writing "Attachments"-element
                            if(attachmentsSize) {
                                // update offsets in "SeekHead"-element
                                if(seekInfo.push(0, MatroskaIds::Attachments, currentOffset + elementSize)) {
                                    goto calculateSegmentSize;
                                } else {
                                    // add size of "Attachments"-element
                                    elementSize += attachmentsSize;
                                }
                            }
                        }
                        // start the actual writing
                        if(!segmentIndex) {
                            // nothing written so far (just prepared)
                            // -> decided whether it is necessary to rewrite the entire file
                            // TODO
                            // write EBML header (before writing the first segment)
                            updateStatus("Writing EBML header ...");
                            outputWriter.writeUInt32BE(EbmlIds::Header);
                            sizeLength = EbmlElement::makeSizeDenotation(ebmlHeaderSize, buff);
                            outputStream.write(buff, sizeLength);
                            EbmlElement::makeSimpleElement(outputStream, EbmlIds::Version, m_version);
                            EbmlElement::makeSimpleElement(outputStream, EbmlIds::ReadVersion, m_readVersion);
                            EbmlElement::makeSimpleElement(outputStream, EbmlIds::MaxIdLength, m_maxIdLength);
                            EbmlElement::makeSimpleElement(outputStream, EbmlIds::MaxSizeLength, m_maxSizeLength);
                            EbmlElement::makeSimpleElement(outputStream, EbmlIds::DocType, m_doctype);
                            EbmlElement::makeSimpleElement(outputStream, EbmlIds::DocTypeVersion, m_doctypeVersion);
                            EbmlElement::makeSimpleElement(outputStream, EbmlIds::DocTypeReadVersion, m_doctypeReadVersion);
                        }
                        // write "Segment"-element actually
                        updateStatus("Writing segment header ...");
                        outputWriter.writeUInt32BE(MatroskaIds::Segment);
                        sizeLength = EbmlElement::makeSizeDenotation(elementSize, buff);
                        outputStream.write(buff, sizeLength);
                        offset = outputStream.tellp(); // store segment data offset here
                        // write CRC-32 element ...
                        if(elementHasCrc32) {
                            // ... if the original element had a CRC-32 element
                            *buff = EbmlIds::Crc32;
                            *(buff + 1) = 0x84; // length denotation: 4 byte
                            // set the value after writing the element
                            crc32Offsets.emplace_back(outputStream.tellp(), elementSize);
                            outputStream.write(buff, 6);
                        }
                        // write "SeekHead"-element (except there is no seek information for the current segment)
                        seekInfo.invalidateNotifications();
                        seekInfo.make(outputStream);
                        addNotifications(seekInfo);
                        // write "SegmentInfo"-element
                        for(level1Element = level0Element->childById(MatroskaIds::SegmentInfo); level1Element; level1Element = level1Element->siblingById(MatroskaIds::SegmentInfo)) {
                            // -> write ID and size
                            outputWriter.writeUInt32BE(MatroskaIds::SegmentInfo);
                            sizeLength = EbmlElement::makeSizeDenotation(segmentInfoElementDataSize, buff);
                            outputStream.write(buff, sizeLength);
                            // -> write childs
                            for(level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                                switch(level2Element->id()) {
                                case EbmlIds::Void: // skipped
                                case EbmlIds::Crc32: // skipped
                                case MatroskaIds::Title: // written separately
                                case MatroskaIds::MuxingApp: // written separately
                                case MatroskaIds::WrittingApp: // written separately
                                    break;
                                default:
                                    level2Element->copyEntirely(outputStream);
                                }
                            }
                            // -> write "Title"-element
                            if(segmentIndex < m_titles.size()) {
                                const auto &title = m_titles[segmentIndex];
                                if(!title.empty()) {
                                    EbmlElement::makeSimpleElement(outputStream, MatroskaIds::Title, title);
                                }
                            }
                            // -> write "MuxingApp"- and "WritingApp"-element
                            EbmlElement::makeSimpleElement(outputStream, MatroskaIds::MuxingApp, appInfo, appInfoElementDataSize);
                            EbmlElement::makeSimpleElement(outputStream, MatroskaIds::WrittingApp, appInfo, appInfoElementDataSize);
                        }
                        // write "Tracks"- and "Chapters"-element
                        for(const auto id : initializer_list<EbmlElement::identifierType>{MatroskaIds::Tracks, MatroskaIds::Chapters}) {
                            for(level1Element = level0Element->childById(id); level1Element; level1Element = level1Element->siblingById(id)) {
                                level1Element->copyEntirely(outputStream);
                            }
                        }
                        if(fileInfo().tagPosition() == TagPosition::BeforeData && segmentIndex == 0) {
                            // write "Tags"-element
                            if(tagsSize) {
                                outputWriter.writeUInt32BE(MatroskaIds::Tags);
                                sizeLength = EbmlElement::makeSizeDenotation(tagElementsSize, buff);
                                outputStream.write(buff, sizeLength);
                                for(auto &maker : tagMaker) {
                                    maker.make(outputStream);
                                }
                                // no need to add notifications; this has been done when creating the make
                            }
                            // write "Attachments"-element
                            if(attachmentsSize) {
                                outputWriter.writeUInt32BE(MatroskaIds::Attachments);
                                sizeLength = EbmlElement::makeSizeDenotation(attachedFileElementsSize, buff);
                                outputStream.write(buff, sizeLength);
                                for(auto &maker : attachmentMaker) {
                                    maker.make(outputStream);
                                }
                                // no need to add notifications; this has been done when creating the make
                            }
                        }
                        // write "Cues"-element
                        if(cuesPresent) {
                            try {
                                cuesUpdater.make(outputStream);
                            } catch(Failure &) {
                                addNotifications(cuesUpdater);
                                throw;
                            }
                        }
                        // update status, check whether the operation has been aborted
                        if(isAborted()) {
                            throw OperationAbortedException();
                        } else {
                            addNotifications(cuesUpdater);
                            updateStatus("Writing segment data ...", static_cast<double>(static_cast<uint64>(outputStream.tellp()) - offset) / elementSize);
                        }
                        // write "Cluster"-element
                        for(level1Element = level0Element->childById(MatroskaIds::Cluster), clusterSizesIterator = clusterSizes.cbegin();
                            level1Element; level1Element = level1Element->siblingById(MatroskaIds::Cluster), ++clusterSizesIterator) {
                            // calculate position of cluster in segment
                            clusterSize = currentOffset + (static_cast<uint64>(outputStream.tellp()) - offset);
                            // write header; checking whether clusterSizesIterator is valid shouldn't be necessary
                            outputWriter.writeUInt32BE(MatroskaIds::Cluster);
                            sizeLength = EbmlElement::makeSizeDenotation(*clusterSizesIterator, buff);
                            outputStream.write(buff, sizeLength);
                            // write childs
                            for(level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                                switch(level2Element->id()) {
                                case EbmlIds::Void:
                                case EbmlIds::Crc32:
                                    break;
                                case MatroskaIds::Position:
                                    EbmlElement::makeSimpleElement(outputStream, MatroskaIds::Position, clusterSize);
                                    break;
                                default:
                                    level2Element->copyEntirely(outputStream);
                                }
                            }
                            // update percentage, check whether the operation has been aborted
                            if(isAborted()) {
                                throw OperationAbortedException();
                            } else {
                                updatePercentage(static_cast<double>(static_cast<uint64>(outputStream.tellp()) - offset) / elementSize);
                            }
                        }
                        if(fileInfo().tagPosition() == TagPosition::AfterData && segmentIndex == lastSegmentIndex) {
                            // write "Tags"-element
                            if(tagsSize) {
                                outputWriter.writeUInt32BE(MatroskaIds::Tags);
                                sizeLength = EbmlElement::makeSizeDenotation(tagElementsSize, buff);
                                outputStream.write(buff, sizeLength);
                                for(auto &maker : tagMaker) {
                                    maker.make(outputStream);
                                }
                                // no need to add notifications; this has been done when creating the make
                            }
                            // write "Attachments"-element
                            if(attachmentsSize) {
                                outputWriter.writeUInt32BE(MatroskaIds::Attachments);
                                sizeLength = EbmlElement::makeSizeDenotation(attachedFileElementsSize, buff);
                                outputStream.write(buff, sizeLength);
                                for(auto &maker : attachmentMaker) {
                                    maker.make(outputStream);
                                }
                                // no need to add notifications; this has been done when creating the make
                            }
                        }
                        ++segmentIndex; // increase the current segment index
                        currentOffset += 4 + sizeLength + elementSize; // increase current write offset by the size of the segment which has just been written
                        readOffset = level0Element->totalSize(); // increase the read offset by the size of the segment read from the orignial file
                        break;
                    default:
                        // just copy any unknown top-level elements
                        addNotification(NotificationType::Warning, "The top-level element \"" + level0Element->idToString() + "\" of the original file is unknown and will just be copied.", context);
                        level0Element->copyEntirely(outputStream);
                    }
                }
            } catch(OperationAbortedException &) {
                throw;
            } catch(Failure &) {
                // any failures here are caused because the original faile could not be parsed correctly (except OperationAbortedException which is handled above)
                addNotifications(cuesUpdater);
                addNotification(NotificationType::Critical, "Unable to parse content in top-level element at " + numberToString(level0Element->startOffset()) + " of original file.", context);
                throw;
            }
            // reparse what is written so far
            updateStatus("Reparsing output file ...");
            outputStream.close(); // the outputStream needs to be reopened to be able to read again
            outputStream.open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);
            setStream(outputStream);
            reset();
            try {
                parseHeader();
            } catch(Failure &) {
                addNotification(NotificationType::Critical, "Unable to reparse the header of the new file.", context);
                throw;
            }
            // update CRC-32 checksums
            if(!crc32Offsets.empty()) {
                updateStatus("Updating CRC-32 checksums ...");
                for(const auto &crc32Offset : crc32Offsets) {
                    outputStream.seekg(get<0>(crc32Offset) + 6);
                    outputStream.seekp(get<0>(crc32Offset) + 2);
                    writer().writeUInt32LE(reader().readCrc32(get<1>(crc32Offset) - 6));
                }
            }
            updatePercentage(100.0);
            // flush output stream
            outputStream.flush();
        // handle errors which occured after renaming/creating backup file
        } catch(OperationAbortedException &) {
            setStream(outputStream);
            reset();
            addNotification(NotificationType::Information, "Rewriting the file to apply changed tag information has been aborted.", context);
            BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, outputStream, backupStream);
            throw;
        } catch(Failure &) {
            setStream(outputStream);
            reset();
            addNotification(NotificationType::Critical, "Rewriting the file to apply changed tag information failed.", context);
            BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, outputStream, backupStream);
            throw;
        } catch(ios_base::failure &) {
            setStream(outputStream);
            reset();
            addNotification(NotificationType::Critical, "An IO error occured when rewriting the file to apply changed tag information.", context);
            BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, outputStream, backupStream);
            throw;
        }
    // handle errors which occured before renaming/creating backup file
    } catch(Failure &) {
        addNotification(NotificationType::Critical, "Parsing the original file failed.", context);
        throw;
    } catch(ios_base::failure &) {
        addNotification(NotificationType::Critical, "An IO error occured when parsing the original file.", context);
        throw;
    }
}

}
