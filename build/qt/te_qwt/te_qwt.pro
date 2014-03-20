TARGET = te_qwt

include (../config.pri)

CONFIG += dll qt

DEFINES += TLQWT_AS_DLL

win32 {
     !win32-g++ {
	  debug:QMAKE_LIBDIR += $$TERRALIBPATH/Debug/qwt
	  release:QMAKE_LIBDIR += $$TERRALIBPATH/Release/qwt
     }
}

UI_DIR = $$TERRALIBPATH/ui/qwt
MOC_DIR = $$TERRALIBPATH/moc/qwt

LIBS += -lqwt -lterralib 

INCLUDEPATH += $$TERRALIBPATH/src/terralib/drivers/qwt  \
	      $$TERRALIBPATH/src/qwt/include

HEADERS += $$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtPlot.h \
	$$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtPlotCurve.h \
	$$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtPlotPicker.h \
	$$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtPlotZoomer.h \
	$$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtDefines.h 
SOURCES += $$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtPlot.cpp \
	$$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtPlotCurve.cpp \
	$$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtPlotPicker.cpp \
	$$TERRALIBPATH/src/terralib/drivers/qwt/TeQwtPlotZoomer.cpp