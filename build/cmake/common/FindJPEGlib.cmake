# Find the JPEG installation.
# (c) Gilberto Camara, 2011
# ----------------------------------------------------------------------------
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # JPEG
# FIND_PACKAGE(JPEG)
# ...
# if( JPEG_FOUND )
#   include_directories(${JPEG_INCLUDE_DIRS})
#   link_directories(${JPEG_LIBRARY_DIRS})
# endif( JPEG_FOUND )
# ...
# Remember to include ${JPEG_LIBRARIES} in the target_link_libraries() statement.
#
# ----------------------------------------------------------------------------
# IMPORTANT - You may need to manually set:
#  PATHS in line 54  - path to where the JPEG include files are.
#  PATHS in line 62  - path to where the JPEG library files are.
#  in case FindJPEG.cmake cannot find the include files or the library files.
#
# ----------------------------------------------------------------------------
# The following variables are set if JPEG is found:
#  JPEG_FOUND         - Set to true when JPEG is found.
#  JPEG_INCLUDE_DIR  - Include directories for JPEG
#  JPEG_LIBRARIES     - The JPEG libraries.
#

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

# Find path - tries to find *.h in paths hard-coded by the script
FIND_PATH(JPEG_INCLUDE_DIR jpeglib.h
   PATHS ${T4_DP_INC}/jpeg
)

# Find library - - tries to find *.a,*.so,*.dylib in paths hard-coded by the script
SET(JPEG_NAMES jpeg)

FIND_LIBRARY(JPEG_LIBRARY
   NAMES ${JPEG_NAMES}
   PATHS ${T4_DP_LIB}
)

# Export include and library path for linking with other libraries

IF (JPEG_INCLUDE_DIR AND JPEG_LIBRARY)
	SET(JPEG_FOUND TRUE)
ELSE (JPEG_INCLUDE_DIR AND JPEG_LIBRARY)
	SET(JPEG_FOUND FALSE)
	MESSAGE("Could NOT find JPEG library")
ENDIF (JPEG_INCLUDE_DIR AND JPEG_LIBRARY)

MARK_AS_ADVANCED(  JPEG_LIBRARY  JPEG_INCLUDE_DIR )
