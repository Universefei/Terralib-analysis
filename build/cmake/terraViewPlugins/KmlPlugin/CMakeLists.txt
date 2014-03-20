# TerraView flow cmake project.
# (c) Paulo Renato M. Oliveira, 2011
# ----------------------------------------------------------------------------


cmake_minimum_required(VERSION 2.8)

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/plugins)  

SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/plugins)  

SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/plugins)  


set (PROJ_TVPLGS_DIR "${TVPLGS_DIR}/KmlPlugin") # change this if terralib is somewhere else

set ( PROJ_NAME "KmlPlugin" )

project ( ${PROJ_NAME} )

if( (NOT DEFINED TE_DIRS) OR  (NOT DEFINED TE_LIBS) ) 
	find_package ( terralib REQUIRED )
endif()

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../common") 

FILE ( GLOB SRCS ${PROJ_TVPLGS_DIR}/src/*.cpp )
FILE ( GLOB HEADERS ${PROJ_TVPLGS_DIR}/include/*.h )

#Qt files
#Requires Qt3 multi-threaded
set(QT_MT_REQUIRED, TRUE)
FIND_PACKAGE(Qt3 REQUIRED)
FIND_PACKAGE(terraogc REQUIRED COMPONENTS common xml )

set (TEOGC_BINARIES ${TEOGC_BINARIES} PARENT_SCOPE)
FIND_PACKAGE(Curl REQUIRED)
if(UNIX)
	list( APPEND PACKAGE_DEPENDS libcurl3)
endif()

include_directories (
	${CURL_INCLUDE_DIR}
	${TEOGC_INCLUDE_DIRS}
	${TE_DIRS}
	${TV_DIRS}
	${PROJ_TVPLGS_DIR}/include
	${QT_INCLUDE_DIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"
)

set (
	TO_MOC
	${PROJ_TVPLGS_DIR}/include/kmlplugin.h
)



file( GLOB UI_FILES ${PROJ_TVPLGS_DIR}/ui/*.ui )
file( GLOB H_UI_FILES ${PROJ_TVPLGS_DIR}/ui/*.h )
QT_WRAP_UI( ${PROJ_NAME} HDRS uiFiles ${UI_FILES} )
QT_WRAP_CPP ( ${PROJ_NAME} moc_files ${TO_MOC} )


source_group( "Generated Files\\Header" FILES ${HDRS} )
source_group( "Generated Files\\Source" FILES ${moc_files} ${uiFiles} )
source_group( "UI Files" FILES ${UI_FILES} ${H_UI_FILES} )

	
set (IMGDIR ${PROJ_TVPLGS_DIR}/ui/images )
set (IMGGEN_FILE ${PROJECT_BINARY_DIR}/qimages_src.cpp)
set (IMG_COL ${CMAKE_CURRENT_BINARY_DIR}/qimages.imgcol)

generateImgCol( ${PROJ_NAME} ${IMG_COL} ${IMGDIR} ${IMGGEN_FILE} )

list ( APPEND SRCS ${IMGGEN_FILE} )
list( APPEND TSLIST "kmlplugin_sp" "kmlplugin_pt" )

set (PRO_FILENAME ${CMAKE_BINARY_DIR}/${PROJ_NAME}.pro )
foreach( TS ${TSLIST})
	set ( TSFILES "${TSFILES} ${PROJ_TVPLGS_DIR}/${TS}.ts" )
endforeach()

generatePRO( ${PRO_FILENAME} "${HEADERS}" "${SRCS}" "${UI_FILES}" "${TSFILES}" )
generateTS( ${PRO_FILENAME} )
foreach( TS ${TSLIST})
	set ( QMFILE "${TRANSLATION_DIR_DEBUG}/${TS}.qm" )
	set ( TSFILE "${PROJ_TVPLGS_DIR}/${TS}.ts" )
	generateQM( ${TSFILE} ${QMFILE} )
	FILE ( COPY ${QMFILE} DESTINATION ${TRANSLATION_DIR_RELEASE} )
endforeach()
# -----------------------------------

add_definitions ( ${QT_DEFINITIONS} -DQT_THREAD_SUPPORT )
add_definitions ( -DTDK_KMLASDLL )

SET(PACKAGE_DEPENDS ${PACKAGE_DEPENDS} PARENT_SCOPE)

add_library ( ${PROJ_NAME} SHARED ${SRCS} ${HEADERS} ${HDRS} ${uiFiles} ${moc_files}  ${H_UI_FILES} ${UI_FILES})  

target_link_libraries ( ${PROJ_NAME} terralib terraViewCore terralib_spl ${TEOGC_LIBRARIES} ${CURL_LIBRARY} )
