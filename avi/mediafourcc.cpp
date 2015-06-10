#include "mediafourcc.h"

#include "../mediaformat.h"

namespace Media {

namespace Fourccs {

MediaFormat fourccToMediaFormat(uint32 fourcc) {
    switch(fourcc) {
    case XvidMpeg4:
        return MediaFormat(GeneralMediaFormat::Mpeg4Video, SubFormats::Mpeg4Asp);
    default: return MediaFormat(); // TODO: cover more FOURCCs
    }
}

}

}
