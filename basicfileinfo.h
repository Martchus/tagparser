#ifndef BASICFILEINFO_H
#define BASICFILEINFO_H

#include <c++utilities/application/global.h>
#include <c++utilities/conversion/types.h>

#include <string>
#include <fstream>

namespace Media {

class LIB_EXPORT BasicFileInfo
{
public:
    BasicFileInfo(const std::string &path = std::string());
    BasicFileInfo(const BasicFileInfo &) = delete;
    BasicFileInfo &operator=(const BasicFileInfo &) = delete;
    virtual ~BasicFileInfo();

    void open(bool readOnly = false);
    void reopen(bool readonly = false);
    bool isOpen() const;
    bool isReadOnly() const;
    void close();
    void invalidate();

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
    std::fstream &stream();
    const std::fstream &stream() const;
    uint64 size() const;
    void reportSizeChanged(uint64 newSize);

protected:
    virtual void invalidated();

private:
    std::string m_path;
    std::fstream m_file;
    uint64 m_size;
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
inline std::fstream &BasicFileInfo::stream()
{
    return m_file;
}

/*!
 * \brief Returns the std::fstream for the current instance.
 */
inline const std::fstream &BasicFileInfo::stream() const
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
inline uint64 BasicFileInfo::size() const
{
    return m_size;
}

}

#endif // BASICFILEINFO_H
