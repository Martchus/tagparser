#include "./helper.h"
#include "./overall.h"

#include "../abstracttrack.h"
#include "../tag.h"
#include "../vorbis/vorbiscomment.h"
#include "../vorbis/vorbiscommentfield.h"
#include "../vorbis/vorbiscommentids.h"

#include <c++utilities/io/misc.h>

#include <functional>

using namespace CppUtilities;

/*!
 * \brief Checks "mtx-test-data/ogg/qt4dance_medium.ogg"
 */
void OverallTests::checkOggTestfile1()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Ogg, m_fileInfo.containerFormat());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
    for (const auto &track : tracks) {
        switch (track->id()) {
        case 897658443:
            CPPUNIT_ASSERT_EQUAL(MediaType::Video, track->mediaType());
            CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Theora, track->format().general);
            break;
        case 1755441791:
            CPPUNIT_ASSERT_EQUAL(MediaType::Audio, track->mediaType());
            CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Vorbis, track->format().general);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(2), track->channelCount());
            CPPUNIT_ASSERT_EQUAL(44100u, track->samplingFrequency());
            CPPUNIT_ASSERT_EQUAL(4, track->duration().minutes());
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(m_fileInfo.hasAnyTag());
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
        CPPUNIT_ASSERT_EQUAL("ffmpeg2theora 0.13"s, tags.front()->value(KnownField::Encoder).toString());
        CPPUNIT_ASSERT_EQUAL(std::vector<std::uint64_t>{ 0x68a1ea7f }, tags.front()->target().tracks());
        // Theora tags are currently not supported and hence only the Vorbis comment is
        // taken into account here
        break;
    case TagStatus::TestMetaDataPresent:
        checkOggTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }

    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "mtx-test-data/opus/v-opus.ogg"
 */
void OverallTests::checkOggTestfile2()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Ogg, m_fileInfo.containerFormat());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(1_st, tracks.size());
    for (const auto &track : tracks) {
        switch (track->id()) {
        case 1375632254:
            CPPUNIT_ASSERT_EQUAL(MediaType::Audio, track->mediaType());
            CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Opus, track->format().general);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(2), track->channelCount());
            CPPUNIT_ASSERT_EQUAL(48000u, track->samplingFrequency());
            CPPUNIT_ASSERT_EQUAL(1, track->duration().minutes());
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(m_fileInfo.hasAnyTag());
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
        CPPUNIT_ASSERT_EQUAL("opusenc from opus-tools 0.1.6"s, tags.front()->value(KnownField::Encoder).toString());
        break;
    case TagStatus::TestMetaDataPresent:
        checkOggTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }

    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "ogg/noise-*.opus".
 */
void OverallTests::checkOggTestfile3()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Ogg, m_fileInfo.containerFormat());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(1_st, tracks.size());
    for (const auto &track : tracks) {
        switch (track->id()) {
        case 1843569915:
            CPPUNIT_ASSERT_EQUAL(MediaType::Audio, track->mediaType());
            CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Opus, track->format().general);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(2), track->channelCount());
            CPPUNIT_ASSERT_EQUAL(48000u, track->samplingFrequency());
            CPPUNIT_ASSERT_EQUAL(TimeSpan::fromSeconds(19.461), track->duration());
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(m_fileInfo.hasAnyTag());
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
        CPPUNIT_ASSERT_EQUAL("Lavf58.76.100"s, tags.front()->value(KnownField::Encoder).toString());
        CPPUNIT_ASSERT_EQUAL("eng"s, tags.front()->value(KnownField::Language).toString());
        [[fallthrough]];
    case TagStatus::TestMetaDataPresent:
        checkOggTestMetaDataCover();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }

    if (m_tagStatus != TagStatus::Original) {
        CPPUNIT_ASSERT_MESSAGE("no warnings for non-broken file", m_diag.level() <= DiagLevel::Information);
        return;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("warning present", DiagLevel::Warning, m_diag.level());
    for (const auto &msg : m_diag) {
        if (msg.level() == DiagLevel::Warning) {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("warning due to broken segment termination", "3 bytes left in last segment."s, msg.message());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("warning relates to Vorbis comment", "parsing Vorbis comment"s, msg.context());
            break;
        }
    }
}

/*!
 * \brief Checks whether test meta data for OGG files has been applied correctly.
 */
void OverallTests::checkOggTestMetaData()
{
    // check whether a tag is assigned
    const auto tags = m_fileInfo.tags();
    const auto *const tag = m_fileInfo.vorbisComment();
    CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
    CPPUNIT_ASSERT(tag != nullptr);

    // check test meta data
    CPPUNIT_ASSERT_EQUAL(m_testTitle, tag->value(KnownField::Title));
    CPPUNIT_ASSERT_EQUAL(m_testComment.toString(), tag->value(KnownField::Comment).toString()); // loss of description is ok
    CPPUNIT_ASSERT_EQUAL(m_testAlbum, tag->value(KnownField::Album));
    CPPUNIT_ASSERT_EQUAL(m_preservedMetaData.front(), tag->value(KnownField::Artist));
    CPPUNIT_ASSERT_EQUAL(m_testPosition, tag->value(KnownField::TrackPosition));
    CPPUNIT_ASSERT_EQUAL(m_testPosition, tag->value(KnownField::DiskPosition));
    // TODO: check more fields
    m_preservedMetaData.pop();
}

void OverallTests::checkOggTestMetaDataCover()
{
    // check whether a tag is assigned
    const auto tags = m_fileInfo.tags();
    const auto *const tag = m_fileInfo.vorbisComment();
    CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
    CPPUNIT_ASSERT(tag != nullptr);

    const auto expectedCoverData = readFile(testFilePath("ogg/example-cover.png"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("expected cover assigned", std::string_view(expectedCoverData), tag->value(KnownField::Cover).data());
}

void OverallTests::setOggTestMetaData()
{
    // ensure a tag exists
    auto *const tag = m_fileInfo.createVorbisComment();

    // assign test meta data
    tag->setValue(KnownField::Title, m_testTitle);
    tag->setValue(KnownField::Comment, m_testComment);
    tag->setValue(KnownField::Album, m_testAlbum);
    m_preservedMetaData.push(tag->value(KnownField::Artist));
    tag->setValue(KnownField::TrackPosition, m_testPosition);
    tag->setValue(KnownField::DiskPosition, m_testPosition);
    // TODO: set more fields
}

void OverallTests::setOggTestMetaDataCover()
{
    auto *const tag = m_fileInfo.createVorbisComment();
    const auto cover = readFile(testFilePath("ogg/example-cover.png"));
    tag->setValue(KnownField::Cover, TagValue(cover.data(), cover.size(), TagDataType::Picture));
}

/*!
 * \brief Tests the Ogg parser via MediaFileInfo.
 * \remarks FLAC in Ogg is tested in testFlacParsing().
 */
void OverallTests::testOggParsing()
{
    cerr << endl << "OGG parser" << endl;
    m_fileInfo.setForceFullParse(false);
    m_tagStatus = TagStatus::Original;
    parseFile(testFilePath("mtx-test-data/ogg/qt4dance_medium.ogg"), &OverallTests::checkOggTestfile1);
    parseFile(testFilePath("mtx-test-data/opus/v-opus.ogg"), &OverallTests::checkOggTestfile2);
    parseFile(testFilePath("ogg/noise-broken-segment-termination.opus"), &OverallTests::checkOggTestfile3);
}

/*!
 * \brief Tests the Ogg maker via MediaFileInfo.
 * \remarks
 *  - Relies on the parser to check results.
 *  - FLAC in Ogg is tested in testFlacMaking().
 */
void OverallTests::testOggMaking()
{
    // full parse is required to determine padding
    m_fileInfo.setForceFullParse(true);

    // do the test under different conditions
    for (m_mode = 0; m_mode != 0x2; ++m_mode) {
        using namespace SimpleTestFlags;

        // no need to setup test conditions because the Ogg maker
        // doesn't take those settings into account (currently)

        // print test conditions
        list<string> testConditions;
        if (m_mode & RemoveTag) {
            testConditions.emplace_back("removing tag");
        } else {
            testConditions.emplace_back("modifying tag");
        }
        cerr << endl << "OGG maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        m_tagStatus = (m_mode & RemoveTag) ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        const auto modifyRoutine = (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setOggTestMetaData;
        const auto modifyRoutineCover = (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setOggTestMetaDataCover;
        makeFile(workingCopyPath("mtx-test-data/ogg/qt4dance_medium.ogg"), modifyRoutine, &OverallTests::checkOggTestfile1);
        makeFile(workingCopyPath("mtx-test-data/opus/v-opus.ogg"), modifyRoutine, &OverallTests::checkOggTestfile2);
        makeFile(workingCopyPath("ogg/noise-without-cover.opus"), modifyRoutineCover, &OverallTests::checkOggTestfile3);
    }
}

/*!
 * \brief Tests the Vorbis Comment specifc handling of certain fields done in VorbisComment::convertTotalFields().
 */
void OverallTests::testVorbisCommentFieldHandling()
{
    const auto context = std::string();
    const auto trackNumberFieldId = std::string(VorbisCommentIds::trackNumber());
    const auto trackTotalFieldId = std::string(VorbisCommentIds::trackTotal());
    const auto diskNumberFieldId = std::string(VorbisCommentIds::diskNumber());
    const auto diskTotalFieldId = std::string(VorbisCommentIds::diskTotal());

    auto diag = Diagnostics();
    auto vc = VorbisComment();
    auto trackNumber = VorbisCommentField(trackNumberFieldId, TagValue(5));
    auto trackTotal = VorbisCommentField(trackTotalFieldId, TagValue(20));
    auto &fields = vc.fields();
    fields.insert(std::make_pair(trackNumberFieldId, std::move(trackNumber)));
    fields.insert(std::make_pair(trackTotalFieldId, std::move(trackTotal)));
    vc.convertTotalFields(context, diag);

    const auto convertedValues = vc.values(trackNumberFieldId);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the two fileds have been combined into one", 1_st, fields.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("there is exactly one track number value", 1_st, convertedValues.size());
    const auto convertedTrackNumber = convertedValues.front()->toPositionInSet();
    CPPUNIT_ASSERT_EQUAL(PositionInSet(5, 20), convertedTrackNumber);
    CPPUNIT_ASSERT_EQUAL(0_st, diag.size());

    auto diskNumber = VorbisCommentField(diskNumberFieldId, TagValue("invalid pos"));
    auto diskTotal = VorbisCommentField(diskTotalFieldId, TagValue("invalid total"));
    auto diskTotal2 = VorbisCommentField(diskTotalFieldId, TagValue(42));
    fields.insert(std::make_pair(diskNumberFieldId, std::move(diskNumber)));
    fields.insert(std::make_pair(diskTotalFieldId, std::move(diskTotal)));
    fields.insert(std::make_pair(diskTotalFieldId, std::move(diskTotal2)));
    vc.convertTotalFields(context, diag);

    const auto newDiskNumberValues = vc.values(diskNumberFieldId);
    const auto newDiskTotalValues = vc.values(diskTotalFieldId);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("invalid fields have not been combined", 4_st, fields.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("invalid disk position has been preserved and valid disk total converted", 2_st, newDiskNumberValues.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("invalid disk total has been preserved", 1_st, newDiskTotalValues.size());
    const auto preservedDiskNumber = newDiskNumberValues[0]->toString();
    const auto convertedDiskTotal = newDiskNumberValues[1]->toPositionInSet();
    const auto preservedDiskTotal = newDiskTotalValues[0]->toString();
    CPPUNIT_ASSERT_EQUAL("invalid pos"s, preservedDiskNumber);
    CPPUNIT_ASSERT_EQUAL(PositionInSet(0, 42), convertedDiskTotal);
    CPPUNIT_ASSERT_EQUAL("invalid total"s, preservedDiskTotal);
    CPPUNIT_ASSERT_EQUAL(3_st, diag.size());
}
