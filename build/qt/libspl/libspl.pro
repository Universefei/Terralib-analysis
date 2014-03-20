#win32 {
#  error( "To build LibSPL for Windows use the Visual Studio project file." )
#}

TARGET  = spl
include (../config.pri)

LIBSPLPATH = $$TERRALIBPATH/src/libspl

win32 {
	DEFINES += _LIB \
			SPL_MAKE_DLL
	win32-g++:DEFINES -= UNICODE
	win32-g++:DEFINES += __GNUC__	
}
unix:LIBS += -lpthread
INCLUDEPATH = $$LIBSPLPATH $$LIBSPLPATH/include
MOC_DIR = temp
UI_DIR = temp
HEADERS = \
  $$LIBSPLPATH/spl.h \
  $$LIBSPLPATH/include/spl_pluginargs.h \
  $$LIBSPLPATH/include/spl_pluginserver_linux.h \
  $$LIBSPLPATH/include/spl_pluginserver_solaris.h \
  $$LIBSPLPATH/include/spl_pluginserver_win32.h \
  $$LIBSPLPATH/include/spl_threadlock.h \
  $$LIBSPLPATH/include/spl_mutex.h \
  $$LIBSPLPATH/include/spl_plugininfo.h \
  $$LIBSPLPATH/include/spl_typedefs.h \
  $$LIBSPLPATH/include/spl_platform.h \
  $$LIBSPLPATH/include/spl_pluginserver.h 
SOURCES = \
  $$LIBSPLPATH/source/spl_pluginserver.cpp \
  $$LIBSPLPATH/source/spl_pluginserver_linux.cpp \
  $$LIBSPLPATH/source/spl_pluginserver_solaris.cpp \
  $$LIBSPLPATH/source/spl_pluginserver_win32.cpp \
  $$LIBSPLPATH/source/spl_pluginargs.cpp \
  $$LIBSPLPATH/source/spl_mutex.cpp \
  $$LIBSPLPATH/source/spl_threadlock.cpp \
  $$LIBSPLPATH/source/spl_plugininfo.cpp
