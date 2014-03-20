# This script builds the terraLib modules
# (c) Frederico A. Bedê, june 2011
#

cmake_minimum_required(VERSION 2.8)

#Instaladores
# -----------------------------------------------
OPTION ( TEINSTALLER_SRC "Make a source distribution (for user to build)?" OFF )

set ( PROJ_NAME "terraView" )
project ( ${PROJ_NAME} )

SET( CMAKE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

 

if( UNIX ) 	
	SET( EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}) 
	SET( LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}) 
else()
	SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
	SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)  

	SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
	SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)  

	SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
	SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug) 
endif()  


SET ( TRANSLATION_DIR_DEBUG  ${CMAKE_BINARY_DIR}/Debug/translation )
SET ( TRANSLATION_DIR_RELEASE ${CMAKE_BINARY_DIR}/Release/translation )

file( MAKE_DIRECTORY ${TRANSLATION_DIR_DEBUG} )
file( MAKE_DIRECTORY ${TRANSLATION_DIR_RELEASE} )

#Build the terraView
OPTION ( BUILD_TERRAVIEW "Build also terraView?" ON )

if ( NOT TEINSTALLER_SRC )
#Use a terralib outside the build tree.
	OPTION ( USE_OUT_TREE_TLIB "Uses a terralib previously installed?" OFF )
endif()

#Build terralib examples.
OPTION ( BUILD_TE_EXAMPLES "Build also the examples?" OFF )

#Build terraView plugins.
OPTION ( BUILD_TV_PLUGINS "Build also terraView plug-ins?" OFF )

#Build terraManager.
OPTION ( BUILD_TERRAMANAGER "Build also terraManager?" OFF )

OPTION ( TEINSTALLER_TDKPLUGIN "Install tdk plugin?" OFF )

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_SRC_DIR src CACHE PATH "Installation directory for source files")
set(INSTALL_CMAKE_DIR build/cmake CACHE PATH "Installation directory for CMake files")
set(INSTALL_DOC_DIR documentation CACHE PATH "Installation directory for documentation files")

set ( TE_CMAKEUTILS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/common/CMakeUtils.cmake" )


set (BUILD_FROM_TREE TRUE)
set (DIR_LIBS DIR_LIBS ${CMAKE_BINARY_DIR})

include ("${TE_CMAKEUTILS_FILE}")


OPTION ( GENERATE_DOCUMENTATION "Generate doxygen documentation" OFF ) 
if( GENERATE_DOCUMENTATION )
SET ( DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/doc CACHE PATH  "Directory to generate documentation")
SET ( PROJ_VERSION 4.2.1)
set ( ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../src/terralib )
SET ( PROJ_LOGO ${CMAKE_CURRENT_SOURCE_DIR}/terralib.bmp )
include (${CMAKE_CURRENT_SOURCE_DIR}/common/UseDoxygen.cmake )
endif()


set ( ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

if( MSVC_IDE AND ( MSVC90 OR MSVC10 ) )
  add_definitions( /MP )
endif()


if( NOT USE_OUT_TREE_TLIB AND NOT TEINSTALLER_SRC)
  
	add_subdirectory ( tlib_projs )
  
elseif ( NOT TEINSTALLER_SRC )
	set ( BUILD_FROM_TREE FALSE )
	find_package ( terralib REQUIRED COMPONENTS shp dxf dtm stat pdi spl qt3_drivers qwt3_drivers )
endif()

if( BUILD_TERRAVIEW AND (NOT TEINSTALLER_SRC) )
  list ( APPEND TE_MODULES terraViewCore )
  list ( APPEND TGT_LIBS terraViewCore )
  add_subdirectory ( terraViewCore )
  
  add_definitions( ${QT_DEFINITIONS} )

  include_directories ( ${TE_INCLUDE_DIRS} ${TV_DIRS} ${QT_INCLUDE_DIR} )
  
  
	IF (WIN32)
		SET(GUI_TYPE WIN32)
	ELSEIF (APPLE)
		# Set some Apple MacOS Specific settings
		SET(GUI_TYPE MACOSX_BUNDLE)
#	ELSEIF (UNIX)
		# Set some Unix Specific settings
#		SET(GUI_TYPE UNIX)
	ENDIF ()


  add_executable( ${PROJ_NAME} ${GUI_TYPE} ../../terraView/main.cpp)
	
  target_link_libraries(${PROJ_NAME} ${TE_LIBRARIES} ${TV_LIBS} ${QT_LIBRARIES})
  
  install(
    TARGETS ${PROJ_NAME}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT TVBINARIES
    LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT TVBINARIES
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT TVBINARIES
  )
endif()

if( BUILD_TE_EXAMPLES AND NOT TEINSTALLER_SRC )
	add_subdirectory ( examples )
endif ()

if( BUILD_TV_PLUGINS AND NOT TEINSTALLER_SRC )
	add_subdirectory ( terraViewPlugins )
endif ()

if( BUILD_TERRAMANAGER AND NOT TEINSTALLER_SRC )
	add_subdirectory ( terramanager )
endif ()

# Instalador de código fonte
# -----------------------------------------------

if ( (NOT TEINSTALLER_SRC) AND (NOT USE_OUT_TREE_TLIB) ) 

	# Add all targets to the build-tree export set
	export( TARGETS ${TGT_LIBS}
	  FILE "${PROJECT_BINARY_DIR}/teLibraryDepends.cmake")
	 
	# Export the package for use from the build-tree
	# (this registers the build-tree with a global CMake-registry)
	if(NOT BUILD_TERRAVIEW)
	  export( PACKAGE terralib )
	endif()
	
	
	# Create a FooBarBuildTreeSettings.cmake file for the use from the build tree
	STRING(REPLACE "\\" "/" TE_BINS "${TE_BINS}")
	configure_file(teBuildTreeSettings.cmake.in
	  "${PROJECT_BINARY_DIR}/teBuildTreeSettings.cmake" @ONLY)
	 
	file(RELATIVE_PATH CONF_REL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_CMAKE_DIR}"
	   "${CMAKE_INSTALL_PREFIX}/${INSTALL_INCLUDE_DIR}" )

	file(RELATIVE_PATH CONF_REL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_CMAKE_DIR}"
	   "${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}" )
	   
	configure_file(te-config.cmake.in
	  "${PROJECT_BINARY_DIR}/terralibConfig.cmake" @ONLY)
	configure_file(teConfigVersion.cmake.in
	  "${PROJECT_BINARY_DIR}/terralibConfigVersion.cmake" @ONLY)

	if(UNIX)
		configure_file(terraview.sh.in
			"${PROJECT_BINARY_DIR}/terraview.sh" @ONLY)   
	endif()
	  	
	set ( TE_INCLUDE_DIRS ${TE_DIRS} )
  
endif()

if( TEINSTALLER_SRC AND BUILD_FROM_TREE)
	message ( " -- Making src package. " )
	include ( ${CMAKE_CURRENT_SOURCE_DIR}/common/tlib_src_installer.cmake )
endif( )

# Instalador binários terralib e terraView para desenvolvedores
# -----------------------------------------------
OPTION ( TEINSTALLER_DEV "Make a development distribution (for user to use it)?" OFF )
if( TEINSTALLER_DEV )
	include ( ${CMAKE_CURRENT_SOURCE_DIR}/common/tlib_devel_installer.cmake )
endif()
if( TEINSTALLER_DEV AND NOT BUILD_FROM_TREE )
	if( NOT BUILD_TERRAVIEW )
		OPTION (TEINSTALLER_DEV_DEBUG "Install the library executables on Debug mode?" OFF)#cria uma instalação em debug das dlls da biblioteca
	endif( )
	
endif( )

# Instalador binários terralib e terraView para desenvolvedores
# -----------------------------------------------
OPTION ( TEINSTALLER_CORTEX "Make a development distribution (for user to use it)?" OFF )
if( TEINSTALLER_CORTEX AND BUILD_FROM_TREE )
	include ( ${CMAKE_CURRENT_SOURCE_DIR}/common/tlib_devel_installer.cmake )
	include ( ${CMAKE_CURRENT_SOURCE_DIR}/common/tlib_cortex_installer.cmake )
endif( )

# Instalador terraView
# -----------------------------------------------
OPTION ( TEINSTALLER_BIN "Make a terraView binary distribution (for user to use it)?" OFF )
if( TEINSTALLER_BIN AND BUILD_FROM_TREE )
endif( )

#Instalador dos plugins do TerraView

OPTION (TEINSTALLER_PLUGINS "Include the plugins with the terraview installer" OFF)

include ( ${CMAKE_CURRENT_SOURCE_DIR}/common/tlib_package.cmake )


if( WIN32 )
	set( FILENAME ${CMAKE_BINARY_DIR}/RodaSolution )
	generateBAT(${FILENAME} "${DIR_LIBS}" ${PROJ_NAME} ${CMAKE_BINARY_DIR} Debug)
	generateBAT(${FILENAME} "${DIR_LIBS}" ${PROJ_NAME} ${CMAKE_BINARY_DIR} Release)
endif()
