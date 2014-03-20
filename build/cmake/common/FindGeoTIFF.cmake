# Find the GEOTIFF installation.
# (c) Gilberto Camara, 2011
# ----------------------------------------------------------------------------
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # GEOTIFF
# FIND_PACKAGE(GEOTIFF)
# ...
# if( GEOTIFF_FOUND )
#   include_directories(${GeoTIFF_INCLUDE_DIRS})
#   link_directories(${GeoTIFF_LIBRARY_DIRS})
# endif( GeoTIFF_FOUND )
# ...
# Remember to include ${GeoTIFF_LIBRARIES} in the target_link_libraries() statement.
#
# ----------------------------------------------------------------------------
# IMPORTANT - You may need to manually set:
#  PATHS in line 54  - path to where the GeoTIFF include files are.
#  PATHS in line 62  - path to where the GeoTIFF library files are.
#  in case FindGeoTIFF.cmake cannot find the include files or the library files.
#
# ----------------------------------------------------------------------------
# The following variables are set if GeoTIFF is found:
#  GeoTIFF_FOUND         - Set to true when GeoTIFF is found.
#  GeoTIFF_INCLUDE_DIR  - Include directories for GeoTIFF
#  GeoTIFF_LIBRARIES     - The GeoTIFF libraries.
#

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

# Find path - tries to find *.h in paths hard-coded by the script
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	FIND_PATH(GeoTIFF_INCLUDE_DIR geotiff.h
	   PATHS ${T4_DP_INC}/geotiff
	)
ELSE()
	FIND_PATH(GeoTIFF_INCLUDE_DIR geotiff.h
	   PATHS ${T4_DP_INC}/geotiff
	   PATHS /usr/include/geotiff
	)
ENDIF()

FIND_PATH(TIFF_INCLUDE_DIR tiff.h
   PATHS ${T4_DP_INC}/tiff
)

# Find library - - tries to find *.a,*.so,*.dylib in paths hard-coded by the script

FIND_LIBRARY(GeoTIFF_LIBRARY
   NAMES geotiff
   PATHS ${T4_DP_LIB}
)

FIND_LIBRARY(TIFF_LIBRARY
   NAMES tiff libtiff_i
   PATHS ${T4_DP_LIB}
)

# Export include and library path for linking with other libraries

IF (GeoTIFF_INCLUDE_DIR AND TIFF_INCLUDE_DIR AND TIFF_LIBRARY AND GeoTIFF_LIBRARY)
	SET(GeoTIFF_FOUND TRUE)
ELSE (GeoTIFF_INCLUDE_DIR TIFF_INCLUDE_DIR AND TIFF_LIBRARY AND GeoTIFF_LIBRARY)
	SET(GeoTIFF_FOUND FALSE)
	MESSAGE("Looked for GeoTIFF libraries named geotiff and libtiff.")
	MESSAGE("Could NOT find GeoTIFF library")
ENDIF (GeoTIFF_INCLUDE_DIR AND TIFF_INCLUDE_DIR AND TIFF_LIBRARY AND GeoTIFF_LIBRARY)

MARK_AS_ADVANCED(  GeoTIFF_LIBRARY TIFF_LIBRARY GeoTIFF_INCLUDE_DIR TIFF_INCLUDE_DIR )
