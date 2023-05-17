#include "./overall.h"

#include "../abstracttrack.h"
#include "../tag.h"

#include <regex>

/*!
 * \brief Checks "flac/test.flac" (converted from "mtx-test-data/alac/othertest-itunes.m4a" via ffmpeg).
 * \remarks Raw FLAC stream.
 */
void OverallTests::checkFlacTestfile1()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Flac, m_fileInfo.containerFormat());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(1_st, tracks.size());
    for (const auto &track : tracks) {
        CPPUNIT_ASSERT_EQUAL(MediaType::Audio, track->mediaType());
        CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Flac, track->format().general);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(2), track->channelCount());
        CPPUNIT_ASSERT_EQUAL(44100u, track->samplingFrequency());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(16), track->bitsPerSample());
        CPPUNIT_ASSERT_EQUAL(4, track->duration().minutes());
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        // ffmpeg is able to set some tags from the original file (mtx-test-data/alac/othertest-itunes.m4a)
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
        CPPUNIT_ASSERT_EQUAL("Sad Song"s, tags.front()->value(KnownField::Title).toString());
        CPPUNIT_ASSERT_EQUAL("Oasis"s, tags.front()->value(KnownField::Artist).toString());
        CPPUNIT_ASSERT_EQUAL("Don't Go Away (Apple Lossless)"s, tags.front()->value(KnownField::Album).toString());
        CPPUNIT_ASSERT_EQUAL("Alternative & Punk"s, tags.front()->value(KnownField::Genre).toString());
        TESTUTILS_ASSERT_LIKE("encoder", "Lavf.*", tags.front()->value(KnownField::Encoder).toString());
        CPPUNIT_ASSERT_EQUAL("1998"s, tags.front()->value(KnownField::RecordDate).toString());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).isEmpty());
        //CPPUNIT_ASSERT(tags.front()->value(KnownField::Cover).dataSize() == 0x58f3);
        //CPPUNIT_ASSERT(BE::toInt<std::uint64_t>(tags.front()->value(KnownField::Cover).dataPointer()) == 0xFFD8FFE000104A46);
        CPPUNIT_ASSERT_EQUAL(PositionInSet(3, 4), tags.front()->value(KnownField::TrackPosition).toPositionInSet());
        CPPUNIT_ASSERT_EQUAL(PositionInSet(1, 1), tags.front()->value(KnownField::DiskPosition).toPositionInSet());
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
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Ogg, m_fileInfo.containerFormat());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(1_st, tracks.size());
    for (const auto &track : tracks) {
        CPPUNIT_ASSERT_EQUAL(MediaType::Audio, track->mediaType());
        CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Flac, track->format().general);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(2), track->channelCount());
        CPPUNIT_ASSERT_EQUAL(44100u, track->samplingFrequency());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(16), track->bitsPerSample());
        CPPUNIT_ASSERT_EQUAL(4, track->duration().minutes());
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
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
    parseFile(testFilePath("flac/test.flac"), &OverallTests::checkFlacTestfile1);
    parseFile(testFilePath("flac/test.ogg"), &OverallTests::checkFlacTestfile2);
}

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
        makeFile(workingCopyPath("flac/test.flac"), modifyRoutine, &OverallTests::checkFlacTestfile1);
        makeFile(workingCopyPath("flac/test.ogg"), modifyRoutine, &OverallTests::checkFlacTestfile2);
    }
}
