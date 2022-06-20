#ifndef TAG_PARSER_TAG_TYPE_H
#define TAG_PARSER_TAG_TYPE_H

namespace TagParser {

/*!
 * \brief Specifies the tag type.
 *
 * \sa Tag::type()
 */
enum class TagType : unsigned int {
    Unspecified = 0x00, /**< The tag type is unspecified. */
    Id3v1Tag = 0x01, /**< The tag is a TagParser::Id3v1Tag. */
    Id3v2Tag = 0x02, /**< The tag is a TagParser::Id3v2Tag. */
    Mp4Tag = 0x04, /**< The tag is a TagParser::Mp4Tag. */
    MatroskaTag = 0x08, /**< The tag is a TagParser::MatroskaTag. */
    VorbisComment = 0x10, /**< The tag is a TagParser::VorbisComment. */
    OggVorbisComment = 0x20 /**< The tag is a TagParser::OggVorbisComment. */
};

} // namespace TagParser

#endif // TAG_PARSER_TAG_TYPE_H
