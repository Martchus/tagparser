#ifndef TAG_PARSER_MATROSKATAGFIELD_H
#define TAG_PARSER_MATROSKATAGFIELD_H

#include "../generictagfield.h"

namespace TagParser {

class EbmlElement;
class MatroskaTagField;
class Diagnostics;

/*!
 * \brief Defines traits for the TagField implementation of the MatroskaTagField class.
 */
template <> class TAG_PARSER_EXPORT TagFieldTraits<MatroskaTagField> {
public:
    using IdentifierType = std::string;
    using TypeInfoType = std::string;
    static bool supportsNestedFields();
};

/*!
 * \brief The MatroskaTagField supports nested fields.
 */
inline bool TagFieldTraits<MatroskaTagField>::supportsNestedFields()
{
    return true;
}

class TAG_PARSER_EXPORT MatroskaTagFieldMaker {
    friend class MatroskaTagField;

public:
    void make(std::ostream &stream) const;
    const MatroskaTagField &field() const;
    std::uint64_t requiredSize() const;

private:
    MatroskaTagFieldMaker(MatroskaTagField &field, Diagnostics &diag);

    MatroskaTagField &m_field;
    std::string m_stringValue;
    const std::string &m_language;
    const std::string &m_languageIETF;
    std::uint64_t m_simpleTagSize;
    std::uint64_t m_totalSize;
    std::vector<MatroskaTagFieldMaker> m_nestedMaker;
    bool m_isBinary;
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
inline std::uint64_t MatroskaTagFieldMaker::requiredSize() const
{
    return m_totalSize;
}

class TAG_PARSER_EXPORT MatroskaTagField : public TagField<MatroskaTagField> {
    friend class TagField<MatroskaTagField>;

public:
    MatroskaTagField();
    MatroskaTagField(const std::string &id, const TagValue &value);

    void reparse(EbmlElement &simpleTagElement, Diagnostics &diag, bool parseNestedFields = true);
    MatroskaTagFieldMaker prepareMaking(Diagnostics &diag);
    void make(std::ostream &stream, Diagnostics &diag);
    bool isAdditionalTypeInfoUsed() const;
    bool supportsNestedFields() const;

    static typename std::string fieldIdFromString(std::string_view idString);
    static std::string fieldIdToString(const std::string &id);
    static void normalizeId(std::string &id);
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
inline std::string MatroskaTagField::fieldIdFromString(std::string_view idString)
{
    return std::string(idString);
}

/*!
 * \brief Returns the string representation for the specified \a id.
 * \remarks As Matroska field IDs are text strings the string is just passed.
 */
inline std::string MatroskaTagField::fieldIdToString(const std::string &id)
{
    return id;
}

} // namespace TagParser

#endif // TAG_PARSER_MATROSKATAGFIELD_H
