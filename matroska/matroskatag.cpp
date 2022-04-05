#include "./matroskatag.h"
#include "./ebmlelement.h"

#include "../diagnostics.h"

#include <initializer_list>
#include <stdexcept>
#include <unordered_map>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::MatroskaTag
 * \brief Implementation of TagParser::Tag for the Matroska container.
 */

MatroskaTag::IdentifierType MatroskaTag::internallyGetFieldId(KnownField field) const
{
    using namespace MatroskaTagIds;
    switch (field) {
    case KnownField::Artist:
        return std::string(artist());
    case KnownField::Album:
        return std::string(album());
    case KnownField::Comment:
        return std::string(comment());
    case KnownField::RecordDate:
        return std::string(dateRecorded());
    case KnownField::ReleaseDate:
        return std::string(dateRelease());
    case KnownField::Title:
        return std::string(title());
    case KnownField::Genre:
        return std::string(genre());
    case KnownField::PartNumber:
        return std::string(partNumber());
    case KnownField::TotalParts:
        return std::string(totalParts());
    case KnownField::Encoder:
        return std::string(encoder());
    case KnownField::EncoderSettings:
        return std::string(encoderSettings());
    case KnownField::Bpm:
        return std::string(bpm());
    case KnownField::Bps:
        return std::string(bps());
    case KnownField::Rating:
        return std::string(rating());
    case KnownField::Description:
        return std::string(description());
    case KnownField::Lyrics:
        return std::string(lyrics());
    case KnownField::RecordLabel:
        return std::string(label());
    case KnownField::Performers:
        return std::string(actor());
    case KnownField::Lyricist:
        return std::string(lyricist());
    case KnownField::Composer:
        return std::string(composer());
    case KnownField::Length:
        return std::string(duration());
    case KnownField::Language:
        return std::string(language());
    case KnownField::AlbumArtist:
        return std::string(accompaniment());
    case KnownField::Subtitle:
        return std::string(subtitle());
    case KnownField::LeadPerformer:
        return std::string(leadPerformer());
    case KnownField::Arranger:
        return std::string(arranger());
    case KnownField::Conductor:
        return std::string(conductor());
    case KnownField::Director:
        return std::string(director());
    case KnownField::AssistantDirector:
        return std::string(assistantDirector());
    case KnownField::DirectorOfPhotography:
        return std::string(directorOfPhotography());
    case KnownField::SoundEngineer:
        return std::string(soundEngineer());
    case KnownField::ArtDirector:
        return std::string(artDirector());
    case KnownField::ProductionDesigner:
        return std::string(productionDesigner());
    case KnownField::Choregrapher:
        return std::string(choregrapher());
    case KnownField::CostumeDesigner:
        return std::string(costumeDesigner());
    case KnownField::Actor:
        return std::string(actor());
    case KnownField::Character:
        return std::string(character());
    case KnownField::WrittenBy:
        return std::string(writtenBy());
    case KnownField::ScreenplayBy:
        return std::string(screenplayBy());
    case KnownField::EditedBy:
        return std::string(editedBy());
    case KnownField::Producer:
        return std::string(producer());
    case KnownField::Coproducer:
        return std::string(coproducer());
    case KnownField::ExecutiveProducer:
        return std::string(executiveProducer());
    case KnownField::DistributedBy:
        return std::string(distributedBy());
    case KnownField::MasteredBy:
        return std::string(masteredBy());
    case KnownField::EncodedBy:
        return std::string(encodedBy());
    case KnownField::MixedBy:
        return std::string(mixedBy());
    case KnownField::RemixedBy:
        return std::string(remixedBy());
    case KnownField::ProductionStudio:
        return std::string(productionStudio());
    case KnownField::ThanksTo:
        return std::string(thanksTo());
    case KnownField::Publisher:
        return std::string(publisher());
    case KnownField::Mood:
        return std::string(mood());
    case KnownField::OriginalMediaType:
        return std::string(originalMediaType());
    case KnownField::ContentType:
        return std::string(contentType());
    case KnownField::Subject:
        return std::string(subject());
    case KnownField::Keywords:
        return std::string(keywords());
    case KnownField::Summary:
        return std::string(summary());
    case KnownField::Synopsis:
        return std::string(synopsis());
    case KnownField::InitialKey:
        return std::string(initialKey());
    case KnownField::Period:
        return std::string(period());
    case KnownField::LawRating:
        return std::string(lawRating());
    case KnownField::EncodingDate:
        return std::string(dateEncoded());
    case KnownField::TaggingDate:
        return std::string(dateTagged());
    case KnownField::DigitalizationDate:
        return std::string(dateDigitized());
    case KnownField::WritingDate:
        return std::string(dateWritten());
    case KnownField::PurchasingDate:
        return std::string(datePurchased());
    case KnownField::RecordingLocation:
        return std::string(recordingLocation());
    case KnownField::CompositionLocation:
        return std::string(compositionLocation());
    case KnownField::ComposerNationality:
        return std::string(composerNationality());
    case KnownField::PlayCounter:
        return std::string(playCounter());
    case KnownField::Measure:
        return std::string(measure());
    case KnownField::Tuning:
        return std::string(tuning());
    case KnownField::ISRC:
        return std::string(isrc());
    case KnownField::MCDI:
        return std::string(mcdi());
    case KnownField::ISBN:
        return std::string(isbn());
    case KnownField::Barcode:
        return std::string(barcode());
    case KnownField::CatalogNumber:
        return std::string(catalogNumber());
    case KnownField::LabelCode:
        return std::string(labelCode());
    case KnownField::LCCN:
        return std::string(lccn());
    case KnownField::IMDB:
        return std::string(imdb());
    case KnownField::TMDB:
        return std::string(tmdb());
    case KnownField::TVDB:
        return std::string(tvdb());
    case KnownField::PurchaseItem:
        return std::string(purchaseItem());
    case KnownField::PurchaseInfo:
        return std::string(purchaseInfo());
    case KnownField::PurchaseOwner:
        return std::string(purchaseOwner());
    case KnownField::PurchasePrice:
        return std::string(purchasePrice());
    case KnownField::PurchaseCurrency:
        return std::string(purchaseCurrency());
    case KnownField::Copyright:
        return std::string(copyright());
    case KnownField::ProductionCopyright:
        return std::string(productionCopyright());
    case KnownField::License:
        return std::string(license());
    case KnownField::TermsOfUse:
        return std::string(termsOfUse());
    default:
        return std::string();
    }
}

KnownField MatroskaTag::internallyGetKnownField(const IdentifierType &id) const
{
    using namespace MatroskaTagIds;
    static const std::unordered_map<std::string_view, KnownField> fieldMap({
        { artist(), KnownField::Artist },
        { album(), KnownField::Album },
        { comment(), KnownField::Comment },
        { dateRecorded(), KnownField::RecordDate },
        { dateRelease(), KnownField::ReleaseDate },
        { title(), KnownField::Title },
        { partNumber(), KnownField::PartNumber },
        { totalParts(), KnownField::TotalParts },
        { encoder(), KnownField::Encoder },
        { encoderSettings(), KnownField::EncoderSettings },
        { bpm(), KnownField::Bpm },
        { bps(), KnownField::Bps },
        { rating(), KnownField::Rating },
        { description(), KnownField::Description },
        { lyrics(), KnownField::Lyrics },
        { label(), KnownField::RecordLabel },
        { actor(), KnownField::Performers },
        { lyricist(), KnownField::Lyricist },
        { composer(), KnownField::Composer },
        { duration(), KnownField::Length },
        { language(), KnownField::Language },
        { accompaniment(), KnownField::AlbumArtist },
        { subtitle(), KnownField::Subtitle },
        { leadPerformer(), KnownField::LeadPerformer },
        { arranger(), KnownField::Arranger },
        { conductor(), KnownField::Conductor },
        { director(), KnownField::Director },
        { assistantDirector(), KnownField::AssistantDirector },
        { directorOfPhotography(), KnownField::DirectorOfPhotography },
        { soundEngineer(), KnownField::SoundEngineer },
        { artDirector(), KnownField::ArtDirector },
        { productionDesigner(), KnownField::ProductionDesigner },
        { choregrapher(), KnownField::Choregrapher },
        { costumeDesigner(), KnownField::CostumeDesigner },
        { actor(), KnownField::Actor },
        { character(), KnownField::Character },
        { writtenBy(), KnownField::WrittenBy },
        { screenplayBy(), KnownField::ScreenplayBy },
        { editedBy(), KnownField::EditedBy },
        { producer(), KnownField::Producer },
        { coproducer(), KnownField::Coproducer },
        { executiveProducer(), KnownField::ExecutiveProducer },
        { distributedBy(), KnownField::DistributedBy },
        { masteredBy(), KnownField::MasteredBy },
        { encodedBy(), KnownField::EncodedBy },
        { mixedBy(), KnownField::MixedBy },
        { remixedBy(), KnownField::RemixedBy },
        { productionStudio(), KnownField::ProductionStudio },
        { thanksTo(), KnownField::ThanksTo },
        { publisher(), KnownField::Publisher },
        { mood(), KnownField::Mood },
        { originalMediaType(), KnownField::OriginalMediaType },
        { contentType(), KnownField::ContentType },
        { subject(), KnownField::Subject },
        { keywords(), KnownField::Keywords },
        { summary(), KnownField::Summary },
        { synopsis(), KnownField::Synopsis },
        { initialKey(), KnownField::InitialKey },
        { period(), KnownField::Period },
        { lawRating(), KnownField::LawRating },
        { dateEncoded(), KnownField::EncodingDate },
        { dateTagged(), KnownField::TaggingDate },
        { dateDigitized(), KnownField::DigitalizationDate },
        { dateWritten(), KnownField::WritingDate },
        { datePurchased(), KnownField::PurchasingDate },
        { recordingLocation(), KnownField::RecordingLocation },
        { compositionLocation(), KnownField::CompositionLocation },
        { composerNationality(), KnownField::ComposerNationality },
        { playCounter(), KnownField::PlayCounter },
        { measure(), KnownField::Measure },
        { tuning(), KnownField::Tuning },
        { isrc(), KnownField::ISRC },
        { mcdi(), KnownField::MCDI },
        { isbn(), KnownField::ISBN },
        { barcode(), KnownField::Barcode },
        { catalogNumber(), KnownField::CatalogNumber },
        { labelCode(), KnownField::LabelCode },
        { lccn(), KnownField::LCCN },
        { imdb(), KnownField::IMDB },
        { tmdb(), KnownField::TMDB },
        { tvdb(), KnownField::TVDB },
        { purchaseItem(), KnownField::PurchaseItem },
        { purchaseInfo(), KnownField::PurchaseInfo },
        { purchaseOwner(), KnownField::PurchaseOwner },
        { purchasePrice(), KnownField::PurchasePrice },
        { purchaseCurrency(), KnownField::PurchaseCurrency },
        { copyright(), KnownField::Copyright },
        { productionCopyright(), KnownField::ProductionCopyright },
        { license(), KnownField::License },
        { termsOfUse(), KnownField::TermsOfUse },
    });
    const auto knownField(fieldMap.find(id));
    return knownField != fieldMap.cend() ? knownField->second : KnownField::Invalid;
}

/*!
 * \brief Parses tag information from the specified \a tagElement.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaTag::parse(EbmlElement &tagElement, Diagnostics &diag)
{
    parse2(tagElement, MatroskaTagFlags::None, diag);
}

/*!
 * \brief Parses tag information from the specified \a tagElement.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaTag::parse2(EbmlElement &tagElement, MatroskaTagFlags flags, Diagnostics &diag)
{
    static const string context("parsing Matroska tag");
    m_size = tagElement.totalSize();
    tagElement.parse(diag);
    if (tagElement.totalSize() > numeric_limits<std::uint32_t>::max()) {
        // FIXME: Support this? Likely not very useful in practise.
        diag.emplace_back(DiagLevel::Critical, "Matroska tag is too big.", context);
        throw NotImplementedException();
    }
    const auto normalize = flags & MatroskaTagFlags::NormalizeKnownFieldIds;
    for (EbmlElement *child = tagElement.firstChild(); child; child = child->nextSibling()) {
        child->parse(diag);
        switch (child->id()) {
        case MatroskaIds::SimpleTag:
            try {
                auto field = MatroskaTagField();
                field.reparse(*child, diag, true);
                if (normalize) {
                    auto normalizedId = field.id();
                    MatroskaTagField::normalizeId(normalizedId);
                    if (internallyGetKnownField(normalizedId) != KnownField::Invalid) {
                        field.id() = std::move(normalizedId);
                    }
                }
                fields().emplace(field.id(), std::move(field));
            } catch (const Failure &) {
                // message will be added to diag anyways
            }
            break;
        case MatroskaIds::Targets:
            parseTargets(*child, diag);
            break;
        }
    }
}

/*!
 * \brief Parses the specified \a targetsElement.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaTag::parseTargets(EbmlElement &targetsElement, Diagnostics &diag)
{
    static const string context("parsing targets of Matroska tag");
    m_target.clear();
    bool targetTypeValueFound = false;
    bool targetTypeFound = false;
    targetsElement.parse(diag);
    for (EbmlElement *child = targetsElement.firstChild(); child; child = child->nextSibling()) {
        try {
            child->parse(diag);
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Unable to parse children of Targets element.", context);
            break;
        }
        switch (child->id()) {
        case MatroskaIds::TargetTypeValue:
            if (!targetTypeValueFound) {
                m_target.setLevel(child->readUInteger());
                targetTypeValueFound = true;
            } else {
                diag.emplace_back(
                    DiagLevel::Warning, "Targets element contains multiple TargetTypeValue elements. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::TargetType:
            if (!targetTypeFound) {
                m_target.setLevelName(child->readString());
                targetTypeFound = true;
            } else {
                diag.emplace_back(
                    DiagLevel::Warning, "Targets element contains multiple TargetType elements. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::TagTrackUID:
            m_target.tracks().emplace_back(child->readUInteger());
            break;
        case MatroskaIds::TagEditionUID:
            m_target.editions().emplace_back(child->readUInteger());
            break;
        case MatroskaIds::TagChapterUID:
            m_target.chapters().emplace_back(child->readUInteger());
            break;
        case MatroskaIds::TagAttachmentUID:
            m_target.attachments().emplace_back(child->readUInteger());
            break;
        default:
            diag.emplace_back(DiagLevel::Warning, "Targets element contains unknown element. It will be ignored.", context);
        }
    }
    if (!m_target.level()) {
        m_target.setLevel(50); // default level
    }
}

/*!
 * \class TagParser::MatroskaTagMaker
 * \brief The MatroskaTagMaker class helps writing Matroska "Tag"-elements storing tag information.
 *
 * An instance can be obtained using the MatroskaTag::prepareMaking() method.
 */

/*!
 * \brief Prepares making the specified \a tag.
 * \sa See MatroskaTag::prepareMaking() for more information.
 */
MatroskaTagMaker::MatroskaTagMaker(MatroskaTag &tag, Diagnostics &diag)
    : m_tag(tag)
{
    // calculate size of "Targets" element
    m_targetsSize = 0; // NOT including ID and size
    if (m_tag.target().level() != 50) {
        // size of "TargetTypeValue"
        m_targetsSize += 2u + 1u + EbmlElement::calculateUIntegerLength(m_tag.target().level());
    }
    if (!m_tag.target().levelName().empty()) {
        // size of "TargetType"
        m_targetsSize += 2u + EbmlElement::calculateSizeDenotationLength(m_tag.target().levelName().size()) + m_tag.target().levelName().size();
    }
    for (const auto &v : initializer_list<vector<std::uint64_t>>{
             m_tag.target().tracks(), m_tag.target().editions(), m_tag.target().chapters(), m_tag.target().attachments() }) {
        for (auto uid : v) {
            // size of UID denotation
            m_targetsSize += 2u + 1u + EbmlElement::calculateUIntegerLength(uid);
        }
    }
    m_tagSize = 2u + EbmlElement::calculateSizeDenotationLength(m_targetsSize) + m_targetsSize;
    // calculate size of "SimpleTag" elements
    m_maker.reserve(m_tag.fields().size());
    m_simpleTagsSize = 0; // including ID and size
    for (auto &pair : m_tag.fields()) {
        if (pair.second.value().isNull()) {
            continue;
        }
        try {
            m_maker.emplace_back(pair.second.prepareMaking(diag));
            m_simpleTagsSize += m_maker.back().requiredSize();
        } catch (const Failure &) {
        }
    }
    m_tagSize += m_simpleTagsSize;
    m_totalSize = 2u + EbmlElement::calculateSizeDenotationLength(m_tagSize) + m_tagSize;
}

/*!
 * \brief Saves the tag (specified when constructing the object) to the
 *        specified \a stream (makes a "Tag"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw TagParser::Failure or a derived exception.
 */
void MatroskaTagMaker::make(ostream &stream) const
{
    // write header
    char buff[11];
    BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::Tag), buff);
    stream.write(buff, 2); // ID
    std::uint8_t len = EbmlElement::makeSizeDenotation(m_tagSize, buff);
    stream.write(buff, len); // size
    // write "Targets" element
    BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::Targets), buff);
    stream.write(buff, 2);
    len = EbmlElement::makeSizeDenotation(m_targetsSize, buff);
    stream.write(buff, len);
    const TagTarget &t = m_tag.target();
    if (t.level() != 50) {
        // write "TargetTypeValue"
        BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::TargetTypeValue), buff);
        stream.write(buff, 2);
        len = EbmlElement::makeUInteger(t.level(), buff);
        stream.put(static_cast<char>(0x80 | len));
        stream.write(buff, len);
    }
    if (!t.levelName().empty()) {
        // write "TargetType"
        BE::getBytes(static_cast<std::uint16_t>(MatroskaIds::TargetType), buff);
        stream.write(buff, 2);
        len = EbmlElement::makeSizeDenotation(t.levelName().size(), buff);
        stream.write(buff, len);
        stream.write(t.levelName().c_str(), static_cast<std::streamsize>(t.levelName().size()));
    }
    // write UIDs
    using p = pair<std::uint16_t, vector<std::uint64_t>>;
    for (const auto &pair : initializer_list<p>{ p(MatroskaIds::TagTrackUID, t.tracks()), p(MatroskaIds::TagEditionUID, t.editions()),
             p(MatroskaIds::TagChapterUID, t.chapters()), p(MatroskaIds::TagAttachmentUID, t.attachments()) }) {
        if (!pair.second.empty()) {
            BE::getBytes(pair.first, buff);
            for (auto uid : pair.second) {
                len = EbmlElement::makeUInteger(uid, buff + 3);
                *(buff + 2) = static_cast<char>(0x80 | len);
                stream.write(buff, 3 + len);
            }
        }
    }
    // write "SimpleTag" elements using maker objects prepared previously
    for (const auto &maker : m_maker) {
        maker.make(stream);
    }
}

} // namespace TagParser
