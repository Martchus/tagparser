#ifndef TAG_PARSER_ID3V2FRAMEIDS_H
#define TAG_PARSER_ID3V2FRAMEIDS_H

#include "../global.h"

#include <cstdint>

#include <string>

namespace TagParser {

namespace Id3v2FrameIds {
enum KnownValue : std::uint32_t {
    lAlbum = 0x54414c42, /**< TALB */
    lArtist = 0x54504531, /**< TPE1 */
    lComment = 0x434f4d4d, /**< COMM */
    lYear = 0x54594552, /**< TYER */
    lOriginalYear = 0x544F5259, /**< TORY */
    lRecordingDates = 0x54524441, /**< TRDA */
    lDate = 0x54444154, /**< TDAT */
    lTime = 0x54494D45, /**< TIME */
    lRecordDate = 0x54445243, /**< FIXME v10: remove in favor of lRecordingTime */
    lRecordingTime = 0x54445243, /**< TDRC */
    lReleaseTime = 0x5444524C, /**< TDRL */
    lOriginalReleaseTime = 0x54444F52, /**< TDOR */
    lTaggingTime = 0x54445447, /**< TDTG */
    lTitle = 0x54495432, /**< TIT2 */
    lGenre = 0x54434f4e, /**< TCON */
    lTrackPosition = 0x5452434b, /**< TRCK */
    lDiskPosition = 0x54504f53, /**< TPOS */
    lEncoder = 0x54454e43, /**< TENC */
    lBpm = 0x5442504d, /**< TBPM */
    lCover = 0x41504943, /**< APIC */
    lWriter = 0x54455854, /**< TEXT */
    lLength = 0x544c454e, /**< TLEN */
    lLanguage = 0x544c414e, /**< TLAN */
    lEncoderSettings = 0x54535345, /**< TSSE */
    lUnsynchronizedLyrics = 0x55534c54, /**< USLT */
    lSynchronizedLyrics = 0x53594C54, /**< SYLT */
    lAlbumArtist = 0x54504532, /**< TPE2 */
    lContentGroupDescription = 0x54495431, /**< TIT1 */
    lRecordLabel = 0x54505542, /**< TPUB */
    lUniqueFileId = 0x55464944, /**< UFID */
    lComposer = 0x54434f4d, /**< TCOM */
    lRating = 0x504f504d, /**< POPM */
    lUserDefinedText = 0x54585858, /**< TXXX */

    sAlbum = 0x54414c, /**< ?TAL */
    sArtist = 0x545031, /**< ?TP1 */
    sComment = 0x434f4d, /**< ?COM */
    sYear = 0x545945, /**< ?TYE */
    sOriginalYear = 0x544F5259, /**< TORY */
    sRecordingDates = 0x545244, /**< ?TRD */
    sDate = 0x544441, /**< ?TDA */
    sTime = 0x54494D, /**< ?TIM */
    sTitle = 0x545432, /**< ?TT2 */
    sGenre = 0x54434f, /**< ?TCO */
    sTrackPosition = 0x54524b, /**< ?TRK */
    sDiskPosition = 0x545041, /**< ?TPA */
    sEncoder = 0x54454e, /**< ?TEN */
    sBpm = 0x544250, /**< ?TBP */
    sCover = 0x504943, /**< ?PIC */
    sWriter = 0x545854, /**< ?TXT */
    sLength = 0x544c45, /**< ?TLE */
    sLanguage = 0x544c41, /**< ?TLA */
    sEncoderSettings = 0x545353, /**< ?TSS */
    sUnsynchronizedLyrics = 0x554C54, /**< ?ULT */
    sSynchronizedLyrics = 0x534C54, /**< ?SLT */
    sAlbumArtist = 0x545032, /**< ?TP2 */
    sContentGroupDescription = 0x545431, /**< ?TT1 */
    sRecordLabel = 0x545042, /**< ?TPB */
    sUniqueFileId = 0x554649, /**< ?UFI */
    sComposer = 0x54434d, /**< ?TCM */
    sRating = 0x504f50, /**< ?POP */
    sUserDefinedText = 0x545858, /**< ?TXX */
};

TAG_PARSER_EXPORT std::uint32_t convertToShortId(std::uint32_t id);
TAG_PARSER_EXPORT std::uint32_t convertToLongId(std::uint32_t id);
TAG_PARSER_EXPORT bool isPreId3v24Id(std::uint32_t id);
TAG_PARSER_EXPORT bool isOnlyId3v24Id(std::uint32_t id);

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
