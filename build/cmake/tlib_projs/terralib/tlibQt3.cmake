#Requires Qt3 multi-threaded
set(QT_MT_REQUIRED, TRUE)
find_package(Qt3 REQUIRED)

add_definitions ( ${QT_DEFINITIONS} -DQT_THREAD_SUPPORT )

list ( APPEND TE_DIRS "${INCLDIR}/drivers/qt" ${terralib_BINARY_DIR} "${ROOT}/app_icons" )
list ( APPEND TE_INST_DIRS "terralib/drivers/qt" )

include_directories ( ${QT_INCLUDE_DIR} )

set ( DRVQT_DIR ${SRCDIR}/drivers/qt )
set ( _DIRS drivers/qt )

# Instalation tree directories
appFPrefix( "terralib" "${_DIRS}" "${TEDRVQT_INST_DIRS}" )

list ( APPEND TE_INST_DIRS "${TEDRVQT_INST_DIRS}" "${DRVQT_INST_DIRS}" )

# Build tree directories
appFPrefix( "${INCLDIR}" "${_DIRS}" TEDRVQT_DIRS )
list ( APPEND TE_DIRS "${TEDRVQT_DIRS}" "${DRVQT_DIRS}" )

getFfiles ( "${INCLDIR}" "${_DIRS}" drvqt_h_files true "" )
getFfiles ( "${SRCDIR}" "${_DIRS}" drvqt_srcs_files false "" )

list ( APPEND SRCS "${drvqt_h_files}" "${drvqt_srcs_files}" )

#Appending files of Qt3 driver source files.
#file (GLOB QT3_SRCS ${DRVQT_DIR}/*.cpp)

list( REMOVE_ITEM SRCS
	${DRVQT_DIR}/TeQtLatEdit.cpp
	${DRVQT_DIR}/TeQtLongEdit.cpp
)

SET( 
	TOMOC_LIST 
	${DRVQT_DIR}/TeQtBigTable.h
	${DRVQT_DIR}/TeQtCanvas.h
	${DRVQT_DIR}/TeQtColorBar.h
	${DRVQT_DIR}/TeQtDatabasesListView.h
	${DRVQT_DIR}/TeQtDataSource.h
	${DRVQT_DIR}/TeQtFrame.h
	${DRVQT_DIR}/TeQtGLWidget.h
	${DRVQT_DIR}/TeQtGrid.h
	${DRVQT_DIR}/TeQtGridSource.h
	${DRVQT_DIR}/TeQtLegendSource.h
	${DRVQT_DIR}/TeQtProgress.h
	${DRVQT_DIR}/TeQtViewsListView.h
)

file( GLOB UI_FILES ${DRVQT_DIR}/ui/*.ui )

QT_WRAP_CPP ( ${PROJ_NAME} MOC_SRCS ${TOMOC_LIST} )

QT_WRAP_UI(${PROJ_NAME} HDRS UI_SRCS ${UI_FILES} )
set ( fh_path "Generated Files\\qt" )
source_group( ${fh_path} FILES ${MOC_SRCS} )



list ( APPEND TE_MODULES qt3_drivers )

list ( APPEND TE_LIBS ${QT_LIBRARIES} )

if(WIN32)
  list ( APPEND TE_LIBS opengl32 glu32 )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR "APPLE")
  find_package(GL REQUIRED)
  list (APPEND TE_LIBS ${GL_LIBRARY} ${GLU_LIBRARY})
endif()

#Qt3 embed image files
set (IMGDIR ${ROOT}/app_icons )
set (IMGGEN_FILE ${PROJECT_BINARY_DIR}/qimages_src.cpp)
set (IMG_COL ${CMAKE_BINARY_DIR}/qimages.imgcol)

generateImgCol( ${PROJ_NAME} ${IMG_COL} ${IMGDIR} ${IMGGEN_FILE} )

list ( APPEND SRCS ${QT3_SRCS} ${MOC_SRCS} ${UI_SRCS} ${IMGGEN_FILE} )

