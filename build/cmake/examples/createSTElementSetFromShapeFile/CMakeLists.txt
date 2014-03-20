# Create a new Spatial Temporal Element Set (STElementSet) from a shapefile example.
# (c) Paulo Renato M. Oliveira, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Terralib example of how to create a new Spatial Temporal Element Set (STElementSet) from a shapefile.
#
# Description
# -----------
#  This file illustrates an example of how to create a new Spatial Temporal Element 
#  Set (STElementSet) from a shapefile. 

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "createSTElementSetFromShapeFile" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/createSTElementSetFromShapeFile/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )