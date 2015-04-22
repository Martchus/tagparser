#ifndef FIELDBASEDTAG_H
#define FIELDBASEDTAG_H

#include "tag.h"

#include <map>
#include <functional>

namespace Media
{

/*!
 * \class Media::FieldMapBasedTag
 * \brief The FieldMapBasedTag provides a generic implementation of Tag which stores
 *        the tag fields using std::multimap.
 *
 * The FieldMapBasedTag class only provides the interface and common functionality.
 * It is meant to be subclassed.
 *
 * \tparam FieldType Specifies the class used to store the fields. Should be a subclass
 *                   of TagField.
 *
 * \tparam Compare Specifies the key comparsion function. Default is std::less.
 */
template <class FieldType, class Compare = std::less<typename FieldType::identifierType> >
class FieldMapBasedTag : public Tag
{
public:
    FieldMapBasedTag();
    virtual ~FieldMapBasedTag();

    virtual const TagValue &value(KnownField value) const;
    virtual const TagValue &value(const typename FieldType::identifierType &id) const;
    virtual std::list<const TagValue *> values(const typename FieldType::identifierType &id) const;
    virtual bool setValue(KnownField field, const TagValue &value);
    virtual bool setValue(const typename FieldType::identifierType &id, const TagValue &value);
    virtual bool hasField(KnownField field) const;
    virtual bool hasField(const typename FieldType::identifierType &id) const;
    virtual void removeAllFields();
    const std::multimap<typename FieldType::identifierType, FieldType, Compare> &fields() const;
    std::multimap<typename FieldType::identifierType, FieldType, Compare> &fields();
    virtual int fieldCount() const;
    virtual typename FieldType::identifierType fieldId(KnownField value) const = 0;
    virtual KnownField knownField(const typename FieldType::identifierType &id) const = 0;
    virtual bool supportsField(KnownField field) const;
    using Tag::proposedDataType;
    virtual TagDataType proposedDataType(const typename FieldType::identifierType &id) const;
    virtual int insertFields(const FieldMapBasedTag<FieldType, Compare> &from, bool overwrite);
    virtual int insertValues(const Tag &from, bool overwrite);
    typedef FieldType fieldType;

private:
    std::multimap<typename FieldType::identifierType, FieldType, Compare> m_fields;
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
template <class FieldType, class Compare>
FieldMapBasedTag<FieldType, Compare>::FieldMapBasedTag()
{}

/*!
 * \brief Destroys the tag.
 */
template <class FieldType, class Compare>
FieldMapBasedTag<FieldType, Compare>::~FieldMapBasedTag()
{}

template <class FieldType, class Compare>
inline const TagValue &FieldMapBasedTag<FieldType, Compare>::value(KnownField value) const
{
    return this->value(fieldId(value));
}

/*!
 * \brief Returns the value of the field with the specified \a id.
 */
template <class FieldType, class Compare>
inline const TagValue &FieldMapBasedTag<FieldType, Compare>::value(const typename FieldType::identifierType &id) const
{
    auto i = m_fields.find(id);
    return i != m_fields.end() ? i->second.value() : TagValue::empty();
}

/*!
 * \brief Returns the values of the field with the specified \a id.
 *
 * There might me more then one value assigned to a \a field. Whereas value()
 * returns only the first value, this method returns all values.
 */
template <class FieldType, class Compare>
inline std::list<const TagValue *> FieldMapBasedTag<FieldType, Compare>::values(const typename FieldType::identifierType &id) const
{
    auto range = m_fields.equal_range(id);
    std::list<const TagValue *> values;
    for(auto i = range.first; i != range.second; ++i) {
        values.push_back(&i->second.value());
    }
    return values;
}

template <class FieldType, class Compare>
inline bool FieldMapBasedTag<FieldType, Compare>::setValue(KnownField field, const TagValue &value)
{
    return setValue(fieldId(field), value);
}

/*!
 * \brief Assigns the given \a value to the field with the specified \a id.
 */
template <class FieldType, class Compare>
bool FieldMapBasedTag<FieldType, Compare>::setValue(const typename FieldType::identifierType &id, const Media::TagValue &value)
{
    auto i = m_fields.find(id);
    if(i != m_fields.end()) { // field already exists -> set its value
        i->second.setValue(value);
    } else if(!value.isEmpty())  {// field doesn't exist -> create new one if value is not null
        m_fields.insert(std::pair<typename FieldType::identifierType, FieldType>(id, FieldType(id, value)));
    } else { // otherwise return false
        return false;
    }
    return true;
}

template <class FieldType, class Compare>
inline bool FieldMapBasedTag<FieldType, Compare>::hasField(KnownField field) const
{
    return hasField(fieldId(field));
}

/*!
 * \brief Returns an indication whether the field with the specified \a id is present.
 */
template <class FieldType, class Compare>
inline bool FieldMapBasedTag<FieldType, Compare>::hasField(const typename FieldType::identifierType &id) const
{
    return m_fields.count(id);
}

template <class FieldType, class Compare>
inline void FieldMapBasedTag<FieldType, Compare>::removeAllFields()
{
    m_fields.clear();
}

/*!
 * \brief Returns the fields of the tag.
 *
 * This method provides direct access to the fields of the tag. It might
 * be usefull when the convenience methods value(), setValue(), hasField(), ...
 * to not offer the required functionality.
 */
template <class FieldType, class Compare>
inline const std::multimap<typename FieldType::identifierType, FieldType, Compare> &FieldMapBasedTag<FieldType, Compare>::fields() const
{
    return m_fields;
}

/*!
 * \brief Returns the fields of the tag.
 *
 * This method provides direct access to the fields of the tag. It might
 * be usefull when the convenience methods value(), setValue(), hasField(), ...
 * to not offer the required functionality.
 */
template <class FieldType, class Compare>
inline std::multimap<typename FieldType::identifierType, FieldType, Compare> &FieldMapBasedTag<FieldType, Compare>::fields()
{
    return m_fields;
}

template <class FieldType, class Compare>
inline int FieldMapBasedTag<FieldType, Compare>::fieldCount() const
{
    return m_fields.size();
}

template <class FieldType, class Compare>
inline bool FieldMapBasedTag<FieldType, Compare>::supportsField(KnownField field) const
{
    static typename FieldType::identifierType def;
    return fieldId(field) != def;
}

/*!
 * \brief Returns the proposed data type for the field with the specified \a id.
 */
template <class FieldType, class Compare>
inline TagDataType FieldMapBasedTag<FieldType, Compare>::proposedDataType(const typename FieldType::identifierType &id) const
{
    return Tag::proposedDataType(knownField(id));
}

/*!
 * \brief Inserts all fields \a from another tag of the same field type and compare function.
 * \param from Specifies the tag the fields should be inserted from.
 * \param overwrite Indicates whether existing fields should be overwritten.
 * \return Returns the number of fields that have been inserted.
 */
template <class FieldType, class Compare>
int FieldMapBasedTag<FieldType, Compare>::insertFields(const FieldMapBasedTag<FieldType, Compare> &from, bool overwrite)
{
    int fieldsInserted = 0;
    for(const auto &pair : from.fields()) {
        const FieldType &fromField = pair.second;
        if(fromField.value().isEmpty())
            continue;
        bool fieldInserted = false;
        auto range = fields().equal_range(fromField.id());
        for(auto i = range.first; i != range.second; ++i) {
            FieldType &ownField = i->second;
            if((fromField.isTypeInfoAssigned() && ownField.isTypeInfoAssigned()
                    && fromField.typeInfo() == ownField.typeInfo())
                    || (!fromField.isTypeInfoAssigned() && ! ownField.isTypeInfoAssigned())) {
                if(overwrite || ownField.value().isEmpty()) {
                    ownField = fromField;
                    ++fieldsInserted;
                }
                fieldInserted = true;
                continue;
            }
        }
        if(!fieldInserted) {
            fields().insert(std::pair<typename FieldType::identifierType, FieldType>(fromField.id(), fromField));
            ++fieldsInserted;
        }
    }
    return fieldsInserted;
}

template <class FieldType, class Compare>
int FieldMapBasedTag<FieldType, Compare>::insertValues(const Tag &from, bool overwrite)
{
    if(type() == from.type()) {
        // the tags are of the same type, we can insert the fields directly
        return insertFields(static_cast<const FieldMapBasedTag<FieldType, Compare> &>(from), overwrite);
    } else {
        return Tag::insertValues(from, overwrite);
    }
}

}

#endif // FIELDBASEDTAG_H
