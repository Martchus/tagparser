cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

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
    tagvalue.h
    vorbis/vorbiscomment.h
    vorbis/vorbiscommentfield.h
    vorbis/vorbiscommentids.h
    vorbis/vorbisidentificationheader.h
    vorbis/vorbispackagetypes.h
    wav/waveaudiostream.h
)
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
    wav/waveaudiostream.cpp
)
set(TEST_HEADER_FILES
    tests/helper.h
    tests/overall.h
)
set(TEST_SRC_FILES
    tests/cppunit.cpp
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
)

set(DOC_FILES
    README.md
)

# meta data
set(META_PROJECT_NAME tagparser)
set(META_PROJECT_VARNAME TAG_PARSER)
set(META_APP_NAME "Tag Parser")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "C++ library for reading and writing MP4 (iTunes), ID3, Vorbis, Opus, FLAC and Matroska tags")
set(META_VERSION_MAJOR 8)
set(META_VERSION_MINOR 0)
set(META_VERSION_PATCH 0)
set(META_PUBLIC_SHARED_LIB_DEPENDS c++utilities)
set(META_PUBLIC_STATIC_LIB_DEPENDS c++utilities_static)
set(META_PRIVATE_COMPILE_DEFINITIONS LEGACY_API)
set(META_REQUIRED_CPP_UNIT_VERSION 1.14.0)

# find c++utilities
find_package(c++utilities 4.9.0 REQUIRED)
use_cpp_utilities()

# find 3rd party libraries
include(3rdParty)
# zlib
use_external_library_from_package(
    z
    ZLIB ANY_VERSION
    ZLIB_INCLUDE_DIRS
    ZLIB_LIBRARIES
    AUTO_LINKAGE
    REQUIRED
)
# crypto (optional for testing integrity of testfiles)
find_external_library_from_package(
    crypto
    OpenSSL ANY_VERSION
    OPENSSL_INCLUDE_DIR
    OPENSSL_CRYPTO_LIBRARY
    AUTO_LINKAGE
    OPTIONAL
)
link_tests_against_library(
    crypto AUTO_LINKAGE OPTIONAL
)
if(NOT USE_crypto)
    list(REMOVE_ITEM TEST_SRC_FILES tests/testfilecheck.cpp)
    message(WARNING "Unable to check testfile integrity because OpenSSL is not available.")
endif()

# include modules to apply configuration
include(BasicConfig)
include(WindowsResources)
include(LibraryTarget)
include(TestTarget)
include(Doxygen)
include(ConfigHeader)
