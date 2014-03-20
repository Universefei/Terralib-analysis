#Path definitions
#----------------------------------
INCPATH= ../../../src/terralib/drivers/gdal
SRCPATH = $$INCPATH
#-----------------------------------
#Project definitions
#-----------------------------------
TEMPLATE = lib
TARGET = te_gdal
CONFIG	+= warn_on \
		rtti \
		exceptions \
		thread \
		dll
CONFIG -= qt
LANGUAGE	= C++
#------------------------------
# Configure to debug or release
#------------------------------
isEmpty(TE_PROJECT_TYPE) {
    TE_PROJECT_TYPE=RELEASE
}
contains(TE_PROJECT_TYPE , DEBUG) {
    CONFIG -= release
    CONFIG += debug
}
contains(TE_PROJECT_TYPE , RELEASE) {
    CONFIG -= debug
    CONFIG += release
}

include (../config.pri)

OBJECTS_DIR = $$OBJECTS_DIR/te_gdal

DEFINES += TEGDAL_EXPORTS
#---------------
# Project files
#---------------
INCLUDEPATH	+= ../../../src/terralib/kernel \
			   ../../../src/terralib/utils \
			   ../../../src/terralib/drivers/gdal \
			   ../../../../thirdparty/gdal-1.7.2/gcore \
			   ../../../../thirdparty/gdal-1.7.2/port \
			   ../../../../thirdparty/gdal-1.7.2/ogr \
			   ../../../../thirdparty/gdal-1.7.2/ogr/ogrsf_frmts
			   
HEADERS	+= $$INCPATH/TeOGRDriver.h \
		   $$INCPATH/TeOGRDriverFactory.h \
		   $$INCPATH/TeOGRUtils.h \
		   $$INCPATH/TeGDALDefines.h \
		   $$INCPATH/TeGDALUtils.h \
		   $$INCPATH/TeGDALDecoder.h
		   
SOURCES	+= $$SRCPATH/TeOGRDriver.cpp \
		   $$SRCPATH/TeOGRDriverFactory.cpp \
		   $$INCPATH/TeOGRUtils.cpp \
		   $$INCPATH/TeGDALUtils.cpp \
		   $$INCPATH/TeGDALDecoder.cpp
unix {
	debug:LIBS += -L../../Debug 
	release:LIBS += -L../../Release 
}
win32 {
	!win32-g++ {
		debug:QMAKE_LIBDIR += ../../../../thirdparty/gdal-1.7.2 \
							  ../../../Debug/terralib \
							  ../../../Debug/te_utils
		release:QMAKE_LIBDIR += ../../../../thirdparty/gdal-1.7.2 \
								../../../Release/terralib \
								../../../Release/te_utils
	}
	else {
		debug:LIBS += -L../../Debug
		release:LIBS += -L../../Release
	}			
}
LIBS += -lgdal_i -lterralib -lte_utils