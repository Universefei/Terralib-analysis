CONFIG += dll
TARGET = te_apputils

include (../config.pri)

# Directory definitions
UTILSSRC = $$TERRALIBPATH/src/appUtils
QTDRVSRC = $$TERRALIBPATH/src/terralib/drivers/qt
debug:TEOUT = $$TERRALIBPATH/Debug
release:TEOUT = $$TERRALIBPATH/Release
# --------------------
CONFIG += qt
DEFINES += TLAPPUTILS_AS_DLL

MOC_DIR = $$TERRALIBPATH/moc/appUtils
UI_DIR = $$TERRALIBPATH/ui/appUtils

win32 {
	!win32-g++:QMAKE_LIBDIR += $$TEOUT/te_utils \
								$$TEOUT/stat
	LIBS += -lopengl32 -lglu32
	DEFINES	+= UNICODE
}
unix:LIBS += -lGL -lGLU 
LIBS += -lterralib -lte_utils -lstat

INCLUDEPATH += $$UTILSSRC \
			$$QTDRVSRC \
			$$TERRALIBPATH/src/terralib/utils \
			$$TERRALIBPATH/src/terralib/stat \
			$$TERRALIBPATH/terraView/ui

HEADERS += $$UTILSSRC/TeAppUtilsDefines.h \
		$$UTILSSRC/TeAppTheme.h \
		$$UTILSSRC/TeApplicationUtils.h \
		$$UTILSSRC/TePlotTheme.h \
		$$UTILSSRC/TePlotView.h \
		$$QTDRVSRC/TeQtAnimaThread.h \
		$$QTDRVSRC/TeQtCanvas.h \
		$$QTDRVSRC/TeQtChartItem.h \
		$$QTDRVSRC/TeQtCheckListItem.h \
		$$QTDRVSRC/TeQtColorBar.h \
		$$QTDRVSRC/TeQtDatabaseItem.h \
		$$QTDRVSRC/TeQtDatabasesListView.h \
		$$QTDRVSRC/TeQtFrame.h \
		$$QTDRVSRC/TeQtGLWidget.h \
		$$QTDRVSRC/TeQtDataSource.h \
		$$QTDRVSRC/TeQtGrid.h \
		$$QTDRVSRC/TeQtLayerItem.h \
		$$QTDRVSRC/TeQtLegendItem.h \
		$$QTDRVSRC/TeQtLegendSource.h \
		$$QTDRVSRC/TeQtShowMedia.h \
		$$QTDRVSRC/TeQtBigTable.h \
		$$QTDRVSRC/TeQtTable.h \
		$$QTDRVSRC/TeQtTextEdit.h \
		$$QTDRVSRC/TeQtThemeItem.h \
		$$QTDRVSRC/TeQtViewItem.h \
		$$QTDRVSRC/TeQtViewsListView.h \
		$$QTDRVSRC/TeWaitCursor.h \
		$$QTDRVSRC/TeGUIUtils.h \
		$$QTDRVSRC/TeQtProgress.h \
		$$QTDRVSRC/TeDecoderQtImage.h 
SOURCES += $$UTILSSRC/TeAppTheme.cpp \
		$$UTILSSRC/TeApplicationUtils.cpp \
		$$QTDRVSRC/TeQtAnimaThread.cpp \
		$$QTDRVSRC/TeQtCanvas.cpp \
		$$QTDRVSRC/TeQtChartItem.cpp \
		$$QTDRVSRC/TeQtCheckListItem.cpp \
		$$QTDRVSRC/TeQtColorBar.cpp \
		$$QTDRVSRC/TeQtDatabaseItem.cpp \
		$$QTDRVSRC/TeQtDatabasesListView.cpp \
		$$QTDRVSRC/TeQtFrame.cpp \
		$$QTDRVSRC/TeQtGLWidget.cpp \
		$$QTDRVSRC/TeQtGrid.cpp \
		$$QTDRVSRC/TeQtLayerItem.cpp \
		$$QTDRVSRC/TeQtLegendItem.cpp \
		$$QTDRVSRC/TeQtLegendSource.cpp \
		$$QTDRVSRC/TeQtShowMedia.cpp \
		$$QTDRVSRC/TeQtBigTable.cpp \
		$$QTDRVSRC/TeQtTable.cpp \
		$$QTDRVSRC/TeQtTextEdit.cpp \
		$$QTDRVSRC/TeQtThemeItem.cpp \
		$$QTDRVSRC/TeQtViewItem.cpp \
		$$QTDRVSRC/TeQtViewsListView.cpp \
		$$QTDRVSRC/TeWaitCursor.cpp \
		$$QTDRVSRC/TeGUIUtils.cpp \
		$$QTDRVSRC/TeDecoderQtImage.cpp 
FORMS += $$UTILSSRC/ui/animation.ui \
		$$UTILSSRC/ui/help.ui \
		$$UTILSSRC/ui/mediaDescription.ui \
		$$UTILSSRC/ui/urlWindow.ui