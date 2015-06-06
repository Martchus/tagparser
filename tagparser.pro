projectname = tagparser

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

TEMPLATE = lib

CONFIG -= qt

win32 {
    CONFIG += dll
}

SOURCES += \
    abstractcontainer.cpp \
    abstracttrack.cpp \
    backuphelper.cpp \
    basicfileinfo.cpp \
    exceptions.cpp \
    id3/id3genres.cpp \
    id3/id3v1tag.cpp \
    id3/id3v2frame.cpp \
    id3/id3v2frameids.cpp \
    id3/id3v2tag.cpp \
    matroska/ebmlelement.cpp \
    matroska/matroskacodecid.cpp \
    matroska/matroskacontainer.cpp \
    matroska/matroskaid.cpp \
    matroska/matroskatag.cpp \
    matroska/matroskatagfield.cpp \
    matroska/matroskatagid.cpp \
    matroska/matroskatrack.cpp \
    matroska/matroskaseekinfo.cpp \
    matroska/matroskacues.cpp \
    mediafileinfo.cpp \
    mp4/mp4atom.cpp \
    mp4/mp4container.cpp \
    mp4/mp4ids.cpp \
    mp4/mp4tag.cpp \
    mp4/mp4tagfield.cpp \
    mp4/mp4track.cpp \
    mpegaudio/mpegaudioframe.cpp \
    mpegaudio/mpegaudioframestream.cpp \
    notification.cpp \
    signature.cpp \
    statusprovider.cpp \
    tag.cpp \
    tagtarget.cpp \
    tagvalue.cpp \
    wav/waveaudiostream.cpp \
    avc/avcinfo.cpp \
    ogg/oggpage.cpp \
    vorbis/vorbiscomment.cpp \
    vorbis/vorbiscommentfield.cpp \
    ogg/oggstream.cpp \
    ogg/oggcontainer.cpp \
    vorbis/vorbisidentificationheader.cpp \
    ogg/oggiterator.cpp \
    vorbis/vorbiscommentids.cpp \
    abstractchapter.cpp \
    matroska/matroskaeditionentry.cpp \
    matroska/matroskachapter.cpp \
    abstractattachment.cpp \
    matroska/matroskaattachment.cpp \
    mediaformat.cpp \
    avc/avcconfiguration.cpp \
    mp4/mpeg4descriptor.cpp

HEADERS  += \
    abstractcontainer.h \
    abstracttrack.h \
    backuphelper.h \
    basicfileinfo.h \
    exceptions.h \
    fieldbasedtag.h \
    genericcontainer.h \
    genericfileelement.h \
    id3/id3genres.h \
    id3/id3v1tag.h \
    id3/id3v2frame.h \
    id3/id3v2frameids.h \
    id3/id3v2tag.h \
    margin.h \
    matroska/ebmlelement.h \
    matroska/ebmlid.h \
    matroska/matroskacodecid.h \
    matroska/matroskacontainer.h \
    matroska/matroskaid.h \
    matroska/matroskatag.h \
    matroska/matroskatagfield.h \
    matroska/matroskatagid.h \
    matroska/matroskatrack.h \
    matroska/matroskaseekinfo.h \
    matroska/matroskacues.h \
    matroskaid.h \
    mediafileinfo.h \
    mp4/mp4atom.h \
    mp4/mp4container.h \
    mp4/mp4ids.h \
    mp4/mp4tag.h \
    mp4/mp4tagfield.h \
    mp4/mp4track.h \
    mpegaudio/mpegaudioframe.h \
    mpegaudio/mpegaudioframestream.h \
    notification.h \
    positioninset.h \
    signature.h \
    size.h \
    statusprovider.h \
    tag.h \
    tagtarget.h \
    tagvalue.h \
    wav/waveaudiostream.h \
    avc/avcinfo.h \
    ogg/oggpage.h \
    vorbis/vorbiscomment.h \
    vorbis/vorbiscommentfield.h \
    ogg/oggstream.h \
    ogg/oggcontainer.h \
    vorbis/vorbispackagetypes.h \
    vorbis/vorbisidentificationheader.h \
    ogg/oggiterator.h \
    vorbis/vorbiscommentids.h \
    abstractchapter.h \
    matroska/matroskaeditionentry.h \
    matroska/matroskachapter.h \
    localeawarestring.h \
    abstractattachment.h \
    matroska/matroskaattachment.h \
    mediaformat.h \
    avc/avcconfiguration.h \
    generictagfield.h \
    mp4/mpeg4descriptor.h

LIBS += -lz

CONFIG(debug, debug|release) {
       LIBS += -L../../ -lc++utilitiesd
} else {
       LIBS += -L../../ -lc++utilities
}

forcefullparsedefault {
    DEFINES += FORCE_FULL_PARSE_DEFAULT
}

OTHER_FILES += \
    pkgbuild/default/PKGBUILD \
    pkgbuild/mingw-w64/PKGBUILD

INCLUDEPATH += ../

# installs
target.path = $$(INSTALL_ROOT)/lib
INSTALLS += target
for(dir, $$list(./ avc id3 matroska mp4 mpegaudio ogg vorbis wav)) {
    eval(inc_$${dir} = $${dir})
    inc_$${dir}.path = $$(INSTALL_ROOT)/include/$$projectname/$${dir}
    inc_$${dir}.files = $${dir}/*.h
    INSTALLS += inc_$${dir}
}

