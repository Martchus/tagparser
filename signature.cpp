#include "./signature.h"
#include "./matroska/matroskatagid.h"

#include <c++utilities/conversion/binaryconversion.h>

#include <cstdint>

using namespace CppUtilities;

namespace TagParser {

/*!
 * \brief Holds 64-bit signatures.
 */
enum Sig64 : std::uint64_t {
    ApeTag = 0x4150455441474558ul, // APETAGEX
    Ar = 0x213C617263683E0A,
    Asf1 = 0x3026B2758E66CF11ul,
    Asf2 = 0xA6D900AA0062CE6Cul,
    Png = 0x89504E470D0A1A0Aul,
    RiffAvi = 0x415649204C495354ul,
    YUV4Mpeg2 = 0x595556344D504547ul,
};

/*!
 * \brief Holds 52-bit signatures.
 */
enum Sig56 : std::uint64_t {
    Rar = 0x526172211A0700ul,
};

/*!
 * \brief Holds 48-bit signatures.
 */
enum Sig48 : std::uint64_t {
    Gif87a = 0x474946383761ul,
    Gif89a = 0x474946383961ul,
    SevenZ = 0x377ABCAF271Cul,
    Xz = 0xFD377A585A00ul,
};

/*!
 * \brief Holds 40-bit signatures.
 */
enum Sig40 : std::uint64_t {
    Aiff = 0x464F524D00ul,
};

/*!
 * \brief Holds 32-bit signatures.
 */
enum Sig32 : std::uint32_t {
    Dirac = 0x42424344u,
    Elf = 0x7F454C46u,
    Flac = 0x664C6143u,
    Ivf = 0x444B4946u,
    JavaClassFile = 0xCAFEBABEu,
    Ebml = 0x1A45DFA3u,
    Midi = 0x4D546864u,
    MonkeysAudio = 0x4D414320u,
    Mp4 = 0x66747970u,
    Ogg = 0x4F676753u,
    PhotoshopDocument = 0x38425053u,
    QuickTime = 0x6D6F6F76u,
    Riff = 0x52494646u,
    RiffWave = 0x57415645u,
    TiffBigEndian = 0x4D4D002Au,
    TiffLittleEndian = 0x49492A00u,
    Utf32Text = 0xFFFE0000u,
    WavPack = 0x7776706Bu,
    WindowsIcon = 0x00000100u,
    Lzip = 0x4C5A4950u,
    Zip1 = 0x504B0304u,
    Zip2 = 0x504B0506u,
    Zip3 = 0x504B0708u,
    Zstd = 0x28b52ffdu,
};

/*!
 * \brief Holds 24-bit signatures.
 */
enum Sig24 : std::uint32_t {
    Bzip2 = 0x425A68u,
    Flv = 0x464C56u,
    Gzip = 0x1F8B08u,
    Id3v2 = 0x494433u,
    Utf8Text = 0xEFBBBFu,
};

/*!
 * \brief Holds 16-bit signatures.
 */
enum Sig16 : std::uint16_t {
    Ac3 = 0x0B77u,
    Adts = 0xFFF0u,
    AdtsMask = 0xFFF6u,
    Jpeg = 0xffd8u,
    Lha = 0x1FA0u,
    Lzw = 0x1F9Du,
    MpegAudioFrames = 0x7FFu,
    PortableExecutable = 0x4D5Au,
    Utf16Text = 0xFFFEu,
    WindowsBitmap = 0x424du,
};

/*!
 * \brief Parses the signature read from the specified \a buffer.
 * \param buffer Specifies the buffer to read the signature from.
 * \param bufferSize Specifies the size of \a buffer.
 * \return Returns the container format denoted by the signature. If the
 *         signature is unknown ContainerFormat::Unknown is returned.
 */
ContainerFormat parseSignature(std::string_view buffer)
{
    // read signature
    std::uint64_t sig = 0;
    if (buffer.size() >= 8) {
        sig = BE::toInt<std::uint64_t>(buffer.data());
    } else if (buffer.size() >= 4) {
        sig = BE::toInt<std::uint32_t>(buffer.data());
        sig <<= 4;
    } else if (buffer.size() >= 2) {
        sig = BE::toInt<std::uint16_t>(buffer.data());
        sig <<= 6;
    } else {
        return ContainerFormat::Unknown;
    }
    // return corresponding container format
    switch (sig) { // check 64-bit signatures
    case ApeTag:
        return ContainerFormat::ApeTag;
    case Ar:
        return ContainerFormat::Ar;
    case Asf1:
        return ContainerFormat::Asf;
    case Asf2:
        return ContainerFormat::Asf;
    case Png:
        return ContainerFormat::Png;
    case YUV4Mpeg2:
        if (buffer.size() >= 10 && buffer[8] == 0x32 && buffer[9] == 0x20) {
            return ContainerFormat::YUV4Mpeg2;
        }
        break;
    default:;
    }
    switch (sig & 0x00000000FFFFFFFF) { // check 32-bit signatures @ bit 31
    case Mp4:
        return ContainerFormat::Mp4;
    case QuickTime:
        return ContainerFormat::QuickTime;
    default:;
    }
    switch (sig >> 8) { // check 56-bit signatures
    case Rar:
        return ContainerFormat::Rar;
    default:;
    }
    switch (sig >> 16) { // check 48-bit signatures
    case Gif87a:
        return ContainerFormat::Gif87a;
    case Gif89a:
        return ContainerFormat::Gif89a;
    case SevenZ:
        return ContainerFormat::SevenZ;
    case Xz:
        return ContainerFormat::Xz;
    default:;
    }
    switch (sig >> 24) { // check 40-bit signatures
    case Aiff:
        return ContainerFormat::Aiff;
    default:;
    }
    switch (sig >> 32) { // check 32-bit signatures
    case Dirac:
        return ContainerFormat::Dirac;
    case Elf:
        return ContainerFormat::Elf;
    case Flac:
        return ContainerFormat::Flac;
    case Ivf:
        return ContainerFormat::Ivf;
    case JavaClassFile:
        return ContainerFormat::JavaClassFile;
    case Ebml:
        return ContainerFormat::Ebml;
    case Midi:
        return ContainerFormat::Midi;
    case MonkeysAudio:
        return ContainerFormat::MonkeysAudio;
    case Ogg:
        return ContainerFormat::Ogg;
    case PhotoshopDocument:
        return ContainerFormat::PhotoshopDocument;
    case Riff:
        if (buffer.size() >= 16 && BE::toInt<std::uint64_t>(buffer.data() + 8) == Sig64::RiffAvi) {
            return ContainerFormat::RiffAvi;
        } else if (buffer.size() >= 12 && BE::toInt<std::uint32_t>(buffer.data() + 8) == RiffWave) {
            return ContainerFormat::RiffWave;
        } else {
            return ContainerFormat::Riff;
        }
    case TiffBigEndian:
        return ContainerFormat::TiffBigEndian;
    case TiffLittleEndian:
        return ContainerFormat::TiffLittleEndian;
    case Utf32Text:
        return ContainerFormat::Utf32Text;
    case WavPack:
        return ContainerFormat::WavPack;
    case WindowsIcon:
        return ContainerFormat::WindowsIcon;
    case Lzip:
        return ContainerFormat::Lzip;
    case Zip1:
    case Zip2:
    case Zip3:
        return ContainerFormat::Zip;
    case Zstd:
        return ContainerFormat::Zstd;
    default:;
    }
    switch (sig >> 40) { // check 24-bit signatures
    case Bzip2:
        return ContainerFormat::Bzip2;
    case Flv:
        return ContainerFormat::FlashVideo;
    case Gzip:
        return ContainerFormat::Gzip;
    case Id3v2:
        return ContainerFormat::Id3v2Tag;
    case Utf8Text:
        return ContainerFormat::Utf8Text;
    }
    switch (sig >> 48) { // check 16-bit signatures
    case Ac3:
        return ContainerFormat::Ac3Frames;
    case Jpeg:
        return ContainerFormat::Jpeg;
    case Lha:
        return ContainerFormat::Lha;
    case Lzw:
        return ContainerFormat::Lzw;
    case PortableExecutable:
        return ContainerFormat::PortableExecutable;
    case Utf16Text:
        return ContainerFormat::Utf16Text;
    case WindowsBitmap:
        return ContainerFormat::WindowsBitmap;
    default:;
    }
    // check other signatures
    if (((sig >> 48) & AdtsMask) == Adts) {
        return ContainerFormat::Adts;
    }
    if ((sig >> 53) == MpegAudioFrames) {
        return ContainerFormat::MpegAudioFrames;
    }
    return ContainerFormat::Unknown;
}

/*!
 * \brief Returns the abbreviation of the container format as C-style string considering
 *        the specified media type and version.
 * \remarks The abbreviation might be used as file extension.
 * \returns Returns an empty string if no abbreviation is available.
 */
std::string_view containerFormatAbbreviation(ContainerFormat containerFormat, MediaType mediaType, unsigned int version)
{
    switch (containerFormat) {
    case ContainerFormat::Ac3Frames:
        return "ac3";
    case ContainerFormat::Ar:
        return "a";
    case ContainerFormat::Asf:
        return "asf";
    case ContainerFormat::Dirac:
        return "drc";
    case ContainerFormat::Elf:
        return "elf";
    case ContainerFormat::Flac:
        return "flac";
    case ContainerFormat::FlashVideo:
        return "flv";
    case ContainerFormat::Gif87a:
    case ContainerFormat::Gif89a:
        return "gif";
    case ContainerFormat::Ivf:
        return "ivf";
    case ContainerFormat::JavaClassFile:
        return "class";
    case ContainerFormat::Jpeg:
        return "jpeg";
    case ContainerFormat::Lha:
        return "lzh";
    case ContainerFormat::Lzw:
        return "lzw";
    case ContainerFormat::Mp4:
        switch (mediaType) {
        case MediaType::Audio:
            return "m4a";
        default:
            return "mp4";
        }
    case ContainerFormat::Ogg:
        switch (mediaType) {
        case MediaType::Video:
            return "ogv";
        default:
            switch (version) {
            case static_cast<unsigned int>(GeneralMediaFormat::Opus):
                return "opus";
            case static_cast<unsigned int>(GeneralMediaFormat::Speex):
                return "spx";
            default:
                return "ogg";
            }
        }
    case ContainerFormat::PhotoshopDocument:
        return "psd";
    case ContainerFormat::Png:
        return "png";
    case ContainerFormat::PortableExecutable:
        return "exe";
    case ContainerFormat::Rar:
        return "rar";
    case ContainerFormat::Matroska:
        switch (mediaType) {
        case MediaType::Audio:
            return "mka";
        default:
            return "mkv";
        }
    case ContainerFormat::MpegAudioFrames:
        switch (version) {
        case 1:
            return "mp1";
        case 2:
            return "mp2";
        default:
            return "mp3";
        }
    case ContainerFormat::Riff:
        return "riff";
    case ContainerFormat::RiffWave:
        return "wav";
    case ContainerFormat::RiffAvi:
        return "avi";
    case ContainerFormat::Tar:
        return "tar";
    case ContainerFormat::TiffBigEndian:
    case ContainerFormat::TiffLittleEndian:
        return "tiff";
    case ContainerFormat::WindowsBitmap:
        return "bmp";
    case ContainerFormat::WindowsIcon:
        return "ico";
    case ContainerFormat::Bzip2:
        return "bz";
    case ContainerFormat::Gzip:
        return "gz";
    case ContainerFormat::Lzip:
        return "lz";
    case ContainerFormat::QuickTime:
        return "mov";
    case ContainerFormat::Zip:
        return "zip";
    case ContainerFormat::SevenZ:
        return "7z";
    case ContainerFormat::Xz:
        return "xz";
    case ContainerFormat::YUV4Mpeg2:
        return "y4m";
    case ContainerFormat::WavPack:
        return "wv";
    case ContainerFormat::MonkeysAudio:
        return "ape";
    case ContainerFormat::Midi:
        return "mid";
    case ContainerFormat::Aiff:
        return "aiff";
    case ContainerFormat::Zstd:
        return "zst";
    default:
        return "";
    }
}

/*!
 * \brief Returns the name of the specified container format as C-style string.
 *
 * Returns "unknown" if no name is available.
 */
std::string_view containerFormatName(ContainerFormat containerFormat)
{
    switch (containerFormat) {
    case ContainerFormat::Ac3Frames:
        return "raw Dolby Digital";
    case ContainerFormat::Adts:
        return "Audio Data Transport Stream";
    case ContainerFormat::Ar:
        return "Archive (GNU ar)";
    case ContainerFormat::Asf:
        return "Advanced Systems Format";
    case ContainerFormat::Dirac:
        return "raw Dirac";
    case ContainerFormat::Elf:
        return "Executable and Linkable Format";
    case ContainerFormat::Flac:
        return "raw Free Lossless Audio Codec frames";
    case ContainerFormat::FlashVideo:
        return "Flash Video";
    case ContainerFormat::Gif87a:
    case ContainerFormat::Gif89a:
        return "Graphics Interchange Format";
    case ContainerFormat::Ivf:
        return "IVF";
    case ContainerFormat::JavaClassFile:
        return "Java class file";
    case ContainerFormat::Jpeg:
        return "JPEG File Interchange Format";
    case ContainerFormat::Lha:
        return "LHA compressed file";
    case ContainerFormat::Lzw:
        return "LZW compressed file";
    case ContainerFormat::Mp4:
        return "MPEG-4 Part 14";
    case ContainerFormat::Ogg:
        return "Ogg transport bitstream";
    case ContainerFormat::PhotoshopDocument:
        return "Photoshop document";
    case ContainerFormat::Png:
        return "Portable Network Graphics";
    case ContainerFormat::PortableExecutable:
        return "Portable Executable";
    case ContainerFormat::Rar:
        return "RAR Archive";
    case ContainerFormat::Ebml:
        return "EBML";
    case ContainerFormat::Matroska:
        return "Matroska";
    case ContainerFormat::Webm:
        return "WebM";
    case ContainerFormat::MpegAudioFrames:
        return "MPEG-1 Layer 1/2/3 frames";
    case ContainerFormat::Riff:
        return "Resource Interchange File Format";
    case ContainerFormat::RiffWave:
        return "RIFF/WAVE";
    case ContainerFormat::RiffAvi:
        return "RIFF/Audio Video Interleave";
    case ContainerFormat::Tar:
        return "TAR archive";
    case ContainerFormat::TiffBigEndian:
    case ContainerFormat::TiffLittleEndian:
        return "Tagged Image File Format";
    case ContainerFormat::Utf16Text:
        return "UTF-16 text";
    case ContainerFormat::Utf32Text:
        return "UTF-32 text";
    case ContainerFormat::Utf8Text:
        return "UTF-8 text";
    case ContainerFormat::WavPack:
        return "WavPack";
    case ContainerFormat::WindowsBitmap:
        return "Microsoft Windows Bitmap";
    case ContainerFormat::WindowsIcon:
        return "Microsoft Windows Icon";
    case ContainerFormat::Bzip2:
        return "bzip2 compressed file";
    case ContainerFormat::Gzip:
        return "gzip compressed file";
    case ContainerFormat::Lzip:
        return "lzip compressed file";
    case ContainerFormat::SevenZ:
        return "7z archive";
    case ContainerFormat::QuickTime:
        return "Quick Time";
    case ContainerFormat::Xz:
        return "xz compressed file";
    case ContainerFormat::YUV4Mpeg2:
        return "YUV4MPEG2";
    case ContainerFormat::Zip:
        return "ZIP archive";
    case ContainerFormat::MonkeysAudio:
        return "Monkey's Audio";
    case ContainerFormat::Midi:
        return "MIDI";
    case ContainerFormat::Aiff:
        return "Audio Interchange File Format";
    case ContainerFormat::Zstd:
        return "Zstandard compressed file";
    case ContainerFormat::Id3v2Tag:
        return "ID3v2 tag";
    case ContainerFormat::ApeTag:
        return "APE tag";
    default:
        return "unknown";
    }
}

/*!
 * \brief Returns the subversion of the container format as C-style string.
 *
 * Returns an empty string if there is no subversion available.
 */
std::string_view containerFormatSubversion(ContainerFormat containerFormat)
{
    switch (containerFormat) {
    case ContainerFormat::Gif87a:
        return "87a";
    case ContainerFormat::Gif89a:
        return "89a";
    case ContainerFormat::TiffBigEndian:
        return "big endian";
    case ContainerFormat::TiffLittleEndian:
        return "little endian";
    default:
        return "";
    }
}

/*!
 * \brief Returns the MIME-type of the container format as C-style string.
 *
 * Returns an empty string if there is no MIME-type available.
 */
std::string_view containerMimeType(ContainerFormat containerFormat, MediaType mediaType)
{
    switch (containerFormat) {
    case ContainerFormat::Ac3Frames:
        return "audio/ac3";
    case ContainerFormat::Asf:
        return "video/x-ms-asf";
    case ContainerFormat::Flac:
        return "audio/flac";
    case ContainerFormat::FlashVideo:
        return "video/x-flv";
    case ContainerFormat::Gif87a:
    case ContainerFormat::Gif89a:
        return "image/gif";
    case ContainerFormat::Jpeg:
        return "image/jpeg";
    case ContainerFormat::Png:
        return "image/png";
    case ContainerFormat::MpegAudioFrames:
        return "audio/mpeg";
    case ContainerFormat::Mp4:
        switch (mediaType) {
        case MediaType::Audio:
            return "audio/mp4";
        default:
            return "video/mp4";
        }
    case ContainerFormat::Ogg:
        switch (mediaType) {
        case MediaType::Audio:
            return "audio/ogg";
        default:
            return "video/ogg";
        }
    case ContainerFormat::Matroska:
        switch (mediaType) {
        case MediaType::Audio:
            return "audio/x-matroska";
        default:
            return "video/x-matroska";
        }
    case ContainerFormat::Midi:
        return "audio/midi";
    case ContainerFormat::Bzip2:
        return "application/x-bzip";
    case ContainerFormat::Gzip:
        return "application/gzip";
    case ContainerFormat::Lha:
        return "application/x-lzh-compressed";
    case ContainerFormat::Rar:
        return "application/x-rar-compressed";
    case ContainerFormat::Lzip:
        return "application/x-lzip";
    case ContainerFormat::QuickTime:
        return "video/quicktime";
    case ContainerFormat::Zip:
        return "application/zip";
    case ContainerFormat::SevenZ:
        return "application/x-7z-compressed";
    case ContainerFormat::Xz:
        return "application/x-xz";
    case ContainerFormat::WindowsBitmap:
        return "image/bmp";
    case ContainerFormat::WindowsIcon:
        return "image/vnd.microsoft.icon";
    case ContainerFormat::Zstd:
        return "application/zstd";
    default:
        return "";
    }
}

/*!
 * \brief Returns the general TagTargetLevel for the specified \a container format and raw \a targetLevelValue.
 */
TagTargetLevel containerTargetLevel(ContainerFormat containerFormat, std::uint64_t targetLevelValue)
{
    switch (containerFormat) {
    case ContainerFormat::Matroska:
    case ContainerFormat::Webm:
        return matroskaTagTargetLevel(targetLevelValue);
    default:
        return TagTargetLevel::Unspecified;
    }
}

/*!
 * \brief Returns the raw target level value for the specified \a containerFormat and general \a targetLevel.
 */
std::uint64_t containerTargetLevelValue(ContainerFormat containerFormat, TagTargetLevel targetLevel)
{
    switch (containerFormat) {
    case ContainerFormat::Matroska:
    case ContainerFormat::Webm:
        return matroskaTagTargetLevelValue(targetLevel);
    default:
        return 0;
    }
}

} // namespace TagParser
