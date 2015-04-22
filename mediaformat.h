#ifndef MEDIAFORMAT_H
#define MEDIAFORMAT_H

namespace Media {

/*!
 * \brief The MediaType enum specifies the type of media data (acoustic, visual, textual, ...).
 */
enum class MediaType
{
    Acoustic,
    Visual,
    Textual,
    Hint,
    Unknown
};

/*!
 * \brief The MediaFormat enum specifies the format of media data (PCM, MPEG-4, PNG, ...).
 */
enum class MediaFormat
{
    Pcm,
    Mpeg1,
    Mpeg2,
    MpegL1,
    MpegL2,
    MpegL3,
    Aac,
    Alac,
    Vorbis,
    Png,
    Jpeg,
    Bitmap,
    Mpeg4Sp,
    Mpeg4Avc,
    Mpeg4Asp,
    Mpeg4,
    Gif,
    Tiff,
    UncompressedRgb,
    AdpcmAcm,
    ImaadpcmAcm,
    Ac3,
    Ac4,
    Dts,
    Flac,
    RealAudio,
    RealVideo,
    QuicktimeVideo,
    QuicktimeAudio,
    Theora,
    ProRes,
    VobSub,
    Unknown
};

const char *mediaFormatName(MediaFormat mediaFormat);

}

#endif // MEDIAFORMAT_H
