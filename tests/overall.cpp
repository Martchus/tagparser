#include "../mediafileinfo.h"
#include "../tag.h"
#include "../abstracttrack.h"
#include "../mp4/mp4ids.h"
#include "../mpegaudio/mpegaudioframe.h"
#include "../matroska/matroskacontainer.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/tests/testutils.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;
using namespace ConversionUtilities;
using namespace Media;

using namespace CPPUNIT_NS;

enum class TagStatus
{
    Original,
    TestMetaDataPresent,
    Removed
};

class OverallTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(OverallTests);
    CPPUNIT_TEST(testMp4Parsing);
    CPPUNIT_TEST(testMp4Making);
    CPPUNIT_TEST(testMp3Parsing);
    CPPUNIT_TEST(testMp3Making);
    CPPUNIT_TEST(testOggParsing);
    CPPUNIT_TEST(testOggMaking);
    CPPUNIT_TEST(testMkvParsing);
    CPPUNIT_TEST(testMkvMaking);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void parseFile(const string &path, void (OverallTests::* checkRoutine)(void));
    void makeFile(const string &path, void (OverallTests::* modifyRoutine)(void), void (OverallTests::* checkRoutine)(void));

    void checkMkvTestfile1();
    void checkMkvTestfile2();
    void checkMkvTestfile3();
    void checkMkvTestfile4();
    void checkMkvTestfile5();
    void checkMkvTestfile6();
    void checkMkvTestfile7();
    void checkMkvTestfile8();
    void checkMkvTestMetaData();
    void checkMkvPaddingConstraints();

    void setMkvTestMetaData();
    void removeAllTags();

    void testMkvParsing();
    void testMkvMaking();
    void testMp4Parsing();
    void testMp4Making();
    void testMp3Parsing();
    void testMp3Making();
    void testOggParsing();
    void testOggMaking();

private:
    MediaFileInfo m_fileInfo;
    TagValue m_testTitle;
    TagValue m_testComment;
    TagValue m_testAlbum;
    TagValue m_testPartNumber;
    TagValue m_testTotalParts;
    TagStatus m_tagStatus;
    uint16 m_mode;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OverallTests);

/*!
 * \brief Creates some test meta data.
 */
void OverallTests::setUp()
{
    m_testTitle.assignText("some title", TagTextEncoding::Utf8);
    m_testComment.assignText("some comment", TagTextEncoding::Utf8);
    m_testAlbum.assignText("some album", TagTextEncoding::Utf8);
    m_testPartNumber.assignInteger(41);
    m_testTotalParts.assignInteger(61);
}

void OverallTests::tearDown()
{}

/*!
 * \brief Parses the specified file and tests the results using the specified check routine.
 */
void OverallTests::parseFile(const string &path, void (OverallTests::* checkRoutine)(void))
{
    // print current file
    cerr << "- testing " << path << endl;
    // ensure file is open and everything is parsed
    m_fileInfo.setPath(path);
    m_fileInfo.reopen(true);
    m_fileInfo.parseEverything();
    // invoke testroutine to check whether parsing results are correct
    (this->*checkRoutine)();
    m_fileInfo.close();
}

/*!
 * \brief Parses the specified file, modifies it using the specified modify routine, parses the file again and checks the results using the
 *        specified check routine.
 */
void OverallTests::makeFile(const string &path, void (OverallTests::*modifyRoutine)(void), void (OverallTests::*checkRoutine)(void))
{
    // print current file
    cerr << "- testing " << path << endl;
    // ensure file is open and everything is parsed
    m_fileInfo.setPath(path);
    m_fileInfo.reopen(true);
    m_fileInfo.parseEverything();
    // invoke testroutine to do and apply changes
    (this->*modifyRoutine)();
    // apply changes and ensure that the previous parsing results are cleared
    m_fileInfo.applyChanges();
    m_fileInfo.clearParsingResults();
    // reparse the file and invoke testroutine to check whether changings have been applied correctly
    m_fileInfo.parseEverything();
    (this->*checkRoutine)();
    // invoke suitable testroutine to check padding constraints
    switch(m_fileInfo.containerFormat()) {
    case ContainerFormat::Matroska:
        checkMkvPaddingConstraints();
        break;
    default:
        ;
    }
    m_fileInfo.close();
}

/*!
 * \brief Checks "matroska_wave1/test1.mkv".
 */
void OverallTests::checkMkvTestfile1()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Matroska);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 2);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 2422994868:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::MicrosoftMpeg4);
            break;
        case 3653291187:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Mpeg1Audio);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Big Buck Bunny - test 1");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).toString() == "Matroska Validation File1, basic MPEG4.2 and MP3 with only SimpleBlock");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Year).toString() == "2010");
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "matroska_wave1/test2.mkv".
 */
void OverallTests::checkMkvTestfile2()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Matroska);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 2);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1863976627:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Avc);
            CPPUNIT_ASSERT(track->displaySize() == Size(1354, 576));
            break;
        case 3134325680:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Aac);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Elephant Dream - test 2");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).toString() == "Matroska Validation File 2, 100,000 timecode scale, odd aspect ratio, and CRC-32. Codecs are AVC and AAC");
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "matroska_wave1/test3.mkv".
 */
void OverallTests::checkMkvTestfile3()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Matroska);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 2);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 3927961528:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Avc);
            CPPUNIT_ASSERT(track->displaySize() == Size(1024, 576));
            break;
        case 3391885737:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Mpeg1Audio);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Elephant Dream - test 3");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).toString() == "Matroska Validation File 3, header stripping on the video track and no SimpleBlock");
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "matroska_wave1/test4.mkv".
 */
void OverallTests::checkMkvTestfile4()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Matroska);
    // this file is messed up, it should contain tags but it doesn't
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 2);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1368622492:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Theora);
            CPPUNIT_ASSERT(track->displaySize() == Size(1280, 720));
            break;
        case 3171450505:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Vorbis);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            CPPUNIT_ASSERT(track->channelCount() == 2);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    }
}

/*!
 * \brief Checks "matroska_wave1/test5.mkv".
 */
void OverallTests::checkMkvTestfile5()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Matroska);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 11);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1258329745:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Avc);
            CPPUNIT_ASSERT(track->displaySize() == Size(1024, 576));
            break;
        case 3452711582:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Aac);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            CPPUNIT_ASSERT(track->channelConfig() == Mpeg4ChannelConfigs::FrontLeftFrontRight);
            break;
        case 3554194305:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Text);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::TextSubtitle);
            CPPUNIT_ASSERT(track->language() == "ger");
            break;
        default:
            ;
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Big Buck Bunny - test 8");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).toString() == "Matroska Validation File 8, secondary audio commentary track, misc subtitle tracks");
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "matroska_wave1/test6.mkv".
 */
void OverallTests::checkMkvTestfile6()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Matroska);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 2);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 2422994868:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::MicrosoftMpeg4);
            CPPUNIT_ASSERT(track->pixelSize() == Size(854, 480));
            break;
        case 3653291187:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Mpeg1Audio);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            CPPUNIT_ASSERT(track->channelConfig() == static_cast<byte>(MpegChannelMode::Stereo));
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Big Buck Bunny - test 6");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).toString() == "Matroska Validation File 6, random length to code the size of Clusters and Blocks, no Cues for seeking");
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "matroska_wave1/test7.mkv".
 */
void OverallTests::checkMkvTestfile7()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Matroska);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 2);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 568001708:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Avc);
            CPPUNIT_ASSERT(track->pixelSize() == Size(1024, 576));
            CPPUNIT_ASSERT(!strcmp(track->chromaFormat(), "YUV 4:2:0"));
            break;
        case 2088735154:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Aac);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            CPPUNIT_ASSERT(track->channelConfig() == Mpeg4ChannelConfigs::FrontLeftFrontRight);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Big Buck Bunny - test 7");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).toString() == "Matroska Validation File 7, junk elements are present at the beggining or end of clusters, the parser should skip it. There is also a damaged element at 451418");
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "matroska_wave1/test8.mkv".
 */
void OverallTests::checkMkvTestfile8()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Matroska);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 2);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 568001708:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Avc);
            CPPUNIT_ASSERT(track->pixelSize() == Size(1024, 576));
            CPPUNIT_ASSERT(!strcmp(track->chromaFormat(), "YUV 4:2:0"));
            break;
        case 2088735154:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Aac);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            CPPUNIT_ASSERT(track->channelConfig() == Mpeg4ChannelConfigs::FrontLeftFrontRight);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Big Buck Bunny - test 8");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).toString() == "Matroska Validation File 8, audio missing between timecodes 6.019s and 6.360s");
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks whether test meta data for Matroska files has been applied correctly.
 */
void OverallTests::checkMkvTestMetaData()
{
    // check tags
    const auto tags = m_fileInfo.tags();
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tags.size() == 2);
    CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == m_testTitle.toString());
    CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
    CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).toString() == m_testComment.toString());
    CPPUNIT_ASSERT(tags[1]->target().level() == 30);
    CPPUNIT_ASSERT(tags[1]->target().tracks().at(0) == tracks.at(0)->id());
    CPPUNIT_ASSERT(tags[1]->value(KnownField::Album).toString() == m_testAlbum.toString());
    CPPUNIT_ASSERT(tags[1]->value(KnownField::PartNumber).toInteger() == m_testPartNumber.toInteger());
    CPPUNIT_ASSERT(tags[1]->value(KnownField::TotalParts).toInteger() == m_testTotalParts.toInteger());

    // check attachments
    const auto attachments = m_fileInfo.attachments();
    CPPUNIT_ASSERT(attachments.size() == 1);
    CPPUNIT_ASSERT(attachments[0]->mimeType() == "image/png");
    CPPUNIT_ASSERT(attachments[0]->name() == "cover.jpg");
    CPPUNIT_ASSERT(attachments[0]->data() != nullptr);
    CPPUNIT_ASSERT(attachments[0]->data()->size() == 11964);
    // TODO: validate actual data
}

/*!
 * \brief Checks whether padding constraints are met.
 */
void OverallTests::checkMkvPaddingConstraints()
{
    if((m_mode & 0x8) && (m_mode & 0x1)) {
        CPPUNIT_ASSERT(m_fileInfo.paddingSize() == 4096);
    } else if(m_mode & 0x8) {
        CPPUNIT_ASSERT(m_fileInfo.paddingSize() >= 1024);
        CPPUNIT_ASSERT(m_fileInfo.paddingSize() <= (4096 + 1024));
        // TODO: check tag/index position and rewriting behaviour
    }
}

/*!
 * \brief Creates a tag targeting the first track with some test meta data.
 */
void OverallTests::setMkvTestMetaData()
{
    // change the present tag
    if(!m_fileInfo.container()->tagCount()) {
        // test4.mkv has no tag, so one must be created first
        m_fileInfo.container()->createTag(TagTarget(50));
    }
    Tag *firstTag = m_fileInfo.tags().at(0);
    firstTag->setValue(KnownField::Title, m_testTitle);
    firstTag->setValue(KnownField::Comment, m_testComment);
    // add an additional tag targeting the first track
    TagTarget::IdContainerType trackIds;
    trackIds.emplace_back(m_fileInfo.tracks().at(0)->id());
    if(Tag *newTag = m_fileInfo.container()->createTag(TagTarget(30, trackIds))) {
        newTag->setValue(KnownField::Album, m_testAlbum);
        newTag->setValue(KnownField::PartNumber, m_testPartNumber);
        newTag->setValue(KnownField::TotalParts, m_testTotalParts);
    } else {
        CPPUNIT_FAIL("can not create tag");
    }
    // assign an attachment
    if(AbstractAttachment *attachment = m_fileInfo.container()->createAttachment()) {
        attachment->setFile(TestUtilities::testFilePath("matroska_wave1/logo3_256x256.png"));
        attachment->setMimeType("image/png");
        attachment->setName("cover.jpg");
    } else {
        CPPUNIT_FAIL("can not create attachment");
    }
}

/*!
 * \brief Removes all tags.
 */
void OverallTests::removeAllTags()
{
    m_fileInfo.removeAllTags();
}

/*!
 * \brief Tests the Matroska parser via MediaFileInfo.
 */
void OverallTests::testMkvParsing()
{
    cerr << endl << "Matroska parser" << endl;
    m_fileInfo.setForceFullParse(false);
    m_tagStatus = TagStatus::Original;
    parseFile(TestUtilities::testFilePath("matroska_wave1/test1.mkv"), &OverallTests::checkMkvTestfile1);
    parseFile(TestUtilities::testFilePath("matroska_wave1/test2.mkv"), &OverallTests::checkMkvTestfile2);
    parseFile(TestUtilities::testFilePath("matroska_wave1/test3.mkv"), &OverallTests::checkMkvTestfile3);
    parseFile(TestUtilities::testFilePath("matroska_wave1/test4.mkv"), &OverallTests::checkMkvTestfile4);
    parseFile(TestUtilities::testFilePath("matroska_wave1/test5.mkv"), &OverallTests::checkMkvTestfile5);
    parseFile(TestUtilities::testFilePath("matroska_wave1/test6.mkv"), &OverallTests::checkMkvTestfile6);
    parseFile(TestUtilities::testFilePath("matroska_wave1/test7.mkv"), &OverallTests::checkMkvTestfile7);
    parseFile(TestUtilities::testFilePath("matroska_wave1/test8.mkv"), &OverallTests::checkMkvTestfile8);
}

/*!
 * \brief Tests the Matroska maker via MediaFileInfo.
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testMkvMaking()
{
    // full parse is required to determine padding
    m_fileInfo.setForceFullParse(true);

    // do the test under different conditions
    for(m_mode = 0; m_mode != 0x100; ++m_mode) {
        // setup test conditions
        m_fileInfo.setForceRewrite(m_mode & 0x1);
        if(m_mode & 0x2) {
            m_fileInfo.setTagPosition(ElementPosition::Keep);
        } else {
            m_fileInfo.setTagPosition(m_mode & 0x40 ? ElementPosition::BeforeData : ElementPosition::AfterData);
        }
        if(m_mode & 0x4) {
            if(m_mode & 0x80) {
                continue;
            }
            m_fileInfo.setIndexPosition(ElementPosition::Keep);
        } else {
            m_fileInfo.setTagPosition(m_mode & 0x80 ? ElementPosition::BeforeData : ElementPosition::AfterData);
        }
        m_fileInfo.setPreferredPadding(m_mode & 0x8 ? 4096 : 0);
        m_fileInfo.setMinPadding(m_mode & 0x8 ? 1024 : 0);
        m_fileInfo.setMaxPadding(m_mode & 0x8 ? (4096 + 1024) : 0);
        m_fileInfo.setForceTagPosition(m_mode & 0x10);
        m_fileInfo.setForceIndexPosition(m_mode & 0x20);

        // print test conditions
        list<string> testConditions;
        if(m_mode & 0x1) {
            testConditions.emplace_back("forcing rewrite");
        }
        if(m_mode & 0x2) {
            if(m_mode & 0x40) {
                testConditions.emplace_back("removing tag");
            } else {
                testConditions.emplace_back("keeping tag position");
            }
        } else if(m_mode & 0x40) {
            testConditions.emplace_back("tags before data");
        } else {
            testConditions.emplace_back("tags after data");
        }
        if(m_mode & 0x8) {
            testConditions.emplace_back("keeping index position");
        } else if(m_mode & 0x80) {
            testConditions.emplace_back("index before data");
        } else {
            testConditions.emplace_back("index after data");
        }
        if(m_mode & 0x8) {
            testConditions.emplace_back("padding constraints");
        }
        if(m_mode & 0x10) {
            testConditions.emplace_back("forcing tag position");
        }
        if(m_mode & 0x20) {
            testConditions.emplace_back("forcing index position");
        }
        cerr << endl << "Matroska maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        bool remove = (m_mode & 0x40) && (m_mode & 0x2);
        m_tagStatus = remove ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = remove ? &OverallTests::removeAllTags : &OverallTests::setMkvTestMetaData;
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test1.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile1);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test2.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile2);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test3.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile3);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test4.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile4);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test5.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile5);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test6.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile6);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test7.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile7);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test8.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile8);
    }
}

/*!
 * \brief Tests the MP4 parser via MediaFileInfo.
 */
void OverallTests::testMp4Parsing()
{

}

/*!
 * \brief Tests the MP4 maker via MediaFileInfo.
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testMp4Making()
{

}

/*!
 * \brief Tests the MP3 parser via MediaFileInfo.
 */
void OverallTests::testMp3Parsing()
{

}

/*!
 * \brief Tests the MP3 maker via MediaFileInfo.
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testMp3Making()
{

}

/*!
 * \brief Tests the Ogg parser via MediaFileInfo.
 */
void OverallTests::testOggParsing()
{

}

/*!
 * \brief Tests the Ogg maker via MediaFileInfo.
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testOggMaking()
{

}
