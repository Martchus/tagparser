#include "./helper.h"

#include "../id3/id3genres.h"
#include "../tagvalue.h"

#include <c++utilities/chrono/format.h>
#include <c++utilities/conversion/conversionexception.h>

using namespace CppUtilities;

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace CppUtilities;
using namespace TagParser;
using namespace CPPUNIT_NS;

/*!
 * \brief The TagValueTests class tests the TagParser::TagValue class.
 */
class TagValueTests : public TestFixture {
    CPPUNIT_TEST_SUITE(TagValueTests);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST(testBinary);
    CPPUNIT_TEST(testInteger);
    CPPUNIT_TEST(testPositionInSet);
    CPPUNIT_TEST(testTimeSpan);
    CPPUNIT_TEST(testDateTime);
    CPPUNIT_TEST(testString);
    CPPUNIT_TEST(testEqualityOperator);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testBasics();
    void testBinary();
    void testInteger();
    void testPositionInSet();
    void testTimeSpan();
    void testDateTime();
    void testString();
    void testEqualityOperator();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TagValueTests);

void TagValueTests::setUp()
{
}

void TagValueTests::tearDown()
{
}

void TagValueTests::testBasics()
{
    CPPUNIT_ASSERT(TagValue::empty().isEmpty());
    CPPUNIT_ASSERT_EQUAL(TagDataType::Undefined, TagValue().type());
}

void TagValueTests::testBinary()
{
    const TagValue binary("123", 3, TagDataType::Binary);
    CPPUNIT_ASSERT_EQUAL(TagDataType::Binary, binary.type());
    CPPUNIT_ASSERT_EQUAL("123"s, string(binary.dataPointer(), binary.dataSize()));
    CPPUNIT_ASSERT_THROW(binary.toString(), ConversionException);
    CPPUNIT_ASSERT_THROW(binary.toInteger(), ConversionException);
    CPPUNIT_ASSERT_THROW(binary.toPositionInSet(), ConversionException);
    CPPUNIT_ASSERT_THROW(binary.toStandardGenreIndex(), ConversionException);
}

void TagValueTests::testInteger()
{
    // positive number
    TagValue integer(42);
    CPPUNIT_ASSERT(!integer.isEmpty());
    CPPUNIT_ASSERT_EQUAL(TagDataType::Integer, integer.type());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(42), integer.toInteger());
    CPPUNIT_ASSERT_EQUAL("42"s, integer.toString());
    integer.assignInteger(2);
    CPPUNIT_ASSERT_EQUAL("Country"s, string(Id3Genres::stringFromIndex(integer.toStandardGenreIndex())));
    integer.assignInteger(Id3Genres::emptyGenreIndex());
    CPPUNIT_ASSERT_EQUAL(Id3Genres::emptyGenreIndex(), integer.toStandardGenreIndex());
    integer.clearData();
    CPPUNIT_ASSERT_EQUAL(Id3Genres::emptyGenreIndex(), integer.toStandardGenreIndex());

    // negative number
    integer.assignInteger(-25);
    CPPUNIT_ASSERT_EQUAL("-25"s, integer.toString());
    CPPUNIT_ASSERT_EQUAL(PositionInSet(-25), integer.toPositionInSet());
    CPPUNIT_ASSERT_THROW(integer.toStandardGenreIndex(), ConversionException);

    // zero
    integer.assignInteger(0);
    CPPUNIT_ASSERT_MESSAGE("explicitely assigned zero not considered empty", !integer.isEmpty());
    CPPUNIT_ASSERT_EQUAL("0"s, integer.toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(), integer.toDateTime());
    CPPUNIT_ASSERT_EQUAL(TimeSpan(), integer.toTimeSpan());

    // empty value treatet as zero when using to...() methods
    integer.clearData();
    CPPUNIT_ASSERT_MESSAGE("cleared vale considered empty", integer.isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("only date (but not type) cleared"s, TagDataType::Integer, integer.type());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(0), integer.toInteger());
    CPPUNIT_ASSERT_EQUAL(string(), integer.toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(), integer.toDateTime());
    CPPUNIT_ASSERT_EQUAL(TimeSpan(), integer.toTimeSpan());
}

void TagValueTests::testPositionInSet()
{
    const TagValue test(PositionInSet(4, 23));
    CPPUNIT_ASSERT_EQUAL(PositionInSet(4, 23), test.toPositionInSet());
    CPPUNIT_ASSERT_EQUAL(test.toInteger(), 4);
    CPPUNIT_ASSERT_EQUAL("4/23"s, test.toString());
    CPPUNIT_ASSERT_THROW(test.toStandardGenreIndex(), ConversionException);
    CPPUNIT_ASSERT_THROW(test.toDateTime(), ConversionException);
    CPPUNIT_ASSERT_THROW(test.toTimeSpan(), ConversionException);
}

void TagValueTests::testTimeSpan()
{
    const TimeSpan fiveMinutes(TimeSpan::fromMinutes(5));
    TagValue timeSpan;
    timeSpan.assignTimeSpan(fiveMinutes);
    CPPUNIT_ASSERT_EQUAL(timeSpan, TagValue(timeSpan));
    CPPUNIT_ASSERT_EQUAL(fiveMinutes, timeSpan.toTimeSpan());
    CPPUNIT_ASSERT_EQUAL(fiveMinutes.toString(), timeSpan.toString());
    CPPUNIT_ASSERT_THROW(timeSpan.toInteger(), ConversionException);
    CPPUNIT_ASSERT_THROW(timeSpan.toDateTime(), ConversionException);
    CPPUNIT_ASSERT_THROW(timeSpan.toPositionInSet(), ConversionException);
}

void TagValueTests::testDateTime()
{
    const DateTime now(DateTime::now());
    TagValue dateTime;
    dateTime.assignDateTime(now);
    CPPUNIT_ASSERT_EQUAL(dateTime, TagValue(dateTime));
    CPPUNIT_ASSERT_EQUAL(now, dateTime.toDateTime());
    CPPUNIT_ASSERT_EQUAL(now.toString(DateTimeOutputFormat::IsoOmittingDefaultComponents), dateTime.toString());
    CPPUNIT_ASSERT_THROW(dateTime.toInteger(), ConversionException);
    CPPUNIT_ASSERT_THROW(dateTime.toTimeSpan(), ConversionException);
    CPPUNIT_ASSERT_THROW(dateTime.toPositionInSet(), ConversionException);
}

void TagValueTests::testString()
{
    CPPUNIT_ASSERT_EQUAL("15\xe4"s, TagValue("15ä", 4, TagTextEncoding::Utf8).toString(TagTextEncoding::Latin1));
    CPPUNIT_ASSERT_EQUAL("15\xe4"s, TagValue("15ä", TagTextEncoding::Utf8, TagTextEncoding::Latin1).toString());
    CPPUNIT_ASSERT_EQUAL("15ä"s, TagValue("15ä", 4, TagTextEncoding::Utf8).toString(TagTextEncoding::Utf8));
    CPPUNIT_ASSERT_EQUAL("\x31\0\x35\0"s, TagValue(15).toString(TagTextEncoding::Utf16LittleEndian));
    CPPUNIT_ASSERT_EQUAL("\0\x31\0\x35"s, TagValue(15).toString(TagTextEncoding::Utf16BigEndian));
    CPPUNIT_ASSERT_EQUAL(15, TagValue("\0\x31\0\x35"s, TagTextEncoding::Utf16BigEndian).toInteger());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "original encoding preserved", "15ä"s, TagValue("15ä", 4, TagTextEncoding::Utf8).toString(TagTextEncoding::Unspecified));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("original encoding preserved", "\0\x31\0\x35"s,
        TagValue("\0\x31\0\x35", 4, TagTextEncoding::Utf16BigEndian).toString(TagTextEncoding::Unspecified));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "UTF-8 BOM truncated", "täst"s, TagValue("\xef\xbb\xbftäst", 8, TagTextEncoding::Utf8).toString(TagTextEncoding::Unspecified));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("UTF-16 LE BOM truncated", "\0t\0\xe4\0s\0t"s,
        TagValue("\xff\xfe\0t\0\xe4\0s\0t", 10, TagTextEncoding::Utf16LittleEndian).toString(TagTextEncoding::Unspecified));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("UTF-16 BE BOM truncated", "t\0\xe4\0s\0t\0"s,
        TagValue("\xfe\xfft\0\xe4\0s\0t\0", 10, TagTextEncoding::Utf16BigEndian).toString(TagTextEncoding::Unspecified));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion via c'tor", "15\xe4"s,
        TagValue("\xef\xbb\xbf\x31\x35ä", 7, TagTextEncoding::Utf8, TagTextEncoding::Latin1).toString(TagTextEncoding::Unspecified));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to int", -15, TagValue(" - 156", 5, TagTextEncoding::Utf8).toInteger());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to int", 15, TagValue("\0\x31\0\x35", 4, TagTextEncoding::Utf16BigEndian).toInteger());
    CPPUNIT_ASSERT_THROW_MESSAGE("failing conversion to int", TagValue("15ä", 4, TagTextEncoding::Utf8).toInteger(), ConversionException);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to pos", PositionInSet(4, 15), TagValue("4 / 15", 6, TagTextEncoding::Utf8).toPositionInSet());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "conversion to pos", PositionInSet(15), TagValue("\0\x31\0\x35", 4, TagTextEncoding::Utf16BigEndian).toPositionInSet());
    CPPUNIT_ASSERT_THROW_MESSAGE("failing conversion pos", TagValue("a4 / 15", 7, TagTextEncoding::Utf8).toPositionInSet(), ConversionException);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "conversion to date", DateTime::fromDate(2004, 4, 15), TagValue("2004-04-15", 10, TagTextEncoding::Utf8).toDateTime());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to date from UTF-16", DateTime::fromDate(2015, 4, 15),
        TagValue("\0\x32\0\x30\0\x31\0\x35\0\x2d\0\x30\0\x34\0\x2d\0\x31\0\x35", 20, TagTextEncoding::Utf16BigEndian).toDateTime());
    CPPUNIT_ASSERT_THROW_MESSAGE("failing conversion to date", TagValue("_", 1, TagTextEncoding::Utf8).toDateTime(), ConversionException);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to time span", TimeSpan::fromHours(1.5), TagValue("01:30:00", 10, TagTextEncoding::Utf8).toTimeSpan());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to time span from UTF-16", TimeSpan::fromHours(1.5),
        TagValue("\0\x31\0\x3a\0\x33\0\x30\0\x3a\0\x30\0\x30", 14, TagTextEncoding::Utf16BigEndian).toTimeSpan());
    CPPUNIT_ASSERT_THROW_MESSAGE("failing conversion to time span", TagValue("_", 1, TagTextEncoding::Utf8).toTimeSpan(), ConversionException);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "conversion to genre from index", 15, TagValue("\0\x31\0\x35", 4, TagTextEncoding::Utf16BigEndian).toStandardGenreIndex());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to genre from name", 2, TagValue("Country", 7, TagTextEncoding::Latin1).toStandardGenreIndex());
    CPPUNIT_ASSERT_THROW_MESSAGE(
        "failing conversion to genre", TagValue("Kountry", 7, TagTextEncoding::Latin1).toStandardGenreIndex(), ConversionException);
}

void TagValueTests::testEqualityOperator()
{
    CPPUNIT_ASSERT_MESSAGE("equality requires identical types or identical string representation"s, TagValue(0) != TagValue::empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("comparision of equal types"s, TagValue(15), TagValue(15));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("types might differ"s, TagValue("15", 2, TagTextEncoding::Latin1), TagValue(15));
    CPPUNIT_ASSERT_MESSAGE("but some types shall never be considered equal"s, TagValue(DateTime(0)) != TagValue(TimeSpan(0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("comparision of equal UTF-16 strings"s, TagValue("\x31\0\x32\0", 4, TagTextEncoding::Utf16LittleEndian),
        TagValue("\x31\0\x32\0", 4, TagTextEncoding::Utf16LittleEndian));
    CPPUNIT_ASSERT_MESSAGE("comparision of different UTF-16 strings"s,
        TagValue("\x31\0\x33\0", 4, TagTextEncoding::Utf16LittleEndian) != TagValue("\x31\0\x32\0", 4, TagTextEncoding::Utf16LittleEndian));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "comparision of equal binary data"s, TagValue("\x31\0\x32\0", 4, TagDataType::Binary), TagValue("\x31\0\x32\0", 4, TagDataType::Binary));
    CPPUNIT_ASSERT_MESSAGE("comparision of different binary data"s,
        TagValue("\x31\0\x33\0", 4, TagDataType::Binary) != TagValue("\x31\0\x32\0", 4, TagDataType::Binary));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("different encodings are converted if neccassary"s, TagValue("\0\x31\0\x35", 4, TagTextEncoding::Utf16BigEndian),
        TagValue("15", 2, TagTextEncoding::Latin1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "encoding is ignored when not relevant for types"s, TagValue("\0\x31\0\x35", 4, TagTextEncoding::Utf16BigEndian), TagValue(15));
    const TagValue fooTagValue("foo", 3, TagDataType::Text), fOoTagValue("fOo", 3, TagDataType::Text);
    CPPUNIT_ASSERT_MESSAGE("string comparison case-sensitive by default"s, fooTagValue != fOoTagValue);
    CPPUNIT_ASSERT_MESSAGE("case-insensitive string comparision"s, fooTagValue.compareTo(fOoTagValue, TagValueComparisionFlags::CaseInsensitive));

    // meta-data
    TagValue withDescription(15);
    withDescription.setDescription("test");
    CPPUNIT_ASSERT_MESSAGE("meta-data must be equal"s, withDescription != TagValue(15));
    CPPUNIT_ASSERT_MESSAGE("different meta-data ignored"s, withDescription.compareTo(TagValue(15), TagValueComparisionFlags::IgnoreMetaData));
    TagValue withDescription2(withDescription);
    CPPUNIT_ASSERT_EQUAL(withDescription, withDescription2);
    withDescription2.setMimeType("foo/bar");
    CPPUNIT_ASSERT(withDescription != withDescription2);
    withDescription.setMimeType(withDescription2.mimeType());
    CPPUNIT_ASSERT_EQUAL(withDescription, withDescription2);
    withDescription2.setDescription("Test");
    CPPUNIT_ASSERT_MESSAGE("meta-data case must match by default"s, withDescription != withDescription2);
    CPPUNIT_ASSERT_MESSAGE("meta-data case ignored"s, withDescription.compareTo(withDescription2, TagValueComparisionFlags::CaseInsensitive));
}
