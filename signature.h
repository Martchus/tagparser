#ifndef TAG_PARSER_SIGNATURE_H
#define TAG_PARSER_SIGNATURE_H

#include "./mediaformat.h"

#include <cstdint>
#include <string_view>

namespace TagParser {

enum class TagTargetLevel : std::uint8_t;

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
    Id3v2Tag, /**< file holding an ID3v2 tag only */
    Ivf, /**< IVF (simple file format that transports raw VP8/VP9/AV1 data) */
    JavaClassFile, /**< Java class file */
    Jpeg, /**< JPEG File Interchange Format */
    Lha, /**< LHA */
    Lzip, /**< lz compressed file */
    Lzw, /**< LZW */
    Matroska, /**< Matroska (subset of EBML) */
    Midi, /**< Musical Instrument Digital Interface (MIDI) */
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
    Aiff, /**< Audio Interchange File Format */
    Zstd, /**< Zstandard-compressed data */
    ApeTag, /**< APE tag */
};

TAG_PARSER_EXPORT ContainerFormat parseSignature(const char *buffer, std::size_t bufferSize);
TAG_PARSER_EXPORT ContainerFormat parseSignature(std::string_view buffer);
TAG_PARSER_EXPORT std::string_view containerFormatName(ContainerFormat containerFormat);
TAG_PARSER_EXPORT std::string_view containerFormatAbbreviation(
    ContainerFormat containerFormat, MediaType mediaType = MediaType::Unknown, unsigned int version = 0);
TAG_PARSER_EXPORT std::string_view containerFormatSubversion(ContainerFormat containerFormat);
TAG_PARSER_EXPORT std::string_view containerMimeType(ContainerFormat containerFormat, MediaType mediaType = MediaType::Unknown);
TAG_PARSER_EXPORT TagTargetLevel containerTargetLevel(ContainerFormat containerFormat, std::uint64_t targetLevelValue);
TAG_PARSER_EXPORT std::uint64_t containerTargetLevelValue(ContainerFormat containerFormat, TagTargetLevel targetLevel);

inline ContainerFormat parseSignature(const char *buffer, std::size_t bufferSize)
{
    return parseSignature(std::string_view(buffer, bufferSize));
}

} // namespace TagParser

#endif // TAG_PARSER_SIGNATURE_H
