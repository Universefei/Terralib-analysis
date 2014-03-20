CONFIG += dll
TARGET = te_utils

include (../config.pri)

DEFINES += TLUTILS_AS_DLL

LIBS += -lterralib -lte_functions

INCLUDEPATH += $$TERRALIBPATH/src/terralib/utils \
	      $$TERRALIBPATH/src/terralib/functions

HEADERS += $$TERRALIBPATH/src/terralib/utils/TeUtilsDefines.h \
	$$TERRALIBPATH/src/terralib/utils/TeColorUtils.h \
	$$TERRALIBPATH/src/terralib/utils/TeDatabaseUtils.h \
	$$TERRALIBPATH/src/terralib/utils/mtrand.h \
	$$TERRALIBPATH/src/terralib/utils/TeUpdateDBVersion.h \ 
	$$TERRALIBPATH/src/terralib/utils/TeWKBGeometryDecoder.h \
	$$TERRALIBPATH/src/terralib/utils/TeWKTGeometryDecoder.h
SOURCES += $$TERRALIBPATH/src/terralib/utils/TeColorUtils.cpp \
	$$TERRALIBPATH/src/terralib/utils/TeDatabaseUtils.cpp \
	$$TERRALIBPATH/src/terralib/utils/mtrand.cpp \
	$$TERRALIBPATH/src/terralib/utils/TeUpdateDBVersion.cpp \
	$$TERRALIBPATH/src/terralib/utils/TeWKBGeometryDecoder.cpp \
	$$TERRALIBPATH/src/terralib/utils/TeWKTGeometryDecoder.cpp
