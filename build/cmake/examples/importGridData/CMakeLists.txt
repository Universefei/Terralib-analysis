# Create database example.
# (c) André G. de Oliveira, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of terralib table creation.
#
# Description
# -----------
#	This is an example of how to import, to a TerraLib database, a raster data: 
#	a binary raw grid data. Each point of the grid is a float value that represents the 
#	elevation over an area 

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "importGridData" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/importGridData/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )