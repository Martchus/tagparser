#ifndef TAG_PARSER_SIGNATURE_H
#define TAG_PARSER_SIGNATURE_H

#include "./mediaformat.h"

#include <c++utilities/conversion/types.h>

namespace TagParser {

DECLARE_ENUM_CLASS(TagTargetLevel, byte);

/*!
 * \brief Specifies the container format.
 *
 * Raw streams like ADTS or raw FLAC count as container format in this context.
 */
enum class ContainerFormat : unsigned int {
    Unknown, /**< unknown container format */
    Ac3Frames, /**< raw AC-3/Dolby Digital frames */
    Adts, /**< Audio Data Transport Stream */
    Ar, /**< "GNU ar" archive */
    Asf, /**< Advanced Systems Format */
    Bzip2, /**< bzip2 compressed file */
    Dirac, /**< raw Dirac */
    Ebml, /**< EBML */
    Elf, /**< Executable and Linkable Format */
    Flac, /**< raw Free Lossless Audio Codec */
    FlashVideo, /**< Flash (FLV) */
    Gif87a, /**< Graphics Interchange Format (1987) */
    Gif89a, /**< Graphics Interchange Format (1989) */
    Gzip, /**< gzip compressed file */
    Id2v2Tag, /**< file holding an ID2v2 tag only */
    JavaClassFile, /**< Java class file */
    Jpeg, /**< JPEG File Interchange Format */
    Lha, /**< LHA */
    Lzip, /**< lz compressed file */
    Lzw, /**< LZW */
    Matroska, /**< Matroska (subset of EBML) */
    MonkeysAudio, /**< Monkey's Audio */
    Mp4, /**< MPEG-4 Part 14 (subset of QuickTime container) */
    MpegAudioFrames, /**< MPEG-1 Layer 1/2/3 frames */
    Ogg, /**< Ogg */
    PhotoshopDocument, /**< Photoshop document */
    Png, /**< Portable Network Graphics */
    PortableExecutable, /**< Portable Executable */
    QuickTime, /**< QuickTime container */
    Rar, /**< RAR Archive */
    Riff, /**< Resource Interchange File Format */
    RiffAvi, /**< Audio Video Interleave (subset of RIFF) */
    RiffWave, /**< WAVE (subset of RIFF) */
    SevenZ, /**< 7z archive */
    Tar, /**< Tar archive */
    TiffBigEndian, /**< Tagged Image File Format (big endian) */
    TiffLittleEndian, /**< Tagged Image File Format (little endian) */
    Utf16Text, /**< UTF-16 text */
    Utf32Text, /**< UTF-32 text */
    Utf8Text, /**< UTF-8 text */
    WavPack, /**< WavPack */
    Webm, /**< WebM (subset of Matroska) */
    WindowsBitmap, /**< Microsoft Windows Bitmap */
    WindowsIcon, /**< Microsoft Windows Icon */
    Xz, /**< xz compressed file */
    YUV4Mpeg2, /**< YUV4MPEG2 */
    Zip, /**< ZIP archive */
};

TAG_PARSER_EXPORT ContainerFormat parseSignature(const char *buffer, int bufferSize);
TAG_PARSER_EXPORT const char *containerFormatName(ContainerFormat containerFormat);
TAG_PARSER_EXPORT const char *containerFormatAbbreviation(
    ContainerFormat containerFormat, MediaType mediaType = MediaType::Unknown, unsigned int version = 0);
TAG_PARSER_EXPORT const char *containerFormatSubversion(ContainerFormat containerFormat);
TAG_PARSER_EXPORT const char *containerMimeType(ContainerFormat containerFormat, MediaType mediaType = MediaType::Unknown);
TAG_PARSER_EXPORT TagTargetLevel containerTargetLevel(ContainerFormat containerFormat, uint64 targetLevelValue);
TAG_PARSER_EXPORT uint64 containerTargetLevelValue(ContainerFormat containerFormat, TagTargetLevel targetLevel);

} // namespace TagParser

#endif // TAG_PARSER_SIGNATURE_H
