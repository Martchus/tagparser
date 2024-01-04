#ifndef TAGPARSER_OVERALL_TESTS_H
#define TAGPARSER_OVERALL_TESTS_H

#include "./helper.h"

#include "../diagnostics.h"
#include "../mediafileinfo.h"
#include "../progressfeedback.h"
#include "../tagvalue.h"

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/format.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/tests/testutils.h>
using namespace CppUtilities;

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <queue>
#include <string>

using namespace std;
using namespace CppUtilities::Literals;
using namespace TagParser;
using namespace CPPUNIT_NS;

enum class TagStatus { Original, TestMetaDataPresent, Removed };

namespace SimpleTestFlags {
enum TestFlag {
    RemoveTag = 0x1,
};
}

/*!
 * \brief The OverallTests class tests reading and writing tags and parsing technical information
 *        for all supported container/tag formats.
 */
class OverallTests : public TestFixture {
    CPPUNIT_TEST_SUITE(OverallTests);
    CPPUNIT_TEST(testMp4Parsing);
    CPPUNIT_TEST(testMp3Parsing);
    CPPUNIT_TEST(testOggParsing);
    CPPUNIT_TEST(testFlacParsing);
    CPPUNIT_TEST(testMkvParsing);
    CPPUNIT_TEST(testMp4Making);
    CPPUNIT_TEST(testMp3Making);
    CPPUNIT_TEST(testOggMaking);
    CPPUNIT_TEST(testFlacMaking);
    CPPUNIT_TEST(testMkvMakingWithDifferentSettings);
    CPPUNIT_TEST(testMkvMakingNestedTags);
    CPPUNIT_TEST(testVorbisCommentFieldHandling);
    CPPUNIT_TEST_SUITE_END();

public:
    OverallTests();

    void setUp() override;
    void tearDown() override;

private:
    void parseFile(const string &path, void (OverallTests::*checkRoutine)(void));
    void makeFile(const string &path, void (OverallTests::*modifyRoutine)(void), void (OverallTests::*checkRoutine)(void));

    void checkMkvTestfile1();
    void checkMkvTestfile2();
    void checkMkvTestfile3();
    void checkMkvTestfile4();
    void checkMkvTestfile5();
    void checkMkvTestfile6();
    void checkMkvTestfile7();
    void checkMkvTestfile8();
    void checkMkvTestfileHandbrakeChapters();
    void checkMkvTestfileNestedTags();
    void checkMkvTestMetaData();
    void checkMkvConstraints();

    void checkMp4Testfile1();
    void checkMp4Testfile2();
    void checkMp4Testfile3();
    void checkMp4Testfile4();
    void checkMp4Testfile5();
    void checkMp4Testfile6();
    void checkMp4Testfile7();
    void checkMp4TestMetaData();
    void checkMp4Constraints();

    void checkMp3Testfile1();
    void checkMp3Testfile2();
    void checkMp3TestMetaData();
    void checkMp3PaddingConstraints();

    void checkOggTestfile1();
    void checkOggTestfile2();
    void checkOggTestfile3();
    void checkOggTestMetaData();
    void checkOggTestMetaDataCover();

    void checkFlacTestfile1();
    void checkFlacTestfile2();

    void setMkvTestMetaData();
    void setMp4TestMetaData();
    void setMp3TestMetaData1();
    void setMp3TestMetaData2();
    void setOggTestMetaData();
    void setOggTestMetaDataCover();
    void removeAllTags();
    void noop();
    void alterMp4Tracks();
    void removeSecondTrack();

public:
    void testMkvParsing();
    void testMp4Parsing();
    void testMp3Parsing();
    void testOggParsing();
    void testFlacParsing();
    void testMkvMakingWithDifferentSettings();
    void testMkvMakingNestedTags();
    void testMp4Making();
    void testMp3Making();
    void testOggMaking();
    void testFlacMaking();
    void testVorbisCommentFieldHandling();

private:
    MediaFileInfo m_fileInfo;
    MediaFileInfo m_additionalFileInfo;
    Diagnostics m_diag;
    AbortableProgressFeedback m_progress;
    TagValue m_testTitle;
    TagValue m_testComment;
    TagValue m_testCommentWithoutDescription;
    TagValue m_testAlbum;
    TagValue m_testPartNumber;
    TagValue m_testTotalParts;
    TagValue m_testPosition;
    string m_testCover;
    queue<TagValue> m_preservedMetaData;
    TagStatus m_tagStatus;
    std::uint16_t m_mode;
    ElementPosition m_expectedTagPos;
    ElementPosition m_expectedIndexPos;
};

#endif // TAGPARSER_OVERALL_TESTS_H
