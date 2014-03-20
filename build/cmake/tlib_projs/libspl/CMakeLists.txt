# This script builds the libspl library
# (c) Frederico A. Bed�, june 2011
#
# To use it, you should adjust ROOT 
# change ROOT to the directory where libspl source is located

# Export variables
# SPL_INST_DIRS - used from instalation tree
# SPL_DIRS - used from build tree
# SPL_LIB - library name

cmake_minimum_required(VERSION 2.8)

# IMPORTANT - controls the source and build tree locations

SET(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libspl") # change this if libspl is somewhere else
SET(SRCDIR ${ROOT}/source) # this is the directory where libspl sources will be located
SET(INCLDIR ${ROOT}/include) # this is the directory where libspl header files will be located

SET (PROJ_NAME "terralib_spl")
project( ${PROJ_NAME} ) # the project name is terraView
SET( LIB_NAME ${PROJ_NAME} ) # the library name is terraView
SET ( LIBS ${LIB_NAME} )

add_definitions ( -DSPL_MAKE_DLL )

set ( 
	_DIRS
	.
)

# Instalation tree directories
appFPrefix( "libspl" "${_DIRS}" SPL_INST_DIRS )

# Build tree directories
appFPrefix( "${ROOT}" "${_DIRS}" SPL_DIRS )

getFfiles ( "${ROOT}/include" "${_DIRS}" spl_h_files true "" )
getFfiles ( "${ROOT}/source" "${_DIRS}" spl_src_files false "" )

list ( APPEND SRCS ${spl_h_files} ${spl_src_files} )

include_directories ( "${SPL_DIRS}" )

add_library ( ${LIB_NAME} SHARED ${SRCS} )

# Export variables
set ( SPL_DIRS ${SPL_DIRS} PARENT_SCOPE )
set ( SPL_INST_DIRS ${SPL_INST_DIRS} PARENT_SCOPE )
set ( SPL_LIB ${LIB_NAME} PARENT_SCOPE )

install(
	TARGETS ${PROJ_NAME}
	EXPORT teLibraryDepends
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT BINARIES
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
)