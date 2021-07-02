#ifndef TAG_PARSER_ID3GENRES_H
#define TAG_PARSER_ID3GENRES_H

#include "../global.h"

#include <cstdint>
#include <string_view>

namespace TagParser {

class TAG_PARSER_EXPORT Id3Genres {
public:
    static inline std::string_view stringFromIndex(int index);
    static int indexFromString(std::string_view genre);
    static constexpr int genreCount();
    static constexpr int emptyGenreIndex();
    static constexpr bool isEmptyGenre(int index);
    static constexpr bool isIndexSupported(int index);

private:
    static const std::string_view *genreNames();
};

/*!
 * \brief Returns the genre name for the specified numerical denotation as C-style string.
 */
inline std::string_view Id3Genres::stringFromIndex(int index)
{
    return isIndexSupported(index) ? genreNames()[index] : std::string_view();
}

/*!
 * \brief Returns the number of supported genres.
 */
constexpr int Id3Genres::genreCount()
{
    return 192;
}

/*!
 * \brief Returns the preferred genre index to indicate that no genre is set at all.
 * \remarks Apparently some files use 255 to indicate the genre information is missing although this
 *          is not explicitly specified on [ID3.org](http://id3.org/ID3v1).
 */
constexpr int Id3Genres::emptyGenreIndex()
{
    return 255;
}

/*!
 * \brief Returns whether the genre \a index indicates the genre field is not set at all.
 */
constexpr bool Id3Genres::isEmptyGenre(int index)
{
    return index == emptyGenreIndex();
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
