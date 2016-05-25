#include "./tag.h"

using namespace std;

namespace Media {

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
Tag::Tag() :
    m_size(0)
{}

/*!
 * \brief Destroys the Tag.
 */
Tag::~Tag()
{}

/*!
 * \brief Returns a string representation of the tag.
 */
string Tag::toString() const
{
    stringstream ss;
    ss << typeName();
    if(supportsTarget() && !target().isEmpty()) {
        ss << " targeting " << targetString();
    }
    return ss.str();
}

/*!
 * \fn Tag::value()
 * \brief Returns the value of the specified \a field.
 *
 * If no value for the specified \a field is assigned an
 * empty TagValue will be returned.
 *
 * \sa setValue()
 * \sa hasField()
 */

/*!
 * \fn Tag::setValue()
 * \brief Assigns the given \a value to the specified \a field.
 *
 * If an empty \a value is given, the field will be be removed.
 *
 * \sa value()
 * \sa hasField()
 */

/*!
 * \fn Tag::hasField()
 * \brief Returns an indication whether the specified \a field is present.
 *
 * \sa value()
 * \sa setValue()
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
 */
unsigned int Tag::insertValues(const Tag &from, bool overwrite)
{
    unsigned int count = 0;
    for(int i = static_cast<int>(KnownField::Invalid) + 1, last = static_cast<int>(KnownField::Description);
        i <= last; ++i) {
        KnownField field = static_cast<KnownField>(i);
        const TagValue &ownValue = value(field);
        if(overwrite || ownValue.isEmpty()) {
            const TagValue &otherValue = from.value(field);
            if(!otherValue.isEmpty() && setValue(field, otherValue)) {
                ++count;
            }
        }
    }
    return count;
}

//bool Tag::setParent(Tag *tag)
//{
//    if(m_parent != tag) {
//        // ensure this tag is a valid parent for the specified tag
//        if(!tag->supportsChild(this)) {
//            return false;
//        }
//        // ensure the new parent is no child of this tag
//        Tag *newParent = tag->parent();
//        while(newParent) {
//            if(newParent == this) {
//                return false;
//            }
//            newParent = newParent->parent();
//        }
//        // remove this tag from the nested tags of the old parent
//        if(m_parent) {
//            m_parent->m_nestedTags.erase(std::remove(m_nestedTags.begin(), m_nestedTags.end(), this));
//        }
//        // add this tag to the nested tags of the new parent
//        if((m_parent = tag)) {
//            m_parent->m_nestedTags.push_back(this);
//        }
//    }
//    return true;
//}

}
