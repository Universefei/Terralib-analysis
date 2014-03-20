# (c) Willyan A. Machado, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Description
# -----------
# This file shows an example of PDI MMIOMatching.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "TePDIMMIOMatching" )
project ( ${PROJ_NAME} )

if( (NOT DEFINED TE_DIRS) OR  (NOT DEFINED TE_LIBS) ) 
	find_package ( terralib REQUIRED )
endif()

if( NOT DEFINED TE_EXAMPLES_DIR )
	set ( TE_EXAMPLES_DIR "../../../../../examples" )
endif( )

include_directories (
	${TE_DIRS}
)

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/image_processing/source/TePDIMMIOMatching/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )