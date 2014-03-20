CONFIG += shared
TARGET = te_functions

include (../config.pri)

# Path definitions
# ----------------------------------------------------------
FCSRCPATH = $${TERRALIBPATH}/src/terralib/functions
# ----------------------------------------------------------


DEFINES += TLFUNCTIONS_AS_DLL

LIBS += -lterralib

INCLUDEPATH += $${FCSRCPATH}

HEADERS += $${FCSRCPATH}/TeFunctionsDefines.h \
        $${FCSRCPATH}/TeCoordAlgorithms.h \
        $${FCSRCPATH}/TeSPRFile.h \
        $${FCSRCPATH}/TeDriverMIDMIF.h \
        $${FCSRCPATH}/TeDriverSPRING.h \
        $${FCSRCPATH}/TeGeoProcessingFunctions.h \
        $${FCSRCPATH}/TeCellAlgorithms.h \
        $${FCSRCPATH}/TeAddressLocator.h \
        $${FCSRCPATH}/TeDriverBNA.h \
        $${FCSRCPATH}/TeDriverCSV.h \
        $${FCSRCPATH}/TeLayerFunctions.h \
        $${FCSRCPATH}/TeMIFProjection.h \
        $${FCSRCPATH}/TeThemeFunctions.h
SOURCES += $${FCSRCPATH}/TeCoordAlgorithms.cpp \
        $${FCSRCPATH}/TeSimilarity.cpp \
        $${FCSRCPATH}/TeSPRFile.cpp \
        $${FCSRCPATH}/TeExportMIF.cpp \
        $${FCSRCPATH}/TeExportSPR.cpp \
        $${FCSRCPATH}/TeGeoProcessingFunctions.cpp \
        $${FCSRCPATH}/TeCellAlgorithms.cpp \
        $${FCSRCPATH}/TeAddressLocator.cpp \
        $${FCSRCPATH}/TeImportBNA.cpp \
        $${FCSRCPATH}/TeImportCSV.cpp \
        $${FCSRCPATH}/TeImportGeo.cpp \
        $${FCSRCPATH}/TeImportMIF.cpp \
        $${FCSRCPATH}/TeLayerFunctions.cpp \
        $${FCSRCPATH}/TeMIFProjection.cpp \
        $${FCSRCPATH}/TeExportCSV.cpp \
        $${FCSRCPATH}/TeThemeFunctions.cpp

include (../install_cfg.pri)

CONFIG(copy_dir_files) {
    include.path = $${DEPLOY_DIR}/include/functions
    include.files = $${HEADERS}

    INSTALLS += include
}

DEPENDPATH += $${INCLUDEPATH}
