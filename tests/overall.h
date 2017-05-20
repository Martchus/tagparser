#ifndef TAGPARSER_OVERALL_TESTS_H
#define TAGPARSER_OVERALL_TESTS_H

#include "../mediafileinfo.h"
#include "../tagvalue.h"

#include <c++utilities/tests/testutils.h>
#include <c++utilities/conversion/stringconversion.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <string>
#include <queue>

using namespace std;
using namespace ConversionUtilities;
using namespace IoUtilities;
using namespace TestUtilities;
using namespace Media;

using namespace CPPUNIT_NS;

enum class TagStatus
{
    Original,
    TestMetaDataPresent,
    Removed
};

namespace SimpleTestFlags {
enum TestFlag
{
    RemoveTag = 0x1,
};
}

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
    CPPUNIT_TEST(testMkvMakingWithDifferentSettings);
    CPPUNIT_TEST(testMkvMakingNestedTags);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

private:
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
    void checkMkvTestfileHandbrakeChapters();
    void checkMkvTestfileNestedTags();
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
    void noop();
    void createMkvWithNestedTags();
    void createFlacFiles();

public:
    void testMkvParsing();
    void testMp4Parsing();
    void testMp3Parsing();
    void testOggParsing();
    void testFlacParsing();
#ifdef PLATFORM_UNIX
    void testMkvMakingWithDifferentSettings();
    void testMkvMakingNestedTags();
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
    string m_testCover;
    queue<TagValue> m_preservedMetaData;
    string m_nestedTagsMkvPath;
    string m_rawFlacPath;
    string m_flacInOggPath;
    TagStatus m_tagStatus;
    uint16 m_mode;
};

#endif // TAGPARSER_OVERALL_TESTS_H
