CONFIG += shared
TARGET = te_mysql

include (../config.pri)
# Path definitions
# ----------------------------------------------------------
MSQLSRCPATH = $${TERRALIBPATH}/src/terralib/drivers/MySQL
MSQLDEPPATH = $${TERRALIBPATH}/dependencies
unix:MSQLDEPPATH = $${MSQLDEPPATH}/linux
win32 {
    contains(QMAKE_HOST.arch, x86_64):MSQLDEPPATH = $${MSQLDEPPATH}/win64
    else:MSQLDEPPATH = $${MSQLDEPPATH}/win32
}
MSQLDEPPATH = $${MSQLDEPPATH}/MySQL
# ----------------------------------------------------------

DEFINES += TLMYSQL_AS_DLL

LIBS += -lterralib 

win32 {
    win32-g++:LIBS += -L$${MSQLDEPPATH}/lib/mingw -lmysql
    else:QMAKE_LIBDIR += $${MSQLDEPPATH}/lib/ms
#    LIBS += -llibmysql
#    }
#    else:LIBS += -L$${MSQLDEPPATH}/lib/mingw -lmysql_mingw
}

unix:LIBS += -L$${MSQLDEPPATH}/lib \
		-lmysqlclient

INCLUDEPATH += $${TERRALIBPATH}/src/mysql

HEADERS += $${MSQLSRCPATH}/TeMySQL.h \
        $${MSQLSRCPATH}/TeMySQLDefines.h

SOURCES += $${MSQLSRCPATH}/TeMySQL.cpp

include (../install_cfg.pri)

CONFIG(copy_dir_files) {
    msql.path = $${DEPLOY_DIR}/include/MySQL/mysql
    msql.files = $${TERRALIBPATH}/src/mysql/mysql/*.h

    include.path = $${DEPLOY_DIR}/include/MySQL
    include.files = $${MSQLSRCPATH}/*.h

    INSTALLS += msql include
}
win32 {
    bin.files = $${MSQLDEPPATH}/bin/*.dll
    bin.path = $${DEPLOY_DIR}/bin
    INSTALLS += bin
}

DEPENDPATH += $${INCLUDEPATH}
