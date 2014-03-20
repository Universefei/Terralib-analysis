# Create database example.
# (c) Frederico A. Bede, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of terralib database creation.
#
# Description
# -----------
# This in an example of how to create a TerraLib database called "DB320RC1"
# in an MySQL DBMS Server running in the local machine, supposing there is an
# user called "root" with no password.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "createDatabase" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/createDatabase/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )