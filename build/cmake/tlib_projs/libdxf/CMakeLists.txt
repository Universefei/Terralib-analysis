# This script builds the libdxf library
# (c) Frederico A. Bed�, june 2011
#
# To use it, you should adjust ROOT 
# change ROOT to the directory where libdxf source is located

# Export variables
# DXF_INST_DIRS - used from instalation tree
# DXF_DIRS - used from build tree
# DXF_LIB - library name

cmake_minimum_required(VERSION 2.8)

# IMPORTANT - controls the source and build tree locations

SET( ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/dxflib") # change this if libdxf is somewhere else
SET( SRCDIR ${ROOT} ) # this is the directory where libdxf sources will be located
SET( INCLDIR ${ROOT} ) # this is the directory where libdxf header files will be located
SET( INST_INCLDIR ${ROOT} ) #This is used for locate at instalation tree.
#SET( INST_INCLDIR "../../../src/dxflib" ) #This is used for locate at instalation tree.

SET ( PROJ_NAME "terralib_dxf" )
project( ${PROJ_NAME} ) # the project name is terraView
SET( LIB_NAME ${PROJ_NAME} ) # the library name is terraView
SET ( LIBS ${LIB_NAME} )

include_directories (
  ${INCLDIR}
)

#FILE (GLOB SRCS ${SRCDIR}/*.cpp)
#FILE (GLOB HEADS ${INCLDIR}/*.h)

set ( 
	_DIRS
	.
)

# Instalation tree directories
appFPrefix( "libdxf" "${_DIRS}" DXF_INST_DIRS )

# Build tree directories
appFPrefix( "${ROOT}" "${_DIRS}" DXF_DIRS )

getFfiles ( "${ROOT}" "${_DIRS}" dxf_h_files true "" )
getFfiles ( "${ROOT}" "${_DIRS}" dxf_src_files false "" )

list ( APPEND SRCS ${dxf_h_files} ${dxf_src_files} )
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_library ( ${LIB_NAME} STATIC ${SRCS} )
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_library ( ${LIB_NAME} SHARED ${SRCS} )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")  

# Export variables
set ( DXF_DIRS ${DXF_DIRS} PARENT_SCOPE )
set ( DXF_INST_DIRS ${DXF_INST_DIRS} PARENT_SCOPE )
set ( DXF_LIB ${LIB_NAME} PARENT_SCOPE )

install(
	TARGETS ${PROJ_NAME}
	EXPORT teLibraryDepends
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT BINARIES
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
)