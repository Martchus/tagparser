#ifndef TAG_PARSER_MP4TAG_H
#define TAG_PARSER_MP4TAG_H

#include "./mp4tagfield.h"

#include "../fieldbasedtag.h"

namespace TagParser {

class Mp4Atom;
class Mp4Tag;

struct TAG_PARSER_EXPORT Mp4ExtendedFieldId {
    Mp4ExtendedFieldId(std::string_view mean, std::string_view name, bool updateOnly = false);
    Mp4ExtendedFieldId(KnownField field);

    operator bool() const;
    bool matches(const Mp4TagField &field) const;

    /// \brief mean parameter, usually Mp4TagExtendedMeanIds::iTunes
    std::string_view mean;
    /// \brief name parameter
    std::string_view name;
    /// \brief Whether only existing fields should be updated but *no* new extended field should be created
    bool updateOnly = false;
};

/*!
 * \brief Constructs a new instance with the specified parameter.
 */
inline Mp4ExtendedFieldId::Mp4ExtendedFieldId(std::string_view mean, std::string_view name, bool updateOnly)
    : mean(mean)
    , name(name)
    , updateOnly(updateOnly)
{
}

/*!
 * \brief Returns whether valid parameter are assigned.
 */
inline Mp4ExtendedFieldId::operator bool() const
{
    return !mean.empty() && !name.empty();
}

/*!
 * \brief Returns whether the current parameter match the specified \a field.
 */
inline bool Mp4ExtendedFieldId::matches(const Mp4TagField &field) const
{
    return field.mean() == mean && field.name() == name;
}

class TAG_PARSER_EXPORT Mp4TagMaker {
    friend class Mp4Tag;

public:
    void make(std::ostream &stream, Diagnostics &diag);
    const Mp4Tag &tag() const;
    std::uint64_t requiredSize() const;

private:
    Mp4TagMaker(Mp4Tag &tag, Diagnostics &diag);

    Mp4Tag &m_tag;
    std::vector<Mp4TagFieldMaker> m_maker;
    std::uint64_t m_metaSize;
    std::uint64_t m_ilstSize;
    bool m_omitPreDefinedGenre;
};

/*!
 * \brief Returns the associated tag.
 */
inline const Mp4Tag &Mp4TagMaker::tag() const
{
    return m_tag;
}

/*!
 * \brief Returns the number of bytes which will be written when making the tag.
 */
inline std::uint64_t Mp4TagMaker::requiredSize() const
{
    return m_metaSize;
}

/*!
 * \brief Defines traits for the TagField implementation of the Mp4Tag class.
 */
template <> class TAG_PARSER_EXPORT FieldMapBasedTagTraits<Mp4Tag> {
public:
    using FieldType = Mp4TagField;
    using Compare = std::less<typename FieldType::IdentifierType>;
};

class TAG_PARSER_EXPORT Mp4Tag final : public FieldMapBasedTag<Mp4Tag> {
    friend class FieldMapBasedTag<Mp4Tag>;

public:
    Mp4Tag();

    static constexpr TagType tagType = TagType::Mp4Tag;
    static constexpr std::string_view tagName = "MP4/iTunes tag";
    static constexpr TagTextEncoding defaultTextEncoding = TagTextEncoding::Utf8;
    bool canEncodingBeUsed(TagTextEncoding encoding) const override;

    bool supportsField(KnownField field) const override;
    using FieldMapBasedTag<Mp4Tag>::value;
    const TagValue &value(KnownField value) const override;
    using FieldMapBasedTag<Mp4Tag>::values;
    std::vector<const TagValue *> values(KnownField field) const override;
    const TagValue &value(std::string_view mean, std::string_view name) const;
    using FieldMapBasedTag<Mp4Tag>::setValue;
    bool setValue(KnownField field, const TagValue &value) override;
    using FieldMapBasedTag<Mp4Tag>::setValues;
    bool setValues(KnownField field, const std::vector<TagValue> &values) override;
    bool setValue(std::string_view mean, std::string_view name, const TagValue &value);
    using FieldMapBasedTag<Mp4Tag>::hasField;
    bool hasField(KnownField value) const override;
    bool supportsMultipleValues(KnownField) const override;

    void parse(Mp4Atom &metaAtom, Diagnostics &diag);
    Mp4TagMaker prepareMaking(Diagnostics &diag);
    void make(std::ostream &stream, Diagnostics &diag);

protected:
    IdentifierType internallyGetFieldId(KnownField field) const;
    KnownField internallyGetKnownField(const IdentifierType &id) const;
    void internallyGetValuesFromField(const FieldType &field, std::vector<const TagValue *> &values) const;
};

/*!
 * \brief Constructs a new tag.
 */
inline Mp4Tag::Mp4Tag()
{
}

inline bool Mp4Tag::supportsField(KnownField field) const
{
    switch (field) {
    case KnownField::EncoderSettings:
        return true;
    default:
        return FieldMapBasedTag<Mp4Tag>::supportsField(field);
    }
}

/*!
 * \brief Returns false for all fields (for now).
 * \remarks Not sure whether iTunes-style MP4 tags allow this. Let's return false for now.
 * \todo Do some research whether it is supported or not.
 */
inline bool Mp4Tag::supportsMultipleValues(KnownField) const
{
    return false;
}

} // namespace TagParser

#endif // TAG_PARSER_MP4TAG_H
