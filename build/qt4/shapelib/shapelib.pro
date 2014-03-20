TARGET  = shapelib
CONFIG += shared
win32:DEFINES += _USRDLL \
    SHAPELIB_DLLEXPORT
include (../config.pri)

SHPPATH = $${TERRALIBPATH}/src/shapelib

SOURCES = $${SHPPATH}/dbfopen.c \
    	$${SHPPATH}/shpopen.c
HEADERS = $${SHPPATH}/shapefil.h

include (../install_cfg.pri)

CONFIG(copy_dir_files) {
    include.path = $${DEPLOY_DIR}/include/shapelib/include
    include.files = $${SHPPATH}/*.h

    INSTALLS += include
}

DEPENDPATH += $${INCLUDEPATH}
