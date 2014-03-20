# TerraView Fill Cell plugin cmake project.
# (c) André G de Oliveira, 2011
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/plugins)  

SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/plugins)  

SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/plugins)  

set (TVPLGS_DIR "${TVPLGS_DIR}/wmsclient") # change this if terralib is somewhere else

set ( PROJ_NAME "wmsClient" )

project ( ${PROJ_NAME} )

if( (NOT DEFINED TE_DIRS) OR  (NOT DEFINED TE_LIBS) ) 
	find_package ( terralib REQUIRED )
endif()

find_package ( terraogc REQUIRED COMPONENTS common xml xsd ows owsclient owstheme se sld filter gml xacml wms wmsclient wmstheme )
set (TEOGC_BINARIES ${TEOGC_BINARIES} PARENT_SCOPE)

include_directories (
	${TE_DIRS}
	${TV_DIRS}
	${TVPLGS_DIR}/include
	${TVPLGS_DIR}
	${TVPLGS_DIR}/images
	${QT_INCLUDE_DIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"
	${TEOGC_INCLUDE_DIRS}
)

#Qt files
#Requires Qt3 multi-threaded
set(QT_MT_REQUIRED, TRUE)
FIND_PACKAGE(Qt3 REQUIRED)

set ( ui_dir "${TVPLGS_DIR}/ui")
set ( inc_dir "${TVPLGS_DIR}/include")
set ( src_dir "${TVPLGS_DIR}/src")

#Configuring files to uic
file( GLOB UI_FILES ${ui_dir}/*.ui )
file( GLOB H_UI_FILES ${ui_dir}/*.h )
QT_WRAP_UI(${PROJ_NAME} PLG_UI_HDRS PLG_UI_SRCS ${UI_FILES} )

SET ( 
	TOMOC_LIST 
	${TVPLGS_DIR}/include/wmsclientPlugin.h
	${TVPLGS_DIR}/include/WMSConfigurer.h
	${PLG_UI_HDRS}
)

QT_WRAP_CPP ( ${PROJ_NAME} MOC_SRCS ${TOMOC_LIST} )

set ( _DIRS	
	.
)  

getFfiles ( "${inc_dir}" "${_DIRS}" h_files true "" )
getFfiles ( "${src_dir}" "${_DIRS}" srcs_files false "" )

file( GLOB UI_SRCS ${ui_dir}/*.h )

set ( SRCS ${h_files} ${srcs_files} ${UI_SRCS} ${UI_FILES} ${MOC_SRCS} ${PLG_UI_HDRS} ${H_UI_FILES}  ${PLG_UI_SRCS} )

# Ui filter	
source_group( "UI Files" FILES ${UI_FILES} ${H_UI_FILES} )
source_group( "Generated UI Files" FILES ${PLG_UI_HDRS} ${PLG_UI_SRCS} )
source_group( "Generated MOC Files" FILES ${MOC_SRCS} )


list( APPEND TSLIST "wmsclient_pt" "wmsclient_es" )

set (PRO_FILENAME ${CMAKE_BINARY_DIR}/${PROJ_NAME}.pro )
foreach( TS ${TSLIST})
	set ( TSFILES "${TSFILES} ${TVPLGS_DIR}/${TS}.ts" )
endforeach()

generatePRO( ${PRO_FILENAME} "${h_files}" "${srcs_files}" "${UI_FILES}" "${TSFILES}" )
generateTS( ${PRO_FILENAME} )
foreach( TS ${TSLIST})
	set ( QMFILE "${TRANSLATION_DIR_DEBUG}/${TS}.qm" )
	set ( TSFILE "${TVPLGS_DIR}/${TS}.ts" )
	generateQM( ${TSFILE} ${QMFILE} )
	FILE ( COPY ${QMFILE} DESTINATION ${TRANSLATION_DIR_RELEASE} )
endforeach()

# -----------------------------------

SET(PACKAGE_DEPENDS ${PACKAGE_DEPENDS} PARENT_SCOPE)

add_definitions ( ${QT_DEFINITIONS} -DQT_THREAD_SUPPORT )

add_library ( ${PROJ_NAME} SHARED ${SRCS}  )  

target_link_libraries ( ${PROJ_NAME} terralib terraViewCore ${TEOGC_LIBRARIES} )
