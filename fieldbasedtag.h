#ifndef TAG_PARSER_FIELDBASEDTAG_H
#define TAG_PARSER_FIELDBASEDTAG_H

#include "./tag.h"

#include <functional>
#include <map>

namespace TagParser {

/*!
 * \class TagParser::FieldMapBasedTagTraits
 * \brief Defines traits for the specified \a ImplementationType.
 *
 * A template specialization for each FieldMapBasedTag subclass must be provided.
 */
template <typename ImplementationType> class FieldMapBasedTagTraits {};

/*!
 * \class TagParser::FieldMapBasedTag
 * \brief The FieldMapBasedTag provides a generic implementation of Tag which stores
 *        the tag fields using std::multimap.
 *
 * The FieldMapBasedTag class only provides the interface and common functionality.
 * It is meant to be subclassed using CRTP pattern.
 *
 * \remarks This template class is intended to be subclassed using
 *          with the "Curiously recurring template pattern".
 */
template <class ImplementationType> class FieldMapBasedTag : public Tag {
    friend class FieldMapBasedTagTraits<ImplementationType>;

public:
    using FieldType = typename FieldMapBasedTagTraits<ImplementationType>::FieldType;
    using IdentifierType = typename FieldMapBasedTagTraits<ImplementationType>::FieldType::IdentifierType;
    using Compare = typename FieldMapBasedTagTraits<ImplementationType>::Compare;

    FieldMapBasedTag();

    TagType type() const;
    std::string_view typeName() const;
    TagTextEncoding proposedTextEncoding() const;
    const TagValue &value(const IdentifierType &id) const;
    const TagValue &value(KnownField field) const;
    std::vector<const TagValue *> values(const IdentifierType &id) const;
    std::vector<const TagValue *> values(KnownField field) const;
    bool setValue(const IdentifierType &id, const TagValue &value);
    bool setValue(KnownField field, const TagValue &value);
    bool setValues(const IdentifierType &id, const std::vector<TagValue> &values);
    bool setValues(KnownField field, const std::vector<TagValue> &values);
    bool hasField(KnownField field) const;
    bool hasField(const IdentifierType &id) const;
    void removeAllFields();
    const std::multimap<IdentifierType, FieldType, Compare> &fields() const;
    std::multimap<IdentifierType, FieldType, Compare> &fields();
    std::size_t fieldCount() const;
    IdentifierType fieldId(KnownField value) const;
    KnownField knownField(const IdentifierType &id) const;
    bool supportsField(KnownField field) const;
    using Tag::proposedDataType;
    TagDataType proposedDataType(const IdentifierType &id) const;
    std::size_t insertFields(const FieldMapBasedTag<ImplementationType> &from, bool overwrite);
    std::size_t insertValues(const Tag &from, bool overwrite);
    void ensureTextValuesAreProperlyEncoded();

protected:
    using CRTPBase = FieldMapBasedTag<ImplementationType>;

    const TagValue &internallyGetValue(const IdentifierType &id) const;
    void internallyGetValuesFromField(const FieldType &field, std::vector<const TagValue *> &values) const;
    std::vector<const TagValue *> internallyGetValues(const IdentifierType &id) const;
    bool internallySetValue(const IdentifierType &id, const TagValue &value);
    bool internallySetValues(const IdentifierType &id, const std::vector<TagValue> &values);
    bool internallyHasField(const IdentifierType &id) const;
    // no default implementation: IdentifierType internallyGetFieldId(KnownField field) const;
    // no default implementation: KnownField internallyGetKnownField(const IdentifierType &id) const;
    TagDataType internallyGetProposedDataType(const IdentifierType &id) const;

private:
    std::multimap<IdentifierType, FieldType, Compare> m_fields;
};

/*!
 * \fn FieldMapBasedTag::fieldId()
 * \brief Returns the ID for the specified \a field.
 *
 * Needs to be implemented when subclassing.
 */

/*!
 * \fn FieldMapBasedTag::knownField()
 * \brief Returns the field for the specified \a ID.
 *
 * Needs to be implemented when subclassing.
 */

/*!
 * \brief Constructs a new FieldMapBasedTag.
 */
template <class ImplementationType> FieldMapBasedTag<ImplementationType>::FieldMapBasedTag()
{
}

template <class ImplementationType> TagType FieldMapBasedTag<ImplementationType>::type() const
{
    return ImplementationType::tagType;
}

template <class ImplementationType> std::string_view FieldMapBasedTag<ImplementationType>::typeName() const
{
    return ImplementationType::tagName;
}

template <class ImplementationType> TagTextEncoding FieldMapBasedTag<ImplementationType>::proposedTextEncoding() const
{
    return ImplementationType::defaultTextEncoding;
}

/*!
 * \brief Default implementation for value().
 * \remarks Shadow in subclass to provide custom implementation.
 */
template <class ImplementationType> const TagValue &FieldMapBasedTag<ImplementationType>::internallyGetValue(const IdentifierType &id) const
{
    auto i = m_fields.find(id);
    return i != m_fields.end() ? i->second.value() : TagValue::empty();
}

/*!
 * \brief Default way to gather values from a field in internallyGetValues().
 * \remarks Shadow in subclass to provide custom implementation.
 */
template <class ImplementationType>
void FieldMapBasedTag<ImplementationType>::internallyGetValuesFromField(
    const FieldMapBasedTag<ImplementationType>::FieldType &field, std::vector<const TagValue *> &values) const
{
    if (!field.value().isEmpty()) {
        values.emplace_back(&field.value());
    }
}

/*!
 * \brief Default implementation for values().
 * \remarks Shadow in subclass to provide custom implementation.
 */
template <class ImplementationType>
std::vector<const TagValue *> FieldMapBasedTag<ImplementationType>::internallyGetValues(const IdentifierType &id) const
{
    auto range = m_fields.equal_range(id);
    std::vector<const TagValue *> values;
    for (auto i = range.first; i != range.second; ++i) {
        static_cast<const ImplementationType *>(this)->internallyGetValuesFromField(i->second, values);
    }
    return values;
}

/*!
 * \brief Returns the value of the field with the specified \a id.
 * \sa Tag::value()
 */
template <class ImplementationType> inline const TagValue &FieldMapBasedTag<ImplementationType>::value(const IdentifierType &id) const
{
    return static_cast<const ImplementationType *>(this)->internallyGetValue(id);
}

template <class ImplementationType> inline const TagValue &FieldMapBasedTag<ImplementationType>::value(KnownField field) const
{
    return value(fieldId(field));
}

/*!
 * \brief Returns the values of the field with the specified \a id.
 * \sa Tag::values()
 */
template <class ImplementationType> inline std::vector<const TagValue *> FieldMapBasedTag<ImplementationType>::values(const IdentifierType &id) const
{
    return static_cast<const ImplementationType *>(this)->internallyGetValues(id);
}

template <class ImplementationType> inline std::vector<const TagValue *> FieldMapBasedTag<ImplementationType>::values(KnownField field) const
{
    return static_cast<const ImplementationType *>(this)->values(fieldId(field));
}

template <class ImplementationType> inline bool FieldMapBasedTag<ImplementationType>::setValue(KnownField field, const TagValue &value)
{
    const auto id = fieldId(field);
    if constexpr (std::is_arithmetic_v<IdentifierType>) {
        if (!id) {
            return false;
        }
    } else {
        if (id.empty()) {
            return false;
        }
    }
    return setValue(id, value);
}

/*!
 * \brief Default implementation for setValue().
 * \remarks Shadow in subclass to provide custom implementation.
 */
template <class ImplementationType> bool FieldMapBasedTag<ImplementationType>::internallySetValue(const IdentifierType &id, const TagValue &value)
{
    auto i = m_fields.find(id);
    if (i != m_fields.end()) { // field already exists -> set its value
        i->second.setValue(value);
    } else if (!value.isEmpty()) { // field doesn't exist -> create new one if value is not null
        m_fields.insert(std::make_pair(id, FieldType(id, value)));
    } else { // otherwise return false
        return false;
    }
    return true;
}

/*!
 * \brief Default implementation for setValues().
 * \remarks Shadow in subclass to provide custom implementation.
 */
template <class ImplementationType>
bool FieldMapBasedTag<ImplementationType>::internallySetValues(const FieldMapBasedTag::IdentifierType &id, const std::vector<TagValue> &values)
{
    auto valuesIterator = values.cbegin();
    auto range = m_fields.equal_range(id);
    // iterate through all specified and all existing values
    for (; valuesIterator != values.cend() && range.first != range.second; ++valuesIterator) {
        // replace existing value with non-empty specified value
        if (!valuesIterator->isEmpty()) {
            auto &field = range.first->second;
            field.clearValue();
            field.setValue(*valuesIterator);
            ++range.first;
        }
    }
    // add remaining specified values (there are more specified values than existing ones)
    for (; valuesIterator != values.cend(); ++valuesIterator) {
        if (!valuesIterator->isEmpty()) {
            m_fields.insert(std::make_pair(id, FieldType(id, *valuesIterator)));
        }
    }
    // remove remaining existing values (there are more existing values than specified ones)
    for (; range.first != range.second; ++range.first) {
        range.first->second.clearValue();
    }
    return true;
}

/*!
 * \brief Assigns the given \a value to the field with the specified \a id.
 * \sa Tag::setValue()
 */
template <class ImplementationType> bool FieldMapBasedTag<ImplementationType>::setValue(const IdentifierType &id, const TagParser::TagValue &value)
{
    return static_cast<ImplementationType *>(this)->internallySetValue(id, value);
}

/*!
 * \brief Assigns the given \a values to the field with the specified \a id.
 * \remarks There might me more than one value assigned to an \a id. Whereas setValue() only alters the first value, this
 *          method will replace all currently assigned values with the specified \a values.
 * \sa Tag::setValues()
 */
template <class ImplementationType>
bool FieldMapBasedTag<ImplementationType>::setValues(const IdentifierType &id, const std::vector<TagValue> &values)
{
    return static_cast<ImplementationType *>(this)->internallySetValues(id, values);
}

/*!
 * \brief Assigns the given \a values to the field with the specified \a id.
 * \remarks There might me more than one value assigned to a \a field. Whereas setValue() only alters the first value, this
 *          method will replace all currently assigned values with the specified \a values.
 * \sa Tag::setValues()
 */
template <class ImplementationType> bool FieldMapBasedTag<ImplementationType>::setValues(KnownField field, const std::vector<TagValue> &values)
{
    const auto id = fieldId(field);
    if constexpr (std::is_arithmetic_v<IdentifierType>) {
        if (!id) {
            return false;
        }
    } else {
        if (id.empty()) {
            return false;
        }
    }
    return setValues(id, values);
}

template <class ImplementationType> inline bool FieldMapBasedTag<ImplementationType>::hasField(KnownField field) const
{
    return hasField(fieldId(field));
}

/*!
 * \brief Default implementation for hasField().
 * \remarks Shadow in subclass to provide custom implementation.
 */
template <class ImplementationType> bool FieldMapBasedTag<ImplementationType>::internallyHasField(const IdentifierType &id) const
{
    for (auto range = m_fields.equal_range(id); range.first != range.second; ++range.first) {
        if (!range.first->second.value().isEmpty()) {
            return true;
        }
    }
    return false;
}

/*!
 * \brief Returns an indication whether the field with the specified \a id is present.
 */
template <class ImplementationType> inline bool FieldMapBasedTag<ImplementationType>::hasField(const IdentifierType &id) const
{
    return static_cast<const ImplementationType *>(this)->internallyHasField(id);
}

template <class ImplementationType> inline void FieldMapBasedTag<ImplementationType>::removeAllFields()
{
    m_fields.clear();
}

/*!
 * \brief Returns the fields of the tag by providing direct access to the field map of the tag.
 */
template <class ImplementationType>
inline auto FieldMapBasedTag<ImplementationType>::fields() const -> const std::multimap<IdentifierType, FieldType, Compare> &
{
    return m_fields;
}

/*!
 * \brief Returns the fields of the tag by providing direct access to the field map of the tag.
 */
template <class ImplementationType> inline auto FieldMapBasedTag<ImplementationType>::fields() -> std::multimap<IdentifierType, FieldType, Compare> &
{
    return m_fields;
}

template <class ImplementationType> std::size_t FieldMapBasedTag<ImplementationType>::fieldCount() const
{
    auto count = std::size_t(0);
    for (const auto &field : m_fields) {
        if (!field.second.value().isEmpty()) {
            ++count;
        }
    }
    return count;
}

/*!
 * \brief Returns the field ID for the specified \a value.
 * \remarks Must be implemented in internallyGetFieldId() when creating subclass.
 */
template <class ImplementationType>
inline typename FieldMapBasedTag<ImplementationType>::IdentifierType FieldMapBasedTag<ImplementationType>::fieldId(KnownField value) const
{
    return static_cast<const ImplementationType *>(this)->internallyGetFieldId(value);
}

/*!
 * \brief Returns the KnownField for the specified \a id.
 * \remarks Must be implemented in internallyGetKnownField() when creating subclass.
 */
template <class ImplementationType> inline KnownField FieldMapBasedTag<ImplementationType>::knownField(const IdentifierType &id) const
{
    return static_cast<const ImplementationType *>(this)->internallyGetKnownField(id);
}

template <class ImplementationType> inline bool FieldMapBasedTag<ImplementationType>::supportsField(KnownField field) const
{
    static const auto def = IdentifierType();
    return fieldId(field) != def;
}

/*!
 * \brief Default implementation for proposedDataType().
 * \remarks Shadow in subclass to provide custom implementation.
 */
template <class ImplementationType>
inline TagDataType FieldMapBasedTag<ImplementationType>::internallyGetProposedDataType(const IdentifierType &id) const
{
    return Tag::proposedDataType(knownField(id));
}

/*!
 * \brief Returns the proposed data type for the field with the specified \a id.
 */
template <class ImplementationType> inline TagDataType FieldMapBasedTag<ImplementationType>::proposedDataType(const IdentifierType &id) const
{
    return static_cast<ImplementationType *>(this)->determineProposedDataType(id);
}

/*!
 * \brief Inserts all fields \a from another tag of the same field type and compare function.
 * \param from Specifies the tag the fields should be inserted from.
 * \param overwrite Indicates whether existing fields should be overwritten.
 * \return Returns the number of fields that have been inserted.
 */
template <class ImplementationType>
std::size_t FieldMapBasedTag<ImplementationType>::insertFields(const FieldMapBasedTag<ImplementationType> &from, bool overwrite)
{
    auto fieldsInserted = std::size_t(0);
    for (const auto &pair : from.fields()) {
        const FieldType &fromField = pair.second;
        if (fromField.value().isEmpty()) {
            continue;
        }
        bool fieldInserted = false;
        auto range = fields().equal_range(fromField.id());
        for (auto i = range.first; i != range.second; ++i) {
            FieldType &ownField = i->second;
            if ((fromField.isTypeInfoAssigned() && ownField.isTypeInfoAssigned() && fromField.typeInfo() == ownField.typeInfo())
                || (!fromField.isTypeInfoAssigned() && !ownField.isTypeInfoAssigned())) {
                if (overwrite || ownField.value().isEmpty()) {
                    ownField = fromField;
                    ++fieldsInserted;
                }
                fieldInserted = true;
                continue;
            }
        }
        if (!fieldInserted) {
            fields().insert(std::make_pair(fromField.id(), fromField));
            ++fieldsInserted;
        }
    }
    return fieldsInserted;
}

template <class ImplementationType> std::size_t FieldMapBasedTag<ImplementationType>::insertValues(const Tag &from, bool overwrite)
{
    if (type() == from.type()) {
        // the tags are of the same type, we can insert the fields directly
        return insertFields(static_cast<const FieldMapBasedTag<ImplementationType> &>(from), overwrite);
    } else {
        return Tag::insertValues(from, overwrite);
    }
}

template <class ImplementationType> void FieldMapBasedTag<ImplementationType>::ensureTextValuesAreProperlyEncoded()
{
    for (auto &field : fields()) {
        field.second.value().convertDataEncodingForTag(this);
    }
}

} // namespace TagParser

#endif // TAG_PARSER_FIELDBASEDTAG_H
