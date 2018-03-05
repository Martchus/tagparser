#include <c++utilities/chrono/format.h>

#include "./overall.h"

#include "../abstracttrack.h"
#include "../mpegaudio/mpegaudioframe.h"
#include "../mp4/mp4ids.h"
#include "../matroska/matroskacontainer.h"

#include <c++utilities/chrono/timespan.h>
#include <c++utilities/conversion/binaryconversion.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/misc.h>

#include <fstream>
#include <cstring>

using namespace ChronoUtilities;

namespace MkvTestFlags {
enum TestFlag
{
    ForceRewring = 0x1,
    KeepTagPos = 0x2,
    TagsBeforeData = 0x40,
    RemoveTag = KeepTagPos & TagsBeforeData,
    KeepIndexPos = 0x4,
    IndexBeforeData = 0x80,
    PaddingConstraints = 0x8,
    ForceTagPos = 0x10,
    ForceIndexPos = 0x20,
};
}

/*!
 * \brief Checks "matroska_wave1/test1.mkv".
 */
void OverallTests::checkMkvTestfile1()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan::fromMinutes(1) + TimeSpan::fromSeconds(27) + TimeSpan::fromMilliseconds(336), m_fileInfo.duration());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 2422994868:
            CPPUNIT_ASSERT_EQUAL(MediaType::Video, track->mediaType());
            CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::MicrosoftMpeg4, track->format().general);
            break;
        case 3653291187:
            CPPUNIT_ASSERT_EQUAL(MediaType::Audio, track->mediaType());
            CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Mpeg1Audio, track->format().general);
            CPPUNIT_ASSERT_EQUAL(48000u, track->samplingFrequency());
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
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "matroska_wave1/test2.mkv".
 */
void OverallTests::checkMkvTestfile2()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan::fromSeconds(47) + TimeSpan::fromMilliseconds(509), m_fileInfo.duration());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
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
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "matroska_wave1/test3.mkv".
 */
void OverallTests::checkMkvTestfile3()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan::fromSeconds(49) + TimeSpan::fromMilliseconds(64), m_fileInfo.duration());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
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
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "matroska_wave1/test4.mkv".
 * \remarks This file is using the EBML feature that allows Master elements to have no known size.
 */
void OverallTests::checkMkvTestfile4()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan(), m_fileInfo.duration());
    // this file is messed up, it should contain tags but it doesn't
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
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
            switch(m_tagStatus) {
            case TagStatus::Original:
            case TagStatus::Removed:
                CPPUNIT_ASSERT_EQUAL("und"s, track->language());
                CPPUNIT_ASSERT_EQUAL(string(), track->name());
                CPPUNIT_ASSERT(track->isEnabled());
                CPPUNIT_ASSERT(!track->isForced());
                CPPUNIT_ASSERT(!track->isDefault());
                break;
            case TagStatus::TestMetaDataPresent:
                CPPUNIT_ASSERT_EQUAL("ger"s, track->language());
                CPPUNIT_ASSERT_EQUAL("the name"s, track->name());
                CPPUNIT_ASSERT(track->isEnabled());
                CPPUNIT_ASSERT(track->isForced());
                CPPUNIT_ASSERT(track->isDefault());
                break;
            }
            break;
        default:
            CPPUNIT_FAIL("unknown track ID");
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    }

    // tolerate critical notifications here because live stream feature used by the file is not supported in v6 yet
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Critical);
}

/*!
 * \brief Checks "matroska_wave1/test5.mkv".
 */
void OverallTests::checkMkvTestfile5()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan::fromSeconds(46) + TimeSpan::fromMilliseconds(665), m_fileInfo.duration());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(11_st, tracks.size());
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
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "matroska_wave1/test6.mkv".
 */
void OverallTests::checkMkvTestfile6()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan::fromMinutes(1) + TimeSpan::fromSeconds(27) + TimeSpan::fromMilliseconds(336), m_fileInfo.duration());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
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
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "matroska_wave1/test7.mkv".
 */
void OverallTests::checkMkvTestfile7()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan::fromSeconds(37) + TimeSpan::fromMilliseconds(43), m_fileInfo.duration());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
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
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
        CPPUNIT_ASSERT_EQUAL("Big Buck Bunny - test 7"s, tags.front()->value(KnownField::Title).toString());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT_EQUAL("Matroska Validation File 7, junk elements are present at the beggining or end of clusters, the parser should skip it. There is also a damaged element at 451418"s, tags.front()->value(KnownField::Comment).toString());
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }

    for(const auto &msg : m_diag) {
        if(msg.level() != DiagLevel::Warning) {
            continue;
        }
        CPPUNIT_ASSERT(startsWith(msg.context(), "parsing header of EBML element 0xEA \"cue codec state\" at"));
        CPPUNIT_ASSERT_EQUAL("Data of EBML element seems to be truncated; unable to parse siblings of that element."s, msg.message());
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Warning);
}

/*!
 * \brief Checks "matroska_wave1/test8.mkv".
 */
void OverallTests::checkMkvTestfile8()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan::fromSeconds(47) + TimeSpan::fromMilliseconds(341), m_fileInfo.duration());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
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
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
        CPPUNIT_ASSERT_EQUAL("Big Buck Bunny - test 8"s, tags.front()->value(KnownField::Title).toString());
        CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
        CPPUNIT_ASSERT_EQUAL("Matroska Validation File 8, audio missing between timecodes 6.019s and 6.360s"s, tags.front()->value(KnownField::Comment).toString());
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "mtx-test-data/mkv/handbrake-chapters-2.mkv".
 */
void OverallTests::checkMkvTestfileHandbrakeChapters()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    CPPUNIT_ASSERT_EQUAL(TimeSpan::fromSeconds(27) + TimeSpan::fromMilliseconds(569), m_fileInfo.duration());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tracks.size());
    for(const auto &track : tracks) {
        switch(track->id()) {
        case 1:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Video);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Avc);
            CPPUNIT_ASSERT_EQUAL(4.0, track->version());
            CPPUNIT_ASSERT(track->pixelSize() == Size(1280, 544));
            CPPUNIT_ASSERT(track->displaySize() == Size(1280, 544));
            CPPUNIT_ASSERT(track->fps() == 23);
            break;
        case 2:
            CPPUNIT_ASSERT(track->mediaType() == MediaType::Audio);
            CPPUNIT_ASSERT(track->format() == GeneralMediaFormat::Aac);
            CPPUNIT_ASSERT(track->samplingFrequency() == 44100);
            CPPUNIT_ASSERT(track->channelConfig() == Mpeg4ChannelConfigs::FrontLeftFrontRight);
            break;
        default:
            CPPUNIT_FAIL(argsToString("unknown track ID ", track->id()));
        }
    }
    const auto chapters = m_fileInfo.chapters();
    CPPUNIT_ASSERT_EQUAL(2_st, chapters.size());
    for(const auto &chapter : chapters) {
        switch(chapter->id()) {
        case 1:
            CPPUNIT_ASSERT(!strcmp(chapter->names().at(0).data(), "Kapitel 01"));
            CPPUNIT_ASSERT_EQUAL(0l, chapter->startTime().totalTicks());
            CPPUNIT_ASSERT_EQUAL(15, chapter->endTime().seconds());
            break;
        case 2:
            CPPUNIT_ASSERT(!strcmp(chapter->names().at(0).data(), "Kapitel 02"));
            CPPUNIT_ASSERT_EQUAL(15, chapter->startTime().seconds());
            CPPUNIT_ASSERT_EQUAL(27, chapter->endTime().seconds());
            break;
        default:
            CPPUNIT_FAIL(argsToString("unknown chapter ID ", chapter->id()));
        }
    }
    const auto tags = m_fileInfo.tags();
    switch(m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT_EQUAL(2_st, tags.size());
        CPPUNIT_ASSERT(tags[0]->target().isEmpty());
        CPPUNIT_ASSERT_EQUAL(""s, static_cast<MatroskaTag *>(tags[0])->value("CREATION_TIME").toString());
        CPPUNIT_ASSERT_EQUAL("Lavf55.12.0"s, tags[0]->value(KnownField::Encoder).toString());
        CPPUNIT_ASSERT_EQUAL(2_st, tags[1]->target().tracks().at(0));
        CPPUNIT_ASSERT_EQUAL("eng"s, tags[1]->value(KnownField::Language).toString());
        break;
    case TagStatus::TestMetaDataPresent:
        checkMkvTestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "mtx-test-data/mkv/nested-tags.mkv" ("mtx-test-data/mkv/tags.mkv" where "mkv/nested-tags.xml" has been applied).
 */
void OverallTests::checkMkvTestfileNestedTags()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    const auto tags = m_fileInfo.tags();
    bool generalTagFound = false;
    switch(m_tagStatus) {
    case TagStatus::Original:
    case TagStatus::TestMetaDataPresent:
        CPPUNIT_ASSERT_EQUAL(5_st, tags.size());
        for(const Tag *tag : tags) {
            CPPUNIT_ASSERT(tag->type() == TagType::MatroskaTag);
            const auto *mkvTag = static_cast<const MatroskaTag *>(tag);
            const auto &target = mkvTag->target();
            if(target.level() == 50 && target.tracks().empty()) {
                generalTagFound = true;
                CPPUNIT_ASSERT_EQUAL("Vanilla Sky"s, tag->value(KnownField::Title).toString());
                const auto &fields = mkvTag->fields();
                const auto &artistField = fields.find(mkvTag->fieldId(KnownField::Artist));
                CPPUNIT_ASSERT(artistField != fields.end());
                CPPUNIT_ASSERT_EQUAL("Test artist"s, artistField->second.value().toString());
                const auto &nestedFields = artistField->second.nestedFields();
                CPPUNIT_ASSERT_EQUAL(1_st, nestedFields.size());
                CPPUNIT_ASSERT_EQUAL("ADDRESS"s, nestedFields[0].idToString());
                CPPUNIT_ASSERT_EQUAL("Test address"s, nestedFields[0].value().toString());
            }
        }
        CPPUNIT_ASSERT(generalTagFound);
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }

    // the file contains in fact the unknown element [44][B4]
    // TODO: find out what this element is about (its data is only the single byte 0x01)
    for(const auto &msg : m_diag) {
        if(msg.level() != DiagLevel::Warning) {
            continue;
        }
        CPPUNIT_ASSERT(startsWith(msg.message(), "\"SimpleTag\"-element contains unknown element 0x44B4 at"));
    }
    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Warning);
}

/*!
 * \brief Checks whether test meta data for Matroska files has been applied correctly.
 */
void OverallTests::checkMkvTestMetaData()
{
    // check tags
    const auto tags = m_fileInfo.tags();
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(2_st, tags.size());
    CPPUNIT_ASSERT_EQUAL(m_testTitle.toString(), tags.front()->value(KnownField::Title).toString());
    CPPUNIT_ASSERT(tags.front()->value(KnownField::Artist).isEmpty());
    CPPUNIT_ASSERT_EQUAL(m_testComment.toString(), tags.front()->value(KnownField::Comment).toString());
    CPPUNIT_ASSERT_EQUAL(30_st, tags[1]->target().level());
    CPPUNIT_ASSERT_EQUAL(tracks.at(0)->id(), tags[1]->target().tracks().at(0));
    CPPUNIT_ASSERT_EQUAL(m_testAlbum.toString(), tags[1]->value(KnownField::Album).toString());
    CPPUNIT_ASSERT_EQUAL(m_testPartNumber.toInteger(), tags[1]->value(KnownField::PartNumber).toInteger());
    CPPUNIT_ASSERT_EQUAL(m_testTotalParts.toInteger(), tags[1]->value(KnownField::TotalParts).toInteger());

    // check attachments
    const auto attachments = m_fileInfo.attachments();
    CPPUNIT_ASSERT_EQUAL(1_st, attachments.size());
    CPPUNIT_ASSERT(attachments[0]->mimeType() == "image/png");
    CPPUNIT_ASSERT(attachments[0]->name() == "cover.jpg");
    const StreamDataBlock *attachmentData = attachments[0]->data();
    CPPUNIT_ASSERT(attachmentData != nullptr);
    if (m_testCover.empty()) {
        m_testCover = readFile(testFilePath("matroska_wave1/logo3_256x256.png"), 20000);
    }
    CPPUNIT_ASSERT_EQUAL(m_testCover.size(), static_cast<size_t>(attachmentData->size()));
    istream &attachmentSteam = attachmentData->stream();
    attachmentSteam.seekg(attachmentData->startOffset());
    for (char expectedChar : m_testCover) {
        CPPUNIT_ASSERT_EQUAL(expectedChar, static_cast<char>(attachmentSteam.get()));
    }
}

/*!
 * \brief Checks whether padding and element position constraints are met.
 */
void OverallTests::checkMkvConstraints()
{
    using namespace MkvTestFlags;

    CPPUNIT_ASSERT(m_fileInfo.container());
    if(m_mode & PaddingConstraints) {
        if(m_mode & ForceRewring) {
            CPPUNIT_ASSERT_EQUAL(4096_st, m_fileInfo.paddingSize());
        } else {
            CPPUNIT_ASSERT(m_fileInfo.paddingSize() >= 1024);
            CPPUNIT_ASSERT(m_fileInfo.paddingSize() <= (4096 + 1024));
        }
        if(!(m_mode & RemoveTag) && (m_expectedTagPos != ElementPosition::Keep) && ((m_mode & ForceRewring) || (m_mode & ForceTagPos))) {
            CPPUNIT_ASSERT_EQUAL(m_expectedTagPos,  m_fileInfo.container()->determineTagPosition(m_diag));
        }
        if((m_expectedIndexPos != ElementPosition::Keep) && ((m_mode & ForceRewring) || (m_mode & ForceIndexPos))) {
            CPPUNIT_ASSERT_EQUAL(m_expectedIndexPos, m_fileInfo.container()->determineIndexPosition(m_diag));
        }
    }
}

/*!
 * \brief Creates a tag targeting the first track with some test meta data.
 */
void OverallTests::setMkvTestMetaData()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Matroska, m_fileInfo.containerFormat());
    auto *container = static_cast<MatroskaContainer *>(m_fileInfo.container());

    // change the present tag
    const string fileName(m_fileInfo.fileName());
    if(fileName == "test4.mkv") {
        // test4.mkv has no tag, so one must be created first
        container->createTag(TagTarget(50));
        // also change language, name, forced and default of track "3171450505" to German
        MatroskaTrack *track = container->trackById(3171450505);
        CPPUNIT_ASSERT(track);
        track->setLanguage("ger");
        track->setName("the name");
        track->setDefault(true);
        track->setEnabled(true);
        track->setForced(true);
    } else if(fileName == "handbrake-chapters-2.mkv") {
        // remove 2nd tag
        m_fileInfo.removeTag(m_fileInfo.tags().at(1));
    }
    Tag *firstTag = m_fileInfo.tags().at(0);
    firstTag->setValue(KnownField::Title, m_testTitle);
    firstTag->setValue(KnownField::Comment, m_testComment);
    // add an additional tag targeting the first track
    TagTarget::IdContainerType trackIds;
    trackIds.emplace_back(m_fileInfo.tracks().at(0)->id());
    Tag *newTag = container->createTag(TagTarget(30, trackIds));
    CPPUNIT_ASSERT_MESSAGE("create tag", newTag);
    newTag->setValue(KnownField::Album, m_testAlbum);
    newTag->setValue(KnownField::PartNumber, m_testPartNumber);
    newTag->setValue(KnownField::TotalParts, m_testTotalParts);
    // assign an attachment
    AbstractAttachment *attachment = container->createAttachment();
    CPPUNIT_ASSERT_MESSAGE("create attachment", attachment);
    attachment->setFile(TestUtilities::testFilePath("matroska_wave1/logo3_256x256.png"), m_diag);
    attachment->setMimeType("image/png");
    attachment->setName("cover.jpg");
}

/*!
 * \brief Creates a Matroska test file with nested tags from "mtx-test-data/mkv/nested-tags.mkv" using "mkv/nested-tags.xml".
 * \remarks Requires mkvmerge.
 * \todo Make mkvmerge path variable.
 */
void OverallTests::createMkvWithNestedTags()
{
#ifdef PLATFORM_UNIX
    m_nestedTagsMkvPath = workingCopyPathMode("mtx-test-data/mkv/nested-tags.mkv", WorkingCopyMode::NoCopy);
    remove(m_nestedTagsMkvPath.data());

    cerr << "\n\n- Create testfile \"" << m_nestedTagsMkvPath << "\" with mkvmerge" << endl;
    const string tagsMkvPath(testFilePath("mtx-test-data/mkv/tags.mkv"));
    const string tagsXmlPath(testFilePath("mkv/nested-tags.xml"));
    const char *const mkvmergeArgs[] = {
        "--ui-language en_US",
        "--output", m_nestedTagsMkvPath.data(),
        "--no-global-tags", "--language", "0:und", "--default-track", "0:yes", "--language", "1:und", "--default-track", "1:yes",
        "(", tagsMkvPath.data(), ")",
        "--global-tags", tagsXmlPath.data(), "--track-order", "0:0,0:1", nullptr
    };
    string mkvmergeOutput, mkvmergeErrors;
    int res = execHelperApp("/bin/mkvmerge", mkvmergeArgs, mkvmergeOutput, mkvmergeErrors);
    cout << mkvmergeOutput << endl;
    cerr << mkvmergeErrors << endl;
    if(res) {
        cerr << "- failure (exit code " << res << "); unable to test nested tags" << endl;
        remove(m_nestedTagsMkvPath.data());
        m_nestedTagsMkvPath.clear();
    }
#endif
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
    parseFile(TestUtilities::testFilePath("mtx-test-data/mkv/handbrake-chapters-2.mkv"), &OverallTests::checkMkvTestfileHandbrakeChapters);
    createMkvWithNestedTags();
    if(!m_nestedTagsMkvPath.empty()) {
        parseFile(m_nestedTagsMkvPath, &OverallTests::checkMkvTestfileNestedTags);
    }
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Tests the Matroska maker via MediaFileInfo.
 *
 * This method tests various combinations of the possible settings.
 *
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testMkvMakingWithDifferentSettings()
{
    // full parse is required to determine padding
    m_fileInfo.setForceFullParse(true);

    // do the test under different conditions
    for(m_mode = 0; m_mode != 0x100; ++m_mode) {
        using namespace MkvTestFlags;

        // setup test conditions
        m_fileInfo.setForceRewrite(m_mode & ForceRewring);
        if(m_mode & KeepTagPos) {
            m_fileInfo.setTagPosition(ElementPosition::Keep);
        } else {
            m_fileInfo.setTagPosition(m_mode & TagsBeforeData ? ElementPosition::BeforeData : ElementPosition::AfterData);
        }
        if(m_mode & KeepIndexPos) {
            if(m_mode & IndexBeforeData) {
                continue;
            }
            m_fileInfo.setIndexPosition(ElementPosition::Keep);
        } else {
            m_fileInfo.setIndexPosition(m_mode & IndexBeforeData ? ElementPosition::BeforeData : ElementPosition::AfterData);
        }
        m_fileInfo.setPreferredPadding(m_mode & PaddingConstraints ? 4096 : 0);
        m_fileInfo.setMinPadding(m_mode & PaddingConstraints ? 1024 : 0);
        m_fileInfo.setMaxPadding(m_mode & PaddingConstraints ? (4096 + 1024) : static_cast<size_t>(-1));
        m_fileInfo.setForceTagPosition(m_mode & ForceTagPos);
        m_fileInfo.setForceIndexPosition(m_mode & ForceIndexPos);

        // print test conditions
        list<string> testConditions;
        if(m_mode & ForceRewring) {
            testConditions.emplace_back("forcing rewrite");
        }
        if(m_mode & KeepTagPos) {
            if(m_mode & RemoveTag) {
                testConditions.emplace_back("removing tag");
            } else {
                testConditions.emplace_back("keeping tag position");
            }
        } else if(m_mode & TagsBeforeData) {
            testConditions.emplace_back("tags before data");
        } else {
            testConditions.emplace_back("tags after data");
        }
        if(m_mode & KeepIndexPos) {
            testConditions.emplace_back("keeping index position");
        } else if(m_mode & IndexBeforeData) {
            testConditions.emplace_back("index before data");
        } else {
            testConditions.emplace_back("index after data");
        }
        if(m_mode & PaddingConstraints) {
            testConditions.emplace_back("padding constraints");
        }
        if(m_mode & ForceTagPos) {
            testConditions.emplace_back("forcing tag position");
        }
        if(m_mode & ForceIndexPos) {
            testConditions.emplace_back("forcing index position");
        }
        cerr << endl << "Matroska maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        m_tagStatus = (m_mode & RemoveTag) ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setMkvTestMetaData;
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test1.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile1);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test2.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile2);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test3.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile3);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test4.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile4);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test5.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile5);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test6.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile6);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test7.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile7);
        makeFile(TestUtilities::workingCopyPath("matroska_wave1/test8.mkv"), modifyRoutine, &OverallTests::checkMkvTestfile8);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mkv/handbrake-chapters-2.mkv"), modifyRoutine, &OverallTests::checkMkvTestfileHandbrakeChapters);
    }
}

/*!
 * \brief Tests making a Matroska file with nested tags via MediaFileInfo.
 * \remarks Relies on the parser to check results.
 */
void OverallTests::testMkvMakingNestedTags()
{
    createMkvWithNestedTags();
    if(!m_nestedTagsMkvPath.empty()) {
        cerr << endl << "Matroska maker - rewrite file with nested tags" << endl;
        m_fileInfo.setMinPadding(0);
        m_fileInfo.setMaxPadding(0);
        m_fileInfo.setTagPosition(ElementPosition::BeforeData);
        m_fileInfo.setIndexPosition(ElementPosition::BeforeData);
        makeFile(m_nestedTagsMkvPath, &OverallTests::noop, &OverallTests::checkMkvTestfileNestedTags);
    }
}
#endif
