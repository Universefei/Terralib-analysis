# Find the GD installation.
# (c) Frederico A. Bedê, 2011
# ----------------------------------------------------------------------------
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# FIND_PACKAGE( gdLib )
# ...
# if( GDLIB_FOUND )
#   include_directories(${GD_INCLUDE_DIR})
#   link_directories(${GD_LIBRARY})
# endif(  )
# ...
# Remember to include ${GD_LIBRARY} in the target_link_libraries() statement.
#
# ----------------------------------------------------------------------------
# The following variables are set if GD is found:
#  GDLIB_FOUND         - Set to true when GD is found.
#  GDLIB_INCLUDE_DIR   - Include directories for GD
#  GDLIB_LIBRARY     	- The GD libraries.

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

# Find path - tries to find *.h in paths hard-coded by the script
FIND_PATH(GDLIB_INCLUDE_DIR gd.h
   PATHS ${T4_DP_INC}/gdWin32
)

set ( GD_NAMES gd libbgd bgd )

FIND_LIBRARY(GDLIB_LIBRARY 
  NAMES ${GD_NAMES}
  HINTS  /opt/local/lib /usr/local/lib /usr/lib ${T4_DP_LIB}
)

IF (GDLIB_INCLUDE_DIR AND GDLIB_LIBRARY)
	SET(GDLIB_FOUND TRUE)
	MESSAGE(" -- Found library GD in ${GDLIB_LIBRARY}.")
ELSE ()
	SET(GDLIB_FOUND FALSE)
	MESSAGE("Looked for GD library named ${GD_NAMES}.")
ENDIF ()

MARK_AS_ADVANCED( GDLIB_INCLUDE_DIR GDLIB_LIBRARY )
