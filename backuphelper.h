#ifndef TAG_PARSER_BACKUPHELPER_H
#define TAG_PARSER_BACKUPHELPER_H

#include "./global.h"

#include <c++utilities/io/nativefilestream.h>

namespace TagParser {

class MediaFileInfo;
class Diagnostics;

namespace BackupHelper {

TAG_PARSER_EXPORT void restoreOriginalFileFromBackupFile(const std::string &originalPath, const std::string &backupPath,
    CppUtilities::NativeFileStream &originalStream, CppUtilities::NativeFileStream &backupStream);
TAG_PARSER_EXPORT void createBackupFile(const std::string &backupDir, const std::string &originalPath, std::string &backupPath,
    CppUtilities::NativeFileStream &originalStream, CppUtilities::NativeFileStream &backupStream);
TAG_PARSER_EXPORT void createBackupFileCanonical(const std::string &backupDir, std::string &originalPath, std::string &backupPath,
    CppUtilities::NativeFileStream &originalStream, CppUtilities::NativeFileStream &backupStream);
TAG_PARSER_EXPORT void handleFailureAfterFileModified(MediaFileInfo &fileInfo, const std::string &backupPath,
    CppUtilities::NativeFileStream &outputStream, CppUtilities::NativeFileStream &backupStream, Diagnostics &diag,
    const std::string &context = "making file");
TAG_PARSER_EXPORT void handleFailureAfterFileModifiedCanonical(MediaFileInfo &fileInfo, const std::string &originalPath,
    const std::string &backupPath, CppUtilities::NativeFileStream &outputStream, CppUtilities::NativeFileStream &backupStream, Diagnostics &diag,
    const std::string &context = "making file");

} // namespace BackupHelper

} // namespace TagParser

#endif // TAG_PARSER_BACKUPHELPER_H
