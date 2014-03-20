CONFIG += dll
TARGET = te_mysql

include (../config.pri)

DEFINES += TLMYSQL_AS_DLL

LIBS += -lterralib 

win32 {
    !win32-g++ {
	    QMAKE_LIBDIR += $$TERRALIBPATH/dependencies/win32/MySQL/lib/ms
	    LIBS += -llibmysql
    }
    else:LIBS += -L$$TERRALIBPATH/dependencies/win32/MySQL/lib/mingw -lmysql_mingw
}

unix:LIBS += -L$${TERRALIBPATH}/dependencies/linux/MySQL/lib \
		-lmysqlclient
		
linux-g++-64:LIBS += -L$${TERRALIBPATH}/dependencies/linux64/MySQL/lib \
		-lmysqlclient 

INCLUDEPATH += $$TERRALIBPATH/src/terralib/drivers/MySQL  \
	      $$TERRALIBPATH/src/mysql

HEADERS += $$TERRALIBPATH/src/terralib/drivers/MySQL/TeMySQL.h \
	$$TERRALIBPATH/src/terralib/drivers/MySQL/TeMySQLDefines.h 

SOURCES += $$TERRALIBPATH/src/terralib/drivers/MySQL/TeMySQL.cpp