#include "./helper.h"

#include "../aspectratio.h"
#include "../backuphelper.h"
#include "../diagnostics.h"
#include "../exceptions.h"
#include "../margin.h"
#include "../mediafileinfo.h"
#include "../mediaformat.h"
#include "../positioninset.h"
#include "../progressfeedback.h"
#include "../signature.h"
#include "../size.h"
#include "../tagtarget.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/tests/testutils.h>
using namespace CppUtilities;

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstdio>
#include <filesystem>
#include <regex>

using namespace std;
using namespace CppUtilities::Literals;
using namespace TagParser;
using namespace CPPUNIT_NS;

/*!
 * \brief The UtilitiesTests class tests various utility classes and functions of the tagparser library.
 */
class UtilitiesTests : public TestFixture {
    CPPUNIT_TEST_SUITE(UtilitiesTests);
    CPPUNIT_TEST(testSize);
    CPPUNIT_TEST(testTagTarget);
    CPPUNIT_TEST(testSignature);
    CPPUNIT_TEST(testMargin);
    CPPUNIT_TEST(testAspectRatio);
    CPPUNIT_TEST(testMediaFormat);
    CPPUNIT_TEST(testPositionInSet);
    CPPUNIT_TEST(testProgressFeedback);
    CPPUNIT_TEST(testAbortableProgressFeedback);
    CPPUNIT_TEST(testDiagnostics);
    CPPUNIT_TEST(testBackupFile);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testSize();
    void testStatusProvider();
    void testTagTarget();
    void testSignature();
    void testMargin();
    void testAspectRatio();
    void testMediaFormat();
    void testPositionInSet();
    void testProgressFeedback();
    void testAbortableProgressFeedback();
    void testDiagnostics();
    void testBackupFile();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UtilitiesTests);

void UtilitiesTests::setUp()
{
}

void UtilitiesTests::tearDown()
{
}

void UtilitiesTests::testSize()
{
    static_assert(Size().isNull(), "Size::isNull()");
    static_assert(!Size(3, 4).isNull(), "Size::isNull()");
    static_assert(Size(3, 4).resolution() == 12, "Size::resolution");

    Size size(1920, 1080);
    CPPUNIT_ASSERT_EQUAL("width: 1920, height: 1080"s, size.toString());
    CPPUNIT_ASSERT_EQUAL("1080p"s, string(size.abbreviation()));
    size.setWidth(1280);
    size.setHeight(720);
    CPPUNIT_ASSERT_EQUAL("720p"s, string(size.abbreviation()));
}

void UtilitiesTests::testTagTarget()
{
    TagTarget target;
    CPPUNIT_ASSERT(target.isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("default level is 50", static_cast<std::uint64_t>(50), target.level());
    CPPUNIT_ASSERT_EQUAL("level 50"s, target.toString(TagTargetLevel::Unspecified));
    target = TagTarget(30, { 1, 2, 3 }, { 4 }, { 5, 6 }, { 7, 8, 9 });
    CPPUNIT_ASSERT(!target.isEmpty());
    const auto mapping = [](std::uint64_t level) { return level == 30 ? TagTargetLevel::Track : TagTargetLevel::Unspecified; };
    CPPUNIT_ASSERT_EQUAL(
        "level 30 'track, song, chapter', track 1, track 2, track 3, chapter 4, edition 5, edition 6, attachment  7, attachment  8, attachment  9"s,
        target.toString(mapping));
    target.setLevel(40);
    CPPUNIT_ASSERT_EQUAL("level 40, track 1, track 2, track 3, chapter 4, edition 5, edition 6, attachment  7, attachment  8, attachment  9"s,
        target.toString(mapping));
    target.setLevelName("test");
    CPPUNIT_ASSERT_EQUAL("level 40 'test', track 1, track 2, track 3, chapter 4, edition 5, edition 6, attachment  7, attachment  8, attachment  9"s,
        target.toString(mapping));
    CPPUNIT_ASSERT(target == TagTarget(40, { 1, 2, 3 }, { 4 }, { 5, 6 }, { 7, 8, 9 }));
    target.clear();
    CPPUNIT_ASSERT(target.isEmpty());
}

void UtilitiesTests::testSignature()
{
    const unsigned char xzHead[12] = { 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6, 0xd6, 0xb4, 0x46 };

    // truncated buffer
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Unknown, parseSignature(reinterpret_cast<const char *>(xzHead), 3));
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Unknown, parseSignature(reinterpret_cast<const char *>(xzHead), 2));
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Unknown, parseSignature(reinterpret_cast<const char *>(xzHead), 0));

    const auto containerFormat = parseSignature(reinterpret_cast<const char *>(xzHead), sizeof(xzHead));
    CPPUNIT_ASSERT_EQUAL(ContainerFormat::Xz, containerFormat);
    CPPUNIT_ASSERT_EQUAL("xz compressed file"s, string(containerFormatName(containerFormat)));
    CPPUNIT_ASSERT_EQUAL("xz"s, string(containerFormatAbbreviation(containerFormat)));
    CPPUNIT_ASSERT_EQUAL(string(), string(containerFormatSubversion(containerFormat)));
}

void UtilitiesTests::testMargin()
{
    static_assert(Margin().isNull(), "empty margin");
    static_assert(!Margin(0, 2).isNull(), "non-empty margin");

    CPPUNIT_ASSERT_EQUAL("top: 1; left: 2; bottom: 3; right: 4"s, Margin(1, 2, 3, 4).toString());
}

void UtilitiesTests::testAspectRatio()
{
    static_assert(!AspectRatio().isValid(), "invalid aspect ratio");
    static_assert(AspectRatio(16, 9).isValid(), "valid aspect ratio");
    static_assert(AspectRatio(16, 9).isExtended(), "extended aspect ratio");

    const AspectRatio ratio(4);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(16), ratio.numerator);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(11), ratio.denominator);
    const AspectRatio ratio2(77);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0), ratio2.numerator);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0), ratio2.denominator);
}

void UtilitiesTests::testMediaFormat()
{
    // unspecific format
    MediaFormat aac(GeneralMediaFormat::Aac);
    CPPUNIT_ASSERT_EQUAL("Advanced Audio Coding"s, string(aac.name()));
    CPPUNIT_ASSERT_EQUAL("AAC"s, string(aac.abbreviation()));
    CPPUNIT_ASSERT_EQUAL("AAC"s, string(aac.shortAbbreviation()));

    // specific format
    aac += MediaFormat(GeneralMediaFormat::Aac, SubFormats::AacMpeg4LowComplexityProfile, ExtensionFormats::SpectralBandReplication);
    CPPUNIT_ASSERT(aac == GeneralMediaFormat::Aac);
    CPPUNIT_ASSERT(aac != GeneralMediaFormat::Mpeg1Audio);
    CPPUNIT_ASSERT_EQUAL("Advanced Audio Coding Low Complexity Profile"s, string(aac.name()));
    CPPUNIT_ASSERT_EQUAL("MPEG-4 AAC-LC"s, string(aac.abbreviation()));
    CPPUNIT_ASSERT_EQUAL("HE-AAC"s, string(aac.shortAbbreviation()));
    CPPUNIT_ASSERT_EQUAL("Spectral Band Replication / HE-AAC"s, string(aac.extensionName()));
}

void UtilitiesTests::testPositionInSet()
{
    const PositionInSet empty;
    CPPUNIT_ASSERT(empty.isNull());
    CPPUNIT_ASSERT_EQUAL(0, empty.position());
    CPPUNIT_ASSERT_EQUAL(0, empty.total());
    CPPUNIT_ASSERT_EQUAL(""s, empty.toString());

    const PositionInSet oneOfThree(1, 3);
    CPPUNIT_ASSERT(!oneOfThree.isNull());
    CPPUNIT_ASSERT_EQUAL(1, oneOfThree.position());
    CPPUNIT_ASSERT_EQUAL(3, oneOfThree.total());
    CPPUNIT_ASSERT_EQUAL("1/3"s, oneOfThree.toString());

    const PositionInSet posOnly(5, 0);
    CPPUNIT_ASSERT(!posOnly.isNull());
    CPPUNIT_ASSERT_EQUAL(5, posOnly.position());
    CPPUNIT_ASSERT_EQUAL(0, posOnly.total());
    CPPUNIT_ASSERT_EQUAL("5"s, posOnly.toString());

    const PositionInSet totalOnly(0, 5);
    CPPUNIT_ASSERT(!totalOnly.isNull());
    CPPUNIT_ASSERT_EQUAL(0, totalOnly.position());
    CPPUNIT_ASSERT_EQUAL(5, totalOnly.total());
    CPPUNIT_ASSERT_EQUAL("/5"s, totalOnly.toString());
}

void UtilitiesTests::testProgressFeedback()
{
    unsigned int steps = 0;
    string step;
    unsigned int stepPercentage;
    unsigned int overallPercentage = 0;

    ProgressFeedback progressFeedback(
        [&](const ProgressFeedback &progress) {
            ++steps;
            step = progress.step();
            stepPercentage = progress.stepPercentage();
            overallPercentage = progress.overallPercentage();
        },
        [&](const ProgressFeedback &progress) {
            stepPercentage = progress.stepPercentage();
            overallPercentage = progress.overallPercentage();
        });
    CPPUNIT_ASSERT_EQUAL(0u, steps);
    progressFeedback.updateOverallPercentage(25);
    CPPUNIT_ASSERT_EQUAL(0u, steps);
    CPPUNIT_ASSERT_EQUAL(25u, overallPercentage);
    progressFeedback.updateStep("foo", 45);
    CPPUNIT_ASSERT_EQUAL(1u, steps);
    CPPUNIT_ASSERT_EQUAL("foo"s, step);
    CPPUNIT_ASSERT_EQUAL(45u, stepPercentage);
    CPPUNIT_ASSERT_EQUAL(25u, overallPercentage);
    progressFeedback.updateStepPercentage(60);
    CPPUNIT_ASSERT_EQUAL(1u, steps);
    CPPUNIT_ASSERT_EQUAL("foo"s, step);
    CPPUNIT_ASSERT_EQUAL(60u, stepPercentage);
    CPPUNIT_ASSERT_EQUAL(25u, overallPercentage);
    progressFeedback.updateStepPercentageFromFraction(0.75);
    CPPUNIT_ASSERT_EQUAL(1u, steps);
    CPPUNIT_ASSERT_EQUAL("foo"s, step);
    CPPUNIT_ASSERT_EQUAL(75u, stepPercentage);
    CPPUNIT_ASSERT_EQUAL(25u, overallPercentage);
}

void UtilitiesTests::testAbortableProgressFeedback()
{
    unsigned int steps = 0;
    string step;
    unsigned int stepPercentage;
    unsigned int overallPercentage = 0;

    AbortableProgressFeedback progressFeedback(
        [&](const AbortableProgressFeedback &progress) {
            ++steps;
            step = progress.step();
            stepPercentage = progress.stepPercentage();
            overallPercentage = progress.overallPercentage();
        },
        [&](const AbortableProgressFeedback &progress) {
            stepPercentage = progress.stepPercentage();
            overallPercentage = progress.overallPercentage();
        });
    CPPUNIT_ASSERT(!progressFeedback.isAborted());
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("stop does nothing if not aborted", progressFeedback.stopIfAborted());
    CPPUNIT_ASSERT_EQUAL(0u, steps);
    progressFeedback.updateOverallPercentage(25);
    CPPUNIT_ASSERT_EQUAL(0u, steps);
    CPPUNIT_ASSERT_EQUAL(25u, overallPercentage);
    progressFeedback.updateStep("foo", 45);
    CPPUNIT_ASSERT_EQUAL(1u, steps);
    CPPUNIT_ASSERT_EQUAL("foo"s, step);
    CPPUNIT_ASSERT_EQUAL(45u, stepPercentage);
    CPPUNIT_ASSERT_EQUAL(25u, overallPercentage);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("next step continues if not aborted", progressFeedback.nextStepOrStop("bar", 33));
    CPPUNIT_ASSERT_EQUAL(2u, steps);
    CPPUNIT_ASSERT_EQUAL("bar"s, step);
    CPPUNIT_ASSERT_EQUAL(33u, stepPercentage);
    CPPUNIT_ASSERT_EQUAL(25u, overallPercentage);
    progressFeedback.tryToAbort();
    CPPUNIT_ASSERT(progressFeedback.isAborted());
    CPPUNIT_ASSERT_THROW(progressFeedback.nextStepOrStop("not going to happen", 33), OperationAbortedException);
    CPPUNIT_ASSERT_EQUAL(2u, steps);
    CPPUNIT_ASSERT_EQUAL("bar"s, step);
    CPPUNIT_ASSERT_EQUAL(33u, stepPercentage);
    CPPUNIT_ASSERT_EQUAL(25u, overallPercentage);
}

void UtilitiesTests::testDiagnostics()
{
    Diagnostics diag;
    CPPUNIT_ASSERT_EQUAL(DiagLevel::None, diag.level());
    diag.emplace_back(DiagLevel::Warning, "warning msg", "context");
    CPPUNIT_ASSERT_EQUAL(DiagLevel::Warning, diag.level());
    CPPUNIT_ASSERT(!diag.has(DiagLevel::Critical));
    diag.emplace_back(DiagLevel::Critical, "critical msg", "context");
    CPPUNIT_ASSERT_EQUAL(DiagLevel::Critical, diag.level());
    CPPUNIT_ASSERT(diag.has(DiagLevel::Critical));
}

void UtilitiesTests::testBackupFile()
{
    using namespace BackupHelper;

    // setup testfile
    MediaFileInfo file(workingCopyPath("unsupported.bin"));
    file.setBackupDirectory(string()); // ensure backup directory is empty, so backups will be created in the same directory as the original file
    const auto workingDir(file.containingDirectory());
    file.open();

    // create backup file
    string backupPath1, backupPath2;
    NativeFileStream backupStream1, backupStream2;
    createBackupFile(string(), file.path(), backupPath1, file.stream(), backupStream1);
    CPPUNIT_ASSERT_EQUAL(workingDir + "/unsupported.bin.bak", backupPath1);

    // recreate original file (like the 'make' methods would do to apply changes)
    file.stream().open(file.path(), ios_base::out);
    file.stream() << "test1" << endl;

    // create a 2nd backup which should not override the first one
    createBackupFile(string(), file.path(), backupPath2, file.stream(), backupStream2);
    CPPUNIT_ASSERT_EQUAL(workingDir + "/unsupported.bin.1.bak", backupPath2);

    // get rid of 2nd backup, recreate original file
    backupStream2.close();
    remove(backupPath2.data());
    file.stream().open(file.path(), ios_base::out);
    file.stream() << "test2" << endl;

    // create backup under another location
    try {
        createBackupFile("bak", file.path(), backupPath2, file.stream(), backupStream2);
        CPPUNIT_FAIL("renaming failed because backup dir does not exist");
    } catch (const std::ios_base::failure &failure) {
        TESTUTILS_ASSERT_LIKE("renaming error", "Unable to create backup file .* of .* before rewriting it: .*"s, string(failure.what()));
    }
    backupStream2.clear();
    workingCopyPath("bak/unsupported.bin", WorkingCopyMode::NoCopy);
    createBackupFile("bak", file.path(), backupPath2, file.stream(), backupStream2);
    CPPUNIT_ASSERT_EQUAL(workingDir + "/bak/unsupported.bin", backupPath2);

    // get rid of 2nd backup (again)
    backupStream2.close();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("remove " + backupPath2, 0, remove(backupPath2.data()));
    std::filesystem::remove_all(workingDir + "/bak");

    // should be able to use backup stream, eg. seek to the end
    backupStream1.seekg(0, ios_base::end);
    CPPUNIT_ASSERT_EQUAL(41_st, static_cast<size_t>(backupStream1.tellg()));

    // restore backup
    restoreOriginalFileFromBackupFile(file.path(), backupPath1, file.stream(), backupStream1);

    // check restored backup
    file.open(true);
    file.stream().seekg(0x1D);
    CPPUNIT_ASSERT_EQUAL(0x34_st, static_cast<size_t>(file.stream().get()));
    file.close();

    // restore after user aborted
    createBackupFile(string(), file.path(), backupPath1, file.stream(), backupStream1);
    try {
        throw OperationAbortedException();
    } catch (...) {
        Diagnostics diag;
        CPPUNIT_ASSERT_THROW(
            handleFailureAfterFileModified(file, backupPath1, file.stream(), backupStream1, diag, "test"), OperationAbortedException);
        CPPUNIT_ASSERT(diag.level() < DiagLevel::Critical);
        CPPUNIT_ASSERT(!diag.empty());
        CPPUNIT_ASSERT_EQUAL("Rewriting the file to apply changed tag information has been aborted."s, diag.front().message());
        CPPUNIT_ASSERT_EQUAL("The original file has been restored."s, diag.back().message());
    }

    // restore after error
    createBackupFile(string(), file.path(), backupPath1, file.stream(), backupStream1);
    try {
        throw Failure();
    } catch (...) {
        Diagnostics diag;
        CPPUNIT_ASSERT_THROW(handleFailureAfterFileModified(file, backupPath1, file.stream(), backupStream1, diag, "test"), Failure);
        CPPUNIT_ASSERT(diag.level() >= DiagLevel::Critical);
        CPPUNIT_ASSERT_EQUAL("Rewriting the file to apply changed tag information failed."s, diag.front().message());
        CPPUNIT_ASSERT_EQUAL("The original file has been restored."s, diag.back().message());
    }

    // restore after io failure
    createBackupFile(string(), file.path(), backupPath1, file.stream(), backupStream1);
    try {
        throw std::ios_base::failure("simulated IO failure");
    } catch (const std::ios_base::failure &) {
        Diagnostics diag;
        CPPUNIT_ASSERT_THROW_MESSAGE("IO failure re-thrown",
            handleFailureAfterFileModified(file, backupPath1, file.stream(), backupStream1, diag, "test"), std::ios_base::failure);
        CPPUNIT_ASSERT(diag.level() >= DiagLevel::Critical);
        CPPUNIT_ASSERT_EQUAL("An IO error occurred when rewriting the file to apply changed tag information."s, diag.front().message());
        CPPUNIT_ASSERT_EQUAL("The original file has been restored."s, diag.back().message());
    }

    CPPUNIT_ASSERT_EQUAL(0, remove(file.path().data()));
}
