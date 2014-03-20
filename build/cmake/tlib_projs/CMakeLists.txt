# This script builds the terraLib modules
# This script builds the terraLib modules
# (c) Frederico A. Bedê, june 2011

# Export variables
# TE_INST_DIRS - used from instalation tree
# TE_DIRS - used from build tree
# TE_SRC_DIR - used from build tree
# TE_LIBS - library name
# SPL_LIB - spl library name
# TE_MYSQL_ENABLED Support to MySQL enabling
# TE_PGRES_ENABLED Support to PostgreSQL / PostGIS enabling
# TE_ORACLE_ENABLED Support to Oracle /Oracle Spatial enabling
# TE_FBIRD_ENABLED Support to Firebird enabling
# TE_ADO_ENABLED Support to ado (Access, SQL Server) enabling
# TE_GDAL_ENABLED Support GDAL/OGR
# TGT_LIBS TerraLib libs builded.

cmake_minimum_required(VERSION 2.8)

project( tlib_proj )
 
set( TLIB_PROJ_MAJOR_VERSION 4)
set( TLIB_PROJ_MINOR_VERSION 2)
set( TLIB_PROJ_PATCH_VERSION 0)
set( TLIB_PROJ_VERSION
  ${TLIB_PROJ_MAJOR_VERSION}.${TLIB_PROJ_MINOR_VERSION}.${TLIB_PROJ_PATCH_VERSION})
 
# Offer the user the choice of overriding the installation directories
# set up include-directories
include_directories(
  "${PROJECT_SOURCE_DIR}"   # to find foo/foo.h
  "${PROJECT_BINARY_DIR}")  # to find foo/config.h
 
if(TE_DXF_ENABLED)
  add_subdirectory( libdxf )

  list (APPEND TGT_LIBS terralib_dxf )

  MESSAGE ( "DXF enabled." )
  
endif(TE_DXF_ENABLED)
  
if(TE_SPL_ENABLED)

  add_subdirectory( libspl )

  list (APPEND TGT_LIBS terralib_spl )

  MESSAGE ( "SPL enabled." )
  
endif(TE_SPL_ENABLED)

if(TE_SHP_ENABLED)
  add_subdirectory( shapelib )

  list (APPEND TGT_LIBS terralib_shp )
  
  MESSAGE ( "Shapelib enabled." )
  
endif(TE_SHP_ENABLED)



if(TE_ADO_ENABLED)
	SET( ADO_FOUND TRUE)
else()
	SET( ADO_FOUND FALSE )
endif(TE_ADO_ENABLED)

add_subdirectory( terralib )

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND NOT MINGW)
	if(TE_ADO_ENABLED)	
		add_subdirectory( ado )

		list (APPEND TGT_LIBS terralib_ado )
			if (TEINSTALLER_DEV_DEBUG)
		list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Debug/terralib_ado.dll ) 
	else(TEINSTALLER_DEV_DEBUG)
			list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/terralib_ado.dll ) 
	endif(TEINSTALLER_DEV_DEBUG)
	
		MESSAGE ( "ADO enabled." )
		list( APPEND TE_BINS 
			$ENV{T4_DEPS_DIR}/bin/win32-vs2010/libeay32.dll 
			$ENV{T4_DEPS_DIR}/bin/win32-vs2010/ssleay32.dll 
			$ENV{T4_DEPS_DIR}/bin/win32-vs2010/oledb32.dll
			$ENV{T4_DEPS_DIR}/bin/win32-vs2010/iconv.dll
			$ENV{T4_DEPS_DIR}/bin/win32-vs2010/msado26.tlb
			$ENV{T4_DEPS_DIR}/bin/win32-vs2010/msadox.dll
		)  
		
		
		set (TE_ADO_ENABLED TRUE PARENT_SCOPE)
		list ( APPEND TE_MODULES ado)
		SET( ADO_FOUND TRUE)		
			
		list ( APPEND TE_DIRS ${ADO_DIRS})		
		list ( APPEND TE_INST_DIRS ${ADO_INST_DIRS}  ) 	
	else()
		SET( ADO_FOUND FALSE )
	endif(TE_ADO_ENABLED)

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND NOT MINGW)


list (APPEND TGT_LIBS terralib )

# Export variables
set ( TE_INST_DIRS ${TE_INST_DIRS} PARENT_SCOPE )  
set ( DIR_LIBS ${DIR_LIBS} PARENT_SCOPE )  
set ( TE_DIRS ${TE_DIRS} PARENT_SCOPE )  
set ( TE_BINS ${TE_BINS} PARENT_SCOPE )  
set ( TE_TRANSLATIONS ${TE_TRANSLATIONS} PARENT_SCOPE )  
set ( TE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/terralib" PARENT_SCOPE )  
set ( TE_LIBS ${TE_LIBS} PARENT_SCOPE )  
set ( SPL_LIB ${SPL_LIB} PARENT_SCOPE )  
set ( SPL_DIRS ${SPL_DIRS} PARENT_SCOPE )
set ( ADO_LIB ${ADO_LIB} PARENT_SCOPE )  
set ( ADO_DIRS ${ADO_DIRS} PARENT_SCOPE )
set ( ADO_FOUND ${ADO_FOUND} PARENT_SCOPE )
set ( SQLite_ENABLED ${TE_MYSQL_ENABLED} PARENT_SCOPE)
set ( TE_ZLIB_ENABLED ${TE_ZLIB_ENABLED} PARENT_SCOPE)
set ( TE_MYSQL_ENABLED ${TE_MYSQL_ENABLED} PARENT_SCOPE)
set ( TE_PGRES_ENABLED ${TE_PGRES_ENABLED} PARENT_SCOPE)
set ( TE_ORACLE_ENABLED ${TE_ORACLE_ENABLED} PARENT_SCOPE)
set ( TE_FBIRD_ENABLED ${TE_FBIRD_ENABLED} PARENT_SCOPE)
set ( TE_ADO_ENABLED ${TE_ADO_ENABLED} PARENT_SCOPE)
set ( TE_GDAL_ENABLED ${TE_GDAL_ENABLED} PARENT_SCOPE)
set ( TGT_LIBS ${TGT_LIBS} PARENT_SCOPE)
set ( TE_MODULES ${TE_MODULES} PARENT_SCOPE )
set ( PACKAGE_DEPENDS ${PACKAGE_DEPENDS} PARENT_SCOPE)

