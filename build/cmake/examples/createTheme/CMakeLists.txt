# Create database example.
# (c) Andr� G. de Oliveir,a 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of terralib table creation.
#
# Description
# -----------
#  This example illustrates how to create themes in a TerraLib database

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "createTheme" )
project ( ${PROJ_NAME} )

if( (NOT DEFINED TE_DIRS) OR  (NOT DEFINED TE_LIBS) ) 
	find_package ( terralib REQUIRED )
endif()

if( NOT DEFINED TE_EXAMPLES_DIR )
	set ( TE_EXAMPLES_DIR "../../../../examples" )
endif( )

include_directories (
	${TE_DIRS}
)

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/createTheme/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )