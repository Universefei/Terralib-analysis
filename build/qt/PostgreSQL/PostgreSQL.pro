CONFIG += dll
TARGET = te_postgresql

include (../config.pri)

DEFINES += TLPOSTGRESQL_AS_DLL

LIBS += -lterralib 

win32 {
	win32-g++:LIBS += -L$$TERRALIBPATH/dependencies/win32/PostgreSQL/lib \
			   		-lwsock32
	else:QMAKE_LIBDIR += $$$$TERRALIBPATH/dependencies/win32/PostgreSQL/lib
	LIBS += -llibpq
}
unix:LIBS += -L$${TERRALIBPATH}/dependencies/linux/PostgreSQL/lib \
		-lpq -lcrypt
		
linux-g++-64:LIBS += -L$${TERRALIBPATH}/dependencies/linux64/PostgreSQL/lib \
		-lpq -lcrypt

INCLUDEPATH += $$TERRALIBPATH/src/terralib/drivers/PostgreSQL  \
	      $$TERRALIBPATH/src/terralib/drivers/PostgreSQL/includepg

HEADERS += $$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePGInterface.h \
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePGUtils.h \
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePostgreSQL.h \
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePostGIS.h \ 
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePostgreSQLDefines.h \  
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePostgreSQLConnection.h

SOURCES += $$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePGInterface.cpp \
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePGUtils.cpp \
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePostgreSQL.cpp \
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePostGIS.cpp \
	$$TERRALIBPATH/src/terralib/drivers/PostgreSQL/TePostgreSQLConnection.cpp
	