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
BasicFileInfo::BasicFileInfo()
    : m_size(0)
    , m_readOnly(false)
{
    m_file.exceptions(ios_base::failbit | ios_base::badbit);
}

/*!
 * \brief Constructs a new BasicFileInfo for the specified file.
 *
 * \param path Specifies the absolute or relative path of the file.
 */
BasicFileInfo::BasicFileInfo(std::string &&path)
    : m_path(std::move(path))
    , m_size(0)
    , m_readOnly(false)
{
    m_file.exceptions(ios_base::failbit | ios_base::badbit);
}

/*!
 * \brief Constructs a new BasicFileInfo for the specified file.
 *
 * \param path Specifies the absolute or relative path of the file.
 */
BasicFileInfo::BasicFileInfo(std::string_view path)
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
    m_file.open(
        pathForOpen(path()).data(), (m_readOnly = readOnly) ? ios_base::in | ios_base::binary : ios_base::in | ios_base::out | ios_base::binary);
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
void BasicFileInfo::setPath(std::string_view path)
{
    if (path != m_path) {
        invalidated();
        m_path = path;
    }
}

/*!
 * \brief Sets the current file.
 *
 * A possibly opened std::fstream will be closed and invalidated() will be called.
 *
 * \param path Specifies the absolute or relative path of the file to be set.
 */
void BasicFileInfo::setPath(std::string &&path)
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
 * \param cutExtension Indicates whether the extension/suffix should be cut.
 */
std::string BasicFileInfo::fileName(std::string_view path, bool cutExtension)
{
    const auto lastSlash = path.rfind('/');
    const auto lastBackSlash = path.rfind('\\');
    const auto lastPoint = cutExtension ? path.rfind('.') : string::npos;
    auto lastSeparator = decltype(lastSlash)();
    if (lastSlash == string::npos && lastBackSlash == string::npos) {
        return std::string(lastPoint == string::npos ? path : path.substr(0, lastPoint));
    } else if (lastSlash == string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    return std::string(lastPoint != string::npos ? path.substr(lastSeparator + 1, lastPoint - lastSeparator - 1) : path.substr(lastSeparator + 1));
}

/*!
 * \brief Returns the file name of the current file.
 *
 * \param cutExtension Indicates whether the extension should be cut.
 */
std::string BasicFileInfo::fileName(bool cutExtension) const
{
    return fileName(m_path, cutExtension);
}

/*!
 * \brief Returns the extension of the given file.
 *
 * \param path Specifies the path of the file.
 */
std::string BasicFileInfo::extension(std::string_view path)
{
    const auto lastPoint = path.rfind('.');
    if (lastPoint == std::string::npos) {
        return std::string();
    } else {
        return std::string(path.data() + lastPoint, path.size() - lastPoint);
    }
}

/*!
 * \brief Returns the extension of the current file.
 */
std::string BasicFileInfo::extension() const
{
    return extension(m_path);
}

/*!
 * \brief Returns a copy of the given path without the extension/suffix.
 */
std::string BasicFileInfo::pathWithoutExtension(std::string_view fullPath)
{
    const auto lastPoint = fullPath.rfind('.');
    return std::string(lastPoint != std::string::npos ? fullPath.substr(0, lastPoint) : fullPath);
}

/*!
 * \brief Returns the path of the current file without the extension/suffix.
 */
std::string BasicFileInfo::pathWithoutExtension() const
{
    return pathWithoutExtension(m_path);
}

/*!
 * \brief Returns the path of the directory containing the given file.
 */
std::string BasicFileInfo::containingDirectory(std::string_view path)
{
    const auto lastSlash = path.rfind('/');
    const auto lastBackSlash = path.rfind('\\');
    auto lastSeparator = decltype(lastSlash)();
    if (lastSlash == string::npos && lastBackSlash == std::string::npos) {
        return std::string();
    } else if (lastSlash == string::npos) {
        lastSeparator = lastBackSlash;
    } else if (lastBackSlash == string::npos) {
        lastSeparator = lastSlash;
    } else {
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    }
    if (lastSeparator > 0) {
        return std::string(path.substr(0, lastSeparator));
    } else {
        return std::string();
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
