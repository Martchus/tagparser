#ifndef MEDIA_VORBISCOMMENTFIELD_H
#define MEDIA_VORBISCOMMENTFIELD_H

#include "../generictagfield.h"
#include "../statusprovider.h"

namespace IoUtilities {
class BinaryReader;
class BinaryWriter;
}

namespace Media {

class VorbisCommentField;

/*!
 * \brief Defines traits for the TagField implementation of the VorbisCommentField class.
 */
template <>
class LIB_EXPORT TagFieldTraits<VorbisCommentField>
{
public:
    /*!
     * \brief Fields in a Vorbis comment are identified by 32-bit unsigned integers.
     */
    typedef std::string identifierType;
    /*!
     * \brief The type info is stored using 32-bit unsigned integers.
     */
    typedef byte typeInfoType;
    /*!
     * \brief The implementation type is VorbisCommentField.
     */
    typedef VorbisCommentField implementationType;
};

class OggIterator;

class LIB_EXPORT VorbisCommentField : public TagField<VorbisCommentField>, public StatusProvider
{
    friend class TagField<VorbisCommentField>;

public:
    VorbisCommentField();
    VorbisCommentField(const identifierType &id, const TagValue &value);

    void parse(OggIterator &iterator);
    void make(IoUtilities::BinaryWriter &writer);
    bool isAdditionalTypeInfoUsed() const;
    bool supportsNestedFields() const;

protected:
    void cleared();
};

/*!
 * \brief Returns whether the additional type info is used.
 */
inline bool VorbisCommentField::isAdditionalTypeInfoUsed() const
{
    return false;
}

/*!
 * \brief Returns whether nested fields are supported.
 */
inline bool VorbisCommentField::supportsNestedFields() const
{
    return false;
}

/*!
 * \brief Ensures the field is cleared.
 */
inline void VorbisCommentField::cleared()
{}

}

#endif // MEDIA_VORBISCOMMENTFIELD_H
