#include <c++utilities/tests/testutils.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

using namespace CPPUNIT_NS;

class MatroskaTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(MatroskaTests);
    CPPUNIT_TEST(testParsing);
    CPPUNIT_TEST(testMaking);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testParsing();
    void testMaking();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MatroskaTests);

void MatroskaTests::setUp()
{}

void MatroskaTests::tearDown()
{}

/*!
 * \brief Tests the Matroska parser via MediaFileInfo.
 */
void MatroskaTests::testParsing()
{
    cerr << TestUtilities::workingCopyPath("matroska/test1.mkv") << endl;
}

/*!
 * \brief Tests the Matroska maker via MediaFileInfo.
 */
void MatroskaTests::testMaking()
{

}
