#Defining paths
#-----------------------------
isEmpty(TERRALIBPATH) {
  # Trying to get it from the user environment variable with the same name
  TERRALIBPATH = $$(TERRALIB_DIR)
  
  # The default - if none of previews attempts succeed
  isEmpty(TERRALIBPATH) {
	 TERRALIBPATH = ../../..
  } 
}
#-----------------------------
TEMPLATE = lib
VERSION = 4.0.0
CONFIG	+= warn_on \
		rtti \
		exceptions \
		thread 
CONFIG -= qt		
LANGUAGE	= C++
#------------------------------
# Configure to debug or release
#------------------------------
isEmpty(TE_PROJECT_TYPE) {
  # Trying to get it from the user environment variable with the same name
  TE_PROJECT_TYPE = $$(TE_PROJECT_TYPE)

  # The default - if none of previews attempts succeed
  isEmpty(TE_PROJECT_TYPE) {
    TE_PROJECT_TYPE=RELEASE
  }
}
contains(TE_PROJECT_TYPE , DEBUG) {
    CONFIG += debug
    CONFIG -= release
}
contains(TE_PROJECT_TYPE , RELEASE) {
    CONFIG += release
    CONFIG -= debug
}
#-----------------------------
#Dependencies definitions
#-----------------------------
BUILDLOC = Debug
debug {
	DEFINES = _DEBUG
	CONFIG += console
}
release {
	DEFINES = NDEBUG
	BUILDLOC = Release
}
unix:DESTDIR = $$TERRALIBPATH/$$BUILDLOC/linux-g++
OBJECTS_DIR = $$DESTDIR/obj/$$TARGET
LIBS += -L$$DESTDIR 
INCLUDEPATH += $$TERRALIBPATH/src/terralib/kernel
win32 {
	win32-g++:include(config_win32-g++.pri)
	else:include(config_win32-msvc.pri)
}