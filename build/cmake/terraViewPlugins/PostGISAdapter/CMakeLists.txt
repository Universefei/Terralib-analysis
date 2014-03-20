


SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/plugins)  

SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/plugins)  

SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}/plugins)
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/plugins)  


set (POSTGISADAPTER_TVPLGS_DIR "${TVPLGS_DIR}/PostGISAdapter") # change this if terralib is somewhere else
set ( PROJ_NAME "PostGISAdapter" )

project ( ${PROJ_NAME} )

if( (NOT DEFINED TE_DIRS) OR  (NOT DEFINED TE_LIBS) ) 
	find_package ( terralib REQUIRED )
endif()

include_directories (
	${TE_DIRS}
	${TV_DIRS}
	${POSTGISADAPTER_TVPLGS_DIR}/include
	${QT_INCLUDE_DIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"
)

FILE ( GLOB SRCS ${POSTGISADAPTER_TVPLGS_DIR}/src/*.cpp )
FILE ( GLOB HEADERS ${POSTGISADAPTER_TVPLGS_DIR}/include/*.h )

#Qt files
#Requires Qt3 multi-threaded
set(QT_MT_REQUIRED, TRUE)
FIND_PACKAGE(Qt3 REQUIRED)

set (
	TO_MOC
	${POSTGISADAPTER_TVPLGS_DIR}/include/postgisadapter.h
)

file( GLOB UI_FILES ${POSTGISADAPTER_TVPLGS_DIR}/ui/*.ui )
file( GLOB H_UI_FILES ${POSTGISADAPTER_TVPLGS_DIR}/ui/*.h )
QT_WRAP_UI( ${PROJ_NAME} HDRS uiFiles ${UI_FILES} )
QT_WRAP_CPP ( ${PROJ_NAME} moc_files ${TO_MOC} )


source_group( "Generated Files\\Header" FILES ${HDRS} )
source_group( "Generated Files\\Source" FILES ${moc_files} ${uiFiles} )
source_group( "UI Files" FILES ${UI_FILES} ${H_UI_FILES} )


list( APPEND TSLIST "postgisadapter_pt" "postgisadapter_sp" )

set (PRO_FILENAME ${CMAKE_BINARY_DIR}/${PROJ_NAME}.pro )
foreach( TS ${TSLIST})
	set ( TSFILES "${TSFILES} ${POSTGISADAPTER_TVPLGS_DIR}/${TS}.ts" )
endforeach()

generatePRO( ${PRO_FILENAME} "${HEADERS}" "${SRCS}" "${UI_FILES}" "${TSFILES}" )
generateTS( ${PRO_FILENAME} )
foreach( TS ${TSLIST})
	set ( QMFILE "${TRANSLATION_DIR_DEBUG}/${TS}.qm" )
	set ( TSFILE "${POSTGISADAPTER_TVPLGS_DIR}/${TS}.ts" )
	generateQM( ${TSFILE} ${QMFILE} )
	FILE ( COPY ${QMFILE} DESTINATION ${TRANSLATION_DIR_RELEASE} )
endforeach()


# -----------------------------------

add_definitions ( ${QT_DEFINITIONS} -DQT_THREAD_SUPPORT )

add_library ( ${PROJ_NAME} SHARED ${SRCS} ${HEADERS} ${HDRS} ${uiFiles} ${moc_files} ${H_UI_FILES} ${UI_FILES})  

target_link_libraries ( ${PROJ_NAME} terralib terraViewCore )




