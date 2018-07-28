#ifndef TAG_PARSER_IVFRAME_H
#define TAG_PARSER_IVFRAME_H

#include "../diagnostics.h"

#include <c++utilities/conversion/types.h>

namespace IoUtilities {
class BinaryReader;
}

namespace TagParser {

class TAG_PARSER_EXPORT IvfFrame {
public:
    constexpr IvfFrame();
    void parseHeader(IoUtilities::BinaryReader &reader, Diagnostics &diag);

private:
    uint64 startOffset;
    uint64 timestamp;
    uint32 size;
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
