#include "mediaformat.h"

namespace Media {

/*!
 * \brief Returns the name of the specified media format as C-style string.
 *
 * Returns an empty string if no name is available.
 */
const char *mediaFormatName(MediaFormat mediaFormat)
{
    switch(mediaFormat) {
    case MediaFormat::Pcm:
        return "Puls-Code-Modulation";
    case MediaFormat::Mpeg1:
        return "MPEG-1";
    case MediaFormat::Mpeg2:
        return "MPEG-2";
    case MediaFormat::MpegL1:
        return "MPEG-1 Layer 1";
    case MediaFormat::MpegL2:
        return "MPEG-1 Layer 2";
    case MediaFormat::MpegL3:
        return "MPEG-1 Layer 3";
    case MediaFormat::Aac:
        return "Advanced Audio Coding";
    case MediaFormat::Vorbis:
        return "Vorbis";
    case MediaFormat::Png:
        return "Portable Network Graphics";
    case MediaFormat::Jpeg:
        return "JPEG File Interchange Format";
    case MediaFormat::Mpeg4Sp:
        return "H.264/MPEG-4 Simple profile";
    case MediaFormat::Mpeg4Avc:
        return "H.264/MPEG-4 Advanced Video Coding";
    case MediaFormat::Mpeg4Asp:
        return "H.263/MPEG-4 Advanced Simple Profile";
    case MediaFormat::Mpeg4:
        return "MPEG-4";
    case MediaFormat::Gif:
        return "Graphics Interchange Format";
    case MediaFormat::Tiff:
        return "Tagged Image File Format";
    case MediaFormat::UncompressedRgb:
        return "Uncompressed RGB";
    case MediaFormat::AdpcmAcm:
        return "Microsoft ADPCM-ACM code 2";
    case MediaFormat::ImaadpcmAcm:
        return "DVI/Intel IMAADPCM-ACM code 17";
    case MediaFormat::Ac3:
        return "Dolby Digital (AC-3)";
    case MediaFormat::Ac4:
        return "Dolby Digital (AC-4)";
    case MediaFormat::RealVideo:
        return "Real Video";
    case MediaFormat::RealAudio:
        return "Real Audio";
    case MediaFormat::QuicktimeVideo:
        return "Quicktime video";
    case MediaFormat::QuicktimeAudio:
        return "Quicktime audio";
    case MediaFormat::Dts:
        return "Digital Theatre System";
    case MediaFormat::Theora:
        return "Theora";
    case MediaFormat::ProRes:
        return "Apple ProRes";
    case MediaFormat::Alac:
        return "Apple lossless audio codec";
    default:
        return "";
    }
}

}
