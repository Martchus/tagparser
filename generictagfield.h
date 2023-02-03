#ifndef TAG_PARSER_TAGFIELD_H
#define TAG_PARSER_TAGFIELD_H

#include "./tagvalue.h"

namespace TagParser {

template <class implementationType> class TagField;

/*!
 * \class TagParser::TagFieldTraits
 * \brief Defines traits for the specified \a ImplementationType.
 *
 * A template specialization for each TagField subclass must be provided.
 */
template <typename ImplementationType> class TagFieldTraits {};

/*!
 * \brief The TagField class is used by FieldMapBasedTag to store the fields.
 *
 * A TagField consists of an identifier and a value. An additional type info
 * might be assigned as well. The usage of the type info depends on the
 * particular tag implementation.
 *
 * \tparam ImplementationType Specifies the type of the actual implementation.
 * \remarks This template class is intended to be subclassed using
 *          with the "Curiously recurring template pattern".
 */
template <class ImplementationType> class TAG_PARSER_EXPORT TagField {
    friend class TagFieldTraits<ImplementationType>;

public:
    using IdentifierType = typename TagFieldTraits<ImplementationType>::IdentifierType;
    using TypeInfoType = typename TagFieldTraits<ImplementationType>::TypeInfoType;

    TagField();
    TagField(const IdentifierType &id, const TagValue &value);
    ~TagField();

    IdentifierType &id();
    const IdentifierType &id() const;
    std::string idToString() const;
    void setId(const IdentifierType &id);
    void clearId();

    TagValue &value();
    const TagValue &value() const;
    void setValue(const TagValue &value);
    void clearValue();

    const TypeInfoType &typeInfo() const;
    void setTypeInfo(const TypeInfoType &typeInfo);
    void removeTypeInfo();
    bool isTypeInfoAssigned() const;

    bool isDefault() const;
    void setDefault(bool isDefault);

    void clear();

    bool isAdditionalTypeInfoUsed() const;

    const std::vector<ImplementationType> &nestedFields() const;
    std::vector<ImplementationType> &nestedFields();
    bool supportsNestedFields() const;

protected:
    void internallyClearValue();
    void internallyClearFurtherData();

private:
    IdentifierType m_id;
    TagValue m_value;
    TypeInfoType m_typeInfo;
    bool m_typeInfoAssigned;
    bool m_default;
    std::vector<ImplementationType> m_nestedFields;
};

/*!
 * \brief Constructs an empty TagField.
 */
template <class ImplementationType>
TagField<ImplementationType>::TagField()
    : m_id(IdentifierType())
    , m_value(TagValue())
    , m_typeInfo(TypeInfoType())
    , m_typeInfoAssigned(false)
    , m_default(false)
{
}

/*!
 * \brief Constructs a new TagField with the specified \a id and \a value.
 */
template <class ImplementationType>
TagField<ImplementationType>::TagField(const IdentifierType &id, const TagValue &value)
    : m_id(id)
    , m_value(value)
    , m_typeInfo(TypeInfoType())
    , m_typeInfoAssigned(false)
    , m_default(false)
{
}

/*!
 * \brief Destroys the TagField.
 */
template <class ImplementationType> TagField<ImplementationType>::~TagField()
{
}

/*!
 * \brief Returns the id of the current TagField.
 */
template <class ImplementationType> inline typename TagField<ImplementationType>::IdentifierType &TagField<ImplementationType>::id()
{
    return m_id;
}

/*!
 * \brief Returns the id of the current TagField.
 */
template <class ImplementationType> inline const typename TagField<ImplementationType>::IdentifierType &TagField<ImplementationType>::id() const
{
    return m_id;
}

/*!
 * \brief Returns the id of the current TagField as string.
 */
template <class ImplementationType> inline std::string TagField<ImplementationType>::idToString() const
{
    return ImplementationType::fieldIdToString(m_id);
}

/*!
 * \brief Sets the id of the current Tag Field.
 */
template <class ImplementationType> inline void TagField<ImplementationType>::setId(const IdentifierType &id)
{
    m_id = id;
}

/*!
 * \brief Clears the id of the current TagField.
 */
template <class ImplementationType> inline void TagField<ImplementationType>::clearId()
{
    m_id = IdentifierType();
}

/*!
 * \brief Returns the value of the current TagField.
 */
template <class ImplementationType> inline TagValue &TagField<ImplementationType>::value()
{
    return m_value;
}

/*!
 * \brief Returns the value of the current TagField.
 */
template <class ImplementationType> inline const TagValue &TagField<ImplementationType>::value() const
{
    return m_value;
}

/*!
 * \brief Sets the value of the current TagField.
 */
template <class ImplementationType> inline void TagField<ImplementationType>::setValue(const TagValue &value)
{
    m_value = value;
}

/*!
 * \brief Clears the value of the current TagField.
 */
template <class ImplementationType> inline void TagField<ImplementationType>::clearValue()
{
    static_cast<ImplementationType *>(this)->internallyClearValue();
}

/*!
 * \brief Returns the type info of the current TagField.
 */
template <class ImplementationType> inline const typename TagField<ImplementationType>::TypeInfoType &TagField<ImplementationType>::typeInfo() const
{
    return m_typeInfo;
}

/*!
 * \brief Sets the type info of the current TagField.
 */
template <class ImplementationType> inline void TagField<ImplementationType>::setTypeInfo(const TypeInfoType &typeInfo)
{
    m_typeInfo = typeInfo;
    m_typeInfoAssigned = true;
}

/*!
 * \brief Removes the type info from the current TagField.
 */
template <class ImplementationType> inline void TagField<ImplementationType>::removeTypeInfo()
{
    m_typeInfo = TypeInfoType();
    m_typeInfoAssigned = false;
}

/*!
 * \brief Returns an indication whether a type info is assigned.
 */
template <class ImplementationType> inline bool TagField<ImplementationType>::isTypeInfoAssigned() const
{
    return m_typeInfoAssigned;
}

/*!
 * \brief Returns an indication whether the field is labeled as default.
 */
template <class ImplementationType> inline bool TagField<ImplementationType>::isDefault() const
{
    return m_default;
}

/*!
 * \brief Sets whether the field is labeled as default.
 */
template <class ImplementationType> inline void TagField<ImplementationType>::setDefault(bool isDefault)
{
    m_default = isDefault;
}

/*!
 * \brief Clears id, value, type info, sets default flag to false and resets further implementation specific values.
 */
template <class ImplementationType> void TagField<ImplementationType>::clear()
{
    clearId();
    clearValue();
    static_cast<ImplementationType *>(this)->internallyClearFurtherData();
    m_typeInfo = TypeInfoType();
    m_typeInfoAssigned = false;
    m_default = true;
}

/*!
 * \brief Returns an indication whether the additional type info is used.
 *
 * The default implementation always returns false. The method might be reimplemented
 * when subclassing.
 */
template <class ImplementationType> inline bool TagField<ImplementationType>::isAdditionalTypeInfoUsed() const
{
    return static_cast<ImplementationType *>(this)->isAdditionalTypeInfoUsed();
}

/*!
 * \brief Returns the nested fields.
 */
template <class ImplementationType> const std::vector<ImplementationType> &TagField<ImplementationType>::nestedFields() const
{
    return m_nestedFields;
}

/*!
 * \brief Returns the nested fields.
 * \remarks Might be modified. Not all implementations support nested fields.
 * \sa supportsNestedFields()
 */
template <class ImplementationType> inline std::vector<ImplementationType> &TagField<ImplementationType>::nestedFields()
{
    return m_nestedFields;
}

/*!
 * \brief Returns whether nested fields are supported by the implementation.
 */
template <class ImplementationType> inline bool TagField<ImplementationType>::supportsNestedFields() const
{
    return static_cast<ImplementationType *>(this)->supportsNestedFields();
}

/*!
 * \brief Clears the assigned value; called via clearValue() and clear().
 * \remarks Shadow when subclassing to customize clearning a value.
 */
template <class ImplementationType> void TagField<ImplementationType>::internallyClearValue()
{
    m_value.clearDataAndMetadata();
}

/*!
 * \brief Clears further data; called via clear().
 * \remarks Shadow when subclassing to clear further data the subclass has.
 */
template <class ImplementationType> void TagField<ImplementationType>::internallyClearFurtherData()
{
}

} // namespace TagParser

#endif // TAG_PARSER_TAGFIELD_H
