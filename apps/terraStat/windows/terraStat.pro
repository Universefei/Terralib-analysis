TEMPLATE = app
TARGET = terraStat
CONFIG += qt warn_on thread debug
LANGUAGE = C++

win32: DEFINES	+= /MTd /GR /GX -UUNICODE -U_MBCS

MOC_DIR = .moc

win32: UI_DIR = .ui

unix {
    UI_DIR = ui
    OBJECTS_DIR = .obj
}

TRANSLATIONS =  \
	../terraStat_pt.ts \
	../terraStat_sp.ts

SOURCES	+= \
	../main.cpp \
	../terraStat.cpp \
	../../../src/terralib/functions/TeAddressLocator.cpp \
	../../../src/terralib/application/TeSemivarModelFactory.cpp \
	../../../src/terralib/application/TeMSVFactory.cpp \
	../../../src/terralib/application/TeApplicationUtils.cpp \
	../../../src/terralib/functions/TeCellAlgorithms.cpp \
	../../../src/terralib/application/TeColorUtils.cpp \
	../../../src/terralib/functions/TeCoordAlgorithms.cpp \
	../../../src/terralib/application/TeDatabaseUtils.cpp \
	../../../src/terralib/application/qt/TeDecoderQtImage.cpp \
	../../../src/terralib/functions/TeExportMIF.cpp \
	../../../src/terralib/functions/TeExportSHP.cpp \
	../../../src/terralib/functions/TeExportSPR.cpp \
	../../../src/terralib/functions/TeGeoProcessingFunctions.cpp \
	../../../src/terralib/application/TeGUIUtils.cpp \
	../../../src/terralib/functions/TeImportBNA.cpp \
	../../../src/terralib/functions/TeImportDBF.cpp \
	../../../src/terralib/functions/TeImportGeo.cpp \
	../../../src/terralib/functions/TeImportMIF.cpp \
	../../../src/terralib/functions/TeImportSHP.cpp \
	../../../src/terralib/functions/TeLayerFunctions.cpp \
	../../../src/terralib/functions/TeMIFProjection.cpp \
	../../../src/terralib/drivers/MySQL/TeMySQL.cpp \
	../../../src/terralib/application/qt/TeQtAnimaThread.cpp \
	../../../src/terralib/application/qt/TeQtBasicCanvas.cpp \
	../../../src/terralib/application/qt/TeQtCanvas.cpp \
	../../../src/terralib/application/qt/TeQtChartItem.cpp \
	../../../src/terralib/application/qt/TeQtCheckListItem.cpp \
	../../../src/terralib/application/qt/TeQtDatabaseItem.cpp \
	../../../src/terralib/application/qt/TeQtDatabasesListView.cpp \
	../../../src/terralib/application/qt/TeQtGraph.cpp \
	../../../src/terralib/application/qt/TeQtGrid.cpp \
	../../../src/terralib/application/qt/TeQtLayerItem.cpp \
	../../../src/terralib/application/qt/TeQtLegendItem.cpp \
	../../../src/terralib/application/qt/TeQtMethods.cpp \
	../../../src/terralib/application/qt/TeQtShowMedia.cpp \
	../../../src/terralib/application/qt/TeQtTerraStat.cpp \
	../../../src/terralib/application/qt/TeQtTextEdit.cpp \
	../../../src/terralib/application/qt/TeQtThemeItem.cpp \
	../../../src/terralib/application/qt/TeQtViewItem.cpp \
	../../../src/terralib/application/qt/TeQtViewsListView.cpp \
	../../../src/terralib/functions/TeSimilarity.cpp \
	../../../src/terralib/functions/TeSPRFile.cpp \
	../../../src/terralib/application/TeThemeApplication.cpp \
	../../../src/terralib/application/TeUpdateDBVersion.cpp \
	../../../src/terralib/application/qt/TeWaitCursor.cpp

HEADERS	+= \
	../../../src/terralib/application/qt/resultVisual.h \
	../terraStat.h \
	../../../src/terralib/functions/TeAddressLocator.h \
	../../../src/terralib/application/TeSemivarModelFactory.h \
	../../../src/terralib/application/TeMSVFactory.h \
	../../../src/terralib/application/TeApplicationUtils.h \
	../../../src/terralib/functions/TeCellAlgorithms.h \
	../../../src/terralib/application/TeColorUtils.h \
	../../../src/terralib/functions/TeCoordAlgorithms.h \
	../../../src/terralib/application/TeDatabaseUtils.h \
	../../../src/terralib/application/qt/TeDecoderQtImage.h \
	../../../src/terralib/functions/TeGeoProcessingFunctions.h \
	../../../src/terralib/functions/TeImportExport.h \
	../../../src/terralib/functions/TeLayerFunctions.h \
	../../../src/terralib/functions/TeMIFProjection.h \
	../../../src/terralib/drivers/MySQL/TeMySQL.h \
	../../../src/terralib/application/TePlotTheme.h \
	../../../src/terralib/application/qt/TeQtAnimaThread.h \
	../../../src/terralib/application/qt/TeQtBasicCanvas.h \
	../../../src/terralib/application/qt/TeQtCanvas.h \
	../../../src/terralib/application/qt/TeQtChartItem.h \
	../../../src/terralib/application/qt/TeQtCheckListItem.h \
	../../../src/terralib/application/qt/TeQtDatabaseItem.h \
	../../../src/terralib/application/qt/TeQtDatabasesListView.h \
	../../../src/terralib/application/qt/TeQtGraph.h \
	../../../src/terralib/application/qt/TeQtGrid.h \
	../../../src/terralib/application/qt/TeQtLayerItem.h \
	../../../src/terralib/application/qt/TeQtLegendItem.h \
	../../../src/terralib/application/qt/TeQtMethods.h \
	../../../src/terralib/application/qt/TeQtProgress.h \
	../../../src/terralib/application/qt/TeQtShowMedia.h \
	../../../src/terralib/application/qt/TeQtTerraStat.h \
	../../../src/terralib/application/qt/TeQtTextEdit.h \
	../../../src/terralib/application/qt/TeQtThemeItem.h \
	../../../src/terralib/application/qt/TeQtViewItem.h \
	../../../src/terralib/application/qt/TeQtViewsListView.h \
	../../../src/terralib/functions/TeSPRFile.h \
	../../../src/terralib/application/TeThemeApplication.h \
	../../../src/terralib/application/TeUpdateDBVersion.h \
	../../../src/terralib/application/qt/TeWaitCursor.h \
	../../../src/zlib/zlib.h

win32 {
	SOURCES	+= \
		../../../src/terralib/drivers/OracleSpatial/connect.cpp \
		../../../src/terralib/drivers/OracleSpatial/cursor.cpp \
		../../../src/terralib/drivers/OracleSpatial/sdo_oci.cpp \
		../../../terralibw/ado/TeAdoDB.cpp \
		../../../terralibw/ado/TeAdoOracle.cpp \
		../../../terralibw/ado/TeAdoSqlServer.cpp \
		../../../src/terralib/drivers/OracleSpatial/TeOracleSpatial.cpp \
		../../../src/terralib/drivers/PostgreSQL/TePGInterface.cpp \
		../../../src/terralib/drivers/PostgreSQL/TePostgreSQL.cpp

	HEADERS	+= \
		../../../src/terralib/drivers/OracleSpatial/connect.h \
		../../../src/terralib/drivers/OracleSpatial/cursor.h \
		../../../src/terralib/drivers/OracleSpatial/ociap.h \
		../../../src/terralib/drivers/OracleSpatial/sdo_oci.h \
		../../../terralibw/ado/TeAdoDB.h \
		../../../terralibw/ado/TeAdoOracle.h \
		../../../terralibw/ado/TeAdoSqlServer.h \
		../../../src/terralib/drivers/OracleSpatial/TeOracleSpatial.h \
		../../../src/terralib/drivers/PostgreSQL/TePGInterface.h \
		../../../src/terralib/drivers/PostgreSQL/TePostgreSQL.h
}

win32 {
	INCLUDEPATH +=	\
		../../../src/STLport \
		. \
		.. \
		../../../src/terralib/kernel \
		../../../src/terralib/application \
		../../../src/terralib/functions \
		../../../src/terralib/stat \
		../../../src/shapelib \
		../../../src/zlib \
		../../../src/freetype/include \
		../../../src/terralib/application/qt \
		../../../terralibw/ado \
		../../../src/terralib/drivers/MySQL \
		../../../src/terralib/drivers/MySQL/include \
		../../../src/terralib/drivers/OracleSpatial \
		../../../src/terralib/drivers/OracleSpatial/OCI/include \
		../../../src/terralib/drivers/PostgreSQL \
		../../../src/terralib/drivers/PostgreSQL/includepg \
		../../../src/terralib/drivers/PostgreSQL/interfacespg/libpq \
		$(QTDIR)/include
}

unix {
	INCLUDEPATH += \
		. \
		.. \
		../../../src/terralib/kernel \
		../../../src/terralib/application \
		../../../src/terralib/functions \
		../../../src/terralib/stat \
		../../../src/shapelib \
		../../../src/zlib \
		../../../src/freetype/include \
		../../../src/terralib/application/qt \
		../../../src/terralib/drivers/MySQL \
		../../../src/terralib/drivers/MySQL/include \
		../../../src/terralib/drivers/OracleSpatial \
		../../../src/terralib/drivers/OracleSpatial/OCI/include \
		../../../src/terralib/drivers/PostgreSQL \
		../../../src/terralib/drivers/PostgreSQL/includepg \
		../../../src/terralib/drivers/PostgreSQL/interfacespg/libpq
}

win32 {
	LIBS = \
		../../../terralibw/freetype/Debug/freetype.lib \
		../../../terralibw/shapelib/Debug/shapelib.lib \
		../../../terralibw/stat/Debug/stat.lib \
		../../../terralibw/terralib/Debug/terralib.lib \
		../../../terralibw/tiff/Debug/tiff.lib \
		../../../terralibw/jpeg/ijl15.lib \
		../../../terralibw/OracleSpatial/Oci/Oci.lib \
		../../../terralibw/MySQL/libMySQL.lib \
		../../../terralibw/PostgreSQL/libpqdll.lib
} 

unix {
	LIBS += \
		-L../../../terralibx/shapelib \
		-L../../../terralibx/stat \
		-L../../../terralibx/terralib \
		-L../../../terralibx/tiff \
        -L/usr/local/mysql/lib -L/usr/lib/mysql -L../../../terralibx/MySQL \
		-L. \
        -lshapelib -lstat -lterralib -ltiff -lfreetype -lmysqlclient
}

FORMS = \
	../../ui/addColumn.ui \
	../../ui/addressLocatorWindow.ui \
	../../ui/addTheme.ui \
	../../ui/addView.ui \
	../../ui/adjustAxis.ui \
	../../ui/anaesp.ui \
	../../ui/animation.ui \
	../../ui/bayesWindow.ui \
	../../ui/changeColData.ui \
	../../ui/connectView.ui \
	../../ui/contrast.ui \
	../../ui/countComputing.ui \
	../../ui/createCells.ui \
	../../ui/createLayerFromTheme.ui \
	../../ui/createTable.ui \
	../../ui/database.ui \
	../../ui/databaseProp.ui \
	../../ui/display.ui \
	../../ui/exportWindow.ui \
	../../ui/fillAttrCell.ui \
	../../ui/geoOpAdd.ui \
	../../ui/geoOpAggregation.ui \
	../../ui/geoOpOverlayIntersection.ui \
	../../ui/geoOpOverlayUnion.ui \
	../../ui/geoOpSpatialJoin.ui \
	../../ui/graphic.ui \
	../../ui/graphicParams.ui \
	../../ui/help.ui \
	../../ui/import.ui \
	../../ui/importRaster.ui \
	../../ui/importRasterSimple.ui \
	../../ui/importTable.ui \
	../../ui/importTablePoints.ui \
	../../ui/insertPoint.ui \
	../../ui/insertText.ui \
	../../ui/insertSymbol.ui \
	../../ui/kernelWindow.ui \
	../../ui/layerProp.ui \
	../../ui/lbsDefinition.ui \
	../../ui/legend.ui \
	../../ui/linkExtTable.ui \
	../../ui/mediaDescription.ui \
	../../ui/projection.ui \
	../../ui/query.ui \
	../../ui/rasterSlicingWindow.ui \
	../../ui/removeTable.ui \
	../../ui/saveAsTable.ui \
	../../ui/saveRaster.ui \
	../../ui/selThemeTables.ui \
	../../ui/skaterGraphWindow.ui \
	../../ui/skaterWindow.ui \
	../../ui/semivar.ui \
	../../ui/semivarGraph.ui \
	../../ui/spatializationWindow.ui \
	../../ui/spatialQuery.ui \
	../../ui/statistic.ui \
	../../ui/tableProp.ui \
	../../ui/terraViewBase.ui \
	../../ui/textRep.ui \
	../../ui/themeProp.ui \
	../../ui/themeVis.ui \
	../../ui/urlWindow.ui \
	../../ui/viewProp.ui \
	../../ui/visual.ui

IMAGES	= \
	../../ui/images/and.bmp \
	../../ui/images/area.bmp \
	../../ui/images/buffer.bmp \
	../../ui/images/clearColor.bmp \
	../../ui/images/closeDB.bmp \
	../../ui/images/containLin.bmp \
	../../ui/images/containLinPol.bmp \
	../../ui/images/containLinPon.bmp \
	../../ui/images/containPol.bmp \
	../../ui/images/convexHull.bmp \
	../../ui/images/coverByPol.bmp \
	../../ui/images/coverLinPol.bmp \
	../../ui/images/coverPol.bmp \
	../../ui/images/crossLin.bmp \
	../../ui/images/crossLinPol.bmp \
	../../ui/images/database.bmp \
	../../ui/images/databasesTree.bmp \
	../../ui/images/disjointLin.bmp \
	../../ui/images/disjointLinPol.bmp \
	../../ui/images/disjointLinPon.bmp \
	../../ui/images/disjointPol.bmp \
	../../ui/images/disjointPonPol.bmp \
	../../ui/images/display.bmp \
	../../ui/images/distance.bmp \
	../../ui/images/distMeter.bmp \
	../../ui/images/draw.bmp \
	../../ui/images/edit.xpm \
	../../ui/images/equalLin.bmp \
	../../ui/images/equalPol.bmp \
	../../ui/images/graphic.bmp \
	../../ui/images/graphicCursor.bmp \
	../../ui/images/grid.bmp \
	../../ui/images/import.bmp \
	../../ui/images/infoCursor.bmp \
	../../ui/images/infolayer.bmp \
	../../ui/images/invertColor1.bmp \
	../../ui/images/invertSelection.bmp \
	../../ui/images/isContainLin.bmp \
	../../ui/images/isContainLinPol.bmp \
	../../ui/images/isContainPol.bmp \
	../../ui/images/isContainPonPol.bmp \
	../../ui/images/left.bmp \
	../../ui/images/length.bmp \
	../../ui/images/minus.bmp \
	../../ui/images/nearstNeigh.bmp \
	../../ui/images/nextDisplay.bmp \
	../../ui/images/nonEdit.xpm \
	../../ui/images/openfile.bmp \
	../../ui/images/or.bmp \
	../../ui/images/overlapLin.bmp \
	../../ui/images/overlapPol.bmp \
	../../ui/images/panCursor.bmp \
	../../ui/images/pointer.bmp \
	../../ui/images/previousDisplay.bmp \
	../../ui/images/reset.bmp \
	../../ui/images/right.bmp \
	../../ui/images/table.bmp \
	../../ui/images/terralib.bmp \
	../../ui/images/theme.bmp \
	../../ui/images/tileWindows.bmp \
	../../ui/images/touchLin.bmp \
	../../ui/images/touchLinPol.bmp \
	../../ui/images/touchLinPon.bmp \
	../../ui/images/touchPol.bmp \
	../../ui/images/touchPonPol.bmp \
	../../ui/images/unselect.bmp \
	../../ui/images/view.bmp \
	../../ui/images/viewsTree.bmp \
	../../ui/images/xor.bmp \
	../../ui/images/zoomCursor.bmp \
	../../ui/images/zoomIn.bmp \
	../../ui/images/zoomOut.bmp
