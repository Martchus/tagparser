#include "./helper.h"
#include "./overall.h"

#include "../tag.h"
#include "../abstracttrack.h"
#include "../vorbis/vorbiscomment.h"

/*!
 * \brief Checks "mtx-test-data/ogg/qt4dance_medium.ogg"
 */
void OverallTests::checkOggTestfile1()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Ogg);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 2);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 897658443:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Theora);
            break;
        case 1755441791:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Vorbis);
            CPPUNIT_ASSERT(track->channelCount() == 2);
            CPPUNIT_ASSERT(track->samplingFrequency() == 44100);
            CPPUNIT_ASSERT(track->duration().minutes() == 4);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Encoder).toString() == "ffmpeg2theora 0.13");
        // Theora tags are currently not supported and hence only the Vorbis comment is
        // taken into account here
        break;
    case TagStatus::TestMetaDataPresent:
        checkOggTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tracks.size());
    }
}

/*!
 * \brief Checks "mtx-test-data/opus/v-opus.ogg"
 */
void OverallTests::checkOggTestfile2()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Ogg);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1375632254:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Opus);
            CPPUNIT_ASSERT(track->channelCount() == 2);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            CPPUNIT_ASSERT(track->duration().minutes() == 1);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Encoder).toString() == "opusenc from opus-tools 0.1.6");
        break;
    case TagStatus::TestMetaDataPresent:
        checkOggTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tracks.size());
    }
}

/*!
 * \brief Checks whether test meta data for OGG files has been applied correctly.
 */
void OverallTests::checkOggTestMetaData()
{
    // check whether a tag is assigned
    const auto tags = m_fileInfo.tags();
    VorbisComment *tag = m_fileInfo.vorbisComment();
    CPPUNIT_ASSERT(tags.size() == 1);
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
    VorbisComment *tag = m_fileInfo.createVorbisComment();

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
    parseFile(TestUtilities::testFilePath("mtx-test-data/ogg/qt4dance_medium.ogg"), &OverallTests::checkOggTestfile1);
    parseFile(TestUtilities::testFilePath("mtx-test-data/opus/v-opus.ogg"), &OverallTests::checkOggTestfile2);
}

#ifdef PLATFORM_UNIX
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
    for(m_mode = 0; m_mode != 0x2; ++m_mode) {
        using namespace SimpleTestFlags;

        // no need to setup test conditions because the Ogg maker
        // doesn't take those settings into account (currently)

        // print test conditions
        list<string> testConditions;
        if(m_mode & RemoveTag) {
            testConditions.emplace_back("removing tag");
        } else {
            testConditions.emplace_back("modifying tag");
        }
        cerr << endl << "OGG maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        m_tagStatus = (m_mode & RemoveTag) ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setOggTestMetaData;
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/ogg/qt4dance_medium.ogg"), modifyRoutine, &OverallTests::checkOggTestfile1);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/opus/v-opus.ogg"), modifyRoutine, &OverallTests::checkOggTestfile2);
    }
}
#endif
