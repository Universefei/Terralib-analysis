# Create proximity matrix example.
# (c) Paulo Renato M. Oliveira, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Terralib example of of how to create a proximity matrix from a Spatial Temporal Element Set 
#
# Description
# -----------
#  This file illustrates an example of how to create a proximity matrix from a 
#  Spatial Temporal Element Set (STElementSet). 
#  In this example, the distance strategy is used to construct the matrix. 

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "createProximityMatrix" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/createProximityMatrix/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )