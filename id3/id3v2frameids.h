#ifndef ID3V2FRAMEIDS_H
#define ID3V2FRAMEIDS_H

#include <c++utilities/conversion/types.h>

#include <string>

namespace Media {

namespace Id3v2FrameIds {
enum KnownValue : uint32 {
    lAlbum = 0x54414c42,
    lArtist = 0x54504531,
    lComment = 0x434f4d4d,
    lYear = 0x54594552,
    lRecordDate = 0x54445243,
    lTitle = 0x54495432,
    lGenre = 0x54434f4e,
    lTrackPosition = 0x5452434b,
    lDiskPosition = 0x54504f53,
    lEncoder = 0x54454e43,
    lBpm = 0x5442504d,
    lCover = 0x41504943,
    //lPerformers = 0x54504532,
    lWriter = 0x54455854,
    lLength = 0x544c454e,
    lLanguage = 0x544c414e,
    lEncoderSettings = 0x54535345,
    lUnsynchronizedLyrics = 0x55534c54,
    lSynchronizedLyrics = 0x53594C54,
    lGrouping = 0x54504532,
    lRecordLabel = 0x54505542,
    lUniqueFileId = 0x55464944,
    lComposer = 0x54434f4d,
    lRating = 0x504f504d,

    sAlbum = 0x54414c,
    sArtist = 0x545031,
    sComment = 0x434f4d,
    sYear = 0x545945,
    sRecordDate = 0x545243,
    sTitle = 0x545432,
    sGenre = 0x54434f,
    sTrackPosition = 0x54524b,
    sEncoder = 0x54454e,
    sBpm = 0x544250,
    sCover = 0x504943,
    //sPerformers = 0x545032,
    sWriter = 0x545854,
    sLength = 0x544c45,
    sLanguage = 0x544c41,
    sEncoderSettings = 0x545353,
    sUnsynchronizedLyrics = 0x554C54,
    sSynchronizedLyrics = 0x534C54,
    sGrouping = 0x545032,
    sRecordLabel = 0x545042,
    sUniqueFileId = 0x554649,
    sComposer = 0x54434d,
    sRating = 0x504f50
};

uint32 convertToShortId(uint32 id);
uint32 convertToLongId(uint32 id);

/*!
 * \brief Returns an indication whether the specified \a id is a long frame id.
 */
inline bool isLongId(uint32 id)
{
    return (id & 0x00ffffff) != id;
}

/*!
 * \brief Returns an indication whether the specified \a id is a short frame id.
 */
inline bool isShortId(uint32 id)
{
    return (id & 0x00ffffff) == id;
}

/*!
 * \brief Returns an indication whether the specified \a id is a text frame id.
 */
inline bool isTextFrame(uint32 id)
{
    if(isShortId(id)) {
        return (id & 0x00FF0000u) == 0x00540000u;
    } else {
        return (id & 0xFF000000u) == 0x54000000u;
    }
}

}

}
#endif // ID3V2FRAMEIDS_H
