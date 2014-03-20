# Create database example.
# (c) Willyan A. Machado, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of terralib database creation.
#
# Description
# -----------
#   This file shows an example of how to use the database interface to do some
#   spatial queries involving objects with points, lines and polygon geometries,
#   and to generate a buffer operation.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "spatialQueryAndBuffer" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/spatialQueryAndBuffer/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )