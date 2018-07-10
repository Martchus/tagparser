#ifndef TAG_PARSER_ID3GENRES_H
#define TAG_PARSER_ID3GENRES_H

#include "../global.h"

#include <c++utilities/conversion/types.h>

#include <string>

namespace TagParser {

class TAG_PARSER_EXPORT Id3Genres {
public:
    static inline const char *stringFromIndex(int index);
    static int indexFromString(const std::string &genre);
    static constexpr int genreCount();
    static constexpr bool isIndexSupported(int index);

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
 * \brief Returns the number of supported genres.
 */
constexpr int Id3Genres::genreCount()
{
    return 192;
}

/*!
 * \brief Returns an indication whether the specified numerical denotation is
 *        supported by this class.
 */
constexpr bool Id3Genres::isIndexSupported(int index)
{
    return index >= 0 && index < genreCount();
}

} // namespace TagParser

#endif // TAG_PARSER_ID3GENRES_H
