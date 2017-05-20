#include "./overall.h"

#include "../abstracttrack.h"
#include "../mp4/mp4ids.h"
#include "../mp4/mp4tag.h"

namespace Mp4TestFlags {
enum TestFlag
{
    ForceRewring = 0x1,
    KeepTagPos = 0x2,
    TagsBeforeData = 0x10,
    RemoveTag = KeepTagPos & TagsBeforeData,
    PaddingConstraints = 0x4,
    ForceTagPos = 0x8,
};
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
    using namespace Mp4TestFlags;

    if(m_mode & PaddingConstraints) {
        if(m_mode & ForceRewring) {
            CPPUNIT_ASSERT(m_fileInfo.paddingSize() == 4096);
        } else {
            CPPUNIT_ASSERT(m_fileInfo.paddingSize() >= 1024);
            CPPUNIT_ASSERT(m_fileInfo.paddingSize() <= (4096 + 1024));
            // TODO: check tag position and rewriting behaviour
        }
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
        using namespace Mp4TestFlags;

        // setup test conditions
        m_fileInfo.setForceRewrite(m_mode & ForceRewring);
        if(m_mode & KeepTagPos) {
            m_fileInfo.setTagPosition(ElementPosition::Keep);
        } else {
            m_fileInfo.setTagPosition(m_mode & TagsBeforeData ? ElementPosition::BeforeData : ElementPosition::AfterData);
        }
        m_fileInfo.setIndexPosition(m_fileInfo.tagPosition());
        m_fileInfo.setPreferredPadding(m_mode & PaddingConstraints ? 4096 : 0);
        m_fileInfo.setMinPadding(m_mode & PaddingConstraints ? 1024 : 0);
        m_fileInfo.setMaxPadding(m_mode & PaddingConstraints ? (4096 + 1024) : static_cast<size_t>(-1));
        m_fileInfo.setForceTagPosition(m_mode & ForceTagPos);
        m_fileInfo.setForceIndexPosition(m_mode & ForceTagPos);

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
        if(m_mode & PaddingConstraints) {
            testConditions.emplace_back("padding constraints");
        }
        if(m_mode & ForceTagPos) {
            testConditions.emplace_back("forcing tag position");
        }
        cerr << endl << "MP4 maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        m_tagStatus = (m_mode & RemoveTag) ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setMp4TestMetaData;
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mp4/10-DanseMacabreOp.40.m4a"), modifyRoutine, &OverallTests::checkMp4Testfile1);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mp4/1080p-DTS-HD-7.1.mp4"), modifyRoutine, &OverallTests::checkMp4Testfile2);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mp4/dash/dragon-age-inquisition-H1LkM6IVlm4-video.mp4"), modifyRoutine, &OverallTests::checkMp4Testfile3);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/alac/othertest-itunes.m4a"), modifyRoutine, &OverallTests::checkMp4Testfile4);
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/aac/he-aacv2-ps.m4a"), modifyRoutine, &OverallTests::checkMp4Testfile5);
    }
}
#endif
