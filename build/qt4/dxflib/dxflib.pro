TARGET = dxf
win32 {
    win32-g++:CONFIG += shared
    else:CONFIG += staticlib
}
unix:CONFIG += shared

include (../config.pri)

SRCPATH = $${TERRALIBPATH}/src/dxflib

win32-msvc:DEFINES += _CRT_SECURE_NO_DEPRECATE

macx {
  QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
  CONFIG+=x86 ppc
}

INCLUDEPATH += $${SRCPATH}
HEADERS += $${SRCPATH}/dl_attributes.h\
    $${SRCPATH}/dl_codes.h\
    $${SRCPATH}/dl_creationadapter.h\
    $${SRCPATH}/dl_creationinterface.h\
    $${SRCPATH}/dl_dxf.h\
    $${SRCPATH}/dl_entities.h\
    $${SRCPATH}/dl_exception.h\
    $${SRCPATH}/dl_extrusion.h\
    $${SRCPATH}/dl_writer.h\
    $${SRCPATH}/dl_writer_ascii.h
SOURCES += $${SRCPATH}/dl_dxf.cpp\
    $${SRCPATH}/dl_writer_ascii.cpp

include (../install_cfg.pri)

DEPENDPATH += $${INCLUDEPATH}
