# Create database example.
# (c) Willyan A. Machado, 2011
# ----------------------------------------------------------------------------
# brief
# -----
# Example of mosaic Tiff creation.
#
# Description
# -----------
# This file shows an example of how to build a mosaic of two images  
# in GeoTiff format in a layer of a TerraLib database.

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "mosaicTIFFImages" )
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

FILE ( GLOB SRCS ${TE_EXAMPLES_DIR}/mosaicTIFFImages/*.cpp )

add_executable ( ${PROJ_NAME} ${SRCS} )

target_link_libraries ( ${PROJ_NAME} terralib )