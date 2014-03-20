# Create database example.
# (c) André G. de Oliveira, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of terralib table creation.
#
# Description
# -----------
#   This is an example of how to import a DBF table as
#	an attribute table in a TerraLib database. This attribute 
#	table will be registered in the database as an external table.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "importDBF" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/importDBF/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )