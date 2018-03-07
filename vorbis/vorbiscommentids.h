#ifndef TAG_PARSER_VORBISCOMMENTIDS_H
#define TAG_PARSER_VORBISCOMMENTIDS_H

#include "../global.h"

namespace TagParser {

/*!
 * \brief Encapsulates Vorbis comment IDs.
 */
namespace VorbisCommentIds {

inline TAG_PARSER_EXPORT const char *trackNumber()
{
    return "TRACKNUMBER";
}
inline TAG_PARSER_EXPORT const char *diskNumber()
{
    return "DISCNUMBER";
}
inline TAG_PARSER_EXPORT const char *part()
{
    return "PART";
}
inline TAG_PARSER_EXPORT const char *partNumber()
{
    return "PARTNUMBER";
}
inline TAG_PARSER_EXPORT const char *title()
{
    return "TITLE";
}
inline TAG_PARSER_EXPORT const char *version()
{
    return "VERSION";
}
inline TAG_PARSER_EXPORT const char *artist()
{
    return "ARTIST";
}
inline TAG_PARSER_EXPORT const char *album()
{
    return "ALBUM";
}
inline TAG_PARSER_EXPORT const char *label()
{
    return "LABEL";
}
inline TAG_PARSER_EXPORT const char *labelNo()
{
    return "LABELNO";
}
inline TAG_PARSER_EXPORT const char *language()
{
    return "LANGUAGE";
}
inline TAG_PARSER_EXPORT const char *performer()
{
    return "PERFORMER";
}
inline TAG_PARSER_EXPORT const char *composer()
{
    return "COMPOSER";
}
inline TAG_PARSER_EXPORT const char *ensemble()
{
    return "ENSEMBLE";
}
inline TAG_PARSER_EXPORT const char *arranger()
{
    return "ARRANGER";
}
inline TAG_PARSER_EXPORT const char *lyricist()
{
    return "LYRICIST";
}
inline TAG_PARSER_EXPORT const char *author()
{
    return "AUTHOR";
}
inline TAG_PARSER_EXPORT const char *conductor()
{
    return "CONDUCTOR";
}
inline TAG_PARSER_EXPORT const char *encoder()
{
    return "ENCODER";
}
inline TAG_PARSER_EXPORT const char *publisher()
{
    return "PUBLISHER";
}
inline TAG_PARSER_EXPORT const char *genre()
{
    return "GENRE";
}
inline TAG_PARSER_EXPORT const char *originalMediaType()
{
    return "ORIGINAL_TAG_PARSER_TYPE";
}
inline TAG_PARSER_EXPORT const char *contentType()
{
    return "CONTENT_TYPE";
}
inline TAG_PARSER_EXPORT const char *subject()
{
    return "SUBJECT";
}
inline TAG_PARSER_EXPORT const char *description()
{
    return "DESCRIPTION";
}
inline TAG_PARSER_EXPORT const char *isrc()
{
    return "ISRC";
}
inline TAG_PARSER_EXPORT const char *eanupn()
{
    return "EAN/UPN";
}
inline TAG_PARSER_EXPORT const char *comment()
{
    return "COMMENT";
}
inline TAG_PARSER_EXPORT const char *encoderSettings()
{
    return "ENCODING";
}
inline TAG_PARSER_EXPORT const char *date()
{
    return "DATE";
}
inline TAG_PARSER_EXPORT const char *location()
{
    return "LOCATION";
}
inline TAG_PARSER_EXPORT const char *license()
{
    return "LICENSE";
}
inline TAG_PARSER_EXPORT const char *copyright()
{
    return "COPYRIGHT";
}
inline TAG_PARSER_EXPORT const char *opus()
{
    return "OPUS";
}
inline TAG_PARSER_EXPORT const char *sourceMedia()
{
    return "SOURCEMEDIA";
}
inline TAG_PARSER_EXPORT const char *cover()
{
    return "METADATA_BLOCK_PICTURE";
}

} // namespace VorbisCommentIds

} // namespace TagParser

#endif // TAG_PARSER_VORBISCOMMENTIDS_H
