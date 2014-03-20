# This script builds the TerraView executable
# (c) Frederico Augusto Bedê, june 2011
#
# TerraView requires TerraLib compiled with support for PDI, STAT, DTM and
# also all drivers for database available in terralib.

# Export variables
# TV_INST_DIRS - used from instalation tree
# TV_DIRS - used from build tree
# TV_LIBS - libraries 

cmake_minimum_required(VERSION 2.8)

# IMPORTANT - controls the source and build tree locations

SET(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../terraView") # change this if TerraView is somewhere else
SET(SRCDIR ${ROOT}) # this is the directory where TerraView sources will be located
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../common) # this is the directory where additional scritps are located

add_definitions(
  -DTVIEW_AS_DLL 
#  -DTLAPPUTILS_AS_DLL 
#  -DTLQWT_AS_DLL 
  ${QT_DEFINITIONS} -DQT_THREAD_SUPPORT -DUNICODE
)

SET (PROJ_NAME "terraViewCore")
project(${PROJ_NAME}) # the project name is terraView
SET(LIB_NAME ${PROJ_NAME}) # the library name is terraView
SET ( LIBS ${LIB_NAME} )



# AUXILIARY LIBRARIES NEEDED BY TERRAVIEW

# TerraView uses the following additional libraries
# REQUIRED - must be already installed for terralib to run properly
# TerraLib with DTM, PDI and STAT support enabled (see variables TE_STAT_ENABLED, TE_STAT_ENABLED, TE_STAT_ENABLED),
# and also support for DXF and al least one database driver support.
#	spl  - Simple plugin layer (http://www.unitedbytes.de/go.php?site=spl) 
#	qt3  - ????
#
# OPTIONAL - these depend on what additional data sources you want to use
#	postgresql - open source database (http://www.postgresql.org/)
#	postgis - extends PostgreSQL with vector GIS capabilities (http://postgis.refractions.net/)
#	mysql   - MySQL open source database (http://www.mysql.com/)
#

#Requires QWT qt3-compatible.
FIND_PACKAGE(QWT REQUIRED)


#Requires Qt3 multi-threaded
set(QT_MT_REQUIRED, TRUE)
FIND_PACKAGE(Qt3 REQUIRED)
FIND_PACKAGE(Gdal REQUIRED)

if( TEINSTALLER_TDKPLUGIN )
	FIND_PACKAGE ( sigDesktop REQUIRED )
	SET ( SigDesktop_DIR ${SigDesktop_DIR} PARENT_SCOPE)
	SET( PLUGINS_BINARIES ${PLUGINS_BINARIES} PARENT_SCOPE )
	SET( PLUGINS_TRANSLATIONS ${PLUGINS_TRANSLATIONS} PARENT_SCOPE )
	SET( TDK_PACKAGE_DEPENDS ${TDK_PACKAGE_DEPENDS} PARENT_SCOPE)
endif()

if(UNIX)
	list( APPEND PACKAGE_DEPENDS libqwt5-qt3 libgdal1-1.7.0 )
endif()
#IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
 # FIND_PACKAGE(GL REQUIRED)
  #IF(GL_FOUND)
   # list (APPEND TV_LIBS ${GL_LIBRARY} ${GLU_LIBRARY} )
  #ENDIF()
#ENDIF()

#Configuring qt, qwt and Tview files
#Files contained in these folders should be moved out of TerraLib source files
# -------------------------------------------------------
#SET( DRVQWT_DIR ${TE_SRC_DIR}/drivers/qwt ) 
#SET( DRVQT_DIR ${TE_SRC_DIR}/drivers/qt )
#SET( APPUTLS_DIR ${TE_SRC_DIR}/../appUtils )
#SET (IMGDIR ${ROOT}/ui/images)
# -------------------------------------------------------

if(WIN32)
	list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/terraviewCore.dll ) 
else()
	list( APPEND TE_BINS ${CMAKE_BINARY_DIR}/Release/libterraViewCore.so ) 
endif()

list ( 
  APPEND TV_DIRS
  ${QWT_INCLUDE_DIR}
#  ${DRVQWT_DIR} 
#  ${DRVQT_DIR} 
#  ${APPUTLS_DIR} 
  ${terraViewCore_BINARY_DIR} 
  ${ROOT} 
  ${ROOT}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../app_icons
  ${TE_DIRS}
  ${SPL_DIRS}
)

list ( 
  APPEND TV_INST_DIRS
#  ${QWT_INCLUDE_DIR}
#  "../src/terralib/drivers/qt"
#  "../src/terralib/drivers/qwt"
#  "../src/appUtils"
  "../terraView/include"
  "../terraView"
#  "../src/libspl"
#  "../src/terralib/drivers/spl"
)

#Configuring qtutils files
#FILE (GLOB UTLSRCS ${DRVQWT_DIR}/*.cpp ${DRVQT_DIR}/*.cpp ${APPUTLS_DIR}/*.cpp ${SRCDIR}/*.cpp )
FILE (GLOB UTLSRCS ${SRCDIR}/*.cpp )
FILE (GLOB UTLHEADERS ${SRCDIR}/*.h )
list(APPEND SRCS ${UTLSRCS} ) 
list(APPEND HEADERS ${UTLHEADERS} ) 

#These files are present but not used
#list( REMOVE_ITEM SRCS
#  ${DRVQT_DIR}/TeQtLatEdit.cpp
#  ${DRVQT_DIR}/TeQtLongEdit.cpp
#)

#Configuring files to uic
file( GLOB UI_FILES ${ROOT}/ui/*.ui )
file( GLOB H_UI_FILES ${ROOT}/ui/*.h )
QT_WRAP_UI(${PROJ_NAME} HDRS TERRAVIEW_UI_SRCS ${UI_FILES} )

#Configuring files to MOC.
#file(GLOB MOC_HDRS_UTILS ${APPUTLS_DIR}/*.h)
SET( 
  TOMOC_LIST 
  ${ROOT}/TeQtShowMedia.h
  ${ROOT}/TeQtMethods.h
  ${ROOT}/TeQtTerraStat.h
  ${SRCDIR}/terraView.h
)
QT_WRAP_CPP ( ${PROJ_NAME} MOC_SRCS ${TOMOC_LIST} )

source_group( "UI Files" FILES ${UI_FILES} ${H_UI_FILES} )	
source_group( "Generated Files\\Header" FILES ${HDRS} )	
source_group( "Generated Files\\Source" FILES ${MOC_SRCS} ${TERRAVIEW_UI_SRCS})	

list ( APPEND SRCS ${HDRS} ${MOC_SRCS}  ${UI_FILES} ${H_UI_FILES} )



#Qt3 embed image files
set (IMGDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../app_icons )
set (IMGGEN_FILE ${PROJECT_BINARY_DIR}/qimages_src.cpp)
set (IMG_COL ${CMAKE_CURRENT_BINARY_DIR}/qimages.imgcol)

generateImgCol( ${PROJ_NAME} ${IMG_COL} ${IMGDIR} ${IMGGEN_FILE} )


include_directories (${TE_INCLUDE_DIRS} ${TV_DIRS} ${QT_INCLUDE_DIR} ${CMAKE_BINARY_DIR})

add_library(
  ${LIB_NAME} SHARED 
  ${SRCS} 
  ${MOC_SRCS} 
  ${TERRAVIEW_UI_SRCS} 
  ${HEADERS} 
  ${IMGGEN_FILE}
)

if( WIN32 )
  add_definitions(-DIBPP_WINDOWS)
else()
    add_definitions(-DIBPP_LINUX)
endif()

list ( APPEND TV_LIBS ${QT_LIBRARIES} ${QWT_LIBRARY} )

set ( TLIBS terralib terralib_shp terralib_spl )

if(WIN32 AND TE_ADO_ENABLED )
	list ( APPEND TLIBS "terralib_ado" )
endif()
 
 
target_link_libraries( ${LIB_NAME} ${TV_LIBS} ${TLIBS} )

# Export variables
set ( TV_LIBS ${LIB_NAME} ${TV_LIBS} PARENT_SCOPE)
set ( TV_DIRS ${TV_DIRS} PARENT_SCOPE )
set ( TV_INST_DIRS ${TV_INST_DIRS} PARENT_SCOPE )

set ( TE_LIBS ${TE_LIBS} ${LIB_NAME} ${TV_LIBS} PARENT_SCOPE)
set ( TE_DIRS ${TE_DIRS} ${TV_DIRS} PARENT_SCOPE )
set ( TE_INST_DIRS ${TE_INST_DIRS} ${TV_INST_DIRS} PARENT_SCOPE )
set ( TE_BINS ${TE_BINS} PARENT_SCOPE )
set ( PACKAGE_DEPENDS ${PACKAGE_DEPENDS} PARENT_SCOPE )


list( APPEND TSLIST "terraView_es" "terraView_pt" )

set (PRO_FILENAME ${CMAKE_BINARY_DIR}/${PROJ_NAME}.pro )
foreach( TS ${TSLIST})
	set ( TSFILES "${TSFILES} ${SRCDIR}/${TS}.ts" )
endforeach()

generatePRO( ${PRO_FILENAME} "${HEADERS}" "${SRCS}" "${UI_FILES}" "${TSFILES}" )
generateTS( ${PRO_FILENAME} )
foreach( TS ${TSLIST})
	set ( QMFILE "${TRANSLATION_DIR_DEBUG}/${TS}.qm" )
	set ( TSFILE "${SRCDIR}/${TS}.ts" )	
	generateQM( ${TSFILE} ${QMFILE} )
	FILE ( COPY ${QMFILE} DESTINATION ${TRANSLATION_DIR_RELEASE} )
endforeach()

install(
  TARGETS terraViewCore
  EXPORT teLibraryDepends
  RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT TVBINARIES
  LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT TVBINARIES
  ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT TVBINARIES
)
