#Requires Qt3 multi-threaded and qwt
set(QT_MT_REQUIRED, TRUE)

find_package(Qt3 REQUIRED)
find_package(QWT REQUIRED)

add_definitions ( ${QT_DEFINITIONS} -DQT_THREAD_SUPPORT )

list ( APPEND TE_DIRS "${INCLDIR}/drivers/qwt" ${terralib_BINARY_DIR} )
list ( APPEND TE_INST_DIRS "terralib/drivers/qwt" )

include_directories ( ${QT_INCLUDE_DIR} ${QWT_INCLUDE_DIR} ${terralib_BINARY_DIR} )

set ( DRVQWT_DIR ${SRCDIR}/drivers/qwt )
set ( _DIRS drivers/qwt )

#Appending files of Qt3 driver source files.
#file (GLOB QWT3_SRCS ${DRVQWT_DIR}/*.cpp)

# Build tree directories
appFPrefix( "${INCLDIR}" "${_DIRS}" TEDRVQWT_DIRS )
list ( APPEND TE_DIRS "${TEDRVQWT_DIRS}" "${DRVQWT_DIRS}" )

getFfiles ( "${INCLDIR}" "${_DIRS}" drvqwt_h_files true "" )
getFfiles ( "${SRCDIR}" "${_DIRS}" drvqwt_srcs_files false "" )

appFPrefix( "terralib" "${_DIRS}" "${TEDRVQWT_INST_DIRS}" )
list ( APPEND TE_INST_DIRS "${TEDRVQWT_INST_DIRS}" "${DRVQWT_INST_DIRS}" )
list ( APPEND SRCS "${drvqwt_h_files}" "${drvqwt_srcs_files}"  )

SET( 
  TOMOC_LIST 
  ${DRVQWT_DIR}/TeQwtPlotPicker.h
  ${DRVQWT_DIR}/TeQwtPlot.h
  ${DRVQWT_DIR}/TeQwtPlotZoomer.h
)

QT_WRAP_CPP ( ${PROJ_NAME} MOC_SRCS ${TOMOC_LIST} )
set ( fh_path "Generated Files\\qwt" )
source_group( ${fh_path} FILES ${MOC_SRCS} )



list ( APPEND TE_MODULES qwt3_drivers )

list ( APPEND TE_LIBS ${QT_LIBRARIES} ${QWT_LIBRARY} )

list ( APPEND SRCS ${MOC_SRCS} )

