#ifndef TAG_PARSER_TAG_H
#define TAG_PARSER_TAG_H

#include "./tagtarget.h"
#include "./tagtype.h"
#include "./tagvalue.h"

#include <c++utilities/io/binaryreader.h>

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>

namespace TagParser {

/*!
 * \brief Specifies the field.
 *
 * These "known" fields are used to specify a field without using
 * the field identifier used by the underlying tag type.
 *
 * Not all fields are supported by all tag types (see Tag::supportsField()).
 *
 * Mapping proposed by HAK: https://wiki.hydrogenaud.io/index.php?title=Tag_Mapping
 *
 * \sa Tag::type()
 */
enum class KnownField : unsigned int {
    Invalid = std::numeric_limits<unsigned int>::max(), /**< invalid field name, do not map this value when subclassing Tag */
    Title = 0, /**< title */
    Album, /**< album/collection */
    Artist, /**< artist/band */
    Genre, /**< genre */
    Comment, /**< comment */
    Bpm, /**< beats per minute */
    Bps, /**< beats per second */
    Lyricist, /**< lyricist */
    TrackPosition, /**< track/part number and total track/part count */
    DiskPosition, /**< disk number and total disk count */
    PartNumber, /**< track/part number */
    TotalParts, /**< total track/part count */
    Encoder, /**< encoder */
    RecordDate, /**< record date */
    Performers, /**< performers */
    Length, /**< length */
    Language, /**< language */
    EncoderSettings, /**< encoder settings */
    Lyrics, /**< lyrics */
    SynchronizedLyrics, /**< synchronized lyrics */
    Grouping, /**< grouping */
    RecordLabel, /**< record label */
    Cover, /**< cover */
    Composer, /**< composer */
    Rating, /**< rating */
    Description, /**< description */
    Vendor, /**< vendor */
    AlbumArtist, /**< album artist */
    ReleaseDate, /**< release date */
    Subtitle, /**< subtitle */
    LeadPerformer, /** lead performer */
    Arranger, /** the person who arranged the piece */
    Conductor, /** conductor */
    Director, /** director */
    AssistantDirector, /** assistant director */
    DirectorOfPhotography, /** director of photography */
    SoundEngineer, /** sound engineer */
    ArtDirector, /** art director */
    ProductionDesigner, /** production designer */
    Choregrapher, /** choregrapher */
    CostumeDesigner, /** costume designer */
    Actor, /** actor */
    Character, /** character */
    WrittenBy, /** written by */
    ScreenplayBy, /** screenplay by */
    EditedBy, /** edited by */
    Producer, /** producer */
    Coproducer, /** coproducer */
    ExecutiveProducer, /** executive producer */
    DistributedBy, /** distributed by */
    MasteredBy, /** mastered by */
    EncodedBy, /** encoded by */
    MixedBy, /** mixed by */
    RemixedBy, /** remixed by */
    ProductionStudio, /** production studio */
    ThanksTo, /** thanks to */
    Publisher, /** publisher */
    Mood, /** mood */
    OriginalMediaType, /** original media type */
    ContentType, /** content type */
    Subject, /** subject */
    Keywords, /** keywords */
    Summary, /** summary */
    Synopsis, /** synopsis */
    InitialKey, /** initial key */
    Period, /** period */
    LawRating, /** law rating */
    EncodingDate, /** encoding date */
    TaggingDate, /** tagging date */
    OriginalReleaseDate, /** original release date */
    DigitalizationDate, /** digitalization date */
    WritingDate, /** writing date */
    PurchasingDate, /** purchasing date */
    RecordingLocation, /** recording location */
    CompositionLocation, /** composition location */
    ComposerNationality, /** composer nationality */
    PlayCounter, /** play counter */
    Measure, /** measure */
    Tuning, /** tuning */
    ISRC, /** International Standard Recording Code */
    MCDI, /** binary dump of the TOC of the CDROM that this item was taken from */
    ISBN, /** International Standard Book Number */
    Barcode, /** barcode */
    CatalogNumber, /** catalog number */
    LabelCode, /** label code */
    LCCN, /** Library of Congress Control Number */
    IMDB, /** Internet Movie Database ID */
    TMDB, /** The Movie DB “movie_id” or “tv_id” identifier for movies/TV shows */
    TVDB, /** The TV Database “Series ID” or “Episode ID” identifier for TV shows */
    PurchaseItem, /** purchase item URL */
    PurchaseInfo, /** purchase info */
    PurchaseOwner, /** purchase owner */
    PurchasePrice, /** purchase price */
    PurchaseCurrency, /** purchase currency */
    Copyright, /** copyright */
    ProductionCopyright, /** production copyright */
    License, /** license */
    TermsOfUse, /** terms of use */
    PublisherWebpage, /** the publisher's official webpage */
};

/*!
 * \brief The first valid entry in the TagParser::KnownField enum.
 */
constexpr KnownField firstKnownField = KnownField::Title;

/*!
 * \brief The last valid entry in the TagParser::KnownField enum.
 */
constexpr KnownField lastKnownField = KnownField::PublisherWebpage;

/*!
 * \brief The number of valid entries in the TagParser::KnownField enum.
 */
constexpr unsigned int knownFieldArraySize = static_cast<unsigned int>(lastKnownField) + 1;

/*!
 * \brief Returns whether the specified \a field is deprecated and should not be used anymore.
 */
constexpr bool isKnownFieldDeprecated(KnownField field)
{
    CPP_UTILITIES_UNUSED(field)
    return false;
}

/*!
 * \brief Returns the next known field skipping any deprecated fields. Returns KnownField::Invalid if there is not next field.
 */
constexpr KnownField nextKnownField(KnownField field)
{
    const auto next = field == lastKnownField ? KnownField::Invalid : static_cast<KnownField>(static_cast<int>(field) + 1);
    return isKnownFieldDeprecated(next) ? nextKnownField(next) : next;
}

struct TagPrivate;

class TAG_PARSER_EXPORT Tag {
public:
    virtual ~Tag();

    virtual TagType type() const;
    virtual std::string_view typeName() const;
    std::string toString() const;
    virtual TagTextEncoding proposedTextEncoding() const;
    virtual bool canEncodingBeUsed(TagTextEncoding encoding) const;
    virtual const TagValue &value(KnownField field) const = 0;
    virtual std::vector<const TagValue *> values(KnownField field) const;
    virtual bool setValue(KnownField field, const TagValue &value) = 0;
    virtual bool setValues(KnownField field, const std::vector<TagValue> &values);
    virtual bool hasField(KnownField field) const = 0;
    virtual void removeAllFields() = 0;
    const std::string &version() const;
    std::uint64_t size() const;
    virtual bool supportsTarget() const;
    const TagTarget &target() const;
    TagTarget &target();
    void setTarget(const TagTarget &target);
    virtual TagTargetLevel targetLevel() const;
    std::string_view targetLevelName() const;
    bool isTargetingLevel(TagTargetLevel tagTargetLevel) const;
    std::string targetString() const;
    virtual std::size_t fieldCount() const = 0;
    virtual bool supportsField(KnownField field) const = 0;
    virtual TagDataType proposedDataType(KnownField field) const;
    virtual bool supportsDescription(KnownField field) const;
    virtual bool supportsMimeType(KnownField field) const;
    virtual bool supportsMultipleValues(KnownField field) const;
    virtual std::size_t insertValues(const Tag &from, bool overwrite);
    virtual void ensureTextValuesAreProperlyEncoded() = 0;

protected:
    Tag();

    std::string m_version;
    std::uint64_t m_size;
    std::unique_ptr<TagPrivate> m_p;
    TagTarget m_target;
};

inline TagType Tag::type() const
{
    return TagType::Unspecified;
}

inline std::string_view Tag::typeName() const
{
    return "unspecified";
}

inline TagTextEncoding Tag::proposedTextEncoding() const
{
    return TagTextEncoding::Latin1;
}

inline bool Tag::canEncodingBeUsed(TagTextEncoding encoding) const
{
    return encoding == proposedTextEncoding();
}

inline const std::string &Tag::version() const
{
    return m_version;
}

inline std::uint64_t Tag::size() const
{
    return m_size;
}

inline bool Tag::supportsTarget() const
{
    return false;
}

inline const TagTarget &Tag::target() const
{
    return m_target;
}

inline TagTarget &Tag::target()
{
    return m_target;
}

inline void Tag::setTarget(const TagTarget &target)
{
    m_target = target;
}

inline TagTargetLevel Tag::targetLevel() const
{
    return TagTargetLevel::Unspecified;
}

inline std::string_view Tag::targetLevelName() const
{
    return supportsTarget() ? tagTargetLevelName(targetLevel()) : std::string_view();
}

inline bool Tag::isTargetingLevel(TagTargetLevel tagTargetLevel) const
{
    return !supportsTarget() || static_cast<std::uint8_t>(targetLevel()) >= static_cast<std::uint8_t>(tagTargetLevel);
}

inline std::string Tag::targetString() const
{
    return target().toString(targetLevel());
}

inline TagDataType Tag::proposedDataType(KnownField field) const
{
    switch (field) {
    case KnownField::Bpm:
    case KnownField::Bps:
    case KnownField::PartNumber:
    case KnownField::TotalParts:
    case KnownField::PlayCounter:
        return TagDataType::Integer;
    case KnownField::Cover:
        return TagDataType::Picture;
    case KnownField::Length:
        return TagDataType::TimeSpan;
    case KnownField::TrackPosition:
    case KnownField::DiskPosition:
        return TagDataType::PositionInSet;
    case KnownField::Genre:
        return TagDataType::StandardGenreIndex;
    case KnownField::MCDI:
        return TagDataType::Binary;
    case KnownField::Rating:
        // could also be a plain integer but popularity should generally be used (and can be converted
        // to an integer)
        return TagDataType::Popularity;
    case KnownField::SynchronizedLyrics:
        // not supported
        return TagDataType::Undefined;
    default:
        return TagDataType::Text;
    }
}

inline bool Tag::supportsDescription(KnownField) const
{
    return false;
}

inline bool Tag::supportsMimeType(KnownField) const
{
    return false;
}

inline bool Tag::supportsMultipleValues(KnownField) const
{
    return false;
}

} // namespace TagParser

#endif // TAG_PARSER_TAG_H
