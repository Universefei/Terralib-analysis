# Create Spatial Temporal Element Set (STElementSet) example.
# (c) Paulo Renato M. Oliveira, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Terralib example of how to create a new Spatial Temporal Element Set (STElementSet)
#
# Description
# -----------
#  This file illustrates an example of how to create a new Spatial Temporal Element 
#  Set (STElementSet) from a theme. A Spatial Temporal Element Set can be created 
#  from a layer or from a theme.
#  In this example the STElementSet is created from a layer and is filled only
#  with attributes.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "createSTElementSet" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/createSTElementSet/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )