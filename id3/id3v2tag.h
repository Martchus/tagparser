#ifndef TAG_PARSER_ID3V2TAG_H
#define TAG_PARSER_ID3V2TAG_H

#include "./id3v2frame.h"

#include "../fieldbasedtag.h"

#include <c++utilities/misc/flagenumclass.h>

#include <map>

namespace TagParser {

/*!
 * \brief The Id3v2Flags enum specifies flags which controls parsing and making of ID3v2 tags.
 */
enum class Id3v2HandlingFlags : std::uint64_t {
    None = 0, /**< Regular parsing/making. */
    ConvertRecordDateFields = (1 << 1), /**< whether record date fields should be converted when parsing/making */
    Defaults = ConvertRecordDateFields, /**< set of flags considered good defaults */
};

} // namespace TagParser

CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(TagParser, TagParser::Id3v2HandlingFlags)

namespace TagParser {

class Id3v2Tag;

struct TAG_PARSER_EXPORT FrameComparer {
    bool operator()(std::uint32_t lhs, std::uint32_t rhs) const;
};

class TAG_PARSER_EXPORT Id3v2TagMaker {
    friend class Id3v2Tag;

public:
    void make(std::ostream &stream, std::uint32_t padding, Diagnostics &diag);
    const Id3v2Tag &tag() const;
    std::uint64_t requiredSize() const;

private:
    Id3v2TagMaker(Id3v2Tag &tag, Diagnostics &diag);

    Id3v2Tag &m_tag;
    std::uint32_t m_framesSize;
    std::uint32_t m_requiredSize;
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
inline std::uint64_t Id3v2TagMaker::requiredSize() const
{
    return m_requiredSize;
}

/*!
 * \brief Defines traits for the TagField implementation of the Id3v2Tag class.
 */
template <> class TAG_PARSER_EXPORT FieldMapBasedTagTraits<Id3v2Tag> {
public:
    using FieldType = Id3v2Frame;
    using Compare = FrameComparer;
};

class TAG_PARSER_EXPORT Id3v2Tag final : public FieldMapBasedTag<Id3v2Tag> {
    friend class FieldMapBasedTag<Id3v2Tag>;
    friend class Id3v2TagMaker;

public:
    Id3v2Tag();

    static constexpr TagType tagType = TagType::Id3v2Tag;
    static constexpr std::string_view tagName = "ID3v2 tag";
    static constexpr TagTextEncoding defaultTextEncoding = TagTextEncoding::Utf16LittleEndian;
    TagTextEncoding proposedTextEncoding() const override;
    bool canEncodingBeUsed(TagTextEncoding encoding) const override;
    bool supportsDescription(KnownField field) const override;
    bool supportsMimeType(KnownField field) const override;
    bool supportsMultipleValues(KnownField field) const override;
    void ensureTextValuesAreProperlyEncoded() override;

    void parse(std::istream &sourceStream, const std::uint64_t maximalSize, Diagnostics &diag);
    Id3v2TagMaker prepareMaking(Diagnostics &diag);
    void make(std::ostream &targetStream, std::uint32_t padding, Diagnostics &diag);
    Id3v2HandlingFlags handlingFlags() const;
    void setHandlingFlags(Id3v2HandlingFlags flags);

    std::uint8_t majorVersion() const;
    std::uint8_t revisionVersion() const;
    void setVersion(std::uint8_t majorVersion, std::uint8_t revisionVersion);
    bool isVersionSupported() const;
    std::uint8_t flags() const;
    bool isUnsynchronisationUsed() const;
    bool hasExtendedHeader() const;
    bool isExperimental() const;
    bool hasFooter() const;
    std::uint32_t extendedHeaderSize() const;
    std::uint64_t paddingSize() const;

protected:
    IdentifierType internallyGetFieldId(KnownField field) const;
    KnownField internallyGetKnownField(const IdentifierType &id) const;
    TagDataType internallyGetProposedDataType(const std::uint32_t &id) const;
    void internallyGetValuesFromField(const FieldType &field, std::vector<const TagValue *> &values) const;
    bool internallySetValues(const IdentifierType &id, const std::vector<TagValue> &values);

private:
    void convertOldRecordDateFields(const std::string &diagContext, Diagnostics &diag);
    void removeOldRecordDateRelatedFields();
    void prepareRecordDataForMaking(const std::string &diagContext, Diagnostics &diag);

private:
    std::uint8_t m_majorVersion;
    std::uint8_t m_revisionVersion;
    std::uint8_t m_flags;
    std::uint32_t m_sizeExcludingHeader;
    std::uint32_t m_extendedHeaderSize;
    std::uint64_t m_paddingSize;
    Id3v2HandlingFlags m_handlingFlags;
};

/*!
 * \brief Constructs a new tag.
 */
inline Id3v2Tag::Id3v2Tag()
    : m_majorVersion(4)
    , m_revisionVersion(0)
    , m_flags(0)
    , m_sizeExcludingHeader(0)
    , m_extendedHeaderSize(0)
    , m_paddingSize(0)
    , m_handlingFlags(Id3v2HandlingFlags::Defaults)
{
}

inline TagTextEncoding Id3v2Tag::proposedTextEncoding() const
{
    return m_majorVersion > 3 ? TagTextEncoding::Utf8 : TagTextEncoding::Utf16LittleEndian;
}

inline bool Id3v2Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == TagTextEncoding::Latin1 || (encoding == TagTextEncoding::Utf8 && m_majorVersion > 3)
        || encoding == TagTextEncoding::Utf16BigEndian || encoding == TagTextEncoding::Utf16LittleEndian;
}

inline bool Id3v2Tag::supportsDescription(KnownField field) const
{
    switch (field) {
    case KnownField::Cover:
    case KnownField::Lyrics:
    case KnownField::SynchronizedLyrics:
        return true;
    default:
        return false;
    }
}

inline bool Id3v2Tag::supportsMimeType(KnownField field) const
{
    return field == KnownField::Cover;
}

/*!
 * \brief Returns flags influencing the behavior when parsing/making the ID3v2 tag.
 */
inline Id3v2HandlingFlags Id3v2Tag::handlingFlags() const
{
    return m_handlingFlags;
}

/*!
 * \brief Sets flags influencing the behavior when parsing/making the ID3v2 tag.
 */
inline void Id3v2Tag::setHandlingFlags(Id3v2HandlingFlags flags)
{
    m_handlingFlags = flags;
}

/*!
 * \brief Returns the major version if known; otherwise returns 0.
 */
inline std::uint8_t Id3v2Tag::majorVersion() const
{
    return m_majorVersion;
}

/*!
 * \brief Returns the revision version if known; otherwise returns 0.
 */
inline std::uint8_t Id3v2Tag::revisionVersion() const
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
inline std::uint8_t Id3v2Tag::flags() const
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
inline std::uint32_t Id3v2Tag::extendedHeaderSize() const
{
    return m_extendedHeaderSize;
}

/*!
 * \brief Returns the size of the padding between the tag and the first MPEG frame
 *        if one is present; otherwise returns 0.
 */
inline std::uint64_t Id3v2Tag::paddingSize() const
{
    return m_paddingSize;
}

} // namespace TagParser

#endif // TAG_PARSER_ID3V2TAG_H
