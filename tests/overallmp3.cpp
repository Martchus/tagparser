#include "./helper.h"
#include "./overall.h"

#include "../abstracttrack.h"
#include "../id3/id3v1tag.h"
#include "../id3/id3v2tag.h"
#include "../mpegaudio/mpegaudioframe.h"

#include <regex>

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
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(2), track->channelCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(MpegChannelMode::JointStereo), track->channelConfig());
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
            CPPUNIT_ASSERT_EQUAL("1984"s, tag->value(KnownField::RecordDate).toString());
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

    auto warningAboutEncoding = false;
    for (auto &msg : m_diag) {
        if (msg.message() == "The used encoding is unlikely to be supported by other software.") {
            CPPUNIT_ASSERT_EQUAL(DiagLevel::Warning, msg.level());
            warningAboutEncoding = true;
            msg = DiagMessage(DiagLevel::Information, string(), string());
        }
    }
    const auto encodingWarningExpected
        = m_tagStatus == TagStatus::TestMetaDataPresent && (m_mode & Mp3TestFlags::Id3v1Only || m_mode & Mp3TestFlags::Id3v2AndId3v1);
    CPPUNIT_ASSERT_EQUAL(encodingWarningExpected, warningAboutEncoding);
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
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(2), track->channelCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(MpegChannelMode::Stereo), track->channelConfig());
        CPPUNIT_ASSERT_EQUAL(44100u, track->samplingFrequency());
        CPPUNIT_ASSERT_EQUAL(20, track->duration().seconds());
    }
    const auto tags = m_fileInfo.tags();
    const bool expectId3v24 = m_tagStatus == TagStatus::Original || m_mode & Mp3TestFlags::UseId3v24;
    switch (m_tagStatus) {
    case TagStatus::Original:
    case TagStatus::TestMetaDataPresent:
        CPPUNIT_ASSERT(!m_fileInfo.id3v1Tag());
        CPPUNIT_ASSERT_EQUAL(1_st, m_fileInfo.id3v2Tags().size());
        CPPUNIT_ASSERT_EQUAL(1_st, tags.size());
        for (const auto &tag : tags) {
            if (tag->type() != TagType::Id3v2Tag) {
                CPPUNIT_FAIL(argsToString("no ", tag->typeName(), " tag expected"));
            }
            const auto *const id3v2Tag = static_cast<const Id3v2Tag *>(tag);

            // check values as usual
            CPPUNIT_ASSERT_EQUAL(expectId3v24 ? 4 : 3, static_cast<int>(id3v2Tag->majorVersion()));
            CPPUNIT_ASSERT_EQUAL(
                expectId3v24 ? TagTextEncoding::Utf8 : TagTextEncoding::Utf16LittleEndian, tag->value(KnownField::Title).dataEncoding());
            CPPUNIT_ASSERT_EQUAL("Infinite (Original Mix)"s, tag->value(KnownField::Title).toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL("B-Front"s, tag->value(KnownField::Artist).toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL("Infinite"s, tag->value(KnownField::Album).toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL(m_tagStatus == TagStatus::TestMetaDataPresent ? "Test"s : "Hardstyle"s,
                tag->value(KnownField::Genre).toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL("Lavf57.83.100"s, tag->value(KnownField::EncoderSettings).toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL("Roughstate"s, tag->value(KnownField::Publisher).toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL("2017"s, tag->value(KnownField::RecordDate).toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL(1, tag->value(KnownField::TrackPosition).toPositionInSet().position());
            CPPUNIT_ASSERT(tag->value(KnownField::Length).toTimeSpan().isNull());
            CPPUNIT_ASSERT(tag->value(KnownField::Lyricist).isEmpty());

            // check additional text frame values
            const auto &fields = id3v2Tag->fields();
            auto genreFields = fields.equal_range(Id3v2FrameIds::lGenre);
            CPPUNIT_ASSERT_MESSAGE("genre field present"s, genreFields.first != genreFields.second);
            const auto &genreField = genreFields.first->second;
            const auto &additionalValues = genreField.additionalValues();
            if (m_tagStatus == TagStatus::TestMetaDataPresent) {
                CPPUNIT_ASSERT_EQUAL("Test"s, tag->value(KnownField::Genre).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL(1_st, additionalValues.size());
                CPPUNIT_ASSERT_EQUAL("Example"s, additionalValues[0].toString(TagTextEncoding::Utf8));
            } else {
                CPPUNIT_ASSERT_EQUAL("Hardstyle"s, tag->value(KnownField::Genre).toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL(3_st, additionalValues.size());
                CPPUNIT_ASSERT_EQUAL("Test"s, additionalValues[0].toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Example"s, additionalValues[1].toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Hard Dance"s, additionalValues[2].toString(TagTextEncoding::Utf8));
            }
            CPPUNIT_ASSERT_MESSAGE("exactly one genre field present"s, ++genreFields.first == genreFields.second);

            // check whether additional text frame values are returned correctly by values()
            const auto artists = id3v2Tag->values(KnownField::Artist);
            CPPUNIT_ASSERT_EQUAL(m_tagStatus == TagStatus::TestMetaDataPresent ? 3_st : 2_st, artists.size());
            CPPUNIT_ASSERT_EQUAL("B-Front"s, artists[0]->toString(TagTextEncoding::Utf8));
            CPPUNIT_ASSERT_EQUAL("Second Artist Example"s, artists[1]->toString(TagTextEncoding::Utf8));
            if (m_tagStatus == TagStatus::TestMetaDataPresent) {
                CPPUNIT_ASSERT_EQUAL("3rd Artist Example"s, artists[2]->toString(TagTextEncoding::Utf8));
            }

            const auto genres = id3v2Tag->values(KnownField::Genre);
            if (m_tagStatus == TagStatus::TestMetaDataPresent) {
                CPPUNIT_ASSERT_EQUAL(2_st, genres.size());
                CPPUNIT_ASSERT_EQUAL("Test"s, genres[0]->toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Example"s, genres[1]->toString(TagTextEncoding::Utf8));
            } else {
                CPPUNIT_ASSERT_EQUAL(4_st, genres.size());
                CPPUNIT_ASSERT_EQUAL("Hardstyle"s, genres[0]->toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Test"s, genres[1]->toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Example"s, genres[2]->toString(TagTextEncoding::Utf8));
                CPPUNIT_ASSERT_EQUAL("Hard Dance"s, genres[3]->toString(TagTextEncoding::Utf8));
            }
        }
        break;
    case TagStatus::Removed:
        CPPUNIT_ASSERT_EQUAL(0_st, tags.size());
    }

    if (expectId3v24 || m_tagStatus == TagStatus::Removed) {
        CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Information);
        return;
    }

    CPPUNIT_ASSERT(m_diag.level() <= DiagLevel::Warning);
    int warningCount = 0;
    for (const auto &msg : m_diag) {
        if (msg.level() != DiagLevel::Warning) {
            continue;
        }
        ++warningCount;
        TESTUTILS_ASSERT_LIKE("context", "(parsing|making) (TPE1|TCON)( frame)?", msg.context());
        TESTUTILS_ASSERT_LIKE("message",
            "Multiple strings (found|assigned) .*"
            "Additional (values \"Second Artist Example\" and \"3rd Artist Example\" are|"
            "value \"Example\" is) "
            "supposed to be ignored.",
            msg.message());
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("exactly 4 warnings present", 4, warningCount);
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
        CPPUNIT_ASSERT_EQUAL(TagTextEncoding::Latin1, id3v1Tag->value(KnownField::Title).dataEncoding());
        CPPUNIT_ASSERT_EQUAL(m_testTitle, id3v1Tag->value(KnownField::Title));
        CPPUNIT_ASSERT_EQUAL(m_testCommentWithoutDescription, id3v1Tag->value(KnownField::Comment));
        CPPUNIT_ASSERT_EQUAL(m_testAlbum, id3v1Tag->value(KnownField::Album));
        CPPUNIT_ASSERT_EQUAL(m_preservedMetaData.front(), id3v1Tag->value(KnownField::Artist));
        m_preservedMetaData.pop();
    }
    if (id3v2Tag) {
        const TagValue &titleValue = id3v2Tag->value(KnownField::Title);
        const TagValue &commentValue = id3v2Tag->value(KnownField::Comment);

        if (m_mode & UseId3v24) {
            CPPUNIT_ASSERT_EQUAL(TagTextEncoding::Utf8, titleValue.dataEncoding());
            CPPUNIT_ASSERT_EQUAL(m_testTitle, titleValue);
            CPPUNIT_ASSERT_EQUAL(m_testComment, commentValue);
            CPPUNIT_ASSERT_EQUAL(m_testAlbum, id3v2Tag->value(KnownField::Album));
            CPPUNIT_ASSERT_EQUAL(m_preservedMetaData.front(), id3v2Tag->value(KnownField::Artist));
            // TODO: check more fields
        } else {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("not attempted to use UTF-8 in ID3v2.3", TagTextEncoding::Utf16LittleEndian, titleValue.dataEncoding());
            CPPUNIT_ASSERT_EQUAL(m_testTitle, titleValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("not attempted to use UTF-8 in ID3v2.3", TagTextEncoding::Utf16LittleEndian, commentValue.dataEncoding());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "not attempted to use UTF-8 in ID3v2.3", TagTextEncoding::Utf16LittleEndian, commentValue.descriptionEncoding());
            CPPUNIT_ASSERT_EQUAL(m_testComment, commentValue);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "description is also converted to UTF-16", "s\0o\0m\0e\0 \0d\0e\0s\0c\0r\0i\0p\0t\0i\0\xf3\0n\0"s, commentValue.description());
            CPPUNIT_ASSERT_EQUAL(m_testAlbum, id3v2Tag->value(KnownField::Album));
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
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(4096), m_fileInfo.paddingSize());
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

/*!
 * \brief Sets meta-data for "mtx-test-data/mp3/id3-tag-and-xing-header.mp3".
 */
void OverallTests::setMp3TestMetaData1()
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
    for (Tag *const tag : initializer_list<Tag *>{ id3v1Tag, id3v2Tag }) {
        if (!tag) {
            continue;
        }
        tag->setValue(KnownField::Title, m_testTitle);
        tag->setValue(KnownField::Comment, m_testComment);
        tag->setValue(KnownField::Album, m_testAlbum);
        m_preservedMetaData.push(tag->value(KnownField::Artist));
        tag->setValue(KnownField::TrackPosition, m_testPosition);
        tag->setValue(KnownField::DiskPosition, m_testPosition);
        // TODO: set more fields
    }
    if (id3v1Tag) {
        id3v1Tag->ensureTextValuesAreProperlyEncoded();
    }
}

/*!
 * \brief Sets meta-data for "misc/multiple_id3v2_4_values.mp3".
 */
void OverallTests::setMp3TestMetaData2()
{
    using namespace Mp3TestFlags;

    CPPUNIT_ASSERT_EQUAL(1_st, m_fileInfo.id3v2Tags().size());
    auto &id3v2Tag(m_fileInfo.id3v2Tags().front());
    id3v2Tag->setVersion((m_mode & UseId3v24) ? 4 : 3, 0);
    const auto artists = id3v2Tag->values(KnownField::Artist);
    CPPUNIT_ASSERT_EQUAL(2_st, artists.size());
    id3v2Tag->setValues(KnownField::Artist, { *artists[0], *artists[1], TagValue("3rd Artist Example") });
    id3v2Tag->setValues(KnownField::Genre, { TagValue("Test"), TagValue("Example") });
}

/*!
 * \brief Tests the MP3 parser via MediaFileInfo.
 */
void OverallTests::testMp3Parsing()
{
    cerr << endl << "MP3 parser" << endl;
    m_fileInfo.setForceFullParse(false);
    m_tagStatus = TagStatus::Original;
    parseFile(testFilePath("mtx-test-data/mp3/id3-tag-and-xing-header.mp3"), &OverallTests::checkMp3Testfile1);
    parseFile(testFilePath("misc/multiple_id3v2_4_values.mp3"), &OverallTests::checkMp3Testfile2);
}

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
        makeFile(workingCopyPath("mtx-test-data/mp3/id3-tag-and-xing-header.mp3"),
            (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setMp3TestMetaData1, &OverallTests::checkMp3Testfile1);
        makeFile(workingCopyPath("misc/multiple_id3v2_4_values.mp3"),
            (m_mode & RemoveTag) ? &OverallTests::removeAllTags : &OverallTests::setMp3TestMetaData2, &OverallTests::checkMp3Testfile2);
    }
}
