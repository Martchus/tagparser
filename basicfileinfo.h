#ifndef TAG_PARSER_BASICFILEINFO_H
#define TAG_PARSER_BASICFILEINFO_H

#include "./global.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/nativefilestream.h>

#include <cstdint>
#include <string>

namespace TagParser {

class TAG_PARSER_EXPORT BasicFileInfo {
public:
    // constructor, destructor
    BasicFileInfo(const std::string &path = std::string());
    BasicFileInfo(const BasicFileInfo &) = delete;
    BasicFileInfo &operator=(const BasicFileInfo &) = delete;
    virtual ~BasicFileInfo();

    // methods to control associated file stream
    void open(bool readOnly = false);
    void reopen(bool readOnly = false);
    bool isOpen() const;
    bool isReadOnly() const;
    void close();
    void invalidate();
    CppUtilities::NativeFileStream &stream();
    const CppUtilities::NativeFileStream &stream() const;

    // methods to get, set path (components)
    const std::string &path() const;
    void setPath(const std::string &path);
    static std::string fileName(const std::string &path, bool cutExtension = false);
    std::string fileName(bool cutExtension = false) const;
    static std::string extension(const std::string &path);
    std::string extension() const;
    static std::string pathWithoutExtension(const std::string &fullPath);
    std::string pathWithoutExtension() const;
    static std::string containingDirectory(const std::string &path);
    std::string containingDirectory() const;
    static const char *pathForOpen(const std::string &url);

    // methods to get, set the file size
    std::uint64_t size() const;
    void reportSizeChanged(std::uint64_t newSize);
    void reportPathChanged(const std::string &newPath);

protected:
    virtual void invalidated();

private:
    std::string m_path;
    CppUtilities::NativeFileStream m_file;
    std::uint64_t m_size;
    bool m_readOnly;
};

/*!
 * \brief Indicates whether a std::fstream is open for the current file.
 *
 * \sa stream()
 */
inline bool BasicFileInfo::isOpen() const
{
    return m_file.is_open();
}

/*!
 * \brief Indicates whether the last open()/reopen() call was read-only.
 */
inline bool BasicFileInfo::isReadOnly() const
{
    return m_readOnly;
}

/*!
 * \brief Returns the std::fstream for the current instance.
 */
inline CppUtilities::NativeFileStream &BasicFileInfo::stream()
{
    return m_file;
}

/*!
 * \brief Returns the std::fstream for the current instance.
 */
inline const CppUtilities::NativeFileStream &BasicFileInfo::stream() const
{
    return m_file;
}

/*!
 * \brief Returns the path of the current file.
 *
 * \sa setPath()
 */
inline const std::string &BasicFileInfo::path() const
{
    return m_path;
}

/*!
 * \brief Returns size of the current file in bytes.
 * \remarks The file needs to be opened. Otherwise zero or the size of the
 *          previously opened file is returned.
 *          The size is not automatically updated when the file is modified.
 *          You might update the size using the reportSizeChanged() method.
 */
inline std::uint64_t BasicFileInfo::size() const
{
    return m_size;
}

/*!
 * \brief Call this function to report that the size changed.
 * \remarks Should be called after writing/truncating the stream().
 */
inline void BasicFileInfo::reportSizeChanged(std::uint64_t newSize)
{
    m_size = newSize;
}

/*!
 * \brief Call this function to report that the path changed.
 * \remarks Should be called after associating another file to the stream() manually.
 */
inline void BasicFileInfo::reportPathChanged(const std::string &newPath)
{
    m_path = newPath;
}

/*!
 * \brief Returns removes the "file:/" prefix from \a url to be able to pass it to functions
 *        like open(), stat() and truncate().
 * \remarks If \a url is already a plain path it won't changed.
 * \returns Returns a pointer the URL data itself. No copy is made.
 */
inline const char *BasicFileInfo::pathForOpen(const std::string &url)
{
    return CppUtilities::startsWith(url, "file:/") ? url.data() + 6 : url.data();
}

} // namespace TagParser

#endif // TAG_PARSER_BASICFILEINFO_H
