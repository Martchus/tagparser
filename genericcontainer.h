#ifndef TAG_PARSER_GENERICCONTAINER_H
#define TAG_PARSER_GENERICCONTAINER_H

#include "./abstractcontainer.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace TagParser {

/*!
 * \class TagParser::GenericContainer
 * \brief The GenericContainer class helps parsing header, track, tag and chapter information
 *        of a file.
 *
 * \tparam FileInfoType Specifies the file info class (a class derived from TagParser::BasicFileInfo) which is used to specify the related file.
 * \tparam TagType Specifies the class which is used to deal with the tag information of the file.
 * \tparam TrackType Specifies the class which is used to deal with the track of the file.
 * \tparam ElementType Specifies the class which is used to deal with the elements the file consists of.
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType> class TAG_PARSER_EXPORT GenericContainer : public AbstractContainer {
    friend FileInfoType;

public:
    GenericContainer(FileInfoType &fileInfo, std::uint64_t startOffset);
    ~GenericContainer() override;

    void validateElementStructure(Diagnostics &diag, AbortableProgressFeedback &progress, std::uint64_t *paddingSize = nullptr);
    FileInfoType &fileInfo() const;
    ElementType *firstElement() const;
    const std::vector<std::unique_ptr<ElementType>> &additionalElements() const;
    std::vector<std::unique_ptr<ElementType>> &additionalElements();
    TagType *tag(std::size_t index) override;
    std::size_t tagCount() const override;
    TrackType *track(std::size_t index) override;
    TrackType *trackById(std::uint64_t id);
    std::size_t trackCount() const override;
    const std::vector<std::unique_ptr<TagType>> &tags() const;
    std::vector<std::unique_ptr<TagType>> &tags();
    const std::vector<std::unique_ptr<TrackType>> &tracks() const;
    std::vector<std::unique_ptr<TrackType>> &tracks();

    TagType *createTag(const TagTarget &target = TagTarget()) override;
    bool removeTag(Tag *tag) override;
    void removeAllTags() override;
    bool addTrack(TrackType *track);
    bool removeTrack(AbstractTrack *track) override;
    void removeAllTracks() override;
    void reset() override;

    using ContainerFileInfoType = FileInfoType;
    using ContainerTagType = TagType;
    using ContainerTrackType = TrackType;
    using ContainerElementType = ElementType;

protected:
    std::unique_ptr<ElementType> m_firstElement;
    std::vector<std::unique_ptr<ElementType>> m_additionalElements;
    std::vector<std::unique_ptr<TagType>> m_tags;
    std::vector<std::unique_ptr<TrackType>> m_tracks;

private:
    FileInfoType *m_fileInfo;
};

/*!
 * \brief Constructs a new container for the specified \a fileInfo at the specified \a startOffset.
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
GenericContainer<FileInfoType, TagType, TrackType, ElementType>::GenericContainer(FileInfoType &fileInfo, std::uint64_t startOffset)
    : AbstractContainer(fileInfo.stream(), startOffset)
    , m_fileInfo(&fileInfo)
{
}

/*!
 * \brief Destroys the container.
 *
 * Destroys the reader, the writer and track, tag, chapter and attachment objects as well.
 * Does NOT destroy the stream which has been specified when constructing the object.
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
GenericContainer<FileInfoType, TagType, TrackType, ElementType>::~GenericContainer()
{
}

/*!
 * \brief Parses all elements the file consists of.
 *
 * All parsing notifications will be stored in \a gatheredNotifications.
 * The size of padding/void elements will be accumulated and stored in
 * at \a paddingSize if it is not a null pointer.
 *
 * \throws Throws Failure or a derived class when a parsing error occurs.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline void GenericContainer<FileInfoType, TagType, TrackType, ElementType>::validateElementStructure(
    Diagnostics &diag, AbortableProgressFeedback &progress, std::uint64_t *paddingSize)
{
    parseHeader(diag, progress);
    if (m_firstElement) {
        m_firstElement->validateSubsequentElementStructure(diag, paddingSize, &progress);
    }
}

/*!
 * \brief Returns the related file info.
 *
 * The related file info has been specified when constructing the container.
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline FileInfoType &GenericContainer<FileInfoType, TagType, TrackType, ElementType>::fileInfo() const
{
    return *m_fileInfo;
}

/*!
 * \brief Returns the first element of the file if available; otherwiese returns nullptr.
 *
 * This method gives access to the element structure of the container - the entire element tree
 * can be looked up using the nextSibling() and firstChild() methods of the returned element.
 *
 * The header needs to be parsed before (see parseHeader()).
 *
 * The container keeps ownership over the returned element.
 *
 * \sa isHeaderParsed()
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline ElementType *GenericContainer<FileInfoType, TagType, TrackType, ElementType>::firstElement() const
{
    return m_firstElement.get();
}

/*!
 * \brief Returns all available additional elements.
 *
 * The parser might decide to split up a file's element tree to skip irrelevant elements to achieve better performance.
 * This method gives access to those sub element trees. Each of the returned elements represents an independent element
 * tree within the file.
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline const std::vector<std::unique_ptr<ElementType>> &GenericContainer<FileInfoType, TagType, TrackType, ElementType>::additionalElements() const
{
    return m_additionalElements;
}

/*!
 * \brief Returns all available additional elements.
 *
 * The parser might decide to split up a file's element tree to skip irrelevant elements to achieve better performance.
 * This method gives access to those sub element trees. Each of the returned elements represents an independent element
 * tree within the file.
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline std::vector<std::unique_ptr<ElementType>> &GenericContainer<FileInfoType, TagType, TrackType, ElementType>::additionalElements()
{
    return m_additionalElements;
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline TagType *GenericContainer<FileInfoType, TagType, TrackType, ElementType>::tag(std::size_t index)
{
    return m_tags[index].get();
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline std::size_t GenericContainer<FileInfoType, TagType, TrackType, ElementType>::tagCount() const
{
    return m_tags.size();
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline TrackType *GenericContainer<FileInfoType, TagType, TrackType, ElementType>::track(std::size_t index)
{
    return m_tracks[index].get();
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline TrackType *GenericContainer<FileInfoType, TagType, TrackType, ElementType>::trackById(std::uint64_t id)
{
    for (auto &track : m_tracks) {
        if (track->id() == id) {
            return track.get();
        }
    }
    return nullptr;
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline std::size_t GenericContainer<FileInfoType, TagType, TrackType, ElementType>::trackCount() const
{
    return m_tracks.size();
}

/*!
 * \brief Returns the tags of the file.
 *
 * The tags need to be parsed before (see parseTags()).
 *
 * The container keeps ownership over the returned tags.
 *
 * \sa areTagsParsed()
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline const std::vector<std::unique_ptr<TagType>> &GenericContainer<FileInfoType, TagType, TrackType, ElementType>::tags() const
{
    return m_tags;
}

/*!
 * \brief Returns the tags of the file.
 *
 * The tags need to be parsed before (see parseTags()).
 *
 * The container keeps ownership over the returned tags. Do not push or remove elements to the returned vector.
 *
 * \sa areTagsParsed()
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline std::vector<std::unique_ptr<TagType>> &GenericContainer<FileInfoType, TagType, TrackType, ElementType>::tags()
{
    return m_tags;
}

/*!
 * \brief Returns the tracks of the file.
 *
 * The tags need to be parsed before (see parseTracks()).
 *
 * The container keeps ownership over the returned tracks.
 *
 * \sa areTracksParsed()
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline const std::vector<std::unique_ptr<TrackType>> &GenericContainer<FileInfoType, TagType, TrackType, ElementType>::tracks() const
{
    return m_tracks;
}

/*!
 * \brief Returns the tracks of the file.
 *
 * The tags need to be parsed before (see parseTracks()).
 *
 * The container keeps ownership over the returned tracks. Do not push or remove elements to the returned vector.
 *
 * \sa areTracksParsed()
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline std::vector<std::unique_ptr<TrackType>> &GenericContainer<FileInfoType, TagType, TrackType, ElementType>::tracks()
{
    return m_tracks;
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
TagType *GenericContainer<FileInfoType, TagType, TrackType, ElementType>::createTag(const TagTarget &target)
{
    // check whether a tag matching the specified target is already assigned
    if (!m_tags.empty()) {
        if (!target.isEmpty() && m_tags.front()->supportsTarget()) {
            for (auto &tag : m_tags) {
                if (tag->target() == target) {
                    return tag.get();
                }
            }
        } else {
            return m_tags.front().get();
        }
    }

    // a new tag must be created
    const auto &tag = m_tags.emplace_back(std::make_unique<TagType>());
    tag->setTarget(target);
    return tag.get();
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
bool GenericContainer<FileInfoType, TagType, TrackType, ElementType>::removeTag(Tag *tag)
{
    if (const auto size = m_tags.size()) {
        m_tags.erase(std::remove_if(m_tags.begin(), m_tags.end(),
                         [tag](const std::unique_ptr<TagType> &existingTag) -> bool { return static_cast<Tag *>(existingTag.get()) == tag; }),
            m_tags.end());
        return size != m_tags.size();
    }
    return false;
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
inline void GenericContainer<FileInfoType, TagType, TrackType, ElementType>::removeAllTags()
{
    m_tags.clear();
}

/*!
 * \brief Adds the specified \a track to the container.
 *
 * Adding tracks might be not supported by the implementation.
 * \sa supportsTrackModifications()
 *
 * The tracks needs to be parsed before additional tracks can be added.
 * \sa areTracksParsed()
 * \sa parseTracks()
 *
 * \remarks The container takes ownership over the specified \a track if it was possible
 *          to add the track. This makes adding a track from another container impossible
 *          without removing it from the other container first.
 *
 * \returns Returns an indication whether the \a track could be added.
 */
template <class FileInfoType, class TagType, class TrackType, class ElementType>
bool GenericContainer<FileInfoType, TagType, TrackType, ElementType>::addTrack(TrackType *track)
{
    if (!areTracksParsed() || !supportsTrackModifications()) {
        return false;
    }
    // ensure ID is unique
    auto id = track->id();
ensureIdIsUnique:
    for (const auto &existingTrack : m_tracks) {
        if (existingTrack->id() == id) {
            ++id;
            goto ensureIdIsUnique;
        }
    }
    track->setId(id);

    m_tracks.emplace_back(track);
    return m_tracksAltered = true;
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
bool GenericContainer<FileInfoType, TagType, TrackType, ElementType>::removeTrack(AbstractTrack *track)
{
    if (!areTracksParsed() || !supportsTrackModifications() || m_tracks.empty()) {
        return false;
    }
    auto removed = false;
    for (auto i = m_tracks.end() - 1, begin = m_tracks.begin();; --i) {
        if (static_cast<AbstractTrack *>(i->get()) == track) {
            i->release();
            m_tracks.erase(i);
            removed = true;
        }
        if (i == begin) {
            break;
        }
    }
    if (removed) {
        m_tracksAltered = true;
    }
    return removed;
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
void GenericContainer<FileInfoType, TagType, TrackType, ElementType>::removeAllTracks()
{
    if (areTracksParsed() && supportsTrackModifications() && m_tracks.size()) {
        m_tracks.clear();
        m_tracksAltered = true;
    }
}

template <class FileInfoType, class TagType, class TrackType, class ElementType>
void GenericContainer<FileInfoType, TagType, TrackType, ElementType>::reset()
{
    AbstractContainer::reset();
    m_firstElement.reset();
    m_additionalElements.clear();
    m_tracks.clear();
    m_tags.clear();
}

} // namespace TagParser

#endif // TAG_PARSER_GENERICCONTAINER_H
