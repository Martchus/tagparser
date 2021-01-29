#ifndef TAG_PARSER_ABSTRACTCONTAINER_H
#define TAG_PARSER_ABSTRACTCONTAINER_H

#include "./exceptions.h"
#include "./settings.h"
#include "./tagtarget.h"

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <iostream>

namespace CppUtilities {
class BinaryReader;
class BinaryWriter;
} // namespace CppUtilities

namespace TagParser {

class Tag;
class AbstractTrack;
class AbstractChapter;
class AbstractAttachment;
class Diagnostics;
class AbortableProgressFeedback;
enum class KnownField : unsigned int;

class TAG_PARSER_EXPORT AbstractContainer {
public:
    virtual ~AbstractContainer();

    std::iostream &stream();
    void setStream(std::iostream &stream);
    std::uint64_t startOffset() const;
    CppUtilities::BinaryReader &reader();
    CppUtilities::BinaryWriter &writer();

    void parseHeader(Diagnostics &diag);
    void parseTags(Diagnostics &diag);
    void parseTracks(Diagnostics &diag);
    void parseChapters(Diagnostics &diag);
    void parseAttachments(Diagnostics &diag);
    void makeFile(Diagnostics &diag, AbortableProgressFeedback &progress);

    bool isHeaderParsed() const;
    bool areTagsParsed() const;
    bool areTracksParsed() const;
    bool areChaptersParsed() const;
    bool areAttachmentsParsed() const;

    virtual Tag *createTag(const TagTarget &target = TagTarget());
    virtual Tag *tag(std::size_t index);
    virtual std::size_t tagCount() const;
    virtual bool removeTag(Tag *tag);
    virtual void removeAllTags();
    virtual ElementPosition determineTagPosition(Diagnostics &diag) const;

    virtual AbstractTrack *track(std::size_t index);
    virtual std::size_t trackCount() const;
    virtual bool removeTrack(AbstractTrack *track);
    virtual void removeAllTracks();
    virtual bool supportsTrackModifications() const;
    virtual ElementPosition determineIndexPosition(Diagnostics &diag) const;

    virtual AbstractChapter *chapter(std::size_t index);
    virtual std::size_t chapterCount() const;

    virtual AbstractAttachment *createAttachment();
    virtual AbstractAttachment *attachment(std::size_t index);
    virtual std::size_t attachmentCount() const;

    virtual std::initializer_list<KnownField> tagFieldsStoredAsAttachments() const;
    virtual bool mapTagsToAttachments(Diagnostics &diag, std::initializer_list<KnownField> tagFields = {});
    virtual bool mapAttachmentsToTags(Diagnostics &diag, std::initializer_list<KnownField> tagFields = {});

    std::uint64_t version() const;
    std::uint64_t readVersion() const;
    const std::string &documentType() const;
    std::uint64_t doctypeVersion() const;
    std::uint64_t doctypeReadVersion() const;
    const std::vector<std::string> &titles() const;
    void setTitle(const std::string &title, std::size_t segmentIndex = 0);
    virtual bool supportsTitle() const;
    virtual std::size_t segmentCount() const;
    CppUtilities::TimeSpan duration() const;
    CppUtilities::DateTime creationTime() const;
    CppUtilities::DateTime modificationTime() const;
    std::uint32_t timeScale() const;

    virtual void reset();

protected:
    AbstractContainer(std::iostream &stream, std::uint64_t startOffset);

    virtual void internalParseHeader(Diagnostics &diag);
    virtual void internalParseTags(Diagnostics &diag);
    virtual void internalParseTracks(Diagnostics &diag);
    virtual void internalParseChapters(Diagnostics &diag);
    virtual void internalParseAttachments(Diagnostics &diag);
    virtual void internalMakeFile(Diagnostics &diag, AbortableProgressFeedback &progress);

    std::uint64_t m_version;
    std::uint64_t m_readVersion;
    std::string m_doctype;
    std::uint64_t m_doctypeVersion;
    std::uint64_t m_doctypeReadVersion;
    std::vector<std::string> m_titles;
    CppUtilities::TimeSpan m_duration;
    CppUtilities::DateTime m_creationTime;
    CppUtilities::DateTime m_modificationTime;
    std::uint32_t m_timeScale;

    bool m_headerParsed;
    bool m_tagsParsed;
    bool m_tracksParsed;
    bool m_tracksAltered;
    bool m_chaptersParsed;
    bool m_attachmentsParsed;

private:
    std::uint64_t m_startOffset;
    std::iostream *m_stream;
    CppUtilities::BinaryReader m_reader;
    CppUtilities::BinaryWriter m_writer;
};

/*!
 * \brief Returns the related stream.
 */
inline std::iostream &AbstractContainer::stream()
{
    return *m_stream;
}

/*!
 * \brief Sets the related stream.
 */
inline void AbstractContainer::setStream(std::iostream &stream)
{
    m_stream = &stream;
    m_reader.setStream(m_stream);
    m_writer.setStream(m_stream);
}

/*!
 * \brief Returns the start offset in the related stream.
 */
inline std::uint64_t AbstractContainer::startOffset() const
{
    return m_startOffset;
}

/*!
 * \brief Returns the related BinaryReader.
 */
inline CppUtilities::BinaryReader &AbstractContainer::reader()
{
    return m_reader;
}

/*!
 * \brief Returns the related BinaryWriter.
 */
inline CppUtilities::BinaryWriter &AbstractContainer::writer()
{
    return m_writer;
}

/*!
 * \brief Returns an indication whether the header has been parsed yet.
 */
inline bool AbstractContainer::isHeaderParsed() const
{
    return m_headerParsed;
}

/*!
 * \brief Returns an indication whether the tags have been parsed yet.
 */
inline bool AbstractContainer::areTagsParsed() const
{
    return m_tagsParsed;
}

/*!
 * \brief Returns an indication whether the chapters have been parsed yet.
 */
inline bool AbstractContainer::areChaptersParsed() const
{
    return m_chaptersParsed;
}

/*!
 * \brief Returns an indication whether the attachments have been parsed yet.
 */
inline bool AbstractContainer::areAttachmentsParsed() const
{
    return m_attachmentsParsed;
}

/*!
 * \brief Returns tag fields which are actually stored as attachments by the underlying format.
 */
inline std::initializer_list<KnownField> AbstractContainer::tagFieldsStoredAsAttachments() const
{
    static constexpr auto tagFields = std::initializer_list<KnownField>();
    return tagFields;
}

/*!
 * \brief Turns assigned tags which actually need to be treated as attachments by the underlying format into attachments.
 * \remarks
 * - Affects all tag fields returned by tagFieldsStoredAsAttachments() if \a tagFields is empty. Otherwise affects only the
 *   specified \a tagFields (if those are convertible to attachments).
 * - Call this function before applying changes to be able to store the specified \a tagFields as attachments if that is how
 *   these tags are actually stored by the underlying format.
 * - The affected tags will no longer be accessible as tags.
 */
inline bool AbstractContainer::mapTagsToAttachments(Diagnostics &diag, std::initializer_list<KnownField> tagFields)
{
    CPP_UTILITIES_UNUSED(diag)
    CPP_UTILITIES_UNUSED(tagFields)
    return true;
}

/*!
 * \brief Turns assigned attachments which are commonly treated as tags into tags.
 * \remarks
 * - Call this function after parsing a file to be able to obtain the specified \a tagFields as tags.
 * - Affects all tag fields returned by tagFieldsStoredAsAttachments() if \a tagFields is empty. Otherwise affects only the
 *   specified \a tagFields (if those are convertible to attachments).
 * - The affected attachments will no longer be accessible as attachments.
 * - If you've been calling this function, be sure to call the inverse function mapTagsToAttachments() before applying
 *   changes.
 */
inline bool AbstractContainer::mapAttachmentsToTags(Diagnostics &diag, std::initializer_list<KnownField> tagFields)
{
    CPP_UTILITIES_UNUSED(diag)
    CPP_UTILITIES_UNUSED(tagFields)
    return true;
}

/*!
 * \brief Returns an indication whether the tracks have been parsed yet.
 */
inline bool AbstractContainer::areTracksParsed() const
{
    return m_tracksParsed;
}

/*!
 * \brief Returns the version if known; otherwise returns 0.
 */
inline std::uint64_t AbstractContainer::version() const
{
    return m_version;
}

/*!
 * \brief Returns the "read version" if known; otherwise returns 0.
 *
 * This is the minimum version a parser has to support to read the file.
 */
inline std::uint64_t AbstractContainer::readVersion() const
{
    return m_readVersion;
}

/*!
 * \brief Returns a string that describes the document type if available; otherwise returns an empty string.
 */
inline const std::string &AbstractContainer::documentType() const
{
    return m_doctype;
}

/*!
 * \brief Returns the document type version if known; otherwise returns 0.
 */
inline std::uint64_t AbstractContainer::doctypeVersion() const
{
    return m_doctypeVersion;
}

/*!
 * \brief Returns the document type "read version" if known; otherwise returns 0.
 *
 * This is the minimum version an interpreter has to support to read the file.
 */
inline std::uint64_t AbstractContainer::doctypeReadVersion() const
{
    return m_doctypeReadVersion;
}

/*!
 * \brief Returns the title(s) of the file.
 * \remarks
 *  - If the container does not support titles an empty vector will be returned.
 *  - If there are multiple segments, the title of each segment is returned.
 * \sa setTitle()
 */
inline const std::vector<std::string> &AbstractContainer::titles() const
{
    return m_titles;
}

/*!
 * \brief Sets the title for the specified segment.
 * \remarks The title is ignored if it is not supported by the concrete container format.
 * \throws Throws out_of_range if the segment does not exist.
 * \sa titles()
 */
inline void AbstractContainer::setTitle(const std::string &title, std::size_t segmentIndex)
{
    m_titles.at(segmentIndex) = title;
}

/*!
 * \brief Returns the duration of the file if known; otherwise returns a time span of zero ticks.
 */
inline CppUtilities::TimeSpan AbstractContainer::duration() const
{
    return m_duration;
}

/*!
 * \brief Returns the creation time of the file if known; otherwise the returned date time is null.
 */
inline CppUtilities::DateTime AbstractContainer::creationTime() const
{
    return m_creationTime;
}

/*!
 * \brief Returns the modification time of the file if known; otherwise the returned date time is null.
 */
inline CppUtilities::DateTime AbstractContainer::modificationTime() const
{
    return m_modificationTime;
}

/*!
 * \brief Returns the time scale of the file if known; otherwise returns 0.
 */
inline std::uint32_t AbstractContainer::timeScale() const
{
    return m_timeScale;
}

} // namespace TagParser

#endif // TAG_PARSER_ABSTRACTCONTAINER_H
