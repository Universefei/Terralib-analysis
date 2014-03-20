CONFIG += shared
TARGET = te_oracle

include (../config.pri)
# Path definitions
# ----------------------------------------------------------
ORSRCPATH = $${TERRALIBPATH}/src/terralib/drivers/Oracle
ORDEP = $${TERRALIBPATH}/dependencies
win32:ORDEP = $${ORDEP}/win32
unix:ORDEP = $${ORDEP}/linux
ORDEP = $${ORDEP}/Oracle
# ----------------------------------------------------------

DEFINES += TLORACLE_AS_DLL

LIBS += -lterralib 

win32 {
     !win32-g++:QMAKE_LIBDIR += $${ORDEP}/lib
     else:LIBS += -L$${ORDEP}/lib

     LIBS += -loci
}

unix:LIBS += -L$${ORDEP}/lib \
		-lclntsh -lnnz10 -lociei

INCLUDEPATH += $${ORSRCPATH}  \
              $${ORSRCPATH}/OCI/include

HEADERS += $${ORSRCPATH}/TeOracleSpatial.h \
        $${ORSRCPATH}/TeOCIOracle.h \
        $${ORSRCPATH}/TeOCIConnect.h \
        $${ORSRCPATH}/TeOCICursor.h \
        $${ORSRCPATH}/TeOCISDO.h \
        $${ORSRCPATH}/TeOracleDefines.h

SOURCES += $${ORSRCPATH}/TeOracleSpatial.cpp \
        $${ORSRCPATH}/TeOCIOracle.cpp \
        $${ORSRCPATH}/TeOCIConnect.cpp \
        $${ORSRCPATH}/TeOCICursor.cpp \
        $${ORSRCPATH}/TeOCISDO.cpp

unix {
    create_link.commands = cd $${ORDEP}/lib/ && ln -sf libclntsh.so.10.1 libclntsh.so
    QMAKE_EXTRA_TARGETS += create_link
    PRE_TARGETDEPS += create_link
}

include (../install_cfg.pri)

CONFIG(copy_dir_files) {
    ora.path = $${DEPLOY_DIR}/include/Oracle/OCI/include
    ora.files = $${ORSRCPATH}/OCI/include/*.h

    include.path = $${DEPLOY_DIR}/include/Oracle
    include.files = $${ORSRCPATH}/*.h

    INSTALLS += ora include
}
#Oracle dependencies install
unix {
    cop_dps.commands = cp -f $${ORDEP}/lib/* $${target.path}
    QMAKE_EXTRA_TARGETS += cop_dps
    POST_TARGETDEPS += cop_dps
}
win32 {
    bin.files = $${ORDEP}/bin/*.dll
    bin.path = $${DEPLOY_DIR}/bin
    INSTALLS += bin
}

DEPENDPATH += $${INCLUDEPATH}
