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
    CPPUNIT_TEST(testUnsignedInteger);
    CPPUNIT_TEST(testPositionInSet);
    CPPUNIT_TEST(testTimeSpan);
    CPPUNIT_TEST(testDateTime);
    CPPUNIT_TEST(testDateTimeExpression);
    CPPUNIT_TEST(testPopularity);
    CPPUNIT_TEST(testString);
    CPPUNIT_TEST(testEqualityOperator);
    CPPUNIT_TEST(testPopularityScaling);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testBasics();
    void testBinary();
    void testInteger();
    void testUnsignedInteger();
    void testPositionInSet();
    void testTimeSpan();
    void testDateTime();
    void testDateTimeExpression();
    void testPopularity();
    void testString();
    void testEqualityOperator();
    void testPopularityScaling();
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
    auto integer = TagValue(42);
    CPPUNIT_ASSERT(!integer.isEmpty());
    CPPUNIT_ASSERT_EQUAL(TagDataType::Integer, integer.type());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(42), integer.toInteger());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(42), integer.toUnsignedInteger());
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
    CPPUNIT_ASSERT_MESSAGE("explicitly assigned zero not considered empty", !integer.isEmpty());
    CPPUNIT_ASSERT_EQUAL("0"s, integer.toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(), integer.toDateTime());
    CPPUNIT_ASSERT_EQUAL(TimeSpan(), integer.toTimeSpan());

    // empty value treatet as zero when using to...() methods
    integer.clearData();
    CPPUNIT_ASSERT_MESSAGE("cleared vale considered empty", integer.isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("only date (but not type) cleared"s, TagDataType::Integer, integer.type());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(0), integer.toInteger());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), integer.toUnsignedInteger());
    CPPUNIT_ASSERT_EQUAL(string(), integer.toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(), integer.toDateTime());
    CPPUNIT_ASSERT_EQUAL(TimeSpan(), integer.toTimeSpan());
}

void TagValueTests::testUnsignedInteger()
{
    auto unsignedInteger = TagValue(static_cast<std::uint64_t>(42ul));
    CPPUNIT_ASSERT(!unsignedInteger.isEmpty());
    CPPUNIT_ASSERT_EQUAL(TagDataType::UnsignedInteger, unsignedInteger.type());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(42), unsignedInteger.toInteger());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(42), unsignedInteger.toUnsignedInteger());
    CPPUNIT_ASSERT_EQUAL("42"s, unsignedInteger.toString());
    unsignedInteger.assignUnsignedInteger(2);
    CPPUNIT_ASSERT_EQUAL("Country"s, string(Id3Genres::stringFromIndex(unsignedInteger.toStandardGenreIndex())));
    unsignedInteger.assignInteger(Id3Genres::emptyGenreIndex());
    CPPUNIT_ASSERT_EQUAL(Id3Genres::emptyGenreIndex(), unsignedInteger.toStandardGenreIndex());
    unsignedInteger.clearData();
    CPPUNIT_ASSERT_EQUAL(Id3Genres::emptyGenreIndex(), unsignedInteger.toStandardGenreIndex());

    // zero
    unsignedInteger.assignInteger(0);
    CPPUNIT_ASSERT_MESSAGE("explicitly assigned zero not considered empty", !unsignedInteger.isEmpty());
    CPPUNIT_ASSERT_EQUAL("0"s, unsignedInteger.toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(), unsignedInteger.toDateTime());
    CPPUNIT_ASSERT_EQUAL(TimeSpan(), unsignedInteger.toTimeSpan());
}

void TagValueTests::testPositionInSet()
{
    const TagValue test(PositionInSet(4, 23));
    CPPUNIT_ASSERT_EQUAL(PositionInSet(4, 23), test.toPositionInSet());
    CPPUNIT_ASSERT_EQUAL(4, test.toInteger());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(4), test.toUnsignedInteger());
    CPPUNIT_ASSERT_EQUAL("4/23"s, test.toString());
    CPPUNIT_ASSERT_THROW(test.toStandardGenreIndex(), ConversionException);
    CPPUNIT_ASSERT_THROW(test.toDateTime(), ConversionException);
    CPPUNIT_ASSERT_THROW(test.toTimeSpan(), ConversionException);
}

void TagValueTests::testTimeSpan()
{
    const TimeSpan fiveMinutes(TimeSpan::fromMinutes(5.0));
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
    const auto now = DateTime::now();
    auto value = TagValue();
    value.assignDateTime(now);
    CPPUNIT_ASSERT_EQUAL(value, TagValue(value));
    CPPUNIT_ASSERT_EQUAL(now, value.toDateTime());
    CPPUNIT_ASSERT_EQUAL(now.toIsoString(), value.toString());
    CPPUNIT_ASSERT_THROW(value.toInteger(), ConversionException);
    CPPUNIT_ASSERT_THROW(value.toTimeSpan(), ConversionException);
    CPPUNIT_ASSERT_THROW(value.toPositionInSet(), ConversionException);
}

void TagValueTests::testDateTimeExpression()
{
    auto expr = DateTimeExpression::fromIsoString("2007");
    auto value = TagValue();
    value.assignDateTimeExpression(expr);
    CPPUNIT_ASSERT_EQUAL(value, TagValue(expr));
    CPPUNIT_ASSERT_EQUAL(expr.value, value.toDateTime());
    CPPUNIT_ASSERT_EQUAL(expr, value.toDateTimeExpression());
    CPPUNIT_ASSERT_EQUAL(expr.toIsoString(), value.toString());
    CPPUNIT_ASSERT_THROW(value.toInteger(), ConversionException);
    CPPUNIT_ASSERT_THROW(value.toTimeSpan(), ConversionException);
    CPPUNIT_ASSERT_THROW(value.toPositionInSet(), ConversionException);
}

void TagValueTests::testPopularity()
{
    const auto tagValue = TagValue(Popularity{ .user = "foo", .rating = 40.0, .playCounter = 123, .scale = TagType::VorbisComment });
    const auto popularity = tagValue.toPopularity();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to popularity (user)", "foo"s, popularity.user);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to popularity (rating)", 40.0, popularity.rating);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to popularity (play counter)", std::uint64_t(123), popularity.playCounter);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to popularity (scale)", TagType::VorbisComment, popularity.scale);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to string", "foo|40|123"s, tagValue.toString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to string (only rating)", "43"s, TagValue(Popularity{ .rating = 43 }).toString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to integer", 40, tagValue.toInteger());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to unsigned integer", static_cast<std::uint64_t>(40), tagValue.toUnsignedInteger());
    CPPUNIT_ASSERT_THROW_MESSAGE(
        "failing conversion to other type", TagValue("foo|bar"sv, TagTextEncoding::Latin1).toPopularity(), ConversionException);
    const auto scaledPopularity = tagValue.toScaledPopularity();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rating scaled to generic scale", 2.0, scaledPopularity.rating);
    CPPUNIT_ASSERT_THROW_MESSAGE(
        "failed to scale if no scaling for specified format defined", tagValue.toScaledPopularity(TagType::Mp4Tag), ConversionException);
}

void TagValueTests::testString()
{
    CPPUNIT_ASSERT_EQUAL("15\xe4"s, TagValue("15ä", 4, TagTextEncoding::Utf8).toString(TagTextEncoding::Latin1));
    CPPUNIT_ASSERT_EQUAL("15\xe4"s, TagValue("15ä", TagTextEncoding::Utf8, TagTextEncoding::Latin1).toString());
    CPPUNIT_ASSERT_EQUAL("15ä"s, TagValue("15ä", 4, TagTextEncoding::Utf8).toString(TagTextEncoding::Utf8));
    CPPUNIT_ASSERT_EQUAL("\x31\0\x35\0"s, TagValue(15).toString(TagTextEncoding::Utf16LittleEndian));
    CPPUNIT_ASSERT_EQUAL("\0\x31\0\x35"s, TagValue(15).toString(TagTextEncoding::Utf16BigEndian));
    CPPUNIT_ASSERT_EQUAL(15, TagValue("\0\x31\0\x35"s, TagTextEncoding::Utf16BigEndian).toInteger());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(15), TagValue("\0\x31\0\x35"s, TagTextEncoding::Utf16BigEndian).toUnsignedInteger());
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
        "conversion to date time", DateTime::fromDate(2004, 4, 15), TagValue("2004-04-15", 10, TagTextEncoding::Utf8).toDateTime());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to date time expression", DateTimeExpression::fromIsoString("2004-04"),
        TagValue("2004-04-15", 7, TagTextEncoding::Utf8).toDateTimeExpression());
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
    const auto popularity = TagValue("foo|42|123"sv).toPopularity();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to popularity (user)", "foo"s, popularity.user);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to popularity (rating)", 42.0, popularity.rating);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("conversion to popularity (play counter)", std::uint64_t(123), popularity.playCounter);
    CPPUNIT_ASSERT_THROW_MESSAGE("failing conversion to popularity", TagValue("foo|bar"sv).toPopularity(), ConversionException);
}

void TagValueTests::testEqualityOperator()
{
    CPPUNIT_ASSERT_MESSAGE("equality requires identical types or identical string representation"s, TagValue(0) != TagValue::empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("comparison of equal types"s, TagValue(15), TagValue(15));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("types might differ"s, TagValue("15", 2, TagTextEncoding::Latin1), TagValue(15));
    CPPUNIT_ASSERT_MESSAGE("but some types shall never be considered equal"s, TagValue(DateTime(0)) != TagValue(TimeSpan(0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("comparison of equal UTF-16 strings"s, TagValue("\x31\0\x32\0", 4, TagTextEncoding::Utf16LittleEndian),
        TagValue("\x31\0\x32\0", 4, TagTextEncoding::Utf16LittleEndian));
    CPPUNIT_ASSERT_MESSAGE("comparison of different UTF-16 strings"s,
        TagValue("\x31\0\x33\0", 4, TagTextEncoding::Utf16LittleEndian) != TagValue("\x31\0\x32\0", 4, TagTextEncoding::Utf16LittleEndian));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "comparison of equal binary data"s, TagValue("\x31\0\x32\0", 4, TagDataType::Binary), TagValue("\x31\0\x32\0", 4, TagDataType::Binary));
    CPPUNIT_ASSERT_MESSAGE(
        "comparison of different binary data"s, TagValue("\x31\0\x33\0", 4, TagDataType::Binary) != TagValue("\x31\0\x32\0", 4, TagDataType::Binary));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("different encodings are converted if neccassary"s, TagValue("\0\x31\0\x35", 4, TagTextEncoding::Utf16BigEndian),
        TagValue("15", 2, TagTextEncoding::Latin1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "encoding is ignored when not relevant for types"s, TagValue("\0\x31\0\x35", 4, TagTextEncoding::Utf16BigEndian), TagValue(15));
    const TagValue fooTagValue("foo", 3, TagDataType::Text), fOoTagValue("fOo", 3, TagDataType::Text);
    CPPUNIT_ASSERT_MESSAGE("string comparison case-sensitive by default"s, fooTagValue != fOoTagValue);
    CPPUNIT_ASSERT_MESSAGE("case-insensitive string comparison"s, fooTagValue.compareTo(fOoTagValue, TagValueComparisionFlags::CaseInsensitive));
    const auto popularity = Popularity{ .user = "some user", .rating = 200, .playCounter = 0 };
    const auto first = TagValue(popularity), second = TagValue(popularity);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("comparison of equal popularity (string and binary representation)"s, TagValue("some user|200.0"sv), first);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("comparison of equal popularity (only binary representation)"s, first, second);
    CPPUNIT_ASSERT_MESSAGE("default-popularity not equal to empty tag value"s, TagValue(Popularity()) != TagValue());
    CPPUNIT_ASSERT_MESSAGE("popularity not equal"s, first != TagValue(Popularity({ .rating = 200 })));

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

void TagValueTests::testPopularityScaling()
{
    const auto genericZero = Popularity{ .rating = 0.0, .scale = TagType::Unspecified };
    const auto genericMin = Popularity{ .rating = 1.0, .scale = TagType::Unspecified };
    const auto genericMax = Popularity{ .rating = 5.0, .scale = TagType::Unspecified };
    const auto genericMiddle = Popularity{ .rating = 3.0, .scale = TagType::Unspecified };
    const auto id3zero = Popularity{ .rating = 0.0, .scale = TagType::Id3v2Tag };
    const auto id3min = Popularity{ .rating = 1.0, .scale = TagType::Id3v2Tag };
    const auto id3max = Popularity{ .rating = 255.0, .scale = TagType::Id3v2Tag };
    const auto id3middle = Popularity{ .rating = 128.0, .scale = TagType::Id3v2Tag };
    const auto vorbisZero = Popularity{ .rating = 0.0, .scale = TagType::VorbisComment };
    const auto vorbisMin = Popularity{ .rating = 20.0, .scale = TagType::VorbisComment };
    const auto vorbisMax = Popularity{ .rating = 100.0, .scale = TagType::OggVorbisComment };
    const auto vorbisMiddle = Popularity{ .rating = 60.0, .scale = TagType::OggVorbisComment };
    const auto mkvMin = Popularity{ .rating = 0.0, .scale = TagType::MatroskaTag };
    const auto mkvMax = Popularity{ .rating = 5.0, .scale = TagType::MatroskaTag };
    const auto mkvMiddle = Popularity{ .rating = 2.5, .scale = TagType::MatroskaTag };
    for (const auto &rawZero : { id3zero, vorbisZero }) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("zero: raw to generic", genericZero.rating, rawZero.scaled(TagType::Unspecified).rating);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("zero: generic to raw ", rawZero.rating, genericZero.scaled(rawZero.scale).rating);
    }
    for (const auto &rawMin : { id3min, vorbisMin, mkvMin }) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("min: raw to generic", genericMin.rating, rawMin.scaled(TagType::Unspecified).rating);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("min: generic to raw ", rawMin.rating, genericMin.scaled(rawMin.scale).rating);
    }
    for (const auto &rawMax : { id3max, vorbisMax, mkvMax }) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("max: raw to generic", genericMax.rating, rawMax.scaled(TagType::Unspecified).rating);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("max: generic to raw ", rawMax.rating, genericMax.scaled(rawMax.scale).rating);
    }
    for (const auto &rawMiddle : { id3middle, vorbisMiddle, mkvMiddle }) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("middle: raw to generic", genericMiddle.rating, rawMiddle.scaled(TagType::Unspecified).rating);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("middle: generic to raw ", rawMiddle.rating, genericMiddle.scaled(rawMiddle.scale).rating);
    }
}
