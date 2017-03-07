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
class TAG_PARSER_EXPORT TagFieldTraits<MatroskaTagField>
{
public:
    typedef std::string IdentifierType;
    typedef std::string TypeInfoType;
    static bool supportsNestedFields();
};

/*!
 * \brief The MatroskaTagField supports nested fields.
 */
inline bool TagFieldTraits<MatroskaTagField>::supportsNestedFields()
{
    return true;
}

class TAG_PARSER_EXPORT MatroskaTagFieldMaker
{
    friend class MatroskaTagField;

public:
    void make(std::ostream &stream) const;
    const MatroskaTagField &field() const;
    uint64 requiredSize() const;

private:
    MatroskaTagFieldMaker(MatroskaTagField &field);

    MatroskaTagField &m_field;
    bool m_isBinary;
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



class TAG_PARSER_EXPORT MatroskaTagField : public TagField<MatroskaTagField>, public StatusProvider
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

    static typename std::string fieldIdFromString(const char *idString, std::size_t idStringSize = std::string::npos);
    static std::string fieldIdToString(const std::string &id);

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
 * \brief Converts the specified ID string representation to an actual ID.
 * \remarks As Matroska field IDs are text strings the string is just passed.
 */
inline std::string MatroskaTagField::fieldIdFromString(const char *idString, std::size_t idStringSize)
{
    return idStringSize != std::string::npos ? std::string(idString, idStringSize) : std::string(idString);
}

/*!
 * \brief Returns the string representation for the specified \a id.
 * \remarks As Matroska field IDs are text strings the string is just passed.
 */
inline std::string MatroskaTagField::fieldIdToString(const std::string &id)
{
    return id;
}

/*!
 * \brief Ensures the field is cleared.
 */
inline void MatroskaTagField::cleared()
{}

}

#endif // MEDIA_MATROSKATAGFIELD_H
