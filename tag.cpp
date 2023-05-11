#include "./tag.h"

using namespace std;

namespace TagParser {

/// \brief The TagPrivate struct contains private fields of the Tag class.
struct TagPrivate {};

/*!
 * \class TagParser::Tag
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
    return setValue(field, values.size() ? values.front() : TagValue());
}

/*!
 * \brief Inserts all compatible values \a from another Tag.
 * \param from Specifies the Tag the values should be inserted from.
 * \param overwrite Indicates whether existing values should be overwritten.
 * \return Returns the number of values that have been inserted.
 * \remarks The encoding of the inserted text values might not be supported by the tag.
 *          To fix this, call ensureTextValuesAreProperlyEncoded() after insertion.
 */
std::size_t Tag::insertValues(const Tag &from, bool overwrite)
{
    auto count = std::size_t(0);
    for (int i = static_cast<int>(KnownField::Invalid) + 1, last = static_cast<int>(KnownField::Description); i <= last; ++i) {
        auto field = static_cast<KnownField>(i);
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
 * \fn Tag::type()
 * \brief Returns the type of the tag as TagParser::TagType.
 *
 * This is TagType::Unspecified by default and might be overwritten
 * when subclassing.
 */

/*!
 * \fn Tag::typeName()
 * \brief Returns the type name of the tag as C-style string.
 *
 * This is "unspecified" by default and might be overwritten
 * when subclassing.
 */

/*!
 * \fn Tag::version()
 * \brief Returns the version of the tag as std::string.
 * The version denotation depends on the tag type.
 */

/*!
 * \fn Tag::size()
 * \brief Returns the size the tag within the file it is parsed from in bytes.
 * \remarks
 * - Zero is returned if the tag has not been parsed yet. If the corresponding MediaFileInfo
 *   objects tags have already been parsed this shouldn't be the case unless the tag is not
 *   actually present in the file yet, e.g. it has been added via MediaFileInfo::createAppropriateTags()
 *   but has not been applied yet via MediaFileInfo::applyChanges().
 * - Can **not** be used to determine the number of bytes it would require to write the tag
 *   in its current state. For this, use functions like Mp4Tag::prepareMaking() instead.
 */

/*!
 * \fn Tag::targetLevel()
 * \brief Returns the name of the current tag target level.
 * \remarks Returns TagTargetLevel::Unspecified if target levels are not supported by the tag.
 */

/*!
 * \fn Tag::targetLevelName()
 * \brief Returns the name of the current target level.
 * \remarks Returns nullptr if target levels are not supported by the tag.
 */

/*!
 * \fn Tag::isTargetingLevel()
 * \brief Returns whether the tag is targeting the specified \a tagTargetLevel.
 * \remarks If targets are not supported by the tag it is considered targeting
 *          everything and hence this method returns always true in this case.
 */

/*!
 * \fn Tag::targetString()
 * \brief Returns the string representation for the assigned tag target.
 */

/*!
 * \fn Tag::proposedTextEncoding()
 * \brief Returns the proposed text encoding.
 *
 * This is TagTextEncoding::Latin1 by default an might be
 * overwritten when subclassing.
 *
 * The tag class and its subclasses do not perform any conversions.
 * You have to provide all string values using an encoding which is
 * appropriate for the specific tag type. This method returns such
 * an encoding.
 *
 * \sa canEncodingBeUsed()
 */

/*!
 * \fn Tag::canEncodingBeUsed()
 * \brief Returns an indication whether the specified \a encoding
 *        can be used to provide string values for the tag.
 *
 * Only the proposedTextEncoding() is accepted by default. This might
 * be overwritten when subclassing.
 *
 * The tag class and its subclasses do not perform any conversions.
 * You have to provide all string values using an encoding which is
 * appropriate for the specific tag type. This method is meant to
 * determine if a particular \a encoding can be used.
 *
 * \sa canEncodingBeUsed()
 */

/*!
 * \fn Tag::supportsTarget()
 * \brief Returns an indication whether a target is supported by the tag.
 *
 * If no target is supported, setting a target using setTarget()
 * has no effect when saving the tag.
 *
 * Most tag types don't support this feature so the default implementation
 * returns always false. This might be overwritten when subclassing.
 */

/*!
 * \fn Tag::target()
 * \brief Returns the target of tag.
 *
 * \sa supportsTarget()
 * \sa setTarget()
 */

/*!
 * \fn Tag::setTarget()
 * \brief Sets the target of tag.
 *
 * Most tag types don't support this feature so setting
 * the target has no effect when saving the file.
 *
 * \sa supportsTarget()
 * \sa target()
 */

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
 * \fn Tag::proposedDataType()
 * \brief Returns the proposed data type for the specified \a field as TagDataType.
 *
 * Most values need to be provided as string (see proposedTextEncoding() and
 * canEncodingBeUsed()). Other values need to be provided as integer or an other
 * TagDataType. This method helps to determine which type is required for a particular
 * \a field.
 *
 * \remarks
 * - The tag class and its subclasses try to convert the provided values. So using
 *   exactly the proposed type is not neccassary. Nevertheless it can help to detect
 *   conversion errors early. A GUI application could use this method to determine
 *   which widget should be used.
 * - The default implementation returns a data type which is most commonly used for
 *   the specified \a field. The default implementation might be overwritten when
 *   subclassing.
 */

/*!
 * \fn Tag::supportsMimeType()
 * \brief Returns an indications whether the specified field supports mime types.
 * \remarks
 * - If you assign a mime types to a field value and the field does not support
 *   mime types the mime type is ignored when saving the tag.
 * - The default implementation returns false for all fields. This might be overwritten
 *   when subclassing.
 */

/*!
 * \fn Tag::supportsDescription()
 * \brief Returns an indications whether the specified field supports descriptions.
 * \remarks
 * - If you assign a description to a field value and the field does not support
 *   descriptions the description is ignored when saving the tag.
 * - The default implementation returns false for all fields. This might be overwritten
 *   when subclassing.
 */

/*!
 * \fn Tag::supportsMultipleValues()
 * \brief Returns an indications whether the specified field supports multiple values.
 * \remarks
 * - If you assign multiple values to a field which doesn't support multiple values,
 *   the tag implementation might just ignore additional values. It might also try
 *   to preserve the values nevertheless by bending the rules of the tag format
 *   specification when it is safe to do so. (Usually it is safe because additional
 *   values would be simply ignored by other applications.)
 * - So it is not really mandatory to check this before adding multiple values. Nothing
 *   bad will happen otherwise. However, a GUI application could use this method to
 *   determine which widget to use.
 * - In case it is not known whether multiple values are supported, this method returns
 *   false. If you know better, you can try to assign multiple values anyways.
 * - If this method returns true, there might be further constraints, though. Eg. only
 *   one cover of a certain type may be present at a time in an ID3v2 tag.
 * - The default implementation returns false for all fields. This might be overwritten
 *   when subclassing.
 */

/*!
 * \fn Tag::supportsField()
 * \brief Returns an indication whether the specified \a field
 *        is supported by the tag.
 */

/*!
 * \fn Tag::ensureTextValuesAreProperlyEncoded()
 * \brief Ensures the encoding of all assigned text values is supported by the tag by
 *        converting the character set if necessary.
 */

} // namespace TagParser
