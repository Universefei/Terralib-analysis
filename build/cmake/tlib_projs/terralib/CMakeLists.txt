# This script builds the terralib library
# (c) Gilberto Camara, june 2011
#
# To use it, you should adjust ROOT 
# change ROOT to the directory where terralib source is located
#

# Export variables
# TE_INST_DIRS - used from instalation tree
# TE_DIRS - used from build tree
# TE_LIBS - library name

cmake_minimum_required(VERSION 2.8)

# IMPORTANT - controls the source and build tree locations

SET(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../..") # change this if terralib is somewhere else
SET(SRCDIR ${ROOT}/src/terralib) # this is the directory where terralib sources will be located
SET(INCLDIR ${SRCDIR}) # this is the directory where terralib header files will be located
#SET( INST_INCLDIR ${ROOT}/src ) #This is used for locate at instalation tree.

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../common) # this is the directory where additional scritps are located

MESSAGE ("Using additional find package files located in ${CMAKE_MODULE_PATH}")

# Sets the project name
SET (PROJ_NAME "terralib")
project(${PROJ_NAME}) # the project name is terralib
SET(LIB_NAME ${PROJ_NAME}) # the library name is terralib

# TerraLib needs many other libraries - they will be linked together 
# if they are not listed, the build will fail
SET (LIBS ${LIB_NAME}) # to be used later on

#Definition need for windows compiling ( If not defined, it may generate warnings. )
add_definitions(-DTL_AS_DLL -DTLUTILS_AS_DLL -DTLFUNCTIONS_AS_DLL)

# Select the source files

#file(GLOB SRCS 
#	${SRCDIR}/kernel/*.cpp  
#	${SRCDIR}/utils/*.cpp  
#	${SRCDIR}/functions/*.cpp  
#	)

set ( _DIRS
	.
	kernel
	utils
	functions
)  

# Instalation tree directories
appFPrefix( "terralib" "${_DIRS}" CORE_INST_DIRS )
list ( APPEND TE_INST_DIRS ${CORE_INST_DIRS} )

# Build tree directories
appFPrefix( "${INCLDIR}" "${_DIRS}" CORE_DIRS )
list ( APPEND TE_DIRS ${CORE_DIRS} "${ROOT}/src")

getFfiles ( "${INCLDIR}" "${_DIRS}" core_h_files true "" )
getFfiles ( "${SRCDIR}" "${_DIRS}" core_srcs_files false "" )

list ( APPEND SRCS "${core_h_files}" "${core_srcs_files}" )
IF( WIN32 )
	if (TEINSTALLER_DEV_DEBUG)
		list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Debug/terralib.dll ) 
	else(TEINSTALLER_DEV_DEBUG)
		list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/terralib.dll )
	endif(TEINSTALLER_DEV_DEBUG)
ELSE(WIN32)
	list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/libterralib.so ) 
ENDIF(WIN32)
	
# these files are present but are not used
list(REMOVE_ITEM SRCS
  ${SRCDIR}/functions/TeImportSPR.cpp
)

# Select the include files
#include_directories(
#  	${INCLDIR}/kernel
#  	${INCLDIR}/utils
#  	${INCLDIR}/functions
#  	)

#list (APPEND TE_INST_DIRS 
#  "terralib"
#  "terralib/kernel" 
#  "terralib/utils"
#  "terralib/functions"
#)  

#list ( APPEND TE_DIRS 
#  "${ROOT}/src"
#  "${INCLDIR}" 
#  "${INCLDIR}/kernel" 
#  "${INCLDIR}/utils"
#  "${INCLDIR}/functions"
#)  
  
# TE_LIBS - library name
list (APPEND  TE_LIBS ${PROJ_NAME} )
#set (TE_MODULES "terralib")
  
#Compile DTM only if required    
OPTION (TE_DTM_ENABLED "Add DTM support?" ON)

#Compile Stat only if required    
OPTION (TE_STAT_ENABLED "Add statistics support?" ON)

#Compile PDI only if required    
OPTION (TE_PDI_ENABLED "Add Image Processing support?" ON)

#Using Shapelib of terralib
OPTION (TE_SHP_ENABLED "Add shapelib support?" ON)

#Using DXF of terralib
OPTION (TE_DXF_ENABLED "Add dxf support?" ON)

#Using DXF of terralib
OPTION (TE_SPL_ENABLED "Add spl support?" ON)

#Using ADO of terralib
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND NOT MINGW)
	OPTION (TE_ADO_ENABLED "Add ado support?" ON)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND NOT MINGW)

#Using Qt3 drivers available in terralib
OPTION (TE_QT3_DRIVERS_ENABLED "Add qt3 drivers support?" ON)

#Using Qt3 drivers available in terralib
OPTION (TE_QWT3_DRIVERS_ENABLED "Add qwt-qt3 drivers support?" ON)

if(TE_DTM_ENABLED)
	MESSAGE ("DTM enabled.")

	#Appending DTM source files.
	#  FILE (GLOB DTMSRCS ${SRCDIR}/dtm/*.cpp)
	#	list(APPEND SRCS ${DTMSRCS} )

	#Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DDTM_AS_DLL)

	set ( _DIRS
		dtm
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" DTM_INST_DIRS )
	list ( APPEND TE_INST_DIRS ${DTM_INST_DIRS} )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" DTM_DIRS )
	list ( APPEND TE_DIRS ${DTM_DIRS} )

	getFfiles ( "${INCLDIR}" "${_DIRS}" dtm_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" dtm_srcs_files false "" )

	list ( APPEND SRCS "${dtm_h_files}" "${dtm_srcs_files}" )

  #Add DTM headers path.
 # list ( APPEND TE_DIRS "${INCLDIR}/dtm" )
 # list ( APPEND TE_INST_DIRS "terralib/dtm" )
	list ( APPEND TE_MODULES "dtm")
  
else (TE_DTM_ENABLED)  
  MESSAGE ("DTM disabled.")
endif(TE_DTM_ENABLED)    
    
if(TE_STAT_ENABLED)
	MESSAGE ("Statistics enabled.")

	set ( _DIRS
		stat
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" STT_INST_DIRS )
	list ( APPEND TE_INST_DIRS ${STT_INST_DIRS} )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" STT_DIRS )
	list ( APPEND TE_DIRS ${STT_DIRS} )

	getFfiles ( "${INCLDIR}" "${_DIRS}" stt_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" stt_srcs_files false "" )

	list ( APPEND SRCS "${stt_h_files}" "${stt_srcs_files}" )
  #Appending Stat source files.
#  FILE (GLOB STATSRCS ${SRCDIR}/stat/*.cpp)
#	list(APPEND SRCS ${STATSRCS} )
  
  #File present but not used
	list(REMOVE_ITEM SRCS
		${SRCDIR}/stat/TeTreeMinimum.cpp
		${SRCDIR}/stat/mtrand.cpp
	)

	#Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DSTAT_AS_DLL)

	#Add Stat headers path.
	# list ( APPEND TE_DIRS "${INCLDIR}/stat" )
	# list ( APPEND TE_INST_DIRS "terralib/stat" )

	list ( APPEND TE_MODULES stat)

else (TE_STAT_ENABLED)  
  MESSAGE ("Statistics disabled.")
endif(TE_STAT_ENABLED)    
    
if(TE_PDI_ENABLED)
  MESSAGE ("Image processing enabled.")
  
  #Appending PDI source files.
#  FILE (GLOB PDISRCS ${SRCDIR}/image_processing/*.cpp)
#	list(APPEND SRCS ${PDISRCS} )
  
  #Definition need for windows compiling ( If not defined, it may generate warnings. )
	set ( _DIRS
		image_processing
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" PDI_INST_DIRS )
	list ( APPEND TE_INST_DIRS ${PDI_INST_DIRS} )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" PDI_DIRS )
	list ( APPEND TE_DIRS ${PDI_DIRS} )

	getFfiles ( "${INCLDIR}" "${_DIRS}" pdi_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" pdi_srcs_files false "" )

	list ( APPEND SRCS "${pdi_h_files}" "${pdi_srcs_files}" )

	add_definitions(-DPDI_AS_DLL)

  #Add PDI headers path.
#  list ( APPEND TE_DIRS "${INCLDIR}/image_processing" )
 # list ( APPEND TE_INST_DIRS "terralib/image_processing" )
  #list ( APPEND TE_MODULES pdi)
  
else (TE_PDI_ENABLED)  
  MESSAGE ("Image processing disabled.")
endif(TE_PDI_ENABLED)    

if (TE_SHP_ENABLED)


	set ( _DIRS
		drivers/shapelib
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" TESHP_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${TESHP_INST_DIRS}" "${SHP_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" TESHP_DIRS )
	list ( APPEND TE_DIRS "${TESHP_DIRS}" "${SHP_DIRS}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" shp_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" shp_srcs_files false "" )

	list ( APPEND SRCS "${shp_h_files}" "${shp_srcs_files}" )

#  list ( APPEND TE_DIRS "${INCLDIR}/drivers/shapelib" ${SHP_DIRS})
 # list ( APPEND TE_INST_DIRS "terralib/drivers/shapelib" ${SHP_INST_DIRS})
  
#Appending files of Shapelib driver.
 # FILE (GLOB SHPSRCS ${SRCDIR}/drivers/shapelib/*.cpp)
 # list(APPEND SRCS ${SHPSRCS} ) 

#These files present but not used
	list( REMOVE_ITEM SRCS
		${SRCDIR}/drivers/shapelib/TeImportDBF.cpp
		${SRCDIR}/drivers/shapelib/TeExportDBF.cpp
		${SRCDIR}/drivers/shapelib/TeImportSHP.cpp
		${SRCDIR}/drivers/shapelib/TeExportSHP.cpp
		${SRCDIR}/drivers/shapelib/TeQuerierSHP.cpp
		${SRCDIR}/drivers/shapelib/TeSTEFunctionsSHP.cpp
	)

	#Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DTLSHP_AS_DLL)
	list (APPEND TE_LIBS ${SHP_LIB} )
	list ( APPEND TE_MODULES shp)
	IF( WIN32 )
		if (TEINSTALLER_DEV_DEBUG)
			list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Debug/terralib_shp.dll ) 
		else(TEINSTALLER_DEV_DEBUG)
			list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/terralib_shp.dll ) 
		endif(TEINSTALLER_DEV_DEBUG)		
	ELSE()		
		list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/libterralib_shp.so ) 
	ENDIF()
endif( TE_SHP_ENABLED )

if( TE_DXF_ENABLED )
	set ( _DIRS
		drivers/DXF
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" TEDXF_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${TEDXF_INST_DIRS}" "${DXF_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" TEDXF_DIRS )
	list ( APPEND TE_DIRS "${TEDXF_DIRS}" "${DXF_DIRS}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" dxf_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" dxf_srcs_files false "" )

	list ( APPEND SRCS "${dxf_h_files}" "${dxf_srcs_files}" )
#  list ( APPEND TE_DIRS "${INCLDIR}/drivers/DXF" ${DXF_DIRS})
 # list ( APPEND TE_INST_DIRS "terralib/drivers/DXF" ${DXF_INST_DIRS})

  #Appending files of DXF driver.
#  FILE (GLOB DXFSRCS ${SRCDIR}/drivers/DXF/*.cpp)
#	list(APPEND SRCS ${DXFSRCS} ) 

  #Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DTEDXF_EXPORTS)

	list ( APPEND TE_LIBS ${DXF_LIB} )

	list ( APPEND TE_MODULES dxf)
	
endif( TE_DXF_ENABLED )

if( TE_SPL_ENABLED )
	set ( _DIRS
		drivers/spl
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" TESPL_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${TESPL_INST_DIRS}" "${SPL_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" TESPL_DIRS )
	list ( APPEND TE_DIRS "${TESPL_DIRS}" "${SPL_DIRS}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" spl_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" spl_srcs_files false "" )
	
	list ( APPEND SRCS "${spl_h_files}" "${spl_srcs_files}" )

	#	list ( APPEND TE_DIRS "${INCLDIR}/drivers/spl" ${SPL_DIRS})
#	list ( APPEND TE_INST_DIRS "terralib/drivers/spl" ${SPL_INST_DIRS})

	#Appending files of SPL driver.
#	FILE (GLOB SPL_SRCS ${SRCDIR}/drivers/spl/*.cpp)
#	list ( APPEND SRCS ${SPL_SRCS} )

	#Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions( -DTLSPL_AS_DLL )

	list ( APPEND TE_LIBS ${SPL_LIB} )
	IF( WIN32 )	
		if (TEINSTALLER_DEV_DEBUG)
			list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Debug/terralib_spl.dll ) 
		else(TEINSTALLER_DEV_DEBUG)
			list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/terralib_spl.dll )
		endif(TEINSTALLER_DEV_DEBUG)
	ELSE()		
		list ( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/libterralib_spl.so )
	ENDIF()
	list ( APPEND TE_MODULES spl )
endif()


if ( TE_QT3_DRIVERS_ENABLED )
	IF( WIN32 )
		FILE ( GLOB DLL $ENV{QTDIR}/bin/qaxcontainer.dll $ENV{QTDIR}/bin/qt-*.dll )
		list ( APPEND TE_BINS ${DLL} )
	ENDIF()
	include (${CMAKE_CURRENT_SOURCE_DIR}/tlibQt3.cmake)
	list ( APPEND TE_TRANSLATIONS ${ROOT}/terraView/qt_pt.qm ${ROOT}/terraView/qt_sp.qm )
endif()

if ( TE_QWT3_DRIVERS_ENABLED )
	
	IF( WIN32 )
		if( ${QWT_LIBRARY} MATCHES "3.3.8") 
			list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/qwt_qt-3.3.8/qwt500.dll )
		else()
			list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/qwt_qt-3.2.0/qwt500.dll )
		endif()
	ENDIF()
	include (${CMAKE_CURRENT_SOURCE_DIR}/tlibQwtQt3.cmake)
endif()

# AUXILIARY LIBRARIES NEEDED BY TERRALIB

# TerraLib uses the following additional libraries
# REQUIRED - must be already installed for terralib to run properly
# libtiff - reads TIFF and GeoTIFF files (http://www.libtiff.org/)
#	libjpeg - reads JPEG files (http://www.ijg.org)
#	zlib - compression utility (http://zlib.net/)
#	shapelib - reads ESRI's shapefiles (http://shapelib.maptools.org/)
#
# OPTIONAL - these depend on what additional data sources you want to use
#	postgresql - open source database (http://www.postgresql.org/)
#	postgis - extends PostgreSQL with vector GIS capabilities (http://postgis.refractions.net/)
#	mysql   - MySQL open source database (http://www.mysql.com/)
#	gdal/ogr - library for raster/vectorial geospatial data formats (http://www.gdal.org/)
#

SET (CMAKE_FIND_FRAMEWORK "LAST")
SET (CMAKE_FIND_APPBUNDLE "NEVER")

OPTION( SQLite_ENABLED "Add SQLite support?" ON )
if ( SQLite_ENABLED ) 

	set ( _DIRS
		drivers/SQLite
		drivers/SQLite/sqlite3
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" SQL_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${SQL_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" SQL_DIRS )
	list ( APPEND TE_DIRS "${SQL_DIRS}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" sql_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" sql_srcs_files false "" )
	
	list ( APPEND SRCS "${sql_h_files}" "${sql_srcs_files}" )
#  list ( APPEND TE_DIRS "${INCLDIR}/drivers/SQLite" )
 # list ( APPEND TE_DIRS "${INCLDIR}/drivers/SQLite/sqlite3" )
 # list ( APPEND TE_INST_DIRS "terralib/drivers/SQLite" )
 # list ( APPEND TE_INST_DIRS "terralib/drivers/SQLite/sqlite3" )  
    
  #FILE (GLOB SQLiteQSRCS ${SRCDIR}/drivers/SQLite/*.cpp ${SRCDIR}/drivers/SQLite/sqlite3/*.c )
 # list(APPEND SRCS ${SQLiteQSRCS} )  
	add_definitions( -DTLSQLITE_AS_DLL -DSQLITE_ENABLE_COLUMN_METADATA -DSQLITE_DEFAULT_FOREIGN_KEYS=1 -DSQLITE_ENABLE_STAT3)

	set (SQLite_ENABLED TRUE PARENT_SCOPE)
	list ( APPEND TE_MODULES SQLite)
endif()

#MySQL 
#set (TE_MYSQL_ENABLED FALSE PARENT_SCOPE)
FIND_PACKAGE(MySQL)
if( MySQL_FOUND )
	MESSAGE (" -- found mySQL library ${MySQL_LIBRARY} and include dir ${MySQL_INCLUDE_DIR}")
#  include_directories ( ${MySQL_INCLUDE_DIR} )
 # list ( APPEND TE_DIRS "${INCLDIR}/drivers/MySQL" )
  #list ( APPEND TE_INST_DIRS "terralib/drivers/MySQL" )
	if(UNIX)
		list( APPEND PACKAGE_DEPENDS libmysqlclient18 )
	endif()
	set ( _DIRS
		drivers/MySQL
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" MSQL_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${MSQL_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" MSQL_DIRS )
	list ( APPEND TE_DIRS "${MSQL_DIRS}" "${MySQL_INCLUDE_DIR}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" msql_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" msql_srcs_files false "" )
	
	list ( APPEND SRCS "${msql_h_files}" "${msql_srcs_files}" )

	list ( APPEND TE_LIBS ${MySQL_LIBRARY} )
	
    
  #Appending files of MySQL driver.
#  FILE (GLOB MSQSRCS ${SRCDIR}/drivers/MySQL/*.cpp)
#	list(APPEND SRCS ${MSQSRCS} )

  #Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DTLMYSQL_AS_DLL)

	set (TE_MYSQL_ENABLED TRUE PARENT_SCOPE)

	list ( APPEND TE_MODULES mysql)
	IF( CMAKE_GENERATOR MATCHES "Visual Studio 10" OR CMAKE_GENERATOR MATCHES "Visual Studio 8 2005")
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/libmysql.dll ) 
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 10 Win64")
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win64-vs2010/libmysql.dll ) 
	ENDIF()
  
endif( MySQL_FOUND )

#PostgreSQL 
#Have you installed PostgreSQL? please update the following line
#set (TE_PGRES_ENABLED FALSE PARENT_SCOPE)
FIND_PACKAGE(PostGIS)
IF( PostGIS_FOUND )
	MESSAGE (" -- found PostGIS libraries ${PostgreSQL_LIBRARY} and include dir ${PostGIS_INCLUDE_DIR}")
	if(UNIX)
		list( APPEND PACKAGE_DEPENDS postgresql-client)
	endif()
#	include_directories( ${PostGIS_INCLUDE_DIR} )

 # list ( APPEND TE_DIRS "${INCLDIR}/drivers/PostgreSQL" )
  #list ( APPEND TE_INST_DIRS "terralib/drivers/PostgreSQL" )

	set ( _DIRS
		drivers/PostgreSQL
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" PSQL_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${PSQL_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" PSQL_DIRS )
	list ( APPEND TE_DIRS "${PSQL_DIRS}" "${PostGIS_INCLUDE_DIR}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" psql_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" psql_srcs_files false "" )
	
	list ( APPEND SRCS "${psql_h_files}" "${psql_srcs_files}" )

	list ( APPEND TE_LIBS ${PGres_LIBS} )

  #Appending files of PostgreSQL driver.
#  FILE (GLOB PGSRCS ${SRCDIR}/drivers/PostgreSQL/*.cpp)
#	list(APPEND SRCS ${PGSRCS} )

  #Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DTLPOSTGRESQL_AS_DLL)

	set (TE_PGRES_ENABLED TRUE PARENT_SCOPE)

	list ( APPEND TE_MODULES postgres)
	IF( CMAKE_GENERATOR MATCHES "Visual Studio 10" OR CMAKE_GENERATOR MATCHES "Visual Studio 8 2005") 
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/libpq.dll 
		$ENV{T4_DEPS_DIR}/bin/win32-vs2010/intl.dll
		$ENV{T4_DEPS_DIR}/bin/win32-vs2010/libintl-2.dll
		$ENV{T4_DEPS_DIR}/bin/win32-vs2010/libiconv-2.dll
		$ENV{T4_DEPS_DIR}/bin/win32-vs2010/ssleay32.dll
		$ENV{T4_DEPS_DIR}/bin/win32-vs2010/libeay32.dll
		) 
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 10 Win64")
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win64-vs2010/libpq.dll 
		$ENV{T4_DEPS_DIR}/bin/win64-vs2010/libintl.dll
		$ENV{T4_DEPS_DIR}/bin/win64-vs2010/libintl-8.dll
		$ENV{T4_DEPS_DIR}/bin/win64-vs2010/libiconv.dll	
		$ENV{T4_DEPS_DIR}/bin/win64-vs2010/ssleay32.dll
		$ENV{T4_DEPS_DIR}/bin/win64-vs2010/libeay32.dll 
		) 
	ENDIF()
ENDIF( PostGIS_FOUND )

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list (APPEND TE_LIBS wsock32)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#Oracle
#Have you installed Oracle? please update the following line
set (TE_ORACLE_ENABLED FALSE PARENT_SCOPE)
FIND_PACKAGE(Oracle)
IF( ORACLE_FOUND )
	MESSAGE (" -- found Oracle libraries ${ORACLE_LIBS} and include dir ${ORACLE_INCLUDE_DIR}")
#	include_directories( ${ORACLE_INCLUDE_DIR} )

 # list ( APPEND TE_DIRS "${INCLDIR}/drivers/Oracle" ${ORACLE_INCLUDE_DIR})
  #list ( APPEND TE_INST_DIRS "terralib/drivers/Oracle" )# ${ORACLE_INCLUDE_DIR})

	set ( _DIRS
		drivers/Oracle
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" ORA_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${ORA_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" ORA_DIRS )
	list ( APPEND TE_DIRS "${ORA_DIRS}" "${ORACLE_INCLUDE_DIR}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" ora_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" ora_srcs_files false "" )
	
	list ( APPEND SRCS "${ora_h_files}" "${ora_srcs_files}" )
	
	list ( APPEND TE_LIBS ${ORACLE_LIBS} )
	
  #Appending files of Oracle driver.
#  FILE (GLOB ORASRCS ${SRCDIR}/drivers/Oracle/*.cpp)
#	list(APPEND SRCS ${ORASRCS} )

#Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DTLORACLE_AS_DLL)

	set (TE_ORACLE_ENABLED TRUE PARENT_SCOPE)
	list ( APPEND TE_MODULES oracle)
	IF( WIN32 )
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/oci.dll ) 
	ENDIF()
ENDIF( ORACLE_FOUND )

#Firebird 
FIND_PACKAGE(Firebird)
if( FBIRD_FOUND )
	MESSAGE (" -- found Firebird libraries ${FBIRD_LIBRARY}" )
	if(UNIX)
		list( APPEND PACKAGE_DEPENDS libfbclient2)
	endif()
#  list ( APPEND TE_DIRS "${INCLDIR}/drivers/Firebird" "${FBIRD_INC_DIRS}" )
#  list ( APPEND TE_INST_DIRS "terralib/drivers/Firebird" )#"${FBIRD_INC_DIRS}")
#	include_directories(${INCLDIR}/drivers/Firebird ${INCLDIR}/drivers/Firebird/ibpp/core )

	set ( _DIRS
		drivers/Firebird
		drivers/Firebird/ibpp/core
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" FBIRD_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${FBIRD_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" FBIRD_DIRS )
	list ( APPEND TE_DIRS "${FBIRD_DIRS}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" fb_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" fb_srcs_files false "" )
	
	list ( APPEND SRCS "${fb_h_files}" "${fb_srcs_files}" )

	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		add_definitions(-DIBPP_WINDOWS)
	ELSE (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

		IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			SET (LIBS ${LIBS} ${ICUDATA_LIBRARY} ${ICUUC_LIBRARY} ${ICUI18N_LIBRARY} )
			list (APPEND TE_LIBS ${FBIRD_LIBRARY})
			add_definitions(-DIBPP_LINUX)
		ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")  

	list (REMOVE_ITEM SRCS ${SRCDIR}/drivers/Firebird/ibpp/core/all_in_one.cpp )
  #Appending files of Firebird driver.
 # FILE (GLOB FBSRCS ${SRCDIR}/drivers/Firebird/*.cpp ${SRCDIR}/drivers/Firebird/ibpp/core/all_in_one.cpp )
	#list(APPEND SRCS ${FBSRCS} )

  #Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DTLFIREBIRD_AS_DLL)

	set (TE_FBIRD_ENABLED TRUE PARENT_SCOPE)
	list ( APPEND TE_MODULES firebird)
	IF( CMAKE_GENERATOR MATCHES "Visual Studio 10" OR CMAKE_GENERATOR MATCHES "Visual Studio 8 2005" )
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/fbclient.dll ) 
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 10 Win64")
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win64-vs2010/fbclient.dll ) 
	ENDIF()
endif( FBIRD_FOUND )

#TIFF
FIND_PACKAGE( GeoTIFF REQUIRED )
if( GeoTIFF_FOUND )
   	MESSAGE (" -- found geotiff library ${GeoTIFF_LIBRARY} ${TIFF_LIBRARY} and include_dir ${GeoTIFF_INCLUDE_DIR}")
	if(UNIX)
		list( APPEND PACKAGE_DEPENDS libgeotiff2)
	endif()
   	include_directories(${TIFF_INCLUDE_DIR} ${GeoTIFF_INCLUDE_DIR} ${GeoTIFF_INCLUDE_DIR}/libxtiff)
    
   	list (APPEND TE_LIBS ${GeoTIFF_LIBRARY} ${TIFF_LIBRARY})
	IF( CMAKE_GENERATOR MATCHES "Visual Studio 10" OR CMAKE_GENERATOR MATCHES "Visual Studio 8 2005")
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/geotiff.dll $ENV{T4_DEPS_DIR}/bin/win32-vs2010/libtiff.dll ) 
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 10 Win64")
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win64-vs2010/geotiff.dll $ENV{T4_DEPS_DIR}/bin/win64-vs2010/libtiff.dll ) 
	ENDIF()
endif(GeoTIFF_FOUND )

#JPEG
FIND_PACKAGE(JPEGlib REQUIRED)
if( JPEG_FOUND )
	MESSAGE (" -- found jpeg library ${JPEG_LIBRARY} and include_dir ${JPEG_INCLUDE_DIR} ")
	if(UNIX)
		list( APPEND PACKAGE_DEPENDS libjpeg8)
	endif()
	set ( _DIRS
		drivers/libjpeg
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" JPG_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${JPG_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" JPG_DIRS )
	list ( APPEND TE_DIRS "${JPG_DIRS}" "${JPEG_INCLUDE_DIR}")

	getFfiles ( "${INCLDIR}" "${_DIRS}" jpg_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" jpg_srcs_files false "" )
	
	list ( APPEND SRCS "${jpg_h_files}" "${jpg_srcs_files}" )
#  include_directories(${JPEG_INCLUDE_DIR})

#  list ( APPEND TE_DIRS "${INCLDIR}/drivers/jpeg" )
 # list ( APPEND TE_INST_DIRS "terralib/drivers/jpeg" )
 
	list ( APPEND TE_LIBS ${JPEG_LIBRARY})
    
  #Appending files of JPEG driver.
#  FILE (GLOB JPGSRCS ${SRCDIR}/drivers/libjpeg/*.cpp ${SRCDIR}/drivers/libjpeg/*.c)
#	list(APPEND SRCS ${JPGSRCS} ) 

endif(JPEG_FOUND )

#ZLIB
FIND_PACKAGE(ZLIB REQUIRED)
if( ZLIB_FOUND )
	MESSAGE (" -- found zlib library ${ZLIB_LIBRARIES} and include_dir ${ZLIB_INCLUDE_DIR}")
	if(UNIX)
		list( APPEND PACKAGE_DEPENDS libzip2)
	endif()
	include_directories( ${ZLIB_INCLUDE_DIR} )

  list (APPEND TE_LIBS ${ZLIB_LIBRARIES})
  set (TE_ZLIB_ENABLED TRUE PARENT_SCOPE) 
endif(ZLIB_FOUND )

# GDAL
FIND_PACKAGE(GDAL)
if( GDAL_FOUND )
	MESSAGE (" -- found gdal/ogr library ${GDAL_LIBRARY} and include dirs ${GDAL_CORE_INCLUDE_DIR} ${GDAL_OGR_INCLUDE_DIR} ${GDAL_OGR_FRMTS_INCLUDE_DIR} ${GDAL_PORT_INCLUDE_DIR}")
	
	if(UNIX)
		list( APPEND PACKAGE_DEPENDS libgdal1-1.7.0)
	endif()

	include_directories ( ${GDAL_CORE_INCLUDE_DIR} )
	include_directories ( ${GDAL_OGR_INCLUDE_DIR} )
	include_directories ( ${GDAL_OGR_FRMTS_INCLUDE_DIR} )
	include_directories ( ${GDAL_PORT_INCLUDE_DIR} )
  
	set ( _DIRS
		drivers/gdal
	)  

	# Instalation tree directories
	appFPrefix( "terralib" "${_DIRS}" GDAL_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${GDAL_INST_DIRS}" )

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" GDAL_DIRS )
	list ( APPEND TE_DIRS "${GDAL_DIRS}" "${GDAL_INCLUDE_DIR}")

	getFfiles ( "${INCLDIR}" "${_DIRS}" gdal_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" gdal_srcs_files false "" )
	
	list ( APPEND SRCS "${gdal_h_files}" "${gdal_srcs_files}" )

	list ( APPEND TE_LIBS ${GDAL_LIBRARY} )
	
	#Definition need for windows compiling ( If not defined, it may generate warnings. )
	add_definitions(-DTEGDAL_EXPORTS)
  
	set (TE_GDAL_ENABLED TRUE )
	set (TE_GDAL_ENABLED ${TE_GDAL_ENABLED} PARENT_SCOPE)
	list ( APPEND TE_MODULES gdal)
	IF( WIN32 )
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/gdal19.dll )
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/xerces-c_3_1.dll )
		list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/libexpat.dll )
	ENDIF()
endif( GDAL_FOUND )

# Adicionado em razão de alguns arquivos fazerem caminho relativo de terralib/kernel
list ( APPEND TE_DIRS "${ROOT}/src" ${CMAKE_BINARY_DIR})

#Writing the config.h file
MESSAGE (" -- Writing teConfig.h file. -- ")
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/teConfig.h)

include_directories ( ${TE_DIRS} ${CMAKE_BINARY_DIR} )

add_library(${LIB_NAME} SHARED ${SRCS} )

TARGET_LINK_LIBRARIES( ${TE_LIBS} )

# Export variables
set (TE_LIBS ${TE_LIBS} PARENT_SCOPE)
set (TE_DIRS ${TE_DIRS} PARENT_SCOPE)
set (TE_BINS ${TE_BINS} PARENT_SCOPE)
set (TE_TRANSLATIONS ${TE_TRANSLATIONS} PARENT_SCOPE)
set (TE_INST_DIRS ${TE_INST_DIRS} PARENT_SCOPE)
set (TE_MODULES ${TE_MODULES} PARENT_SCOPE )
set ( PACKAGE_DEPENDS ${PACKAGE_DEPENDS} PARENT_SCOPE)

SET_TARGET_PROPERTIES( ${PROJ_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "" )

install(
	TARGETS ${PROJ_NAME}
	EXPORT teLibraryDepends
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT BINARIES
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
)
