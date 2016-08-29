#ifndef ID3V2TAG_H
#define ID3V2TAG_H

#include "./id3v2frame.h"

#include "../fieldbasedtag.h"

#include <map>

namespace Media
{

class Id3v2Tag;

struct TAG_PARSER_EXPORT FrameComparer
{
    bool operator()(const uint32 &lhs, const uint32 &rhs) const;
};

class TAG_PARSER_EXPORT Id3v2TagMaker
{
    friend class Id3v2Tag;

public:
    void make(std::ostream &stream, uint32 padding);
    const Id3v2Tag &tag() const;
    uint64 requiredSize() const;

private:
    Id3v2TagMaker(Id3v2Tag &tag);

    Id3v2Tag &m_tag;
    uint32 m_framesSize;
    uint32 m_requiredSize;
    std::vector<Id3v2FrameMaker> m_maker;
};

/*!
 * \brief Returns the associated tag.
 */
inline const Id3v2Tag &Id3v2TagMaker::tag() const
{
    return m_tag;
}

/*!
 * \brief Returns the number of bytes which will be written when making the tag.
 * \remarks Excludes padding!
 */
inline uint64 Id3v2TagMaker::requiredSize() const
{
    return m_requiredSize;
}

class TAG_PARSER_EXPORT Id3v2Tag : public FieldMapBasedTag<Id3v2Frame, FrameComparer>
{
public:
    Id3v2Tag();

    TagType type() const;
    const char *typeName() const;
    TagTextEncoding proposedTextEncoding() const;
    bool canEncodingBeUsed(TagTextEncoding encoding) const;
    uint32 fieldId(KnownField value) const;
    KnownField knownField(const uint32 &id) const;
    TagDataType proposedDataType(const uint32 &id) const;
    using FieldMapBasedTag<Id3v2Frame, FrameComparer>::value;
    const TagValue &value(const typename Id3v2Frame::identifierType &id) const;
    using FieldMapBasedTag<Id3v2Frame, FrameComparer>::setValue;
    bool setValue(const typename Id3v2Frame::identifierType &id, const TagValue &value);
    bool supportsDescription(KnownField field) const;
    bool supportsMimeType(KnownField field) const;

    void parse(std::istream &sourceStream, const uint64 maximalSize = 0);
    Id3v2TagMaker prepareMaking();
    void make(std::ostream &targetStream, uint32 padding);

    byte majorVersion() const;
    byte revisionVersion() const;
    void setVersion(byte majorVersion, byte revisionVersion);
    bool isVersionSupported() const;
    byte flags() const;
    bool isUnsynchronisationUsed() const;
    bool hasExtendedHeader() const;
    bool isExperimental() const;
    bool hasFooter() const;
    uint32 extendedHeaderSize() const;
    uint32 paddingSize() const;

private:
    byte m_majorVersion;
    byte m_revisionVersion;
    byte m_flags;
    uint32 m_sizeExcludingHeader;
    uint32 m_extendedHeaderSize;
    uint32 m_paddingSize;
};

/*!
 * \brief Constructs a new tag.
 */
inline Id3v2Tag::Id3v2Tag() :
    m_majorVersion(4),
    m_revisionVersion(0),
    m_flags(0),
    m_sizeExcludingHeader(0),
    m_extendedHeaderSize(0),
    m_paddingSize(0)
{}

inline TagType Id3v2Tag::type() const
{
    return TagType::Id3v2Tag;
}

inline const char *Id3v2Tag::typeName() const
{
    return "ID3v2 tag";
}

inline TagTextEncoding Id3v2Tag::proposedTextEncoding() const
{
    return TagTextEncoding::Utf8;
}

inline bool Id3v2Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == TagTextEncoding::Latin1 || encoding == TagTextEncoding::Utf8 || encoding == TagTextEncoding::Utf16BigEndian;
}

inline bool Id3v2Tag::supportsDescription(KnownField field) const
{
    return field == KnownField::Cover;
}

inline bool Id3v2Tag::supportsMimeType(KnownField field) const
{
    return field == KnownField::Cover;
}

/*!
 * \brief Returns the major version if known; otherwise returns 0.
 */
inline byte Id3v2Tag::majorVersion() const
{
    return m_majorVersion;
}

/*!
 * \brief Returns the revision version if known; otherwise returns 0.
 */
inline byte Id3v2Tag::revisionVersion() const
{
    return m_revisionVersion;
}

/*!
 * \brief Returns an indication whether the version is supported by
 *        the Id3v2Tag class.
 *
 * Major versions 2, 3 and 4 are currently supported.
 */
inline bool Id3v2Tag::isVersionSupported() const
{
    return m_majorVersion == 2 || m_majorVersion == 3 || m_majorVersion == 4;
}

/*!
 * \brief Returns the flags read from the ID3v2 header.
 */
inline byte Id3v2Tag::flags() const
{
    return m_flags;
}

/*!
 * \brief Returns an indication whether unsynchronisation is used.
 */
inline bool Id3v2Tag::isUnsynchronisationUsed() const
{
    return m_flags & 0x80;
}

/*!
 * \brief Returns an indication whether an extended header is used.
 */
inline bool Id3v2Tag::hasExtendedHeader() const
{
    return (m_majorVersion >= 3) && (m_flags & 0x40);
}

/*!
 * \brief Returns an indication whether the tag is labeled as experimental.
 */
inline bool Id3v2Tag::isExperimental() const
{
    return (m_majorVersion >= 3) && (m_flags & 0x20);
}

/*!
 * \brief Returns an indication whether a footer is present.
 */
inline bool Id3v2Tag::hasFooter() const
{
    return (m_majorVersion >= 3) && (m_flags & 0x10);
}

/*!
 * \brief Returns the size of the extended header if one is present; otherwise returns 0.
 */
inline uint32 Id3v2Tag::extendedHeaderSize() const
{
    return m_extendedHeaderSize;
}

/*!
 * \brief Returns the size of the padding between the tag and the first MPEG frame
 *        if one is present; otherwise returns 0.
 */
inline uint32 Id3v2Tag::paddingSize() const
{
    return m_paddingSize;
}

}

#endif // ID3V2TAG_H
