# Add geometry representation example.
# (c) Paulo Renato M. Oliveira, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of terralib how to create a point representation.
#
# Description
# -----------
# This file shows an example of how to create a point representation
#(centroid of polygons) to a layer in a TerraLib database.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "addGeomRepresentation" )
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

#add_definitions ( -fopenmp )

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/addGeomRepresentation/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )