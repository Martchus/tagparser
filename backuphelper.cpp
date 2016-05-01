#include "./backuphelper.h"
#include "./mediafileinfo.h"

#include <c++utilities/conversion/stringconversion.h>

#ifdef PLATFORM_WINDOWS
# include <windows.h>
#else
# include <sys/stat.h>
#endif

#include <string>
#include <fstream>
#include <cstdio>
#include <stdexcept>

using namespace std;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \namespace Media::BackupHelper
 * \brief Helps to create and restore backup files when rewriting
 *        files to apply changed tag information.
 *
 * Methods in this namespace are internally used eg. in implementations of AbstractContainer::internalMake().
 */

namespace BackupHelper {

/*!
 * \brief Returns the directory used to store backup files.
 *
 * Setting this value allows creation of backup files in a custom location
 * instead of the directory of the file being modified.
 */
string &backupDirectory()
{
    static string backupDir;
    return backupDir;
}

/*!
 * \brief Restores the original file from the specified backup file.
 * \param originalPath Specifies the path to the original file.
 * \param backupPath Specifies the path to the backup file.
 * \param originalStream A std::fstream instance for the original file.
 * \param backupStream A std::fstream instance for the backup file.
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
 */
void restoreOriginalFileFromBackupFile(const string &originalPath, const string &backupPath, fstream &originalStream, fstream &backupStream)
{
    // ensure the orignal stream is closed
    if(originalStream.is_open()) {
        originalStream.close();
    }
    // check wether backup file actually exists and close the backup stream afterwards
    backupStream.exceptions(ios_base::goodbit);
    backupStream.close();
    backupStream.clear();
    backupStream.open(backupPath, ios_base::in | ios_base::binary);
    if(backupStream.is_open()) {
        backupStream.close();
    } else {
        throw ios_base::failure("Backup/temporary file has not been created.");
    }
    // remove original file and restore backup
    std::remove(originalPath.c_str());
    if(std::rename(backupPath.c_str(), originalPath.c_str()) != 0) { // restore backup
        // unable to move the file
        try { // to copy
            // need to open all streams again
            backupStream.exceptions(ios_base::failbit | ios_base::badbit);
            originalStream.exceptions(ios_base::failbit | ios_base::badbit);
            backupStream.open(backupPath, ios_base::in | ios_base::binary);
            originalStream.open(originalPath, ios_base::out | ios_base::binary);
            originalStream << backupStream.rdbuf();
            // TODO: callback for progress updates
        } catch(const ios_base::failure &) {
            throw ios_base::failure("Unable to restore original file from backup file \"" + backupPath + "\" after failure.");
        }
    }
}

/*!
 * \brief Creates a backup file for the specified file.
 * \param originalPath Specifies the path of the file to be backuped.
 * \param backupPath Contains the path of the created backup file when this function returns.
 * \param originalStream Specifies a std::fstream for the original file.
 * \param backupStream Specifies a std::fstream for creating the backup file.
 *
 * This helper function is used by MediaFileInfo and container implementations to create a backup file
 * when applying changes. The path of the created backup file is set to \a backup path.
 * The specified \a backupStream will be closed if currently open. Then is is
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
 */
void createBackupFile(const string &originalPath, string &backupPath, fstream &originalStream, fstream &backupStream)
{
    // determine the backup path
    const string &backupDir = backupDirectory();
#ifndef PLATFORM_WINDOWS
    struct stat backupStat;
#endif
    for(unsigned int i = 0; ; ++i) {
        if(backupDir.empty()) {
            if(i) {
                backupPath = originalPath + '.' + numberToString(i) + ".bak";
            } else {
                backupPath = originalPath + ".bak";
            }
        } else {
            const string fileName = BasicFileInfo::fileName(originalPath, i);
            if(i) {
                const string ext = BasicFileInfo::extension(originalPath);
                if(backupDir.at(0) != '/' && (backupDir.size() < 2 || backupDir.at(1) != ':')) {
                    // backupDir is a relative path
                    backupPath = BasicFileInfo::containingDirectory(originalPath);
                    backupPath += '/';
                    backupPath += backupDir;
                    backupPath += '/';
                    backupPath += fileName;
                    backupPath += '.';
                    backupPath += numberToString(i);
                    backupPath += ext;
                } else {
                    // backupDir is an absolute path
                    backupPath = backupDir;
                    backupPath += '/';
                    backupPath += fileName;
                    backupPath += '.';
                    backupPath += numberToString(i);
                    backupPath += ext;
                }
            } else {
                if(backupDir.at(0) != '/' && (backupDir.size() < 2 || backupDir.at(1) != ':')) {
                    // backupDir is a relative path
                    backupPath = BasicFileInfo::containingDirectory(originalPath);
                    backupPath += '/';
                    backupPath += backupDir;
                    backupPath += '/';
                    backupPath += fileName;
                } else {
                    // backupDir is an absolute path
                    backupPath = backupDir;
                    backupPath += '/';
                    backupPath += fileName;
                }
            }

        }
        // test whether the backup file already exists
#ifdef PLATFORM_WINDOWS
        if(GetFileAttributes(backupPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
#else
        if(stat(backupPath.c_str(), &backupStat)) {
#endif
            break;
        } // else: the backup file already exists -> find another file name
    }

    // remove backup file if already exists
    std::remove(backupPath.c_str());
    // ensure original file is closed
    if(originalStream.is_open()) {
        originalStream.close();
    }
    // rename original file
    if(std::rename(originalPath.c_str(), backupPath.c_str()) != 0) {
        // can't rename/move the file
        try { // to copy
            backupStream.exceptions(ios_base::failbit | ios_base::badbit);
            originalStream.exceptions(ios_base::failbit | ios_base::badbit);
            // ensure backupStream is opened as write-only
            if(backupStream.is_open()) {
                backupStream.close();
            }
            backupStream.open(backupPath, ios_base::out | ios_base::binary);
            // ensure originalStream is opened with read permissions
            originalStream.open(originalPath, ios_base::in | ios_base::binary);
            // do the actual copying
            backupStream << originalStream.rdbuf();
            // streams are closed in the next try-block
            // TODO: callback for progress updates
        } catch(const ios_base::failure &) {
            throw ios_base::failure("Unable to rename original file before rewriting it.");
        }
    }
    try {
        // ensure there is not file associated with the originalStream object
        if(originalStream.is_open()) {
            originalStream.close();
        }
        // ensure there is no file associated with the backupStream object
        if(backupStream.is_open()) {
            backupStream.close();
        }
        // open backup stream
        backupStream.exceptions(ios_base::failbit | ios_base::badbit);
        backupStream.open(backupPath, ios_base::in | ios_base::binary);
    } catch(const ios_base::failure &) {
        // can't open the new file
        // -> try to re-rename backup file in the error case to restore previous state
        if(std::rename(backupPath.c_str(), originalPath.c_str()) != 0) {
            throw ios_base::failure("Unable to restore original file from backup file \"" + backupPath + "\" after failure.");
        } else {
            throw ios_base::failure("Unable to open backup file.");
        }
    }
}

/*!
 * \brief Handles a failure/abort which occured after the file has been modified.
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
 * \param context      Specifies the context used to add notifications.
 */
void handleFailureAfterFileModified(MediaFileInfo &fileInfo, const string &backupPath, fstream &outputStream, fstream &backupStream, const string &context)
{
    // reset the associated container in any case
    if(fileInfo.container()) {
        fileInfo.container()->reset();
    }

    // re-throw the current exception
    try {
        throw;
    } catch(const OperationAbortedException &) {
        if(!backupPath.empty()) {
            // a temp/backup file has been created -> restore original file
            fileInfo.addNotification(NotificationType::Information, "Rewriting the file to apply changed tag information has been aborted.", context);
            try {
                restoreOriginalFileFromBackupFile(fileInfo.path(), backupPath, outputStream, backupStream);
                fileInfo.addNotification(NotificationType::Information, "The original file has been restored.", context);
            } catch(const ios_base::failure &ex) {
                fileInfo.addNotification(NotificationType::Critical, ex.what(), context);
            }
        } else {
            fileInfo.addNotification(NotificationType::Information, "Applying new tag information has been aborted.", context);
        }
        throw;

    } catch(const Failure &) {
        if(!backupPath.empty()) {
            // a temp/backup file has been created -> restore original file
            fileInfo.addNotification(NotificationType::Critical, "Rewriting the file to apply changed tag information failed.", context);
            try {
                restoreOriginalFileFromBackupFile(fileInfo.path(), backupPath, outputStream, backupStream);
                fileInfo.addNotification(NotificationType::Information, "The original file has been restored.", context);
            } catch(const ios_base::failure &ex) {
                fileInfo.addNotification(NotificationType::Critical, ex.what(), context);
            }
        } else {
            fileInfo.addNotification(NotificationType::Critical, "Applying new tag information failed.", context);
        }
        throw;

    } catch(const ios_base::failure &) {
        if(!backupPath.empty()) {
            // a temp/backup file has been created -> restore original file
            fileInfo.addNotification(NotificationType::Critical, "An IO error occured when rewriting the file to apply changed tag information.", context);
            try {
                restoreOriginalFileFromBackupFile(fileInfo.path(), backupPath, outputStream, backupStream);
                fileInfo.addNotification(NotificationType::Information, "The original file has been restored.", context);
            } catch(const ios_base::failure &ex) {
                fileInfo.addNotification(NotificationType::Critical, ex.what(), context);
            }
        } else {
            fileInfo.addNotification(NotificationType::Critical, "An IO error occured when applying tag information.", context);
        }
        throw;

    }
}

}

}
