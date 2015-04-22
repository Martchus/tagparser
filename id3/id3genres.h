#ifndef ID3GENRES_H
#define ID3GENRES_H

#include <c++utilities/application/global.h>
#include <c++utilities/conversion/types.h>

#include <string>

namespace Media
{

class LIB_EXPORT Id3Genres
{
public:
    static const char *stringFromIndex(int index);
    static int indexFromString(const std::string &genre);
    static bool isIndexSupported(int index);
    static int genreCount();

private:
    static const char **genreNames();
};

/*!
 * \brief Returns the genre name for the specified numerical denotation as C-style string.
 */
inline const char *Id3Genres::stringFromIndex(int index)
{
    return isIndexSupported(index) ? genreNames()[index] : nullptr;
}

/*!
 * \brief Returns an indication whether the specified numerical denotation is
 *        supported by this class.
 */
inline bool Id3Genres::isIndexSupported(int index)
{
    return index >= 0 && index < genreCount();
}

/*!
 * \brief Returns the number of supported genres.
 */
inline int Id3Genres::genreCount()
{
    return 126;
}

}

#endif // ID3GENRES_H
