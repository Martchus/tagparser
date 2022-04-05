#ifndef TAG_PARSER_MATROSKATAGIDS_H
#define TAG_PARSER_MATROSKATAGIDS_H

#include "../tagtarget.h"

#include <string_view>

namespace TagParser {

namespace MatroskaTagIds {

// nesting information

constexpr TAG_PARSER_EXPORT std::string_view original()
{
    return "ORIGINAL";
}
constexpr TAG_PARSER_EXPORT std::string_view sample()
{
    return "SAMPLE";
}
constexpr TAG_PARSER_EXPORT std::string_view country()
{
    return "COUNTRY";
}

// organization information

constexpr TAG_PARSER_EXPORT std::string_view totalParts()
{
    return "TOTAL_PARTS";
}
constexpr TAG_PARSER_EXPORT std::string_view partNumber()
{
    return "PART_NUMBER";
}
constexpr TAG_PARSER_EXPORT std::string_view partOffset()
{
    return "PART_OFFSET";
}

// titles

constexpr TAG_PARSER_EXPORT std::string_view title()
{
    return "TITLE";
}
constexpr TAG_PARSER_EXPORT std::string_view subtitle()
{
    return "SUBTITLE";
}

// nested information

constexpr TAG_PARSER_EXPORT std::string_view url()
{
    return "URL";
}
constexpr TAG_PARSER_EXPORT std::string_view sortWith()
{
    return "SORT_WITH";
}
constexpr TAG_PARSER_EXPORT std::string_view instruments()
{
    return "INSTRUMENTS";
}
constexpr TAG_PARSER_EXPORT std::string_view email()
{
    return "EMAIL";
}
constexpr TAG_PARSER_EXPORT std::string_view address()
{
    return "ADDRESS";
}
constexpr TAG_PARSER_EXPORT std::string_view fax()
{
    return "FAX";
}
constexpr TAG_PARSER_EXPORT std::string_view phone()
{
    return "PHONE";
}

// entities

constexpr TAG_PARSER_EXPORT std::string_view artist()
{
    return "ARTIST";
}
constexpr TAG_PARSER_EXPORT std::string_view album()
{
    return "ALBUM";
}
constexpr TAG_PARSER_EXPORT std::string_view leadPerformer()
{
    return "LEAD_PERFORMER";
}
constexpr TAG_PARSER_EXPORT std::string_view accompaniment()
{
    return "ACCOMPANIMENT";
}
constexpr TAG_PARSER_EXPORT std::string_view composer()
{
    return "COMPOSER";
}
constexpr TAG_PARSER_EXPORT std::string_view arranger()
{
    return "ARRANGER";
}
constexpr TAG_PARSER_EXPORT std::string_view lyrics()
{
    return "LYRICS";
}
constexpr TAG_PARSER_EXPORT std::string_view lyricist()
{
    return "LYRICIST";
}
constexpr TAG_PARSER_EXPORT std::string_view conductor()
{
    return "CONDUCTOR";
}
constexpr TAG_PARSER_EXPORT std::string_view director()
{
    return "DIRECTOR";
}
constexpr TAG_PARSER_EXPORT std::string_view assistantDirector()
{
    return "ASSISTANT_DIRECTOR";
}
constexpr TAG_PARSER_EXPORT std::string_view directorOfPhotography()
{
    return "DIRECTOR_OF_PHOTOGRAPHY";
}
constexpr TAG_PARSER_EXPORT std::string_view soundEngineer()
{
    return "SOUND_ENGINEER";
}
constexpr TAG_PARSER_EXPORT std::string_view artDirector()
{
    return "ART_DIRECTOR";
}
constexpr TAG_PARSER_EXPORT std::string_view productionDesigner()
{
    return "PRODUCTION_DESIGNER";
}
constexpr TAG_PARSER_EXPORT std::string_view choregrapher()
{
    return "CHOREGRAPHER";
}
constexpr TAG_PARSER_EXPORT std::string_view costumeDesigner()
{
    return "COSTUME_DESIGNER";
}
constexpr TAG_PARSER_EXPORT std::string_view actor()
{
    return "ACTOR";
}
constexpr TAG_PARSER_EXPORT std::string_view character()
{
    return "CHARACTER";
}
constexpr TAG_PARSER_EXPORT std::string_view writtenBy()
{
    return "WRITTEN_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view screenplayBy()
{
    return "SCREENPLAY_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view editedBy()
{
    return "EDITED_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view producer()
{
    return "PRODUCER";
}
constexpr TAG_PARSER_EXPORT std::string_view coproducer()
{
    return "COPRODUCER";
}
constexpr TAG_PARSER_EXPORT std::string_view executiveProducer()
{
    return "EXECUTIVE_PRODUCER";
}
constexpr TAG_PARSER_EXPORT std::string_view distributedBy()
{
    return "DISTRIBUTED_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view masteredBy()
{
    return "MASTERED_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view encodedBy()
{
    return "ENCODED_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view mixedBy()
{
    return "MIXED_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view remixedBy()
{
    return "REMIXED_BY";
}
constexpr TAG_PARSER_EXPORT std::string_view productionStudio()
{
    return "PRODUCTION_STUDIO";
}
constexpr TAG_PARSER_EXPORT std::string_view thanksTo()
{
    return "THANKS_TO";
}
constexpr TAG_PARSER_EXPORT std::string_view publisher()
{
    return "PUBLISHER";
}
constexpr TAG_PARSER_EXPORT std::string_view label()
{
    return "LABEL";
}

// search and classification

constexpr TAG_PARSER_EXPORT std::string_view genre()
{
    return "GENRE";
}
constexpr TAG_PARSER_EXPORT std::string_view mood()
{
    return "MOOD";
}
constexpr TAG_PARSER_EXPORT std::string_view originalMediaType()
{
    return "ORIGINAL_MEDIA_TYPE";
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
constexpr TAG_PARSER_EXPORT std::string_view keywords()
{
    return "KEYWORDS";
}
constexpr TAG_PARSER_EXPORT std::string_view summary()
{
    return "SUMMARY";
}
constexpr TAG_PARSER_EXPORT std::string_view synopsis()
{
    return "SYNOPSIS";
}
constexpr TAG_PARSER_EXPORT std::string_view initialKey()
{
    return "INITIAL_KEY";
}
constexpr TAG_PARSER_EXPORT std::string_view period()
{
    return "PERIOD";
}
constexpr TAG_PARSER_EXPORT std::string_view lawRating()
{
    return "LAW_RATING";
}
/// \deprecated Not sure what this is or should have been. Remove in v12.
constexpr TAG_PARSER_EXPORT std::string_view icra()
{
    return "ICRA";
}

// temporal information

constexpr TAG_PARSER_EXPORT std::string_view dateRelease()
{
    return "DATE_RELEASED";
}
constexpr TAG_PARSER_EXPORT std::string_view dateRecorded()
{
    return "DATE_RECORDED";
}
constexpr TAG_PARSER_EXPORT std::string_view dateEncoded()
{
    return "DATE_ENCODED";
}
constexpr TAG_PARSER_EXPORT std::string_view dateTagged()
{
    return "DATE_TAGGED";
}
constexpr TAG_PARSER_EXPORT std::string_view dateDigitized()
{
    return "DATE_DIGITIZED";
}
constexpr TAG_PARSER_EXPORT std::string_view dateWritten()
{
    return "DATE_WRITTEN";
}
constexpr TAG_PARSER_EXPORT std::string_view datePurchased()
{
    return "DATE_PURCHASED";
}

// spatial information

constexpr TAG_PARSER_EXPORT std::string_view recordingLocation()
{
    return "RECORDING_LOCATION";
}
constexpr TAG_PARSER_EXPORT std::string_view compositionLocation()
{
    return "COMPOSITION_LOCATION";
}
constexpr TAG_PARSER_EXPORT std::string_view composerNationality()
{
    return "COMPOSER_NATIONALITY";
}

// personal

constexpr TAG_PARSER_EXPORT std::string_view comment()
{
    return "COMMENT";
}
constexpr TAG_PARSER_EXPORT std::string_view playCounter()
{
    return "PLAY_COUNTER";
}
constexpr TAG_PARSER_EXPORT std::string_view rating()
{
    return "RATING";
}

// technical information

constexpr TAG_PARSER_EXPORT std::string_view encoder()
{
    return "ENCODER";
}
constexpr TAG_PARSER_EXPORT std::string_view encoderSettings()
{
    return "ENCODER_SETTINGS";
}
constexpr TAG_PARSER_EXPORT std::string_view bps()
{
    return "BPS";
}
constexpr TAG_PARSER_EXPORT std::string_view fps()
{
    return "FPS";
}
constexpr TAG_PARSER_EXPORT std::string_view bpm()
{
    return "BPM";
}
constexpr TAG_PARSER_EXPORT std::string_view measure()
{
    return "MEASURE";
}
constexpr TAG_PARSER_EXPORT std::string_view tuning()
{
    return "TUNING";
}
constexpr TAG_PARSER_EXPORT std::string_view replaygainGain()
{
    return "REPLAYGAIN_GAIN";
}
constexpr TAG_PARSER_EXPORT std::string_view replaygainPeak()
{
    return "REPLAYGAIN_PEAK";
}
constexpr TAG_PARSER_EXPORT std::string_view duration()
{
    return "DURATION";
}
constexpr TAG_PARSER_EXPORT std::string_view numberOfFrames()
{
    return "NUMBER_OF_FRAMES";
}
constexpr TAG_PARSER_EXPORT std::string_view numberOfBytes()
{
    return "NUMBER_OF_BYTES";
}

// identifiers

constexpr TAG_PARSER_EXPORT std::string_view identifiers()
{
    return "Identifiers";
}
constexpr TAG_PARSER_EXPORT std::string_view isrc()
{
    return "ISRC";
}
constexpr TAG_PARSER_EXPORT std::string_view mcdi()
{
    return "MCDI";
}
constexpr TAG_PARSER_EXPORT std::string_view isbn()
{
    return "ISBN";
}
constexpr TAG_PARSER_EXPORT std::string_view barcode()
{
    return "BARCODE";
}
constexpr TAG_PARSER_EXPORT std::string_view catalogNumber()
{
    return "CATALOG_NUMBER";
}
constexpr TAG_PARSER_EXPORT std::string_view labelCode()
{
    return "LABEL_CODE";
}
constexpr TAG_PARSER_EXPORT std::string_view lccn()
{
    return "LCCN";
}
constexpr TAG_PARSER_EXPORT std::string_view imdb()
{
    return "IMDB";
}
constexpr TAG_PARSER_EXPORT std::string_view tmdb()
{
    return "TMDB";
}
constexpr TAG_PARSER_EXPORT std::string_view tvdb()
{
    return "TVDB";
}

// commercial

constexpr TAG_PARSER_EXPORT std::string_view purchaseItem()
{
    return "PURCHASE_ITEM";
}
constexpr TAG_PARSER_EXPORT std::string_view purchaseInfo()
{
    return "PURCHASE_INFO";
}
constexpr TAG_PARSER_EXPORT std::string_view purchaseOwner()
{
    return "PURCHASE_OWNER";
}
constexpr TAG_PARSER_EXPORT std::string_view purchasePrice()
{
    return "PURCHASE_PRICE";
}
constexpr TAG_PARSER_EXPORT std::string_view purchaseCurrency()
{
    return "PURCHASE_CURRENCY";
}

// legal

constexpr TAG_PARSER_EXPORT std::string_view copyright()
{
    return "COPYRIGHT";
}
constexpr TAG_PARSER_EXPORT std::string_view productionCopyright()
{
    return "PRODUCTION_COPYRIGHT";
}
constexpr TAG_PARSER_EXPORT std::string_view license()
{
    return "LICENSE";
}
constexpr TAG_PARSER_EXPORT std::string_view termsOfUse()
{
    return "TERMS_OF_USE";
}

// other

constexpr TAG_PARSER_EXPORT std::string_view language()
{
    return "LANGUAGE";
}

namespace TrackSpecific {
constexpr TAG_PARSER_EXPORT std::string_view numberOfBytes()
{
    return "NUMBER_OF_BYTES";
}
constexpr TAG_PARSER_EXPORT std::string_view numberOfFrames()
{
    return "NUMBER_OF_FRAMES";
}
constexpr TAG_PARSER_EXPORT std::string_view duration()
{
    return "DURATION";
}
/// \brief The track's bit rate in bits per second.
constexpr TAG_PARSER_EXPORT std::string_view bitrate()
{
    return "BPS";
}
constexpr TAG_PARSER_EXPORT std::string_view writingApp()
{
    return "_STATISTICS_WRITING_APP";
}
constexpr TAG_PARSER_EXPORT std::string_view writingDate()
{
    return "_STATISTICS_WRITING_DATE_UTC";
}
constexpr TAG_PARSER_EXPORT std::string_view statisticsTags()
{
    return "_STATISTICS_TAGS";
}
} // namespace TrackSpecific

} // namespace MatroskaTagIds

/*!
 * \brief Returns the general TagTargetLevel for the Matroska specific \a targetLevelValue.
 */
constexpr TAG_PARSER_EXPORT TagTargetLevel matroskaTagTargetLevel(std::uint64_t targetLevelValue)
{
    return targetLevelValue > 70 ? TagTargetLevel::Collection : static_cast<TagTargetLevel>(targetLevelValue / 10);
}

/*!
 * \brief Returns the Matroska specific target level value for the specified general \a targetLevel.
 */
constexpr TAG_PARSER_EXPORT std::uint64_t matroskaTagTargetLevelValue(TagTargetLevel targetLevel)
{
    return static_cast<std::uint64_t>(targetLevel) * 10;
}

} // namespace TagParser

#endif // TAG_PARSER_MATROSKATAGIDS_H
