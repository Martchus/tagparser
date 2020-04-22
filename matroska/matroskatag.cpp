#include "./matroskatag.h"
#include "./ebmlelement.h"

#include "../diagnostics.h"

#include <initializer_list>
#include <map>
#include <stdexcept>

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
        return artist();
    case KnownField::Album:
        return album();
    case KnownField::Comment:
        return comment();
    case KnownField::RecordDate:
    case KnownField::Year:
        return dateRecorded();
    case KnownField::ReleaseDate:
        return dateRelease();
    case KnownField::Title:
        return title();
    case KnownField::Genre:
        return genre();
    case KnownField::PartNumber:
        return partNumber();
    case KnownField::TotalParts:
        return totalParts();
    case KnownField::Encoder:
        return encoder();
    case KnownField::EncoderSettings:
        return encoderSettings();
    case KnownField::Bpm:
        return bpm();
    case KnownField::Bps:
        return bps();
    case KnownField::Rating:
        return rating();
    case KnownField::Description:
        return description();
    case KnownField::Lyrics:
        return lyrics();
    case KnownField::RecordLabel:
        return label();
    case KnownField::Performers:
        return actor();
    case KnownField::Lyricist:
        return lyricist();
    case KnownField::Composer:
        return composer();
    case KnownField::Length:
        return duration();
    case KnownField::Language:
        return language();
    default:
        return string();
    }
}

KnownField MatroskaTag::internallyGetKnownField(const IdentifierType &id) const
{
    using namespace MatroskaTagIds;
    static const map<string, KnownField> fieldMap({
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
    static const string context("parsing Matroska tag");
    tagElement.parse(diag);
    if (tagElement.totalSize() > numeric_limits<std::uint32_t>::max()) {
        // FIXME: Support this? Likely not very useful in practise.
        diag.emplace_back(DiagLevel::Critical, "Matroska tag is too big.", context);
        throw NotImplementedException();
    }
    m_size = static_cast<std::uint32_t>(tagElement.totalSize());
    for (EbmlElement *child = tagElement.firstChild(); child; child = child->nextSibling()) {
        child->parse(diag);
        switch (child->id()) {
        case MatroskaIds::SimpleTag:
            try {
                MatroskaTagField field;
                field.reparse(*child, diag, true);
                fields().emplace(field.id(), move(field));
            } catch (const Failure &) {
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
        m_targetsSize += 2 + 1 + EbmlElement::calculateUIntegerLength(m_tag.target().level());
    }
    if (!m_tag.target().levelName().empty()) {
        // size of "TargetType"
        m_targetsSize += 2 + EbmlElement::calculateSizeDenotationLength(m_tag.target().levelName().size()) + m_tag.target().levelName().size();
    }
    for (const auto &v : initializer_list<vector<std::uint64_t>>{
             m_tag.target().tracks(), m_tag.target().editions(), m_tag.target().chapters(), m_tag.target().attachments() }) {
        for (auto uid : v) {
            // size of UID denotation
            m_targetsSize += 2 + 1 + EbmlElement::calculateUIntegerLength(uid);
        }
    }
    m_tagSize = 2 + EbmlElement::calculateSizeDenotationLength(m_targetsSize) + m_targetsSize;
    // calculate size of "SimpleTag" elements
    m_maker.reserve(m_tag.fields().size());
    m_simpleTagsSize = 0; // including ID and size
    for (auto &pair : m_tag.fields()) {
        try {
            m_maker.emplace_back(pair.second.prepareMaking(diag));
            m_simpleTagsSize += m_maker.back().requiredSize();
        } catch (const Failure &) {
        }
    }
    m_tagSize += m_simpleTagsSize;
    m_totalSize = 2 + EbmlElement::calculateSizeDenotationLength(m_tagSize) + m_tagSize;
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
        stream.write(t.levelName().c_str(), t.levelName().size());
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
