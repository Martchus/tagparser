#ifndef MEDIA_MATROSKATAGIDS_H
#define MEDIA_MATROSKATAGIDS_H

#include "../tagtarget.h"

namespace Media {

/*!
 * \brief Encapsulates Matroska tag IDs.
 */
namespace MatroskaTagIds {

inline TAG_PARSER_EXPORT const char *original() {
    return "ORIGINAL";
}
inline TAG_PARSER_EXPORT const char *sample() {
    return "SAMPLE";
}
inline TAG_PARSER_EXPORT const char *country() {
    return "COUNTRY";
}

inline TAG_PARSER_EXPORT const char *totalParts() {
    return "TOTAL_PARTS";
}
inline TAG_PARSER_EXPORT const char *partNumber() {
    return "PART_NUMBER";
}
inline TAG_PARSER_EXPORT const char *partOffset() {
    return "PART_OFFSET";
}

inline TAG_PARSER_EXPORT const char *title() {
    return "TITLE";
}
inline TAG_PARSER_EXPORT const char *subtitle() {
    return "SUBTITLE";
}

inline TAG_PARSER_EXPORT const char *url() {
    return "URL";
}
inline TAG_PARSER_EXPORT const char *sortWith() {
    return "SORT_WITH";
}
inline TAG_PARSER_EXPORT const char *instruments() {
    return "INSTRUMENTS";
}
inline TAG_PARSER_EXPORT const char *email() {
    return "EMAIL";
}
inline TAG_PARSER_EXPORT const char *address() {
    return "ADDRESS";
}
inline TAG_PARSER_EXPORT const char *fax() {
    return "FAX";
}
inline TAG_PARSER_EXPORT const char *phone() {
    return "PHONE";
}

inline TAG_PARSER_EXPORT const char *artist() {
    return "ARTIST";
}
inline TAG_PARSER_EXPORT const char *album() {
    return "ALBUM";
}
inline TAG_PARSER_EXPORT const char *leadPerformer() {
    return "LEAD_PERFORMER";
}
inline TAG_PARSER_EXPORT const char *accompaniment() {
    return "ACCOMPANIMENT";
}
inline TAG_PARSER_EXPORT const char *composer() {
    return "COMPOSER";
}
inline TAG_PARSER_EXPORT const char *arranger() {
    return "ARRANGER";
}
inline TAG_PARSER_EXPORT const char *lyrics() {
    return "LYRICS";
}
inline TAG_PARSER_EXPORT const char *lyricist() {
    return "LYRICIST";
}
inline TAG_PARSER_EXPORT const char *conductor() {
    return "CONDUCTOR";
}
inline TAG_PARSER_EXPORT const char *director() {
    return "DIRECTOR";
}
inline TAG_PARSER_EXPORT const char *assistantDirector() {
    return "ASSISTANT_DIRECTOR";
}
inline TAG_PARSER_EXPORT const char *directorOfPhotography() {
    return "DIRECTOR_OF_PHOTOGRAPHY";
}
inline TAG_PARSER_EXPORT const char *soundEngineer() {
    return "SOUND_ENGINEER";
}
inline TAG_PARSER_EXPORT const char *artDirector() {
    return "ART_DIRECTOR";
}
inline TAG_PARSER_EXPORT const char *productionDesigner() {
    return "PRODUCTION_DESIGNER";
}
inline TAG_PARSER_EXPORT const char *choregrapher() {
    return "CHOREGRAPHER";
}
inline TAG_PARSER_EXPORT const char *costumeDesigner() {
    return "COSTUME_DESIGNER";
}
inline TAG_PARSER_EXPORT const char *actor() {
    return "ACTOR";
}
inline TAG_PARSER_EXPORT const char *character() {
    return "CHARACTER";
}
inline TAG_PARSER_EXPORT const char *writtenBy() {
    return "WRITTEN_BY";
}
inline TAG_PARSER_EXPORT const char *screenplayBy() {
    return "SCREENPLAY_BY";
}
inline TAG_PARSER_EXPORT const char *editedBy() {
    return "EDITED_BY";
}
inline TAG_PARSER_EXPORT const char *producer() {
    return "PRODUCER";
}
inline TAG_PARSER_EXPORT const char *coproducer() {
    return "COPRODUCER";
}
inline TAG_PARSER_EXPORT const char *executiveProducer() {
    return "EXECUTIVE_PRODUCER";
}
inline TAG_PARSER_EXPORT const char *distributedBy() {
    return "DISTRIBUTED_BY";
}
inline TAG_PARSER_EXPORT const char *masteredBy() {
    return "MASTERED_BY";
}
inline TAG_PARSER_EXPORT const char *encodedBy() {
    return "ENCODED_BY";
}
inline TAG_PARSER_EXPORT const char *mixedBy() {
    return "MIXED_BY";
}
inline TAG_PARSER_EXPORT const char *remixedBy() {
    return "REMIXED_BY";
}
inline TAG_PARSER_EXPORT const char *productionStudio() {
    return "PRODUCTION_STUDIO";
}
inline TAG_PARSER_EXPORT const char *thanksTo() {
    return "THANKS_TO";
}
inline TAG_PARSER_EXPORT const char *publisher() {
    return "PUBLISHER";
}
inline TAG_PARSER_EXPORT const char *label() {
    return "LABEL";
}

inline TAG_PARSER_EXPORT const char *genre() {
    return "GENRE";
}
inline TAG_PARSER_EXPORT const char *mood() {
    return "MOOD";
}
inline TAG_PARSER_EXPORT const char *originalMediaType() {
    return "ORIGINAL_MEDIA_TYPE";
}
inline TAG_PARSER_EXPORT const char *contentType() {
    return "CONTENT_TYPE";
}
inline TAG_PARSER_EXPORT const char *subject() {
    return "SUBJECT";
}
inline TAG_PARSER_EXPORT const char *description() {
    return "DESCRIPTION";
}
inline TAG_PARSER_EXPORT const char *keywords() {
    return "KEYWORDS";
}
inline TAG_PARSER_EXPORT const char *summary() {
    return "SUMMARY";
}
inline TAG_PARSER_EXPORT const char *synopsis() {
    return "SYNOPSIS";
}
inline TAG_PARSER_EXPORT const char *initialKey() {
    return "INITIAL_KEY";
}
inline TAG_PARSER_EXPORT const char *period() {
    return "PERIOD";
}
inline TAG_PARSER_EXPORT const char *lawRating() {
    return "LAW_RATING";
}
inline TAG_PARSER_EXPORT const char *icra() {
    return "ICRA";
}

inline TAG_PARSER_EXPORT const char *dateRelease() {
    return "DATE_RELEASED";
}
inline TAG_PARSER_EXPORT const char *dateRecorded() {
    return "DATE_RECORDED";
}
inline TAG_PARSER_EXPORT const char *dateEncoded() {
    return "DATE_ENCODED";
}
inline TAG_PARSER_EXPORT const char *dateTagged() {
    return "DATE_TAGGED";
}
inline TAG_PARSER_EXPORT const char *dateDigitized() {
    return "DATE_DIGITIZED";
}
inline TAG_PARSER_EXPORT const char *dateWritten() {
    return "DATE_WRITTEN";
}
inline TAG_PARSER_EXPORT const char *datePurchased() {
    return "DATE_PURCHASED";
}

inline TAG_PARSER_EXPORT const char *recordingLocation() {
    return "RECORDING_LOCATION";
}
inline TAG_PARSER_EXPORT const char *compositionLocation() {
    return "COMPOSITION_LOCATION";
}
inline TAG_PARSER_EXPORT const char *composerNationality() {
    return "COMPOSER_NATIONALITY";
}

inline TAG_PARSER_EXPORT const char *comment() {
    return "COMMENT";
}
inline TAG_PARSER_EXPORT const char *playCounter() {
    return "PLAY_COUNTER";
}
inline TAG_PARSER_EXPORT const char *rating() {
    return "RATING";
}

inline TAG_PARSER_EXPORT const char *encoder() {
    return "ENCODER";
}
inline TAG_PARSER_EXPORT const char *encoderSettings() {
    return "ENCODER_SETTINGS";
}
inline TAG_PARSER_EXPORT const char *bps() {
    return "BPS";
}
inline TAG_PARSER_EXPORT const char *fps() {
    return "FPS";
}
inline TAG_PARSER_EXPORT const char *bpm() {
    return "BPM";
}
inline TAG_PARSER_EXPORT const char *duration() {
    return "DURATION";
}
inline TAG_PARSER_EXPORT const char *language() {
    return "LANGUAGE";
}
inline TAG_PARSER_EXPORT const char *numberOfFrames() {
    return "NUMBER_OF_FRAMES";
}
inline TAG_PARSER_EXPORT const char *numberOfBytes() {
    return "NUMBER_OF_BYTES";
}
inline TAG_PARSER_EXPORT const char *measure() {
    return "MEASURE";
}
inline TAG_PARSER_EXPORT const char *tuning() {
    return "TUNING";
}
inline TAG_PARSER_EXPORT const char *replaygainGain() {
    return "REPLAYGAIN_GAIN";
}
inline TAG_PARSER_EXPORT const char *replaygainPeak() {
    return "REPLAYGAIN_PEAK";
}
inline TAG_PARSER_EXPORT const char *identifiers() {
    return "Identifiers";
}
inline TAG_PARSER_EXPORT const char *isrc() {
    return "ISRC";
}
inline TAG_PARSER_EXPORT const char *mcdi() {
    return "MCDI";
}
inline TAG_PARSER_EXPORT const char *isbn() {
    return "ISBN";
}
inline TAG_PARSER_EXPORT const char *barcode() {
    return "BARCODE";
}
inline TAG_PARSER_EXPORT const char *catalogNumber() {
    return "CATALOG_NUMBER";
}
inline TAG_PARSER_EXPORT const char *labelCode() {
    return "LABEL_CODE";
}
inline TAG_PARSER_EXPORT const char *lccn() {
    return "LCCN";
}

inline TAG_PARSER_EXPORT const char *purchaseItem() {
    return "PURCHASE_ITEM";
}
inline TAG_PARSER_EXPORT const char *purchaseInfo() {
    return "PURCHASE_INFO";
}
inline TAG_PARSER_EXPORT const char *purchaseOwner() {
    return "PURCHASE_OWNER";
}
inline TAG_PARSER_EXPORT const char *purchasePrice() {
    return "PURCHASE_PRICE";
}
inline TAG_PARSER_EXPORT const char *purchaseCurrency() {
    return "PURCHASE_CURRENCY";
}

inline TAG_PARSER_EXPORT const char *copyright() {
    return "COPYRIGHT";
}
inline TAG_PARSER_EXPORT const char *productionCopyright() {
    return "PRODUCTION_COPYRIGHT";
}
inline TAG_PARSER_EXPORT const char *license() {
    return "LICENSE";
}
inline TAG_PARSER_EXPORT const char *termsOfUse() {
    return "TERMS_OF_USE";
}

}

/*!
 * \brief Returns the general TagTargetLevel for the Matroska specific \a targetLevelValue.
 */
inline TAG_PARSER_EXPORT TagTargetLevel matroskaTagTargetLevel(uint64 targetLevelValue)
{
    return targetLevelValue > 70 ? TagTargetLevel::Collection : static_cast<TagTargetLevel>(targetLevelValue / 10);
}

/*!
 * \brief Returns the Matroska specific target level value for the specified general \a targetLevel.
 */
inline TAG_PARSER_EXPORT uint64 matroskaTagTargetLevelValue(TagTargetLevel targetLevel)
{
    return static_cast<uint64>(targetLevel) * 10;
}

} // namespace Media

#endif // MEDIA_MATROSKATAGIDS_H
