#-----------------------------------
#Project definitions
#-----------------------------------
TARGET = te_dxf
CONFIG	+= dll

include (../config.pri)

#Path definitions
#----------------------------------
INCPATH= $$TERRALIBPATH/src/terralib/drivers/DXF
SRCPATH = $$INCPATH

DEFINES += TEDXF_EXPORTS
#---------------
# Project files
#---------------
INCLUDEPATH	+= $$TERRALIBPATH/src/terralib/kernel \
		$$TERRALIBPATH/src/dxflib \
		$$TERRALIBPATH/src/terralib/drivers/DXF
HEADERS	+= $$INCPATH/tedxfdefines.h \
	$$INCPATH/tedxflayer.h \
	$$INCPATH/tedxfvisitor.h \
	$$INCPATH/TeDXFDriver.h \
	$$INCPATH/TeDXFDriverFactory.h
SOURCES	+= $$SRCPATH/tedxflayer.cpp \
	$$SRCPATH/tedxfvisitor.cpp \
	$$SRCPATH/TeDXFDriver.cpp \
	$$SRCPATH/TeDXFDriverFactory.cpp 
	
win32 {
	!win32-g++ {
		debug:QMAKE_LIBDIR += $$TERRALIBPATH/Debug/dxflib \
				$$TERRALIBPATH/Debug/terralib
		release:QMAKE_LIBDIR += $$TERRALIBPATH/Release/dxflib \
				$$TERRALIBPATH/Release/terralib
	}
}
LIBS += -ldxf -lterralib