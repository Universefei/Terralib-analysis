#Defining paths
#-----------------------------
isEmpty(TERRALIBPATH) {
  # Trying to get it from the user environment variable with the same name
  TERRALIBPATH = $$(TERRALIB_DIR)
  
  # The default - if none of previews attempts succeed
  isEmpty(TERRALIBPATH):TERRALIBPATH = ../../..
}

isEmpty(TEBUILDPATH):TEBUILDPATH = $${TERRALIBPATH}/build/qt4

#-----------------------------
TEMPLATE = lib
unix:VERSION = 4.1.0
CONFIG	+= warn_on \
        rtti \
        exceptions \
        thread \
        debug_and_release
CONFIG -= qt
LANGUAGE	= C++
#-----------------------------
#Dependencies definitions
#-----------------------------
CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    BUILDLOC = Debug
}
CONFIG(release, debug|release) {
    DEFINES += NDEBUG
    BUILDLOC = Release
}
DEFINES -= UNICODE

isEmpty(DESTDIR):unix:DESTDIR = $${TERRALIBPATH}/$${BUILDLOC}/linux-g++

unix {
    LIBS += -L$${DESTDIR}
    OBJECTS_DIR = $${DESTDIR}/obj/$${TARGET}
}

INCLUDEPATH += $${TERRALIBPATH}/src/terralib/kernel

win32 {
    win32-g++:include($${TEBUILDPATH}/config_win32-g++.pri)
    else:include($${TEBUILDPATH}/config_win32-msvc.pri)
}
