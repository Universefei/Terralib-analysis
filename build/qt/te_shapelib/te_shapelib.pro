CONFIG += dll
TARGET = te_shapelib

include (../config.pri)

DEFINES += TLSHP_AS_DLL

win32 {
     !win32-g++ {
	  debug:QMAKE_LIBDIR += $$TERRALIBPATH/Debug/shapelib
	  release:QMAKE_LIBDIR += $$TERRALIBPATH/Release/shapelib
     }
}

LIBS += -lshapelib -lterralib 

INCLUDEPATH += $$TERRALIBPATH/src/terralib/shapelib  \
	      $$TERRALIBPATH/src/shapelib

HEADERS += $$TERRALIBPATH/src/terralib/drivers/shapelib/TeDriverSHPDBF.h \
	$$TERRALIBPATH/src/terralib/drivers/shapelib/TeSHPDriverFactory.h \
	$$TERRALIBPATH/src/terralib/drivers/shapelib/TeSHPDefines.h 
SOURCES += $$TERRALIBPATH/src/terralib/drivers/shapelib/TeDriverSHPDBF.cpp \
	$$TERRALIBPATH/src/terralib/drivers/shapelib/TeSHPDriverFactory.cpp 