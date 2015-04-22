#ifndef MEDIA_MATROSKATAGFIELD_H
#define MEDIA_MATROSKATAGFIELD_H

#include "../generictagfield.h"
#include "../statusprovider.h"

namespace Media {

class EbmlElement;
class MatroskaTagField;

/*!
 * \brief Defines traits for the TagField implementation of the MatroskaTagField class.
 */
template <>
class LIB_EXPORT TagFieldTraits<MatroskaTagField>
{
public:
    /*!
     * \brief Fields in a Matroska tag are identified by strings.
     */
    typedef std::string identifierType;

    /*!
     * \brief The type info is stored using strings.
     */
    typedef std::string typeInfoType;

    /*!
     * \brief The implementation type is EbmlElement.
     */
    typedef MatroskaTagField implementationType;

    static bool supportsNestedFields();
};

/*!
 * \brief The MatroskaTagField supports nested fields.
 */
inline bool TagFieldTraits<MatroskaTagField>::supportsNestedFields()
{
    return true;
}

class LIB_EXPORT MatroskaTagFieldMaker
{
    friend class MatroskaTagField;

public:
    void make(std::ostream &stream) const;
    const MatroskaTagField &field() const;
    uint64 requiredSize() const;

private:
    MatroskaTagFieldMaker(MatroskaTagField &field);

    const MatroskaTagField &m_field;
    std::string m_stringValue;
    uint64 m_simpleTagSize;
    uint64 m_totalSize;
    std::vector<MatroskaTagFieldMaker> m_nestedMaker;
};

/*!
 * \brief Returns the associated field.
 */
inline const MatroskaTagField &MatroskaTagFieldMaker::field() const
{
    return m_field;
}

/*!
 * \brief Returns number of bytes which will be written when making the field.
 */
inline uint64 MatroskaTagFieldMaker::requiredSize() const
{
    return m_totalSize;
}



class LIB_EXPORT MatroskaTagField : public TagField<MatroskaTagField>, public StatusProvider
{
    friend class TagField<MatroskaTagField>;

public:
    MatroskaTagField();
    MatroskaTagField(const std::string &id, const TagValue &value);

    void reparse(EbmlElement &simpleTagElement, bool parseNestedFields = true);
    MatroskaTagFieldMaker prepareMaking();
    void make(std::ostream &stream);
    bool isAdditionalTypeInfoUsed() const;
    bool supportsNestedFields() const;

protected:
    void cleared();
};

/*!
 * \brief Returns whether the additional type info is used.
 */
inline bool MatroskaTagField::isAdditionalTypeInfoUsed() const
{
    return false;
}

/*!
 * \brief Returns whether nested fields are supported.
 */
inline bool MatroskaTagField::supportsNestedFields() const
{
    return true;
}

/*!
 * \brief Ensures the field is cleared.
 */
inline void MatroskaTagField::cleared()
{}

}

#endif // MEDIA_MATROSKATAGFIELD_H
