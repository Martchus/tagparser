#ifndef TAG_PARSER_VORBISCOMMENTIDS_H
#define TAG_PARSER_VORBISCOMMENTIDS_H

#include "../global.h"

#include <string_view>

namespace TagParser {

/*!
 * \brief Encapsulates Vorbis comment field names.
 * \sa
 * - See https://xiph.org/vorbis/doc/v-comment.html for the upstream documentation of the field names.
 * - See https://wiki.xiph.org/Field_names for an additional proposal that is most notably introducing
 *   `DISCNUMBER` and `…TOTAL` fields.
 * - See https://wiki.hydrogenaud.io/index.php?title=Tag_Mapping for further conventions and a
 *   comparision with other formats.
 */
namespace VorbisCommentIds {

constexpr TAG_PARSER_EXPORT std::string_view trackNumber()
{
    return "TRACKNUMBER";
}
constexpr TAG_PARSER_EXPORT std::string_view trackTotal()
{
    return "TRACKTOTAL";
}
constexpr TAG_PARSER_EXPORT std::string_view diskNumber()
{
    return "DISCNUMBER";
}
constexpr TAG_PARSER_EXPORT std::string_view diskTotal()
{
    return "DISCTOTAL";
}
constexpr TAG_PARSER_EXPORT std::string_view part()
{
    return "PART";
}
constexpr TAG_PARSER_EXPORT std::string_view partNumber()
{
    return "PARTNUMBER";
}
constexpr TAG_PARSER_EXPORT std::string_view partTotal()
{
    return "PARTTOTAL";
}
constexpr TAG_PARSER_EXPORT std::string_view title()
{
    return "TITLE";
}
constexpr TAG_PARSER_EXPORT std::string_view version()
{
    return "VERSION";
}
constexpr TAG_PARSER_EXPORT std::string_view artist()
{
    return "ARTIST";
}
constexpr TAG_PARSER_EXPORT std::string_view albumArtist()
{
    return "ALBUMARTIST";
}
constexpr TAG_PARSER_EXPORT std::string_view grouping()
{
    return "GROUPING";
}
constexpr TAG_PARSER_EXPORT std::string_view album()
{
    return "ALBUM";
}
constexpr TAG_PARSER_EXPORT std::string_view label()
{
    return "LABEL";
}
constexpr TAG_PARSER_EXPORT std::string_view labelNo()
{
    return "LABELNO";
}
constexpr TAG_PARSER_EXPORT std::string_view language()
{
    return "LANGUAGE";
}
constexpr TAG_PARSER_EXPORT std::string_view performer()
{
    return "PERFORMER";
}
constexpr TAG_PARSER_EXPORT std::string_view composer()
{
    return "COMPOSER";
}
constexpr TAG_PARSER_EXPORT std::string_view ensemble()
{
    return "ENSEMBLE";
}
constexpr TAG_PARSER_EXPORT std::string_view arranger()
{
    return "ARRANGER";
}
constexpr TAG_PARSER_EXPORT std::string_view lyricist()
{
    return "LYRICIST";
}
constexpr TAG_PARSER_EXPORT std::string_view lyrics()
{
    return "LYRICS";
}
constexpr TAG_PARSER_EXPORT std::string_view author()
{
    return "AUTHOR";
}
constexpr TAG_PARSER_EXPORT std::string_view conductor()
{
    return "CONDUCTOR";
}
constexpr TAG_PARSER_EXPORT std::string_view encoder()
{
    return "ENCODER";
}
constexpr TAG_PARSER_EXPORT std::string_view encodedBy()
{
    return "ENCODED_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view encoderSettings()
{
    return "ENCODER_OPTIONS";
}
constexpr TAG_PARSER_EXPORT std::string_view publisher()
{
    return "PUBLISHER";
}
constexpr TAG_PARSER_EXPORT std::string_view genre()
{
    return "GENRE";
}
constexpr TAG_PARSER_EXPORT std::string_view originalMediaType()
{
    return "ORIGINAL_TAG_PARSER_TYPE";
}
constexpr TAG_PARSER_EXPORT std::string_view contentType()
{
    return "CONTENT_TYPE";
}
constexpr TAG_PARSER_EXPORT std::string_view subject()
{
    return "SUBJECT";
}
constexpr TAG_PARSER_EXPORT std::string_view description()
{
    return "DESCRIPTION";
}
constexpr TAG_PARSER_EXPORT std::string_view director()
{
    return "DIRECTOR";
}
constexpr TAG_PARSER_EXPORT std::string_view isrc()
{
    return "ISRC";
}
constexpr TAG_PARSER_EXPORT std::string_view rating()
{
    return "RATING";
}
constexpr TAG_PARSER_EXPORT std::string_view eanupn()
{
    return "EAN/UPN";
}
constexpr TAG_PARSER_EXPORT std::string_view comment()
{
    return "COMMENT";
}
constexpr TAG_PARSER_EXPORT std::string_view date()
{
    return "DATE";
}
constexpr TAG_PARSER_EXPORT std::string_view year()
{
    return "YEAR"; // not mentioned in https://xiph.org/vorbis/doc/v-comment.html but seen in the wild
}
constexpr TAG_PARSER_EXPORT std::string_view location()
{
    return "LOCATION";
}
constexpr TAG_PARSER_EXPORT std::string_view license()
{
    return "LICENSE";
}
constexpr TAG_PARSER_EXPORT std::string_view copyright()
{
    return "COPYRIGHT";
}
constexpr TAG_PARSER_EXPORT std::string_view opus()
{
    return "OPUS";
}
constexpr TAG_PARSER_EXPORT std::string_view sourceMedia()
{
    return "SOURCEMEDIA";
}
constexpr TAG_PARSER_EXPORT std::string_view cover()
{
    return "METADATA_BLOCK_PICTURE";
}
constexpr TAG_PARSER_EXPORT std::string_view bpm()
{
    return "BPM";
}
constexpr TAG_PARSER_EXPORT std::string_view publisherWebpage()
{
    return "WWWPUBLISHER";
}

} // namespace VorbisCommentIds

} // namespace TagParser

#endif // TAG_PARSER_VORBISCOMMENTIDS_H
