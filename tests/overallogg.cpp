#include "./helper.h"
#include "./overall.h"

#include "../abstracttrack.h"
#include "../tag.h"
#include "../vorbis/vorbiscomment.h"

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
        void (OverallTests::*modifyRoutine)(void) = (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setOggTestMetaData;
        makeFile(workingCopyPath("mtx-test-data/ogg/qt4dance_medium.ogg"), modifyRoutine, &OverallTests::checkOggTestfile1);
        makeFile(workingCopyPath("mtx-test-data/opus/v-opus.ogg"), modifyRoutine, &OverallTests::checkOggTestfile2);
    }
}
