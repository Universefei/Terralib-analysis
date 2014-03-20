DEFINES += WIN32 _WINDOWS

isEmpty(DESTDIR) {
    win32-msvc2003:DESTDIR = $${TERRALIBPATH}/$${BUILDLOC}/win32-msvc2003
    win32-msvc2005:DESTDIR = $${TERRALIBPATH}/$${BUILDLOC}/win32-msvc2005
    win32-msvc2008:DESTDIR = $${TERRALIBPATH}/$${BUILDLOC}/win32-msvc2008
    win32-msvc2010:DESTDIR = $${TERRALIBPATH}/$${BUILDLOC}/win32-msvc2010
}

OBJECTS_DIR = $${DESTDIR}/obj/$${TARGET}

CONFIG -= incremental

QMAKE_LIBDIR += $${DESTDIR}

CONFIG(debug, debug|release){
	contains(CONFIG, dll) {
		 QMAKE_CFLAGS_MT_DLLDBG += -MDd -W4
		 QMAKE_CXXFLAGS_MT_DLLDBG += -MDd -Od -Fd$${OBJECTS_DIR}/ -ZI -Wp64 -Gm
	}
}
CONFIG(release, debug|release) {
	contains(CONFIG, dll) {
		 QMAKE_CFLAGS_MT_DLL += -O2
		 QMAKE_CXXFLAGS_MT_DLL += -O2 -Fd$${OBJECTS_DIR}/
	}
}
QMAKE_LFLAGS += /IMPLIB:$(OutDir)/$(ProjectName).lib
QMAKE_LFLAGS -= /NOLOGO 
QMAKE_LFLAGS += /PDB:$(OutDir)/$(ProjectName).pdb 
QMAKE_LFLAGS += /MACHINE:X86
QMAKE_CFLAGS += /GS -wd4100
QMAKE_CXXFLAGS += /GS -wd4100
QMAKE_CFLAGS_DEBUG += -RTC1 
QMAKE_CXXFLAGS_DEBUG += -RTC1 
