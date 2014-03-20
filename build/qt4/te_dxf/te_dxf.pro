#-----------------------------------
#Project definitions
#-----------------------------------
TARGET = te_dxf
CONFIG	+= dll

include (../config.pri)

#Path definitions
#----------------------------------
INCLUDEPATH= $${TERRALIBPATH}/src/terralib/drivers/DXF
SRCPATH = $${INCLUDEPATH}

DEFINES += TEDXF_EXPORTS
#---------------
# Project files
#---------------
INCLUDEPATH	+= $${TERRALIBPATH}/src/terralib/kernel \
                $${TERRALIBPATH}/src/dxflib \
                $${TERRALIBPATH}/src/terralib/drivers/DXF
HEADERS	+= $${INCLUDEPATH}/tedxfdefines.h \
	$${INCLUDEPATH}/tedxflayer.h \
	$${INCLUDEPATH}/tedxfvisitor.h \
	$${INCLUDEPATH}/TeDXFDriver.h \
	$${INCLUDEPATH}/TeDXFDriverFactory.h
SOURCES	+= $${SRCPATH}/tedxflayer.cpp \
	$${SRCPATH}/tedxfvisitor.cpp \
	$${SRCPATH}/TeDXFDriver.cpp \
	$${SRCPATH}/TeDXFDriverFactory.cpp 
	
LIBS += -ldxf -lterralib

include (../install_cfg.pri)

DEPENDPATH += $${INCLUDEPATH}
