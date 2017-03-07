#include "./matroskatag.h"
#include "./ebmlelement.h"

#include <map>
#include <initializer_list>
#include <stdexcept>

using namespace std;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::MatroskaTag
 * \brief Implementation of Media::Tag for the Matroska container.
 */

MatroskaTag::IdentifierType MatroskaTag::internallyGetFieldId(KnownField field) const
{
    using namespace MatroskaTagIds;
    switch(field) {
    case KnownField::Artist: return artist();
    case KnownField::Album: return album();
    case KnownField::Comment: return comment();
    case KnownField::RecordDate: return dateRecorded();
    case KnownField::Year: return dateRelease();
    case KnownField::Title: return title();
    case KnownField::Genre: return genre();
    case KnownField::PartNumber: return partNumber();
    case KnownField::TotalParts: return totalParts();
    case KnownField::Encoder: return encoder();
    case KnownField::EncoderSettings: return encoderSettings();
    case KnownField::Bpm: return bpm();
    case KnownField::Bps: return bps();
    case KnownField::Rating: return rating();
    case KnownField::Description: return description();
    case KnownField::Lyrics: return lyrics();
    case KnownField::RecordLabel: return label();
    case KnownField::Performers: return actor();
    case KnownField::Lyricist: return lyricist();
    case KnownField::Composer: return composer();
    case KnownField::Length: return duration();
    case KnownField::Language: return language();
    default: return string();
    }
}

KnownField MatroskaTag::internallyGetKnownField(const IdentifierType &id) const
{
    using namespace MatroskaTagIds;
    static const map<string, KnownField> map({
        {artist(), KnownField::Artist},
        {album(), KnownField::Album},
        {comment(), KnownField::Comment},
        {dateRecorded(), KnownField::RecordDate},
        {dateRelease(), KnownField::Year},
        {title(), KnownField::Title},
        {partNumber(), KnownField::PartNumber},
        {totalParts(), KnownField::TotalParts},
        {encoder(), KnownField::Encoder},
        {encoderSettings(), KnownField::EncoderSettings},
        {bpm(), KnownField::Bpm},
        {bps(), KnownField::Bps},
        {rating(), KnownField::Rating},
        {description(), KnownField::Description},
        {lyrics(), KnownField::Lyrics},
        {label(), KnownField::RecordLabel},
        {actor(), KnownField::Performers},
        {lyricist(), KnownField::Lyricist},
        {composer(), KnownField::Composer},
        {duration(), KnownField::Length},
        {language(), KnownField::Language},
    });
    try {
        return map.at(id);
    } catch(const out_of_range &) {
        return KnownField::Invalid;
    }
}

/*!
 * \brief Parses tag information from the specified \a tagElement.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaTag::parse(EbmlElement &tagElement)
{
    invalidateStatus();
    static const string context("parsing Matroska tag");
    tagElement.parse();
    m_size = tagElement.totalSize();
    MatroskaTagField field;
    for(EbmlElement *child = tagElement.firstChild(); child; child = child->nextSibling()) {
        child->parse();
        switch(child->id()) {
        case MatroskaIds::SimpleTag: {
            try {
                field.invalidateNotifications();
                field.reparse(*child, true);
                fields().insert(make_pair(field.id(), field));
            } catch(const Failure &) {
            }
            addNotifications(context, field);
            break;
        } case MatroskaIds::Targets:
            parseTargets(*child);
            break;
        }
    }
}

/*!
 * \brief Prepares making.
 * \returns Returns a MatroskaTagMaker object which can be used to actually make the tag.
 * \remarks The tag must NOT be mutated after making is prepared when it is intended to actually
 *          make the tag using the make() method of the returned object.
 * \throws Throws Media::Failure or a derived exception when a making error occurs.
 *
 * This method might be useful when it is necessary to know the size of the tag before making it.
 * \sa make()
 * \todo Make inline in next major release.
 */
MatroskaTagMaker MatroskaTag::prepareMaking()
{
    return MatroskaTagMaker(*this);
}

/*!
 * \brief Writes tag information to the specified \a stream (makes a "Tag"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 * \sa prepareMaking()
 * \todo Make inline in next major release.
 */
void MatroskaTag::make(ostream &stream)
{
    prepareMaking().make(stream);
}

/*!
 * \brief Parses the specified \a targetsElement.
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a parsing
 *         error occurs.
 */
void MatroskaTag::parseTargets(EbmlElement &targetsElement)
{
    static const string context("parsing targets of Matroska tag");
    m_target.clear();
    bool targetTypeValueFound = false;
    bool targetTypeFound = false;
    targetsElement.parse();
    for(EbmlElement *child = targetsElement.firstChild(); child; child = child->nextSibling()) {
        try {
            child->parse();
        } catch(const Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse childs of Targets element.", context);
            break;
        }
        switch(child->id()) {
        case MatroskaIds::TargetTypeValue:
            if(!targetTypeValueFound) {
                m_target.setLevel(child->readUInteger());
                targetTypeValueFound = true;
            } else {
                addNotification(NotificationType::Warning, "Targets element contains multiple TargetTypeValue elements. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::TargetType:
            if(!targetTypeFound) {
                m_target.setLevelName(child->readString());
                targetTypeFound = true;
            } else {
                addNotification(NotificationType::Warning, "Targets element contains multiple TargetType elements. Surplus elements will be ignored.", context);
            }
            break;
        case MatroskaIds::TagTrackUId:
            m_target.tracks().emplace_back(child->readUInteger());
            break;
        case MatroskaIds::TagEditionUId:
            m_target.editions().emplace_back(child->readUInteger());
            break;
        case MatroskaIds::TagChapterUId:
            m_target.chapters().emplace_back(child->readUInteger());
            break;
        case MatroskaIds::TagAttachmentUId:
            m_target.attachments().emplace_back(child->readUInteger());
            break;
        default:
            addNotification(NotificationType::Warning, "Targets element contains unknown element. It will be ignored.", context);
        }
    }
    if(!m_target.level()) {
        m_target.setLevel(50); // default level
    }
}

/*!
 * \class Media::MatroskaTagMaker
 * \brief The MatroskaTagMaker class helps writing Matroska "Tag"-elements storing tag information.
 *
 * An instance can be obtained using the MatroskaTag::prepareMaking() method.
 */

/*!
 * \brief Prepares making the specified \a tag.
 * \sa See MatroskaTag::prepareMaking() for more information.
 */
MatroskaTagMaker::MatroskaTagMaker(MatroskaTag &tag) :
    m_tag(tag)
{
    // calculate size of "Targets" element
    m_targetsSize = 0; // NOT including ID and size
    if(m_tag.target().level() != 50) {
        // size of "TargetTypeValue"
        m_targetsSize += 2 + 1 + EbmlElement::calculateUIntegerLength(m_tag.target().level());
    }
    if(!m_tag.target().levelName().empty()) {
        // size of "TargetType"
        m_targetsSize += 2 + EbmlElement::calculateSizeDenotationLength(m_tag.target().levelName().size()) + m_tag.target().levelName().size();
    }
    for(const auto &v : initializer_list<vector<uint64> >{m_tag.target().tracks(), m_tag.target().editions(), m_tag.target().chapters(), m_tag.target().attachments()}) {
        for(auto uid : v) {
            // size of UID denotation
            m_targetsSize += 2 + 1 + EbmlElement::calculateUIntegerLength(uid);
        }
    }
    m_tagSize = 2 + EbmlElement::calculateSizeDenotationLength(m_targetsSize) + m_targetsSize;
    // calculate size of "SimpleTag" elements
    m_maker.reserve(m_tag.fields().size());
    m_simpleTagsSize = 0; // including ID and size
    for(auto &pair : m_tag.fields()) {
        try {
            m_maker.emplace_back(pair.second.prepareMaking());
            m_simpleTagsSize += m_maker.back().requiredSize();
        } catch(const Failure &) {
            // nothing to do here; notifications will be added anyways
        }
        m_tag.addNotifications(pair.second);
    }
    m_tagSize += m_simpleTagsSize;
    m_totalSize = 2 + EbmlElement::calculateSizeDenotationLength(m_tagSize) + m_tagSize;
}

/*!
 * \brief Saves the tag (specified when constructing the object) to the
 *        specified \a stream (makes a "Tag"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Assumes the data is already validated and thus does NOT
 *                throw Media::Failure or a derived exception.
 */
void MatroskaTagMaker::make(ostream &stream) const
{
    // write header
    char buff[11];
    BE::getBytes(static_cast<uint16>(MatroskaIds::Tag), buff);
    stream.write(buff, 2); // ID
    byte len = EbmlElement::makeSizeDenotation(m_tagSize, buff);
    stream.write(buff, len); // size
    // write "Targets" element
    BE::getBytes(static_cast<uint16>(MatroskaIds::Targets), buff);
    stream.write(buff, 2);
    len = EbmlElement::makeSizeDenotation(m_targetsSize, buff);
    stream.write(buff, len);
    const TagTarget &t = m_tag.target();
    if(t.level() != 50) {
        // write "TargetTypeValue"
        BE::getBytes(static_cast<uint16>(MatroskaIds::TargetTypeValue), buff);
        stream.write(buff, 2);
        len = EbmlElement::makeUInteger(t.level(), buff);
        stream.put(0x80 | len);
        stream.write(buff, len);
    }
    if(!t.levelName().empty()) {
        // write "TargetType"
        BE::getBytes(static_cast<uint16>(MatroskaIds::TargetType), buff);
        stream.write(buff, 2);
        len = EbmlElement::makeSizeDenotation(t.levelName().size(), buff);
        stream.write(buff, len);
        stream.write(t.levelName().c_str(), t.levelName().size());
    }
    // write UIDs
    typedef pair<uint16, vector<uint64> > p;
    for(const auto &pair : initializer_list<p>{p(MatroskaIds::TagTrackUId, t.tracks()), p(MatroskaIds::TagEditionUId, t.editions()), p(MatroskaIds::TagChapterUId, t.chapters()), p(MatroskaIds::TagAttachmentUId, t.attachments())}) {
        if(!pair.second.empty()) {
            BE::getBytes(pair.first, buff);
            for(auto uid : pair.second) {
                len = EbmlElement::makeUInteger(uid, buff + 3);
                *(buff + 2) = 0x80 | len;
                stream.write(buff, 3 + len);

            }
        }
    }
    // write "SimpleTag" elements using maker objects prepared previously
    for(const auto &maker : m_maker) {
        maker.make(stream);
    }
}

} // namespace Media
