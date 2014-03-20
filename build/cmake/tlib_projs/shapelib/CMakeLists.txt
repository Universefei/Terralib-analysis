# This script builds the shapelib library
# (c) Frederico A. Bed�, june 2011
#
# To use it, you should adjust ROOT 
# change ROOT to the directory where shapelib source is located

# Export variables
# SHP_INST_DIRS - used from instalation tree
# SHP_DIRS - used from build tree
# SHP_LIB - library name

cmake_minimum_required(VERSION 2.8)

# IMPORTANT - controls the source and build tree locations

SET(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/shapelib") # change this if shapelib is somewhere else
SET(SRCDIR ${ROOT}) # this is the directory where shapelib sources will be located
SET(INCLDIR ${ROOT}) # this is the directory where shapelib header files will be located

SET (PROJ_NAME "terralib_shp")
project( ${PROJ_NAME} ) # the project name is terraView
SET( LIB_NAME ${PROJ_NAME} ) # the library name is terraView
SET ( LIBS ${LIB_NAME} )

add_definitions ( -DSHAPELIB_DLLEXPORT )

#include_directories (
#  ${INCLDIR}
#)

#FILE (GLOB SRCS ${SRCDIR}/*.c)
#FILE (GLOB HEADS ${INCLDIR}/*.h)

set ( 
	_DIRS
	.
)

# Instalation tree directories
appFPrefix( "shapelib" "${_DIRS}" SHP_INST_DIRS )

# Build tree directories
appFPrefix( "${ROOT}" "${_DIRS}" SHP_DIRS )

getFfiles ( "${ROOT}" "${_DIRS}" shp_h_files true "" )
getFfiles ( "${ROOT}" "${_DIRS}" shp_src_files false "" )

list ( APPEND SRCS ${shp_h_files} ${shp_src_files} )

include_directories ( "${SHP_DIRS}" )

add_library ( ${LIB_NAME} SHARED ${SRCS} )

# Export variables
set ( SHP_DIRS ${SHP_DIRS} PARENT_SCOPE )
set ( SHP_INST_DIRS "${SHP_INST_DIRS}" PARENT_SCOPE )
set ( SHP_LIB ${PROJ_NAME} PARENT_SCOPE )

install(
	TARGETS ${PROJ_NAME}
	EXPORT teLibraryDepends
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT BINARIES
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
)
