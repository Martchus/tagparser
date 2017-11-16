#include "./helper.h"

#include "../size.h"
#include "../statusprovider.h"
#include "../tagtarget.h"
#include "../signature.h"
#include "../margin.h"
#include "../aspectratio.h"
#include "../mediaformat.h"
#include "../mediafileinfo.h"
#include "../exceptions.h"
#include "../backuphelper.h"

#include <c++utilities/io/catchiofailure.h>
#include <c++utilities/tests/testutils.h>
using namespace TestUtilities;

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstdio>

using namespace std;
using namespace Media;
using namespace IoUtilities;
using namespace TestUtilities::Literals;

using namespace CPPUNIT_NS;

/*!
 * \brief The TestStatusProvider class helps testing the StatusProvider class.
 */
class TestStatusProvider : public StatusProvider
{
public:
    TestStatusProvider();
};

TestStatusProvider::TestStatusProvider()
{
}

/*!
 * \brief The UtilitiesTests class tests various utility classes and functions of the tagparser library.
 */
class UtilitiesTests : public TestFixture {
    CPPUNIT_TEST_SUITE(UtilitiesTests);
    CPPUNIT_TEST(testSize);
    CPPUNIT_TEST(testStatusProvider);
    CPPUNIT_TEST(testTagTarget);
    CPPUNIT_TEST(testSignature);
    CPPUNIT_TEST(testMargin);
    CPPUNIT_TEST(testAspectRatio);
    CPPUNIT_TEST(testMediaFormat);
#ifdef PLATFORM_UNIX
    CPPUNIT_TEST(testBackupFile);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testSize();
    void testStatusProvider();
    void testTagTarget();
    void testSignature();
    void testMargin();
    void testAspectRatio();
    void testMediaFormat();
#ifdef PLATFORM_UNIX
    void testBackupFile();
#endif
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

void UtilitiesTests::testStatusProvider()
{
    const string context("unit tests");
    TestStatusProvider status, status2;

    // notifications
    CPPUNIT_ASSERT(!status.hasNotifications());
    CPPUNIT_ASSERT_EQUAL(NotificationType::None, status.worstNotificationType());
    status.addNotification(NotificationType::Debug, "debug notification", context);
    CPPUNIT_ASSERT_EQUAL(NotificationType::Debug, status.worstNotificationType());
    CPPUNIT_ASSERT(!status.hasCriticalNotifications());
    status.addNotification(NotificationType::Warning, "warning", context);
    CPPUNIT_ASSERT_EQUAL(NotificationType::Warning, status.worstNotificationType());
    CPPUNIT_ASSERT_EQUAL("warning"s, status.notifications().back().message());
    CPPUNIT_ASSERT(!status.hasCriticalNotifications());
    status.addNotification(NotificationType::Critical, "error", context);
    CPPUNIT_ASSERT_EQUAL(NotificationType::Critical, status.worstNotificationType());
    CPPUNIT_ASSERT(status.hasCriticalNotifications());
    CPPUNIT_ASSERT_EQUAL(3_st, status.notifications().size());
    CPPUNIT_ASSERT(status.hasNotifications());
    status2.addNotifications(status);
    status.invalidateNotifications();
    CPPUNIT_ASSERT(!status.hasNotifications());
    CPPUNIT_ASSERT(!status.hasCriticalNotifications());
    CPPUNIT_ASSERT_EQUAL(3_st, status2.notifications().size());
    status.addNotification(status2.notifications().back());
    CPPUNIT_ASSERT(status.hasCriticalNotifications());

    // status and percentage
    CPPUNIT_ASSERT_EQUAL(string(), status.currentStatus());
    CPPUNIT_ASSERT_EQUAL(0.0, status.currentPercentage());
    CPPUNIT_ASSERT(!status.isAborted());
    bool statusUpdateReceived = false, firstStatusUpdate = true;
    const auto callbackId = status.registerCallback([&status, &statusUpdateReceived, &firstStatusUpdate] (StatusProvider &sender) {
        CPPUNIT_ASSERT(&status == &sender);
        if(firstStatusUpdate) {
            CPPUNIT_ASSERT_EQUAL("test"s, sender.currentStatus());
            CPPUNIT_ASSERT_EQUAL(0.5, sender.currentPercentage());
            firstStatusUpdate = false;
        }
        sender.tryToAbort();
        statusUpdateReceived = true;
    });
    status.updateStatus("test", 0.5);
    CPPUNIT_ASSERT_MESSAGE("status update for updated status received", statusUpdateReceived);
    CPPUNIT_ASSERT(status.isAborted());
    statusUpdateReceived = false;
    status.updatePercentage(0.625);
    CPPUNIT_ASSERT_MESSAGE("status update for updated percentage received", statusUpdateReceived);
    statusUpdateReceived = false;
    status.addNotification(status2.notifications().front());
    CPPUNIT_ASSERT_MESSAGE("status update for new notification received", statusUpdateReceived);
    statusUpdateReceived = false;
    status.unregisterCallback(callbackId);
    status.updatePercentage(0.65);
    CPPUNIT_ASSERT_MESSAGE("no status update received after callback unregistered", !statusUpdateReceived);

    // forwarding
    TestStatusProvider forwardReceiver;
    status.forwardStatus(&forwardReceiver);
    statusUpdateReceived = false;
    forwardReceiver.registerCallback([&status, &statusUpdateReceived] (StatusProvider &sender) {
        CPPUNIT_ASSERT(&status == &sender);
        CPPUNIT_ASSERT_EQUAL("test2"s, sender.currentStatus());
        CPPUNIT_ASSERT_EQUAL(0.75, sender.currentPercentage());
        statusUpdateReceived = true;
    });
    status.updateStatus("test2", 0.75);
    CPPUNIT_ASSERT(statusUpdateReceived);
}

void UtilitiesTests::testTagTarget()
{
    TagTarget target;
    CPPUNIT_ASSERT(target.isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("default level is 50", 50ul, target.level());
    CPPUNIT_ASSERT_EQUAL("level 50"s, target.toString(TagTargetLevel::Unspecified));
    target = TagTarget(30, {1, 2, 3}, {4}, {5, 6}, {7, 8, 9});
    CPPUNIT_ASSERT(!target.isEmpty());
    const auto mapping = [] (uint64 level) {
        return level == 30 ? TagTargetLevel::Track : TagTargetLevel::Unspecified;
    };
    CPPUNIT_ASSERT_EQUAL("level 30 'track, song, chapter', track 1, track 2, track 3, chapter 4, edition 5, edition 6, attachment  7, attachment  8, attachment  9"s, target.toString(mapping));
    target.setLevel(40);
    CPPUNIT_ASSERT_EQUAL("level 40, track 1, track 2, track 3, chapter 4, edition 5, edition 6, attachment  7, attachment  8, attachment  9"s, target.toString(mapping));
    target.setLevelName("test");
    CPPUNIT_ASSERT_EQUAL("level 40 'test', track 1, track 2, track 3, chapter 4, edition 5, edition 6, attachment  7, attachment  8, attachment  9"s, target.toString(mapping));
    CPPUNIT_ASSERT(target == TagTarget(40, {1, 2, 3}, {4}, {5, 6}, {7, 8, 9}));
    target.clear();
    CPPUNIT_ASSERT(target.isEmpty());

}

void UtilitiesTests::testSignature()
{
    const unsigned char xzHead[12] = {
        0xfd, 0x37, 0x7a, 0x58,
        0x5a, 0x00, 0x00, 0x04,
        0xe6, 0xd6, 0xb4, 0x46
    };

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
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16>(16), ratio.numerator);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16>(11), ratio.denominator);
    const AspectRatio ratio2(77);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16>(0), ratio2.numerator);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16>(0), ratio2.denominator);
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

#ifdef PLATFORM_UNIX
void UtilitiesTests::testBackupFile()
{
    using namespace BackupHelper;

    // ensure backup directory is empty, so backups will be created in the same directory
    // as the original file
    backupDirectory().clear();

    // setup testfile
    MediaFileInfo file(workingCopyPath("unsupported.bin"));
    const string workingDir(file.containingDirectory());
    file.open();

    // create backup file
    string backupPath1, backupPath2;
    NativeFileStream backupStream1, backupStream2;
    createBackupFile(file.path(), backupPath1, file.stream(), backupStream1);
    CPPUNIT_ASSERT_EQUAL(workingDir + "/unsupported.bin.bak", backupPath1);

    // recreate original file
    file.stream().open(file.path(), ios_base::out);
    file.stream() << "test1" << endl;

    // create a 2nd backup which should not override the first one
    createBackupFile(file.path(), backupPath2, file.stream(), backupStream2);
    CPPUNIT_ASSERT_EQUAL(workingDir + "/unsupported.bin.1.bak", backupPath2);

    // get rid of 2nd backup, recreate original file
    backupStream2.close();
    remove(backupPath2.data());
    file.stream().open(file.path(), ios_base::out);
    file.stream() << "test2" << endl;

    // create backup under another location
    backupDirectory() = workingDir + "/bak";
    try {
        createBackupFile(file.path(), backupPath2, file.stream(), backupStream2);
        CPPUNIT_FAIL("renaming failed because backup dir does not exist");
    } catch(...) {
        const char *what = catchIoFailure();
        CPPUNIT_ASSERT(strstr(what, "Unable to rename original file before rewriting it."));
    }
    backupStream2.clear();
    workingCopyPathMode("bak/unsupported.bin", WorkingCopyMode::NoCopy);
    createBackupFile(file.path(), backupPath2, file.stream(), backupStream2);
    CPPUNIT_ASSERT_EQUAL(workingDir + "/bak/unsupported.bin", backupPath2);

    // get rid of 2nd backup (again)
    backupStream2.close();
    remove(backupPath2.data());
    remove(backupDirectory().data());

    // should be able to use backup stream, eg. seek to the end
    backupStream1.seekg(0, ios_base::end);
    CPPUNIT_ASSERT_EQUAL(41_st, static_cast<size_t>(backupStream1.tellg()));

    // restore backup
    restoreOriginalFileFromBackupFile(file.path(), backupPath1, file.stream(), backupStream1);

    // check restored backup
    file.open(true);
    file.stream().seekg(0x1D);
    CPPUNIT_ASSERT_EQUAL(static_cast<ios::int_type>(0x34), file.stream().get());
    file.close();

    CPPUNIT_ASSERT_MESSAGE("file has no critical notifications yet", !file.hasCriticalNotifications());

    // reset backup dir again
    backupDirectory().clear();

    // restore after user aborted
    createBackupFile(file.path(), backupPath1, file.stream(), backupStream1);
    try {
        throw OperationAbortedException();
    } catch(...) {
        CPPUNIT_ASSERT_THROW(handleFailureAfterFileModified(file, backupPath1, file.stream(), backupStream1, "test"), OperationAbortedException);
    }
    CPPUNIT_ASSERT(!file.hasCriticalNotifications());
    CPPUNIT_ASSERT(file.hasNotifications());
    CPPUNIT_ASSERT_EQUAL("Rewriting the file to apply changed tag information has been aborted."s, file.notifications().front().message());
    CPPUNIT_ASSERT_EQUAL("The original file has been restored."s, file.notifications().back().message());
    file.invalidateNotifications();

    // restore after error
    createBackupFile(file.path(), backupPath1, file.stream(), backupStream1);
    try {
        throw Failure();
    } catch(...) {
        CPPUNIT_ASSERT_THROW(handleFailureAfterFileModified(file, backupPath1, file.stream(), backupStream1, "test"), Failure);
    }
    CPPUNIT_ASSERT(file.hasCriticalNotifications());
    CPPUNIT_ASSERT_EQUAL("Rewriting the file to apply changed tag information failed."s, file.notifications().front().message());
    CPPUNIT_ASSERT_EQUAL("The original file has been restored."s, file.notifications().back().message());
    file.invalidateNotifications();

    // restore after io failure
    createBackupFile(file.path(), backupPath1, file.stream(), backupStream1);
    try {
        throwIoFailure("simulated IO failure");
    } catch(...) {
        try {
            handleFailureAfterFileModified(file, backupPath1, file.stream(), backupStream1, "test");
            CPPUNIT_FAIL("IO failure rethrown");
        } catch(...) {
            catchIoFailure();
        }
    }
    CPPUNIT_ASSERT(file.hasCriticalNotifications());
    CPPUNIT_ASSERT_EQUAL("An IO error occured when rewriting the file to apply changed tag information."s, file.notifications().front().message());
    CPPUNIT_ASSERT_EQUAL("The original file has been restored."s, file.notifications().back().message());
    file.invalidateNotifications();

    remove(file.path().data());
}
#endif
