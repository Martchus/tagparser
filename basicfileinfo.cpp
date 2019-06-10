#include "./basicfileinfo.h"

#include <c++utilities/conversion/stringconversion.h>

using namespace std;
using namespace CppUtilities;

/*!
 * \brief Contains all classes and functions of the TagInfo library.
 */
namespace TagParser {
/*!
 * \class BasicFileInfo
 * \brief The BasicFileInfo class provides basic file information such as
 *        file name, extension, directory and size for a specified file.
 */

/*!
 * \brief Constructs a new BasicFileInfo for the specified file.
 *
 * \param path Specifies the absolute or relative path of the file.
 */
BasicFileInfo::BasicFileInfo(const std::string &path)
    : m_path(path)
    , m_size(0)
    , m_readOnly(false)
{
    m_file.exceptions(ios_base::failbit | ios_base::badbit);
}

/*!
 * \brief Destroys the BasicFileInfo.
 *
 * A possibly opened std::fstream will be closed.
 */
BasicFileInfo::~BasicFileInfo()
{
    close();
}

/*!
 * \brief Opens a std::fstream for the current file. Does nothing a stream is already open.
 * \param readOnly Indicates whether the stream should be opend as read-only.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
void BasicFileInfo::open(bool readOnly)
{
    if (!isOpen()) {
        reopen(readOnly);
    }
}

/*!
 * \brief Opens a std::fstream for the current file. Closes a possibly already opened stream and
 *        clears all flags before.
 * \param readOnly Indicates whether the stream should be opend as read-only.
 * \throws Throws std::ios_base::failure when an IO error occurs.
 */
void BasicFileInfo::reopen(bool readOnly)
{
    invalidated();
    m_file.open(pathForOpen(path()), (m_readOnly = readOnly) ? ios_base::in | ios_base::binary : ios_base::in | ios_base::out | ios_base::binary);
    m_file.seekg(0, ios_base::end);
    m_size = static_cast<std::uint64_t>(m_file.tellg());
    m_file.seekg(0, ios_base::beg);
}

/*!
 * \brief A possibly opened std::fstream will be closed. All flags of the stream will be cleared.
 */
void BasicFileInfo::close()
{
    if (isOpen()) {
        m_file.close();
    }
    m_file.clear();
}

/*!
 * \brief Invalidates the file info manually.
 */
void BasicFileInfo::invalidate()
{
    invalidated();
}

/*!
 * \brief Sets the current file.
 *
 * A possibly opened std::fstream will be closed and invalidated() will be called.
 *
 * \param path Specifies the absolute or relative path of the file to be set.
 */
void BasicFileInfo::setPath(const string &path)
{
    if (path != m_path) {
        invalidated();
        m_path = path;
    }
}

/*!
 * \brief Returns the file name of the given file.
 *
 * \param path Specifies the path of the file.
 * \param cutExtension Indicates whether the extension/suffix should be cutted.
 */
string BasicFileInfo::fileName(const string &path, bool cutExtension)
{
    size_t lastSlash = path.rfind('/');
    size_t lastBackSlash = path.rfind('\\');
    size_t lastPoint = cutExtension ? path.rfind('.') : string::npos;
    size_t lastSeparator;
    if (lastSlash == string::npos && lastBackSlash == string::npos) {
        return (lastPoint == string::npos) ? path : path.substr(0, lastPoint);
    } else if (lastSlash == string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    return (lastPoint != string::npos) ? path.substr(lastSeparator + 1, lastPoint - lastSeparator - 1) : path.substr(lastSeparator + 1);
}

/*!
 * \brief Returns the file name of the current file.
 *
 * \param cutExtension Indicates whether the extension should be cutted.
 */
string BasicFileInfo::fileName(bool cutExtension) const
{
    return fileName(m_path, cutExtension);
}

/*!
 * \brief Returns the extension of the given file.
 *
 * \param path Specifies the path of the file.
 */
string BasicFileInfo::extension(const string &path)
{
    size_t lastPoint = path.rfind('.');
    if (lastPoint == string::npos) {
        return string();
    } else {
        return path.substr(lastPoint);
    }
}

/*!
 * \brief Returns the extension of the current file.
 */
string BasicFileInfo::extension() const
{
    return extension(m_path);
}

/*!
 * \brief Returns a copy of the given path without the extension/suffix.
 */
string BasicFileInfo::pathWithoutExtension(const string &fullPath)
{
    size_t lastPoint = fullPath.rfind('.');
    return lastPoint != string::npos ? fullPath.substr(0, lastPoint) : fullPath;
}

/*!
 * \brief Returns the path of the current file without the extension/suffix.
 */
string BasicFileInfo::pathWithoutExtension() const
{
    return pathWithoutExtension(m_path);
}

/*!
 * \brief Returns the path of the directory containing the given file.
 */
string BasicFileInfo::containingDirectory(const string &path)
{
    size_t lastSlash = path.rfind('/');
    size_t lastBackSlash = path.rfind('\\');
    size_t lastSeparator;
    if (lastSlash == string::npos && lastBackSlash == string::npos) {
        return string();
    } else if (lastSlash == string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    if (lastSeparator > 0) {
        return path.substr(0, lastSeparator);
    } else {
        return string();
    }
}

/*!
 * \brief Returns the path of the directory containing the current file.
 *
 * The returned path is relative if the path of the file (specified using
 * the setPath() method) is relative.
 */
string BasicFileInfo::containingDirectory() const
{
    return containingDirectory(m_path);
}

/*!
 * \brief This function is called when the BasicFileInfo gets invalidated.
 *        This is the case when the current file changes or is reopened.
 *
 * When subclassing and overwriting this virtual method invoke the base
 * implementation by calling BasicFileInfo::invalidated() before the reimplemented code.
 */
void BasicFileInfo::invalidated()
{
    m_size = 0;
    close();
}

} // namespace TagParser
