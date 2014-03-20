# Create database example.
# (c) Willyan A. Machado, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of terralib database creation.
#
# Description
# -----------
#    This example shows how to use a querier from theme, using a spatial restriction 
#	defined by a rectangle (TeBOX). This spatial restriction is associated to querier.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "querierWithSpatialRestBox" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/querierWithSpatialRestBox/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )