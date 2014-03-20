CONFIG += shared
TARGET = te_firebird
include (../config.pri)
# Path definitions
# ----------------------------------------------------------
FBSRCPATH = $${TERRALIBPATH}/src/terralib/drivers/Firebird
FBDEP = $${TERRALIBPATH}/dependencies
win32 {
    DEFINES += _WIN32
    contains(QMAKE_HOST.arch, x86_64):FBDEP = $${FBDEP}/win64
    else:FBDEP = $${FBDEP}/win32
}
unix:FBDEP = $${FBDEP}/linux
FBDEP = $${FBDEP}/Firebird
# ----------------------------------------------------------
DEFINES += TLFIREBIRD_AS_DLL
LIBS += -lterralib 
unix:LIBS += -L$${FBDEP}/lib \
		-lfbembed
win32:DEFINES += IBPP_WINDOWS
win32-g++:DEFINES += _LP64
unix:DEFINES += IBPP_LINUX
INCLUDEPATH += $${FBSRCPATH}
HEADERS += $${FBSRCPATH}/ibpp/core/_ibpp.h \
          $${FBSRCPATH}/ibpp/core/ibase.h \
          $${FBSRCPATH}/ibpp/core/iberror.h \
          $${FBSRCPATH}/ibpp/core/ibpp.h \
          $${FBSRCPATH}/TeFirebird.h \
          $${FBSRCPATH}/TeFirebirdDefines.h
SOURCES += $${FBSRCPATH}/TeFirebird.cpp \
        $${FBSRCPATH}/ibpp/core/all_in_one.cpp
#        $${FBSRCPATH}/ibpp/core/database.cpp
include (../install_cfg.pri)
CONFIG(copy_dir_files) {
    #Firebird dependencies install
    ibpp.path = $${DEPLOY_DIR}/include/Firebird/ibpp/core
    ibpp.files = $${FBSRCPATH}/ibpp/core/*.h

    include.path = $${DEPLOY_DIR}/include/Firebird
    include.files = $${FBSRCPATH}/*.h

    INSTALLS += ibpp include
}
unix {
    cop_dps.commands = cp -f $${FBDEP}/lib/* $${target.path} && \
                       cd $${target.path} && ln -sf libicudata.so.30 libicudata.so && ln -sf libicui18n.so.30 libicui18n.so &&  ln -sf libicuuc.so.30 libicuuc.so
    QMAKE_EXTRA_TARGETS += cop_dps
    POST_TARGETDEPS += cop_dps
}
win32 {
    bin.files = $${FBDEP}/bin/*.dll
    bin.path = $${DEPLOY_DIR}/bin
    INSTALLS += bin
}

DEPENDPATH += $${INCLUDEPATH}
