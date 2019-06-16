#include "./id3v2frameids.h"

#include "../exceptions.h"

namespace TagParser {

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
namespace Id3v2FrameIds {

/*!
 * \brief Converts the specified long frame ID to the equivalent short frame ID.
 * \returns Returns the short ID if available; otherwise returns 0.
 */
std::uint32_t convertToShortId(std::uint32_t id)
{
    switch (id) {
    case lAlbum:
        return sAlbum;
    case lArtist:
        return sArtist;
    case lComment:
        return sComment;
    case lYear:
        return sYear;
    case lRecordDate:
        return sRecordDate;
    case lTitle:
        return sTitle;
    case lGenre:
        return sGenre;
    case lTrackPosition:
        return sTrackPosition;
    case lDiskPosition:
        return sDiskPosition;
    case lEncoder:
        return sEncoder;
    case lBpm:
        return sBpm;
    case lCover:
        return sCover;
    case lWriter:
        return sWriter;
    case lLength:
        return sLength;
    case lLanguage:
        return sLanguage;
    case lEncoderSettings:
        return sEncoderSettings;
    case lUnsynchronizedLyrics:
        return sUnsynchronizedLyrics;
    case lAlbumArtist:
        return sAlbumArtist;
    case lContentGroupDescription:
        return sContentGroupDescription;
    case lRecordLabel:
        return sRecordLabel;
    case lUserDefinedText:
        return sUserDefinedText;
    default:
        return 0;
    }
}

/*!
 * \brief Converts the specified short frame ID to the equivalent long frame ID.
 * \returns Returns the long ID if available; otherwise returns 0.
 */
std::uint32_t convertToLongId(std::uint32_t id)
{
    switch (id) {
    case sAlbum:
        return lAlbum;
    case sArtist:
        return lArtist;
    case sComment:
        return lComment;
    case sYear:
        return lYear;
    case sRecordDate:
        return lRecordDate;
    case sTitle:
        return lTitle;
    case sGenre:
        return lGenre;
    case sTrackPosition:
        return lTrackPosition;
    case sDiskPosition:
        return lDiskPosition;
    case sEncoder:
        return lEncoder;
    case sBpm:
        return lBpm;
    case sCover:
        return lCover;
    case sWriter:
        return lWriter;
    case sLength:
        return lLength;
    case sLanguage:
        return lLanguage;
    case sEncoderSettings:
        return lEncoderSettings;
    case sUnsynchronizedLyrics:
        return lUnsynchronizedLyrics;
    case sAlbumArtist:
        return lAlbumArtist;
    case sContentGroupDescription:
        return lContentGroupDescription;
    case sRecordLabel:
        return lRecordLabel;
    case sUserDefinedText:
        return lUserDefinedText;
    default:
        return 0;
    }
}

} // namespace Id3v2FrameIds

} // namespace TagParser
