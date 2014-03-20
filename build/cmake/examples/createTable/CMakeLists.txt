# Create database example.
# (c) André G. de Oliveira, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of terralib table creation.
#
# Description
# -----------
#   This file illustrates how to execute some operations relative to attribute tables
#	in a MySQL TerraLib database:
#	 - how to create an attribute table;
#	 - how to add some columns;
#	 - how to insert same data into it;
#	 - how to retrieve some rows using an SQL.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "createTable" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/createTable/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )