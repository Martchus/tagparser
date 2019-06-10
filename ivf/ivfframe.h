#ifndef TAG_PARSER_IVFRAME_H
#define TAG_PARSER_IVFRAME_H

#include "../diagnostics.h"

namespace CppUtilities {
class BinaryReader;
}

namespace TagParser {

class TAG_PARSER_EXPORT IvfFrame {
public:
    constexpr IvfFrame();
    void parseHeader(CppUtilities::BinaryReader &reader, Diagnostics &diag);

private:
    std::uint64_t startOffset;
    std::uint64_t timestamp;
    std::uint32_t size;
};

/*!
 * \brief Constructs a new frame.
 */
constexpr IvfFrame::IvfFrame()
    : startOffset(0)
    , timestamp(0)
    , size(0)
{
}

} // namespace TagParser

#endif // TAG_PARSER_IVFRAME_H
