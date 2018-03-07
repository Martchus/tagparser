#include "./overall.h"

#include "../abstracttrack.h"
#include "../tag.h"

/*!
 * \brief Checks "flac/test.flac" (converted from "mtx-test-data/alac/othertest-itunes.m4a" via ffmpeg).
 * \remarks Raw FLAC stream.
 */
void OverallTests::checkFlacTestfile1()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Flac);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for (const auto &track : tracks) {
        CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
        CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Flac);
        CPPUNIT_ASSERT(track->channelCount() == 2);
        CPPUNIT_ASSERT(track->samplingFrequency() == 44100);
        CPPUNIT_ASSERT(track->bitsPerSample() == 16);
        CPPUNIT_ASSERT(track->duration().minutes() == 4);
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        // ffmpeg is able to set some tags from the original file (mtx-test-data/alac/othertest-itunes.m4a)
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Sad Song");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).toString() == "Oasis");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Album).toString() == "Don't Go Away (Apple Lossless)");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Genre).toString() == "Alternative & Punk");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Encoder).toString() == "Lavf57.25.100");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Year).toString() == "1998");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).isEmpty());
        //CPPUNIT_ASSERT(tags.front()->value(KnownField::Cover).dataSize() == 0x58f3);
        //CPPUNIT_ASSERT(BE::toUInt64(tags.front()->value(KnownField::Cover).dataPointer()) == 0xFFD8FFE000104A46);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::TrackPosition).toPositionInSet() == PositionInSet(3, 4));
        CPPUNIT_ASSERT(tags.front()->value(KnownField::DiskPosition).toPositionInSet() == PositionInSet(1, 1));
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
 * \brief Checks "flac/test.ogg" (converted from "flac/test.flac" via ffmpeg).
 * \remarks FLAC in Ogg.
 */
void OverallTests::checkFlacTestfile2()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Ogg);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for (const auto &track : tracks) {
        CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
        CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Flac);
        CPPUNIT_ASSERT(track->channelCount() == 2);
        CPPUNIT_ASSERT(track->samplingFrequency() == 44100);
        CPPUNIT_ASSERT(track->bitsPerSample() == 16);
        CPPUNIT_ASSERT(track->duration().minutes() == 4);
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        break;
    case TagStatus::TestMetaDataPresent:
        checkOggTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }

    // check for unexpected critical notifications or warnings
    if (m_tagStatus == TagStatus::Removed) {
        bool gotMessageAboutMissingVorbisComment = false;
        for (const auto &msg : m_diag) {
            if (msg.level() == DiagLevel::Critical) {
                CPPUNIT_ASSERT_EQUAL("OGG page after FLAC-to-Ogg mapping header doesn't contain Vorbis comment."s, msg.message());
                gotMessageAboutMissingVorbisComment = true;
                continue;
            }
            CPPUNIT_ASSERT(msg.level() <= DiagLevel::Information);
        }
        CPPUNIT_ASSERT(gotMessageAboutMissingVorbisComment);
    } else {
        CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
    }
}

/*!
 * \brief Tests the FLAC parser via MediaFileInfo.
 */
void OverallTests::testFlacParsing()
{
    cerr << endl << "FLAC parser" << endl;
    m_fileInfo.setForceFullParse(false);
    m_tagStatus = TagStatus::Original;
    parseFile(TestUtilities::testFilePath("flac/test.flac"), &OverallTests::checkFlacTestfile1);
    parseFile(TestUtilities::testFilePath("flac/test.ogg"), &OverallTests::checkFlacTestfile2);
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Tests the FLAC maker via MediaFileInfo.
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testFlacMaking()
{
    // full parse is required to determine padding
    m_fileInfo.setForceFullParse(true);

    // do the test under different conditions
    for (m_mode = 0; m_mode != 0x2; ++m_mode) {
        using namespace SimpleTestFlags;

        // TODO: setup test conditions

        // print test conditions
        list<string> testConditions;
        if (m_mode & RemoveTag) {
            testConditions.emplace_back("removing tag");
        } else {
            testConditions.emplace_back("modifying tag");
        }
        cerr << endl << "FLAC maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        m_tagStatus = (m_mode & RemoveTag) ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setOggTestMetaData;
        makeFile(TestUtilities::workingCopyPath("flac/test.flac"), modifyRoutine, &OverallTests::checkFlacTestfile1);
        makeFile(TestUtilities::workingCopyPath("flac/test.ogg"), modifyRoutine, &OverallTests::checkFlacTestfile2);
    }
}
#endif
