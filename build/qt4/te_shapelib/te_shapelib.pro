CONFIG += shared
TARGET = te_shapelib

include (../config.pri)

# Path definitions
# ----------------------------------------------------------
SHPSRCPATH = $${TERRALIBPATH}/src/terralib/drivers/shapelib
# ----------------------------------------------------------

DEFINES += TLSHP_AS_DLL

LIBS += -lshapelib -lterralib 

INCLUDEPATH += $${SHPSRCPATH}  \
	      $${TERRALIBPATH}/src/shapelib

HEADERS += $${SHPSRCPATH}/TeDriverSHPDBF.h \
        $${SHPSRCPATH}/TeSHPDriverFactory.h \
        $${SHPSRCPATH}/TeSHPDefines.h
SOURCES += $${SHPSRCPATH}/TeDriverSHPDBF.cpp \
        $${SHPSRCPATH}/TeSHPDriverFactory.cpp
	
include (../install_cfg.pri)	

CONFIG(copy_dir_files)
{
    include.path = $${DEPLOY_DIR}/include/shapelib
    include.files = $${HEADERS}

    INSTALLS += include
}

DEPENDPATH += $${INCLUDEPATH}
