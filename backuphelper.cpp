#include "backuphelper.h"
#include "basicfileinfo.h"

#include <string>
#include <fstream>
#include <cstdio>
#include <stdexcept>

using namespace std;

namespace Media {

/*!
 * \namespace Media::BackupHelper
 * \brief Helps to create and restore backup files when rewriting
 *        files to apply changed tag information.
 *
 * This class is only internally used (eg. in implementations of AbstractContainer::internalMake()).
 */

namespace BackupHelper {

/*!
 * \brief Returns the directory used to store backup files.
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
 * This helper function is used by MediaFileInfo to restore the original
 * file from the specified backup file in the case a Failure
 * or an IO error occurs. The specified streams will be closed if
 * currently open.
 *
 * \throws Throws std::ios_base::failure on failure.
 */
void restoreOriginalFileFromBackupFile(const string &originalPath, const string &backupPath, fstream &originalStream, fstream &backupStream)
{
    if(originalStream.is_open()) {
        originalStream.close();
    }
    if(backupStream.is_open()) {
        backupStream.close();
    }
    std::remove(originalPath.c_str());
    if(std::rename(backupPath.c_str(), originalPath.c_str()) != 0) { // restore backup
        throw ios_base::failure("Unable to restore original file from backup file \"" + backupPath + "\" after failure.");
    }
}

/*!
 * \brief Creates a backup file for the specified file.
 * \param originalPath Specifies the path of the file to be backuped.
 * \param backupPath Contains the path of the created backup file when this function returns.
 * \param backupStream Specifies a std::fstream for creating the backup file.
 *
 * This helper function is used by MediaFileInfo to create a backup file
 * when applying changes. The path of the created backup file is set to \a backup path.
 * The specified \a backupStream will be closed if currently open. Then is is
 * used to open the backup file using the flags ios_base::in and ios_base::binary.
 *
 * The original file can now be rewritten to apply changes. When this operation fails
 * the created backup file can be restored using restoreOriginalFileFromBackupFile().
 *
 * \throws Throws std::ios_base::failure on failure.
 */
void createBackupFile(const string &originalPath, string &backupPath, fstream &backupStream)
{
    // set the backup path
    const string &backupDir = backupDirectory();
    if(backupDir.empty()) {
        backupPath = originalPath + ".bak";
    } else {
        string fileName = BasicFileInfo::fileName(originalPath);
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
    // remove backup file if already exists
    std::remove(backupPath.c_str());
    // rename original file
    if(std::rename(originalPath.c_str(), backupPath.c_str()) != 0) {
        throw ios_base::failure("Unable to rename original file before rewriting it.");
    }
    try {
        // ensure there is no file associated with the backuStream object
        if(backupStream.is_open()) {
            backupStream.close();
        }
        // open backup stream
        backupStream.exceptions(ifstream::failbit | ifstream::badbit);
        backupStream.open(backupPath.c_str(), ios_base::in | ios_base::binary);
    } catch(ios_base::failure &) {
        // try to re-rename backup file in the error case
        if(std::rename(backupPath.c_str(), originalPath.c_str()) != 0) {
            throw ios_base::failure("Unable to restore original file from backup file \"" + backupPath + "\" after failure.");
        } else {
            throw ios_base::failure("Unable to open backup file.");
        }
    }
}

}

}
