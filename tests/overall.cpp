#include "../mediafileinfo.h"
#include "../tag.h"
#include "../abstracttrack.h"
#include "../mp4/mp4ids.h"
#include "../mpegaudio/mpegaudioframe.h"
#include "../matroska/matroskacontainer.h"
#include "../id3/id3v1tag.h"
#include "../id3/id3v2tag.h"
#include "../mp4/mp4tag.h"
#include "../vorbis/vorbiscomment.h"

#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/tests/testutils.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <cstring>
#include <cstdio>

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

/*!
 * \brief The OverallTests class tests reading and writing tags and parsing technical information
 *        for all supported container/tag formats.
 */
class OverallTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(OverallTests);
    CPPUNIT_TEST(testMp4Parsing);
    CPPUNIT_TEST(testMp3Parsing);
    CPPUNIT_TEST(testOggParsing);
    CPPUNIT_TEST(testFlacParsing);
    CPPUNIT_TEST(testMkvParsing);
#ifdef PLATFORM_UNIX
    CPPUNIT_TEST(testMp4Making);
    CPPUNIT_TEST(testMp3Making);
    CPPUNIT_TEST(testOggMaking);
    CPPUNIT_TEST(testFlacMaking);
    CPPUNIT_TEST(testMkvMaking);
#endif
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

    void checkMp4Testfile1();
    void checkMp4Testfile2();
    void checkMp4Testfile3();
    void checkMp4Testfile4();
    void checkMp4Testfile5();
    void checkMp4TestMetaData();
    void checkMp4PaddingConstraints();

    void checkMp3Testfile1();
    void checkMp3TestMetaData();
    void checkMp3PaddingConstraints();

    void checkOggTestfile1();
    void checkOggTestfile2();
    void checkOggTestMetaData();

    void checkFlacTestfile1();
    void checkFlacTestfile2();

    void setMkvTestMetaData();
    void setMp4TestMetaData();
    void setMp3TestMetaData();
    void setOggTestMetaData();
    void removeAllTags();

    void testMkvParsing();
    void testMp4Parsing();
    void testMp3Parsing();
    void testOggParsing();
    void testFlacParsing();
#ifdef PLATFORM_UNIX
    void testMkvMaking();
    void testMp4Making();
    void testMp3Making();
    void testOggMaking();
    void testFlacMaking();
#endif

private:
    MediaFileInfo m_fileInfo;
    TagValue m_testTitle;
    TagValue m_testComment;
    TagValue m_testAlbum;
    TagValue m_testPartNumber;
    TagValue m_testTotalParts;
    TagValue m_testPosition;
    queue<TagValue> m_preservedMetaData;
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
    m_testComment.assignText("some cómment", TagTextEncoding::Utf8);
    m_testAlbum.assignText("some album", TagTextEncoding::Utf8);
    m_testPartNumber.assignInteger(41);
    m_testTotalParts.assignInteger(61);
    m_testPosition.assignPosition(PositionInSet(41, 61));
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
    case ContainerFormat::Mp4:
        checkMp4PaddingConstraints();
        break;
    case ContainerFormat::MpegAudioFrames:
    case ContainerFormat::Adts:
        checkMp3PaddingConstraints();
        break;
    default:
        ;
    }
    // close and remove file and backup files
    m_fileInfo.close();
    remove(path.c_str());
    remove((path + ".bak").c_str());
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
 * \brief Checks "mtx-test-data/mp4/10-DanseMacabreOp.40.m4a"
 */
void OverallTests::checkMp4Testfile1()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Mp4);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Aac);
            CPPUNIT_ASSERT(track->creationTime().year() == 2012);
            CPPUNIT_ASSERT(track->samplingFrequency() == 44100);
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
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Danse Macabre, Op.40");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).toString() == "Saint-Saëns");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Genre).toString() == "Classical");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Encoder).toString() == "qaac 1.32, CoreAudioToolbox 7.9.7.3, AAC-LC Encoder, TVBR q63, Quality 96");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::TrackPosition).toPositionInSet().position() == 10);
        break;
    case TagStatus::TestMetaDataPresent:
        checkMp4TestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "mtx-test-data/mp4/1080p-DTS-HD-7.1.mp4"
 */
void OverallTests::checkMp4Testfile2()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Mp4);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 5);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Avc);
            CPPUNIT_ASSERT(track->format().sub == SubFormats::AvcHighProfile);
            CPPUNIT_ASSERT(track->version() == 4);
            CPPUNIT_ASSERT(track->creationTime().year() == 2013);
            CPPUNIT_ASSERT(track->pixelSize() == Size(1920, 750));
            break;
        case 2:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Aac);
            CPPUNIT_ASSERT(track->format().sub == SubFormats::AacMpeg4LowComplexityProfile);
            CPPUNIT_ASSERT(!(track->format().extension & ExtensionFormats::SpectralBandReplication));
            CPPUNIT_ASSERT(!(track->format().extension & ExtensionFormats::ParametricStereo));
            CPPUNIT_ASSERT(track->language() == "eng");
            CPPUNIT_ASSERT(track->creationTime().year() == 2013);
            CPPUNIT_ASSERT(track->samplingFrequency() == 48000);
            CPPUNIT_ASSERT(track->channelConfig() == Mpeg4ChannelConfigs::FrontLeftFrontRight);
            break;
        case 3:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Ac3);
            CPPUNIT_ASSERT(track->language() == "eng");
            CPPUNIT_ASSERT(track->creationTime().year() == 2013);
            break;
        case 4:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::DtsHd);
            CPPUNIT_ASSERT(track->language() == "eng");
            CPPUNIT_ASSERT(track->creationTime().year() == 2013);
            break;
        case 6:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Text);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::TimedText);
            CPPUNIT_ASSERT(track->creationTime().year() == 2013);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 0);
        break;
    case TagStatus::TestMetaDataPresent:
        checkMp4TestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "mtx-test-data/mp4/dash/dragon-age-inquisition-H1LkM6IVlm4-video.mp4"
 */
void OverallTests::checkMp4Testfile3()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Mp4);
    CPPUNIT_ASSERT(m_fileInfo.container() && m_fileInfo.container()->documentType() == "dash");
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Avc);
            CPPUNIT_ASSERT(track->format().sub == SubFormats::AvcMainProfile);
            CPPUNIT_ASSERT(track->version() == 3.1);
            CPPUNIT_ASSERT(track->creationTime().year() == 2014);
            CPPUNIT_ASSERT(track->pixelSize() == Size(854, 480));
            CPPUNIT_ASSERT(!strcmp(track->chromaFormat(), "YUV 4:2:0"));
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 0);
        break;
    case TagStatus::TestMetaDataPresent:
        checkMp4TestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "mtx-test-data/mp4/alac/othertest-itunes.m4a"
 */
void OverallTests::checkMp4Testfile4()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Mp4);
    CPPUNIT_ASSERT(m_fileInfo.container() && m_fileInfo.container()->documentType() == "M4A ");
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Alac);
            CPPUNIT_ASSERT(track->creationTime().year() == 2008);
            CPPUNIT_ASSERT(track->channelCount() == 2);
            CPPUNIT_ASSERT(track->bitsPerSample() == 16);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Title).toString() == "Sad Song");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).toString() == "Oasis");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Album).toString() == "Don't Go Away (Apple Lossless)");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Genre).toString() == "Alternative & Punk");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Encoder).toString() == "iTunes v7.5.0.20");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Year).toString() == "1998");
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Comment).isEmpty());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Cover).dataSize() == 0x58f3);
        CPPUNIT_ASSERT(BE::toUInt64(tags.front()->value(KnownField::Cover).dataPointer()) == 0xFFD8FFE000104A46);
        CPPUNIT_ASSERT(tags.front()->value(KnownField::TrackPosition).toPositionInSet() == PositionInSet(3, 4));
        CPPUNIT_ASSERT(tags.front()->value(KnownField::DiskPosition).toPositionInSet() == PositionInSet(1, 1));
        break;
    case TagStatus::TestMetaDataPresent:
        checkMp4TestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks "mtx-test-data/aac/he-aacv2-ps.m4a"
 */
void OverallTests::checkMp4Testfile5()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Mp4);
    CPPUNIT_ASSERT(m_fileInfo.container() && m_fileInfo.container()->documentType() == "mp42");
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Aac);
            CPPUNIT_ASSERT(track->format().sub == SubFormats::AacMpeg4LowComplexityProfile);
            CPPUNIT_ASSERT(track->format().extension & ExtensionFormats::SpectralBandReplication);
            CPPUNIT_ASSERT(track->format().extension & ExtensionFormats::ParametricStereo);
            CPPUNIT_ASSERT(track->creationTime().year() == 2014);
            CPPUNIT_ASSERT(track->channelCount() == 2);
            CPPUNIT_ASSERT(track->channelConfig() == Mpeg4ChannelConfigs::FrontCenter);
            CPPUNIT_ASSERT(track->extensionChannelConfig() == Mpeg4ChannelConfigs::FrontLeftFrontRight);
            CPPUNIT_ASSERT(track->samplingFrequency() == 24000);
            CPPUNIT_ASSERT(track->extensionSamplingFrequency() == 48000);
            CPPUNIT_ASSERT(track->bitsPerSample() == 16);
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 0);
        break;
    case TagStatus::TestMetaDataPresent:
        checkMp4TestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }
}

/*!
 * \brief Checks whether test meta data for MP4 files has been applied correctly.
 */
void OverallTests::checkMp4TestMetaData()
{
    // check whether a tag is assigned
    const auto tags = m_fileInfo.tags();
    Mp4Tag *tag = m_fileInfo.mp4Tag();
    CPPUNIT_ASSERT(tags.size() == 1);
    CPPUNIT_ASSERT(tag != nullptr);

    // check test meta data
    CPPUNIT_ASSERT(tag->value(KnownField::Title) == m_testTitle);
    CPPUNIT_ASSERT(tag->value(KnownField::Comment) == m_testComment);
    CPPUNIT_ASSERT(tag->value(KnownField::Album) == m_testAlbum);
    CPPUNIT_ASSERT(tag->value(KnownField::Artist) == m_preservedMetaData.front());
    CPPUNIT_ASSERT(tag->value(KnownField::TrackPosition) == m_testPosition);
    CPPUNIT_ASSERT(tag->value(KnownField::DiskPosition) == m_testPosition);
    // TODO: check more fields
    m_preservedMetaData.pop();
}

/*!
 * \brief Checks whether padding constraints are met.
 */
void OverallTests::checkMp4PaddingConstraints()
{
    if((m_mode & 0x4) && (m_mode & 0x1)) {
        CPPUNIT_ASSERT(m_fileInfo.paddingSize() == 4096);
    } else if(m_mode & 0x4) {
        CPPUNIT_ASSERT(m_fileInfo.paddingSize() >= 1024);
        CPPUNIT_ASSERT(m_fileInfo.paddingSize() <= (4096 + 1024));
        // TODO: check tag position and rewriting behaviour
    }
}

/*!
 * \brief Checks "mtx-test-data/mp3/id3-tag-and-xing-header.mp3"
 */
void OverallTests::checkMp3Testfile1()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::MpegAudioFrames);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for(const auto &track : tracks) {
        CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
        CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Mpeg1Audio);
        CPPUNIT_ASSERT(track->format().sub == SubFormats::Mpeg1Layer3);
        CPPUNIT_ASSERT(track->channelCount() == 2);
        CPPUNIT_ASSERT(track->channelConfig() == static_cast<byte>(MpegChannelMode::JointStereo));
        CPPUNIT_ASSERT(track->samplingFrequency() == 44100);
        CPPUNIT_ASSERT(track->duration().seconds() == 3);
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT(m_fileInfo.id3v2Tags().size() == 1);
        CPPUNIT_ASSERT(tags.size() == 2);
        for(const auto &tag : tags) {
            CPPUNIT_ASSERT(tag->value(KnownField::TrackPosition).toPositionInSet().position() == 4);
            CPPUNIT_ASSERT(tag->value(KnownField::Year).toString() == "1984");
            switch(tag->type()) {
            case TagType::Id3v1Tag:
                CPPUNIT_ASSERT(tag->value(KnownField::Title).toString() == "Cohesion");
                CPPUNIT_ASSERT(tag->value(KnownField::Artist).toString() == "Minutemen");
                CPPUNIT_ASSERT(tag->value(KnownField::Album).toString() == "Double Nickels On The Dime");
                CPPUNIT_ASSERT(tag->value(KnownField::Genre).toString() == "Punk Rock");
                CPPUNIT_ASSERT(tag->value(KnownField::Comment).toString() == "ExactAudioCopy v0.95b4");
                break;
            case TagType::Id3v2Tag:
                CPPUNIT_ASSERT(tag->value(KnownField::Title).dataEncoding() == TagTextEncoding::Utf16LittleEndian);
                CPPUNIT_ASSERT(tag->value(KnownField::Title).toWString() == u"Cohesion");
                CPPUNIT_ASSERT(tag->value(KnownField::Title).toString(TagTextEncoding::Utf8) == "Cohesion");
                CPPUNIT_ASSERT(tag->value(KnownField::Artist).toWString() == u"Minutemen");
                CPPUNIT_ASSERT(tag->value(KnownField::Artist).toString(TagTextEncoding::Utf8) == "Minutemen");
                CPPUNIT_ASSERT(tag->value(KnownField::Album).toWString() == u"Double Nickels On The Dime");
                CPPUNIT_ASSERT(tag->value(KnownField::Album).toString(TagTextEncoding::Utf8) == "Double Nickels On The Dime");
                CPPUNIT_ASSERT(tag->value(KnownField::Genre).toWString() == u"Punk Rock");
                CPPUNIT_ASSERT(tag->value(KnownField::Genre).toString(TagTextEncoding::Utf8) == "Punk Rock");
                CPPUNIT_ASSERT(tag->value(KnownField::Comment).toWString() == u"ExactAudioCopy v0.95b4");
                CPPUNIT_ASSERT(tag->value(KnownField::Comment).toString(TagTextEncoding::Utf8) == "ExactAudioCopy v0.95b4");
                CPPUNIT_ASSERT(tag->value(KnownField::TrackPosition).toPositionInSet().total() == 43);
                CPPUNIT_ASSERT(tag->value(KnownField::Length).toTimeSpan().isNull());
                CPPUNIT_ASSERT(tag->value(KnownField::Lyricist).isEmpty());
                break;
            default:
                ;
            }
        }
        break;
    case TagStatus::TestMetaDataPresent:
        checkMp3TestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
    }

}

/*!
 * \brief Checks whether test meta data for MP3 files has been applied correctly.
 */
void OverallTests::checkMp3TestMetaData()
{
    // check whether tags are assigned according to the current test mode
    Id3v1Tag *id3v1Tag = nullptr;
    Id3v2Tag *id3v2Tag = nullptr;
    if(m_mode & 0x2) {
        CPPUNIT_ASSERT(id3v1Tag = m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT(id3v2Tag = m_fileInfo.id3v2Tags().at(0).get());
    } else if(m_mode & 0x8) {
        CPPUNIT_ASSERT(id3v1Tag = m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT(m_fileInfo.id3v2Tags().empty());
    } else {
        CPPUNIT_ASSERT(!m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT(id3v2Tag = m_fileInfo.id3v2Tags().at(0).get());
    }

    // check common test meta data
    for(Tag *tag : initializer_list<Tag *>{id3v1Tag, id3v2Tag}) {
        if(tag) {
            CPPUNIT_ASSERT(tag->value(KnownField::Title) == m_testTitle);
            CPPUNIT_ASSERT(tag->value(KnownField::Comment) == m_testComment);
            CPPUNIT_ASSERT(tag->value(KnownField::Album) == m_testAlbum);
            CPPUNIT_ASSERT(tag->value(KnownField::Artist) == m_preservedMetaData.front());
            // TODO: check more fields
            m_preservedMetaData.pop();
        }
    }
    // test ID3v1 specific test meta data
    if(id3v1Tag) {
        CPPUNIT_ASSERT(id3v1Tag->value(KnownField::TrackPosition).toPositionInSet().position() == m_testPosition.toPositionInSet().position());
    }
    // test ID3v2 specific test meta data
    if(id3v2Tag) {
        CPPUNIT_ASSERT(id3v2Tag->value(KnownField::TrackPosition) == m_testPosition);
        CPPUNIT_ASSERT(id3v2Tag->value(KnownField::DiskPosition) == m_testPosition);
    }
}

/*!
 * \brief Checks whether padding constraints are met.
 */
void OverallTests::checkMp3PaddingConstraints()
{
    if(!(m_mode & 0x8)) {
        if((m_mode & 0x4) && (m_mode & 0x1)) {
            CPPUNIT_ASSERT(m_fileInfo.paddingSize() == 4096);
        } else if((m_mode & 0x4)) {
            CPPUNIT_ASSERT(m_fileInfo.paddingSize() >= 1024);
            CPPUNIT_ASSERT(m_fileInfo.paddingSize() <= (4096 + 1024));
        }
    } else {
        // adding padding is not possible if no ID3v2 tag is present
    }
    // TODO: check rewriting behaviour
}

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
        CPPUNIT_ASSERT(tags.size() == 0);
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
        CPPUNIT_ASSERT(tags.size() == 0);
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
    CPPUNIT_ASSERT(tag->value(KnownField::Title) == m_testTitle);
    CPPUNIT_ASSERT(tag->value(KnownField::Comment) == m_testComment);
    CPPUNIT_ASSERT(tag->value(KnownField::Album) == m_testAlbum);
    CPPUNIT_ASSERT(tag->value(KnownField::Artist) == m_preservedMetaData.front());
    CPPUNIT_ASSERT(tag->value(KnownField::TrackPosition) == m_testPosition);
    CPPUNIT_ASSERT(tag->value(KnownField::DiskPosition) == m_testPosition);
    // TODO: check more fields
    m_preservedMetaData.pop();
}

/*!
 * \brief Checks "flac/test.flac" (converted from "mtx-test-data/alac/othertest-itunes.m4a" via ffmpeg).
 * \remarks Raw FLAC stream.
 */
void OverallTests::checkFlacTestfile1()
{
    CPPUNIT_ASSERT(m_fileInfo.containerFormat() == ContainerFormat::Flac);
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT(tracks.size() == 1);
    for(const auto &track : tracks) {
        CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
        CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Flac);
        CPPUNIT_ASSERT(track->channelCount() == 2);
        CPPUNIT_ASSERT(track->samplingFrequency() == 44100);
        CPPUNIT_ASSERT(track->bitsPerSample() == 16);
        CPPUNIT_ASSERT(track->duration().minutes() == 4);
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
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
        CPPUNIT_ASSERT(tags.size() == 0);
    }
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
    for(const auto &track : tracks) {
        CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
        CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Flac);
        CPPUNIT_ASSERT(track->channelCount() == 2);
        CPPUNIT_ASSERT(track->samplingFrequency() == 44100);
        CPPUNIT_ASSERT(track->bitsPerSample() == 16);
        CPPUNIT_ASSERT(track->duration().minutes() == 4);
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(tags.size() == 1);
        break;
    case TagStatus::TestMetaDataPresent:
        checkOggTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT(tags.size() == 0);
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

void OverallTests::setMp4TestMetaData()
{
    // ensure a tag exists
    Tag *tag = m_fileInfo.container()->createTag();

    // assign test meta data
    tag->setValue(KnownField::Title, m_testTitle);
    tag->setValue(KnownField::Comment, m_testComment);
    tag->setValue(KnownField::Album, m_testAlbum);
    m_preservedMetaData.push(tag->value(KnownField::Artist));
    tag->setValue(KnownField::TrackPosition, m_testPosition);
    tag->setValue(KnownField::DiskPosition, m_testPosition);
    // TODO: set more fields
}

void OverallTests::setMp3TestMetaData()
{
    // ensure tags are assigned according to the current test mode
    Id3v1Tag *id3v1Tag = nullptr;
    Id3v2Tag *id3v2Tag = nullptr;
    if(m_mode & 0x2) {
        id3v1Tag = m_fileInfo.createId3v1Tag();
        id3v2Tag = m_fileInfo.createId3v2Tag();
    } else if(m_mode & 0x8) {
        id3v1Tag = m_fileInfo.createId3v1Tag();
        m_fileInfo.removeAllId3v2Tags();
    } else {
        m_fileInfo.removeId3v1Tag();
        id3v2Tag = m_fileInfo.createId3v2Tag();
    }

    // assign some test meta data
    for(Tag *tag : initializer_list<Tag *>{id3v1Tag, id3v2Tag}) {
        if(tag) {
            tag->setValue(KnownField::Title, m_testTitle);
            tag->setValue(KnownField::Comment, m_testComment);
            tag->setValue(KnownField::Album, m_testAlbum);
            m_preservedMetaData.push(tag->value(KnownField::Artist));
            tag->setValue(KnownField::TrackPosition, m_testPosition);
            tag->setValue(KnownField::DiskPosition, m_testPosition);
            // TODO: set more fields
        }
    }
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

#ifdef PLATFORM_UNIX
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
            m_fileInfo.setIndexPosition(m_mode & 0x80 ? ElementPosition::BeforeData : ElementPosition::AfterData);
        }
        m_fileInfo.setPreferredPadding(m_mode & 0x8 ? 4096 : 0);
        m_fileInfo.setMinPadding(m_mode & 0x8 ? 1024 : 0);
        m_fileInfo.setMaxPadding(m_mode & 0x8 ? (4096 + 1024) : static_cast<size_t>(-1));
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
#endif

/*!
 * \brief Tests the MP4 parser via MediaFileInfo.
 */
void OverallTests::testMp4Parsing()
{
    cerr << endl << "MP4 parser" << endl;
    m_fileInfo.setForceFullParse(false);
    m_tagStatus = TagStatus::Original;
    parseFile(TestUtilities::testFilePath("mtx-test-data/mp4/10-DanseMacabreOp.40.m4a"), &OverallTests::checkMp4Testfile1);
    parseFile(TestUtilities::testFilePath("mtx-test-data/mp4/1080p-DTS-HD-7.1.mp4"), &OverallTests::checkMp4Testfile2);
    parseFile(TestUtilities::testFilePath("mtx-test-data/mp4/dash/dragon-age-inquisition-H1LkM6IVlm4-video.mp4"), &OverallTests::checkMp4Testfile3);
    parseFile(TestUtilities::testFilePath("mtx-test-data/alac/othertest-itunes.m4a"), &OverallTests::checkMp4Testfile4);
    parseFile(TestUtilities::testFilePath("mtx-test-data/aac/he-aacv2-ps.m4a"), &OverallTests::checkMp4Testfile5);
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Tests the MP4 maker via MediaFileInfo.
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testMp4Making()
{
    // full parse is required to determine padding
    m_fileInfo.setForceFullParse(true);

    // do the test under different conditions
    for(m_mode = 0; m_mode != 0x20; ++m_mode) {
        // setup test conditions
        m_fileInfo.setForceRewrite(m_mode & 0x1);
        if(m_mode & 0x2) {
            m_fileInfo.setTagPosition(ElementPosition::Keep);
        } else {
            m_fileInfo.setTagPosition(m_mode & 0x10 ? ElementPosition::BeforeData : ElementPosition::AfterData);
        }
        m_fileInfo.setIndexPosition(m_fileInfo.tagPosition());
        m_fileInfo.setPreferredPadding(m_mode & 0x4 ? 4096 : 0);
        m_fileInfo.setMinPadding(m_mode & 0x4 ? 1024 : 0);
        m_fileInfo.setMaxPadding(m_mode & 0x4 ? (4096 + 1024) : static_cast<size_t>(-1));
        m_fileInfo.setForceTagPosition(m_mode & 0x8);
        m_fileInfo.setForceIndexPosition(m_mode & 0x8);

        // print test conditions
        list<string> testConditions;
        if(m_mode & 0x1) {
            testConditions.emplace_back("forcing rewrite");
        }
        if(m_mode & 0x2) {
            if(m_mode & 0x10) {
                testConditions.emplace_back("removing tag");
            } else {
                testConditions.emplace_back("keeping tag position");
            }
        } else if(m_mode & 0x10) {
            testConditions.emplace_back("tags before data");
        } else {
            testConditions.emplace_back("tags after data");
        }
        if(m_mode & 0x4) {
            testConditions.emplace_back("padding constraints");
        }
        if(m_mode & 0x8) {
            testConditions.emplace_back("forcing tag position");
        }
        cerr << endl << "MP4 maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        bool remove = (m_mode & 0x10) && (m_mode & 0x2);
        m_tagStatus = remove ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = remove ? &OverallTests::removeAllTags : &OverallTests::setMp4TestMetaData;
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mp4/10-DanseMacabreOp.40.m4a"), modifyRoutine, &OverallTests::checkMp4Testfile1);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mp4/1080p-DTS-HD-7.1.mp4"), modifyRoutine, &OverallTests::checkMp4Testfile2);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mp4/dash/dragon-age-inquisition-H1LkM6IVlm4-video.mp4"), modifyRoutine, &OverallTests::checkMp4Testfile3);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/alac/othertest-itunes.m4a"), modifyRoutine, &OverallTests::checkMp4Testfile4);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/aac/he-aacv2-ps.m4a"), modifyRoutine, &OverallTests::checkMp4Testfile5);
    }
}
#endif

/*!
 * \brief Tests the MP3 parser via MediaFileInfo.
 */
void OverallTests::testMp3Parsing()
{
    cerr << endl << "MP3 parser" << endl;
    m_fileInfo.setForceFullParse(false);
    m_tagStatus = TagStatus::Original;
    parseFile(TestUtilities::testFilePath("mtx-test-data/mp3/id3-tag-and-xing-header.mp3"), &OverallTests::checkMp3Testfile1);
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Tests the MP3 maker via MediaFileInfo.
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testMp3Making()
{
    // full parse is required to determine padding
    m_fileInfo.setForceFullParse(true);

    // do the test under different conditions
    for(m_mode = 0; m_mode != 0x10; ++m_mode) {
        // setup test conditions
        m_fileInfo.setForceRewrite(m_mode & 0x1);
        m_fileInfo.setTagPosition(ElementPosition::Keep);
        m_fileInfo.setIndexPosition(ElementPosition::Keep);
        m_fileInfo.setPreferredPadding(m_mode & 0x4 ? 4096 : 0);
        m_fileInfo.setMinPadding(m_mode & 0x4 ? 1024 : 0);
        m_fileInfo.setMaxPadding(m_mode & 0x4 ? (4096 + 1024) : static_cast<size_t>(-1));
        m_fileInfo.setForceTagPosition(false);
        m_fileInfo.setForceIndexPosition(false);

        // print test conditions
        list<string> testConditions;
        if(m_mode & 0x1) {
            testConditions.emplace_back("forcing rewrite");
        }
        if(m_mode & 0x2) {
            if(m_mode & 0x8) {
                testConditions.emplace_back("removing tag");
            } else {
                testConditions.emplace_back("ID3v1 and ID3v2");
            }
        } else if(m_mode & 0x8) {
            testConditions.emplace_back("ID3v1 only");
        } else {
            testConditions.emplace_back("ID3v2 only");
        }
        if(m_mode & 0x4) {
            testConditions.emplace_back("padding constraints");
        }
        cerr << endl << "MP3 maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        bool remove = (m_mode & 0x10) && (m_mode & 0x2);
        m_tagStatus = remove ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = remove ? &OverallTests::removeAllTags : &OverallTests::setMp3TestMetaData;
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mp3/id3-tag-and-xing-header.mp3"), modifyRoutine, &OverallTests::checkMp3Testfile1);
    }
}
#endif

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
        // no need to setup test conditions because the Ogg maker
        // doesn't take those settings into account (currently)

        // print test conditions
        list<string> testConditions;
        if(m_mode & 0x1) {
            testConditions.emplace_back("removing tag");
        } else {
            testConditions.emplace_back("modifying tag");
        }
        cerr << endl << "OGG maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        bool remove = m_mode & 0x1;
        m_tagStatus = remove ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = remove ? &OverallTests::removeAllTags : &OverallTests::setOggTestMetaData;
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/ogg/qt4dance_medium.ogg"), modifyRoutine, &OverallTests::checkOggTestfile1);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/opus/v-opus.ogg"), modifyRoutine, &OverallTests::checkOggTestfile2);
    }
}
#endif

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
    for(m_mode = 0; m_mode != 0x2; ++m_mode) {
        // TODO: setup test conditions

        // print test conditions
        list<string> testConditions;
        if(m_mode & 0x1) {
            testConditions.emplace_back("removing tag");
        } else {
            testConditions.emplace_back("modifying tag");
        }
        cerr << endl << "FLAC maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        bool remove = m_mode & 0x1;
        m_tagStatus = remove ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = remove ? &OverallTests::removeAllTags : &OverallTests::setOggTestMetaData;
        makeFile(TestUtilities::workingCopyPath("flac/test.flac"), modifyRoutine, &OverallTests::checkFlacTestfile1);
        makeFile(TestUtilities::workingCopyPath("flac/test.ogg"), modifyRoutine, &OverallTests::checkFlacTestfile2);
    }
}
#endif
