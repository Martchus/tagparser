#ifndef TAG_PARSER_VORBISCOMMENTIDS_H
#define TAG_PARSER_VORBISCOMMENTIDS_H

#include "../global.h"

namespace TagParser {

/*!
 * \brief Encapsulates Vorbis comment field names.
 * \sa See https://xiph.org/vorbis/doc/v-comment.html for the upstream documentation of the field names.
 */
namespace VorbisCommentIds {

constexpr TAG_PARSER_EXPORT const char *trackNumber()
{
    return "TRACKNUMBER";
}
constexpr TAG_PARSER_EXPORT const char *diskNumber()
{
    return "DISCNUMBER";
}
constexpr TAG_PARSER_EXPORT const char *part()
{
    return "PART";
}
constexpr TAG_PARSER_EXPORT const char *partNumber()
{
    return "PARTNUMBER";
}
constexpr TAG_PARSER_EXPORT const char *title()
{
    return "TITLE";
}
constexpr TAG_PARSER_EXPORT const char *version()
{
    return "VERSION";
}
constexpr TAG_PARSER_EXPORT const char *artist()
{
    return "ARTIST";
}
constexpr TAG_PARSER_EXPORT const char *albumArtist()
{
    return "ALBUMARTIST";
}
constexpr TAG_PARSER_EXPORT const char *grouping()
{
    return "GROUPING";
}
constexpr TAG_PARSER_EXPORT const char *album()
{
    return "ALBUM";
}
constexpr TAG_PARSER_EXPORT const char *label()
{
    return "LABEL";
}
constexpr TAG_PARSER_EXPORT const char *labelNo()
{
    return "LABELNO";
}
constexpr TAG_PARSER_EXPORT const char *language()
{
    return "LANGUAGE";
}
constexpr TAG_PARSER_EXPORT const char *performer()
{
    return "PERFORMER";
}
constexpr TAG_PARSER_EXPORT const char *composer()
{
    return "COMPOSER";
}
constexpr TAG_PARSER_EXPORT const char *ensemble()
{
    return "ENSEMBLE";
}
constexpr TAG_PARSER_EXPORT const char *arranger()
{
    return "ARRANGER";
}
constexpr TAG_PARSER_EXPORT const char *lyricist()
{
    return "LYRICIST";
}
constexpr TAG_PARSER_EXPORT const char *lyrics()
{
    return "LYRICS";
}
constexpr TAG_PARSER_EXPORT const char *author()
{
    return "AUTHOR";
}
constexpr TAG_PARSER_EXPORT const char *conductor()
{
    return "CONDUCTOR";
}
constexpr TAG_PARSER_EXPORT const char *encoder()
{
    return "ENCODER";
}
constexpr TAG_PARSER_EXPORT const char *publisher()
{
    return "PUBLISHER";
}
constexpr TAG_PARSER_EXPORT const char *genre()
{
    return "GENRE";
}
constexpr TAG_PARSER_EXPORT const char *originalMediaType()
{
    return "ORIGINAL_TAG_PARSER_TYPE";
}
constexpr TAG_PARSER_EXPORT const char *contentType()
{
    return "CONTENT_TYPE";
}
constexpr TAG_PARSER_EXPORT const char *subject()
{
    return "SUBJECT";
}
constexpr TAG_PARSER_EXPORT const char *description()
{
    return "DESCRIPTION";
}
constexpr TAG_PARSER_EXPORT const char *isrc()
{
    return "ISRC";
}
constexpr TAG_PARSER_EXPORT const char *eanupn()
{
    return "EAN/UPN";
}
constexpr TAG_PARSER_EXPORT const char *comment()
{
    return "COMMENT";
}
constexpr TAG_PARSER_EXPORT const char *encoderSettings()
{
    return "ENCODING";
}
constexpr TAG_PARSER_EXPORT const char *date()
{
    return "DATE";
}
constexpr TAG_PARSER_EXPORT const char *year()
{
    return "YEAR"; // not mentioned in https://xiph.org/vorbis/doc/v-comment.html but seen in the wild
}
constexpr TAG_PARSER_EXPORT const char *location()
{
    return "LOCATION";
}
constexpr TAG_PARSER_EXPORT const char *license()
{
    return "LICENSE";
}
constexpr TAG_PARSER_EXPORT const char *copyright()
{
    return "COPYRIGHT";
}
constexpr TAG_PARSER_EXPORT const char *opus()
{
    return "OPUS";
}
constexpr TAG_PARSER_EXPORT const char *sourceMedia()
{
    return "SOURCEMEDIA";
}
constexpr TAG_PARSER_EXPORT const char *cover()
{
    return "METADATA_BLOCK_PICTURE";
}

} // namespace VorbisCommentIds

} // namespace TagParser

#endif // TAG_PARSER_VORBISCOMMENTIDS_H
