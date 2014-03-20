# Microsoft Developer Studio Project File - Name="terraStat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=terraStat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "terraStat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "terraStat.mak" CFG="terraStat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "terraStat - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "terraStat - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "terraStat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MT /W3 /GR /GX /O1 /I "..\..\..\src\STLport" /I "." /I ".." /I "..\..\..\src\terralib\kernel" /I "..\..\..\src\terralib\application" /I "..\..\..\src\terralib\functions" /I "..\..\..\src\terralib\stat" /I "..\..\..\src\shapelib" /I "..\..\..\src\zlib" /I "..\..\..\src\freetype\include" /I "..\..\..\src\terralib\application\qt" /I "..\..\..\terralibw\ado" /I "..\..\..\src\terralib\drivers\MySQL" /I "..\..\..\src\terralib\drivers\MySQL\include" /I "..\..\..\src\terralib\drivers\OracleSpatial" /I "..\..\..\src\terralib\drivers\OracleSpatial\OCI\include" /I "..\..\..\src\terralib\drivers\PostgreSQL" /I "..\..\..\src\terralib\drivers\PostgreSQL\includepg" /I "..\..\..\src\terralib\drivers\PostgreSQL\interfacespg\libpq" /I "$(QTDIR)\include" /I ".ui\\" /I "..\..\ui" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "QT_NO_DEBUG" /U "UNICODE" /U "_MBCS" /FD /D /D /D /D /D -Zm200 /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x416
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 delayimp.lib "qt.lib" "qtmain.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "..\..\..\terralibw\freetype\Release\freetype.lib" "..\..\..\terralibw\shapelib\Release\shapelib.lib" "..\..\..\terralibw\stat\Release\stat.lib" "..\..\..\terralibw\terralib\Release\terralib.lib" "..\..\..\terralibw\tiff\Release\tiff.lib" "..\..\..\terralibw\jpeg\ijl15.lib" "..\..\..\terralibw\OracleSpatial\Oci\Oci.lib" "..\..\..\terralibw\MySQL\libMySQL.lib" "..\..\..\terralibw\PostgreSQL\libpqdll.lib" "opengl32.lib" "glu32.lib" "delayimp.lib" /nologo /subsystem:windows /machine:IX86 /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcmtd.lib" /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\..\..\src\STLport" /I "." /I ".." /I "..\..\..\src\terralib\kernel" /I "..\..\..\src\terralib\application" /I "..\..\..\src\terralib\functions" /I "..\..\..\src\terralib\stat" /I "..\..\..\src\shapelib" /I "..\..\..\src\zlib" /I "..\..\..\src\freetype\include" /I "..\..\..\src\terralib\application\qt" /I "..\..\..\terralibw\ado" /I "..\..\..\src\terralib\drivers\MySQL" /I "..\..\..\src\terralib\drivers\MySQL\include" /I "..\..\..\src\terralib\drivers\OracleSpatial" /I "..\..\..\src\terralib\drivers\OracleSpatial\OCI\include" /I "..\..\..\src\terralib\drivers\PostgreSQL" /I "..\..\..\src\terralib\drivers\PostgreSQL\includepg" /I "..\..\..\src\terralib\drivers\PostgreSQL\interfacespg\libpq" /I "$(QTDIR)\include" /I ".ui\\" /I "..\..\ui" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /U "UNICODE" /U "_MBCS" /FD /GZ /D /D /D /D /D -Zm200 /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x416
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 "qt.lib" "qtmain.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "..\..\..\terralibw\freetype\Debug\freetype.lib" "..\..\..\terralibw\shapelib\Debug\shapelib.lib" "..\..\..\terralibw\stat\Debug\stat.lib" "..\..\..\terralibw\terralib\Debug\terralib.lib" "..\..\..\terralibw\tiff\Debug\tiff.lib" "..\..\..\terralibw\jpeg\ijl15.lib" "..\..\..\terralibw\OracleSpatial\Oci\Oci.lib" "..\..\..\terralibw\MySQL\libMySQL.lib" "..\..\..\terralibw\PostgreSQL\libpqdll.lib" "opengl32.lib" "glu32.lib" "delayimp.lib" /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib"

!ENDIF 

# Begin Target

# Name "terraStat - Win32 Release"
# Name "terraStat - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\ui\addColumn.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\addressLocatorWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\addTheme.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\addView.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\anaesp.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\animation.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\bayesWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\changeColData.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\connect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ui\connectView.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\contrast.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\countComputing.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\createCells.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\createLayerFromTheme.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\createTable.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\cursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ui\database.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\display.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\exportWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\fillAttrCell.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpAdd.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpAggregation.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpOverlayIntersection.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpOverlayUnion.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpSpatialJoin.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\graphic.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\graphicParams.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\help.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\import.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\importRaster.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\importRasterSimple.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\importTable.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\importTablePoints.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\insertPoint.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\insertSymbol.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\insertText.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\kernelWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\layerProp.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\lbsDefinition.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\legend.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\linkExtTable.ui.h
# End Source File
# Begin Source File

SOURCE=..\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ui\mediaDescription.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\projection.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\query.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\rasterSlicingWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\removeTable.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\saveAsTable.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\saveRaster.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\sdo_oci.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ui\selThemeTables.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\semivar.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\semivarGraph.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\skaterGraphWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\skaterWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\spatializationWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\spatialQuery.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\statistic.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\tableProp.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeAddressLocator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\terralibw\ado\TeAdoDB.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\terralibw\ado\TeAdoOracle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\terralibw\ado\TeAdoSqlServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeApplicationUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeCellAlgorithms.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeColorUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeCoordAlgorithms.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeDatabaseUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeDecoderQtImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeExportMIF.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeExportSHP.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeExportSPR.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeGeoProcessingFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeGUIUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeImportBNA.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeImportDBF.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeImportGeo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeImportMIF.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeImportSHP.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeLayerFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeMIFProjection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeMSVFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\MySQL\TeMySQL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\TeOracleSpatial.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\PostgreSQL\TePGInterface.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\PostgreSQL\TePostgreSQL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtAnimaThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtBasicCanvas.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtCanvas.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtChartItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtCheckListItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtDatabaseItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtDatabasesListView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtGrid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtLayerItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtLegendItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtMethods.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtShowMedia.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtTerraStat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtTextEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtThemeItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtViewItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtViewsListView.cpp
# End Source File
# Begin Source File

SOURCE=..\terraStat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ui\terraViewBase.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeSemivarModelFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeSimilarity.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeSPRFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeThemeApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeUpdateDBVersion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeWaitCursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ui\textRep.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\themeVis.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\urlWindow.ui.h
# End Source File
# Begin Source File

SOURCE=..\..\ui\visual.ui.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\connect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\cursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\ociap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\resultVisual.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\sdo_oci.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeAddressLocator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\terralibw\ado\TeAdoDB.h
# End Source File
# Begin Source File

SOURCE=..\..\..\terralibw\ado\TeAdoOracle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\terralibw\ado\TeAdoSqlServer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeApplicationUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeCellAlgorithms.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeColorUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeCoordAlgorithms.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeDatabaseUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeDecoderQtImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeGeoProcessingFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeImportExport.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeLayerFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeMIFProjection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeMSVFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\MySQL\TeMySQL.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\OracleSpatial\TeOracleSpatial.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\PostgreSQL\TePGInterface.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TePlotTheme.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\drivers\PostgreSQL\TePostgreSQL.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtAnimaThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtBasicCanvas.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtCanvas.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtCanvas.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtCanvas.h

".moc\moc_TeQtCanvas.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtCanvas.h -o .moc\moc_TeQtCanvas.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTC="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtCanvas.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtCanvas.h

".moc\moc_TeQtCanvas.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtCanvas.h -o .moc\moc_TeQtCanvas.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtChartItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtCheckListItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtDatabaseItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtDatabasesListView.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTD="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtDatabasesListView.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtDatabasesListView.h

".moc\moc_TeQtDatabasesListView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtDatabasesListView.h -o .moc\moc_TeQtDatabasesListView.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTD="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtDatabasesListView.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtDatabasesListView.h

".moc\moc_TeQtDatabasesListView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtDatabasesListView.h -o .moc\moc_TeQtDatabasesListView.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtGraph.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTG="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtGraph.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtGraph.h

".moc\moc_TeQtGraph.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtGraph.h -o .moc\moc_TeQtGraph.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTG="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtGraph.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtGraph.h

".moc\moc_TeQtGraph.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtGraph.h -o .moc\moc_TeQtGraph.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtGrid.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTGR="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtGrid.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtGrid.h

".moc\moc_TeQtGrid.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtGrid.h -o .moc\moc_TeQtGrid.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTGR="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtGrid.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtGrid.h

".moc\moc_TeQtGrid.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtGrid.h -o .moc\moc_TeQtGrid.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtLayerItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtLegendItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtMethods.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTM="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtMethods.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtMethods.h

".moc\moc_TeQtMethods.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtMethods.h -o .moc\moc_TeQtMethods.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTM="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtMethods.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtMethods.h

".moc\moc_TeQtMethods.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtMethods.h -o .moc\moc_TeQtMethods.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtProgress.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTP="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtProgress.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtProgress.h

".moc\moc_TeQtProgress.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtProgress.h -o .moc\moc_TeQtProgress.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTP="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtProgress.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtProgress.h

".moc\moc_TeQtProgress.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtProgress.h -o .moc\moc_TeQtProgress.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtShowMedia.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTS="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtShowMedia.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtShowMedia.h

".moc\moc_TeQtShowMedia.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtShowMedia.h -o .moc\moc_TeQtShowMedia.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTS="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtShowMedia.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtShowMedia.h

".moc\moc_TeQtShowMedia.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtShowMedia.h -o .moc\moc_TeQtShowMedia.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtTerraStat.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTT="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtTerraStat.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtTerraStat.h

".moc\moc_TeQtTerraStat.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtTerraStat.h -o .moc\moc_TeQtTerraStat.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTT="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtTerraStat.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtTerraStat.h

".moc\moc_TeQtTerraStat.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtTerraStat.h -o .moc\moc_TeQtTerraStat.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtTextEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtThemeItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtViewItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeQtViewsListView.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEQTV="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtViewsListView.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtViewsListView.h

".moc\moc_TeQtViewsListView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtViewsListView.h -o .moc\moc_TeQtViewsListView.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEQTV="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\..\..\src\terralib\application\qt\TeQtViewsListView.h...
InputPath=..\..\..\src\terralib\application\qt\TeQtViewsListView.h

".moc\moc_TeQtViewsListView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\..\..\src\terralib\application\qt\TeQtViewsListView.h -o .moc\moc_TeQtViewsListView.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\terraStat.h

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TERRA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\terraStat.h...
InputPath=..\terraStat.h

".moc\moc_terraStat.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\terraStat.h -o .moc\moc_terraStat.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TERRA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing ..\terraStat.h...
InputPath=..\terraStat.h

".moc\moc_terraStat.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\terraStat.h -o .moc\moc_terraStat.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeSemivarModelFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\functions\TeSPRFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeThemeApplication.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\TeUpdateDBVersion.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\terralib\application\qt\TeWaitCursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zlib\zlib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Forms"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=..\..\ui\addColumn.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__ADDCO="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\addColumn.ui...
InputPath=..\..\ui\addColumn.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\addColumn.ui -o .ui\addColumn.h \
	$(QTDIR)\bin\uic ..\..\ui\addColumn.ui -i addColumn.h -o .ui\addColumn.cpp \
	$(QTDIR)\bin\moc .ui\addColumn.h -o .moc\moc_addColumn.cpp \
	

".ui\addColumn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\addColumn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_addColumn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__ADDCO="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\addColumn.ui...
InputPath=..\..\ui\addColumn.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\addColumn.ui -o .ui\addColumn.h \
	$(QTDIR)\bin\uic ..\..\ui\addColumn.ui -i addColumn.h -o .ui\addColumn.cpp \
	$(QTDIR)\bin\moc .ui\addColumn.h -o .moc\moc_addColumn.cpp \
	

".ui\addColumn.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\addColumn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_addColumn.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\addressLocatorWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__ADDRE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\addressLocatorWindow.ui...
InputPath=..\..\ui\addressLocatorWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\addressLocatorWindow.ui -o .ui\addressLocatorWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\addressLocatorWindow.ui -i addressLocatorWindow.h -o .ui\addressLocatorWindow.cpp \
	$(QTDIR)\bin\moc .ui\addressLocatorWindow.h -o .moc\moc_addressLocatorWindow.cpp \
	

".ui\addressLocatorWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\addressLocatorWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_addressLocatorWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__ADDRE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\addressLocatorWindow.ui...
InputPath=..\..\ui\addressLocatorWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\addressLocatorWindow.ui -o .ui\addressLocatorWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\addressLocatorWindow.ui -i addressLocatorWindow.h -o .ui\addressLocatorWindow.cpp \
	$(QTDIR)\bin\moc .ui\addressLocatorWindow.h -o .moc\moc_addressLocatorWindow.cpp \
	

".ui\addressLocatorWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\addressLocatorWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_addressLocatorWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\addTheme.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__ADDTH="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\addTheme.ui...
InputPath=..\..\ui\addTheme.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\addTheme.ui -o .ui\addTheme.h \
	$(QTDIR)\bin\uic ..\..\ui\addTheme.ui -i addTheme.h -o .ui\addTheme.cpp \
	$(QTDIR)\bin\moc .ui\addTheme.h -o .moc\moc_addTheme.cpp \
	

".ui\addTheme.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\addTheme.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_addTheme.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__ADDTH="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\addTheme.ui...
InputPath=..\..\ui\addTheme.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\addTheme.ui -o .ui\addTheme.h \
	$(QTDIR)\bin\uic ..\..\ui\addTheme.ui -i addTheme.h -o .ui\addTheme.cpp \
	$(QTDIR)\bin\moc .ui\addTheme.h -o .moc\moc_addTheme.cpp \
	

".ui\addTheme.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\addTheme.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_addTheme.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\addView.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__ADDVI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\addView.ui...
InputPath=..\..\ui\addView.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\addView.ui -o .ui\addView.h \
	$(QTDIR)\bin\uic ..\..\ui\addView.ui -i addView.h -o .ui\addView.cpp \
	$(QTDIR)\bin\moc .ui\addView.h -o .moc\moc_addView.cpp \
	

".ui\addView.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\addView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_addView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__ADDVI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\addView.ui...
InputPath=..\..\ui\addView.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\addView.ui -o .ui\addView.h \
	$(QTDIR)\bin\uic ..\..\ui\addView.ui -i addView.h -o .ui\addView.cpp \
	$(QTDIR)\bin\moc .ui\addView.h -o .moc\moc_addView.cpp \
	

".ui\addView.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\addView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_addView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\adjustAxis.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__ADJUS="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\adjustAxis.ui...
InputPath=..\..\ui\adjustAxis.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\adjustAxis.ui -o .ui\adjustAxis.h \
	$(QTDIR)\bin\uic ..\..\ui\adjustAxis.ui -i adjustAxis.h -o .ui\adjustAxis.cpp \
	$(QTDIR)\bin\moc .ui\adjustAxis.h -o .moc\moc_adjustAxis.cpp \
	

".ui\adjustAxis.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\adjustAxis.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_adjustAxis.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__ADJUS="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\adjustAxis.ui...
InputPath=..\..\ui\adjustAxis.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\adjustAxis.ui -o .ui\adjustAxis.h \
	$(QTDIR)\bin\uic ..\..\ui\adjustAxis.ui -i adjustAxis.h -o .ui\adjustAxis.cpp \
	$(QTDIR)\bin\moc .ui\adjustAxis.h -o .moc\moc_adjustAxis.cpp \
	

".ui\adjustAxis.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\adjustAxis.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_adjustAxis.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\anaesp.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__ANAES="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\anaesp.ui...
InputPath=..\..\ui\anaesp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\anaesp.ui -o .ui\anaesp.h \
	$(QTDIR)\bin\uic ..\..\ui\anaesp.ui -i anaesp.h -o .ui\anaesp.cpp \
	$(QTDIR)\bin\moc .ui\anaesp.h -o .moc\moc_anaesp.cpp \
	

".ui\anaesp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\anaesp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_anaesp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__ANAES="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\anaesp.ui...
InputPath=..\..\ui\anaesp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\anaesp.ui -o .ui\anaesp.h \
	$(QTDIR)\bin\uic ..\..\ui\anaesp.ui -i anaesp.h -o .ui\anaesp.cpp \
	$(QTDIR)\bin\moc .ui\anaesp.h -o .moc\moc_anaesp.cpp \
	

".ui\anaesp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\anaesp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_anaesp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\animation.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__ANIMA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\animation.ui...
InputPath=..\..\ui\animation.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\animation.ui -o .ui\animation.h \
	$(QTDIR)\bin\uic ..\..\ui\animation.ui -i animation.h -o .ui\animation.cpp \
	$(QTDIR)\bin\moc .ui\animation.h -o .moc\moc_animation.cpp \
	

".ui\animation.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\animation.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_animation.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__ANIMA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\animation.ui...
InputPath=..\..\ui\animation.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\animation.ui -o .ui\animation.h \
	$(QTDIR)\bin\uic ..\..\ui\animation.ui -i animation.h -o .ui\animation.cpp \
	$(QTDIR)\bin\moc .ui\animation.h -o .moc\moc_animation.cpp \
	

".ui\animation.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\animation.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_animation.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\bayesWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__BAYES="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\bayesWindow.ui...
InputPath=..\..\ui\bayesWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\bayesWindow.ui -o .ui\bayesWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\bayesWindow.ui -i bayesWindow.h -o .ui\bayesWindow.cpp \
	$(QTDIR)\bin\moc .ui\bayesWindow.h -o .moc\moc_bayesWindow.cpp \
	

".ui\bayesWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\bayesWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_bayesWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__BAYES="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\bayesWindow.ui...
InputPath=..\..\ui\bayesWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\bayesWindow.ui -o .ui\bayesWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\bayesWindow.ui -i bayesWindow.h -o .ui\bayesWindow.cpp \
	$(QTDIR)\bin\moc .ui\bayesWindow.h -o .moc\moc_bayesWindow.cpp \
	

".ui\bayesWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\bayesWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_bayesWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\changeColData.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__CHANG="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\changeColData.ui...
InputPath=..\..\ui\changeColData.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\changeColData.ui -o .ui\changeColData.h \
	$(QTDIR)\bin\uic ..\..\ui\changeColData.ui -i changeColData.h -o .ui\changeColData.cpp \
	$(QTDIR)\bin\moc .ui\changeColData.h -o .moc\moc_changeColData.cpp \
	

".ui\changeColData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\changeColData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_changeColData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__CHANG="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\changeColData.ui...
InputPath=..\..\ui\changeColData.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\changeColData.ui -o .ui\changeColData.h \
	$(QTDIR)\bin\uic ..\..\ui\changeColData.ui -i changeColData.h -o .ui\changeColData.cpp \
	$(QTDIR)\bin\moc .ui\changeColData.h -o .moc\moc_changeColData.cpp \
	

".ui\changeColData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\changeColData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_changeColData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\connectView.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__CONNE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\connectView.ui...
InputPath=..\..\ui\connectView.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\connectView.ui -o .ui\connectView.h \
	$(QTDIR)\bin\uic ..\..\ui\connectView.ui -i connectView.h -o .ui\connectView.cpp \
	$(QTDIR)\bin\moc .ui\connectView.h -o .moc\moc_connectView.cpp \
	

".ui\connectView.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\connectView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_connectView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__CONNE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\connectView.ui...
InputPath=..\..\ui\connectView.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\connectView.ui -o .ui\connectView.h \
	$(QTDIR)\bin\uic ..\..\ui\connectView.ui -i connectView.h -o .ui\connectView.cpp \
	$(QTDIR)\bin\moc .ui\connectView.h -o .moc\moc_connectView.cpp \
	

".ui\connectView.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\connectView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_connectView.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\contrast.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__CONTR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\contrast.ui...
InputPath=..\..\ui\contrast.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\contrast.ui -o .ui\contrast.h \
	$(QTDIR)\bin\uic ..\..\ui\contrast.ui -i contrast.h -o .ui\contrast.cpp \
	$(QTDIR)\bin\moc .ui\contrast.h -o .moc\moc_contrast.cpp \
	

".ui\contrast.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\contrast.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_contrast.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__CONTR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\contrast.ui...
InputPath=..\..\ui\contrast.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\contrast.ui -o .ui\contrast.h \
	$(QTDIR)\bin\uic ..\..\ui\contrast.ui -i contrast.h -o .ui\contrast.cpp \
	$(QTDIR)\bin\moc .ui\contrast.h -o .moc\moc_contrast.cpp \
	

".ui\contrast.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\contrast.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_contrast.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\countComputing.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__COUNT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\countComputing.ui...
InputPath=..\..\ui\countComputing.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\countComputing.ui -o .ui\countComputing.h \
	$(QTDIR)\bin\uic ..\..\ui\countComputing.ui -i countComputing.h -o .ui\countComputing.cpp \
	$(QTDIR)\bin\moc .ui\countComputing.h -o .moc\moc_countComputing.cpp \
	

".ui\countComputing.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\countComputing.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_countComputing.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__COUNT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\countComputing.ui...
InputPath=..\..\ui\countComputing.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\countComputing.ui -o .ui\countComputing.h \
	$(QTDIR)\bin\uic ..\..\ui\countComputing.ui -i countComputing.h -o .ui\countComputing.cpp \
	$(QTDIR)\bin\moc .ui\countComputing.h -o .moc\moc_countComputing.cpp \
	

".ui\countComputing.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\countComputing.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_countComputing.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\createCells.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__CREAT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\createCells.ui...
InputPath=..\..\ui\createCells.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\createCells.ui -o .ui\createCells.h \
	$(QTDIR)\bin\uic ..\..\ui\createCells.ui -i createCells.h -o .ui\createCells.cpp \
	$(QTDIR)\bin\moc .ui\createCells.h -o .moc\moc_createCells.cpp \
	

".ui\createCells.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\createCells.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_createCells.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__CREAT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\createCells.ui...
InputPath=..\..\ui\createCells.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\createCells.ui -o .ui\createCells.h \
	$(QTDIR)\bin\uic ..\..\ui\createCells.ui -i createCells.h -o .ui\createCells.cpp \
	$(QTDIR)\bin\moc .ui\createCells.h -o .moc\moc_createCells.cpp \
	

".ui\createCells.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\createCells.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_createCells.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\createLayerFromTheme.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__CREATE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\createLayerFromTheme.ui...
InputPath=..\..\ui\createLayerFromTheme.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\createLayerFromTheme.ui -o .ui\createLayerFromTheme.h \
	$(QTDIR)\bin\uic ..\..\ui\createLayerFromTheme.ui -i createLayerFromTheme.h -o .ui\createLayerFromTheme.cpp \
	$(QTDIR)\bin\moc .ui\createLayerFromTheme.h -o .moc\moc_createLayerFromTheme.cpp \
	

".ui\createLayerFromTheme.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\createLayerFromTheme.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_createLayerFromTheme.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__CREATE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\createLayerFromTheme.ui...
InputPath=..\..\ui\createLayerFromTheme.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\createLayerFromTheme.ui -o .ui\createLayerFromTheme.h \
	$(QTDIR)\bin\uic ..\..\ui\createLayerFromTheme.ui -i createLayerFromTheme.h -o .ui\createLayerFromTheme.cpp \
	$(QTDIR)\bin\moc .ui\createLayerFromTheme.h -o .moc\moc_createLayerFromTheme.cpp \
	

".ui\createLayerFromTheme.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\createLayerFromTheme.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_createLayerFromTheme.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\createTable.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__CREATET="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\createTable.ui...
InputPath=..\..\ui\createTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\createTable.ui -o .ui\createTable.h \
	$(QTDIR)\bin\uic ..\..\ui\createTable.ui -i createTable.h -o .ui\createTable.cpp \
	$(QTDIR)\bin\moc .ui\createTable.h -o .moc\moc_createTable.cpp \
	

".ui\createTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\createTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_createTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__CREATET="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\createTable.ui...
InputPath=..\..\ui\createTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\createTable.ui -o .ui\createTable.h \
	$(QTDIR)\bin\uic ..\..\ui\createTable.ui -i createTable.h -o .ui\createTable.cpp \
	$(QTDIR)\bin\moc .ui\createTable.h -o .moc\moc_createTable.cpp \
	

".ui\createTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\createTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_createTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\database.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__DATAB="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\database.ui...
InputPath=..\..\ui\database.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\database.ui -o .ui\database.h \
	$(QTDIR)\bin\uic ..\..\ui\database.ui -i database.h -o .ui\database.cpp \
	$(QTDIR)\bin\moc .ui\database.h -o .moc\moc_database.cpp \
	

".ui\database.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\database.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_database.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__DATAB="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\database.ui...
InputPath=..\..\ui\database.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\database.ui -o .ui\database.h \
	$(QTDIR)\bin\uic ..\..\ui\database.ui -i database.h -o .ui\database.cpp \
	$(QTDIR)\bin\moc .ui\database.h -o .moc\moc_database.cpp \
	

".ui\database.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\database.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_database.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\databaseProp.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__DATABA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\databaseProp.ui...
InputPath=..\..\ui\databaseProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\databaseProp.ui -o .ui\databaseProp.h \
	$(QTDIR)\bin\uic ..\..\ui\databaseProp.ui -i databaseProp.h -o .ui\databaseProp.cpp \
	$(QTDIR)\bin\moc .ui\databaseProp.h -o .moc\moc_databaseProp.cpp \
	

".ui\databaseProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\databaseProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_databaseProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__DATABA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\databaseProp.ui...
InputPath=..\..\ui\databaseProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\databaseProp.ui -o .ui\databaseProp.h \
	$(QTDIR)\bin\uic ..\..\ui\databaseProp.ui -i databaseProp.h -o .ui\databaseProp.cpp \
	$(QTDIR)\bin\moc .ui\databaseProp.h -o .moc\moc_databaseProp.cpp \
	

".ui\databaseProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\databaseProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_databaseProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\display.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__DISPL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\display.ui...
InputPath=..\..\ui\display.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\display.ui -o .ui\display.h \
	$(QTDIR)\bin\uic ..\..\ui\display.ui -i display.h -o .ui\display.cpp \
	$(QTDIR)\bin\moc .ui\display.h -o .moc\moc_display.cpp \
	

".ui\display.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\display.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_display.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__DISPL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\display.ui...
InputPath=..\..\ui\display.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\display.ui -o .ui\display.h \
	$(QTDIR)\bin\uic ..\..\ui\display.ui -i display.h -o .ui\display.cpp \
	$(QTDIR)\bin\moc .ui\display.h -o .moc\moc_display.cpp \
	

".ui\display.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\display.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_display.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\exportWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__EXPOR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\exportWindow.ui...
InputPath=..\..\ui\exportWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\exportWindow.ui -o .ui\exportWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\exportWindow.ui -i exportWindow.h -o .ui\exportWindow.cpp \
	$(QTDIR)\bin\moc .ui\exportWindow.h -o .moc\moc_exportWindow.cpp \
	

".ui\exportWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\exportWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_exportWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__EXPOR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\exportWindow.ui...
InputPath=..\..\ui\exportWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\exportWindow.ui -o .ui\exportWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\exportWindow.ui -i exportWindow.h -o .ui\exportWindow.cpp \
	$(QTDIR)\bin\moc .ui\exportWindow.h -o .moc\moc_exportWindow.cpp \
	

".ui\exportWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\exportWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_exportWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\fillAttrCell.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__FILLA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\fillAttrCell.ui...
InputPath=..\..\ui\fillAttrCell.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\fillAttrCell.ui -o .ui\fillAttrCell.h \
	$(QTDIR)\bin\uic ..\..\ui\fillAttrCell.ui -i fillAttrCell.h -o .ui\fillAttrCell.cpp \
	$(QTDIR)\bin\moc .ui\fillAttrCell.h -o .moc\moc_fillAttrCell.cpp \
	

".ui\fillAttrCell.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\fillAttrCell.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_fillAttrCell.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__FILLA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\fillAttrCell.ui...
InputPath=..\..\ui\fillAttrCell.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\fillAttrCell.ui -o .ui\fillAttrCell.h \
	$(QTDIR)\bin\uic ..\..\ui\fillAttrCell.ui -i fillAttrCell.h -o .ui\fillAttrCell.cpp \
	$(QTDIR)\bin\moc .ui\fillAttrCell.h -o .moc\moc_fillAttrCell.cpp \
	

".ui\fillAttrCell.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\fillAttrCell.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_fillAttrCell.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpAdd.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__GEOOP="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpAdd.ui...
InputPath=..\..\ui\geoOpAdd.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpAdd.ui -o .ui\geoOpAdd.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpAdd.ui -i geoOpAdd.h -o .ui\geoOpAdd.cpp \
	$(QTDIR)\bin\moc .ui\geoOpAdd.h -o .moc\moc_geoOpAdd.cpp \
	

".ui\geoOpAdd.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpAdd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpAdd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__GEOOP="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpAdd.ui...
InputPath=..\..\ui\geoOpAdd.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpAdd.ui -o .ui\geoOpAdd.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpAdd.ui -i geoOpAdd.h -o .ui\geoOpAdd.cpp \
	$(QTDIR)\bin\moc .ui\geoOpAdd.h -o .moc\moc_geoOpAdd.cpp \
	

".ui\geoOpAdd.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpAdd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpAdd.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpAggregation.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__GEOOPA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpAggregation.ui...
InputPath=..\..\ui\geoOpAggregation.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpAggregation.ui -o .ui\geoOpAggregation.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpAggregation.ui -i geoOpAggregation.h -o .ui\geoOpAggregation.cpp \
	$(QTDIR)\bin\moc .ui\geoOpAggregation.h -o .moc\moc_geoOpAggregation.cpp \
	

".ui\geoOpAggregation.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpAggregation.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpAggregation.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__GEOOPA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpAggregation.ui...
InputPath=..\..\ui\geoOpAggregation.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpAggregation.ui -o .ui\geoOpAggregation.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpAggregation.ui -i geoOpAggregation.h -o .ui\geoOpAggregation.cpp \
	$(QTDIR)\bin\moc .ui\geoOpAggregation.h -o .moc\moc_geoOpAggregation.cpp \
	

".ui\geoOpAggregation.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpAggregation.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpAggregation.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpOverlayIntersection.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__GEOOPO="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpOverlayIntersection.ui...
InputPath=..\..\ui\geoOpOverlayIntersection.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpOverlayIntersection.ui -o .ui\geoOpOverlayIntersection.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpOverlayIntersection.ui -i geoOpOverlayIntersection.h -o .ui\geoOpOverlayIntersection.cpp \
	$(QTDIR)\bin\moc .ui\geoOpOverlayIntersection.h -o .moc\moc_geoOpOverlayIntersection.cpp \
	

".ui\geoOpOverlayIntersection.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpOverlayIntersection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpOverlayIntersection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__GEOOPO="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpOverlayIntersection.ui...
InputPath=..\..\ui\geoOpOverlayIntersection.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpOverlayIntersection.ui -o .ui\geoOpOverlayIntersection.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpOverlayIntersection.ui -i geoOpOverlayIntersection.h -o .ui\geoOpOverlayIntersection.cpp \
	$(QTDIR)\bin\moc .ui\geoOpOverlayIntersection.h -o .moc\moc_geoOpOverlayIntersection.cpp \
	

".ui\geoOpOverlayIntersection.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpOverlayIntersection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpOverlayIntersection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpOverlayUnion.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__GEOOPOV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpOverlayUnion.ui...
InputPath=..\..\ui\geoOpOverlayUnion.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpOverlayUnion.ui -o .ui\geoOpOverlayUnion.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpOverlayUnion.ui -i geoOpOverlayUnion.h -o .ui\geoOpOverlayUnion.cpp \
	$(QTDIR)\bin\moc .ui\geoOpOverlayUnion.h -o .moc\moc_geoOpOverlayUnion.cpp \
	

".ui\geoOpOverlayUnion.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpOverlayUnion.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpOverlayUnion.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__GEOOPOV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpOverlayUnion.ui...
InputPath=..\..\ui\geoOpOverlayUnion.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpOverlayUnion.ui -o .ui\geoOpOverlayUnion.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpOverlayUnion.ui -i geoOpOverlayUnion.h -o .ui\geoOpOverlayUnion.cpp \
	$(QTDIR)\bin\moc .ui\geoOpOverlayUnion.h -o .moc\moc_geoOpOverlayUnion.cpp \
	

".ui\geoOpOverlayUnion.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpOverlayUnion.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpOverlayUnion.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\geoOpSpatialJoin.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__GEOOPS="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpSpatialJoin.ui...
InputPath=..\..\ui\geoOpSpatialJoin.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpSpatialJoin.ui -o .ui\geoOpSpatialJoin.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpSpatialJoin.ui -i geoOpSpatialJoin.h -o .ui\geoOpSpatialJoin.cpp \
	$(QTDIR)\bin\moc .ui\geoOpSpatialJoin.h -o .moc\moc_geoOpSpatialJoin.cpp \
	

".ui\geoOpSpatialJoin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpSpatialJoin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpSpatialJoin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__GEOOPS="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\geoOpSpatialJoin.ui...
InputPath=..\..\ui\geoOpSpatialJoin.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\geoOpSpatialJoin.ui -o .ui\geoOpSpatialJoin.h \
	$(QTDIR)\bin\uic ..\..\ui\geoOpSpatialJoin.ui -i geoOpSpatialJoin.h -o .ui\geoOpSpatialJoin.cpp \
	$(QTDIR)\bin\moc .ui\geoOpSpatialJoin.h -o .moc\moc_geoOpSpatialJoin.cpp \
	

".ui\geoOpSpatialJoin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\geoOpSpatialJoin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_geoOpSpatialJoin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\graphic.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__GRAPH="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\graphic.ui...
InputPath=..\..\ui\graphic.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\graphic.ui -o .ui\graphic.h \
	$(QTDIR)\bin\uic ..\..\ui\graphic.ui -i graphic.h -o .ui\graphic.cpp \
	$(QTDIR)\bin\moc .ui\graphic.h -o .moc\moc_graphic.cpp \
	

".ui\graphic.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\graphic.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_graphic.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__GRAPH="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\graphic.ui...
InputPath=..\..\ui\graphic.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\graphic.ui -o .ui\graphic.h \
	$(QTDIR)\bin\uic ..\..\ui\graphic.ui -i graphic.h -o .ui\graphic.cpp \
	$(QTDIR)\bin\moc .ui\graphic.h -o .moc\moc_graphic.cpp \
	

".ui\graphic.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\graphic.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_graphic.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\graphicParams.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__GRAPHI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\graphicParams.ui...
InputPath=..\..\ui\graphicParams.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\graphicParams.ui -o .ui\graphicParams.h \
	$(QTDIR)\bin\uic ..\..\ui\graphicParams.ui -i graphicParams.h -o .ui\graphicParams.cpp \
	$(QTDIR)\bin\moc .ui\graphicParams.h -o .moc\moc_graphicParams.cpp \
	

".ui\graphicParams.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\graphicParams.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_graphicParams.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__GRAPHI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\graphicParams.ui...
InputPath=..\..\ui\graphicParams.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\graphicParams.ui -o .ui\graphicParams.h \
	$(QTDIR)\bin\uic ..\..\ui\graphicParams.ui -i graphicParams.h -o .ui\graphicParams.cpp \
	$(QTDIR)\bin\moc .ui\graphicParams.h -o .moc\moc_graphicParams.cpp \
	

".ui\graphicParams.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\graphicParams.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_graphicParams.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\help.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__HELP_="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\help.ui...
InputPath=..\..\ui\help.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\help.ui -o .ui\help.h \
	$(QTDIR)\bin\uic ..\..\ui\help.ui -i help.h -o .ui\help.cpp \
	$(QTDIR)\bin\moc .ui\help.h -o .moc\moc_help.cpp \
	

".ui\help.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\help.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_help.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__HELP_="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\help.ui...
InputPath=..\..\ui\help.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\help.ui -o .ui\help.h \
	$(QTDIR)\bin\uic ..\..\ui\help.ui -i help.h -o .ui\help.cpp \
	$(QTDIR)\bin\moc .ui\help.h -o .moc\moc_help.cpp \
	

".ui\help.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\help.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_help.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\import.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__IMPOR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\import.ui...
InputPath=..\..\ui\import.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\import.ui -o .ui\import.h \
	$(QTDIR)\bin\uic ..\..\ui\import.ui -i import.h -o .ui\import.cpp \
	$(QTDIR)\bin\moc .ui\import.h -o .moc\moc_import.cpp \
	

".ui\import.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\import.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_import.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__IMPOR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\import.ui...
InputPath=..\..\ui\import.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\import.ui -o .ui\import.h \
	$(QTDIR)\bin\uic ..\..\ui\import.ui -i import.h -o .ui\import.cpp \
	$(QTDIR)\bin\moc .ui\import.h -o .moc\moc_import.cpp \
	

".ui\import.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\import.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_import.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\importRaster.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__IMPORT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\importRaster.ui...
InputPath=..\..\ui\importRaster.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\importRaster.ui -o .ui\importRaster.h \
	$(QTDIR)\bin\uic ..\..\ui\importRaster.ui -i importRaster.h -o .ui\importRaster.cpp \
	$(QTDIR)\bin\moc .ui\importRaster.h -o .moc\moc_importRaster.cpp \
	

".ui\importRaster.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\importRaster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_importRaster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__IMPORT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\importRaster.ui...
InputPath=..\..\ui\importRaster.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\importRaster.ui -o .ui\importRaster.h \
	$(QTDIR)\bin\uic ..\..\ui\importRaster.ui -i importRaster.h -o .ui\importRaster.cpp \
	$(QTDIR)\bin\moc .ui\importRaster.h -o .moc\moc_importRaster.cpp \
	

".ui\importRaster.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\importRaster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_importRaster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\importRasterSimple.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__IMPORTR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\importRasterSimple.ui...
InputPath=..\..\ui\importRasterSimple.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\importRasterSimple.ui -o .ui\importRasterSimple.h \
	$(QTDIR)\bin\uic ..\..\ui\importRasterSimple.ui -i importRasterSimple.h -o .ui\importRasterSimple.cpp \
	$(QTDIR)\bin\moc .ui\importRasterSimple.h -o .moc\moc_importRasterSimple.cpp \
	

".ui\importRasterSimple.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\importRasterSimple.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_importRasterSimple.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__IMPORTR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\importRasterSimple.ui...
InputPath=..\..\ui\importRasterSimple.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\importRasterSimple.ui -o .ui\importRasterSimple.h \
	$(QTDIR)\bin\uic ..\..\ui\importRasterSimple.ui -i importRasterSimple.h -o .ui\importRasterSimple.cpp \
	$(QTDIR)\bin\moc .ui\importRasterSimple.h -o .moc\moc_importRasterSimple.cpp \
	

".ui\importRasterSimple.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\importRasterSimple.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_importRasterSimple.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\importTable.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__IMPORTT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\importTable.ui...
InputPath=..\..\ui\importTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\importTable.ui -o .ui\importTable.h \
	$(QTDIR)\bin\uic ..\..\ui\importTable.ui -i importTable.h -o .ui\importTable.cpp \
	$(QTDIR)\bin\moc .ui\importTable.h -o .moc\moc_importTable.cpp \
	

".ui\importTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\importTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_importTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__IMPORTT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\importTable.ui...
InputPath=..\..\ui\importTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\importTable.ui -o .ui\importTable.h \
	$(QTDIR)\bin\uic ..\..\ui\importTable.ui -i importTable.h -o .ui\importTable.cpp \
	$(QTDIR)\bin\moc .ui\importTable.h -o .moc\moc_importTable.cpp \
	

".ui\importTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\importTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_importTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\importTablePoints.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__IMPORTTA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\importTablePoints.ui...
InputPath=..\..\ui\importTablePoints.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\importTablePoints.ui -o .ui\importTablePoints.h \
	$(QTDIR)\bin\uic ..\..\ui\importTablePoints.ui -i importTablePoints.h -o .ui\importTablePoints.cpp \
	$(QTDIR)\bin\moc .ui\importTablePoints.h -o .moc\moc_importTablePoints.cpp \
	

".ui\importTablePoints.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\importTablePoints.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_importTablePoints.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__IMPORTTA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\importTablePoints.ui...
InputPath=..\..\ui\importTablePoints.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\importTablePoints.ui -o .ui\importTablePoints.h \
	$(QTDIR)\bin\uic ..\..\ui\importTablePoints.ui -i importTablePoints.h -o .ui\importTablePoints.cpp \
	$(QTDIR)\bin\moc .ui\importTablePoints.h -o .moc\moc_importTablePoints.cpp \
	

".ui\importTablePoints.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\importTablePoints.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_importTablePoints.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\insertPoint.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__INSER="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\insertPoint.ui...
InputPath=..\..\ui\insertPoint.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\insertPoint.ui -o .ui\insertPoint.h \
	$(QTDIR)\bin\uic ..\..\ui\insertPoint.ui -i insertPoint.h -o .ui\insertPoint.cpp \
	$(QTDIR)\bin\moc .ui\insertPoint.h -o .moc\moc_insertPoint.cpp \
	

".ui\insertPoint.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\insertPoint.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_insertPoint.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__INSER="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\insertPoint.ui...
InputPath=..\..\ui\insertPoint.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\insertPoint.ui -o .ui\insertPoint.h \
	$(QTDIR)\bin\uic ..\..\ui\insertPoint.ui -i insertPoint.h -o .ui\insertPoint.cpp \
	$(QTDIR)\bin\moc .ui\insertPoint.h -o .moc\moc_insertPoint.cpp \
	

".ui\insertPoint.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\insertPoint.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_insertPoint.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\insertSymbol.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__INSERT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\insertSymbol.ui...
InputPath=..\..\ui\insertSymbol.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\insertSymbol.ui -o .ui\insertSymbol.h \
	$(QTDIR)\bin\uic ..\..\ui\insertSymbol.ui -i insertSymbol.h -o .ui\insertSymbol.cpp \
	$(QTDIR)\bin\moc .ui\insertSymbol.h -o .moc\moc_insertSymbol.cpp \
	

".ui\insertSymbol.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\insertSymbol.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_insertSymbol.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__INSERT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\insertSymbol.ui...
InputPath=..\..\ui\insertSymbol.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\insertSymbol.ui -o .ui\insertSymbol.h \
	$(QTDIR)\bin\uic ..\..\ui\insertSymbol.ui -i insertSymbol.h -o .ui\insertSymbol.cpp \
	$(QTDIR)\bin\moc .ui\insertSymbol.h -o .moc\moc_insertSymbol.cpp \
	

".ui\insertSymbol.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\insertSymbol.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_insertSymbol.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\insertText.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__INSERTT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\insertText.ui...
InputPath=..\..\ui\insertText.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\insertText.ui -o .ui\insertText.h \
	$(QTDIR)\bin\uic ..\..\ui\insertText.ui -i insertText.h -o .ui\insertText.cpp \
	$(QTDIR)\bin\moc .ui\insertText.h -o .moc\moc_insertText.cpp \
	

".ui\insertText.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\insertText.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_insertText.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__INSERTT="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\insertText.ui...
InputPath=..\..\ui\insertText.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\insertText.ui -o .ui\insertText.h \
	$(QTDIR)\bin\uic ..\..\ui\insertText.ui -i insertText.h -o .ui\insertText.cpp \
	$(QTDIR)\bin\moc .ui\insertText.h -o .moc\moc_insertText.cpp \
	

".ui\insertText.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\insertText.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_insertText.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\kernelWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__KERNE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\kernelWindow.ui...
InputPath=..\..\ui\kernelWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\kernelWindow.ui -o .ui\kernelWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\kernelWindow.ui -i kernelWindow.h -o .ui\kernelWindow.cpp \
	$(QTDIR)\bin\moc .ui\kernelWindow.h -o .moc\moc_kernelWindow.cpp \
	

".ui\kernelWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\kernelWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_kernelWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__KERNE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\kernelWindow.ui...
InputPath=..\..\ui\kernelWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\kernelWindow.ui -o .ui\kernelWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\kernelWindow.ui -i kernelWindow.h -o .ui\kernelWindow.cpp \
	$(QTDIR)\bin\moc .ui\kernelWindow.h -o .moc\moc_kernelWindow.cpp \
	

".ui\kernelWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\kernelWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_kernelWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\layerProp.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__LAYER="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\layerProp.ui...
InputPath=..\..\ui\layerProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\layerProp.ui -o .ui\layerProp.h \
	$(QTDIR)\bin\uic ..\..\ui\layerProp.ui -i layerProp.h -o .ui\layerProp.cpp \
	$(QTDIR)\bin\moc .ui\layerProp.h -o .moc\moc_layerProp.cpp \
	

".ui\layerProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\layerProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_layerProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__LAYER="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\layerProp.ui...
InputPath=..\..\ui\layerProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\layerProp.ui -o .ui\layerProp.h \
	$(QTDIR)\bin\uic ..\..\ui\layerProp.ui -i layerProp.h -o .ui\layerProp.cpp \
	$(QTDIR)\bin\moc .ui\layerProp.h -o .moc\moc_layerProp.cpp \
	

".ui\layerProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\layerProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_layerProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\lbsDefinition.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__LBSDE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\lbsDefinition.ui...
InputPath=..\..\ui\lbsDefinition.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\lbsDefinition.ui -o .ui\lbsDefinition.h \
	$(QTDIR)\bin\uic ..\..\ui\lbsDefinition.ui -i lbsDefinition.h -o .ui\lbsDefinition.cpp \
	$(QTDIR)\bin\moc .ui\lbsDefinition.h -o .moc\moc_lbsDefinition.cpp \
	

".ui\lbsDefinition.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\lbsDefinition.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_lbsDefinition.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__LBSDE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\lbsDefinition.ui...
InputPath=..\..\ui\lbsDefinition.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\lbsDefinition.ui -o .ui\lbsDefinition.h \
	$(QTDIR)\bin\uic ..\..\ui\lbsDefinition.ui -i lbsDefinition.h -o .ui\lbsDefinition.cpp \
	$(QTDIR)\bin\moc .ui\lbsDefinition.h -o .moc\moc_lbsDefinition.cpp \
	

".ui\lbsDefinition.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\lbsDefinition.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_lbsDefinition.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\legend.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__LEGEN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\legend.ui...
InputPath=..\..\ui\legend.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\legend.ui -o .ui\legend.h \
	$(QTDIR)\bin\uic ..\..\ui\legend.ui -i legend.h -o .ui\legend.cpp \
	$(QTDIR)\bin\moc .ui\legend.h -o .moc\moc_legend.cpp \
	

".ui\legend.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\legend.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_legend.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__LEGEN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\legend.ui...
InputPath=..\..\ui\legend.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\legend.ui -o .ui\legend.h \
	$(QTDIR)\bin\uic ..\..\ui\legend.ui -i legend.h -o .ui\legend.cpp \
	$(QTDIR)\bin\moc .ui\legend.h -o .moc\moc_legend.cpp \
	

".ui\legend.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\legend.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_legend.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\linkExtTable.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__LINKE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\linkExtTable.ui...
InputPath=..\..\ui\linkExtTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\linkExtTable.ui -o .ui\linkExtTable.h \
	$(QTDIR)\bin\uic ..\..\ui\linkExtTable.ui -i linkExtTable.h -o .ui\linkExtTable.cpp \
	$(QTDIR)\bin\moc .ui\linkExtTable.h -o .moc\moc_linkExtTable.cpp \
	

".ui\linkExtTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\linkExtTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_linkExtTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__LINKE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\linkExtTable.ui...
InputPath=..\..\ui\linkExtTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\linkExtTable.ui -o .ui\linkExtTable.h \
	$(QTDIR)\bin\uic ..\..\ui\linkExtTable.ui -i linkExtTable.h -o .ui\linkExtTable.cpp \
	$(QTDIR)\bin\moc .ui\linkExtTable.h -o .moc\moc_linkExtTable.cpp \
	

".ui\linkExtTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\linkExtTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_linkExtTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\mediaDescription.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__MEDIA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\mediaDescription.ui...
InputPath=..\..\ui\mediaDescription.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\mediaDescription.ui -o .ui\mediaDescription.h \
	$(QTDIR)\bin\uic ..\..\ui\mediaDescription.ui -i mediaDescription.h -o .ui\mediaDescription.cpp \
	$(QTDIR)\bin\moc .ui\mediaDescription.h -o .moc\moc_mediaDescription.cpp \
	

".ui\mediaDescription.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\mediaDescription.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_mediaDescription.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__MEDIA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\mediaDescription.ui...
InputPath=..\..\ui\mediaDescription.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\mediaDescription.ui -o .ui\mediaDescription.h \
	$(QTDIR)\bin\uic ..\..\ui\mediaDescription.ui -i mediaDescription.h -o .ui\mediaDescription.cpp \
	$(QTDIR)\bin\moc .ui\mediaDescription.h -o .moc\moc_mediaDescription.cpp \
	

".ui\mediaDescription.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\mediaDescription.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_mediaDescription.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\projection.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__PROJE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\projection.ui...
InputPath=..\..\ui\projection.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\projection.ui -o .ui\projection.h \
	$(QTDIR)\bin\uic ..\..\ui\projection.ui -i projection.h -o .ui\projection.cpp \
	$(QTDIR)\bin\moc .ui\projection.h -o .moc\moc_projection.cpp \
	

".ui\projection.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\projection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_projection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__PROJE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\projection.ui...
InputPath=..\..\ui\projection.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\projection.ui -o .ui\projection.h \
	$(QTDIR)\bin\uic ..\..\ui\projection.ui -i projection.h -o .ui\projection.cpp \
	$(QTDIR)\bin\moc .ui\projection.h -o .moc\moc_projection.cpp \
	

".ui\projection.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\projection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_projection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\query.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__QUERY="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\query.ui...
InputPath=..\..\ui\query.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\query.ui -o .ui\query.h \
	$(QTDIR)\bin\uic ..\..\ui\query.ui -i query.h -o .ui\query.cpp \
	$(QTDIR)\bin\moc .ui\query.h -o .moc\moc_query.cpp \
	

".ui\query.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\query.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_query.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__QUERY="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\query.ui...
InputPath=..\..\ui\query.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\query.ui -o .ui\query.h \
	$(QTDIR)\bin\uic ..\..\ui\query.ui -i query.h -o .ui\query.cpp \
	$(QTDIR)\bin\moc .ui\query.h -o .moc\moc_query.cpp \
	

".ui\query.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\query.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_query.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\rasterSlicingWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__RASTE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\rasterSlicingWindow.ui...
InputPath=..\..\ui\rasterSlicingWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\rasterSlicingWindow.ui -o .ui\rasterSlicingWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\rasterSlicingWindow.ui -i rasterSlicingWindow.h -o .ui\rasterSlicingWindow.cpp \
	$(QTDIR)\bin\moc .ui\rasterSlicingWindow.h -o .moc\moc_rasterSlicingWindow.cpp \
	

".ui\rasterSlicingWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\rasterSlicingWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_rasterSlicingWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__RASTE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\rasterSlicingWindow.ui...
InputPath=..\..\ui\rasterSlicingWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\rasterSlicingWindow.ui -o .ui\rasterSlicingWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\rasterSlicingWindow.ui -i rasterSlicingWindow.h -o .ui\rasterSlicingWindow.cpp \
	$(QTDIR)\bin\moc .ui\rasterSlicingWindow.h -o .moc\moc_rasterSlicingWindow.cpp \
	

".ui\rasterSlicingWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\rasterSlicingWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_rasterSlicingWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\removeTable.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__REMOV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\removeTable.ui...
InputPath=..\..\ui\removeTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\removeTable.ui -o .ui\removeTable.h \
	$(QTDIR)\bin\uic ..\..\ui\removeTable.ui -i removeTable.h -o .ui\removeTable.cpp \
	$(QTDIR)\bin\moc .ui\removeTable.h -o .moc\moc_removeTable.cpp \
	

".ui\removeTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\removeTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_removeTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__REMOV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\removeTable.ui...
InputPath=..\..\ui\removeTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\removeTable.ui -o .ui\removeTable.h \
	$(QTDIR)\bin\uic ..\..\ui\removeTable.ui -i removeTable.h -o .ui\removeTable.cpp \
	$(QTDIR)\bin\moc .ui\removeTable.h -o .moc\moc_removeTable.cpp \
	

".ui\removeTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\removeTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_removeTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\saveAsTable.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SAVEA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\saveAsTable.ui...
InputPath=..\..\ui\saveAsTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\saveAsTable.ui -o .ui\saveAsTable.h \
	$(QTDIR)\bin\uic ..\..\ui\saveAsTable.ui -i saveAsTable.h -o .ui\saveAsTable.cpp \
	$(QTDIR)\bin\moc .ui\saveAsTable.h -o .moc\moc_saveAsTable.cpp \
	

".ui\saveAsTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\saveAsTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_saveAsTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SAVEA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\saveAsTable.ui...
InputPath=..\..\ui\saveAsTable.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\saveAsTable.ui -o .ui\saveAsTable.h \
	$(QTDIR)\bin\uic ..\..\ui\saveAsTable.ui -i saveAsTable.h -o .ui\saveAsTable.cpp \
	$(QTDIR)\bin\moc .ui\saveAsTable.h -o .moc\moc_saveAsTable.cpp \
	

".ui\saveAsTable.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\saveAsTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_saveAsTable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\saveRaster.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SAVER="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\saveRaster.ui...
InputPath=..\..\ui\saveRaster.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\saveRaster.ui -o .ui\saveRaster.h \
	$(QTDIR)\bin\uic ..\..\ui\saveRaster.ui -i saveRaster.h -o .ui\saveRaster.cpp \
	$(QTDIR)\bin\moc .ui\saveRaster.h -o .moc\moc_saveRaster.cpp \
	

".ui\saveRaster.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\saveRaster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_saveRaster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SAVER="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\saveRaster.ui...
InputPath=..\..\ui\saveRaster.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\saveRaster.ui -o .ui\saveRaster.h \
	$(QTDIR)\bin\uic ..\..\ui\saveRaster.ui -i saveRaster.h -o .ui\saveRaster.cpp \
	$(QTDIR)\bin\moc .ui\saveRaster.h -o .moc\moc_saveRaster.cpp \
	

".ui\saveRaster.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\saveRaster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_saveRaster.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\selThemeTables.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SELTH="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\selThemeTables.ui...
InputPath=..\..\ui\selThemeTables.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\selThemeTables.ui -o .ui\selThemeTables.h \
	$(QTDIR)\bin\uic ..\..\ui\selThemeTables.ui -i selThemeTables.h -o .ui\selThemeTables.cpp \
	$(QTDIR)\bin\moc .ui\selThemeTables.h -o .moc\moc_selThemeTables.cpp \
	

".ui\selThemeTables.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\selThemeTables.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_selThemeTables.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SELTH="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\selThemeTables.ui...
InputPath=..\..\ui\selThemeTables.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\selThemeTables.ui -o .ui\selThemeTables.h \
	$(QTDIR)\bin\uic ..\..\ui\selThemeTables.ui -i selThemeTables.h -o .ui\selThemeTables.cpp \
	$(QTDIR)\bin\moc .ui\selThemeTables.h -o .moc\moc_selThemeTables.cpp \
	

".ui\selThemeTables.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\selThemeTables.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_selThemeTables.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\semivar.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SEMIV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\semivar.ui...
InputPath=..\..\ui\semivar.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\semivar.ui -o .ui\semivar.h \
	$(QTDIR)\bin\uic ..\..\ui\semivar.ui -i semivar.h -o .ui\semivar.cpp \
	$(QTDIR)\bin\moc .ui\semivar.h -o .moc\moc_semivar.cpp \
	

".ui\semivar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\semivar.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_semivar.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SEMIV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\semivar.ui...
InputPath=..\..\ui\semivar.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\semivar.ui -o .ui\semivar.h \
	$(QTDIR)\bin\uic ..\..\ui\semivar.ui -i semivar.h -o .ui\semivar.cpp \
	$(QTDIR)\bin\moc .ui\semivar.h -o .moc\moc_semivar.cpp \
	

".ui\semivar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\semivar.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_semivar.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\semivarGraph.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SEMIVA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\semivarGraph.ui...
InputPath=..\..\ui\semivarGraph.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\semivarGraph.ui -o .ui\semivarGraph.h \
	$(QTDIR)\bin\uic ..\..\ui\semivarGraph.ui -i semivarGraph.h -o .ui\semivarGraph.cpp \
	$(QTDIR)\bin\moc .ui\semivarGraph.h -o .moc\moc_semivarGraph.cpp \
	

".ui\semivarGraph.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\semivarGraph.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_semivarGraph.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SEMIVA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\semivarGraph.ui...
InputPath=..\..\ui\semivarGraph.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\semivarGraph.ui -o .ui\semivarGraph.h \
	$(QTDIR)\bin\uic ..\..\ui\semivarGraph.ui -i semivarGraph.h -o .ui\semivarGraph.cpp \
	$(QTDIR)\bin\moc .ui\semivarGraph.h -o .moc\moc_semivarGraph.cpp \
	

".ui\semivarGraph.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\semivarGraph.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_semivarGraph.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\skaterGraphWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SKATE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\skaterGraphWindow.ui...
InputPath=..\..\ui\skaterGraphWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\skaterGraphWindow.ui -o .ui\skaterGraphWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\skaterGraphWindow.ui -i skaterGraphWindow.h -o .ui\skaterGraphWindow.cpp \
	$(QTDIR)\bin\moc .ui\skaterGraphWindow.h -o .moc\moc_skaterGraphWindow.cpp \
	

".ui\skaterGraphWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\skaterGraphWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_skaterGraphWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SKATE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\skaterGraphWindow.ui...
InputPath=..\..\ui\skaterGraphWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\skaterGraphWindow.ui -o .ui\skaterGraphWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\skaterGraphWindow.ui -i skaterGraphWindow.h -o .ui\skaterGraphWindow.cpp \
	$(QTDIR)\bin\moc .ui\skaterGraphWindow.h -o .moc\moc_skaterGraphWindow.cpp \
	

".ui\skaterGraphWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\skaterGraphWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_skaterGraphWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\skaterWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SKATER="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\skaterWindow.ui...
InputPath=..\..\ui\skaterWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\skaterWindow.ui -o .ui\skaterWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\skaterWindow.ui -i skaterWindow.h -o .ui\skaterWindow.cpp \
	$(QTDIR)\bin\moc .ui\skaterWindow.h -o .moc\moc_skaterWindow.cpp \
	

".ui\skaterWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\skaterWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_skaterWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SKATER="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\skaterWindow.ui...
InputPath=..\..\ui\skaterWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\skaterWindow.ui -o .ui\skaterWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\skaterWindow.ui -i skaterWindow.h -o .ui\skaterWindow.cpp \
	$(QTDIR)\bin\moc .ui\skaterWindow.h -o .moc\moc_skaterWindow.cpp \
	

".ui\skaterWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\skaterWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_skaterWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\spatializationWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SPATI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\spatializationWindow.ui...
InputPath=..\..\ui\spatializationWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\spatializationWindow.ui -o .ui\spatializationWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\spatializationWindow.ui -i spatializationWindow.h -o .ui\spatializationWindow.cpp \
	$(QTDIR)\bin\moc .ui\spatializationWindow.h -o .moc\moc_spatializationWindow.cpp \
	

".ui\spatializationWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\spatializationWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_spatializationWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SPATI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\spatializationWindow.ui...
InputPath=..\..\ui\spatializationWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\spatializationWindow.ui -o .ui\spatializationWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\spatializationWindow.ui -i spatializationWindow.h -o .ui\spatializationWindow.cpp \
	$(QTDIR)\bin\moc .ui\spatializationWindow.h -o .moc\moc_spatializationWindow.cpp \
	

".ui\spatializationWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\spatializationWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_spatializationWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\spatialQuery.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__SPATIA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\spatialQuery.ui...
InputPath=..\..\ui\spatialQuery.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\spatialQuery.ui -o .ui\spatialQuery.h \
	$(QTDIR)\bin\uic ..\..\ui\spatialQuery.ui -i spatialQuery.h -o .ui\spatialQuery.cpp \
	$(QTDIR)\bin\moc .ui\spatialQuery.h -o .moc\moc_spatialQuery.cpp \
	

".ui\spatialQuery.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\spatialQuery.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_spatialQuery.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__SPATIA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\spatialQuery.ui...
InputPath=..\..\ui\spatialQuery.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\spatialQuery.ui -o .ui\spatialQuery.h \
	$(QTDIR)\bin\uic ..\..\ui\spatialQuery.ui -i spatialQuery.h -o .ui\spatialQuery.cpp \
	$(QTDIR)\bin\moc .ui\spatialQuery.h -o .moc\moc_spatialQuery.cpp \
	

".ui\spatialQuery.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\spatialQuery.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_spatialQuery.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\statistic.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__STATI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\statistic.ui...
InputPath=..\..\ui\statistic.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\statistic.ui -o .ui\statistic.h \
	$(QTDIR)\bin\uic ..\..\ui\statistic.ui -i statistic.h -o .ui\statistic.cpp \
	$(QTDIR)\bin\moc .ui\statistic.h -o .moc\moc_statistic.cpp \
	

".ui\statistic.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\statistic.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_statistic.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__STATI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\statistic.ui...
InputPath=..\..\ui\statistic.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\statistic.ui -o .ui\statistic.h \
	$(QTDIR)\bin\uic ..\..\ui\statistic.ui -i statistic.h -o .ui\statistic.cpp \
	$(QTDIR)\bin\moc .ui\statistic.h -o .moc\moc_statistic.cpp \
	

".ui\statistic.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\statistic.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_statistic.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\tableProp.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TABLE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\tableProp.ui...
InputPath=..\..\ui\tableProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\tableProp.ui -o .ui\tableProp.h \
	$(QTDIR)\bin\uic ..\..\ui\tableProp.ui -i tableProp.h -o .ui\tableProp.cpp \
	$(QTDIR)\bin\moc .ui\tableProp.h -o .moc\moc_tableProp.cpp \
	

".ui\tableProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\tableProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_tableProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TABLE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\tableProp.ui...
InputPath=..\..\ui\tableProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\tableProp.ui -o .ui\tableProp.h \
	$(QTDIR)\bin\uic ..\..\ui\tableProp.ui -i tableProp.h -o .ui\tableProp.cpp \
	$(QTDIR)\bin\moc .ui\tableProp.h -o .moc\moc_tableProp.cpp \
	

".ui\tableProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\tableProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_tableProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\terraViewBase.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TERRAV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\terraViewBase.ui...
InputPath=..\..\ui\terraViewBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\terraViewBase.ui -o .ui\terraViewBase.h \
	$(QTDIR)\bin\uic ..\..\ui\terraViewBase.ui -i terraViewBase.h -o .ui\terraViewBase.cpp \
	$(QTDIR)\bin\moc .ui\terraViewBase.h -o .moc\moc_terraViewBase.cpp \
	

".ui\terraViewBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\terraViewBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_terraViewBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TERRAV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\terraViewBase.ui...
InputPath=..\..\ui\terraViewBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\terraViewBase.ui -o .ui\terraViewBase.h \
	$(QTDIR)\bin\uic ..\..\ui\terraViewBase.ui -i terraViewBase.h -o .ui\terraViewBase.cpp \
	$(QTDIR)\bin\moc .ui\terraViewBase.h -o .moc\moc_terraViewBase.cpp \
	

".ui\terraViewBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\terraViewBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_terraViewBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\textRep.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__TEXTR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\textRep.ui...
InputPath=..\..\ui\textRep.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\textRep.ui -o .ui\textRep.h \
	$(QTDIR)\bin\uic ..\..\ui\textRep.ui -i textRep.h -o .ui\textRep.cpp \
	$(QTDIR)\bin\moc .ui\textRep.h -o .moc\moc_textRep.cpp \
	

".ui\textRep.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\textRep.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_textRep.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__TEXTR="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\textRep.ui...
InputPath=..\..\ui\textRep.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\textRep.ui -o .ui\textRep.h \
	$(QTDIR)\bin\uic ..\..\ui\textRep.ui -i textRep.h -o .ui\textRep.cpp \
	$(QTDIR)\bin\moc .ui\textRep.h -o .moc\moc_textRep.cpp \
	

".ui\textRep.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\textRep.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_textRep.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\themeProp.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__THEME="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\themeProp.ui...
InputPath=..\..\ui\themeProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\themeProp.ui -o .ui\themeProp.h \
	$(QTDIR)\bin\uic ..\..\ui\themeProp.ui -i themeProp.h -o .ui\themeProp.cpp \
	$(QTDIR)\bin\moc .ui\themeProp.h -o .moc\moc_themeProp.cpp \
	

".ui\themeProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\themeProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_themeProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__THEME="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\themeProp.ui...
InputPath=..\..\ui\themeProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\themeProp.ui -o .ui\themeProp.h \
	$(QTDIR)\bin\uic ..\..\ui\themeProp.ui -i themeProp.h -o .ui\themeProp.cpp \
	$(QTDIR)\bin\moc .ui\themeProp.h -o .moc\moc_themeProp.cpp \
	

".ui\themeProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\themeProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_themeProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\themeVis.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__THEMEV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\themeVis.ui...
InputPath=..\..\ui\themeVis.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\themeVis.ui -o .ui\themeVis.h \
	$(QTDIR)\bin\uic ..\..\ui\themeVis.ui -i themeVis.h -o .ui\themeVis.cpp \
	$(QTDIR)\bin\moc .ui\themeVis.h -o .moc\moc_themeVis.cpp \
	

".ui\themeVis.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\themeVis.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_themeVis.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__THEMEV="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\themeVis.ui...
InputPath=..\..\ui\themeVis.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\themeVis.ui -o .ui\themeVis.h \
	$(QTDIR)\bin\uic ..\..\ui\themeVis.ui -i themeVis.h -o .ui\themeVis.cpp \
	$(QTDIR)\bin\moc .ui\themeVis.h -o .moc\moc_themeVis.cpp \
	

".ui\themeVis.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\themeVis.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_themeVis.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\urlWindow.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__URLWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\urlWindow.ui...
InputPath=..\..\ui\urlWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\urlWindow.ui -o .ui\urlWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\urlWindow.ui -i urlWindow.h -o .ui\urlWindow.cpp \
	$(QTDIR)\bin\moc .ui\urlWindow.h -o .moc\moc_urlWindow.cpp \
	

".ui\urlWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\urlWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_urlWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__URLWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\urlWindow.ui...
InputPath=..\..\ui\urlWindow.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\urlWindow.ui -o .ui\urlWindow.h \
	$(QTDIR)\bin\uic ..\..\ui\urlWindow.ui -i urlWindow.h -o .ui\urlWindow.cpp \
	$(QTDIR)\bin\moc .ui\urlWindow.h -o .moc\moc_urlWindow.cpp \
	

".ui\urlWindow.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\urlWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_urlWindow.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\viewProp.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__VIEWP="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\viewProp.ui...
InputPath=..\..\ui\viewProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\viewProp.ui -o .ui\viewProp.h \
	$(QTDIR)\bin\uic ..\..\ui\viewProp.ui -i viewProp.h -o .ui\viewProp.cpp \
	$(QTDIR)\bin\moc .ui\viewProp.h -o .moc\moc_viewProp.cpp \
	

".ui\viewProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\viewProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_viewProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__VIEWP="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\viewProp.ui...
InputPath=..\..\ui\viewProp.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\viewProp.ui -o .ui\viewProp.h \
	$(QTDIR)\bin\uic ..\..\ui\viewProp.ui -i viewProp.h -o .ui\viewProp.cpp \
	$(QTDIR)\bin\moc .ui\viewProp.h -o .moc\moc_viewProp.cpp \
	

".ui\viewProp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\viewProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_viewProp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\visual.ui

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__VISUA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\visual.ui...
InputPath=..\..\ui\visual.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\visual.ui -o .ui\visual.h \
	$(QTDIR)\bin\uic ..\..\ui\visual.ui -i visual.h -o .ui\visual.cpp \
	$(QTDIR)\bin\moc .ui\visual.h -o .moc\moc_visual.cpp \
	

".ui\visual.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\visual.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_visual.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__VISUA="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ..\..\ui\visual.ui...
InputPath=..\..\ui\visual.ui

BuildCmds= \
	$(QTDIR)\bin\uic ..\..\ui\visual.ui -o .ui\visual.h \
	$(QTDIR)\bin\uic ..\..\ui\visual.ui -i visual.h -o .ui\visual.cpp \
	$(QTDIR)\bin\moc .ui\visual.h -o .moc\moc_visual.cpp \
	

".ui\visual.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".ui\visual.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".moc\moc_visual.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Translations"

# PROP Default_Filter "ts"
# Begin Source File

SOURCE=..\terraStat_pt.ts
# End Source File
# Begin Source File

SOURCE=..\terraStat_sp.ts
# End Source File
# End Group
# Begin Group "Images"

# PROP Default_Filter "png jpeg bmp xpm"
# Begin Source File

SOURCE=../../ui/images/and.bmp

!IF  "$(CFG)" == "terraStat - Win32 Release"

USERDEP__AND_B="../../ui/images/and.bmp"	"../../ui/images/area.bmp"	"../../ui/images/buffer.bmp"	"../../ui/images/clearColor.bmp"	"../../ui/images/closeDB.bmp"	"../../ui/images/containLin.bmp"	"../../ui/images/containLinPol.bmp"	"../../ui/images/containLinPon.bmp"	"../../ui/images/containPol.bmp"	"../../ui/images/convexHull.bmp"	"../../ui/images/coverByPol.bmp"	"../../ui/images/coverLinPol.bmp"	"../../ui/images/coverPol.bmp"	"../../ui/images/crossLin.bmp"	"../../ui/images/crossLinPol.bmp"	"../../ui/images/database.bmp"	"../../ui/images/databasesTree.bmp"	"../../ui/images/disjointLin.bmp"	"../../ui/images/disjointLinPol.bmp"	"../../ui/images/disjointLinPon.bmp"	"../../ui/images/disjointPol.bmp"	"../../ui/images/disjointPonPol.bmp"	"../../ui/images/display.bmp"	"../../ui/images/distance.bmp"	"../../ui/images/distMeter.bmp"	"../../ui/images/draw.bmp"	"../../ui/images/edit.xpm"	"../../ui/images/equalLin.bmp"	"../../ui/images/equalPol.bmp"	"../../ui/images/graphic.bmp"	"../../ui/images/graphicCursor.bmp"	"../../ui/images/grid.bmp"	"../../ui/images/import.bmp"	"../../ui/images/infoCursor.bmp"	"../../ui/images/infolayer.bmp"	"../../ui/images/invertColor1.bmp"\
	"../../ui/images/invertSelection.bmp"	"../../ui/images/isContainLin.bmp"	"../../ui/images/isContainLinPol.bmp"	"../../ui/images/isContainPol.bmp"	"../../ui/images/isContainPonPol.bmp"	"../../ui/images/left.bmp"	"../../ui/images/length.bmp"	"../../ui/images/minus.bmp"	"../../ui/images/nearstNeigh.bmp"	"../../ui/images/nextDisplay.bmp"	"../../ui/images/nonEdit.xpm"	"../../ui/images/openfile.bmp"	"../../ui/images/or.bmp"	"../../ui/images/overlapLin.bmp"	"../../ui/images/overlapPol.bmp"	"../../ui/images/panCursor.bmp"	"../../ui/images/pointer.bmp"	"../../ui/images/previousDisplay.bmp"	"../../ui/images/reset.bmp"	"../../ui/images/right.bmp"	"../../ui/images/table.bmp"	"../../ui/images/terralib.bmp"	"../../ui/images/theme.bmp"	"../../ui/images/tileWindows.bmp"	"../../ui/images/touchLin.bmp"	"../../ui/images/touchLinPol.bmp"	"../../ui/images/touchLinPon.bmp"	"../../ui/images/touchPol.bmp"	"../../ui/images/touchPonPol.bmp"	"../../ui/images/unselect.bmp"	"../../ui/images/view.bmp"	"../../ui/images/viewsTree.bmp"	"../../ui/images/xor.bmp"	"../../ui/images/zoomCursor.bmp"	"../../ui/images/zoomIn.bmp"	"../../ui/images/zoomOut.bmp"	
# Begin Custom Build - Creating image collection...
InputPath=../../ui/images/and.bmp

".ui\qmake_image_collection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\uic -embed terraStat -f images.tmp -o .ui\qmake_image_collection.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "terraStat - Win32 Debug"

USERDEP__AND_B="../../ui/images/and.bmp"	"../../ui/images/area.bmp"	"../../ui/images/buffer.bmp"	"../../ui/images/clearColor.bmp"	"../../ui/images/closeDB.bmp"	"../../ui/images/containLin.bmp"	"../../ui/images/containLinPol.bmp"	"../../ui/images/containLinPon.bmp"	"../../ui/images/containPol.bmp"	"../../ui/images/convexHull.bmp"	"../../ui/images/coverByPol.bmp"	"../../ui/images/coverLinPol.bmp"	"../../ui/images/coverPol.bmp"	"../../ui/images/crossLin.bmp"	"../../ui/images/crossLinPol.bmp"	"../../ui/images/database.bmp"	"../../ui/images/databasesTree.bmp"	"../../ui/images/disjointLin.bmp"	"../../ui/images/disjointLinPol.bmp"	"../../ui/images/disjointLinPon.bmp"	"../../ui/images/disjointPol.bmp"	"../../ui/images/disjointPonPol.bmp"	"../../ui/images/display.bmp"	"../../ui/images/distance.bmp"	"../../ui/images/distMeter.bmp"	"../../ui/images/draw.bmp"	"../../ui/images/edit.xpm"	"../../ui/images/equalLin.bmp"	"../../ui/images/equalPol.bmp"	"../../ui/images/graphic.bmp"	"../../ui/images/graphicCursor.bmp"	"../../ui/images/grid.bmp"	"../../ui/images/import.bmp"	"../../ui/images/infoCursor.bmp"	"../../ui/images/infolayer.bmp"	"../../ui/images/invertColor1.bmp"\
	"../../ui/images/invertSelection.bmp"	"../../ui/images/isContainLin.bmp"	"../../ui/images/isContainLinPol.bmp"	"../../ui/images/isContainPol.bmp"	"../../ui/images/isContainPonPol.bmp"	"../../ui/images/left.bmp"	"../../ui/images/length.bmp"	"../../ui/images/minus.bmp"	"../../ui/images/nearstNeigh.bmp"	"../../ui/images/nextDisplay.bmp"	"../../ui/images/nonEdit.xpm"	"../../ui/images/openfile.bmp"	"../../ui/images/or.bmp"	"../../ui/images/overlapLin.bmp"	"../../ui/images/overlapPol.bmp"	"../../ui/images/panCursor.bmp"	"../../ui/images/pointer.bmp"	"../../ui/images/previousDisplay.bmp"	"../../ui/images/reset.bmp"	"../../ui/images/right.bmp"	"../../ui/images/table.bmp"	"../../ui/images/terralib.bmp"	"../../ui/images/theme.bmp"	"../../ui/images/tileWindows.bmp"	"../../ui/images/touchLin.bmp"	"../../ui/images/touchLinPol.bmp"	"../../ui/images/touchLinPon.bmp"	"../../ui/images/touchPol.bmp"	"../../ui/images/touchPonPol.bmp"	"../../ui/images/unselect.bmp"	"../../ui/images/view.bmp"	"../../ui/images/viewsTree.bmp"	"../../ui/images/xor.bmp"	"../../ui/images/zoomCursor.bmp"	"../../ui/images/zoomIn.bmp"	"../../ui/images/zoomOut.bmp"	
# Begin Custom Build - Creating image collection...
InputPath=../../ui/images/and.bmp

".ui\qmake_image_collection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\uic -embed terraStat -f images.tmp -o .ui\qmake_image_collection.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=../../ui/images/area.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/buffer.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/clearColor.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/closeDB.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/containLin.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/containLinPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/containLinPon.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/containPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/convexHull.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/coverByPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/coverLinPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/coverPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/crossLin.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/crossLinPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/database.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/databasesTree.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/disjointLin.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/disjointLinPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/disjointLinPon.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/disjointPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/disjointPonPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/display.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/distance.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/distMeter.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/draw.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/edit.xpm
# End Source File
# Begin Source File

SOURCE=../../ui/images/equalLin.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/equalPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/graphic.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/graphicCursor.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/grid.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/import.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/infoCursor.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/infolayer.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/invertColor1.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/invertSelection.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/isContainLin.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/isContainLinPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/isContainPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/isContainPonPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/left.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/length.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/minus.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/nearstNeigh.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/nextDisplay.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/nonEdit.xpm
# End Source File
# Begin Source File

SOURCE=../../ui/images/openfile.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/or.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/overlapLin.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/overlapPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/panCursor.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/pointer.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/previousDisplay.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/reset.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/right.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/table.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/terralib.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/theme.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/tileWindows.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/touchLin.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/touchLinPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/touchLinPon.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/touchPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/touchPonPol.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/unselect.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/view.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/viewsTree.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/xor.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/zoomCursor.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/zoomIn.bmp
# End Source File
# Begin Source File

SOURCE=../../ui/images/zoomOut.bmp
# End Source File
# End Group
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.ui\addColumn.cpp
# End Source File
# Begin Source File

SOURCE=.ui\addColumn.h
# End Source File
# Begin Source File

SOURCE=.ui\addressLocatorWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\addressLocatorWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\addTheme.cpp
# End Source File
# Begin Source File

SOURCE=.ui\addTheme.h
# End Source File
# Begin Source File

SOURCE=.ui\addView.cpp
# End Source File
# Begin Source File

SOURCE=.ui\addView.h
# End Source File
# Begin Source File

SOURCE=.ui\adjustAxis.cpp
# End Source File
# Begin Source File

SOURCE=.ui\adjustAxis.h
# End Source File
# Begin Source File

SOURCE=.ui\anaesp.cpp
# End Source File
# Begin Source File

SOURCE=.ui\anaesp.h
# End Source File
# Begin Source File

SOURCE=.ui\animation.cpp
# End Source File
# Begin Source File

SOURCE=.ui\animation.h
# End Source File
# Begin Source File

SOURCE=.ui\bayesWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\bayesWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\changeColData.cpp
# End Source File
# Begin Source File

SOURCE=.ui\changeColData.h
# End Source File
# Begin Source File

SOURCE=.ui\connectView.cpp
# End Source File
# Begin Source File

SOURCE=.ui\connectView.h
# End Source File
# Begin Source File

SOURCE=.ui\contrast.cpp
# End Source File
# Begin Source File

SOURCE=.ui\contrast.h
# End Source File
# Begin Source File

SOURCE=.ui\countComputing.cpp
# End Source File
# Begin Source File

SOURCE=.ui\countComputing.h
# End Source File
# Begin Source File

SOURCE=.ui\createCells.cpp
# End Source File
# Begin Source File

SOURCE=.ui\createCells.h
# End Source File
# Begin Source File

SOURCE=.ui\createLayerFromTheme.cpp
# End Source File
# Begin Source File

SOURCE=.ui\createLayerFromTheme.h
# End Source File
# Begin Source File

SOURCE=.ui\createTable.cpp
# End Source File
# Begin Source File

SOURCE=.ui\createTable.h
# End Source File
# Begin Source File

SOURCE=.ui\database.cpp
# End Source File
# Begin Source File

SOURCE=.ui\database.h
# End Source File
# Begin Source File

SOURCE=.ui\databaseProp.cpp
# End Source File
# Begin Source File

SOURCE=.ui\databaseProp.h
# End Source File
# Begin Source File

SOURCE=.ui\display.cpp
# End Source File
# Begin Source File

SOURCE=.ui\display.h
# End Source File
# Begin Source File

SOURCE=.ui\exportWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\exportWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\fillAttrCell.cpp
# End Source File
# Begin Source File

SOURCE=.ui\fillAttrCell.h
# End Source File
# Begin Source File

SOURCE=.ui\geoOpAdd.cpp
# End Source File
# Begin Source File

SOURCE=.ui\geoOpAdd.h
# End Source File
# Begin Source File

SOURCE=.ui\geoOpAggregation.cpp
# End Source File
# Begin Source File

SOURCE=.ui\geoOpAggregation.h
# End Source File
# Begin Source File

SOURCE=.ui\geoOpOverlayIntersection.cpp
# End Source File
# Begin Source File

SOURCE=.ui\geoOpOverlayIntersection.h
# End Source File
# Begin Source File

SOURCE=.ui\geoOpOverlayUnion.cpp
# End Source File
# Begin Source File

SOURCE=.ui\geoOpOverlayUnion.h
# End Source File
# Begin Source File

SOURCE=.ui\geoOpSpatialJoin.cpp
# End Source File
# Begin Source File

SOURCE=.ui\geoOpSpatialJoin.h
# End Source File
# Begin Source File

SOURCE=.ui\graphic.cpp
# End Source File
# Begin Source File

SOURCE=.ui\graphic.h
# End Source File
# Begin Source File

SOURCE=.ui\graphicParams.cpp
# End Source File
# Begin Source File

SOURCE=.ui\graphicParams.h
# End Source File
# Begin Source File

SOURCE=.ui\help.cpp
# End Source File
# Begin Source File

SOURCE=.ui\help.h
# End Source File
# Begin Source File

SOURCE=.ui\import.cpp
# End Source File
# Begin Source File

SOURCE=.ui\import.h
# End Source File
# Begin Source File

SOURCE=.ui\importRaster.cpp
# End Source File
# Begin Source File

SOURCE=.ui\importRaster.h
# End Source File
# Begin Source File

SOURCE=.ui\importRasterSimple.cpp
# End Source File
# Begin Source File

SOURCE=.ui\importRasterSimple.h
# End Source File
# Begin Source File

SOURCE=.ui\importTable.cpp
# End Source File
# Begin Source File

SOURCE=.ui\importTable.h
# End Source File
# Begin Source File

SOURCE=.ui\importTablePoints.cpp
# End Source File
# Begin Source File

SOURCE=.ui\importTablePoints.h
# End Source File
# Begin Source File

SOURCE=.ui\insertPoint.cpp
# End Source File
# Begin Source File

SOURCE=.ui\insertPoint.h
# End Source File
# Begin Source File

SOURCE=.ui\insertSymbol.cpp
# End Source File
# Begin Source File

SOURCE=.ui\insertSymbol.h
# End Source File
# Begin Source File

SOURCE=.ui\insertText.cpp
# End Source File
# Begin Source File

SOURCE=.ui\insertText.h
# End Source File
# Begin Source File

SOURCE=.ui\kernelWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\kernelWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\layerProp.cpp
# End Source File
# Begin Source File

SOURCE=.ui\layerProp.h
# End Source File
# Begin Source File

SOURCE=.ui\lbsDefinition.cpp
# End Source File
# Begin Source File

SOURCE=.ui\lbsDefinition.h
# End Source File
# Begin Source File

SOURCE=.ui\legend.cpp
# End Source File
# Begin Source File

SOURCE=.ui\legend.h
# End Source File
# Begin Source File

SOURCE=.ui\linkExtTable.cpp
# End Source File
# Begin Source File

SOURCE=.ui\linkExtTable.h
# End Source File
# Begin Source File

SOURCE=.ui\mediaDescription.cpp
# End Source File
# Begin Source File

SOURCE=.ui\mediaDescription.h
# End Source File
# Begin Source File

SOURCE=.moc\moc_addColumn.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_addressLocatorWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_addTheme.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_addView.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_adjustAxis.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_anaesp.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_animation.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_bayesWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_changeColData.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_connectView.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_contrast.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_countComputing.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_createCells.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_createLayerFromTheme.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_createTable.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_database.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_databaseProp.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_display.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_exportWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_fillAttrCell.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_geoOpAdd.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_geoOpAggregation.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_geoOpOverlayIntersection.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_geoOpOverlayUnion.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_geoOpSpatialJoin.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_graphic.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_graphicParams.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_help.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_import.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_importRaster.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_importRasterSimple.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_importTable.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_importTablePoints.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_insertPoint.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_insertSymbol.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_insertText.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_kernelWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_layerProp.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_lbsDefinition.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_legend.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_linkExtTable.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_mediaDescription.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_projection.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_query.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_rasterSlicingWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_removeTable.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_saveAsTable.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_saveRaster.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_selThemeTables.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_semivar.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_semivarGraph.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_skaterGraphWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_skaterWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_spatializationWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_spatialQuery.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_statistic.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_tableProp.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtDatabasesListView.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtGraph.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtGrid.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtMethods.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtProgress.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtShowMedia.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtTerraStat.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_TeQtViewsListView.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_terraStat.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_terraViewBase.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_textRep.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_themeProp.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_themeVis.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_urlWindow.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_viewProp.cpp
# End Source File
# Begin Source File

SOURCE=.moc\moc_visual.cpp
# End Source File
# Begin Source File

SOURCE=.ui\projection.cpp
# End Source File
# Begin Source File

SOURCE=.ui\projection.h
# End Source File
# Begin Source File

SOURCE=.ui\qmake_image_collection.cpp
# End Source File
# Begin Source File

SOURCE=.ui\query.cpp
# End Source File
# Begin Source File

SOURCE=.ui\query.h
# End Source File
# Begin Source File

SOURCE=.ui\rasterSlicingWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\rasterSlicingWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\removeTable.cpp
# End Source File
# Begin Source File

SOURCE=.ui\removeTable.h
# End Source File
# Begin Source File

SOURCE=.ui\saveAsTable.cpp
# End Source File
# Begin Source File

SOURCE=.ui\saveAsTable.h
# End Source File
# Begin Source File

SOURCE=.ui\saveRaster.cpp
# End Source File
# Begin Source File

SOURCE=.ui\saveRaster.h
# End Source File
# Begin Source File

SOURCE=.ui\selThemeTables.cpp
# End Source File
# Begin Source File

SOURCE=.ui\selThemeTables.h
# End Source File
# Begin Source File

SOURCE=.ui\semivar.cpp
# End Source File
# Begin Source File

SOURCE=.ui\semivar.h
# End Source File
# Begin Source File

SOURCE=.ui\semivarGraph.cpp
# End Source File
# Begin Source File

SOURCE=.ui\semivarGraph.h
# End Source File
# Begin Source File

SOURCE=.ui\skaterGraphWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\skaterGraphWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\skaterWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\skaterWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\spatializationWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\spatializationWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\spatialQuery.cpp
# End Source File
# Begin Source File

SOURCE=.ui\spatialQuery.h
# End Source File
# Begin Source File

SOURCE=.ui\statistic.cpp
# End Source File
# Begin Source File

SOURCE=.ui\statistic.h
# End Source File
# Begin Source File

SOURCE=.ui\tableProp.cpp
# End Source File
# Begin Source File

SOURCE=.ui\tableProp.h
# End Source File
# Begin Source File

SOURCE=.ui\terraViewBase.cpp
# End Source File
# Begin Source File

SOURCE=.ui\terraViewBase.h
# End Source File
# Begin Source File

SOURCE=.ui\textRep.cpp
# End Source File
# Begin Source File

SOURCE=.ui\textRep.h
# End Source File
# Begin Source File

SOURCE=.ui\themeProp.cpp
# End Source File
# Begin Source File

SOURCE=.ui\themeProp.h
# End Source File
# Begin Source File

SOURCE=.ui\themeVis.cpp
# End Source File
# Begin Source File

SOURCE=.ui\themeVis.h
# End Source File
# Begin Source File

SOURCE=.ui\urlWindow.cpp
# End Source File
# Begin Source File

SOURCE=.ui\urlWindow.h
# End Source File
# Begin Source File

SOURCE=.ui\viewProp.cpp
# End Source File
# Begin Source File

SOURCE=.ui\viewProp.h
# End Source File
# Begin Source File

SOURCE=.ui\visual.cpp
# End Source File
# Begin Source File

SOURCE=.ui\visual.h
# End Source File
# End Group
# End Target
# End Project
