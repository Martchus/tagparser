#include "./matroskacontainer.h"
#include "./ebmlid.h"
#include "./matroskacues.h"
#include "./matroskaeditionentry.h"
#include "./matroskaid.h"
#include "./matroskaseekinfo.h"

#include "../backuphelper.h"
#include "../exceptions.h"
#include "../mediafileinfo.h"

#include "resources/config.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/path.h>

#include <chrono>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <random>
#include <unordered_set>

using namespace std;
using namespace std::placeholders;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::MatroskaContainer
 * \brief Implementation of GenericContainer<MediaFileInfo, MatroskaTag, MatroskaTrack, EbmlElement>.
 */

/*!
 * \brief Constructs a new container for the specified \a fileInfo at the specified \a startOffset.
 */
MatroskaContainer::MatroskaContainer(MediaFileInfo &fileInfo, std::uint64_t startOffset)
    : GenericContainer<MediaFileInfo, MatroskaTag, MatroskaTrack, EbmlElement>(fileInfo, startOffset)
    , m_maxIdLength(4)
    , m_maxSizeLength(8)
    , m_segmentCount(0)
{
    m_version = 1;
    m_readVersion = 1;
    m_doctype = "matroska";
    m_doctypeVersion = 1;
    m_doctypeReadVersion = 1;
}

MatroskaContainer::~MatroskaContainer()
{
}

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
    m_segmentCount = 0;
}

/*!
 * \brief Validates the file index (cue entries).
 * \remarks Checks only for cluster positions and missing, unknown or surplus elements.
 */
void MatroskaContainer::validateIndex(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    static const auto context = std::string("validating Matroska file index (cues)");
    auto cuesElementsFound = false;
    if (m_firstElement) {
        auto ids = std::unordered_set<EbmlElement::IdentifierType>();
        auto cueTimeFound = false, cueTrackPositionsFound = false;
        auto clusterElement = std::unique_ptr<EbmlElement>();
        auto pos = std::uint64_t(), prevClusterSize = std::uint64_t(), currentOffset = std::uint64_t();
        // iterate through all segments
        for (EbmlElement *segmentElement = m_firstElement->siblingById(MatroskaIds::Segment, diag); segmentElement;
             segmentElement = segmentElement->siblingById(MatroskaIds::Segment, diag)) {
            segmentElement->parse(diag);
            // iterate through all child elements of the segment (only "Cues"- and "Cluster"-elements are relevant for this method)
            for (EbmlElement *segmentChildElement = segmentElement->firstChild(); segmentChildElement;
                 segmentChildElement = segmentChildElement->nextSibling()) {
                progress.stopIfAborted();
                segmentChildElement->parse(diag);
                switch (segmentChildElement->id()) {
                case EbmlIds::Void:
                case EbmlIds::Crc32:
                    break;
                case MatroskaIds::Cues:
                    cuesElementsFound = true;
                    // parse children of "Cues"-element ("CuePoint"-elements)
                    for (EbmlElement *cuePointElement = segmentChildElement->firstChild(); cuePointElement;
                         cuePointElement = cuePointElement->nextSibling()) {
                        progress.stopIfAborted();
                        cuePointElement->parse(diag);
                        cueTimeFound = cueTrackPositionsFound = false; // to validate quantity of these elements
                        switch (cuePointElement->id()) {
                        case EbmlIds::Void:
                        case EbmlIds::Crc32:
                            break;
                        case MatroskaIds::CuePoint:
                            // parse children of "CuePoint"-element
                            for (EbmlElement *cuePointChildElement = cuePointElement->firstChild(); cuePointChildElement;
                                 cuePointChildElement = cuePointChildElement->nextSibling()) {
                                cuePointChildElement->parse(diag);
                                switch (cuePointChildElement->id()) {
                                case MatroskaIds::CueTime:
                                    // validate uniqueness
                                    if (cueTimeFound) {
                                        diag.emplace_back(
                                            DiagLevel::Warning, "\"CuePoint\"-element contains multiple \"CueTime\" elements.", context);
                                    } else {
                                        cueTimeFound = true;
                                    }
                                    break;
                                case MatroskaIds::CueTrackPositions:
                                    cueTrackPositionsFound = true;
                                    ids.clear();
                                    clusterElement.reset();
                                    for (EbmlElement *subElement = cuePointChildElement->firstChild(); subElement;
                                         subElement = subElement->nextSibling()) {
                                        subElement->parse(diag);
                                        switch (subElement->id()) {
                                        case MatroskaIds::CueTrack:
                                        case MatroskaIds::CueClusterPosition:
                                        case MatroskaIds::CueRelativePosition:
                                        case MatroskaIds::CueDuration:
                                        case MatroskaIds::CueBlockNumber:
                                        case MatroskaIds::CueCodecState:
                                            // validate uniqueness
                                            if (ids.count(subElement->id())) {
                                                diag.emplace_back(DiagLevel::Warning,
                                                    "\"CueTrackPositions\"-element contains multiple \"" % subElement->idToString() + "\" elements.",
                                                    context);
                                            } else {
                                                ids.insert(subElement->id());
                                            }
                                            break;
                                        case EbmlIds::Crc32:
                                        case EbmlIds::Void:
                                        case MatroskaIds::CueReference:
                                            break;
                                        default:
                                            diag.emplace_back(DiagLevel::Warning,
                                                "\"CueTrackPositions\"-element contains unknown element \"" % subElement->idToString() + "\".",
                                                context);
                                        }
                                        switch (subElement->id()) {
                                        case EbmlIds::Void:
                                        case EbmlIds::Crc32:
                                        case MatroskaIds::CueTrack:
                                            break;
                                        case MatroskaIds::CueClusterPosition:
                                            // validate "Cluster" position denoted by "CueClusterPosition"-element
                                            clusterElement = make_unique<EbmlElement>(
                                                *this, segmentElement->dataOffset() + subElement->readUInteger() - currentOffset);
                                            try {
                                                clusterElement->parse(diag);
                                                if (clusterElement->id() != MatroskaIds::Cluster) {
                                                    diag.emplace_back(DiagLevel::Critical,
                                                        "\"CueClusterPosition\" element at " % numberToString(subElement->startOffset())
                                                            + " does not point to \"Cluster\"-element (points to "
                                                            + numberToString(clusterElement->startOffset()) + ").",
                                                        context);
                                                }
                                            } catch (const Failure &) {
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
                                        default:;
                                        }
                                    }
                                    // validate existence of mandatory elements
                                    if (!ids.count(MatroskaIds::CueTrack)) {
                                        diag.emplace_back(DiagLevel::Warning,
                                            "\"CueTrackPositions\"-element does not contain mandatory element \"CueTrack\".", context);
                                    }
                                    if (!clusterElement) {
                                        diag.emplace_back(DiagLevel::Warning,
                                            "\"CueTrackPositions\"-element does not contain mandatory element \"CueClusterPosition\".", context);
                                    } else if (ids.count(MatroskaIds::CueRelativePosition)) {
                                        // validate "Block" position denoted by "CueRelativePosition"-element
                                        EbmlElement referenceElement(*this, clusterElement->dataOffset() + pos);
                                        try {
                                            referenceElement.parse(diag);
                                            switch (referenceElement.id()) {
                                            case MatroskaIds::SimpleBlock:
                                            case MatroskaIds::Block:
                                            case MatroskaIds::BlockGroup:
                                                break;
                                            default:
                                                diag.emplace_back(DiagLevel::Critical,
                                                    "\"CueRelativePosition\" element does not point to \"Block\"-, \"BlockGroup\", or "
                                                    "\"SimpleBlock\"-element (points to "
                                                            % numberToString(referenceElement.startOffset())
                                                        + ").",
                                                    context);
                                            }
                                        } catch (const Failure &) {
                                        }
                                    }
                                    break;
                                case EbmlIds::Crc32:
                                case EbmlIds::Void:
                                    break;
                                default:
                                    diag.emplace_back(DiagLevel::Warning,
                                        "\"CuePoint\"-element contains unknown element \"" % cuePointElement->idToString() + "\".", context);
                                }
                            }
                            // validate existence of mandatory elements
                            if (!cueTimeFound) {
                                diag.emplace_back(
                                    DiagLevel::Warning, "\"CuePoint\"-element does not contain mandatory element \"CueTime\".", context);
                            }
                            if (!cueTrackPositionsFound) {
                                diag.emplace_back(
                                    DiagLevel::Warning, "\"CuePoint\"-element does not contain mandatory element \"CueClusterPosition\".", context);
                            }
                            break;
                        default:;
                        }
                    }
                    break;
                case MatroskaIds::Cluster:
                    // parse children of "Cluster"-element
                    for (EbmlElement *clusterElementChild = segmentChildElement->firstChild(); clusterElementChild;
                         clusterElementChild = clusterElementChild->nextSibling()) {
                        clusterElementChild->parse(diag);
                        switch (clusterElementChild->id()) {
                        case EbmlIds::Void:
                        case EbmlIds::Crc32:
                            break;
                        case MatroskaIds::Position:
                            // validate position
                            if ((pos = clusterElementChild->readUInteger()) > 0
                                && (segmentChildElement->startOffset() - segmentElement->dataOffset() + currentOffset) != pos) {
                                diag.emplace_back(DiagLevel::Critical,
                                    argsToString("\"Position\"-element at ", clusterElementChild->startOffset(), " points to ", pos,
                                        " which is not the offset of the containing \"Cluster\"-element."),
                                    context);
                            }
                            break;
                        case MatroskaIds::PrevSize:
                            // validate prev size
                            if ((pos = clusterElementChild->readUInteger()) != prevClusterSize) {
                                diag.emplace_back(DiagLevel::Critical,
                                    argsToString("\"PrevSize\"-element at ", clusterElementChild->startOffset(), " should be ", prevClusterSize,
                                        " but is ", pos, "."),
                                    context);
                            }
                            break;
                        default:;
                        }
                    }
                    prevClusterSize = segmentChildElement->totalSize();
                    break;
                default:;
                }
            }
            currentOffset += segmentElement->totalSize();
        }
    }
    // add a warning when no index could be found
    if (!cuesElementsFound) {
        diag.emplace_back(DiagLevel::Information, "No \"Cues\"-elements (index) found.", context);
    }
}

/*!
 * \brief Returns an indication whether \a offset equals the start offset of \a element.
 */
bool sameOffset(std::uint64_t offset, const EbmlElement *element)
{
    return element->startOffset() == offset;
}

/*!
 * \brief Returns whether none of the specified \a elements have the specified \a offset.
 * \remarks This method is used when gathering elements to avoid adding the same element twice.
 */
inline bool excludesOffset(const vector<EbmlElement *> &elements, std::uint64_t offset)
{
    return find_if(elements.cbegin(), elements.cend(), std::bind(sameOffset, offset, _1)) == elements.cend();
}

MatroskaChapter *MatroskaContainer::chapter(std::size_t index)
{
    for (const auto &entry : m_editionEntries) {
        const auto &chapters = entry->chapters();
        if (index < chapters.size()) {
            return chapters[index].get();
        } else {
            index -= chapters.size();
        }
    }
    return nullptr;
}

size_t MatroskaContainer::chapterCount() const
{
    size_t count = 0;
    for (const auto &entry : m_editionEntries) {
        count += entry->chapters().size();
    }
    return count;
}

MatroskaAttachment *MatroskaContainer::createAttachment()
{
    // generate unique ID
    static const auto randomEngine(
        default_random_engine(static_cast<default_random_engine::result_type>(chrono::system_clock::now().time_since_epoch().count())));
    std::uint64_t attachmentId;
    auto dice(bind(uniform_int_distribution<decltype(attachmentId)>(), randomEngine));
    std::uint8_t tries = 0;
generateRandomId:
    attachmentId = dice();
    if (tries < 0xFF) {
        for (const auto &attachment : m_attachments) {
            if (attachmentId == attachment->id()) {
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

/*!
 * \brief Determines the position of the element with the specified \a elementId.
 * \sa determineTagPosition() and determineIndexPosition()
 */
ElementPosition MatroskaContainer::determineElementPosition(std::uint64_t elementId, Diagnostics &diag) const
{
    if (!m_firstElement || m_segmentCount != 1) {
        return ElementPosition::Keep;
    }
    const auto *const segmentElement = m_firstElement->siblingByIdIncludingThis(MatroskaIds::Segment, diag);
    if (!segmentElement) {
        return ElementPosition::Keep;
    }
    for (const EbmlElement *childElement = segmentElement->firstChild(); childElement; childElement = childElement->nextSibling()) {
        if (childElement->id() == elementId) {
            return ElementPosition::BeforeData;
        } else if (childElement->id() == MatroskaIds::Cluster) {
            for (const auto &seekInfo : m_seekInfos) {
                for (const auto &info : seekInfo->info()) {
                    if (info.first == elementId) {
                        return ElementPosition::AfterData;
                    }
                }
            }
            return ElementPosition::Keep;
        }
    }
    return ElementPosition::Keep;
}

ElementPosition MatroskaContainer::determineTagPosition(Diagnostics &diag) const
{
    return determineElementPosition(MatroskaIds::Tags, diag);
}

ElementPosition MatroskaContainer::determineIndexPosition(Diagnostics &diag) const
{
    return determineElementPosition(MatroskaIds::Cues, diag);
}

void MatroskaContainer::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing header of Matroska container");
    // reset old results
    m_firstElement = make_unique<EbmlElement>(*this, startOffset());
    m_additionalElements.clear();
    m_tracksElements.clear();
    m_segmentInfoElements.clear();
    m_tagsElements.clear();
    m_seekInfos.clear();
    m_segmentCount = 0;
    std::uint64_t currentOffset = 0;
    vector<MatroskaSeekInfo>::difference_type seekInfosIndex = 0;

    // loop through all top level elements
    for (EbmlElement *topLevelElement = m_firstElement.get(); topLevelElement; topLevelElement = topLevelElement->nextSibling()) {
        try {
            topLevelElement->parse(diag);
            switch (topLevelElement->id()) {
            case EbmlIds::Header:
                for (EbmlElement *subElement = topLevelElement->firstChild(); subElement; subElement = subElement->nextSibling()) {
                    try {
                        subElement->parse(diag);
                        switch (subElement->id()) {
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
                            if (m_maxIdLength > EbmlElement::maximumIdLengthSupported()) {
                                diag.emplace_back(DiagLevel::Critical,
                                    argsToString("Maximum EBML element ID length greater than ", EbmlElement::maximumIdLengthSupported(),
                                        " bytes is not supported."),
                                    context);
                                throw InvalidDataException();
                            }
                            break;
                        case EbmlIds::MaxSizeLength:
                            m_maxSizeLength = subElement->readUInteger();
                            if (m_maxSizeLength > EbmlElement::maximumSizeLengthSupported()) {
                                diag.emplace_back(DiagLevel::Critical,
                                    argsToString("Maximum EBML element size length greater than ", EbmlElement::maximumSizeLengthSupported(),
                                        " bytes is not supported."),
                                    context);
                                throw InvalidDataException();
                            }
                            break;
                        }
                    } catch (const Failure &) {
                        diag.emplace_back(DiagLevel::Critical, "Unable to parse all children of EBML header.", context);
                        break;
                    }
                }
                break;
            case MatroskaIds::Segment:
                ++m_segmentCount;
                for (EbmlElement *subElement = topLevelElement->firstChild(); subElement; subElement = subElement->nextSibling()) {
                    try {
                        subElement->parse(diag);
                        switch (subElement->id()) {
                        case MatroskaIds::SeekHead:
                            m_seekInfos.emplace_back(make_unique<MatroskaSeekInfo>());
                            m_seekInfos.back()->parse(subElement, diag);
                            break;
                        case MatroskaIds::Tracks:
                            if (excludesOffset(m_tracksElements, subElement->startOffset())) {
                                m_tracksElements.push_back(subElement);
                            }
                            break;
                        case MatroskaIds::SegmentInfo:
                            if (excludesOffset(m_segmentInfoElements, subElement->startOffset())) {
                                m_segmentInfoElements.push_back(subElement);
                            }
                            break;
                        case MatroskaIds::Tags:
                            if (excludesOffset(m_tagsElements, subElement->startOffset())) {
                                m_tagsElements.push_back(subElement);
                            }
                            break;
                        case MatroskaIds::Chapters:
                            if (excludesOffset(m_chaptersElements, subElement->startOffset())) {
                                m_chaptersElements.push_back(subElement);
                            }
                            break;
                        case MatroskaIds::Attachments:
                            if (excludesOffset(m_attachmentsElements, subElement->startOffset())) {
                                m_attachmentsElements.push_back(subElement);
                            }
                            break;
                        case MatroskaIds::Cluster:
                            // stop as soon as the first cluster has been reached if all relevant information has been gathered
                            // -> take elements from seek tables within this segment into account
                            for (auto i = m_seekInfos.cbegin() + seekInfosIndex, end = m_seekInfos.cend(); i != end; ++i, ++seekInfosIndex) {
                                for (const auto &infoPair : (*i)->info()) {
                                    std::uint64_t offset = currentOffset + topLevelElement->dataOffset() + infoPair.second;
                                    if (offset >= fileInfo().size()) {
                                        diag.emplace_back(DiagLevel::Critical,
                                            argsToString("Offset (", offset, ") denoted by \"SeekHead\" element is invalid."), context);
                                    } else {
                                        auto element = make_unique<EbmlElement>(*this, offset);
                                        try {
                                            element->parse(diag);
                                            if (element->id() != infoPair.first) {
                                                diag.emplace_back(DiagLevel::Critical,
                                                    argsToString("ID of element ", element->idToString(), " at ", offset,
                                                        " does not match the ID denoted in the \"SeekHead\" element (0x",
                                                        numberToString(infoPair.first, 16u), ")."),
                                                    context);
                                            }
                                            switch (element->id()) {
                                            case MatroskaIds::SegmentInfo:
                                                if (excludesOffset(m_segmentInfoElements, offset)) {
                                                    m_additionalElements.emplace_back(std::move(element));
                                                    m_segmentInfoElements.emplace_back(m_additionalElements.back().get());
                                                }
                                                break;
                                            case MatroskaIds::Tracks:
                                                if (excludesOffset(m_tracksElements, offset)) {
                                                    m_additionalElements.emplace_back(std::move(element));
                                                    m_tracksElements.emplace_back(m_additionalElements.back().get());
                                                }
                                                break;
                                            case MatroskaIds::Tags:
                                                if (excludesOffset(m_tagsElements, offset)) {
                                                    m_additionalElements.emplace_back(std::move(element));
                                                    m_tagsElements.emplace_back(m_additionalElements.back().get());
                                                }
                                                break;
                                            case MatroskaIds::Chapters:
                                                if (excludesOffset(m_chaptersElements, offset)) {
                                                    m_additionalElements.emplace_back(std::move(element));
                                                    m_chaptersElements.emplace_back(m_additionalElements.back().get());
                                                }
                                                break;
                                            case MatroskaIds::Attachments:
                                                if (excludesOffset(m_attachmentsElements, offset)) {
                                                    m_additionalElements.emplace_back(std::move(element));
                                                    m_attachmentsElements.emplace_back(m_additionalElements.back().get());
                                                }
                                                break;
                                            default:;
                                            }
                                        } catch (const Failure &) {
                                            diag.emplace_back(DiagLevel::Critical,
                                                argsToString("Can not parse element at ", offset, " (denoted using \"SeekHead\" element)."), context);
                                        }
                                    }
                                }
                            }
                            // -> stop if tracks and tags have been found or the file exceeds the max. size to fully process
                            if (((!m_tracksElements.empty() && !m_tagsElements.empty()) || fileInfo().size() > fileInfo().maxFullParseSize())
                                && !m_segmentInfoElements.empty()) {
                                goto finish;
                            }
                            break;
                        }
                    } catch (const Failure &) {
                        diag.emplace_back(DiagLevel::Critical, "Unable to parse all children of \"Segment\"-element.", context);
                        break;
                    }
                }
                currentOffset += topLevelElement->totalSize();
                break;
            default:;
            }
        } catch (const Failure &) {
            diag.emplace_back(
                DiagLevel::Critical, argsToString("Unable to parse top-level element at ", topLevelElement->startOffset(), '.'), context);
            break;
        }
    }

    // finally parse the "Info"-element and fetch "EditionEntry"-elements
finish:
    try {
        parseSegmentInfo(diag);
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Critical, "Unable to parse EBML (segment) \"Info\"-element.", context);
    }
}

/*!
 * \brief Parses the (segment) "Info"-element.
 *
 * This private method is called when parsing the header.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaContainer::parseSegmentInfo(Diagnostics &diag)
{
    if (m_segmentInfoElements.empty()) {
        throw NoDataFoundException();
    }
    m_duration = TimeSpan();
    for (EbmlElement *element : m_segmentInfoElements) {
        element->parse(diag);
        EbmlElement *subElement = element->firstChild();
        double rawDuration = 0.0;
        std::uint64_t timeScale = 1000000;
        bool hasTitle = false;
        while (subElement) {
            subElement->parse(diag);
            switch (subElement->id()) {
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
            case MatroskaIds::MuxingApp:
                muxingApplications().emplace_back(subElement->readString());
                break;
            case MatroskaIds::WrittingApp:
                writingApplications().emplace_back(subElement->readString());
                break;
            }
            subElement = subElement->nextSibling();
        }
        // add empty string as title for segment if no
        // "Title"-element has been specified
        if (!hasTitle) {
            m_titles.emplace_back();
        }
        if (rawDuration > 0.0) {
            m_duration += TimeSpan::fromSeconds(rawDuration * static_cast<double>(timeScale) / 1000000000.0);
        }
    }
}

/*!
 * \brief Reads track-specific statistics from tags.
 * \remarks Tags and tracks must have been parsed before calling this method.
 * \sa MatroskaTrack::readStatisticsFromTags()
 */
void MatroskaContainer::readTrackStatisticsFromTags(Diagnostics &diag)
{
    if (tracks().empty() || tags().empty()) {
        return;
    }
    for (const auto &track : tracks()) {
        track->readStatisticsFromTags(tags(), diag);
    }
}

void MatroskaContainer::internalParseTags(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing tags of Matroska container");
    auto flags = MatroskaTagFlags::None;
    if (fileInfo().fileHandlingFlags() & MediaFileHandlingFlags::NormalizeKnownTagFieldIds) {
        flags += MatroskaTagFlags::NormalizeKnownFieldIds;
    }
    for (EbmlElement *const element : m_tagsElements) {
        try {
            element->parse(diag);
            for (EbmlElement *subElement = element->firstChild(); subElement; subElement = subElement->nextSibling()) {
                subElement->parse(diag);
                switch (subElement->id()) {
                case MatroskaIds::Tag:
                    m_tags.emplace_back(make_unique<MatroskaTag>());
                    try {
                        m_tags.back()->parse2(*subElement, flags, diag);
                    } catch (const NoDataFoundException &) {
                        m_tags.pop_back();
                    } catch (const Failure &) {
                        diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse tag ", m_tags.size(), '.'), context);
                    }
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning, "\"Tags\"-element contains unknown child. It will be ignored.", context);
                }
            }
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Element structure seems to be invalid.", context);
            readTrackStatisticsFromTags(diag);
            throw;
        }
    }
    readTrackStatisticsFromTags(diag);
}

void MatroskaContainer::internalParseTracks(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    static const string context("parsing tracks of Matroska container");
    for (EbmlElement *element : m_tracksElements) {
        try {
            element->parse(diag);
            for (EbmlElement *subElement = element->firstChild(); subElement; subElement = subElement->nextSibling()) {
                subElement->parse(diag);
                switch (subElement->id()) {
                case MatroskaIds::TrackEntry:
                    m_tracks.emplace_back(make_unique<MatroskaTrack>(*subElement));
                    try {
                        m_tracks.back()->parseHeader(diag, progress);
                    } catch (const NoDataFoundException &) {
                        m_tracks.pop_back();
                    } catch (const Failure &) {
                        diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse track ", m_tracks.size(), '.'), context);
                    }
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning,
                        "\"Tracks\"-element contains unknown child element \"" % subElement->idToString() + "\". It will be ignored.", context);
                }
            }
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Element structure seems to be invalid.", context);
            readTrackStatisticsFromTags(diag);
            throw;
        }
    }
    readTrackStatisticsFromTags(diag);
}

void MatroskaContainer::internalParseChapters(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    static const string context("parsing editions/chapters of Matroska container");
    for (EbmlElement *element : m_chaptersElements) {
        try {
            element->parse(diag);
            for (EbmlElement *subElement = element->firstChild(); subElement; subElement = subElement->nextSibling()) {
                subElement->parse(diag);
                switch (subElement->id()) {
                case MatroskaIds::EditionEntry:
                    m_editionEntries.emplace_back(make_unique<MatroskaEditionEntry>(subElement));
                    try {
                        m_editionEntries.back()->parseNested(diag, progress);
                    } catch (const NoDataFoundException &) {
                        m_editionEntries.pop_back();
                    } catch (const Failure &) {
                        diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse edition entry ", m_editionEntries.size(), '.'), context);
                    }
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning,
                        "\"Chapters\"-element contains unknown child element \"" % subElement->idToString() + "\". It will be ignored.", context);
                }
            }
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Element structure seems to be invalid.", context);
            throw;
        }
    }
}

void MatroskaContainer::internalParseAttachments(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing attachments of Matroska container");
    for (EbmlElement *element : m_attachmentsElements) {
        try {
            element->parse(diag);
            for (EbmlElement *subElement = element->firstChild(); subElement; subElement = subElement->nextSibling()) {
                subElement->parse(diag);
                switch (subElement->id()) {
                case MatroskaIds::AttachedFile:
                    m_attachments.emplace_back(make_unique<MatroskaAttachment>());
                    try {
                        m_attachments.back()->parse(subElement, diag);
                    } catch (const NoDataFoundException &) {
                        m_attachments.pop_back();
                    } catch (const Failure &) {
                        diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse attached file ", m_attachments.size(), '.'), context);
                    }
                    break;
                case EbmlIds::Crc32:
                case EbmlIds::Void:
                    break;
                default:
                    diag.emplace_back(DiagLevel::Warning,
                        "\"Attachments\"-element contains unknown child element \"" % subElement->idToString() + "\". It will be ignored.", context);
                }
            }
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Element structure seems to be invalid.", context);
            throw;
        }
    }
}

/// \brief The private SegmentData struct is used in MatroskaContainer::internalMakeFile() to store segment specific data.
struct SegmentData {
    /// \brief Constructs a new segment data object.
    SegmentData()
        : hasCrc32(false)
        , cuesElement(nullptr)
        , infoDataSize(0)
        , firstClusterElement(nullptr)
        , clusterEndOffset(0)
        , startOffset(0)
        , newPadding(0)
        , totalDataSize(0)
        , totalSize(0)
        , newDataOffset(0)
        , sizeDenotationLength(0)
    {
    }
    SegmentData(SegmentData &&) = default;

    /// \brief whether CRC-32 checksum is present
    bool hasCrc32;
    /// \brief used to make "SeekHead"-element
    MatroskaSeekInfo seekInfo;
    /// \brief "Cues"-element (original file)
    EbmlElement *cuesElement;
    /// \brief used to make "Cues"-element
    MatroskaCuePositionUpdater cuesUpdater;
    /// \brief size of the "SegmentInfo"-element
    std::uint64_t infoDataSize;
    /// \brief cluster sizes, needed because cluster elements are not necessarily copied as-is so they're size might change
    std::vector<std::uint64_t> clusterSizes;
    /// \brief first "Cluster"-element (original file)
    EbmlElement *firstClusterElement;
    /// \brief end offset of last "Cluster"-element (original file)
    std::uint64_t clusterEndOffset;
    /// \brief start offset (in the new file)
    std::uint64_t startOffset;
    /// \brief padding (in the new file)
    std::uint64_t newPadding;
    /// \brief total size of the segment data (in the new file, excluding header)
    std::uint64_t totalDataSize;
    /// \brief total size of the segment data (in the new file, including header)
    std::uint64_t totalSize;
    /// \brief data offset of the segment in the new file
    std::uint64_t newDataOffset;
    /// \brief header size (in the new file)
    std::uint8_t sizeDenotationLength;
};

void MatroskaContainer::internalMakeFile(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    static const string context("making Matroska container");
    progress.updateStep("Calculating element sizes ...");

    // basic validation of original file
    if (!isHeaderParsed()) {
        diag.emplace_back(DiagLevel::Critical, "The header has not been parsed yet.", context);
        throw InvalidDataException();
    }
    switch (fileInfo().attachmentsParsingStatus()) {
    case ParsingStatus::Ok:
    case ParsingStatus::NotSupported:
        break;
    default:
        diag.emplace_back(DiagLevel::Critical, "Attachments have to be parsed without critical errors before changes can be applied.", context);
        throw InvalidDataException();
    }

    // define variables for parsing the elements of the original file
    EbmlElement *level0Element = firstElement();
    if (!level0Element) {
        diag.emplace_back(DiagLevel::Critical, "No EBML elements could be found.", context);
        throw InvalidDataException();
    }
    EbmlElement *level1Element, *level2Element;

    // define variables needed for precalculation of "Tags"- and "Attachments"-element
    std::vector<MatroskaTagMaker> tagMaker;
    tagMaker.reserve(tags().size());
    std::uint64_t tagElementsSize = 0;
    std::uint64_t tagsSize;
    std::vector<MatroskaAttachmentMaker> attachmentMaker;
    attachmentMaker.reserve(m_attachments.size());
    std::uint64_t attachedFileElementsSize = 0;
    std::uint64_t attachmentsSize;
    std::vector<MatroskaTrackHeaderMaker> trackHeaderMaker;
    trackHeaderMaker.reserve(tracks().size());
    std::uint64_t trackHeaderElementsSize = 0;
    std::uint64_t trackHeaderSize;

    // define variables to store sizes, offsets and other information required to make a header and "Segment"-elements
    // current segment index
    unsigned int segmentIndex = 0;
    // segment specific data
    std::vector<SegmentData> segmentData;
    // offset of the segment which is currently written / offset of "Cues"-element in segment
    std::uint64_t offset;
    // current total offset (including EBML header)
    std::uint64_t totalOffset;
    // current write offset (used to calculate positions)
    std::uint64_t currentPosition = 0;
    // holds the offsets of all CRC-32 elements and the length of the enclosing block
    std::vector<std::tuple<std::uint64_t, std::uint64_t>> crc32Offsets;
    // size length used to make size denotations
    std::uint8_t sizeLength;
    // sizes and offsets for cluster calculation
    std::uint64_t clusterSize, clusterReadSize, clusterReadOffset;

    // define variables needed to manage file layout
    // -> use the preferred tag position by default (might be changed later if not forced)
    ElementPosition newTagPos = fileInfo().tagPosition();
    // -> current tag position (determined later)
    ElementPosition currentTagPos = ElementPosition::Keep;
    // -> use the preferred cue position by default (might be changed later if not forced)
    ElementPosition newCuesPos = fileInfo().indexPosition();
    // --> current cue position (determined later)
    ElementPosition currentCuesPos = ElementPosition::Keep;
    // -> index of the last segment
    unsigned int lastSegmentIndex = numeric_limits<unsigned int>::max();
    // -> holds new padding
    std::uint64_t newPadding;
    // -> whether rewrite is required (always required when forced to rewrite)
    bool rewriteRequired = fileInfo().isForcingRewrite() || !fileInfo().saveFilePath().empty();

    // calculate EBML header size
    // -> sub element ID sizes
    std::uint64_t ebmlHeaderDataSize = 2 * 7;
    // -> content and size denotation length of numeric sub elements
    for (auto headerValue :
        initializer_list<std::uint64_t>{ m_version, m_readVersion, m_maxIdLength, m_maxSizeLength, m_doctypeVersion, m_doctypeReadVersion }) {
        ebmlHeaderDataSize += sizeLength = EbmlElement::calculateUIntegerLength(headerValue);
        ebmlHeaderDataSize += EbmlElement::calculateSizeDenotationLength(sizeLength);
    }
    // -> content and size denotation length of string sub elements
    ebmlHeaderDataSize += m_doctype.size();
    ebmlHeaderDataSize += EbmlElement::calculateSizeDenotationLength(m_doctype.size());
    const std::uint64_t ebmlHeaderSize = 4 + EbmlElement::calculateSizeDenotationLength(ebmlHeaderDataSize) + ebmlHeaderDataSize;

    // calculate size of "WritingLib"-element
    const auto &muxingApps = const_cast<const MatroskaContainer *>(this)->muxingApplications();
    const auto muxingAppName = (fileInfo().fileHandlingFlags() & MediaFileHandlingFlags::PreserveMuxingApplication && !muxingApps.empty())
        ? std::string_view(muxingApps.front())
        : std::string_view(APP_NAME " v" APP_VERSION);
    const auto muxingAppElementTotalSize = std::uint64_t(2 + 1 + muxingAppName.size());

    // calculate size of "WritingApp"-element
    const auto writingApps = const_cast<const MatroskaContainer *>(this)->writingApplications();
    const auto writingAppName = (fileInfo().fileHandlingFlags() & MediaFileHandlingFlags::PreserveWritingApplication && !writingApps.empty())
        ? std::string_view(writingApps.front())
        : std::string_view(fileInfo().writingApplication().empty() ? muxingAppName : std::string_view(fileInfo().writingApplication()));
    const auto writingAppElementTotalSize = std::uint64_t(2 + 1 + writingAppName.size());

    try {
        // calculate size of "Tags"-element
        for (auto &tag : tags()) {
            try {
                const auto &maker = tagMaker.emplace_back(tag->prepareMaking(diag));
                if (maker.requiredSize() > 3) {
                    // a tag of 3 bytes size is empty and can be skipped
                    tagElementsSize += maker.requiredSize();
                }
            } catch (const Failure &) {
            }
        }
        tagsSize = tagElementsSize ? 4 + EbmlElement::calculateSizeDenotationLength(tagElementsSize) + tagElementsSize : 0;

        // calculate size of "Attachments"-element
        for (auto &attachment : m_attachments) {
            if (!attachment->isIgnored()) {
                try {
                    const auto &maker = attachmentMaker.emplace_back(attachment->prepareMaking(diag));
                    if (maker.requiredSize() > 3) {
                        // an attachment of 3 bytes size is empty and can be skipped
                        attachedFileElementsSize += maker.requiredSize();
                    }
                } catch (const Failure &) {
                }
            }
        }
        attachmentsSize
            = attachedFileElementsSize ? 4 + EbmlElement::calculateSizeDenotationLength(attachedFileElementsSize) + attachedFileElementsSize : 0;

        // calculate size of "Tracks"-element
        for (auto &track : tracks()) {
            try {
                const auto &maker = trackHeaderMaker.emplace_back(track->prepareMakingHeader(diag));
                if (maker.requiredSize() > 3) {
                    // a track header of 3 bytes size is empty and can be skipped
                    trackHeaderElementsSize += maker.requiredSize();
                }
            } catch (const Failure &) {
            }
        }
        trackHeaderSize
            = trackHeaderElementsSize ? 4 + EbmlElement::calculateSizeDenotationLength(trackHeaderElementsSize) + trackHeaderElementsSize : 0;

        // inspect layout of original file
        //  - number of segments
        //  - position of tags relative to the media data
        try {
            for (bool firstClusterFound = false, firstTagFound = false; level0Element; level0Element = level0Element->nextSibling()) {
                level0Element->parse(diag);
                switch (level0Element->id()) {
                case MatroskaIds::Segment:
                    ++lastSegmentIndex;
                    for (level1Element = level0Element->firstChild(); level1Element && !firstClusterFound && !firstTagFound;
                         level1Element = level1Element->nextSibling()) {
                        level1Element->parse(diag);
                        switch (level1Element->id()) {
                        case MatroskaIds::Tags:
                        case MatroskaIds::Attachments:
                            firstTagFound = true;
                            break;
                        case MatroskaIds::Cluster:
                            firstClusterFound = true;
                        }
                    }
                    if (firstTagFound) {
                        currentTagPos = ElementPosition::BeforeData;
                    } else if (firstClusterFound) {
                        currentTagPos = ElementPosition::AfterData;
                    }
                }
            }

            // now the number of segments is known -> allocate segment specific data
            segmentData.resize(lastSegmentIndex + 1);

            // now the current tag/cue position might be known
            if (newTagPos == ElementPosition::Keep) {
                if ((newTagPos = currentTagPos) == ElementPosition::Keep) {
                    newTagPos = ElementPosition::BeforeData;
                }
            }

        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical,
                "Unable to parse content in top-level element at " % numberToString(level0Element->startOffset()) + " of original file.", context);
            throw;
        }

        progress.nextStepOrStop("Calculating offsets of elements before cluster ...");
    calculateSegmentData:
        // define variables to store sizes, offsets and other information required to make a header and "Segment"-elements
        // -> current "pretent" write offset
        std::uint64_t currentOffset = ebmlHeaderSize;
        // -> current read offset (used to calculate positions)
        std::uint64_t readOffset = 0;
        // -> index of current element during iteration
        unsigned int index;

        // if rewriting is required always use the preferred tag/cue position
        if (rewriteRequired) {
            newTagPos = fileInfo().tagPosition();
            if (newTagPos == ElementPosition::Keep) {
                if ((newTagPos = currentTagPos) == ElementPosition::Keep) {
                    newTagPos = ElementPosition::BeforeData;
                }
            }
            newCuesPos = fileInfo().indexPosition();
        }

        // calculate sizes and other information required to make segments
        for (level0Element = firstElement(), currentPosition = newPadding = segmentIndex = 0; level0Element;
             level0Element = level0Element->nextSibling()) {
            switch (level0Element->id()) {
            case EbmlIds::Header:
                // header size has already been calculated
                break;

            case EbmlIds::Void:
            case EbmlIds::Crc32:
                // level 0 "Void"- and "Checksum"-elements are omitted
                break;

            case MatroskaIds::Segment: {
                // get reference to the current segment data instance
                SegmentData &segment = segmentData[segmentIndex];

                // parse original "Cues"-element (if present)
                if (!segment.cuesElement && (segment.cuesElement = level0Element->childById(MatroskaIds::Cues, diag))) {
                    segment.cuesUpdater.parse(segment.cuesElement, diag);
                }

                // get first "Cluster"-element
                if (!segment.firstClusterElement) {
                    segment.firstClusterElement = level0Element->childById(MatroskaIds::Cluster, diag);
                }

                // determine current/new cue position
                if (segment.cuesElement && segment.firstClusterElement) {
                    currentCuesPos = segment.cuesElement->startOffset() < segment.firstClusterElement->startOffset() ? ElementPosition::BeforeData
                                                                                                                     : ElementPosition::AfterData;
                    if (newCuesPos == ElementPosition::Keep) {
                        newCuesPos = currentCuesPos;
                    }
                } else if (newCuesPos == ElementPosition::Keep) {
                    newCuesPos = ElementPosition::BeforeData;
                }

                // set start offset of the segment in the new file
                segment.startOffset = currentOffset;

                // check whether the segment has a CRC-32 element
                segment.hasCrc32 = level0Element->firstChild() && level0Element->firstChild()->id() == EbmlIds::Crc32;

                // precalculate the size of the segment
            calculateSegmentSize:

                // pretent writing "CRC-32"-element (which either present and 6 byte long or omitted)
                segment.totalDataSize = segment.hasCrc32 ? 6 : 0;

                // pretend writing "SeekHead"-element
                segment.totalDataSize += segment.seekInfo.actualSize();

                // pretend writing "SegmentInfo"-element
                for (level1Element = level0Element->childById(MatroskaIds::SegmentInfo, diag), index = 0; level1Element;
                     level1Element = level1Element->siblingById(MatroskaIds::SegmentInfo, diag), ++index) {
                    // update offset in "SeekHead"-element
                    if (segment.seekInfo.push(index, MatroskaIds::SegmentInfo, currentPosition + segment.totalDataSize)) {
                        goto calculateSegmentSize;
                    } else {
                        // add size of "SegmentInfo"-element
                        // -> size of "MuxingApp"- and "WritingApp"-element
                        segment.infoDataSize = muxingAppElementTotalSize + writingAppElementTotalSize;
                        // -> add size of "Title"-element
                        if (segmentIndex < m_titles.size()) {
                            const auto &title = m_titles[segmentIndex];
                            if (!title.empty()) {
                                segment.infoDataSize += 2 + EbmlElement::calculateSizeDenotationLength(title.size()) + title.size();
                            }
                        }
                        // -> add size of other children
                        for (level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                            level2Element->parse(diag);
                            switch (level2Element->id()) {
                            case EbmlIds::Void: // skipped
                            case EbmlIds::Crc32: // skipped
                            case MatroskaIds::Title: // calculated separately
                            case MatroskaIds::MuxingApp: // calculated separately
                            case MatroskaIds::WrittingApp: // calculated separately
                                break;
                            default:
                                level2Element->makeBuffer();
                                segment.infoDataSize += level2Element->totalSize();
                            }
                        }
                        // -> calculate total size
                        segment.totalDataSize += 4 + EbmlElement::calculateSizeDenotationLength(segment.infoDataSize) + segment.infoDataSize;
                    }
                }

                // pretend writing "Tracks"-element
                if (trackHeaderSize) {
                    // update offsets in "SeekHead"-element
                    if (segment.seekInfo.push(0, MatroskaIds::Tracks, currentPosition + segment.totalDataSize)) {
                        goto calculateSegmentSize;
                    } else {
                        // add size of "Tracks"-element
                        segment.totalDataSize += trackHeaderSize;
                    }
                }

                // pretend writing "Chapters"-element
                for (level1Element = level0Element->childById(MatroskaIds::Chapters, diag), index = 0; level1Element;
                     level1Element = level1Element->siblingById(MatroskaIds::Chapters, diag), ++index) {
                    // update offset in "SeekHead"-element
                    if (segment.seekInfo.push(index, MatroskaIds::Chapters, currentPosition + segment.totalDataSize)) {
                        goto calculateSegmentSize;
                    } else {
                        // add size of element
                        level1Element->makeBuffer();
                        segment.totalDataSize += level1Element->totalSize();
                    }
                }

                // "Tags"- and "Attachments"-element are written in either the first or the last segment
                // and either before "Cues"- and "Cluster"-elements or after these elements
                // depending on the desired tag position (at the front/at the end)
                if (newTagPos == ElementPosition::BeforeData && segmentIndex == 0) {
                    // pretend writing "Tags"-element
                    if (tagsSize) {
                        // update offsets in "SeekHead"-element
                        if (segment.seekInfo.push(0, MatroskaIds::Tags, currentPosition + segment.totalDataSize)) {
                            goto calculateSegmentSize;
                        } else {
                            // add size of "Tags"-element
                            segment.totalDataSize += tagsSize;
                        }
                    }
                    // pretend writing "Attachments"-element
                    if (attachmentsSize) {
                        // update offsets in "SeekHead"-element
                        if (segment.seekInfo.push(0, MatroskaIds::Attachments, currentPosition + segment.totalDataSize)) {
                            goto calculateSegmentSize;
                        } else {
                            // add size of "Attachments"-element
                            segment.totalDataSize += attachmentsSize;
                        }
                    }
                }

                offset = segment.totalDataSize; // save current offset (offset before "Cues"-element)

                // pretend writing "Cues"-element
                if (newCuesPos == ElementPosition::BeforeData && segment.cuesElement) {
                    // update offset of "Cues"-element in "SeekHead"-element
                    if (segment.seekInfo.push(0, MatroskaIds::Cues, currentPosition + segment.totalDataSize)) {
                        goto calculateSegmentSize;
                    } else {
                        // add size of "Cues"-element
                        progress.updateStep("Calculating cluster offsets and index size ...");
                    addCuesElementSize:
                        segment.totalDataSize += segment.cuesUpdater.totalSize();
                    }
                } else {
                    progress.updateStep("Calculating cluster offsets ...");
                }

                // decide whether it is necessary to rewrite the entire file (if not already rewriting)
                if (!rewriteRequired) {
                    // find first "Cluster"-element
                    if ((level1Element = segment.firstClusterElement)) {
                        // just before the first "Cluster"-element
                        // -> calculate total offset (excluding size denotation and incomplete index)
                        totalOffset = currentOffset + 4 + segment.totalDataSize;

                        if (totalOffset <= segment.firstClusterElement->startOffset()) {
                            // the padding might be big enough, but
                            // - the segment might become bigger (subsequent tags and attachments)
                            // - the header size hasn't been taken into account yet
                            // - seek information for first cluster and subsequent tags and attachments hasn't been taken into account

                            // assume the size denotation length doesn't change -> use length from original file
                            if (level0Element->headerSize() <= 4 || level0Element->headerSize() > 12) {
                                // validate original header size
                                diag.emplace_back(DiagLevel::Critical, "Header size of \"Segment\"-element from original file is invalid.", context);
                                throw InvalidDataException();
                            }
                            segment.sizeDenotationLength = static_cast<std::uint8_t>(level0Element->headerSize() - 4u);

                        nonRewriteCalculations:
                            // pretend writing "Cluster"-elements assuming there is no rewrite required
                            // -> update offset in "SeakHead"-element
                            if (segment.seekInfo.push(
                                    0, MatroskaIds::Cluster, level1Element->startOffset() - 4 - segment.sizeDenotationLength - ebmlHeaderSize)) {
                                goto calculateSegmentSize;
                            }
                            // -> update offset of "Cluster"-element in "Cues"-element and get end offset of last "Cluster"-element
                            bool cuesInvalidated = false;
                            for (index = 0; level1Element; level1Element = level1Element->siblingById(MatroskaIds::Cluster, diag), ++index) {
                                clusterReadOffset = level1Element->startOffset() - level0Element->dataOffset() + readOffset;
                                segment.clusterEndOffset = level1Element->endOffset();
                                if (segment.cuesElement
                                    && segment.cuesUpdater.updateOffsets(
                                        clusterReadOffset, level1Element->startOffset() - 4 - segment.sizeDenotationLength - ebmlHeaderSize)
                                    && newCuesPos == ElementPosition::BeforeData) {
                                    cuesInvalidated = true;
                                }
                                // check whether aborted (because this loop might take some seconds to process)
                                progress.stopIfAborted();
                                // update the progress percentage (using offset / file size should be accurate enough)
                                if (index % 50 == 0) {
                                    progress.updateStepPercentage(static_cast<std::uint8_t>(level1Element->dataOffset() * 100 / fileInfo().size()));
                                }
                            }
                            if (cuesInvalidated) {
                                segment.totalDataSize = offset;
                                goto addCuesElementSize;
                            }
                            segment.totalDataSize = segment.clusterEndOffset - currentOffset - 4 - segment.sizeDenotationLength;

                            // pretend writing "Cues"-element
                            progress.updateStep("Calculating offsets of elements after cluster ...");
                            if (newCuesPos == ElementPosition::AfterData && segment.cuesElement) {
                                // update offset of "Cues"-element in "SeekHead"-element
                                if (segment.seekInfo.push(0, MatroskaIds::Cues, currentPosition + segment.totalDataSize)) {
                                    goto calculateSegmentSize;
                                } else {
                                    // add size of "Cues"-element
                                    segment.totalDataSize += segment.cuesUpdater.totalSize();
                                }
                            }

                            if (newTagPos == ElementPosition::AfterData && segmentIndex == lastSegmentIndex) {
                                // pretend writing "Tags"-element
                                if (tagsSize) {
                                    // update offsets in "SeekHead"-element
                                    if (segment.seekInfo.push(0, MatroskaIds::Tags, currentPosition + segment.totalDataSize)) {
                                        goto calculateSegmentSize;
                                    } else {
                                        // add size of "Tags"-element
                                        segment.totalDataSize += tagsSize;
                                    }
                                }
                                // pretend writing "Attachments"-element
                                if (attachmentsSize) {
                                    // update offsets in "SeekHead"-element
                                    if (segment.seekInfo.push(0, MatroskaIds::Attachments, currentPosition + segment.totalDataSize)) {
                                        goto calculateSegmentSize;
                                    } else {
                                        // add size of "Attachments"-element
                                        segment.totalDataSize += attachmentsSize;
                                    }
                                }
                            }

                            // calculate total offset again (taking everything into account)
                            // -> check whether assumed size denotation was correct
                            if (segment.sizeDenotationLength != (sizeLength = EbmlElement::calculateSizeDenotationLength(segment.totalDataSize))) {
                                // assumption was wrong -> recalculate with new length
                                segment.sizeDenotationLength = sizeLength;
                                level1Element = segment.firstClusterElement;
                                goto nonRewriteCalculations;
                            }

                            totalOffset = currentOffset + 4 + sizeLength + offset;
                            // offset does not include size of "Cues"-element
                            if (newCuesPos == ElementPosition::BeforeData) {
                                totalOffset += segment.cuesUpdater.totalSize();
                            }
                            if (totalOffset <= segment.firstClusterElement->startOffset()) {
                                // calculate new padding
                                if (segment.newPadding != 1) {
                                    // "Void"-element is at least 2 byte long -> can't add 1 byte padding
                                    newPadding += (segment.newPadding = segment.firstClusterElement->startOffset() - totalOffset);
                                } else {
                                    rewriteRequired = true;
                                }
                            } else {
                                rewriteRequired = true;
                            }
                        } else {
                            rewriteRequired = true;
                        }
                    } else {
                        diag.emplace_back(DiagLevel::Warning, argsToString("There are no clusters in segment ", segmentIndex, "."), context);
                    }

                    if (rewriteRequired) {
                        if (newTagPos != ElementPosition::AfterData
                            && (!fileInfo().forceTagPosition()
                                || (fileInfo().tagPosition() == ElementPosition::Keep && currentTagPos == ElementPosition::Keep))) {
                            // rewriting might be avoided by writing the tags at the end
                            newTagPos = ElementPosition::AfterData;
                            rewriteRequired = false;
                        } else if (newCuesPos != ElementPosition::AfterData
                            && (!fileInfo().forceIndexPosition()
                                || (fileInfo().indexPosition() == ElementPosition::Keep && currentCuesPos == ElementPosition::Keep))) {
                            // rewriting might be avoided by writing the cues at the end
                            newCuesPos = ElementPosition::AfterData;
                            rewriteRequired = false;
                        }
                        // do calculations again for rewriting / changed element order
                        goto calculateSegmentData;
                    }
                } else {
                    // if rewrite is required, pretend writing the remaining elements to compute total segment size and cluster sizes

                    // pretend writing "Void"-element (only if there is at least one "Cluster"-element in the segment)
                    if (!segmentIndex && rewriteRequired && (level1Element = level0Element->childById(MatroskaIds::Cluster, diag))) {
                        // simply use the preferred padding
                        segment.totalDataSize += (segment.newPadding = newPadding = fileInfo().preferredPadding());
                    }

                    // pretend writing "Cluster"-element
                    segment.clusterSizes.clear();
                    bool cuesInvalidated = false;
                    for (index = 0; level1Element; level1Element = level1Element->siblingById(MatroskaIds::Cluster, diag), ++index) {
                        // update offset of "Cluster"-element in "Cues"-element
                        clusterReadOffset = level1Element->startOffset() - level0Element->dataOffset() + readOffset;
                        if (segment.cuesElement && segment.cuesUpdater.updateOffsets(clusterReadOffset, currentPosition + segment.totalDataSize)
                            && newCuesPos == ElementPosition::BeforeData) {
                            cuesInvalidated = true;
                        } else {
                            if (index == 0 && segment.seekInfo.push(index, MatroskaIds::Cluster, currentPosition + segment.totalDataSize)) {
                                goto calculateSegmentSize;
                            } else {
                                // add size of "Cluster"-element
                                clusterSize = clusterReadSize = 0;
                                for (level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                                    level2Element->parse(diag);
                                    if (segment.cuesElement
                                        && segment.cuesUpdater.updateRelativeOffsets(clusterReadOffset, clusterReadSize, clusterSize)
                                        && newCuesPos == ElementPosition::BeforeData) {
                                        cuesInvalidated = true;
                                    }
                                    switch (level2Element->id()) {
                                    case EbmlIds::Void:
                                    case EbmlIds::Crc32:
                                        break;
                                    case MatroskaIds::Position:
                                        clusterSize += 1u + 1u + EbmlElement::calculateUIntegerLength(currentPosition + segment.totalDataSize);
                                        break;
                                    default:
                                        clusterSize += level2Element->totalSize();
                                    }
                                    clusterReadSize += level2Element->totalSize();
                                }
                                segment.clusterSizes.push_back(clusterSize);
                                segment.totalDataSize += 4u + EbmlElement::calculateSizeDenotationLength(clusterSize) + clusterSize;
                            }
                        }
                        // check whether aborted (because this loop might take some seconds to process)
                        progress.stopIfAborted();
                        // update the progress percentage (using offset / file size should be accurate enough)
                        if ((index % 50 == 0) && fileInfo().size()) {
                            progress.updateStepPercentage(static_cast<std::uint8_t>(level1Element->dataOffset() * 100 / fileInfo().size()));
                        }
                        // TODO: reduce code duplication for aborting and progress updates
                    }
                    // check whether the total size of the "Cues"-element has been invalidated and recompute cluster if required
                    if (cuesInvalidated) {
                        // reset element size to previously saved offset of "Cues"-element
                        segment.totalDataSize = offset;
                        goto addCuesElementSize;
                    }

                    // pretend writing "Cues"-element
                    progress.updateStep("Calculating offsets of elements after cluster ...");
                    if (newCuesPos == ElementPosition::AfterData && segment.cuesElement) {
                        // update offset of "Cues"-element in "SeekHead"-element
                        if (segment.seekInfo.push(0, MatroskaIds::Cues, currentPosition + segment.totalDataSize)) {
                            goto calculateSegmentSize;
                        } else {
                            // add size of "Cues"-element
                            segment.totalDataSize += segment.cuesUpdater.totalSize();
                        }
                    }

                    // "Tags"- and "Attachments"-element are written in either the first or the last segment
                    // and either before "Cues"- and "Cluster"-elements or after these elements
                    // depending on the desired tag position (at the front/at the end)
                    if (newTagPos == ElementPosition::AfterData && segmentIndex == lastSegmentIndex) {
                        // pretend writing "Tags"-element
                        if (tagsSize) {
                            // update offsets in "SeekHead"-element
                            if (segment.seekInfo.push(0, MatroskaIds::Tags, currentPosition + segment.totalDataSize)) {
                                goto calculateSegmentSize;
                            } else {
                                // add size of "Tags"-element
                                segment.totalDataSize += tagsSize;
                            }
                        }
                        // pretend writing "Attachments"-element
                        if (attachmentsSize) {
                            // update offsets in "SeekHead"-element
                            if (segment.seekInfo.push(0, MatroskaIds::Attachments, currentPosition + segment.totalDataSize)) {
                                goto calculateSegmentSize;
                            } else {
                                // add size of "Attachments"-element
                                segment.totalDataSize += attachmentsSize;
                            }
                        }
                    }
                }

                // increase the current segment index
                ++segmentIndex;

                // increase write offsets by the size of the segment which size has just been computed
                segment.totalSize = 4 + EbmlElement::calculateSizeDenotationLength(segment.totalDataSize) + segment.totalDataSize;
                currentPosition += segment.totalSize;
                currentOffset += segment.totalSize;

                // increase the read offset by the size of the segment read from the original file
                readOffset += level0Element->totalSize();

                break;
            }
            default:
                // just copy any unknown top-level elements
                diag.emplace_back(DiagLevel::Warning,
                    "The top-level element \"" % level0Element->idToString() + "\" of the original file is unknown and will just be copied.",
                    context);
                currentOffset += level0Element->totalSize();
                readOffset += level0Element->totalSize();
            }
        }

        if (!rewriteRequired) {
            // check whether the new padding is ok according to specifications
            if ((rewriteRequired = (newPadding > fileInfo().maxPadding() || newPadding < fileInfo().minPadding()))) {
                // need to recalculate segment data for rewrite
                goto calculateSegmentData;
            }
        }

    } catch (const OperationAbortedException &) {
        diag.emplace_back(DiagLevel::Information, "Applying new tag information has been aborted.", context);
        throw;
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Critical, "Parsing the original file failed.", context);
        throw;
    } catch (const std::ios_base::failure &failure) {
        diag.emplace_back(DiagLevel::Critical, argsToString("An IO error occurred when parsing the original file: ", failure.what()), context);
        throw;
    }

    // setup stream(s) for writing
    // -> update status
    progress.nextStepOrStop("Preparing streams ...");

    // -> define variables needed to handle output stream and backup stream (required when rewriting the file)
    string originalPath = fileInfo().path(), backupPath;
    NativeFileStream &outputStream = fileInfo().stream();
    NativeFileStream backupStream; // create a stream to open the backup/original file for the case rewriting the file is required
    BinaryWriter outputWriter(&outputStream);
    char buff[8]; // buffer used to make size denotations

    if (rewriteRequired) {
        if (fileInfo().saveFilePath().empty()) {
            // move current file to temp dir and reopen it as backupStream, recreate original file
            try {
                BackupHelper::createBackupFileCanonical(fileInfo().backupDirectory(), originalPath, backupPath, outputStream, backupStream);
                // recreate original file, define buffer variables
                outputStream.open(originalPath, ios_base::out | ios_base::binary | ios_base::trunc);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(
                    DiagLevel::Critical, argsToString("Creation of temporary file (to rewrite the original file) failed: ", failure.what()), context);
                throw;
            }
        } else {
            // open the current file as backupStream and create a new outputStream at the specified "save file path"
            try {
                backupStream.exceptions(ios_base::badbit | ios_base::failbit);
                backupStream.open(BasicFileInfo::pathForOpen(fileInfo().path()).data(), ios_base::in | ios_base::binary);
                fileInfo().close();
                outputStream.open(BasicFileInfo::pathForOpen(fileInfo().saveFilePath()).data(), ios_base::out | ios_base::binary | ios_base::trunc);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical, argsToString("Opening streams to write output file failed: ", failure.what()), context);
                throw;
            }
        }

        // set backup stream as associated input stream since we need the original elements to write the new file
        setStream(backupStream);

        // TODO: reduce code duplication

    } else { // !rewriteRequired
        // buffer currently assigned attachments
        for (auto &maker : attachmentMaker) {
            maker.bufferCurrentAttachments(diag);
        }

        // reopen original file to ensure it is opened for writing
        try {
            fileInfo().close();
            outputStream.open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);
        } catch (const std::ios_base::failure &failure) {
            diag.emplace_back(DiagLevel::Critical, argsToString("Opening the file with write permissions failed: ", failure.what()), context);
            throw;
        }
    }

    // start actual writing
    try {
        // write EBML header
        progress.nextStepOrStop("Writing EBML header ...");
        outputWriter.writeUInt32BE(EbmlIds::Header);
        sizeLength = EbmlElement::makeSizeDenotation(ebmlHeaderDataSize, buff);
        outputStream.write(buff, sizeLength);
        EbmlElement::makeSimpleElement(outputStream, EbmlIds::Version, m_version);
        EbmlElement::makeSimpleElement(outputStream, EbmlIds::ReadVersion, m_readVersion);
        EbmlElement::makeSimpleElement(outputStream, EbmlIds::MaxIdLength, m_maxIdLength);
        EbmlElement::makeSimpleElement(outputStream, EbmlIds::MaxSizeLength, m_maxSizeLength);
        EbmlElement::makeSimpleElement(outputStream, EbmlIds::DocType, m_doctype);
        EbmlElement::makeSimpleElement(outputStream, EbmlIds::DocTypeVersion, m_doctypeVersion);
        EbmlElement::makeSimpleElement(outputStream, EbmlIds::DocTypeReadVersion, m_doctypeReadVersion);

        // iterates through all level 0 elements of the original file
        for (level0Element = firstElement(), segmentIndex = 0, currentPosition = 0; level0Element; level0Element = level0Element->nextSibling()) {

            // write all level 0 elements of the original file
            switch (level0Element->id()) {
            case EbmlIds::Header:
                // header has already been written -> skip it here
                break;

            case EbmlIds::Void:
            case EbmlIds::Crc32:
                // level 0 "Void"- and "Checksum"-elements are omitted
                break;

            case MatroskaIds::Segment: {
                // get reference to the current segment data instance
                SegmentData &segment = segmentData[segmentIndex];

                // write "Segment"-element actually
                progress.updateStep("Writing segment header ...");
                outputWriter.writeUInt32BE(MatroskaIds::Segment);
                sizeLength = EbmlElement::makeSizeDenotation(segment.totalDataSize, buff);
                outputStream.write(buff, sizeLength);
                segment.newDataOffset = offset = static_cast<std::uint64_t>(outputStream.tellp()); // store segment data offset here

                // write CRC-32 element ...
                if (segment.hasCrc32) {
                    // ... if the original element had a CRC-32 element
                    *buff = static_cast<char>(EbmlIds::Crc32);
                    *(buff + 1) = static_cast<char>(0x84); // length denotation: 4 byte
                    // set the value after writing the element
                    crc32Offsets.emplace_back(outputStream.tellp(), segment.totalDataSize);
                    outputStream.write(buff, 6);
                }

                // write "SeekHead"-element (except there is no seek information for the current segment)
                segment.seekInfo.make(outputStream, diag);

                // write "SegmentInfo"-element
                for (level1Element = level0Element->childById(MatroskaIds::SegmentInfo, diag); level1Element;
                     level1Element = level1Element->siblingById(MatroskaIds::SegmentInfo, diag)) {
                    // -> write ID and size
                    outputWriter.writeUInt32BE(MatroskaIds::SegmentInfo);
                    sizeLength = EbmlElement::makeSizeDenotation(segment.infoDataSize, buff);
                    outputStream.write(buff, sizeLength);
                    // -> write children
                    for (level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                        switch (level2Element->id()) {
                        case EbmlIds::Void: // skipped
                        case EbmlIds::Crc32: // skipped
                        case MatroskaIds::Title: // written separately
                        case MatroskaIds::MuxingApp: // written separately
                        case MatroskaIds::WrittingApp: // written separately
                            break;
                        default:
                            level2Element->copyBuffer(outputStream);
                            level2Element->discardBuffer();
                        }
                    }
                    // -> write "Title"-element
                    if (segmentIndex < m_titles.size()) {
                        const auto &title = m_titles[segmentIndex];
                        if (!title.empty()) {
                            EbmlElement::makeSimpleElement(outputStream, MatroskaIds::Title, title);
                        }
                    }
                    // -> write "MuxingApp"- and "WritingApp"-element
                    EbmlElement::makeSimpleElement(outputStream, MatroskaIds::MuxingApp, muxingAppName);
                    EbmlElement::makeSimpleElement(outputStream, MatroskaIds::WrittingApp, writingAppName);
                }

                // write "Tracks"-element
                if (trackHeaderElementsSize) {
                    outputWriter.writeUInt32BE(MatroskaIds::Tracks);
                    sizeLength = EbmlElement::makeSizeDenotation(trackHeaderElementsSize, buff);
                    outputStream.write(buff, sizeLength);
                    for (auto &maker : trackHeaderMaker) {
                        maker.make(outputStream);
                    }
                }

                // write "Chapters"-element
                for (level1Element = level0Element->childById(MatroskaIds::Chapters, diag); level1Element;
                     level1Element = level1Element->siblingById(MatroskaIds::Chapters, diag)) {
                    level1Element->copyBuffer(outputStream);
                    level1Element->discardBuffer();
                }

                if (newTagPos == ElementPosition::BeforeData && segmentIndex == 0) {
                    // write "Tags"-element
                    if (tagsSize) {
                        outputWriter.writeUInt32BE(MatroskaIds::Tags);
                        sizeLength = EbmlElement::makeSizeDenotation(tagElementsSize, buff);
                        outputStream.write(buff, sizeLength);
                        for (auto &maker : tagMaker) {
                            maker.make(outputStream);
                        }
                    }
                    // write "Attachments"-element
                    if (attachmentsSize) {
                        outputWriter.writeUInt32BE(MatroskaIds::Attachments);
                        sizeLength = EbmlElement::makeSizeDenotation(attachedFileElementsSize, buff);
                        outputStream.write(buff, sizeLength);
                        for (auto &maker : attachmentMaker) {
                            maker.make(outputStream, diag);
                        }
                    }
                }

                // write "Cues"-element
                if (newCuesPos == ElementPosition::BeforeData && segment.cuesElement) {
                    segment.cuesUpdater.make(outputStream, diag);
                }

                // write padding / "Void"-element
                if (segment.newPadding) {
                    // calculate length
                    std::uint64_t voidLength;
                    if (segment.newPadding < 64) {
                        sizeLength = 1;
                        *buff = static_cast<char>(voidLength = segment.newPadding - 2) | static_cast<char>(0x80);
                    } else {
                        sizeLength = 8;
                        BE::getBytes(static_cast<std::uint64_t>((voidLength = segment.newPadding - 9) | 0x100000000000000), buff);
                    }
                    // write header
                    outputWriter.writeByte(EbmlIds::Void);
                    outputStream.write(buff, sizeLength);
                    // write zeroes
                    for (; voidLength; --voidLength) {
                        outputStream.put(0);
                    }
                }

                // write media data / "Cluster"-elements
                level1Element = level0Element->childById(MatroskaIds::Cluster, diag);
                if (rewriteRequired) {
                    // update status, check whether the operation has been aborted
                    progress.nextStepOrStop("Writing cluster ...",
                        static_cast<std::uint8_t>((static_cast<std::uint64_t>(outputStream.tellp()) - offset) * 100 / segment.totalDataSize));
                    // write "Cluster"-element
                    auto clusterSizesIterator = segment.clusterSizes.cbegin();
                    unsigned int index = 0;
                    for (; level1Element; level1Element = level1Element->siblingById(MatroskaIds::Cluster, diag), ++clusterSizesIterator, ++index) {
                        // calculate position of cluster in segment
                        clusterSize = currentPosition + (static_cast<std::uint64_t>(outputStream.tellp()) - offset);
                        // write header; checking whether clusterSizesIterator is valid shouldn't be necessary
                        outputWriter.writeUInt32BE(MatroskaIds::Cluster);
                        sizeLength = EbmlElement::makeSizeDenotation(*clusterSizesIterator, buff);
                        outputStream.write(buff, sizeLength);
                        // write children
                        for (level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                            switch (level2Element->id()) {
                            case EbmlIds::Void:
                            case EbmlIds::Crc32:
                                break;
                            case MatroskaIds::Position:
                                EbmlElement::makeSimpleElement(outputStream, MatroskaIds::Position, clusterSize);
                                break;
                            default:
                                level2Element->copyEntirely(outputStream, diag, nullptr);
                            }
                        }
                        // update percentage, check whether the operation has been aborted
                        progress.stopIfAborted();
                        if (index % 50 == 0) {
                            progress.updateStepPercentage(
                                static_cast<std::uint8_t>((static_cast<std::uint64_t>(outputStream.tellp()) - offset) * 100 / segment.totalDataSize));
                        }
                    }
                } else {
                    // can't just skip existing "Cluster"-elements: "Position"-elements must be updated
                    progress.nextStepOrStop("Updating cluster ...",
                        static_cast<std::uint8_t>((static_cast<std::uint64_t>(outputStream.tellp()) - offset) * 100 / segment.totalDataSize));
                    for (; level1Element; level1Element = level1Element->nextSibling()) {
                        for (level2Element = level1Element->firstChild(); level2Element; level2Element = level2Element->nextSibling()) {
                            switch (level2Element->id()) {
                            case MatroskaIds::Position:
                                // calculate new position
                                sizeLength = EbmlElement::makeUInteger(level1Element->startOffset() - segmentData.front().newDataOffset, buff,
                                    level2Element->dataSize() > 8 ? 8 : static_cast<std::uint8_t>(level2Element->dataSize()));
                                // new position can only applied if it doesn't need more bytes than the previous position
                                if (level2Element->dataSize() < sizeLength) {
                                    // can't update position -> void position elements ("Position"-elements seem a bit useless anyways)
                                    outputStream.seekp(static_cast<streamoff>(level2Element->startOffset()));
                                    outputStream.put(static_cast<char>(EbmlIds::Void));
                                } else {
                                    // update position
                                    outputStream.seekp(static_cast<streamoff>(level2Element->dataOffset()));
                                    outputStream.write(buff, sizeLength);
                                }
                                break;
                            default:;
                            }
                        }
                    }
                    // skip existing "Cluster"-elements
                    outputStream.seekp(static_cast<streamoff>(segment.clusterEndOffset));
                }

                progress.updateStep("Writing segment tail ...");

                // write "Cues"-element
                if (newCuesPos == ElementPosition::AfterData && segment.cuesElement) {
                    segment.cuesUpdater.make(outputStream, diag);
                }

                if (newTagPos == ElementPosition::AfterData && segmentIndex == lastSegmentIndex) {
                    // write "Tags"-element
                    if (tagsSize) {
                        outputWriter.writeUInt32BE(MatroskaIds::Tags);
                        sizeLength = EbmlElement::makeSizeDenotation(tagElementsSize, buff);
                        outputStream.write(buff, sizeLength);
                        for (auto &maker : tagMaker) {
                            maker.make(outputStream);
                        }
                    }
                    // write "Attachments"-element
                    if (attachmentsSize) {
                        outputWriter.writeUInt32BE(MatroskaIds::Attachments);
                        sizeLength = EbmlElement::makeSizeDenotation(attachedFileElementsSize, buff);
                        outputStream.write(buff, sizeLength);
                        for (auto &maker : attachmentMaker) {
                            maker.make(outputStream, diag);
                        }
                    }
                }

                // increase the current segment index
                ++segmentIndex;

                // increase write offsets by the size of the segment which has just been written
                currentPosition += segment.totalSize;

                break;
            }
            default:
                // just copy any unknown top-level elements
                level0Element->copyEntirely(outputStream, diag, nullptr);
                currentPosition += level0Element->totalSize();
            }
        }

        // reparse what is written so far
        progress.updateStep("Reparsing output file ...");
        if (rewriteRequired) {
            // report new size
            fileInfo().reportSizeChanged(static_cast<std::uint64_t>(outputStream.tellp()));

            // "save as path" is now the regular path
            if (!fileInfo().saveFilePath().empty()) {
                fileInfo().reportPathChanged(fileInfo().saveFilePath());
                fileInfo().setSaveFilePath(string());
            }

            // the outputStream needs to be reopened to be able to read again
            outputStream.close();
            outputStream.open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);
            setStream(outputStream);
        } else {
            const auto newSize = static_cast<std::uint64_t>(outputStream.tellp());
            if (newSize < fileInfo().size()) {
                // file is smaller after the modification -> truncate
                // -> close stream before truncating
                outputStream.close();
                // -> truncate file
                auto ec = std::error_code();
                std::filesystem::resize_file(makeNativePath(fileInfo().path()), newSize, ec);
                if (!ec) {
                    fileInfo().reportSizeChanged(newSize);
                } else {
                    diag.emplace_back(DiagLevel::Critical, "Unable to truncate the file: " + ec.message(), context);
                }
                // -> reopen the stream again
                outputStream.open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);
            } else {
                // file is longer after the modification -> just report new size
                fileInfo().reportSizeChanged(newSize);
            }
        }
        reset();
        try {
            parseHeader(diag, progress);
        } catch (const OperationAbortedException &) {
            throw;
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Unable to reparse the header of the new file.", context);
            throw;
        }

        // update CRC-32 checksums
        if (!crc32Offsets.empty()) {
            progress.updateStep("Updating CRC-32 checksums ...");
            for (const auto &crc32Offset : crc32Offsets) {
                outputStream.seekg(static_cast<streamoff>(get<0>(crc32Offset) + 6));
                outputStream.seekp(static_cast<streamoff>(get<0>(crc32Offset) + 2));
                writer().writeUInt32LE(reader().readCrc32(get<1>(crc32Offset) - 6));
            }
        }

        // prevent deferring final write operations (to catch and handle possible errors here)
        outputStream.flush();

        // handle errors (which might have been occurred after renaming/creating backup file)
    } catch (...) {
        BackupHelper::handleFailureAfterFileModifiedCanonical(fileInfo(), originalPath, backupPath, outputStream, backupStream, diag, context);
    }
}

} // namespace TagParser
