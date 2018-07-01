#include "./helper.h"
#include "./overall.h"

#include "../abstracttrack.h"
#include "../id3/id3v1tag.h"
#include "../id3/id3v2tag.h"
#include "../mpegaudio/mpegaudioframe.h"

namespace Mp3TestFlags {
enum TestFlag {
    ForceRewring = 0x1,
    Id3v2AndId3v1 = 0x2,
    PaddingConstraints = 0x4,
    Id3v1Only = 0x8,
    RemoveTag = Id3v2AndId3v1 & Id3v1Only,
    UseId3v24 = 0x10,
};
}

/*!
 * \brief Checks "mtx-test-data/mp3/id3-tag-and-xing-header.mp3"
 */
void OverallTests::checkMp3Testfile1()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::MpegAudioFrames, m_fileInfo.containerFormat());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(1_st, tracks.size());
    for (const auto &track : tracks) {
        CPPUNIT_ASSERT_EQUAL(MediaType::Audio, track->mediaType());
        CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Mpeg1Audio, track->format().general);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(SubFormats::Mpeg1Layer3), track->format().sub);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16>(2), track->channelCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<byte>(MpegChannelMode::JointStereo), track->channelConfig());
        CPPUNIT_ASSERT_EQUAL(44100u, track->samplingFrequency());
        CPPUNIT_ASSERT_EQUAL(3, track->duration().seconds());
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT_EQUAL(1_st, m_fileInfo.id3v2Tags().size());
        CPPUNIT_ASSERT_EQUAL(2_st, tags.size());
        for (const auto &tag : tags) {
            CPPUNIT_ASSERT_EQUAL(4, tag->value(KnownField::TrackPosition).toPositionInSet().position());
            CPPUNIT_ASSERT_EQUAL("1984"s, tag->value(KnownField::Year).toString());
            switch (tag->type()) {
            case TagType::Id3v1Tag:
                CPPUNIT_ASSERT_EQUAL("Cohesion"s, tag->value(KnownField::Title).toString());
                CPPUNIT_ASSERT_EQUAL("Minutemen"s, tag->value(KnownField::Artist).toString());
                CPPUNIT_ASSERT_EQUAL("Double Nickels On The Dime"s, tag->value(KnownField::Album).toString());
                CPPUNIT_ASSERT_EQUAL("Punk Rock"s, tag->value(KnownField::Genre).toString());
                CPPUNIT_ASSERT_EQUAL("ExactAudioCopy v0.95b4"s, tag->value(KnownField::Comment).toString());
                break;
            case TagType::Id3v2Tag:
                CPPUNIT_ASSERT_EQUAL(TagTextEncoding::Utf16LittleEndian, tag->value(KnownField::Title).dataEncoding());
                CPPUNIT_ASSERT_EQUAL(u"Cohesion"s, tag->value(KnownField::Title).toWString());
                CPPUNIT_ASSERT_EQUAL("Cohesion"s, tag->value(KnownField::Title).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL(u"Minutemen"s, tag->value(KnownField::Artist).toWString());
                CPPUNIT_ASSERT_EQUAL("Minutemen"s, tag->value(KnownField::Artist).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL(u"Double Nickels On The Dime"s, tag->value(KnownField::Album).toWString());
                CPPUNIT_ASSERT_EQUAL("Double Nickels On The Dime"s, tag->value(KnownField::Album).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL(u"Punk Rock"s, tag->value(KnownField::Genre).toWString());
                CPPUNIT_ASSERT_EQUAL("Punk Rock"s, tag->value(KnownField::Genre).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL(u"ExactAudioCopy v0.95b4"s, tag->value(KnownField::Comment).toWString());
                CPPUNIT_ASSERT_EQUAL("ExactAudioCopy v0.95b4"s, tag->value(KnownField::Comment).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL(43, tag->value(KnownField::TrackPosition).toPositionInSet().total());
                CPPUNIT_ASSERT(tag->value(KnownField::Length).toTimeSpan().isNull());
                CPPUNIT_ASSERT(tag->value(KnownField::Lyricist).isEmpty());
                break;
            default:;
            }
        }
        break;
    case TagStatus::TestMetaDataPresent:
        checkMp3TestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tracks.size());
    }

    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
}

/*!
 * \brief Checks "misc/multiple_id3v2_4_values.mp3" (from https://trac.ffmpeg.org/ticket/6949).
 */
void OverallTests::checkMp3Testfile2()
{
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::MpegAudioFrames, m_fileInfo.containerFormat());
    const auto tracks = m_fileInfo.tracks();
    CPPUNIT_ASSERT_EQUAL(1_st, tracks.size());
    for (const auto &track : tracks) {
        CPPUNIT_ASSERT_EQUAL(MediaType::Audio, track->mediaType());
        CPPUNIT_ASSERT_EQUAL(GeneralMediaFormat::Mpeg1Audio, track->format().general);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(SubFormats::Mpeg1Layer3), track->format().sub);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16>(2), track->channelCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<byte>(MpegChannelMode::Stereo), track->channelConfig());
        CPPUNIT_ASSERT_EQUAL(44100u, track->samplingFrequency());
        CPPUNIT_ASSERT_EQUAL(20, track->duration().seconds());
    }
    const auto tags = m_fileInfo.tags();
    switch (m_tagStatus) {
    case TagStatus::Original:
        CPPUNIT_ASSERT(!m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT_EQUAL(1_st, m_fileInfo.id3v2Tags().size());
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
        for (const auto &tag : tags) {
            switch (tag->type()) {
            case TagType::Id3v1Tag:
                CPPUNIT_FAIL("no ID3v1 tag expected");
            case TagType::Id3v2Tag:
                CPPUNIT_ASSERT_EQUAL(TagTextEncoding::Utf8, tag->value(KnownField::Title).dataEncoding());
                CPPUNIT_ASSERT_EQUAL("Infinite (Original Mix)"s, tag->value(KnownField::Title).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("B-Front"s, tag->value(KnownField::Artist).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Infinite"s, tag->value(KnownField::Album).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Hardstyle"s, tag->value(KnownField::Genre).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Lavf57.83.100"s, tag->value(KnownField::EncoderSettings).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Roughstate"s, tag->value(KnownField::RecordLabel).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("2017"s, tag->value(KnownField::RecordDate).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL(1, tag->value(KnownField::TrackPosition).toPositionInSet().position());
                CPPUNIT_ASSERT(tag->value(KnownField::Length).toTimeSpan().isNull());
                CPPUNIT_ASSERT(tag->value(KnownField::Lyricist).isEmpty());
                break;
            default:;
            }
        }
        CPPUNIT_ASSERT_GREATEREQUAL(2_st, m_diag.size());
        CPPUNIT_ASSERT_EQUAL(DiagLevel::Warning, m_diag[0].level());
        CPPUNIT_ASSERT_EQUAL(DiagLevel::Warning, m_diag[1].level());
        CPPUNIT_ASSERT_EQUAL("parsing TCON frame"s, m_diag[1].context());
        CPPUNIT_ASSERT_EQUAL(
            "Multiple strings found. This is not supported so far. Hence the additional values \"Test\", \"Example\", and \"Hard Dance\" are ignored."s,
            m_diag[1].message());
        break;
    case TagStatus::TestMetaDataPresent:
        checkMp3TestMetaData();
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tracks.size());
    }

    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Warning);
}

/*!
 * \brief Checks whether test meta data for MP3 files has been applied correctly.
 */
void OverallTests::checkMp3TestMetaData()
{
    using namespace Mp3TestFlags;

    // check whether tags are assigned according to the current test mode
    Id3v1Tag *id3v1Tag = nullptr;
    Id3v2Tag *id3v2Tag = nullptr;
    if (m_mode & Id3v2AndId3v1) {
        CPPUNIT_ASSERT(id3v1Tag = m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT(id3v2Tag = m_fileInfo.id3v2Tags().at(0).get());
    } else if (m_mode & Id3v1Only) {
        CPPUNIT_ASSERT(id3v1Tag = m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT(m_fileInfo.id3v2Tags().empty());
    } else {
        CPPUNIT_ASSERT(!m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT(id3v2Tag = m_fileInfo.id3v2Tags().at(0).get());
    }

    // check common test meta data
    if (id3v1Tag) {
        CPPUNIT_ASSERT_EQUAL(m_testTitle, id3v1Tag->value(KnownField::Title));
        CPPUNIT_ASSERT_EQUAL(m_testComment.toString(), id3v1Tag->value(KnownField::Comment).toString()); // ignore encoding here
        CPPUNIT_ASSERT_EQUAL(m_testAlbum, id3v1Tag->value(KnownField::Album));
        CPPUNIT_ASSERT_EQUAL(m_preservedMetaData.front(), id3v1Tag->value(KnownField::Artist));
        m_preservedMetaData.pop();
    }
    if (id3v2Tag) {
        const TagValue &titleValue = id3v2Tag->value(KnownField::Title);
        const TagValue &commentValue = id3v2Tag->value(KnownField::Comment);

        if (m_mode & UseId3v24) {
            CPPUNIT_ASSERT_EQUAL(m_testTitle, titleValue);
            CPPUNIT_ASSERT_EQUAL(m_testComment, commentValue);
            CPPUNIT_ASSERT_EQUAL(m_testAlbum, id3v2Tag->value(KnownField::Album));
            CPPUNIT_ASSERT_EQUAL(m_preservedMetaData.front(), id3v2Tag->value(KnownField::Artist));
            // TODO: check more fields
        } else {
            CPPUNIT_ASSERT_MESSAGE("not attempted to use UTF-8 in ID3v2.3", titleValue.dataEncoding() == TagTextEncoding::Utf16LittleEndian);
            CPPUNIT_ASSERT_EQUAL(m_testTitle.toString(), titleValue.toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_MESSAGE("not attempted to use UTF-8 in ID3v2.3", commentValue.dataEncoding() == TagTextEncoding::Utf16LittleEndian);
            CPPUNIT_ASSERT_MESSAGE("not attempted to use UTF-8 in ID3v2.3", commentValue.descriptionEncoding() == TagTextEncoding::Utf16LittleEndian);
            CPPUNIT_ASSERT_EQUAL(m_testComment.toString(), commentValue.toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "description is also converted to UTF-16", "s\0o\0m\0e\0 \0d\0e\0s\0c\0r\0i\0p\0t\0i\0\xf3\0n\0"s, commentValue.description());
            CPPUNIT_ASSERT_EQUAL(m_testAlbum.toString(TagTextEncoding::Utf8), id3v2Tag->value(KnownField::Album).toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL(m_preservedMetaData.front(), id3v2Tag->value(KnownField::Artist));
            // TODO: check more fields
        }

        m_preservedMetaData.pop();
    }

    // test ID3v1 specific test meta data
    if (id3v1Tag) {
        CPPUNIT_ASSERT_EQUAL(m_testPosition.toPositionInSet().position(), id3v1Tag->value(KnownField::TrackPosition).toPositionInSet().position());
    }
    // test ID3v2 specific test meta data
    if (id3v2Tag) {
        CPPUNIT_ASSERT_EQUAL(m_testPosition, id3v2Tag->value(KnownField::TrackPosition));
        CPPUNIT_ASSERT_EQUAL(m_testPosition, id3v2Tag->value(KnownField::DiskPosition));
    }
}

/*!
 * \brief Checks whether padding constraints are met.
 */
void OverallTests::checkMp3PaddingConstraints()
{
    using namespace Mp3TestFlags;

    if (!(m_mode & Id3v1Only)) {
        if (m_mode & PaddingConstraints) {
            if (m_mode & ForceRewring) {
                CPPUNIT_ASSERT_EQUAL(static_cast<uint64>(4096), m_fileInfo.paddingSize());
            } else {
                CPPUNIT_ASSERT(m_fileInfo.paddingSize() >= 1024);
                CPPUNIT_ASSERT(m_fileInfo.paddingSize() <= (4096 + 1024));
            }
        }
    } else {
        // adding padding is not possible if no ID3v2 tag is present
    }
    // TODO: check rewriting behaviour
}

void OverallTests::setMp3TestMetaData()
{
    using namespace Mp3TestFlags;

    // ensure tags are assigned according to the current test mode
    Id3v1Tag *id3v1Tag = nullptr;
    Id3v2Tag *id3v2Tag = nullptr;
    if (m_mode & Id3v2AndId3v1) {
        id3v1Tag = m_fileInfo.createId3v1Tag();
        id3v2Tag = m_fileInfo.createId3v2Tag();
    } else if (m_mode & Id3v1Only) {
        id3v1Tag = m_fileInfo.createId3v1Tag();
        m_fileInfo.removeAllId3v2Tags();
    } else {
        m_fileInfo.removeId3v1Tag();
        id3v2Tag = m_fileInfo.createId3v2Tag();
    }
    if (!(m_mode & Id3v1Only) && m_mode & UseId3v24) {
        id3v2Tag->setVersion(4, 0);
    }

    // assign some test meta data
    for (Tag *tag : initializer_list<Tag *>{ id3v1Tag, id3v2Tag }) {
        if (tag) {
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

/*!
 * \brief Tests the MP3 parser via MediaFileInfo.
 */
void OverallTests::testMp3Parsing()
{
    cerr << endl << "MP3 parser" << endl;
    m_fileInfo.setForceFullParse(false);
    m_tagStatus = TagStatus::Original;
    parseFile(TestUtilities::testFilePath("mtx-test-data/mp3/id3-tag-and-xing-header.mp3"), &OverallTests::checkMp3Testfile1);
    parseFile(TestUtilities::testFilePath("misc/multiple_id3v2_4_values.mp3"), &OverallTests::checkMp3Testfile2);
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
    for (m_mode = 0; m_mode != 0x20; ++m_mode) {
        using namespace Mp3TestFlags;

        // setup test conditions
        m_fileInfo.setForceRewrite(m_mode & ForceRewring);
        if (m_mode & UseId3v24) {
            if (m_mode & Id3v1Only) {
                continue;
            }
        }
        m_fileInfo.setTagPosition(ElementPosition::Keep);
        m_fileInfo.setIndexPosition(ElementPosition::Keep);
        m_fileInfo.setPreferredPadding(m_mode & PaddingConstraints ? 4096 : 0);
        m_fileInfo.setMinPadding(m_mode & PaddingConstraints ? 1024 : 0);
        m_fileInfo.setMaxPadding(m_mode & PaddingConstraints ? (4096 + 1024) : numeric_limits<size_t>::max());
        m_fileInfo.setForceTagPosition(false);
        m_fileInfo.setForceIndexPosition(false);

        // print test conditions
        list<string> testConditions;
        if (m_mode & ForceRewring) {
            testConditions.emplace_back("forcing rewrite");
        }
        if (m_mode & Id3v2AndId3v1) {
            if (m_mode & RemoveTag) {
                testConditions.emplace_back("removing tag");
            } else {
                testConditions.emplace_back("ID3v1 and ID3v2");
            }
        } else if (m_mode & Id3v1Only) {
            testConditions.emplace_back("ID3v1 only");
        } else {
            testConditions.emplace_back("ID3v2 only");
        }
        if (m_mode & PaddingConstraints) {
            testConditions.emplace_back("padding constraints");
        }
        if (m_mode & UseId3v24) {
            testConditions.emplace_back("use ID3v2.4");
        }
        cerr << endl << "MP3 maker - testmode " << m_mode << ": " << joinStrings(testConditions, ", ") << endl;

        // do actual tests
        m_tagStatus = (m_mode & RemoveTag) ? TagStatus::Removed : TagStatus::TestMetaDataPresent;
        void (OverallTests::*modifyRoutine)(void) = (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setMp3TestMetaData;
        makeFile(TestUtilities::workingCopyPath("mtx-test-data/mp3/id3-tag-and-xing-header.mp3"), modifyRoutine, &OverallTests::checkMp3Testfile1);
    }
}
#endif
