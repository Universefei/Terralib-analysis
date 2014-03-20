# TerraView KFunctions cmake project.
# (c) Paulo Renato M. Oliveira, 2011
# ----------------------------------------------------------------------------


cmake_minimum_required(VERSION 2.8)

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/plugins)  

SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/plugins)  

SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/plugins)  


set (PROJ_TVPLGS_DIR "${TVPLGS_DIR}/SpatialStat/KFunctions") # change this if terralib is somewhere else

set ( PROJ_NAME "KFunctions" )

project ( ${PROJ_NAME} )

if( (NOT DEFINED TE_DIRS) OR  (NOT DEFINED TE_LIBS) ) 
	find_package ( terralib REQUIRED )
endif()

include_directories (
	${TE_DIRS}
	${TV_DIRS}
	${PROJ_TVPLGS_DIR}/include
	${PROJ_TVPLGS_DIR}/ui
	${QT_INCLUDE_DIRS}
	"${CMAKE_BINARY_DIR}"
)

FILE ( GLOB SRCS ${PROJ_TVPLGS_DIR}/src/*.cpp )
FILE ( GLOB HEADERS ${PROJ_TVPLGS_DIR}/include/*.h )

#Qt files
#Requires Qt3 multi-threaded
set(QT_MT_REQUIRED, TRUE)
FIND_PACKAGE(Qt3 REQUIRED)

set (
	TO_MOC
	${PROJ_TVPLGS_DIR}/include/KFunctionPluginWindow.h
	${PROJ_TVPLGS_DIR}/include/kfuncplugin.h
	${PROJ_TVPLGS_DIR}/include/xyseries.h
	${PROJ_TVPLGS_DIR}/include/xyplot.h
)

file( GLOB UI_FILES ${PROJ_TVPLGS_DIR}/ui/*.ui )
QT_WRAP_UI( ${PROJ_NAME} HDRS uiFiles ${UI_FILES} )
QT_WRAP_CPP ( ${PROJ_NAME} moc_files ${TO_MOC} )


source_group( "Generated Files\\Header" FILES ${HDRS} )
source_group( "Generated Files\\Source" FILES ${moc_files} ${uiFiles} )
source_group( "UI Files" FILES ${UI_FILES} )

list( APPEND TSLIST "kfunctions_pt" "kfunctions_sp" )

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

add_library ( ${PROJ_NAME} SHARED ${SRCS} ${HEADERS} ${HDRS} ${uiFiles} ${moc_files}  ${UI_FILES})  

target_link_libraries ( ${PROJ_NAME} terralib terraViewCore )
