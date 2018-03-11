#include "./tag.h"

using namespace std;

namespace TagParser {

/*!
 * \class Media::Tag
 * \brief The Tag class is used to store, read and write tag information.
 *
 * The Tag class only provides the interface and common functionality. It
 * is meant to be subclassed.This interface does not include parse/make
 * methods to read/write a tag from/to a stream because the availability
 * and signature of these methods may differ between the individual
 * implementations (eg. an Mp4Tag is read from the "meta" Mp4Atom).
 */

/*!
 * \brief Constructs a new Tag.
 */
Tag::Tag()
    : m_size(0)
{
}

/*!
 * \brief Destroys the Tag.
 */
Tag::~Tag()
{
}

/*!
 * \brief Returns a string representation of the tag.
 */
string Tag::toString() const
{
    string res;
    res += typeName();
    if (supportsTarget()) {
        res += " targeting ";
        res += targetString();
    }
    return res;
}

/*!
 * \brief Returns the values of the specified \a field.
 * \remarks
 * - There might me more than one value assigned to a \a field. Whereas value()
 *   returns only the first value, this method returns all values.
 * - However, the default implementation just returns the first value assuming
 *   multiple values per field are not supported by the tag.
 */
std::vector<const TagValue *> Tag::values(KnownField field) const
{
    std::vector<const TagValue *> values;
    const TagValue &v = value(field);
    if (!v.isEmpty()) {
        values.push_back(&v);
    }
    return values;
}

/*!
 * \brief Assigns the given \a values to the specified \a field.
 * \remarks
 * - There might me more than one value assigned to a \a field. Whereas setValue() only alters the first value, this
 *   method will replace all currently assigned values with the specified \a values.
 * - However, the default implementation just sets the first value and discards additional values assuming
 *   multiple values per field are not supported by the tag.
 */
bool Tag::setValues(KnownField field, const std::vector<TagValue> &values)
{
    return setValue(field, values.size() ? *values.begin() : TagValue());
}

/*!
 * \fn Tag::value()
 * \brief Returns the value of the specified \a field.
 * \remarks
 * - If the specified \a field is not present an empty TagValue will be returned.
 * - Some tags support more than just one value per field. If there are multiple values
 *   this method just returns the first one.
 * \sa setValue(), hasField()
 */

/*!
 * \fn Tag::setValue()
 * \brief Assigns the given \a value to the specified \a field.
 * \remarks
 * - If an empty \a value is given, the field will be be removed.
 * - Some tags support more than just one value per field. This method will only
 *   alter the first value.
 * \sa value(),  hasField()
 */

/*!
 * \fn Tag::hasField()
 * \brief Returns an indication whether the specified \a field is present.
 *
 * \sa value(), setValue()
 */

/*!
 * \fn Tag::removeAllFields()
 * \brief Removes all fields from the tag.
 */

/*!
 * \fn Tag::fieldCount()
 * \brief Returns the number of present fields.
 */

/*!
 * \fn Tag::supportsField()
 * \brief Returns an indication whether the specified \a field
 *        is supported by the tag.
 */

/*!
 * \brief Inserts all compatible values \a from another Tag.
 * \param from Specifies the Tag the values should be inserted from.
 * \param overwrite Indicates whether existing values should be overwritten.
 * \return Returns the number of values that have been inserted.
 * \remarks The encoding of the inserted text values might not be supported by the tag.
 *          To fix this, call ensureTextValuesAreProperlyEncoded() after insertion.
 */
unsigned int Tag::insertValues(const Tag &from, bool overwrite)
{
    unsigned int count = 0;
    for (int i = static_cast<int>(KnownField::Invalid) + 1, last = static_cast<int>(KnownField::Description); i <= last; ++i) {
        KnownField field = static_cast<KnownField>(i);
        const TagValue &ownValue = value(field);
        if (overwrite || ownValue.isEmpty()) {
            const TagValue &otherValue = from.value(field);
            if (!otherValue.isEmpty() && setValue(field, otherValue)) {
                ++count;
            }
        }
    }
    return count;
}

/*!
 * \fn Tag::ensureTextValuesAreProperlyEncoded()
 * \brief Ensures the encoding of all assigned text values is supported by the tag by
 *        converting the character set if neccessary.
 */

} // namespace TagParser
