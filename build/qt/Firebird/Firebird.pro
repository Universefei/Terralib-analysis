CONFIG += dll
TARGET = te_firebird
TEMPLATE = lib
include (../config.pri)

DEFINES += TLFIREBIRD_AS_DLL

LIBS += -lterralib 
unix:LIBS += -L$${TERRALIBPATH}/dependencies/linux/Firebird/lib \
		-lfbembed
		
linux-g++-64:LIBS += -L$${TERRALIBPATH}/dependencies/linux64/Firebird/lib \
		-lfbembed 

win32:DEFINES += IBPP_WINDOWS
unix:DEFINES += IBPP_LINUX

INCLUDEPATH += $$TERRALIBPATH/src/terralib/drivers/Firebird  

HEADERS += $$TERRALIBPATH/src/terralib/drivers/Firebird/ibpp/core/_ibpp.h \
	  $$TERRALIBPATH/src/terralib/drivers/Firebird/ibpp/core/ibase.h \
	  $$TERRALIBPATH/src/terralib/drivers/Firebird/ibpp/core/iberror.h \
	  $$TERRALIBPATH/src/terralib/drivers/Firebird/ibpp/core/ibpp.h \
	  $$TERRALIBPATH/src/terralib/drivers/Firebird/TeFirebird.h \
	  $$TERRALIBPATH/src/terralib/drivers/Firebird/TeFirebirdDefines.h 

SOURCES += $$TERRALIBPATH/src/terralib/drivers/Firebird/TeFirebird.cpp \
	$$TERRALIBPATH/src/terralib/drivers/Firebird/ibpp/core/all_in_one.cpp 
