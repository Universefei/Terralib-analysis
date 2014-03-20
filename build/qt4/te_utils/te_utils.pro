CONFIG += dll
TARGET = te_utils

include (../config.pri)

# Path definitions
# ----------------------------------------------------------
UTSRCPATH = $${TERRALIBPATH}/src/terralib/utils
# ----------------------------------------------------------

DEFINES += TLUTILS_AS_DLL

LIBS += -lterralib -lte_functions
win32 {
    LIBS += -lwsock32
    win32-g++:LIBS += -lws2_32
}

INCLUDEPATH += $${UTSRCPATH} \
	      $${TERRALIBPATH}/src/terralib/functions

HEADERS += $${UTSRCPATH}/TeUtilsDefines.h \
        $${UTSRCPATH}/TeColorUtils.h \
        $${UTSRCPATH}/TeDatabaseUtils.h \
        $${UTSRCPATH}/mtrand.h \
        $${UTSRCPATH}/TeUpdateDBVersion.h \
        $${UTSRCPATH}/TeWKBGeometryDecoder.h \
        $${UTSRCPATH}/TeWKTGeometryDecoder.h
SOURCES += $${UTSRCPATH}/TeColorUtils.cpp \
        $${UTSRCPATH}/TeDatabaseUtils.cpp \
        $${UTSRCPATH}/mtrand.cpp \
        $${UTSRCPATH}/TeUpdateDBVersion.cpp \
        $${UTSRCPATH}/TeWKBGeometryDecoder.cpp \
        $${UTSRCPATH}/TeWKTGeometryDecoder.cpp

include (../install_cfg.pri)

CONFIG(copy_dir_files) {
    include.path = $${DEPLOY_DIR}/include/utils
    include.files = $${HEADERS}

    INSTALLS += include
}
