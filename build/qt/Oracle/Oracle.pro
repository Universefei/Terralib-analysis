CONFIG += dll
TARGET = te_oracle

include (../config.pri)

DEFINES += TLORACLE_AS_DLL

LIBS += -lterralib 

win32 {
     !win32-g++:QMAKE_LIBDIR += $$TERRALIBPATH/dependencies/win32/Oracle/lib
     else:LIBS += -L$$TERRALIBPATH/dependencies/win32/Oracle/lib

     LIBS += -loci
}

unix:LIBS += -L$${TERRALIBPATH}/dependencies/linux/Oracle/lib \
		-lclntsh -lnnz10
		
linux-g++-64:LIBS += -L$${TERRALIBPATH}/dependencies/linux64/Oracle/lib \
		-lclntsh -lnnz10

INCLUDEPATH += $$TERRALIBPATH/src/terralib/drivers/Oracle  \
	      $$TERRALIBPATH/src/terralib/drivers/Oracle/OCI/include

HEADERS += $$TERRALIBPATH/src/terralib/drivers/Oracle/TeOracleSpatial.h \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOCIOracle.h \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOCIConnect.h \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOCICursor.h \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOCISDO.h \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOracleDefines.h 

SOURCES += $$TERRALIBPATH/src/terralib/drivers/Oracle/TeOracleSpatial.cpp \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOCIOracle.cpp \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOCIConnect.cpp \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOCICursor.cpp \
	$$TERRALIBPATH/src/terralib/drivers/Oracle/TeOCISDO.cpp 

unix {
	create_link.commands = cd $${TERRALIBPATH}/dependencies/linux/Oracle/lib/ && ln -sf libclntsh.so.10.1 libclntsh.so
	QMAKE_EXTRA_UNIX_TARGETS += create_link
	PRE_TARGETDEPS += create_link
}