#include "id3v2frameids.h"
#include "../exceptions.h"

namespace Media
{

/*!
 * \brief Encapsulates the most common ID3v2 frame IDs and related functions.
 *
 * There are short frame IDs (used by the first version of ID3v2) and long
 * frame IDs (used by newer versions of ID3v2).
 *
 * The short IDs start with "s" and the long IDs with "l". To convert between
 * these IDs the functions convertToShortId() and convertToLongId() can be
 * used.
 */
namespace Id3v2FrameIds
{

/*!
 * \brief Converts the specified long frame ID to the equivalent short frame ID.
 * \returns Returns the short ID if available; otherwise returns 0.
 */
uint32 convertToShortId(uint32 id)
{
    switch(id) {
    case lAlbum: return sAlbum;
    case lArtist: return sArtist;
    case lComment: return sComment;
    case lYear: return sYear;
    case lRecordDate: return sRecordDate;
    case lTitle: return sTitle;
    case lGenre: return sGenre;
    case lTrackPosition: return sTrackPosition;
    case lEncoder: return sEncoder;
    case lBpm: return sBpm;
    case lCover: return sCover;
    case lWriter: return sWriter;
    case lLength: return sLength;
    case lLanguage: return sLanguage;
    case lEncoderSettings: return sEncoderSettings;
    case lUnsynchronizedLyrics: return sUnsynchronizedLyrics;
    case lGrouping: return sGrouping;
    case lRecordLabel: return sRecordLabel;
    default: return 0;
    }
}

/*!
 * \brief Converts the specified short frame ID to the equivalent long frame ID.
 * \returns Returns the long ID if available; otherwise returns 0.
 */
uint32 convertToLongId(uint32 id)
{
    switch(id) {
    case sAlbum: return lAlbum;
    case sArtist: return lArtist;
    case sComment: return lComment;
    case sYear: return lYear;
    case sRecordDate: return lRecordDate;
    case sTitle: return lTitle;
    case sGenre: return lGenre;
    case sTrackPosition: return lTrackPosition;
    case sEncoder: return lEncoder;
    case sBpm: return lBpm;
    case sCover: return lCover;
    case sWriter: return lWriter;
    case sLength: return lLength;
    case sLanguage: return lLanguage;
    case sEncoderSettings: return lEncoderSettings;
    case sUnsynchronizedLyrics: return lUnsynchronizedLyrics;
    case sGrouping: return lGrouping;
    case sRecordLabel: return lRecordLabel;
    default: return 0;
    }
}

}

}
