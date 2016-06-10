#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "./mediaformat.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/application/global.h>

namespace Media {

/*!
 * \brief Specifies the container format.
 */
enum class ContainerFormat
{
    Unknown, /**< unknown container format */
    Adts, /**< Audio Data Transport Stream */
    Ar, /**< "GNU ar" archive */
    Asf, /**< Advanced Systems Format */
    Bzip2, /**< bzip2 compressed file */
    Elf, /**< Executable and Linkable Format */
    Gif87a, /**< Graphics Interchange Format (1987) */
    Gif89a, /**< Graphics Interchange Format (1989) */
    Gzip, /**< gzip compressed file */
    Id2v2Tag, /**< file holding an ID2v2 tag only */
    JavaClassFile, /**< Java class file */
    Jpeg, /**< JPEG File Interchange Format */
    Lha, /**< LHA */
    Lzw, /**< LZW */
    Mp4, /**< MPEG-4 Part 14 (subset of QuickTime container) */
    Ogg, /**< Ogg */
    PhotoshopDocument, /**< Photoshop document */
    Png, /**< Portable Network Graphics */
    PortableExecutable, /**< Portable Executable */
    Rar, /**< RAR Archive */
    Ebml, /**< EBML */
    Matroska, /**< Matroska (subset of EBML) */
    Webm, /**< WebM (subset of Matroska) */
    MpegAudioFrames, /**< MPEG-1 Layer 1/2/3 frames */
    Riff, /**< Resource Interchange File Format */
    RiffWave, /**< WAVE (subset of RIFF) */
    RiffAvi, /**< Audio Video Interleave (subset of RIFF) */
    Tar, /**< Tar archive */
    TiffBigEndian, /**< Tagged Image File Format (big endian) */
    TiffLittleEndian, /**< Tagged Image File Format (little endian) */
    Utf16Text, /**< UTF-16 text */
    Utf32Text, /**< UTF-32 text */
    Utf8Text, /**< UTF-8 text */
    WindowsBitmap, /**< Microsoft Windows Bitmap */
    WindowsIcon, /**< Microsoft Windows Icon */
    SevenZ, /**< 7z archive */
    Lzip, /**< lz compressed file */
    QuickTime, /**< QuickTime container */
    Zip /**< ZIP archive */
};

LIB_EXPORT ContainerFormat parseSignature(const char *buffer, int bufferSize);
LIB_EXPORT const char *containerFormatName(ContainerFormat containerFormat);
LIB_EXPORT const char *containerFormatAbbreviation(ContainerFormat containerFormat, MediaType mediaType = MediaType::Unknown, unsigned int version = 0);
LIB_EXPORT const char *containerFormatSubversion(ContainerFormat containerFormat);
LIB_EXPORT const char *containerMimeType(ContainerFormat containerFormat, MediaType mediaType = MediaType::Unknown);

}

#endif // SIGNATURE_H
