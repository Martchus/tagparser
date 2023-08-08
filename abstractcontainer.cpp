#include "./abstractcontainer.h"
#include "./diagnostics.h"

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/// \brief The AbstractContainerPrivate struct contains private fields of the AbstractContainer class.
struct AbstractContainerPrivate {
    std::vector<std::string> muxingApps, writingApps;
};

/*!
 * \class TagParser::AbstractContainer
 * \brief The AbstractContainer class provides an interface and common functionality to parse and make a certain container format.
 */

/*!
 * \brief Constructs a new container for the specified file \a stream at the specified \a startOffset.
 */
AbstractContainer::AbstractContainer(iostream &stream, std::uint64_t startOffset)
    : m_version(0)
    , m_readVersion(0)
    , m_doctypeVersion(0)
    , m_doctypeReadVersion(0)
    , m_timeScale(0)
    , m_headerParsed(false)
    , m_tagsParsed(false)
    , m_tracksParsed(false)
    , m_tracksAltered(false)
    , m_chaptersParsed(false)
    , m_attachmentsParsed(false)
    , m_startOffset(startOffset)
    , m_stream(&stream)
    , m_reader(BinaryReader(m_stream))
    , m_writer(BinaryWriter(m_stream))
{
}

/*!
 * \brief Destroys the container.
 *
 * Destroys the reader, the writer and track, tag, chapter and attachment objects as well.
 * Does NOT destroy the stream which has been specified when constructing the object.
 */
AbstractContainer::~AbstractContainer()
{
}

/*!
 * \brief Parses the header if not parsed yet.
 *
 * The information will be read from the associated stream. The stream and the start offset
 * have been specified when constructing the object.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Failure or a derived class when an parsing error occurs.
 */
void AbstractContainer::parseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    if (!isHeaderParsed()) {
        removeAllTags();
        removeAllTracks();
        internalParseHeader(diag, progress);
        m_headerParsed = true;
    }
}

/*!
 * \brief Parses the tag information if not parsed yet.
 *
 * The header will be parsed before if not parsed yet.
 *
 * The information will be read from the associated stream. The stream and the start offset
 * have been specified when constructing the object.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Failure or a derived class when an parsing error occurs.
 *
 * \sa parseHeader()
 * \sa parseTracks()
 * \sa parseAttachments()
 * \sa parseChapters()
 * \sa tags()
 */
void AbstractContainer::parseTags(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    if (!areTagsParsed()) {
        parseHeader(diag, progress);
        internalParseTags(diag, progress);
        m_tagsParsed = true;
    }
}

/*!
 * \brief Parses the tracks of the file if not parsed yet.
 *
 * The header will be parsed before if not parsed yet.
 *
 * The information will be read from the associated stream. The stream and the start offset
 * have been specified when constructing the object.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Failure or a derived class when an parsing error occurs.
 *
 * \sa parseHeader()
 * \sa parseTags()
 * \sa tracks()
 */
void AbstractContainer::parseTracks(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    if (!areTracksParsed()) {
        parseHeader(diag, progress);
        internalParseTracks(diag, progress);
        m_tracksParsed = true;
        m_tracksAltered = false;
    }
}

/*!
 * \brief Parses the chapters of the file if not parsed yet.
 *
 * The information will be read from the associated stream. The stream and the start offset
 * have been specified when constructing the object.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void AbstractContainer::parseChapters(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    if (!areChaptersParsed()) {
        parseHeader(diag, progress);
        internalParseChapters(diag, progress);
        m_chaptersParsed = true;
    }
}

/*!
 * \brief Parses the attachments of the file if not parsed yet.
 *
 * The information will be read from the associated stream. The stream and the start offset
 * have been specified when constructing the object.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void AbstractContainer::parseAttachments(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    if (!areAttachmentsParsed()) {
        parseHeader(diag, progress);
        internalParseAttachments(diag, progress);
        m_attachmentsParsed = true;
    }
}

/*!
 * \brief Rewrites the file to apply changed tag information.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 */
void AbstractContainer::makeFile(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    internalMakeFile(diag, progress);
}

/*!
 * \brief Returns whether the implementation supports adding or removing of tracks.
 */
bool AbstractContainer::supportsTrackModifications() const
{
    return false;
}

/*!
 * \brief Determines the position of the index.
 * \returns Returns ElementPosition::BeforeData or ElementPosition::AfterData if the position could
 *          be determined; otherwise returns ElementPosition::Keep.
 * \remarks
 * - It might be required to parse tracks before the index position can be determined.
 * - Not be applicable for files composed of multiple segments.
 * \sa MediaFileInfo::indexPosition()
 */
ElementPosition AbstractContainer::determineIndexPosition(Diagnostics &diag) const
{
    CPP_UTILITIES_UNUSED(diag);
    return ElementPosition::Keep;
}

/*!
 * \brief Internally called to parse the header.
 *
 * Must be implemented when subclassing to provide this feature.
 *
 * \throws Throws Failure or a derived class when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
void AbstractContainer::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(diag);
    CPP_UTILITIES_UNUSED(progress);
    throw NotImplementedException();
}

/*!
 * \brief Internally called to parse the tags.
 *
 * Must be implemented when subclassing to provide this feature.
 *
 * \throws Throws Failure or a derived class when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
void AbstractContainer::internalParseTags(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(diag);
    CPP_UTILITIES_UNUSED(progress);
    throw NotImplementedException();
}

/*!
 * \brief Internally called to parse the tracks.
 *
 * Must be implemented when subclassing to provide this feature.
 *
 * \throws Throws Failure or a derived class when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
void AbstractContainer::internalParseTracks(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(diag);
    CPP_UTILITIES_UNUSED(progress);
    throw NotImplementedException();
}

/*!
 * \brief Internally called to parse the chapters.
 *
 * Must be implemented when subclassing to provide this feature.
 *
 * \throws Throws Failure or a derived class when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
void AbstractContainer::internalParseChapters(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(diag);
    CPP_UTILITIES_UNUSED(progress);
    throw NotImplementedException();
}

/*!
 * \brief Internally called to parse the attachments.
 *
 * Must be implemented when subclassing to provide this feature.
 *
 * \throws Throws Failure or a derived class when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
void AbstractContainer::internalParseAttachments(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(diag);
    CPP_UTILITIES_UNUSED(progress);
    throw NotImplementedException();
}

/*!
 * \brief Internally called to make the file.
 *
 * Must be implemented when subclassing.
 *
 * \throws Throws Failure or a derived class when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
void AbstractContainer::internalMakeFile(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(diag);
    CPP_UTILITIES_UNUSED(progress);
    throw NotImplementedException();
}

/*!
 * \brief Creates and returns a tag for the specified \a target.
 * \remarks
 *          - If there is already a tag (for the specified \a target) present,
 *            no new tag will be created. The present tag will be returned instead.
 *          - If an empty \a target is specified it will be ignored.
 *          - If targets aren't supported the specified \a target will be ignored.
 *          - If no tag could be created, nullptr is returned.
 *          - The container keeps the ownership over the created tag.
 */
Tag *AbstractContainer::createTag(const TagTarget &)
{
    return nullptr;
}

/*!
 * \brief Returns the tag with the specified \a index.
 *
 * \a index must be less than tagCount().
 */
Tag *AbstractContainer::tag(size_t index)
{
    CPP_UTILITIES_UNUSED(index);
    return nullptr;
}

/*!
 * \brief Returns the number of tags attached to the container.
 *
 * This method returns zero if the tags have not been parsed yet.
 */
size_t AbstractContainer::tagCount() const
{
    return 0;
}

/*!
 * \brief Removes the specified \a tag from the container.
 *
 * Does nothing if the tag is not attached to the container.
 *
 * The tags need to be parsed before a removal is possible.
 * \sa areTagsParsed()
 * \sa parseTags()
 *
 * \remarks The \a tag is not destroyed. The ownership is transferred to the caller.
 *
 * \returns Returns whether the \a tag could be removed.
 */
bool AbstractContainer::removeTag(Tag *tag)
{
    CPP_UTILITIES_UNUSED(tag);
    return false;
}

/*!
 * \brief Removes all tags attached to the container.
 *
 * The tags need to be parsed before they can be removed.
 * \sa areTagsParsed()
 * \sa parseTags()
 *
 * \remarks The current tag objects are destroyed. All pointers obtained
 *          by calling tag() are invalidated.
 */
void AbstractContainer::removeAllTags()
{
}

/*!
 * \brief Determines the position of the tags inside the file.
 * \returns Returns ElementPosition::BeforeData or ElementPosition::AfterData if the position could
 *          be determined; otherwise returns ElementPosition::Keep.
 * \remarks
 * - It might be required to parse tags before the tag position can be determined.
 * - Not be applicable for files composed of multiple segments.
 * \sa MediaFileInfo::tagPosition()
 */
ElementPosition AbstractContainer::determineTagPosition(Diagnostics &diag) const
{
    CPP_UTILITIES_UNUSED(diag);
    return ElementPosition::Keep;
}

/*!
 * \brief Returns the track with the specified \a index.
 *
 * \a index must be less than trackCount().
 */
AbstractTrack *AbstractContainer::track(size_t index)
{
    CPP_UTILITIES_UNUSED(index);
    return nullptr;
}

/*!
 * \brief Returns the number of tracks the container holds.
 */
size_t AbstractContainer::trackCount() const
{
    return 0;
}

/*!
 * \brief Removes the specified \a track to the container.
 *
 * Removal of tracks might be not supported by the implementation.
 * \sa supportsTrackModifications()
 *
 * The tracks need to be parsed before a removal is possible.
 * \sa areTracksParsed()
 * \sa parseTracks()
 *
 * \remarks The \a track is not destroyed. The ownership is transferred to the caller.
 *
 * \returns Returns whether the \a track could be removed.
 */
bool AbstractContainer::removeTrack(AbstractTrack *track)
{
    CPP_UTILITIES_UNUSED(track);
    return false;
}

/*!
 * \brief Removes all tracks from the container.
 *
 * Modifying tracks might be not supported by the implementation.
 * \sa supportsTrackModifications()
 *
 * The tracks need to be parsed before they can be removed.
 * \sa areTracksParsed()
 * \sa parseTracks()
 *
 * \remarks The current track objects are destroyed. All pointers obtained
 *          by calling track() are invalidated.
 */
void AbstractContainer::removeAllTracks()
{
}

/*!
 * \brief Returns the chapter with the specified \a index.
 *
 * \a index must be less than chapterCount().
 */
AbstractChapter *AbstractContainer::chapter(size_t index)
{
    CPP_UTILITIES_UNUSED(index);
    return nullptr;
}

/*!
 * \brief Returns the number of chapters the container holds.
 */
size_t AbstractContainer::chapterCount() const
{
    return 0;
}

/*!
 * \brief Creates and returns a new attachment.
 * \returns Returns the created attachment.
 * \remarks The container keeps the ownership over the created attachment.
 */
AbstractAttachment *AbstractContainer::createAttachment()
{
    return nullptr;
}

/*!
 * \brief Returns the attachment with the specified \a index.
 *
 * \a index must be less than attachmentCount().
 */
AbstractAttachment *AbstractContainer::attachment(size_t index)
{
    CPP_UTILITIES_UNUSED(index);
    return nullptr;
}

/*!
 * \brief Returns the number of attachments the container holds.
 */
size_t AbstractContainer::attachmentCount() const
{
    return 0;
}

/*!
 * \brief Returns whether the title property is supported.
 */
bool AbstractContainer::supportsTitle() const
{
    return false;
}

/*!
 * \brief Returns the muxing applications specified as meta-data.
 */
const std::vector<std::string> &AbstractContainer::muxingApplications() const
{
    static const auto empty = std::vector<std::string>();
    return m_p ? m_p->muxingApps : empty;
}

/*!
 * \brief Returns the muxing applications specified as meta-data.
 */
std::vector<std::string> &AbstractContainer::muxingApplications()
{
    return p()->muxingApps;
}

/*!
 * \brief Returns the writing applications specified as meta-data.
 */
const std::vector<std::string> &AbstractContainer::writingApplications() const
{
    static const auto empty = std::vector<std::string>();
    return m_p ? m_p->writingApps : empty;
}

/*!
 * \brief Returns the writing applications specified as meta-data.
 */
std::vector<std::string> &AbstractContainer::writingApplications()
{
    return p()->writingApps;
}

/*!
 * \brief Returns the number of segments.
 */
size_t AbstractContainer::segmentCount() const
{
    return 1;
}

/*!
 * \brief Discards all parsing results.
 */
void AbstractContainer::reset()
{
    m_headerParsed = false;
    m_tagsParsed = false;
    m_tracksParsed = false;
    m_tracksAltered = false;
    m_chaptersParsed = false;
    m_attachmentsParsed = false;
    m_version = 0;
    m_readVersion = 0;
    m_doctypeVersion = 0;
    m_doctypeReadVersion = 0;
    m_timeScale = 0;
    m_titles.clear();
}

/*!
 * \brief Returns the private data for the container.
 */
std::unique_ptr<AbstractContainerPrivate> &AbstractContainer::p()
{
    if (!m_p) {
        m_p = std::make_unique<AbstractContainerPrivate>();
    }
    return m_p;
}

} // namespace TagParser
