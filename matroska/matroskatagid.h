#ifndef TAG_PARSER_MATROSKATAGIDS_H
#define TAG_PARSER_MATROSKATAGIDS_H

#include "../tagtarget.h"

#include <string_view>

namespace TagParser {

namespace MatroskaTagIds {

// nesting information

constexpr std::string_view original()
{
    return "ORIGINAL";
}
constexpr std::string_view sample()
{
    return "SAMPLE";
}
constexpr std::string_view country()
{
    return "COUNTRY";
}

// organization information

constexpr std::string_view totalParts()
{
    return "TOTAL_PARTS";
}
constexpr std::string_view partNumber()
{
    return "PART_NUMBER";
}
constexpr std::string_view partOffset()
{
    return "PART_OFFSET";
}

// titles

constexpr std::string_view title()
{
    return "TITLE";
}
constexpr std::string_view subtitle()
{
    return "SUBTITLE";
}

// nested information

constexpr std::string_view url()
{
    return "URL";
}
constexpr std::string_view sortWith()
{
    return "SORT_WITH";
}
constexpr std::string_view instruments()
{
    return "INSTRUMENTS";
}
constexpr std::string_view email()
{
    return "EMAIL";
}
constexpr std::string_view address()
{
    return "ADDRESS";
}
constexpr std::string_view fax()
{
    return "FAX";
}
constexpr std::string_view phone()
{
    return "PHONE";
}

// entities

constexpr std::string_view artist()
{
    return "ARTIST";
}
constexpr std::string_view album()
{
    return "ALBUM";
}
constexpr std::string_view leadPerformer()
{
    return "LEAD_PERFORMER";
}
constexpr std::string_view accompaniment()
{
    return "ACCOMPANIMENT";
}
constexpr std::string_view composer()
{
    return "COMPOSER";
}
constexpr std::string_view arranger()
{
    return "ARRANGER";
}
constexpr std::string_view lyrics()
{
    return "LYRICS";
}
constexpr std::string_view lyricist()
{
    return "LYRICIST";
}
constexpr std::string_view conductor()
{
    return "CONDUCTOR";
}
constexpr std::string_view director()
{
    return "DIRECTOR";
}
constexpr std::string_view assistantDirector()
{
    return "ASSISTANT_DIRECTOR";
}
constexpr std::string_view directorOfPhotography()
{
    return "DIRECTOR_OF_PHOTOGRAPHY";
}
constexpr std::string_view soundEngineer()
{
    return "SOUND_ENGINEER";
}
constexpr std::string_view artDirector()
{
    return "ART_DIRECTOR";
}
constexpr std::string_view productionDesigner()
{
    return "PRODUCTION_DESIGNER";
}
constexpr std::string_view choregrapher()
{
    return "CHOREGRAPHER";
}
constexpr std::string_view costumeDesigner()
{
    return "COSTUME_DESIGNER";
}
constexpr std::string_view actor()
{
    return "ACTOR";
}
constexpr std::string_view character()
{
    return "CHARACTER";
}
constexpr std::string_view writtenBy()
{
    return "WRITTEN_BY";
}
constexpr std::string_view screenplayBy()
{
    return "SCREENPLAY_BY";
}
constexpr std::string_view editedBy()
{
    return "EDITED_BY";
}
constexpr std::string_view producer()
{
    return "PRODUCER";
}
constexpr std::string_view coproducer()
{
    return "COPRODUCER";
}
constexpr std::string_view executiveProducer()
{
    return "EXECUTIVE_PRODUCER";
}
constexpr std::string_view distributedBy()
{
    return "DISTRIBUTED_BY";
}
constexpr std::string_view masteredBy()
{
    return "MASTERED_BY";
}
constexpr std::string_view encodedBy()
{
    return "ENCODED_BY";
}
constexpr std::string_view mixedBy()
{
    return "MIXED_BY";
}
constexpr std::string_view remixedBy()
{
    return "REMIXED_BY";
}
constexpr std::string_view productionStudio()
{
    return "PRODUCTION_STUDIO";
}
constexpr std::string_view thanksTo()
{
    return "THANKS_TO";
}
constexpr std::string_view publisher()
{
    return "PUBLISHER";
}
constexpr std::string_view label()
{
    return "LABEL";
}

// search and classification

constexpr std::string_view genre()
{
    return "GENRE";
}
constexpr std::string_view mood()
{
    return "MOOD";
}
constexpr std::string_view originalMediaType()
{
    return "ORIGINAL_MEDIA_TYPE";
}
constexpr std::string_view contentType()
{
    return "CONTENT_TYPE";
}
constexpr std::string_view subject()
{
    return "SUBJECT";
}
constexpr std::string_view description()
{
    return "DESCRIPTION";
}
constexpr std::string_view keywords()
{
    return "KEYWORDS";
}
constexpr std::string_view summary()
{
    return "SUMMARY";
}
constexpr std::string_view synopsis()
{
    return "SYNOPSIS";
}
constexpr std::string_view initialKey()
{
    return "INITIAL_KEY";
}
constexpr std::string_view period()
{
    return "PERIOD";
}
constexpr std::string_view lawRating()
{
    return "LAW_RATING";
}
/// \deprecated Not sure what this is or should have been. Remove in v12.
constexpr std::string_view icra()
{
    return "ICRA";
}

// temporal information

constexpr std::string_view dateRelease()
{
    return "DATE_RELEASED";
}
constexpr std::string_view dateRecorded()
{
    return "DATE_RECORDED";
}
constexpr std::string_view dateEncoded()
{
    return "DATE_ENCODED";
}
constexpr std::string_view dateTagged()
{
    return "DATE_TAGGED";
}
constexpr std::string_view dateDigitized()
{
    return "DATE_DIGITIZED";
}
constexpr std::string_view dateWritten()
{
    return "DATE_WRITTEN";
}
constexpr std::string_view datePurchased()
{
    return "DATE_PURCHASED";
}

// spatial information

constexpr std::string_view recordingLocation()
{
    return "RECORDING_LOCATION";
}
constexpr std::string_view compositionLocation()
{
    return "COMPOSITION_LOCATION";
}
constexpr std::string_view composerNationality()
{
    return "COMPOSER_NATIONALITY";
}

// personal

constexpr std::string_view comment()
{
    return "COMMENT";
}
constexpr std::string_view playCounter()
{
    return "PLAY_COUNTER";
}
constexpr std::string_view rating()
{
    return "RATING";
}

// technical information

constexpr std::string_view encoder()
{
    return "ENCODER";
}
constexpr std::string_view encoderSettings()
{
    return "ENCODER_SETTINGS";
}
constexpr std::string_view bps()
{
    return "BPS";
}
constexpr std::string_view fps()
{
    return "FPS";
}
constexpr std::string_view bpm()
{
    return "BPM";
}
constexpr std::string_view measure()
{
    return "MEASURE";
}
constexpr std::string_view tuning()
{
    return "TUNING";
}
constexpr std::string_view replaygainGain()
{
    return "REPLAYGAIN_GAIN";
}
constexpr std::string_view replaygainPeak()
{
    return "REPLAYGAIN_PEAK";
}
constexpr std::string_view duration()
{
    return "DURATION";
}
constexpr std::string_view numberOfFrames()
{
    return "NUMBER_OF_FRAMES";
}
constexpr std::string_view numberOfBytes()
{
    return "NUMBER_OF_BYTES";
}

// identifiers

constexpr std::string_view identifiers()
{
    return "Identifiers";
}
constexpr std::string_view isrc()
{
    return "ISRC";
}
constexpr std::string_view mcdi()
{
    return "MCDI";
}
constexpr std::string_view isbn()
{
    return "ISBN";
}
constexpr std::string_view barcode()
{
    return "BARCODE";
}
constexpr std::string_view catalogNumber()
{
    return "CATALOG_NUMBER";
}
constexpr std::string_view labelCode()
{
    return "LABEL_CODE";
}
constexpr std::string_view lccn()
{
    return "LCCN";
}
constexpr std::string_view imdb()
{
    return "IMDB";
}
constexpr std::string_view tmdb()
{
    return "TMDB";
}
constexpr std::string_view tvdb()
{
    return "TVDB";
}

// commercial

constexpr std::string_view purchaseItem()
{
    return "PURCHASE_ITEM";
}
constexpr std::string_view purchaseInfo()
{
    return "PURCHASE_INFO";
}
constexpr std::string_view purchaseOwner()
{
    return "PURCHASE_OWNER";
}
constexpr std::string_view purchasePrice()
{
    return "PURCHASE_PRICE";
}
constexpr std::string_view purchaseCurrency()
{
    return "PURCHASE_CURRENCY";
}

// legal

constexpr std::string_view copyright()
{
    return "COPYRIGHT";
}
constexpr std::string_view productionCopyright()
{
    return "PRODUCTION_COPYRIGHT";
}
constexpr std::string_view license()
{
    return "LICENSE";
}
constexpr std::string_view termsOfUse()
{
    return "TERMS_OF_USE";
}

// other

constexpr std::string_view language()
{
    return "LANGUAGE";
}

namespace TrackSpecific {
constexpr std::string_view numberOfBytes()
{
    return "NUMBER_OF_BYTES";
}
constexpr std::string_view numberOfFrames()
{
    return "NUMBER_OF_FRAMES";
}
constexpr std::string_view duration()
{
    return "DURATION";
}
/// \brief The track's bit rate in bits per second.
constexpr std::string_view bitrate()
{
    return "BPS";
}
constexpr std::string_view writingApp()
{
    return "_STATISTICS_WRITING_APP";
}
constexpr std::string_view writingDate()
{
    return "_STATISTICS_WRITING_DATE_UTC";
}
constexpr std::string_view statisticsTags()
{
    return "_STATISTICS_TAGS";
}
} // namespace TrackSpecific

} // namespace MatroskaTagIds

/*!
 * \brief Returns the general TagTargetLevel for the Matroska specific \a targetLevelValue.
 */
constexpr TagTargetLevel matroskaTagTargetLevel(std::uint64_t targetLevelValue)
{
    return targetLevelValue > 70 ? TagTargetLevel::Collection : static_cast<TagTargetLevel>(targetLevelValue / 10);
}

/*!
 * \brief Returns the Matroska specific target level value for the specified general \a targetLevel.
 */
constexpr std::uint64_t matroskaTagTargetLevelValue(TagTargetLevel targetLevel)
{
    return static_cast<std::uint64_t>(targetLevel) * 10;
}

} // namespace TagParser

#endif // TAG_PARSER_MATROSKATAGIDS_H
