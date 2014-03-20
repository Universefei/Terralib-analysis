CONFIG += dll
TARGET = te_functions

include (../config.pri)

DEFINES += TLFUNCTIONS_AS_DLL

LIBS += -lterralib

INCLUDEPATH += $$TERRALIBPATH/src/terralib/functions

HEADERS += $$TERRALIBPATH/src/terralib/functions/TeFunctionsDefines.h \
	$$TERRALIBPATH/src/terralib/functions/TeCoordAlgorithms.h \
	$$TERRALIBPATH/src/terralib/functions/TeSPRFile.h \
	$$TERRALIBPATH/src/terralib/functions/TeDriverMIDMIF.h \
	$$TERRALIBPATH/src/terralib/functions/TeDriverSPRING.h \
	$$TERRALIBPATH/src/terralib/functions/TeGeoProcessingFunctions.h \
	$$TERRALIBPATH/src/terralib/functions/TeCellAlgorithms.h \
	$$TERRALIBPATH/src/terralib/functions/TeAddressLocator.h \
	$$TERRALIBPATH/src/terralib/functions/TeDriverBNA.h \
	$$TERRALIBPATH/src/terralib/functions/TeDriverCSV.h \
	$$TERRALIBPATH/src/terralib/functions/TeLayerFunctions.h \
	$$TERRALIBPATH/src/terralib/functions/TeMIFProjection.h \
	$$TERRALIBPATH/src/terralib/functions/TeThemeFunctions.h 
SOURCES += $$TERRALIBPATH/src/terralib/functions/TeCoordAlgorithms.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeSimilarity.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeSPRFile.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeExportMIF.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeExportSPR.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeGeoProcessingFunctions.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeCellAlgorithms.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeAddressLocator.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeImportBNA.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeImportCSV.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeImportGeo.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeImportMIF.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeLayerFunctions.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeMIFProjection.cpp \
	$$TERRALIBPATH/src/terralib/functions/TeExportCSV.cpp \ 
	$$TERRALIBPATH/src/terralib/functions/TeThemeFunctions.cpp 
