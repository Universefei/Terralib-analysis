# This script builds the terramanager library
# (c) Frederico A. Bed�, june 2011
#
# To use it, you should adjust ROOT 
# change ROOT to the directory where terramanager source is located

# Export variables
# TMGER_INST_DIRS - used from instalation tree
# TMGER_DIRS - used from build tree
# TMGER_LIB - library name

cmake_minimum_required(VERSION 2.8)

# IMPORTANT - controls the source and build tree locations

SET(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../extensions/terramanager") # change this if shapelib is somewhere else
SET(SRCDIR ${ROOT}/src) # this is the directory where shapelib sources will be located
SET(INCLDIR ${SRCDIR}) # this is the directory where shapelib header files will be located
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../common) # this is the directory where additional scritps are located

SET (PROJ_NAME "terramanager")
project( ${PROJ_NAME} ) # the project name is terraView
SET( LIB_NAME ${PROJ_NAME} ) # the library name is terraView
SET ( LIBS ${LIB_NAME} )

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_definitions ( -DTERRAMANAGERDLL )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

find_package ( gdLib REQUIRED )
if(UNIX)
	list( APPEND PACKAGE_DEPENDS libgd2-noxpm )
endif()

if( (NOT DEFINED TE_DIRS) OR  (NOT DEFINED TE_LIBS) ) 
	find_package ( terralib REQUIRED )
	set ( TE_DIRS ${TE_INCLUDE_DIRS} )
endif()
IF( WIN32 )
	if (TEINSTALLER_DEV_DEBUG)
		list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Debug/terramanager.dll ) 
	else(TEINSTALLER_DEV_DEBUG)
		list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/terramanager.dll )
	endif(TEINSTALLER_DEV_DEBUG)
	
	list( APPEND TE_BINS $ENV{T4_DEPS_DIR}/bin/win32-vs2010/bgd.dll )
ELSE()
	list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/terramanager.so )	
ENDIF()

include_directories (
  ${INCLDIR}
  ${TE_DIRS}
  ${GDLIB_INCLUDE_DIR}
)

FILE (GLOB SRCS ${SRCDIR}/*.cpp)
FILE (GLOB HEADS ${INCLDIR}/*.h)

add_library (
  ${LIB_NAME} SHARED ${SRCS} 
)

target_link_libraries ( ${LIB_NAME} ${GDLIB_LIBRARY} terralib )

# Export variables
list ( APPEND TE_DIRS "${INCLDIR}" )
list ( APPEND TE_INST_DIRS "extensions/terramanager/src" )
list ( APPEND TE_LIBS ${PROJ_NAME} )
list ( APPEND TGT_LIBS terramanager )
list ( APPEND TE_MODULES ${PROJ_NAME} )

set ( TMGER_DIRS ${INCLDIR} PARENT_SCOPE )
set ( TMGER_INST_DIRS "extensions/terramanager/src" PARENT_SCOPE )
set ( TMGER_LIB ${PROJ_NAME} PARENT_SCOPE )

set ( TE_DIRS ${TE_DIRS} PARENT_SCOPE)
set ( TE_BINS ${TE_BINS} PARENT_SCOPE)
set ( TE_INST_DIRS ${TE_INST_DIRS} PARENT_SCOPE)
set ( TE_LIBS ${TE_LIBS} PARENT_SCOPE)
set ( TGT_LIBS ${TGT_LIBS} PARENT_SCOPE)
set ( TE_MODULES ${TE_MODULES} PARENT_SCOPE)

SET_TARGET_PROPERTIES( ${PROJ_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "" )

install(
	TARGETS ${PROJ_NAME}
	EXPORT teLibraryDepends
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT BINARIES
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
)
