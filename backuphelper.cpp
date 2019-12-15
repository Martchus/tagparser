#include "./backuphelper.h"
#include "./diagnostics.h"
#include "./mediafileinfo.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include <cstdio>
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
    // ensure the orignal stream is closed
    if (originalStream.is_open()) {
        originalStream.close();
    }
    // check wether backup file actually exists and close the backup stream afterwards
    backupStream.exceptions(ios_base::goodbit);
    backupStream.close();
    backupStream.clear();
    backupStream.open(backupPath, ios_base::in | ios_base::binary);
    if (backupStream.is_open()) {
        backupStream.close();
    } else {
        throw std::ios_base::failure("Backup/temporary file has not been created.");
    }
    // remove original file and restore backup
    std::remove(originalPath.c_str());
    if (std::rename(BasicFileInfo::pathForOpen(backupPath), BasicFileInfo::pathForOpen(originalPath)) == 0) {
        return;
    }
    // can't rename/move the file (maybe backup dir on another partition) -> make a copy instead
    try {
        // need to open all streams again
        backupStream.exceptions(ios_base::failbit | ios_base::badbit);
        originalStream.exceptions(ios_base::failbit | ios_base::badbit);
        backupStream.open(backupPath, ios_base::in | ios_base::binary);
        originalStream.open(originalPath, ios_base::out | ios_base::binary);
        originalStream << backupStream.rdbuf();
        originalStream.flush();
        // TODO: callback for progress updates
    } catch (const std::ios_base::failure &failure) {
        throw std::ios_base::failure("Unable to restore original file from backup file \"" % backupPath % "\" after failure: " + failure.what());
    }
}

/*!
 * \brief Returns whether the specified \a path is relative.
 */
static bool isRelative(const std::string &path)
{
    return path.empty() || (path.front() != '/' && (path.size() < 2 || path[1] != ':'));
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
    const auto backupDirRelative(isRelative(backupDir));
    const auto originalDir(backupDirRelative ? BasicFileInfo::containingDirectory(originalPath) : string());

    // determine the backup path
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
#ifdef PLATFORM_WINDOWS
        if (GetFileAttributes(BasicFileInfo::pathForOpen(backupPath)) == INVALID_FILE_ATTRIBUTES) {
#else
        struct stat backupStat;
        if (stat(BasicFileInfo::pathForOpen(backupPath), &backupStat)) {
#endif
            break;
        }
    }

    // ensure original file is closed
    if (originalStream.is_open()) {
        originalStream.close();
    }

    // rename original file
    if (std::rename(BasicFileInfo::pathForOpen(originalPath), BasicFileInfo::pathForOpen(backupPath))) {
        // can't rename/move the file (maybe backup dir on another partition) -> make a copy instead
        try {
            backupStream.exceptions(ios_base::failbit | ios_base::badbit);
            originalStream.exceptions(ios_base::failbit | ios_base::badbit);
            // ensure backupStream is opened as write-only
            if (backupStream.is_open()) {
                backupStream.close();
            }
            backupStream.open(BasicFileInfo::pathForOpen(backupPath), ios_base::out | ios_base::binary);
            // ensure originalStream is opened with read permissions
            originalStream.open(BasicFileInfo::pathForOpen(originalPath), ios_base::in | ios_base::binary);
            // do the actual copying
            backupStream << originalStream.rdbuf();
            backupStream.flush();
            // streams are closed in the next try-block
            // TODO: callback for progress updates
        } catch (const std::ios_base::failure &failure) {
            throw std::ios_base::failure(argsToString("Unable to rename original file before rewriting it: ", failure.what()));
        }
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
        backupStream.open(BasicFileInfo::pathForOpen(backupPath), ios_base::in | ios_base::binary);
    } catch (const std::ios_base::failure &failure) {
        // can't open the new file
        // -> try to re-rename backup file in the error case to restore previous state
        if (std::rename(BasicFileInfo::pathForOpen(backupPath), BasicFileInfo::pathForOpen(originalPath))) {
            throw std::ios_base::failure("Unable to restore original file from backup file \"" % backupPath % "\" after failure: " + failure.what());
        } else {
            throw std::ios_base::failure(argsToString("Unable to open backup file: ", failure.what()));
        }
    }
}

/*!
 * \brief Handles a failure/abort which occurred after the file has been modified.
 *
 * - Restores the backup file using restoreOriginalFileFromBackupFile() if one has been created.
 * - Adds appropriate notifications to the specified \a fileInfo.
 * - Re-throws the exception.
 *
 * \remarks Must only be called when an exception derived from Failure or ios_base::failure
 *          has been catched; this method uses the "exception dispatcher" idiom.
 *
 * \param fileInfo Specifies the MediaFileInfo instace which has been modified.
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
                restoreOriginalFileFromBackupFile(fileInfo.path(), backupPath, outputStream, backupStream);
                diag.emplace_back(DiagLevel::Information, "The original file has been restored.", context);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical, failure.what(), context);
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
                restoreOriginalFileFromBackupFile(fileInfo.path(), backupPath, outputStream, backupStream);
                diag.emplace_back(DiagLevel::Information, "The original file has been restored.", context);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical, failure.what(), context);
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
                restoreOriginalFileFromBackupFile(fileInfo.path(), backupPath, outputStream, backupStream);
                diag.emplace_back(DiagLevel::Information, "The original file has been restored.", context);
            } catch (const std::ios_base::failure &failure) {
                diag.emplace_back(DiagLevel::Critical, failure.what(), context);
            }
        } else {
            diag.emplace_back(DiagLevel::Critical, "An IO error occurred when applying tag information.", context);
        }
        throw;
    }
}

} // namespace BackupHelper

} // namespace TagParser
