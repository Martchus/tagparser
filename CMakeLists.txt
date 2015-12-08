cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# add project files
set(HEADER_FILES
    exceptions.h
    mp4/mp4atom.h
    mp4/mp4container.h
    mp4/mp4ids.h
    mp4/mp4tag.h
    mp4/mp4tagfield.h
    mp4/mp4track.h
    mp4/mpeg4descriptor.h
#    aac/aaccodebook.h
#    aac/aacframe.h
    abstractattachment.h
    abstractchapter.h
    abstractcontainer.h
    abstracttrack.h
    adts/adtsframe.h
    adts/adtsstream.h
#    avc/avcconfiguration.h
#    avc/avcinfo.h
    avi/bitmapinfoheader.h
    backuphelper.h
    basicfileinfo.h
    caseinsensitivecomparer.h
    mpegaudio/mpegaudioframe.h
    mpegaudio/mpegaudioframestream.h
    nestingsupportingtag.h
    notification.h
    ogg/oggcontainer.h
    ogg/oggiterator.h
    ogg/oggpage.h
    ogg/oggstream.h
    positioninset.h
    signature.h
    size.h
    statusprovider.h
    tag.h
    tagtarget.h
    tagvalue.h
    vorbis/vorbiscomment.h
    vorbis/vorbiscommentfield.h
    vorbis/vorbiscommentids.h
    vorbis/vorbisidentificationheader.h
    vorbis/vorbispackagetypes.h
    wav/waveaudiostream.h
    fieldbasedtag.h
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
    matroska/matroskaid.h
    matroska/ebmlelement.h
    matroska/ebmlid.h
    matroska/matroskaattachment.h
    matroska/matroskachapter.h
    matroska/matroskacodecid.h
    matroska/matroskacontainer.h
    matroska/matroskacues.h
    matroska/matroskaeditionentry.h
    matroska/matroskaseekinfo.h
    matroska/matroskatag.h
    matroska/matroskatagfield.h
    matroska/matroskatagid.h
    matroska/matroskatrack.h
    mediafileinfo.h
    mediaformat.h
)
set(SRC_FILES
    mp4/mp4atom.cpp
    mp4/mp4container.cpp
    mp4/mp4ids.cpp
    mp4/mp4tag.cpp
    mp4/mp4tagfield.cpp
    mp4/mp4track.cpp
    mp4/mpeg4descriptor.cpp
#    aac/aaccodebook.cpp
#    aac/aacframe.cpp
    abstractattachment.cpp
    abstractchapter.cpp
    abstractcontainer.cpp
    abstracttrack.cpp
    adts/adtsframe.cpp
    adts/adtsstream.cpp
#    avc/avcconfiguration.cpp
#    avc/avcinfo.cpp
    avi/bitmapinfoheader.cpp
    backuphelper.cpp
    basicfileinfo.cpp
    exceptions.cpp
    mpegaudio/mpegaudioframe.cpp
    mpegaudio/mpegaudioframestream.cpp
    notification.cpp
    ogg/oggcontainer.cpp
    ogg/oggiterator.cpp
    ogg/oggpage.cpp
    ogg/oggstream.cpp
    signature.cpp
    statusprovider.cpp
    tag.cpp
    tagtarget.cpp
    tagvalue.cpp
    vorbis/vorbiscomment.cpp
    vorbis/vorbiscommentfield.cpp
    vorbis/vorbiscommentids.cpp
    vorbis/vorbisidentificationheader.cpp
    wav/waveaudiostream.cpp
    id3/id3genres.cpp
    id3/id3v1tag.cpp
    id3/id3v2frame.cpp
    id3/id3v2frameids.cpp
    id3/id3v2tag.cpp
    localeawarestring.cpp
    matroska/ebmlelement.cpp
    matroska/matroskaattachment.cpp
    matroska/matroskachapter.cpp
    matroska/matroskacodecid.cpp
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
)

# meta data
set(META_PROJECT_NAME tagparser)
set(META_APP_NAME "Tag Parser")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "C++ library for reading and writing MP4 (iTunes), ID3, Vorbis and Matroska tags.")
set(META_VERSION_MAJOR 4)
set(META_VERSION_MINOR 0)
set(META_VERSION_PATCH 0)

# stringification of meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(META_APP_DESCRIPTION_STR "\"${META_APP_DESCRIPTION}\"")
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
set(META_APP_VERSION_STR "\"${META_APP_VERSION}\"")

# define project
project(${META_PROJECT_NAME})

# add configuration header
configure_file(
    "${PROJECT_SOURCE_DIR}/resources/config.h.in"
    "${PROJECT_BINARY_DIR}/resources/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

# add windows resource file
if(MINGW)
    # creation windows icon from png with ffmpeg if available
    find_program(FFMPEG_BIN ffmpeg avconv)
    if(FFMPEG_BIN)
        set(PNG_ICON_PATH "${PROJECT_SOURCE_DIR}/resources/icons/hicolor/128x128/apps/${META_PROJECT_NAME}.png")
        if(EXISTS ${PNG_ICON_PATH})
            set(WINDOWS_ICON_PATH "${PROJECT_BINARY_DIR}/${META_PROJECT_NAME}.ico")
            set(WINDOWS_ICON_RC_ENTRY "IDI_ICON1   ICON    DISCARDABLE \"${WINDOWS_ICON_PATH}\"")
            add_custom_command(
                OUTPUT "${WINDOWS_ICON_PATH}"
                COMMAND ${FFMPEG_BIN} -y -i "${PNG_ICON_PATH}" -vf crop=iw-20:ih-20:10:10,scale=64:64 "${WINDOWS_ICON_PATH}"
            )
        endif()
    endif(FFMPEG_BIN)
    # create windows rc file from template
    set(WINDOWS_EXT "dll")
    configure_file(
        "${PROJECT_SOURCE_DIR}/resources/windows.rc.in"
        "${PROJECT_BINARY_DIR}/resources/windows.rc"
    )
    # set windres as resource compiler
    set(RES_FILES "${PROJECT_BINARY_DIR}/resources/windows.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    enable_language(RC)
endif(MINGW)

# remove library prefix when building with mingw-w64 (just for consistancy with qmake)
if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(MINGW)

# executable and linking
add_library(${META_PROJECT_NAME} SHARED ${HEADER_FILES} ${SRC_FILES} ${RES_FILES} ${WINDOWS_ICON_PATH})
target_link_libraries(${META_PROJECT_NAME} c++utilities z)
set_target_properties(${META_PROJECT_NAME} PROPERTIES
    VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}
    SOVERSION ${META_VERSION_MAJOR}
    CXX_STANDARD 11
)

# add install target
install(TARGETS ${META_PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT binary
    LIBRARY DESTINATION lib
    COMPONENT binary
    ARCHIVE DESTINATION lib
    COMPONENT binary
)
foreach(HEADER_FILE ${HEADER_FILES})
    get_filename_component(HEADER_DIR ${HEADER_FILE} DIRECTORY)
    install(
        FILES ${HEADER_FILE}
        DESTINATION include/${META_PROJECT_NAME}/${HEADER_DIR}
        COMPONENT header
    )
endforeach()
add_custom_target(install-binary
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-header
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=header -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-mingw-w64
    DEPENDS install-binary install-header
)
add_custom_target(install-binary-strip
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-mingw-w64-strip
    DEPENDS install-binary-strip install-header
)
