win32 {
  error( "To compile tests you must use the Visual Studio projects of each test" )
}

CONFIG = qt warn_on thread debug rtti exceptions

TEMPLATE = app

LANGUAGE = C++

DESTDIR = ../bin
OBJECTS_DIR = ../bin/tmp
MOC_DIR = ../bin/tmp
UI_DIR = ../bin/tmp

TERRALIB_PATH = ../../../

INCLUDEPATH = \
  $$TERRALIB_PATH/src/tiff \
  $$TERRALIB_PATH/src
    
VPATH = ../../src/

contains( CONFIG , release ) {
  DEFINES += NDEBUG
}

LIBS += -L$$TERRALIB_PATH/Debug/linux-g++
LIBS += -ljpeg
LIBS += -lz
LIBS += -lterralibtiff 
LIBS += -lshapelib
LIBS += -lterralib

# LIBS += -L$$TERRALIB_PATH/terralibx/DSDK -lltidsdk
# LIBS += -L$$TERRALIB_PATH/terralibx/DSDK/3rd-party/lib -lgdal   

