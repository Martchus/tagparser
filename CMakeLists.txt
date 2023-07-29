cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# meta data
project(tagparser)
set(META_PROJECT_NAME ${PROJECT_NAME})
set(META_PROJECT_VARNAME TAG_PARSER)
set(META_APP_NAME "Tag Parser")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "C++ library for reading and writing MP4 (iTunes), ID3, Vorbis, Opus, FLAC and Matroska tags")
set(META_VERSION_MAJOR 12)
set(META_VERSION_MINOR 1)
set(META_VERSION_PATCH 0)
set(META_REQUIRED_CPP_UNIT_VERSION 1.14.0)
set(META_ADD_DEFAULT_CPP_UNIT_TEST_APPLICATION ON)

# add project files
set(HEADER_FILES
    aac/aaccodebook.h
    aac/aacframe.h
    abstractattachment.h
    abstractchapter.h
    abstractcontainer.h
    abstracttrack.h
    adts/adtsframe.h
    adts/adtsstream.h
    aspectratio.h
    avc/avcconfiguration.h
    avc/avcinfo.h
    avi/bitmapinfoheader.h
    backuphelper.h
    basicfileinfo.h
    caseinsensitivecomparer.h
    diagnostics.h
    exceptions.h
    fieldbasedtag.h
    flac/flacmetadata.h
    flac/flacstream.h
    flac/flactooggmappingheader.h
    genericcontainer.h
    genericfileelement.h
    generictagfield.h
    id3/id3genres.h
    id3/id3v1tag.h
    id3/id3v2frame.h
    id3/id3v2frameids.h
    id3/id3v2tag.h
    ivf/ivfframe.h
    ivf/ivfstream.h
    localehelper.h
    localeawarestring.h
    margin.h
    matroska/ebmlelement.h
    matroska/ebmlid.h
    matroska/matroskaattachment.h
    matroska/matroskachapter.h
    matroska/matroskacontainer.h
    matroska/matroskacues.h
    matroska/matroskaeditionentry.h
    matroska/matroskaid.h
    matroska/matroskaseekinfo.h
    matroska/matroskatag.h
    matroska/matroskatagfield.h
    matroska/matroskatagid.h
    matroska/matroskatrack.h
    mediafileinfo.h
    mediaformat.h
    mp4/mp4atom.h
    mp4/mp4container.h
    mp4/mp4ids.h
    mp4/mp4tag.h
    mp4/mp4tagfield.h
    mp4/mp4track.h
    mp4/mpeg4descriptor.h
    mpegaudio/mpegaudioframe.h
    mpegaudio/mpegaudioframestream.h
    ogg/oggcontainer.h
    ogg/oggiterator.h
    ogg/oggpage.h
    ogg/oggstream.h
    opus/opusidentificationheader.h
    positioninset.h
    progressfeedback.h
    settings.h
    signature.h
    size.h
    tag.h
    tagtarget.h
    tagtype.h
    tagvalue.h
    vorbis/vorbiscomment.h
    vorbis/vorbiscommentfield.h
    vorbis/vorbiscommentids.h
    vorbis/vorbisidentificationheader.h
    vorbis/vorbispackagetypes.h
    wav/waveaudiostream.h)
set(SRC_FILES
    aac/aaccodebook.cpp
    aac/aacframe.cpp
    abstractattachment.cpp
    abstractchapter.cpp
    abstractcontainer.cpp
    abstracttrack.cpp
    adts/adtsframe.cpp
    adts/adtsstream.cpp
    aspectratio.cpp
    av1/av1configuration.h
    av1/av1configuration.cpp
    avc/avcconfiguration.cpp
    avc/avcinfo.cpp
    avi/bitmapinfoheader.cpp
    backuphelper.cpp
    basicfileinfo.cpp
    diagnostics.cpp
    exceptions.cpp
    flac/flacmetadata.cpp
    flac/flacstream.cpp
    flac/flactooggmappingheader.cpp
    id3/id3genres.cpp
    id3/id3v1tag.cpp
    id3/id3v2frame.cpp
    id3/id3v2frameids.cpp
    id3/id3v2tag.cpp
    ivf/ivfframe.cpp
    ivf/ivfstream.cpp
    localehelper.cpp
    localeawarestring.cpp
    matroska/ebmlelement.cpp
    matroska/matroskaattachment.cpp
    matroska/matroskachapter.cpp
    matroska/matroskacontainer.cpp
    matroska/matroskacues.cpp
    matroska/matroskaeditionentry.cpp
    matroska/matroskaid.cpp
    matroska/matroskaseekinfo.cpp
    matroska/matroskatag.cpp
    matroska/matroskatagfield.cpp
    matroska/matroskatagid.cpp
    matroska/matroskatrack.cpp
    mediafileinfo.cpp
    mediaformat.cpp
    mp4/mp4atom.cpp
    mp4/mp4container.cpp
    mp4/mp4ids.cpp
    mp4/mp4tag.cpp
    mp4/mp4tagfield.cpp
    mp4/mp4track.cpp
    mp4/mpeg4descriptor.cpp
    mpegaudio/mpegaudioframe.cpp
    mpegaudio/mpegaudioframestream.cpp
    ogg/oggcontainer.cpp
    ogg/oggiterator.cpp
    ogg/oggpage.cpp
    ogg/oggstream.cpp
    opus/opusidentificationheader.cpp
    progressfeedback.cpp
    signature.cpp
    size.cpp
    tag.cpp
    tagtarget.cpp
    tagvalue.cpp
    vorbis/vorbiscomment.cpp
    vorbis/vorbiscommentfield.cpp
    vorbis/vorbisidentificationheader.cpp
    wav/waveaudiostream.cpp)
set(TEST_HEADER_FILES tests/helper.h tests/overall.h)
set(TEST_SRC_FILES
    tests/helper.cpp
    tests/mediafileinfo.cpp
    tests/overallflac.cpp
    tests/overallgeneral.cpp
    tests/overallmkv.cpp
    tests/overallmp3.cpp
    tests/overallmp4.cpp
    tests/overallogg.cpp
    tests/tagvalue.cpp
    tests/testfilecheck.cpp
    tests/utils.cpp
    doc/example.cpp)
set(DOC_FILES README.md doc/adding-new-fields.md)
set(LANGUAGE_HEADER_ISO_639_2 "${CMAKE_CURRENT_BINARY_DIR}/resources/iso_language_codes.h")
set(RES_FILES "${LANGUAGE_HEADER_ISO_639_2}")

# find c++utilities
set(CONFIGURATION_PACKAGE_SUFFIX
    ""
    CACHE STRING "sets the suffix for find_package() calls to packages configured via c++utilities")
find_package(c++utilities${CONFIGURATION_PACKAGE_SUFFIX} 5.21.0 REQUIRED)
use_cpp_utilities(VISIBILITY PUBLIC)

# link against a possibly required extra library for std::filesystem
use_standard_filesystem()

# find 3rd party libraries
include(3rdParty)
# zlib
use_zlib()
use_crypto(LIBRARIES_VARIABLE "TEST_LIBRARIES" PACKAGES_VARIABLE "TEST_PACKAGES" OPTIONAL)
if (NOT "OpenSSL::Crypto" IN_LIST "TEST_LIBRARIES")
    list(REMOVE_ITEM TEST_SRC_FILES tests/testfilecheck.cpp)
    message(WARNING "Unable to check testfile integrity because OpenSSL is not available.")
endif ()

# include modules to apply configuration
include(BasicConfig)
include(WindowsResources)
include(LibraryTarget)
include(TestTarget)
include(Doxygen)
include(ConfigHeader)

# add rules to generate code for dealing with language codes
set(LANGUAGE_FILE_ISO_639_2
    ""
    CACHE STRING "path to JSON document from https://salsa.debian.org/iso-codes-team/iso-codes for ISO-639-2")
add_custom_command(
    OUTPUT "${LANGUAGE_HEADER_ISO_639_2}"
    COMMENT "Generating code for ISO-639-2 language codes"
    COMMAND "${CMAKE_COMMAND}" "-DLANGUAGE_FILE=${LANGUAGE_FILE_ISO_639_2}" "-DOUTPUT_PATH=${LANGUAGE_HEADER_ISO_639_2}" -P
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/generate_iso_language_codes.cmake")
