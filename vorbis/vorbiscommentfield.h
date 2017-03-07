#ifndef MEDIA_VORBISCOMMENTFIELD_H
#define MEDIA_VORBISCOMMENTFIELD_H

#include "../generictagfield.h"
#include "../statusprovider.h"

namespace IoUtilities {
class BinaryReader;
class BinaryWriter;
}

namespace Media {

/*!
 * \brief The VorbisCommentFlags enum specifies flags which controls parsing and making of Vorbis comments.
 */
enum class VorbisCommentFlags : byte
{
    None = 0x0, /**< Regular parsing/making. */
    NoSignature = 0x1, /**< Skips the signature when parsing and making. */
    NoFramingByte = 0x2, /**< Doesn't expect the framing bit to be present when parsing; does not make the framing bit when making. */
    NoCovers = 0x4 /**< Skips all covers when making. */
};

inline bool operator &(VorbisCommentFlags lhs, VorbisCommentFlags rhs)
{
    return static_cast<byte>(lhs) & static_cast<byte>(rhs);
}

inline VorbisCommentFlags operator |(VorbisCommentFlags lhs, VorbisCommentFlags rhs)
{
    return static_cast<VorbisCommentFlags>(static_cast<byte>(lhs) | static_cast<byte>(rhs));
}

class VorbisCommentField;

/*!
 * \brief Defines traits for the TagField implementation of the VorbisCommentField class.
 */
template <>
class TAG_PARSER_EXPORT TagFieldTraits<VorbisCommentField>
{
public:
    typedef std::string IdentifierType;
    typedef uint32 TypeInfoType;
};

class OggIterator;

class TAG_PARSER_EXPORT VorbisCommentField : public TagField<VorbisCommentField>, public StatusProvider
{
    friend class TagField<VorbisCommentField>;

public:
    VorbisCommentField();
    VorbisCommentField(const IdentifierType &id, const TagValue &value);

    void parse(OggIterator &iterator);
    void parse(OggIterator &iterator, uint64 &maxSize);
    void parse(std::istream &stream, uint64 &maxSize);
    bool make(IoUtilities::BinaryWriter &writer, VorbisCommentFlags flags = VorbisCommentFlags::None);
    bool isAdditionalTypeInfoUsed() const;
    bool supportsNestedFields() const;

    static typename std::string fieldIdFromString(const char *idString, std::size_t idStringSize = std::string::npos);
    static std::string fieldIdToString(const std::string &id);

protected:
    void cleared();

private:
    template<class StreamType>
    void internalParse(StreamType &stream, uint64 &maxSize);
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
 * \brief Converts the specified ID string representation to an actual ID.
 * \remarks As Vorbis field IDs are plain text the string is just passed.
 */
inline std::string VorbisCommentField::fieldIdFromString(const char *idString, std::size_t idStringSize)
{
    return idStringSize != std::string::npos ? std::string(idString, idStringSize) : std::string(idString);
}

/*!
 * \brief Returns the string representation for the specified \a id.
 * \remarks As Vorbis field IDs are plain text the string is just passed.
 */
inline std::string VorbisCommentField::fieldIdToString(const std::string &id)
{
    return id;
}

/*!
 * \brief Ensures the field is cleared.
 */
inline void VorbisCommentField::cleared()
{}

}

#endif // MEDIA_VORBISCOMMENTFIELD_H
