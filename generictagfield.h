#ifndef TAGFIELD_H
#define TAGFIELD_H

#include "./tagvalue.h"

namespace Media {

template <class implementationType>
class TagField;

/*!
 * \class Media::TagFieldTraits
 * \brief Defines traits for the specified \a ImplementationType.
 *
 * A template specialization for each TagField subclass must be provided.
 */
template<typename ImplementationType>
class TagFieldTraits
{};

/*!
 * \brief The TagField class is used by FieldMapBasedTag to store the fields.
 *
 * A TagField consists of an identifier and a value. An additional type info
 * might be assigned as well. The usage of the type info depends on the
 * particular tag implementation.
 *
 * \remarks This template class is intended to be used
 *          with the "Curiously recurring template pattern".
 */
template <class ImplementationType>
class TAG_PARSER_EXPORT TagField
{
public:
    friend class TagFieldTraits<ImplementationType>;
    typedef typename TagFieldTraits<ImplementationType>::implementationType implementationType;
    typedef typename TagFieldTraits<ImplementationType>::identifierType identifierType;
    typedef typename TagFieldTraits<ImplementationType>::typeInfoType typeInfoType;

    TagField();
    TagField(const identifierType &id, const TagValue &value);
    ~TagField();

    const identifierType &id() const;
    std::string idToString() const;
    void setId(const identifierType &id);
    void clearId();

    TagValue &value();
    const TagValue &value() const;
    void setValue(const TagValue &value);
    void clearValue();

    const typeInfoType &typeInfo() const;
    void setTypeInfo(const typeInfoType &typeInfo);
    void removeTypeInfo();
    bool isTypeInfoAssigned() const;

    bool isDefault() const;
    void setDefault(bool isDefault);

    void clear();

    bool isAdditionalTypeInfoUsed() const;

    const std::vector<ImplementationType> &nestedFields() const;
    std::vector<ImplementationType> &nestedFields();
    bool supportsNestedFields() const;

private:    
    void cleared();

private:
    identifierType m_id;
    TagValue m_value;
    typeInfoType m_typeInfo;
    bool m_typeInfoAssigned;
    bool m_default;
    std::vector<ImplementationType> m_nestedFields;
};

/*!
 * \brief Constructs an empty TagField.
 */
template <class ImplementationType>
TagField<ImplementationType>::TagField() :
    m_id(identifierType()),
    m_value(TagValue()),
    m_typeInfo(typeInfoType()),
    m_typeInfoAssigned(false),
    m_default(false)
{}

/*!
 * \brief Constructs a new TagField with the specified \a id and \a value.
 */
template <class ImplementationType>
TagField<ImplementationType>::TagField(const identifierType &id, const TagValue &value) :
    m_id(id),
    m_value(value),
    m_typeInfo(typeInfoType()),
    m_typeInfoAssigned(false),
    m_default(false)
{}

/*!
 * \brief Destroys the TagField.
 */
template <class ImplementationType>
TagField<ImplementationType>::~TagField()
{}

/*!
 * \brief Returns the id of the current TagField.
 */
template <class ImplementationType>
inline const typename TagField<ImplementationType>::identifierType &TagField<ImplementationType>::id() const
{
    return m_id;
}

template<class ImplementationType>
inline std::string TagField<ImplementationType>::idToString() const
{
    return ImplementationType::fieldIdToString(m_id);
}

/*!
 * \brief Sets the id of the current Tag Field.
 */
template <class ImplementationType>
inline void TagField<ImplementationType>::setId(const identifierType &id)
{
    m_id = id;
}

/*!
 * \brief Clears the id of the current TagField.
 */
template <class ImplementationType>
inline void TagField<ImplementationType>::clearId()
{
    m_id = identifierType();
}

/*!
 * \brief Returns the value of the current TagField.
 */
template <class ImplementationType>
inline TagValue &TagField<ImplementationType>::value()
{
    return m_value;
}

/*!
 * \brief Returns the value of the current TagField.
 */
template <class ImplementationType>
inline const TagValue &TagField<ImplementationType>::value() const
{
    return m_value;
}

/*!
 * \brief Sets the value of the current TagField.
 */
template <class ImplementationType>
inline void TagField<ImplementationType>::setValue(const TagValue &value)
{
    m_value = value;
}

/*!
 * \brief Clears the value of the current TagField.
 */
template <class ImplementationType>
inline void TagField<ImplementationType>::clearValue()
{
    m_value.clearDataAndMetadata();
}

/*!
 * \brief Returns the type info of the current TagField.
 */
template <class ImplementationType>
inline const typename TagField<ImplementationType>::typeInfoType &TagField<ImplementationType>::typeInfo() const
{
    return m_typeInfo;
}

/*!
 * \brief Sets the type info of the current TagField.
 */
template <class ImplementationType>
inline void TagField<ImplementationType>::setTypeInfo(const typeInfoType &typeInfo)
{
    m_typeInfo = typeInfo;
    m_typeInfoAssigned = true;
}

/*!
 * \brief Removes the type info from the current TagField.
 */
template <class ImplementationType>
inline void TagField<ImplementationType>::removeTypeInfo()
{
    m_typeInfo = typeInfoType();
    m_typeInfoAssigned = false;
}

/*!
 * \brief Returns an indication whether a type info is assigned.
 */
template <class ImplementationType>
inline bool TagField<ImplementationType>::isTypeInfoAssigned() const
{
    return m_typeInfoAssigned;
}

/*!
 * \brief Returns an indication whether the field is labeled as default.
 */
template <class ImplementationType>
inline bool TagField<ImplementationType>::isDefault() const
{
    return m_default;
}

/*!
 * \brief Sets whether the field is labeled as default.
 */
template <class ImplementationType>
inline void TagField<ImplementationType>::setDefault(bool isDefault)
{
    m_default = isDefault;
}

/*!
 * \brief Clears id, value, type info and sets default flag to false.
 */
template <class ImplementationType>
void TagField<ImplementationType>::clear()
{
    clearId();
    clearValue();
    m_typeInfo = typeInfoType();
    m_typeInfoAssigned = false;
    m_default = true;
    static_cast<ImplementationType *>(this)->cleared();
}

/*!
 * \brief Returns an indication whether the additional type info is used.
 *
 * The default implementation always returns false. The method might be reimplemented
 * when subclassing.
 */
template <class ImplementationType>
inline bool TagField<ImplementationType>::isAdditionalTypeInfoUsed() const
{
    return static_cast<ImplementationType *>(this)->isAdditionalTypeInfoUsed();
}

/*!
 * \brief Returns the nested fields.
 */
template <class ImplementationType>
const std::vector<ImplementationType> &TagField<ImplementationType>::nestedFields() const
{
    return m_nestedFields;
}

/*!
 * \brief Returns the nested fields.
 * \remarks Might be modified. Not all implementations support nested fields.
 * \sa supportsNestedFields()
 */
template <class ImplementationType>
inline std::vector<ImplementationType> &TagField<ImplementationType>::nestedFields()
{
    return m_nestedFields;
}

/*!
 * \brief Returns whether nested fields are supported by the implementation.
 */
template <class ImplementationType>
inline bool TagField<ImplementationType>::supportsNestedFields() const
{
    return static_cast<ImplementationType *>(this)->supportsNestedFields();
}

/*!
 * \brief Called when the field is cleared.
 */
template <class ImplementationType>
inline void TagField<ImplementationType>::cleared()
{}

}

#endif // TAGFIELD_H
