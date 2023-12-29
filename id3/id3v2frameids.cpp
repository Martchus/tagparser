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
 *
 * \sa
 * - See https://id3.org/id3v2.3.0 for the specification of ID3v2.3.0 frame IDs.
 * - See https://id3.org/id3v2.4.0-frames for the specification of ID3v2.4.0 frame IDs.
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
    case lOriginalYear:
        return sOriginalYear;
    case lRecordingDates:
        return sRecordingDates;
    case lDate:
        return sDate;
    case lTime:
        return sTime;
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
    case lRemixedBy:
        return sRemixedBy;
    case lCopyright:
        return sCopyright;
    case lPlayCounter:
        return sPlayCounter;
    case lRating:
        return sRating;
    case lISRC:
        return sISRC;
    case lPublisherWebpage:
        return sPublisherWebpage;
    case lUserDefinedURL:
        return sUserDefinedURL;
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
    case sOriginalYear:
        return lOriginalYear;
    case sRecordingDates:
        return lRecordingDates;
    case sDate:
        return lDate;
    case sTime:
        return lTime;
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
    case sRemixedBy:
        return lRemixedBy;
    case sCopyright:
        return lCopyright;
    case sPlayCounter:
        return lPlayCounter;
    case sRating:
        return lRating;
    case sISRC:
        return lISRC;
    case sPublisherWebpage:
        return lPublisherWebpage;
    case sUserDefinedURL:
        return lUserDefinedURL;
    default:
        return 0;
    }
}

/*!
 * \brief Returns whether \a id is only supported in ID3v2.3.x and older and therefore can not be used in an ID3v2.4.x tag.
 * \remarks
 * - This function is intended to show warnings. Unknown IDs will be treated as supported everywhere.
 * - Any short ID is obviously not ID3v2.4.x compatible. Only long IDs are considered here. Short IDs need to be converted to
 *   long IDs before passing them to this function.
 */
bool isPreId3v24Id(uint32_t id)
{
    switch (id) {
    case lYear:
    case lOriginalYear:
    case lRecordingDates:
    case lDate:
    case lTime:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns whether \a id is only supported inID3v2.4.x and therefore can not be used in older versions.
 * \remarks This function is intended to show warnings. Unknown IDs will be treated as supported everywhere.
 */
bool isOnlyId3v24Id(uint32_t id)
{
    switch (id) {
    case lRecordingTime:
    case lReleaseTime:
    case lOriginalReleaseTime:
    case lTaggingTime:
        return true;
    default:
        return false;
    }
}

} // namespace Id3v2FrameIds

} // namespace TagParser
