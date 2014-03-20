TARGET = terraViewCore

CONFIG += dll
include (../config.pri)
CONFIG += qt

DEFINES += TVIEW_AS_DLL

TRANSLATIONS =  \
	$$TERRALIBPATH/terraView/terraView_pt.ts \
	$$TERRALIBPATH/terraView/terraView_es.ts

isEmpty(TEDBDRIVERS) {
    TEDBDRIVERS = ALLDRIVERS
}

DLGPATH = $$TERRALIBPATH/terraView/ui
IMGPATH = $$DLGPATH/images

win32 {
     DEFINES += IBPP_WINDOWS \
		UNICODE
     
     win32-g++ {
		 debug {
			LIBS += -L$$TERRALIBPATH/dependencies/win32/DSDK/Debug/lib \
					-L$$TERRALIBPATH/dependencies/win32/DSDK/3rd-party/lib/Debug
		 }
		 release {
			LIBS += -L$$TERRALIBPATH/dependencies/win32/DSDK/Release/lib \
					-L$$TERRALIBPATH/dependencies/win32/DSDK/3rd-party/lib/Release
		 }
     }
     else {
	
		INCLUDEPATH += $$TERRALIBPATH/src/terralib/drivers/ado

		TEOUT = $$TERRALIBPATH/$$BUILDLOC
		QMAKE_LIBDIR += $$TEOUT/qwt \
			   $$TEOUT/terralib \
			   $$TEOUT/stat \
			   $$TEOUT/tiff \
			   $$TEOUT/shapelib \
			   $$TEOUT/libjpeg \
			   $$TEOUT/libspl \
			   $$TEOUT/terralibpdi \
			   $$TEOUT/te_utils \
			   $$TEOUT/te_functions \
			   $$TEOUT/te_qwt \
			   $$TEOUT/te_shapelib \
			   $$TEOUT/te_apputils \
			   $$TEOUT/stat \
			   $$TEOUT/te_ado
		debug:QMAKE_LIBDIR += $$TERRALIBPATH/dependencies/win32/DSDK/Debug/lib \
				   $$TERRALIBPATH/dependencies/win32/DSDK/3rd-party/lib/Debug
		release:QMAKE_LIBDIR += $$TERRALIBPATH/dependencies/win32/DSDK/Release/lib \
				   $$TERRALIBPATH/dependencies/win32/DSDK/3rd-party/lib/Release
	
	        # Database drivers
		contains(TEDBDRIVERS, ALLDRIVERS) {
		    QMAKE_LIBDIR += $$TEOUT/te_mysql \
			            $$TEOUT/te_postgresql \
				    $$TEOUT/te_oracle \
				    $$TEOUT/te_firebird
	    }
		else {
		}
		
		LIBS += -llibjpeg -ltiff -lte_ado
    }
}
unix {
     DEFINES += IBPP_LINUX
	 LIBS += -lterralibtiff -ljpeg 
}

UI_DIR = $$TERRALIBPATH/ui/terraView
MOC_DIR = $$TERRALIBPATH/moc/terraView

LIBS += -lterralib -lterralibpdi -lstat -lshapelib \
	-lqwt -lte_functions -lte_utils -lte_qwt -lte_shapelib -lte_apputils -lte_spl

win32:LIBS += -lopengl32 -lglu32 -llti_dsdk_dll -lgdalogr

contains(TEDBDRIVERS, ALLDRIVERS):LIBS += -lte_mysql -lte_postgresql -lte_oracle -lte_firebird
else {
}
	
INCLUDEPATH += $$TERRALIBPATH/terraView \
	      $$TERRALIBPATH/src/qwt/include \
	      $$TERRALIBPATH/src/terralib/drivers/qwt \
	      $$TERRALIBPATH/src/terralib/drivers/shapelib \
	      $$TERRALIBPATH/src/terralib/drivers/spl \
	      $$TERRALIBPATH/src/terralib/kernel \
	      $$TERRALIBPATH/src/terralib/utils \
	      $$TERRALIBPATH/src/terralib/functions \
	      $$TERRALIBPATH/src/terralib/stat \
	      $$TERRALIBPATH/src/shapelib \
	      $$TERRALIBPATH/src/zlib \
	      $$TERRALIBPATH/src/libspl \
	      $$TERRALIBPATH/src/terralib/drivers/qt \
	      $$TERRALIBPATH/src/terralib/drivers/MySQL \
	      $$TERRALIBPATH/src/terralib/drivers/MySQL/include \ 
	      $$TERRALIBPATH/src/terralib/drivers/Oracle \
	      $$TERRALIBPATH/src/terralib/drivers/Oracle/OCI/include \
	      $$TERRALIBPATH/src/terralib/drivers/PostgreSQL \
	      $$TERRALIBPATH/src/terralib/drivers/PostgreSQL/includepg \
	      $$TERRALIBPATH/src/terralib/drivers/Firebird \
	      $$TERRALIBPATH/src/terralib/image_processing \
	      $$TERRALIBPATH/src/appUtils \
	      $$TERRALIBPATH/ui/appUtils 

HEADERS += $$TERRALIBPATH/terraView/TViewDefines.h \
	$$TERRALIBPATH/terraView/PluginSupportFunctions.h \
	$$TERRALIBPATH/terraView/terraView.h \
    $$TERRALIBPATH/terraView/TViewAbstractPlugin.h \
	$$TERRALIBPATH/src/terralib/drivers/qt/TeQtMethods.h \
	$$TERRALIBPATH/src/terralib/drivers/qt/TeQtTerraStat.h 
SOURCES += $$TERRALIBPATH/terraView/PluginSupportFunctions.cpp \
	$$TERRALIBPATH/terraView/terraView.cpp \
    $$TERRALIBPATH/terraView/TViewAbstractPlugin.cpp \
	$$TERRALIBPATH/src/terralib/drivers/qt/TeQtMethods.cpp \
	$$TERRALIBPATH/src/terralib/drivers/qt/TeQtTerraStat.cpp 
 
FORMS += $$DLGPATH/addColumn.ui \
	$$DLGPATH/addressChoiceWindow.ui \
	$$DLGPATH/addressLocatorWindow.ui \
	$$DLGPATH/addressNameSeparator.ui \
	$$DLGPATH/addTheme.ui \
	$$DLGPATH/addView.ui \
	$$DLGPATH/anaesp.ui \
	$$DLGPATH/bayesWindow.ui \
	$$DLGPATH/changeColData.ui \
	$$DLGPATH/changeColumnName.ui \
	$$DLGPATH/changeColumnType.ui \
	$$DLGPATH/changeMinMax.ui \
	$$DLGPATH/connectView.ui \
	$$DLGPATH/contrast.ui \
	$$DLGPATH/createBuffersWindow.ui \
	$$DLGPATH/countComputing.ui \
	$$DLGPATH/createCells.ui \
	$$DLGPATH/createLayerFromTheme.ui \
	$$DLGPATH/createProxMatrix.ui \
	$$DLGPATH/createTable.ui \
	$$DLGPATH/createTextRepresentation.ui \
	$$DLGPATH/createThemeFromTheme.ui \
	$$DLGPATH/database.ui \
	$$DLGPATH/databaseProp.ui \
	$$DLGPATH/display.ui \
	$$DLGPATH/displaySize.ui \
	$$DLGPATH/exportWindow.ui \
	$$DLGPATH/generateSamplePoints.ui \
	$$DLGPATH/geoOpAdd.ui \
	$$DLGPATH/geoOpAggregation.ui \
	$$DLGPATH/geoOpAssignDataLocationCollect.ui \
	$$DLGPATH/geoOpAssignDataLocationDistribute.ui \
	$$DLGPATH/geoOpOverlayDifference.ui \
	$$DLGPATH/geoOpOverlayIntersection.ui \
	$$DLGPATH/geoOpOverlayUnion.ui \
	$$DLGPATH/geographicalGrid.ui \
	$$DLGPATH/graphic.ui \
	$$DLGPATH/graphic3DWindow.ui \
	$$DLGPATH/graphicConfigurationWindow.ui \
	$$DLGPATH/graphicParams.ui \
	$$DLGPATH/histogramBuildForm.ui \
	$$DLGPATH/import.ui \
	$$DLGPATH/importRasterSimple.ui \
	$$DLGPATH/importTable.ui \
	$$DLGPATH/importTbl.ui \
	$$DLGPATH/importTablePoints.ui \
	$$DLGPATH/importTblPoints.ui \
	$$DLGPATH/insertPoint.ui \
	$$DLGPATH/insertText.ui \
	$$DLGPATH/insertSymbol.ui \
	$$DLGPATH/kernelRatioWindow.ui \
	$$DLGPATH/kernelWindow.ui \
	$$DLGPATH/layerProp.ui \
	$$DLGPATH/lbsDefinition.ui \
	$$DLGPATH/legendWindow.ui \
	$$DLGPATH/linkExtTable.ui \
	$$DLGPATH/localBayesWindow.ui \
	$$DLGPATH/pieBarChart.ui \
	$$DLGPATH/printFileResolution.ui \
	$$DLGPATH/projection.ui \
	$$DLGPATH/query.ui \
	$$DLGPATH/rasterColorCompositionWindow.ui \
	$$DLGPATH/rasterImportWizard.ui \
	$$DLGPATH/rasterSlicingWindow.ui \
	$$DLGPATH/rasterTransparency.ui \
	$$DLGPATH/removeTable.ui \
	$$DLGPATH/saveAndLoadColorBarWindow.ui \
	$$DLGPATH/saveAsTable.ui \
	$$DLGPATH/saveRaster.ui \
	$$DLGPATH/saveTableAsTxt.ui \
	$$DLGPATH/saveThemeToFileWindow.ui \
	$$DLGPATH/selThemeTables.ui \
	$$DLGPATH/selectProxMatrix.ui \
	$$DLGPATH/selectTextRepresentation.ui \
	$$DLGPATH/semivar.ui \
	$$DLGPATH/skaterWindow.ui \
	$$DLGPATH/spatializationWindow.ui \
	$$DLGPATH/spatialQuery.ui \
	$$DLGPATH/statistic.ui \
	$$DLGPATH/tableProp.ui \
	$$DLGPATH/telayer_metadata.ui \
	$$DLGPATH/terraViewBase.ui \
	$$DLGPATH/textRep.ui \
	$$DLGPATH/themeProp.ui \
	$$DLGPATH/themeVis.ui \
	$$DLGPATH/viewProp.ui \
	$$DLGPATH/visual.ui \
	$$DLGPATH/exportLayersThemes.ui \
	$$DLGPATH/dlgDropDatabase.ui
IMAGES += $$IMGPATH/addressDictionary.png \
	$$IMGPATH/addressLayer.png \
	$$IMGPATH/addressReferenceLayer.png \
	$$IMGPATH/addressTable.png \
	$$IMGPATH/and.bmp \
	$$IMGPATH/area.bmp \
	$$IMGPATH/buffer.bmp \
	$$IMGPATH/bufferNoUnion.bmp \
	$$IMGPATH/bufferUnion.bmp \
	$$IMGPATH/clearColor.bmp \
	$$IMGPATH/closeDB.bmp \
	$$IMGPATH/containLin.bmp \
	$$IMGPATH/containLinPol.bmp \
	$$IMGPATH/containLinPon.bmp \
	$$IMGPATH/containPol.bmp \
	$$IMGPATH/convexHull.bmp \
	$$IMGPATH/coverByPol.bmp \
	$$IMGPATH/coverLinPol.bmp \
	$$IMGPATH/coverPol.bmp \
	$$IMGPATH/crossLin.bmp \
	$$IMGPATH/crossLinPol.bmp \
	$$IMGPATH/database.bmp \
	$$IMGPATH/databasesTree.bmp \
	$$IMGPATH/disjointLin.bmp \
	$$IMGPATH/disjointLinPol.bmp \
	$$IMGPATH/disjointLinPon.bmp \
	$$IMGPATH/disjointPol.bmp \
	$$IMGPATH/disjointPonPol.bmp \
	$$IMGPATH/display.bmp \
	$$IMGPATH/distance.bmp \
	$$IMGPATH/distMeter.bmp \
	$$IMGPATH/draw.bmp \
	$$IMGPATH/edit.xpm \
	$$IMGPATH/equalLin.bmp \
	$$IMGPATH/equalPol.bmp \
	$$IMGPATH/graphic.bmp \
	$$IMGPATH/graphicConfig.bmp \
	$$IMGPATH/graphicCursor.bmp \
	$$IMGPATH/grid.bmp \
	$$IMGPATH/help.bmp \
	$$IMGPATH/import.bmp \
	$$IMGPATH/infoCursor.bmp \
	$$IMGPATH/infolayer.bmp \
	$$IMGPATH/insideAndOutside.png \
	$$IMGPATH/invertColor1.bmp \
	$$IMGPATH/invertSelection.bmp \
	$$IMGPATH/isContainLin.bmp \
	$$IMGPATH/isContainLinPol.bmp \
	$$IMGPATH/isContainPol.bmp \
	$$IMGPATH/isContainPonPol.bmp \
	$$IMGPATH/left.bmp \
	$$IMGPATH/length.bmp \
	$$IMGPATH/minus.bmp \
	$$IMGPATH/nearstNeigh.bmp \
	$$IMGPATH/nextDisplay.bmp \
	$$IMGPATH/nonEdit.xpm \
	$$IMGPATH/onlyInside.png \
	$$IMGPATH/onlyOutside.png \
	$$IMGPATH/openfile.bmp \
	$$IMGPATH/or.bmp \
	$$IMGPATH/overlapLin.bmp \
	$$IMGPATH/overlapPol.bmp \
	$$IMGPATH/panCursor.bmp \
	$$IMGPATH/pointer.bmp \
	$$IMGPATH/previousDisplay.bmp \
	$$IMGPATH/print.bmp \
	$$IMGPATH/promote.bmp \
	$$IMGPATH/reset.bmp \
	$$IMGPATH/right.bmp \
	$$IMGPATH/table.bmp \
	$$IMGPATH/terralib.bmp \
	$$IMGPATH/theme.bmp \
	$$IMGPATH/tileWindows.bmp \
	$$IMGPATH/touchLin.bmp \
	$$IMGPATH/touchLinPol.bmp \
	$$IMGPATH/touchLinPon.bmp \
	$$IMGPATH/touchPol.bmp \
	$$IMGPATH/touchPonPol.bmp \
	$$IMGPATH/unselect.bmp \
	$$IMGPATH/view.bmp \
	$$IMGPATH/viewsTree.bmp \
	$$IMGPATH/xor.bmp \
	$$IMGPATH/zoomCursor.bmp \
	$$IMGPATH/zoomIn.bmp \
	$$IMGPATH/tick_circle.png \
	$$IMGPATH/ticket_error.png \
	$$IMGPATH/drop_database.png \
	$$IMGPATH/zoomOut.bmp
