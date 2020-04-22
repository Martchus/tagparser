#ifndef TAG_PARSER_ID3V2FRAMEIDS_H
#define TAG_PARSER_ID3V2FRAMEIDS_H

#include <cstdint>

#include <string>

namespace TagParser {

namespace Id3v2FrameIds {
enum KnownValue : std::uint32_t {
    lAlbum = 0x54414c42,
    lArtist = 0x54504531,
    lComment = 0x434f4d4d,
    lYear = 0x54594552,
    lOriginalYear = 0x544F5259,
    lRecordingDates = 0x54524441,
    lDate = 0x54444154,
    lTime = 0x54494D45,
    lRecordDate = 0x54445243, // FIXME v10: remove in favor of lRecordingTime
    lRecordingTime = 0x54445243,
    lReleaseTime = 0x5444524C,
    lOriginalReleaseTime = 0x54444F52,
    lTaggingTime = 0x54445447,
    lTitle = 0x54495432,
    lGenre = 0x54434f4e,
    lTrackPosition = 0x5452434b,
    lDiskPosition = 0x54504f53,
    lEncoder = 0x54454e43,
    lBpm = 0x5442504d,
    lCover = 0x41504943,
    lWriter = 0x54455854,
    lLength = 0x544c454e,
    lLanguage = 0x544c414e,
    lEncoderSettings = 0x54535345,
    lUnsynchronizedLyrics = 0x55534c54,
    lSynchronizedLyrics = 0x53594C54,
    lAlbumArtist = 0x54504532,
    lContentGroupDescription = 0x54495431,
    lRecordLabel = 0x54505542,
    lUniqueFileId = 0x55464944,
    lComposer = 0x54434f4d,
    lRating = 0x504f504d,
    lUserDefinedText = 0x54585858,

    sAlbum = 0x54414c,
    sArtist = 0x545031,
    sComment = 0x434f4d,
    sYear = 0x545945,
    sOriginalYear = 0x544F5259,
    sRecordingDates = 0x545244,
    sRecordDate = 0x545243,
    sDate = 0x544441,
    sTime = 0x54494D,
    sTitle = 0x545432,
    sGenre = 0x54434f,
    sTrackPosition = 0x54524b,
    sDiskPosition = 0x545041,
    sEncoder = 0x54454e,
    sBpm = 0x544250,
    sCover = 0x504943,
    sWriter = 0x545854,
    sLength = 0x544c45,
    sLanguage = 0x544c41,
    sEncoderSettings = 0x545353,
    sUnsynchronizedLyrics = 0x554C54,
    sSynchronizedLyrics = 0x534C54,
    sAlbumArtist = 0x545032,
    sContentGroupDescription = 0x545431,
    sRecordLabel = 0x545042,
    sUniqueFileId = 0x554649,
    sComposer = 0x54434d,
    sRating = 0x504f50,
    sUserDefinedText = 0x545858,
};

std::uint32_t convertToShortId(std::uint32_t id);
std::uint32_t convertToLongId(std::uint32_t id);

/*!
 * \brief Returns an indication whether the specified \a id is a long frame id.
 */
constexpr bool isLongId(std::uint32_t id)
{
    return (id & 0x00ffffff) != id;
}

/*!
 * \brief Returns an indication whether the specified \a id is a short frame id.
 */
constexpr bool isShortId(std::uint32_t id)
{
    return (id & 0x00ffffff) == id;
}

/*!
 * \brief Returns an indication whether the specified \a id is a text frame id.
 */
constexpr bool isTextFrame(std::uint32_t id)
{
    if (isShortId(id)) {
        return ((id & 0x00FF0000u) == 0x00540000u) && (id != Id3v2FrameIds::sUserDefinedText);
    } else {
        return (id & 0xFF000000u) == 0x54000000u && (id != Id3v2FrameIds::lUserDefinedText);
    }
}

} // namespace Id3v2FrameIds

} // namespace TagParser
#endif // TAG_PARSER_ID3V2FRAMEIDS_H
