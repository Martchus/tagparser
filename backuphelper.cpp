#include "./backuphelper.h"
#include "./diagnostics.h"
#include "./mediafileinfo.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/path.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \namespace TagParser::BackupHelper
 * \brief Helps to create and restore backup files when rewriting
 *        files to apply changed tag information.
 *
 * Methods in this namespace are internally used eg. in implementations of AbstractContainer::internalMakeFile().
 */

namespace BackupHelper {

/*!
 * \brief Restores the original file from the specified backup file.
 * \param originalPath Specifies the path to the original file.
 * \param backupPath Specifies the path to the backup file.
 * \param originalStream Specifies a std::fstream instance for the original file.
 * \param backupStream Specifies a std::fstream instance for the backup file.
 *
 * This helper function is used by MediaFileInfo and container implementations
 * to restore the original file from the specified backup file in the case a Failure
 * or an IO error occurs. The specified streams will be closed if
 * currently open.
 *
 * If moving isn't possible (eg. \a originalPath and \a backupPath refer to different partitions) the backup
 * file will be restored by copying.
 *
 * \throws Throws std::ios_base::failure on failure.
 * \todo Implement callback for progress updates (copy).
 */
void restoreOriginalFileFromBackupFile(
    const std::string &originalPath, const std::string &backupPath, NativeFileStream &originalStream, NativeFileStream &backupStream)
{
    // ensure streams are closed but don't handle any errors anymore at this point
    originalStream.exceptions(ios_base::goodbit);
    backupStream.exceptions(ios_base::goodbit);
    originalStream.close();
    backupStream.close();
    originalStream.clear();
    backupStream.clear();

    // restore usual exception handling of the streams
    originalStream.exceptions(ios_base::badbit | ios_base::failbit);
    backupStream.exceptions(ios_base::badbit | ios_base::failbit);

    // check whether backup file actually exists and close the backup stream afterwards
    const auto originalPathForOpen = std::filesystem::path(makeNativePath(BasicFileInfo::pathForOpen(originalPath)));
    const auto backupPathForOpen = std::filesystem::path(makeNativePath(BasicFileInfo::pathForOpen(backupPath)));
    auto ec = std::error_code();
    if (!std::filesystem::exists(backupPathForOpen, ec) && !ec) {
        throw std::ios_base::failure("Backup/temporary file has not been created.");
    }

    // remove original file and restore backup
    std::filesystem::remove(originalPathForOpen, ec);
    if (ec) {
        throw std::ios_base::failure("Unable to remove original file: " + ec.message());
    }
    std::filesystem::rename(backupPathForOpen, originalPathForOpen, ec);
    if (ec) {
        // try making a copy instead, maybe backup dir is on another partition
        std::filesystem::copy_file(backupPathForOpen, originalPathForOpen, ec);
    }
    if (ec) {
        throw std::ios_base::failure("Unable to restore original file from backup file \"" % backupPath % "\" after failure: " + ec.message());
    }
}

/*!
 * \brief Creates a backup file for the specified file.
 * \param backupDir Specifies the directory to store backup files. If empty, the directory of the file
 *                  to be backuped is used.
 * \param originalPath Specifies the path of the file to be backuped.
 * \param backupPath Contains the path of the created backup file when this function returns.
 * \param originalStream Specifies a std::fstream for the original file.
 * \param backupStream Specifies a std::fstream for creating the backup file.
 *
 * This helper function is used by MediaFileInfo and container implementations to create a backup file
 * when applying changes. The specified \a backupPath is set to the path of the created backup file.
 * The specified \a backupStream will be closed if currently open. Then it is
 * used to open the backup file using the flags ios_base::in and ios_base::binary.
 *
 * The specified \a originalStream is closed before performing the move operation.
 *
 * If moving isn't possible (eg. \a originalPath and \a backupPath refer to different partitions) the backup
 * file will be created by copying.
 *
 * The original file can now be rewritten to apply changes. When this operation fails
 * the created backup file can be restored using restoreOriginalFileFromBackupFile().
 *
 * \throws Throws std::ios_base::failure on failure.
 * \todo Implement callback for progress updates (copy).
 */
void createBackupFile(const std::string &backupDir, const std::string &originalPath, std::string &backupPath, NativeFileStream &originalStream,
    NativeFileStream &backupStream)
{
    // determine dirs
    const auto backupDirRelative = std::filesystem::path(makeNativePath(backupDir)).is_relative();
    const auto originalDir = backupDirRelative ? BasicFileInfo::containingDirectory(originalPath) : string();

    // determine the backup path
    auto ec = std::error_code();
    for (unsigned int i = 0;; ++i) {
        if (backupDir.empty()) {
            if (i) {
                backupPath = originalPath % '.' % i + ".bak";
            } else {
                backupPath = originalPath + ".bak";
            }
        } else {
            const auto fileName(BasicFileInfo::fileName(originalPath, i));
            if (i) {
                const auto ext(BasicFileInfo::extension(originalPath));
                if (backupDirRelative) {
                    backupPath = originalDir % '/' % backupDir % '/' % fileName % '.' % i + ext;
                } else {
                    backupPath = backupDir % '/' % fileName % '.' % i + ext;
                }
            } else {
                if (backupDirRelative) {
                    backupPath = originalDir % '/' % backupDir % '/' + fileName;
                } else {
                    backupPath = backupDir % '/' + fileName;
                }
            }
        }

        // test whether the backup path is still unused; otherwise continue loop
        if (!std::filesystem::exists(makeNativePath(BasicFileInfo::pathForOpen(backupPath)), ec)) {
            break;
        }
    }

    // ensure original file is closed
    if (originalStream.is_open()) {
        originalStream.close();
    }

    // rename original file
    const auto u8originalPath = std::filesystem::path(makeNativePath(originalPath));
    const auto backupPathForOpen = std::filesystem::path(makeNativePath(BasicFileInfo::pathForOpen(backupPath)));
    std::filesystem::rename(u8originalPath, backupPathForOpen, ec);
    if (ec) {
        // try making a copy instead, maybe backup dir is on another partition
        std::filesystem::copy_file(u8originalPath, backupPathForOpen, ec);
    }
    if (ec) {
        throw std::ios_base::failure(argsToString("Unable to create backup file \"", BasicFileInfo::pathForOpen(backupPath), "\" of \"", originalPath,
            "\" before rewriting it: " + ec.message()));
    }

    // manage streams
    try {
        // ensure there is no file associated with the originalStream object
        if (originalStream.is_open()) {
            originalStream.close();
        }
        // ensure there is no file associated with the backupStream object
        if (backupStream.is_open()) {
            backupStream.close();
        }
        // open backup stream
        backupStream.exceptions(ios_base::failbit | ios_base::badbit);
        backupStream.open(BasicFileInfo::pathForOpen(backupPath).data(), ios_base::in | ios_base::binary);
    } catch (const std::ios_base::failure &failure) {
        // try to restore the previous state in the error case
        try {
            restoreOriginalFileFromBackupFile(originalPath, backupPath, originalStream, backupStream);
        } catch (const std::ios_base::failure &) {
            throw std::ios_base::failure("Unable to restore original file from backup file \"" % backupPath % "\" after failure: " + failure.what());
        }
        throw std::ios_base::failure(argsToString("Unable to open backup file: ", failure.what()));
    }
}

/*!
 * \brief Creates a backup file like createBackupFile() but canonicalizes \a originalPath before doing the backup.
 * \remarks
 * - This function sets \a originalPath to be a canonical path.
 * - Using this function (instead of createBackupFile()) is recommended so the actual file is being altered.
 */
void createBackupFileCanonical(const std::string &backupDir, std::string &originalPath, std::string &backupPath,
    CppUtilities::NativeFileStream &originalStream, CppUtilities::NativeFileStream &backupStream)
{
    auto ec = std::error_code();
    if (const auto canonicalPath = std::filesystem::canonical(makeNativePath(BasicFileInfo::pathForOpen(originalPath)), ec); !ec) {
        originalPath = canonicalPath.string();
    } else {
        throw std::ios_base::failure("Unable to canonicalize path of original file before rewriting it: " + ec.message());
    }
    createBackupFile(backupDir, originalPath, backupPath, originalStream, backupStream);
}

/*!
 * \brief Handles a failure/abort which occurred after the file has been modified.
 *
 * - Restores the backup file using restoreOriginalFileFromBackupFile() if one has been created.
 * - Adds appropriate notifications to the specified \a fileInfo.
 * - Re-throws the exception.
 *
 * \remarks Must only be called when an exception derived from Failure or ios_base::failure
 *          has been caught; this method uses the "exception dispatcher" idiom.
 *
 * \param fileInfo Specifies the MediaFileInfo instance which has been modified.
 * \param backupPath Specifies the path of the backup file; might be empty if none has been created.
 * \param outputStream Specifies the stream used to write the output file. This is usually just the stream
 *                     of \a fileInfo, but is specified here explicitly for higher flexibility.
 * \param backupStream Specifies the stream assembled using createBackupFile(); might be a default fstream if
 *                     no backup file has been created.
 * \param diag Specifies the container to add diagnostic messages to.
 * \param context      Specifies the context used to add notifications.
 */
void handleFailureAfterFileModified(MediaFileInfo &fileInfo, const std::string &backupPath, NativeFileStream &outputStream,
    NativeFileStream &backupStream, Diagnostics &diag, const std::string &context)
{
    handleFailureAfterFileModifiedCanonical(fileInfo, fileInfo.path(), backupPath, outputStream, backupStream, diag, context);
}

/*!
 * \brief Handles a failure/abort which occurred after the file has been modified.
 * \remarks Same as handleFailureAfterFileModified() but allows specifying the original path instead of just using the
 *          path from \a mediaFileInfo.
 */
void handleFailureAfterFileModifiedCanonical(MediaFileInfo &fileInfo, const std::string &originalPath, const std::string &backupPath,
    CppUtilities::NativeFileStream &outputStream, CppUtilities::NativeFileStream &backupStream, Diagnostics &diag, const std::string &context)
{
    // reset the associated container in any case
    if (fileInfo.container()) {
        fileInfo.container()->reset();
    }

    // re-throw the current exception
    try {
        throw;
    } catch (const OperationAbortedException &) {
        if (!backupPath.empty()) {
            // a temp/backup file has been created -> restore original file
            diag.emplace_back(DiagLevel::Information, "Rewriting the file to apply changed tag information has been aborted.", context);
            try {
                restoreOriginalFileFromBackupFile(originalPath, backupPath, outputStream, backupStream);
                diag.emplace_back(DiagLevel::Warning, "The original file has been restored.", context);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical, argsToString("The original file could not be restored: ", failure.what()), context);
            }
        } else {
            diag.emplace_back(DiagLevel::Information, "Applying new tag information has been aborted.", context);
        }
        throw;

    } catch (const Failure &) {
        if (!backupPath.empty()) {
            // a temp/backup file has been created -> restore original file
            diag.emplace_back(DiagLevel::Critical, "Rewriting the file to apply changed tag information failed.", context);
            try {
                restoreOriginalFileFromBackupFile(originalPath, backupPath, outputStream, backupStream);
                diag.emplace_back(DiagLevel::Warning, "The original file has been restored.", context);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical, argsToString("The original file could not be restored: ", failure.what()), context);
            }
        } else {
            diag.emplace_back(DiagLevel::Critical, "Applying new tag information failed.", context);
        }
        throw;

    } catch (const std::ios_base::failure &) {
        if (!backupPath.empty()) {
            // a temp/backup file has been created -> restore original file
            diag.emplace_back(DiagLevel::Critical, "An IO error occurred when rewriting the file to apply changed tag information.", context);
            try {
                restoreOriginalFileFromBackupFile(originalPath, backupPath, outputStream, backupStream);
                diag.emplace_back(DiagLevel::Warning, "The original file has been restored.", context);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical, argsToString("The original file could not be restored: ", failure.what()), context);
            }
        } else {
            diag.emplace_back(DiagLevel::Critical, "An IO error occurred when applying tag information.", context);
        }
        throw;
    }
}

} // namespace BackupHelper

} // namespace TagParser
