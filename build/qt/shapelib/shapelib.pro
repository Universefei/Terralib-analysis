TARGET  = shapelib
CONFIG += dll
win32:DEFINES += _USRDLL \
    SHAPELIB_DLLEXPORT
include (../config.pri)

SHPPATH = $$TERRALIBPATH/src/shapelib

SOURCES = $$SHPPATH/dbfopen.c \
    	$$SHPPATH/shpopen.c
HEADERS = $$SHPPATH/shapefil.h



