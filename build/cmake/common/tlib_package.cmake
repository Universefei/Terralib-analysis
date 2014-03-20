# Cmake Packager for terraLib code and binaries
# Components definitions :
# -> SOURCE        - Terralib source code.
# -> HEADER    - Terralib header code.
# -> BINARIES  - Terralib compiled library.

cmake_minimum_required(VERSION 2.8)
 
set (TVPLGS_DIR "../../terraViewPlugins") 
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/common") 
 
if( NOT TE_SRC_DIR )
	message ("Using default te_src")
	set ( TE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src/terralib" )
endif()

	
set (
  common_patterns_2_exclude
  PATTERN "*svn" EXCLUDE
  PATTERN "*cvs*" EXCLUDE

# Por enquanto ainda est�o no reposit�rio, mas n�o s�o usados
# -----------------------------------------------------------
  PATTERN "*GTM*" EXCLUDE
  PATTERN "*MrSID*" EXCLUDE
  PATTERN "*PAMDecoder*" EXCLUDE
  REGEX "data_struct" EXCLUDE
  REGEX "include_pg" EXCLUDE
  REGEX "/OCI/" EXCLUDE
  PATTERN "GL" EXCLUDE
# -----------------------------------------------------------
)

# Macro to filter the header files
MACRO (install_h_dir SRC DEST)
  install ( # -- headers
    DIRECTORY "${SRC}"
    DESTINATION "${DEST}"
    COMPONENT HEADER
    PATTERN "*.c*" EXCLUDE
    REGEX "source" EXCLUDE
    ${common_patterns_2_exclude}
  )
ENDMACRO (install_h_dir SRC DEST)

# Macro to filter the source files
MACRO (install_src_dir SRC DEST)
  install ( # -- headers
    DIRECTORY "${SRC}"
    DESTINATION "${DEST}"
    COMPONENT SOURCE
    PATTERN "*.h*" EXCLUDE
    ${common_patterns_2_exclude}
  )
ENDMACRO (install_src_dir SRC DEST)

if( TEINSTALLER_SRC ) #Projects for source
# terralib projects
  install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION ${INSTALL_CMAKE_DIR}/..
    COMPONENT SOURCE
    PATTERN "*svn" EXCLUDE
    PATTERN "*cvs*" EXCLUDE
  )
endif()



if( TEINSTALLER_DEV )
	# TerraLib configure files
	# -----------------------------------------------------------------------
	install (
	  FILES ${CMAKE_BINARY_DIR}/teConfig.h 
	  DESTINATION ${INSTALL_INCLUDE_DIR}/terralib
	  COMPONENT HEADER
	)

	# kernel
	# -----------------------------------------------------------------------
	install_h_dir(${TE_SRC_DIR}/kernel ${INSTALL_INCLUDE_DIR}/terralib)
	install_h_dir(${TE_SRC_DIR}/functions ${INSTALL_INCLUDE_DIR}/terralib)
	install_h_dir(${TE_SRC_DIR}/utils ${INSTALL_INCLUDE_DIR}/terralib)
	install_h_dir(${TE_SRC_DIR}/drivers/libjpeg ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	if( TEINSTALLER_SRC )
	  install_src_dir(${TE_SRC_DIR}/kernel ${INSTALL_SRC_DIR}/terralib)
	  install_src_dir(${TE_SRC_DIR}/functions ${INSTALL_SRC_DIR}/terralib)
	  install_src_dir(${TE_SRC_DIR}/utils ${INSTALL_SRC_DIR}/terralib)
	  install_src_dir(${TE_SRC_DIR}/drivers/libjpeg ${INSTALL_SRC_DIR}/terralib/drivers)
	endif()
	# -----------------------------------------------------------------------

	# PDI
	# -----------------------------------------------------------------------
	if(TE_PDI_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir( ${TE_SRC_DIR}/image_processing ${INSTALL_INCLUDE_DIR}/terralib )

	  if( TEINSTALLER_SRC )
	    install_src_dir( ${TE_SRC_DIR}/image_processing ${INSTALL_SRC_DIR}/terralib )
	  endif()
	endif(TE_PDI_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------

	# Stat
	# -----------------------------------------------------------------------
	if(TE_STAT_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/stat ${INSTALL_INCLUDE_DIR}/terralib)
	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/stat ${INSTALL_SRC_DIR}/terralib)
	  endif()
	endif(TE_STAT_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------

	# DTM
	# -----------------------------------------------------------------------
	if(TE_DTM_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/dtm ${INSTALL_INCLUDE_DIR}/terralib)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/dtm ${INSTALL_SRC_DIR}/terralib)
	  endif()
	endif(TE_DTM_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------
	  
	# DXF
	# -----------------------------------------------------------------------
	if(TE_DXF_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/../dxflib ${INSTALL_INCLUDE_DIR} )
	  install_h_dir(${TE_SRC_DIR}/drivers/DXF ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/../dxflib ${INSTALL_SRC_DIR} )
	    install_src_dir(${TE_SRC_DIR}/drivers/DXF ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()
	endif(TE_DXF_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------

	# SPL
	# -----------------------------------------------------------------------
	if(TE_SPL_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/../libspl ${INSTALL_INCLUDE_DIR})
	  install_h_dir(${TE_SRC_DIR}/drivers/spl ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/../libspl ${INSTALL_SRC_DIR})
	    install_src_dir(${TE_SRC_DIR}/drivers/spl ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()

	endif(TE_SPL_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------
	  
	# Shapelib
	# -----------------------------------------------------------------------
	if(TE_SHP_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/../shapelib ${INSTALL_INCLUDE_DIR})
	  install_h_dir(${TE_SRC_DIR}/drivers/shapelib ${INSTALL_INCLUDE_DIR}/terralib/drivers)
	  
	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/../shapelib ${INSTALL_SRC_DIR})
	    install_src_dir(${TE_SRC_DIR}/drivers/shapelib ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()

	endif(TE_SHP_ENABLED OR TEINSTALLER_SRC)

	# -----------------------------------------------------------------------
	  
	# MySQL support
	# -----------------------------------------------------------------------
	if (TE_MYSQL_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/drivers/MySQL ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/drivers/MySQL ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()

	  list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libmysqlclient (>=3.0.9)")

	endif(TE_MYSQL_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------
	  
	# SQLite support
	# -----------------------------------------------------------------------
	if (SQLite_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/drivers/SQLite ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/drivers/SQLite ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()
	  
	endif(SQLite_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------
	    
	# PostgreSQL / PostGIS support
	# -----------------------------------------------------------------------
	if(TE_PGRES_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/drivers/PostgreSQL ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/drivers/PostgreSQL ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()

		list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libpq5 (>= 8.4.8)")

	endif(TE_PGRES_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------
	  
	# Oracle support
	# -----------------------------------------------------------------------
	if(TE_ORACLE_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/drivers/Oracle ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/drivers/Oracle ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()

	endif(TE_ORACLE_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------
	  
	# Firebird support
	# -----------------------------------------------------------------------
	if(TE_FBIRD_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/drivers/Firebird ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/drivers/Firebird ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()

	endif(TE_FBIRD_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------
	  
	# ADO support
	# -----------------------------------------------------------------------
	if(TE_ADO_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/drivers/ado ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/drivers/ado ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()
	  


	endif(TE_ADO_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------

	# GDAL support
	# -----------------------------------------------------------------------
	if (TE_GDAL_ENABLED OR TEINSTALLER_SRC)
	  install_h_dir(${TE_SRC_DIR}/drivers/gdal ${INSTALL_INCLUDE_DIR}/terralib/drivers)

	  if( TEINSTALLER_SRC )
	    install_src_dir(${TE_SRC_DIR}/drivers/gdal ${INSTALL_SRC_DIR}/terralib/drivers)
	  endif()

	endif(TE_GDAL_ENABLED OR TEINSTALLER_SRC)
	# -----------------------------------------------------------------------

 	install (
		FILES 
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindADO.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindCrypt.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindCurl.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindFirebird.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindGdal.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindgdLib.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindGeoTIFF.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindGL.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindJPEGlib.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindMySQL.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindOracle.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindPostGIS.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindQWT.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/common/FindZLIB.cmake"	
		"${CMAKE_CURRENT_BINARY_DIR}/teBuildTreeSettings.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/teLibraryDepends.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/terralibConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/terralibConfigVersion.cmake"
		DESTINATION "${INSTALL_CMAKE_DIR}"
		COMPONENT SOURCE
		) 
endif(TEINSTALLER_DEV)


# Qt3 Drivers
# -----------
if( TE_QT3_DRIVERS_ENABLED AND (TEINSTALLER_SRC OR TE_INSTALLER_DEV))
#	install_h_dir( ${TE_SRC_DIR}/drivers/qt ${INSTALL_INCLUDE_DIR}/terralib/drivers )
	file ( GLOB QT3_DRV_H ${TE_SRC_DIR}/drivers/qt/*.h )
	file ( GLOB QT3_DRV_SRC ${TE_SRC_DIR}/drivers/qt/*.src )
	
	install (
		FILES ${QT3_DRV_H} 
		DESTINATION ${INSTALL_INCLUDE_DIR}/terralib/drivers/qt
		COMPONENT HEADER
	)
	
	if ( TEINSTALLER_SRC )
		install (
			FILES ${QT3_DRV_SRC} 
			DESTINATION ${INSTALL_SRC_DIR}/terralib/drivers/qt
			COMPONENT SOURCE
		)
	elseif ( TEINSTALLER_DEV )
		file ( GLOB ui_h_files ${terralib_BINARY_DIR}/*.h )
		install (
			FILES ${ui_h_files} 
			DESTINATION ${INSTALL_INCLUDE_DIR}/terralib/drivers/qt
			COMPONENT HEADER
		)
#		install_h_dir( ${terralib_BINARY_DIR} ${INSTALL_INCLUDE_DIR}/terralib/drivers/qt )
	endif()
endif()

if(TE_QWT3_DRIVERS_ENABLED AND (TEINSTALLER_SRC OR TE_INSTALLER_DEV))
  install_h_dir(${TE_SRC_DIR}/drivers/qwt ${INSTALL_INCLUDE_DIR}/terralib/drivers)

  if( TEINSTALLER_SRC )
    install_src_dir(${TE_SRC_DIR}/drivers/qwt ${INSTALL_SRC_DIR}/terralib/drivers)
  endif()

endif()

#if ( (BUILD_TERRAVIEW AND TEINSTALLER_DEV) OR TEINSTALLER_SRC )
	# Drivers qt e qwt
#	INSTALL ( # Headers
#	  DIRECTORY "${TE_SRC_DIR}/drivers/qwt"
#	  DIRECTORY "${TE_SRC_DIR}/drivers/qt" "${TE_SRC_DIR}/drivers/qwt"
#	  DESTINATION ${INSTALL_INCLUDE_DIR}/terralib/drivers
#	  COMPONENT TVHEADER
#	  PATTERN "*svn" EXCLUDE
#	  PATTERN "*cvs*" EXCLUDE
#	  PATTERN "*.c*" EXCLUDE
#	)
#endif()

#if ( BUILD_TERRAVIEW AND TEINSTALLER_SRC )
#	INSTALL ( # Sources
#	  DIRECTORY "${TE_SRC_DIR}/drivers/qwt"
#	  DIRECTORY "${TE_SRC_DIR}/drivers/qt" "${TE_SRC_DIR}/drivers/qwt"
#	  DESTINATION ${INSTALL_INCLUDE_DIR}/terralib/drivers
#	  COMPONENT TVSOURCE
#	  PATTERN "*svn" EXCLUDE
#	  PATTERN "*cvs*" EXCLUDE
#	  PATTERN "*.h*" EXCLUDE
#	)
#endif()

# ---------------------------------------------

#if ( (BUILD_TERRAVIEW AND TEINSTALLER_DEV) OR TEINSTALLER_SRC )
	# Drivers appUtils
#	INSTALL ( # Headers
#	  DIRECTORY "${TE_SRC_DIR}/../appUtils"
#	  DESTINATION ${INSTALL_INCLUDE_DIR}
#	  COMPONENT TVHEADER
#	  PATTERN "*svn" EXCLUDE
#	  PATTERN "*cvs*" EXCLUDE
#	  PATTERN "*.c*" EXCLUDE
#	  REGEX "/ui/" EXCLUDE
#	)
#endif()

#if ( BUILD_TERRAVIEW AND TEINSTALLER_SRC ) 
#	INSTALL ( # Sources
#	  DIRECTORY "${TE_SRC_DIR}/../appUtils"
#	  DESTINATION ${INSTALL_INCLUDE_DIR}
#	  COMPONENT TVSOURCE
#	  PATTERN "*svn" EXCLUDE
#	  PATTERN "*cvs*" EXCLUDE
#	  PATTERN "*/appUtils/*.h" EXCLUDE
#	)
#endif()
# ---------------------------------------------



# TerraManager Install
if( (BUILD_TERRAMANAGER AND TEINSTALLER_DEV) OR TEINSTALLER_SRC)
  set ( TMGER_FOLD "extensions/terramanager/src")
  set ( TMGER_FOLD_OUT "extensions/terramanager")
  install_h_dir(${TE_SRC_DIR}/../../${TMGER_FOLD} ${INSTALL_INCLUDE_DIR}/${TMGER_FOLD_OUT})

  if( TEINSTALLER_SRC )
    install_src_dir(${TE_SRC_DIR}/../../${TMGER_FOLD} ${INSTALL_SRC_DIR}/${TMGER_FOLD_OUT})
  endif()

endif()
# ---------------------------------------------

SET ( DEPENDENCIES_FOUND false)
if(${TEINSTALLER_DEPENDENCIES} MATCHES "TEINSTALLER_DEPENDENCIES-NOTFOUND")
	SET ( DEPENDENCIES_FOUND false)
else()
	SET ( DEPENDENCIES_FOUND true)
endif()
if(NOT ${DEPENDENCIES_FOUND})
	SET ( TEINSTALLER_DEPENDENCIES "TEINSTALLER_DEPENDENCIES-NOTFOUND" CACHE PATH  "Install dependencies")
	message(FATAL_ERROR "Could NOT find Install dependencies - REQUIRED")
endif()

if ( TEINSTALLER_SRC )

	#TerraView install
	install ( # ../terraView/*.h
	  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../terraView"
	  DESTINATION ${INSTALL_INCLUDE_DIR}/..
	  COMPONENT TVHEADER
	  PATTERN "*.cpp" EXCLUDE
	  PATTERN "*.project" EXCLUDE
	  PATTERN "*.cproject" EXCLUDE
	  PATTERN "*svn" EXCLUDE
	  PATTERN "*cvs*" EXCLUDE
	  PATTERN "*.qm" EXCLUDE
	  PATTERN "*.ts" EXCLUDE
	  PATTERN "*.txt" EXCLUDE
	  REGEX "settings" EXCLUDE
	  REGEX "doc" EXCLUDE
	  REGEX "windows" EXCLUDE
	  REGEX "/ui/" EXCLUDE
	)

	
endif()

file(GLOB UI_FILES ${terraViewCore_BINARY_DIR}/*.h)
file( GLOB TV_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../terraView/*.h )
file( GLOB TV_BASE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../terraViewPlugins/base/*.h )

if ( (BUILD_TERRAVIEW AND TEINSTALLER_DEV) )
	install (
	  FILES ${UI_FILES}
	  DESTINATION ${INSTALL_INCLUDE_DIR}/../terraView/include
	  COMPONENT TV_GEN_HEADER
	)
	install (
	  FILES ${TV_FILES}
	  DESTINATION ${INSTALL_INCLUDE_DIR}/../terraView
	  COMPONENT TV_GEN_HEADER
	)
	
	install (
	  FILES ${TV_BASE_FILES}
	  DESTINATION ${INSTALL_INCLUDE_DIR}/../terraView
	  COMPONENT TV_GEN_HEADER
	)

endif()

if ( TEINSTALLER_SRC )
	install ( # ../terraView/*.cpp
	  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../terraView"
	  DESTINATION ${INSTALL_SRC_DIR}/..
	  COMPONENT TVSOURCE
	  PATTERN "*.h" EXCLUDE
	  PATTERN "*.project" EXCLUDE
	  PATTERN "*.cproject" EXCLUDE
	  PATTERN "*svn" EXCLUDE
	  PATTERN "*cvs*" EXCLUDE
	  PATTERN "*.qm" EXCLUDE
	  PATTERN "*.ts" EXCLUDE
	  PATTERN "*.txt" EXCLUDE
	  REGEX "settings" EXCLUDE
	  REGEX "doc" EXCLUDE
	  REGEX "windows" EXCLUDE
	  REGEX "/ui/" EXCLUDE
	)
endif()

if ( TEINSTALLER_SRC )
	install (
	  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../terraView/ui"
	  DESTINATION ${INSTALL_INCLUDE_DIR}/../terraView
	  COMPONENT TVSOURCE
	  PATTERN "*svn" EXCLUDE
	  PATTERN "*cvs*" EXCLUDE
	)
endif()

if ( (BUILD_TERRAVIEW) OR TEINSTALLER_SRC )
	
	install (
	  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../terraView/doc"
	  DESTINATION ${INSTALL_INCLUDE_DIR}/../terraView
	  COMPONENT TVDOC
	  PATTERN "*svn" EXCLUDE
	  PATTERN "*cvs*" EXCLUDE
	)
endif()

if ( BUILD_TERRAVIEW AND TEINSTALLER_BIN )
	
	
	install (
	  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../terraView/doc"
	  DESTINATION ${INSTALL_INCLUDE_DIR}/../bin
	  COMPONENT TV_BIN_DOC
	  PATTERN "*svn" EXCLUDE
	  PATTERN "*cvs*" EXCLUDE
	)
endif()

if ( TEINSTALLER_CORTEX )

	install(
	  FILES "${CMAKE_CURRENT_BINARY_DIR}/br.org.terralib.pro"
	  DESTINATION "${INSTALL_INCLUDE_DIR}/.."
	  COMPONENT CORTEX
	) 
	
	
	install(
	  FILES "${CMAKE_CURRENT_BINARY_DIR}/extension.xml"
	  DESTINATION "${INSTALL_INCLUDE_DIR}/.."
	  COMPONENT CORTEX
	) 
	
	
endif()

if ( GENERATE_DOCUMENTATION AND ( TEINSTALLER_CORTEX OR TEINSTALLER_SRC OR TEINSTALLER_DEV ) )
	file( GLOB DOCS_HTML ${CMAKE_CURRENT_BINARY_DIR}/doc/html/*.* ) 
	file( GLOB DOCS_SEARCH ${CMAKE_CURRENT_BINARY_DIR}/doc/html/search/*.* ) 
	file( GLOB DOCS_MAN ${CMAKE_CURRENT_BINARY_DIR}/doc/man/man3/*.* ) 
	foreach( DOC_FILE ${DOCS_HTML} )
	
	install(
	  FILES ${DOC_FILE} 
	  DESTINATION ${INSTALL_DOC_DIR}/html
	  COMPONENT DOCUMENTATION
	) 	
	endforeach()
	foreach( DOC_FILE ${DOCS_SEARCH} )
	install(
	  FILES ${DOC_FILE} 
	  DESTINATION ${INSTALL_DOC_DIR}/html/search
	  COMPONENT DOCUMENTATION
	) 	
	endforeach()
	foreach( DOC_FILE ${DOCS_MAN} )
	install(
	  FILES ${DOC_FILE} 
	  DESTINATION ${INSTALL_DOC_DIR}/man/man3
	  COMPONENT DOCUMENTATION
	) 	
	endforeach()
endif()

if ( TEINSTALLER_DEV OR TEINSTALLER_BIN  )
	if( NOT UNIX )
		foreach ( dll_file ${TE_BINS} )
			install (
				FILES ${dll_file}
				DESTINATION ${INSTALL_BIN_DIR}
				COMPONENT BINARIES
			)	
		endforeach()
	endif()  
	if( TE_QT3_DRIVERS_ENABLED )
		foreach ( translation ${TE_TRANSLATIONS} )
		install (
			FILES ${translation}
			DESTINATION ${INSTALL_BIN_DIR}/translation
			COMPONENT BINARIES
		)	
	endforeach()
	endif()
	
endif(TEINSTALLER_DEV OR TEINSTALLER_BIN  )
 
 if (TEINSTALLER_DEV)

 install(
	  FILES 
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindADO.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindCrypt.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindCurl.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindFirebird.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindGdal.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindgdLib.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindGeoTIFF.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindGL.cmake"	
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindJPEGlib.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindMySQL.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindPostGIS.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindQWT.cmake"
	  "${CMAKE_CURRENT_SOURCE_DIR}/common/FindZLIB.cmake"
	  "${CMAKE_CURRENT_BINARY_DIR}/teBuildTreeSettings.cmake" 
	  "${CMAKE_CURRENT_BINARY_DIR}/teLibraryDepends.cmake" 
	  "${CMAKE_CURRENT_BINARY_DIR}/terralibConfig.cmake" 
	  "${CMAKE_CURRENT_BINARY_DIR}/terralibConfigVersion.cmake" 	  
	  DESTINATION "${INSTALL_CMAKE_DIR}"
	  COMPONENT TE_PRJ_BINARIES
	)

 endif(TEINSTALLER_DEV)
 
 if (TEINSTALLER_BIN)
	 
    if (BUILD_TERRAVIEW)
	
		 install(
		  FILES 		 
		  "${TRANSLATION_DIR_RELEASE}/terraView_pt.qm"
		  "${TRANSLATION_DIR_RELEASE}/terraView_es.qm"
		  "${CMAKE_CURRENT_SOURCE_DIR}/../../terraView/qt_pt.qm"
		  "${CMAKE_CURRENT_SOURCE_DIR}/../../terraView/qt_sp.qm"
		  DESTINATION "${INSTALL_BIN_DIR}/translation"
		  COMPONENT BINARIES
		) 
		
		install(
		 FILES "${CMAKE_SOURCE_DIR}/../../terraView/tv_color_ramps.txt"
		 DESTINATION "${INSTALL_BIN_DIR}"
		 COMPONENT BINARIES
		) 	
		
		if( TEINSTALLER_TDKPLUGIN )		

			set (result "the result must be started before the call to getLibraryForOS macro")
	  		set (pluginName "pluginTdk") 			
			getLibraryForOS (${result} ${pluginName})


			list ( APPEND PACKAGE_DEPENDS ${TDK_PACKAGE_DEPENDS})
	 
			
			file( GLOB TDK_PLUGIN  ${sigDesktop_DIR}/../Release/plugins/${result})
			foreach( DLL ${TDK_PLUGIN} )
			install(
			  FILES ${DLL} 
			  DESTINATION "${INSTALL_BIN_DIR}/plugins"
			  COMPONENT BINARIES
			) 	
			endforeach()
			
			foreach( DLL ${PLUGINS_BINARIES} )
			install(
			  FILES ${DLL} 
			  DESTINATION "${INSTALL_BIN_DIR}/plugins"
			  COMPONENT BINARIES
			) 	
			endforeach()
			file( GLOB DAT ${sigDesktop_DIR}/../Debug/plugins/*.dat )
			install(
			  FILES ${DAT} 
			  DESTINATION "${INSTALL_BIN_DIR}/plugins/tdkplugins"
			  COMPONENT BINARIES
			) 	
			
			SET (EB_Translation ${sigDesktop_DIR}/Debug/translation/basicEditionPlugin_pt_EB.qm )
			LIST(REMOVE_ITEM PLUGINS_TRANSLATIONS ${EB_Translation})

			foreach( TRANSLATION ${PLUGINS_TRANSLATIONS} )
			install(
				FILES ${TRANSLATION} 
				DESTINATION "${INSTALL_BIN_DIR}/translation"
				COMPONENT BINARIES
			) 	
			endforeach()
			
			install (
				  DIRECTORY "${sigDesktop_DIR}/Release/translation/messages"
				  DESTINATION "${INSTALL_BIN_DIR}/translation"
				  COMPONENT BINARIES
				  PATTERN "*svn" EXCLUDE
				  PATTERN "*cvs*" EXCLUDE
				)
	
			if( CMAKE_GENERATOR MATCHES "Visual Studio")
				file( GLOB PLUGINS  ${sigDesktop_DIR}/../Release/plugins/*.dll )
				
			else()
				file( GLOB PLUGINS  ${sigDesktop_DIR}/../Release/plugins/*.so )
				
			endif()
			LIST(REMOVE_ITEM PLUGINS ${TDK_PLUGIN} )
			foreach( DLL ${PLUGINS} )
			install(
			  FILES ${DLL} 
			  DESTINATION "${INSTALL_BIN_DIR}/plugins/tdkplugins"
			  COMPONENT BINARIES
			) 	
			endforeach()
		endif()
		
    endif(BUILD_TERRAVIEW)
	
	if (TEINSTALLER_PLUGINS AND BUILD_TERRAVIEW AND BUILD_TV_PLUGINS)
	
	set (result "the result must be started before the call to getLibraryForOS macro")
	set (pluginName "plugin") 
	
	set(PLUGIN_BIN_DIR bin/plugins CACHE PATH "Installation directory for plugins executables")
 
	  if (BUILD_COPYDATABASE)	
		
		set (pluginName "CopyDatabase") 
		getLibraryForOS (${result} ${pluginName})
	  
		install (
		FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
		DESTINATION ${PLUGIN_BIN_DIR}
		COMPONENT COPYDATABASE
		)	
		
		install(
		FILES "${TRANSLATION_DIR_RELEASE}/copyDatabase_es.qm" 
		DESTINATION "${INSTALL_BIN_DIR}/translation"
		COMPONENT COPYDATABASE
		) 

		install(
		FILES "${TRANSLATION_DIR_RELEASE}/copyDatabase_pt.qm"
		DESTINATION "${INSTALL_BIN_DIR}/translation"
		COMPONENT COPYDATABASE
		) 
		
	  endif(BUILD_COPYDATABASE)
	  
	  if (BUILD_EXTERNALTHEME)		
	  
	  	set (pluginName "ExternalTheme") 
		getLibraryForOS (${result} ${pluginName})
		
		install (
		FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
		DESTINATION ${PLUGIN_BIN_DIR}
		COMPONENT EXTERNALTHEME
		)			
		
		install(
		FILES "${TRANSLATION_DIR_RELEASE}/externalTheme_es.qm" 
		DESTINATION "${INSTALL_BIN_DIR}/translation"
		COMPONENT EXTERNALTHEME
		) 

		install(
		FILES "${TRANSLATION_DIR_RELEASE}/externalTheme_pt.qm"
		DESTINATION "${INSTALL_BIN_DIR}/translation"
		COMPONENT EXTERNALTHEME
		) 
		
	  endif(BUILD_EXTERNALTHEME)
	  
	    if (BUILD_FILETHEME)	

			set (pluginName "FileTheme") 
			getLibraryForOS (${result} ${pluginName})
		
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT FILETHEME
			)			
			
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/fileTheme_es.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT FILETHEME
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/fileTheme_pt.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT FILETHEME
			) 
			
	  endif(BUILD_FILETHEME)
	  
	  	if (BUILD_FILLCELL)		
		
			set (pluginName "FillCell") 
			getLibraryForOS (${result} ${pluginName})
		
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT FILLCELL
			)		
			
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/fillCell_es.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT FILLCELL
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/fillCell_pt.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT FILLCELL
			)
			
	  endif(BUILD_FILLCELL)
	  
		if (BUILD_FLOW)		
		
			set (pluginName "Flow") 
			getLibraryForOS (${result} ${pluginName})
		
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT FLOW
			)			
			
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/flow_es.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT FLOW
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/flow_pt.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT FLOW
			)
			
	  endif(BUILD_FLOW)	

	  if (BUILD_MERCADO)		
	  
			set (pluginName "Mercado") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT MERCADO
			)		
			
	  endif(BUILD_MERCADO) 

	  if (BUILD_ORACLESPATIALADAPTER)	

			set (pluginName "oracleSpatialAdapter") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT ORACLESPATIALADAPTER
			)		
			
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/oracleSpatialAdapter_es.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT ORACLESPATIALADAPTER
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/oracleSpatialAdapter_pt.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT ORACLESPATIALADAPTER
			)
			
	  endif(BUILD_ORACLESPATIALADAPTER)

	  if (BUILD_PLUGINGENERATOR)	

			set (pluginName "pluginGenerator") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT PLUGINGENERATOR
			)			
					
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/pluginGenerator_es.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT PLUGINGENERATOR
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/pluginGenerator_pt.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT PLUGINGENERATOR
			)
			
	  endif(BUILD_PLUGINGENERATOR) 

	  if (BUILD_POSTGISADAPTER)		
	  
			set (pluginName "PostGISAdapter") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT POSTGISADAPTER
			)			
					
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/postgisadapter_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT POSTGISADAPTER
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/postgisadapter_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT POSTGISADAPTER
			)
			
	  endif(BUILD_POSTGISADAPTER) 	 

	  if (BUILD_VORONOI)		
	  
	  		set (pluginName "Voronoi") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT VORONOI
			)			
					
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/voronoi_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT VORONOI
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/voronoi_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT VORONOI
			)
			
	  endif(BUILD_VORONOI)  	

 	  if (BUILD_WMSTHEME)	

	  		set (pluginName "wmsClient") 
			getLibraryForOS (${result} ${pluginName})

			
			list ( APPEND PACKAGE_DEPENDS ${TEOGC_PACKAGE_DEPS})
	 
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT WMSTHEME
			)		
		
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/wmsclient_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT WMSTHEME
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/wmsclient_es.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT WMSTHEME
			)
			
			if(UNIX)
				foreach (dll_file ${TEOGC_BINARIES})
					install (
					FILES ${dll_file}
					DESTINATION ${INSTALL_LIB_DIR}
					COMPONENT WMSTHEME
					)	
				endforeach()
			else()			
				foreach (dll_file ${TEOGC_BINARIES})
				
					install (
					FILES ${dll_file}
					DESTINATION ${INSTALL_BIN_DIR}
					COMPONENT WMSTHEME
				)	
				endforeach()
			endif()					
	  endif(BUILD_WMSTHEME)  
	  
	  if (BUILD_WFSTHEME)		
	  	  	set (pluginName "wfsClient") 
			getLibraryForOS (${result} ${pluginName})
	  
			list ( APPEND PACKAGE_DEPENDS ${TEOGC_PACKAGE_DEPS})
			
	 
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT WFSTHEME
			)			
			
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/wfsclient_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT WFSTHEME
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/wfsclient_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT WFSTHEME
			)
			
			if(UNIX)
				foreach (dll_file ${TEOGC_BINARIES})
					install (
					FILES ${dll_file}
					DESTINATION ${INSTALL_LIB_DIR}
					COMPONENT WMSTHEME
					)	
				endforeach()
			else()			
				foreach (dll_file ${TEOGC_BINARIES})
					install (
					FILES ${dll_file}
					DESTINATION ${INSTALL_BIN_DIR}
					COMPONENT WMSTHEME
				)	
				endforeach()
			endif()		
			
	  endif(BUILD_WFSTHEME)  
	  
	  if (BUILD_WCSCLIENT)		
	  
	  	  	set (pluginName "wcsClient") 
			getLibraryForOS (${result} ${pluginName})
	  
			list ( APPEND PACKAGE_DEPENDS ${TEOGC_PACKAGE_DEPS})
	 
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT WCSCLIENT
			)	
	
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/wcsClient_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT WCSCLIENT
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/wcsClient_es.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT WCSCLIENT
			)
	
			if(UNIX)
				foreach (dll_file ${TEOGC_BINARIES})
					install (
					FILES ${dll_file}
					DESTINATION ${INSTALL_LIB_DIR}
					COMPONENT WMSTHEME
					)	
				endforeach()
			else()			
				foreach (dll_file ${TEOGC_BINARIES})
					install (
					FILES ${dll_file}
					DESTINATION ${INSTALL_BIN_DIR}
					COMPONENT WMSTHEME
				)	
				endforeach()
			endif()		
			
	  endif(BUILD_WCSCLIENT)

	  if (BUILD_ATK)	

	  	  	set (pluginName "ATK") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT ATK
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/atk_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT ATK
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/atk_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT ATK
			)
						
	  endif(BUILD_ATK) 	

	  if (BUILD_BESAG)	

	  	  	set (pluginName "Besag") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT BESAG
			)	
			
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/Besag_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT BESAG
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/Besag_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT BESAG
			)		
	  endif(BUILD_BESAG)

 	  if (BUILD_BITHELL)	

	  	  	set (pluginName "Bithell") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT BITHELL
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/bithell_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT BITHELL
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/bithell_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT BITHELL
			)
			
	  endif(BUILD_BITHELL)  

	  if (BUILD_CUZICK)		
	  
	  	  	set (pluginName "Cuzick") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT CUZICK
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/cuzick_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT CUZICK
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/cuzick_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT CUZICK
			)
			
				
	  endif(BUILD_CUZICK) 

	  if (BUILD_DIGGLE)		
	  
	  	  	set (pluginName "DIggle") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT DIGGLE
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/diggle_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT DIGGLE
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/diggle_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT DIGGLE
			)
						
	  endif(BUILD_DIGGLE) 

	  if (BUILD_EBEBIN)	

			set (pluginName "EBEbin") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT EBEBIN
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/ebebin_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT EBEBIN
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/ebebin_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT EBEBIN
			)
							
	  endif(BUILD_EBEBIN) 	

	  if (BUILD_IPOP)		
	  
	  		set (pluginName "Ipop") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT IPOP
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/iPop_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT IPOP
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/iPop_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT IPOP
			)
						
	  endif(BUILD_IPOP) 	

	  if (BUILD_JACQUEZ_KNN)

	  		set (pluginName "Jacquez_kNN") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT JACQUEZ_KNN
			)	
			
			install(
			FILES "${TRANSLATION_DIR_RELEASE}/jacquez_knn_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT JACQUEZ_KNN
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/jacquez_knn_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT JACQUEZ_KNN
			)
				
	  endif(BUILD_JACQUEZ_KNN) 	  
	   
	 if (BUILD_KMLPLUGIN)		
	 
	 	  	set (pluginName "KmlPlugin") 
			getLibraryForOS (${result} ${pluginName})

			list ( APPEND PACKAGE_DEPENDS ${TEOGC_PACKAGE_DEPS})
	 
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT KMLPLUGIN
			)		

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/kmlplugin_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT KMLPLUGIN
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/kmlplugin_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT KMLPLUGIN
			)
						
	  endif(BUILD_KMLPLUGIN) 
	  
	  if (BUILD_KNOX)		
	  
	  	 	set (pluginName "Knox") 
			getLibraryForOS (${result} ${pluginName})
			
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT KNOX
			)		

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/knox_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT KNOX
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/knox_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT KNOX
			)
						
	  endif(BUILD_KNOX) 	

	  if (BUILD_MANTEL)		
	  
	  	  	set (pluginName "Mantel") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT MANTEL
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/mantel_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT MANTEL
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/mantel_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT MANTEL
			)
							
	  endif(BUILD_MANTEL) 

	  if (BUILD_OPGAM)		
	  
			set (pluginName "Opgam") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT OPGAM
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/opgam_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT OPGAM
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/opgam_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT OPGAM
			)
							
	  endif(BUILD_OPGAM) 

	  if (BUILD_SCORE)		
	  
	  		set (pluginName "Score") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT SCORE
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/score_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT SCORE
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/score_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT SCORE
			)
						
	  endif(BUILD_SCORE) 	

	  if (BUILD_SPATIALSCAN)

	  		set (pluginName "SpatialScan") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT SPATIALSCAN
			)	

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/SpatialScan_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT SPATIALSCAN
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/SpatialScan_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT SPATIALSCAN
			)
							
	  endif(BUILD_SPATIALSCAN) 	

	  if (BUILD_TANGO)	

	  		set (pluginName "Tango") 
			getLibraryForOS (${result} ${pluginName})
	  
			install (
			FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/${result}
			DESTINATION ${PLUGIN_BIN_DIR}
			COMPONENT TANGO
			)		

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/tango_pt.qm" 
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT TANGO
			) 

			install(
			FILES "${TRANSLATION_DIR_RELEASE}/tango_sp.qm"
			DESTINATION "${INSTALL_BIN_DIR}/translation"
			COMPONENT TANGO
			)
						
	  endif(BUILD_TANGO) 
	  
	endif(TEINSTALLER_PLUGINS AND BUILD_TERRAVIEW AND BUILD_TV_PLUGINS)
	
		if(CMAKE_GENERATOR MATCHES "Visual Studio")
			set ( COMMON_PATH "$ENV{T4_DEPS_DIR}/../install/common/csv" )
		else()
			set ( COMMON_PATH "${TEINSTALLER_DEPENDENCIES}/common/csv" )
		endif()
		
		install (
		DIRECTORY "${COMMON_PATH}"
		DESTINATION ${INSTALL_BIN_DIR}
		COMPONENT BINARIES
		PATTERN "*svn" EXCLUDE
		PATTERN "*cvs*" EXCLUDE
  )  
	 	
		#The icons
		install(
		  FILES 
		  "${CMAKE_SOURCE_DIR}/terralib_install_icon.ico"
		  "${CMAKE_SOURCE_DIR}/terraview_icon.ico"
		  DESTINATION "${INSTALL_BIN_DIR}"
		  COMPONENT BINARIES
		) 

		IF(WIN32)
			#Win32 dependency
			install(
			  FILES "$ENV{T4_DEPS_DIR}/../install/common/msvcr71.dll"
			  DESTINATION "${INSTALL_BIN_DIR}"
			  COMPONENT BINARIES
			) 
		endif(WIN32)
		
    if(CMAKE_GENERATOR MATCHES "Visual Studio")  
	
		install(
		  FILES "$ENV{T4_DEPS_DIR}/../install/common/vcredist_x86.exe"
		  DESTINATION "${INSTALL_BIN_DIR}"
		  COMPONENT VCREDIST
		)  
	
		 list ( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS ${CPACK_NSIS_EXTRA_INSTALL_COMMANDS} 
			"\nExecWait \\\"$INSTDIR\\\\bin\\\\vcredist_x86.exe /q /norestart\\\"" 
		  )
	 
	elseif (CMAKE_GENERATOR MATCHES "Visual Studio 10 Win64" ) 
	
		install(
		  FILES "$ENV{T4_DEPS_DIR}/../install/common/vcredist_2010_x64.exe"
		  DESTINATION "${INSTALL_BIN_DIR}"
		  COMPONENT VCREDIST
		)  
	
		list ( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS ${CPACK_NSIS_EXTRA_INSTALL_COMMANDS} 
			"\nExecWait \\\"$INSTDIR\\\\bin\\\\vcredist_2010_x64.exe /q /norestart\\\"" 
	    )
	
	endif()

	set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "TerraLib / TerraView binary instalation." )
	set ( CPACK_PACKAGE_VENDOR "www.dpi.inpe.br" )
	set ( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README" )
	set ( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/Copyright.txt" )
	set ( CPACK_PACKAGE_VERSION "4.2.2" )
	set ( CPACK_PACKAGE_VERSION_MAJOR "4" )
	set ( CPACK_PACKAGE_VERSION_MINOR "2" )
	set ( CPACK_PACKAGE_VERSION_PATCH "2" )
	set ( CPACK_NSIS_MUI_FINISHPAGE_RUN TerraView.exe )
	set ( CPACK_PACKAGE_CONTACT "frederico.bede@funcate.org.br" )
	set ( CPACK_PACKAGE_NAME "TerraView-${CPACK_PACKAGE_VERSION}" )
	set ( CPACK_PACKAGE_INSTALL_DIRECTORY "TerraView-${CPACK_PACKAGE_VERSION}" )
	set ( CPACK_PACKAGE_FILE_NAME "TerraView-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}" )
	IF(WIN32 AND NOT UNIX)
	  # There is a bug in NSI that does not handle full unix paths properly. Make
	  # sure there is at least one set of four (4) backlasshes.
	  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\terralib.bmp")
	  SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}\\\\terralib_install_icon.ico")
	  SET(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}\\\\terralib_uninstall_icon.ico")
	  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\TerraView.exe")
	  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${PKG_NAME}")
	  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.dpi.inpe.br")
	  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.funcate.org.br")
	  
	  LIST ( APPEND CPACK_NSIS_CREATE_ICONS_EXTRA ${CPACK_NSIS_CREATE_ICONS_EXTRA}
	  "\nSetOutPath \\\"$INSTDIR\\\\bin\\\""
	 "\nCreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\TerraView-${CPACK_PACKAGE_VERSION}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\TerraView.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\bin\\\\terraview_icon.ico\\\""
	 "\nCreateShortCut \\\"$DESKTOP\\\\TerraView-${CPACK_PACKAGE_VERSION}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\TerraView.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\bin\\\\terraview_icon.ico\\\""
    )

     LIST ( APPEND CPACK_NSIS_DELETE_ICONS_EXTRA ${CPACK_NSIS_DELETE_ICONS_EXTRA}
  	"\nDelete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\TerraView-${CPACK_PACKAGE_VERSION}.lnk\\\""
 	"\nDelete \\\"$DESKTOP\\\\TerraView-${CPACK_PACKAGE_VERSION}.lnk\\\""
   )
	  
	ELSE(WIN32 AND NOT UNIX)		
		list(REMOVE_DUPLICATES PACKAGE_DEPENDS)
		SET (CPACK_DEBIAN_PACKAGE_DEPENDS "")
		SET (IS_FIRST true)
		FOREACH(DEP ${PACKAGE_DEPENDS})
			if(${IS_FIRST})
				SET( CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} ${DEP}")
			else()
				SET( CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${DEP}")
			endif()
			SET (IS_FIRST false)
		ENDFOREACH()
		SET(CPACK_STRIP_FILES "bin/terraView")
		SET(CPACK_SOURCE_STRIP_FILES "") 
		SET(CPACK_SET_DESTDIR "on")
		SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
		SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
		SET(CPACK_DEBIAN_PACKAGE_SECTION "kde")
		set(CPACK_DEBIAN_PACKAGE_MAINTAINER  "DPI INPE <terralib@dpi.inpe.br>")
		SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
		SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)
		set(CPACK_DEBIAN_PACKAGE_NAME "terraview" )
		set(CPACK_DEBIAN_PACKAGE_VERSION "4.2.2" )	
		set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "TerraView is a GIS application built using the TerraLib Gis Library. TerraView main goal is to make available to the GIS Community an easy geographic data viewer with resources that include database queries and data analysis. See : <wwww.dpi.inpe.br/terraview>" )		
		install(
		 	FILES "${PROJECT_BINARY_DIR}/terraview.sh"
		 	DESTINATION "${INSTALL_BIN_DIR}"
			COMPONENT BINARIES
		) 	
	ENDIF(WIN32 AND NOT UNIX)



 endif(TEINSTALLER_BIN)
   
# Terralib instalation tree. 
set ( CPACK_COMPONENT_GROUP_TLIB_DESCRIPTION "Terralib files." )
set ( CPACK_COMPONENT_GROUP_TLIB_DISPLAY_NAME "Terralib" )

set ( CPACK_COMPONENT_HEADER_DISPLAY_NAME "C++ headers" )
set ( CPACK_COMPONENT_HEADER_DESCRIPTION "Terralib header files." )
set ( CPACK_COMPONENT_HEADER_GROUP "tlib")
set ( CPACK_COMPONENT_HEADER_INSTALL_TYPES Developer Full )

set ( CPACK_COMPONENT_SOURCE_DISPLAY_NAME "C++ sources" )
set ( CPACK_COMPONENT_SOURCE_DESCRIPTION "Terralib source files." )
set ( CPACK_COMPONENT_SOURCE_GROUP "tlib")
set ( CPACK_COMPONENT_SOURCE_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation" )
set ( CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "Terralib documentation files." )
set ( CPACK_COMPONENT_DOCUMENTATION_GROUP "tlib")
set ( CPACK_COMPONENT_DOCUMENTATION_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_BINARIES_DISPLAY_NAME "Binaries" )
set ( CPACK_COMPONENT_BINARIES_DESCRIPTION "Platform-dependent binaries." )
set ( CPACK_COMPONENT_BINARIES_GROUP "tlib")
set ( CPACK_COMPONENT_BINARIES_REQUIRED "1")

set ( CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "C++ Libraries" )
set ( CPACK_COMPONENT_LIBRARIES_DESCRIPTION "Platform-dependent libraries." )
set ( CPACK_COMPONENT_LIBRARIES_GROUP "tlib")
set ( CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Developer Full )

set ( CPACK_COMPONENT_CORTEX_DISPLAY_NAME "Import Cortex projects." )
set ( CPACK_COMPONENT_CORTEX_DESCRIPTION "Cortex files. " )
set ( CPACK_COMPONENT_CORTEX_GROUP "tlib" )
set ( CPACK_COMPONENT_CORTEX_INSTALL_TYPES Developer Full )

# TerraView instalation tree. 
set ( CPACK_COMPONENT_GROUP_TVIEW_DESCRIPTION "TerraView files." )
set ( CPACK_COMPONENT_GROUP_TVIEW_DISPLAY_NAME "TerraView" )

set ( CPACK_COMPONENT_TVHEADER_DISPLAY_NAME "C++ headers" )
set ( CPACK_COMPONENT_TVHEADER_DESCRIPTION "Terraview header files." )
set ( CPACK_COMPONENT_TVHEADER_GROUP "tview" )
set ( CPACK_COMPONENT_TVHEADER_INSTALL_TYPES Developer )

set ( CPACK_COMPONENT_TVSOURCE_DISPLAY_NAME "C++ sources" )
set ( CPACK_COMPONENT_TVSOURCE_DESCRIPTION "TerraView source files." )
set ( CPACK_COMPONENT_TVSOURCE_GROUP "tview" )
set ( CPACK_COMPONENT_TVSOURCE_INSTALL_TYPES Developer Full )

set ( CPACK_COMPONENT_TVDOC_DISPLAY_NAME "TerraView Docs." )
set ( CPACK_COMPONENT_TVDOC_DESCRIPTION "TerraView help files." )
set ( CPACK_COMPONENT_TVDOC_GROUP "tview" )
set ( CPACK_COMPONENT_TVDOC_INSTALL_TYPES Developer Full )

set ( CPACK_COMPONENT_TV_BIN_DOC_DISPLAY_NAME "TerraView Docs." )
set ( CPACK_COMPONENT_TV_BIN_DOC_DESCRIPTION "TerraView help files." )
set ( CPACK_COMPONENT_TV_BIN_DOC_GROUP "tview" )
set ( CPACK_COMPONENT_TV_BIN_DOC_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_TE_DEPENDS_DISPLAY_NAME "Terralib dll dependencies." )
set ( CPACK_COMPONENT_TE_DEPENDS_DESCRIPTION "Dependencies used to compile terraView." )
set ( CPACK_COMPONENT_TE_DEPENDS_GROUP "tlib" )
set ( CPACK_COMPONENT_TE_DEPENDS_INSTALL_TYPES Developer Full )

set ( CPACK_COMPONENT_TVBINARIES_DISPLAY_NAME "Binaries" )
set ( CPACK_COMPONENT_TVBINARIES_DESCRIPTION "Platform-dependent binaries." )
set ( CPACK_COMPONENT_TVBINARIES_GROUP "tview" )
set ( CPACK_COMPONENT_TVBINARIES_REQUIRED "1")

set ( CPACK_COMPONENT_VCREDIST_DISPLAY_NAME "Visual C++ redistributable package" )
set ( CPACK_COMPONENT_VCREDIST_DESCRIPTION " VIsual C++ redistributable package." )
set ( CPACK_COMPONENT_VCREDIST_GROUP "tview" )
set ( CPACK_COMPONENT_VCREDIST_INSTALL_TYPES Developer Full )

set ( CPACK_COMPONENT_TV_GEN_HEADER_DISPLAY_NAME "Generated .h files." )
set ( CPACK_COMPONENT_TV_GEN_HEADER_DESCRIPTION "Files generated by qt during process. " )
set ( CPACK_COMPONENT_TV_GEN_HEADER_GROUP "tview" )
set ( CPACK_COMPONENT_TV_GEN_HEADER_INSTALL_TYPES Developer Full )

set ( CPACK_COMPONENT_TE_PRJ_BINARIES_DISPLAY_NAME "Import CMake projects." )
set ( CPACK_COMPONENT_TE_PRJ_BINARIES_DESCRIPTION "CMake files to be used by outside users. " )
set ( CPACK_COMPONENT_TE_PRJ_BINARIES_GROUP "tlib" )
set ( CPACK_COMPONENT_TE_PRJ_BINARIES_INSTALL_TYPES Developer Full )

#TerraView plugins installation tree
set ( CPACK_COMPONENT_GROUP_PLUGINS_DESCRIPTION "TerraView plugins." )
set ( CPACK_COMPONENT_GROUP_PLUGINS_DISPLAY_NAME "Plugins" )

set ( CPACK_COMPONENT_COPYDATABASE_DISPLAY_NAME "Copy Database" )
set ( CPACK_COMPONENT_COPYDATABASE_DESCRIPTION "Copy Database plugin. " )
set ( CPACK_COMPONENT_COPYDATABASE_GROUP "plugins" )
set ( CPACK_COMPONENT_COPYDATABASE_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_EXTERNALTHEME_DISPLAY_NAME "External Theme" )
set ( CPACK_COMPONENT_EXTERNALTHEME_DESCRIPTION "External Theme plugin. " )
set ( CPACK_COMPONENT_EXTERNALTHEME_GROUP "plugins" )
set ( CPACK_COMPONENT_EXTERNALTHEME_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_FILETHEME_DISPLAY_NAME "File Theme" )
set ( CPACK_COMPONENT_FILETHEME_DESCRIPTION "File Theme plugin. " )
set ( CPACK_COMPONENT_FILETHEME_GROUP "plugins" )
set ( CPACK_COMPONENT_FILETHEME_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_FILLCELL_DISPLAY_NAME "FillCell" )
set ( CPACK_COMPONENT_FILLCELL_DESCRIPTION "FillCell plugin. " )
set ( CPACK_COMPONENT_FILLCELL_GROUP "plugins" )
set ( CPACK_COMPONENT_FILLCELL_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_FLOW_DISPLAY_NAME "Flow" )
set ( CPACK_COMPONENT_FLOW_DESCRIPTION "Flow plugin. " )
set ( CPACK_COMPONENT_FLOW_GROUP "plugins" )
set ( CPACK_COMPONENT_FLOW_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_KMLPLUGIN_DISPLAY_NAME "KML" )
set ( CPACK_COMPONENT_KMLPLUGIN_DESCRIPTION "KML plugin. " )
set ( CPACK_COMPONENT_KMLPLUGIN_GROUP "plugins" )
set ( CPACK_COMPONENT_KMLPLUGIN_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_MERCADO_DISPLAY_NAME "Mercado" )
set ( CPACK_COMPONENT_MERCADO_DESCRIPTION "Mercado plugin. " )
set ( CPACK_COMPONENT_MERCADO_GROUP "plugins" )
set ( CPACK_COMPONENT_MERCADO_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_ORACLESPATIALADAPTER_DISPLAY_NAME "Oracle Spatial Adapter" )
set ( CPACK_COMPONENT_ORACLESPATIALADAPTER_DESCRIPTION "Oracle Spatial Adapter plugin. " )
set ( CPACK_COMPONENT_ORACLESPATIALADAPTER_GROUP "plugins" )
set ( CPACK_COMPONENT_ORACLESPATIALADAPTER_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_PLUGINGENERATOR_DISPLAY_NAME "Plugin Generator" )
set ( CPACK_COMPONENT_PLUGINGENERATOR_DESCRIPTION "Plugin Generator plugin. " )
set ( CPACK_COMPONENT_PLUGINGENERATOR_GROUP "plugins" )
set ( CPACK_COMPONENT_PLUGINGENERATOR_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_POSTGISADAPTER_DISPLAY_NAME "PostGIS Adapter" )
set ( CPACK_COMPONENT_POSTGISADAPTER_DESCRIPTION "PostGIS Adapter plugin. " )
set ( CPACK_COMPONENT_POSTGISADAPTER_GROUP "plugins" )
set ( CPACK_COMPONENT_POSTGISADAPTER_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_VORONOI_DISPLAY_NAME "Voronoi" )
set ( CPACK_COMPONENT_VORONOI_DESCRIPTION "Voronoi plugin. " )
set ( CPACK_COMPONENT_VORONOI_GROUP "plugins" )
set ( CPACK_COMPONENT_VORONOI_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_WMSTHEME_DISPLAY_NAME "WMS Client" )
set ( CPACK_COMPONENT_WMSTHEME_DESCRIPTION "WMS Client plugin. " )
set ( CPACK_COMPONENT_WMSTHEME_GROUP "plugins" )
set ( CPACK_COMPONENT_WMSTHEME_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_WFSTHEME_DISPLAY_NAME "WFS Client" )
set ( CPACK_COMPONENT_WFSTHEME_DESCRIPTION "WFS Client plugin. " )
set ( CPACK_COMPONENT_WFSTHEME_GROUP "plugins" )
set ( CPACK_COMPONENT_WFSTHEME_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_WCSCLIENT_DISPLAY_NAME "WCS Client" )
set ( CPACK_COMPONENT_WCSCLIENT_DESCRIPTION "WCS Client plugin. " )
set ( CPACK_COMPONENT_WCSCLIENT_GROUP "plugins" )
set ( CPACK_COMPONENT_WCSCLIENT_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_ATK_DISPLAY_NAME "ATK" )
set ( CPACK_COMPONENT_ATK_DESCRIPTION "ATK plugin. " )
set ( CPACK_COMPONENT_ATK_GROUP "plugins" )
set ( CPACK_COMPONENT_ATK_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_BESAG_DISPLAY_NAME "Besag" )
set ( CPACK_COMPONENT_BESAG_DESCRIPTION "Besag plugin. " )
set ( CPACK_COMPONENT_BESAG_GROUP "plugins" )
set ( CPACK_COMPONENT_BESAG_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_BITHELL_DISPLAY_NAME "Besag" )
set ( CPACK_COMPONENT_BITHELL_DESCRIPTION "Besag plugin. " )
set ( CPACK_COMPONENT_BITHELL_GROUP "plugins" )
set ( CPACK_COMPONENT_BITHELL_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_CUZICK_DISPLAY_NAME "Cuzick" )
set ( CPACK_COMPONENT_CUZICK_DESCRIPTION "Cuzick plugin. " )
set ( CPACK_COMPONENT_CUZICK_GROUP "plugins" )
set ( CPACK_COMPONENT_CUZICK_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_DIGGLE_DISPLAY_NAME "Diggle" )
set ( CPACK_COMPONENT_DIGGLE_DESCRIPTION "Diggle plugin. " )
set ( CPACK_COMPONENT_DIGGLE_GROUP "plugins" )
set ( CPACK_COMPONENT_DIGGLE_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_EBEBIN_DISPLAY_NAME "EBEBin" )
set ( CPACK_COMPONENT_EBEBIN_DESCRIPTION "EBEBin plugin. " )
set ( CPACK_COMPONENT_EBEBIN_GROUP "plugins" )
set ( CPACK_COMPONENT_EBEBIN_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_IPOP_DISPLAY_NAME "Ipop" )
set ( CPACK_COMPONENT_IPOP_DESCRIPTION "Ipop plugin. " )
set ( CPACK_COMPONENT_IPOP_GROUP "plugins" )
set ( CPACK_COMPONENT_IPOP_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_JACQUEZ_KNN_DISPLAY_NAME "Jacquez Knn" )
set ( CPACK_COMPONENT_JACQUEZ_KNN_DESCRIPTION "Jacquez Knn plugin. " )
set ( CPACK_COMPONENT_JACQUEZ_KNN_GROUP "plugins" )
set ( CPACK_COMPONENT_JACQUEZ_KNN_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_KNOX_DISPLAY_NAME "Knox" )
set ( CPACK_COMPONENT_KNOX_DESCRIPTION "Knox plugin. " )
set ( CPACK_COMPONENT_KNOX_GROUP "plugins" )
set ( CPACK_COMPONENT_KNOX_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_MANTEL_DISPLAY_NAME "Mantel" )
set ( CPACK_COMPONENT_MANTEL_DESCRIPTION "Mantel plugin. " )
set ( CPACK_COMPONENT_MANTEL_GROUP "plugins" )
set ( CPACK_COMPONENT_MANTEL_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_OPGAM_DISPLAY_NAME "Opgam" )
set ( CPACK_COMPONENT_OPGAM_DESCRIPTION "Opgam plugin. " )
set ( CPACK_COMPONENT_OPGAM_GROUP "plugins" )
set ( CPACK_COMPONENT_OPGAM_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_SCORE_DISPLAY_NAME "Score" )
set ( CPACK_COMPONENT_SCORE_DESCRIPTION "Score plugin. " )
set ( CPACK_COMPONENT_SCORE_GROUP "plugins" )
set ( CPACK_COMPONENT_SCORE_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_SPATIALSCAN_DISPLAY_NAME "Spatial Scan" )
set ( CPACK_COMPONENT_SPATIALSCAN_DESCRIPTION "Spatial Scan plugin. " )
set ( CPACK_COMPONENT_SPATIALSCAN_GROUP "plugins" )
set ( CPACK_COMPONENT_SPATIALSCAN_INSTALL_TYPES Full )

set ( CPACK_COMPONENT_TANGO_DISPLAY_NAME "Tango" )
set ( CPACK_COMPONENT_TANGO_DESCRIPTION "Tango plugin. " )
set ( CPACK_COMPONENT_TANGO_GROUP "plugins" )
set ( CPACK_COMPONENT_TANGO_INSTALL_TYPES Full )

include (CPack)
#include (InstallRequiredSystemLibraries)
