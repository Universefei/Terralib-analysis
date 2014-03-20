# - Find fbembed
# Find the native Firebird library
#
#  FBIRD_LIBRARIES   - List of libraries when using Firebird.
#  FBIRD_FOUND       - True if Firebird found.

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

SET(FBIRD_LIBRARY_NAMES fbembed fbclient)

FIND_LIBRARY(FBIRD_LIBRARY
   NAMES ${FBIRD_LIBRARY_NAMES}
   PATHS ${T4_DP_LIB}
)

IF (FBIRD_LIBRARY)
	SET(FBIRD_FOUND TRUE)
		
ELSE (FBIRD_LIBRARY)
	SET(FBIRD_FOUND FALSE)
	MESSAGE("Could NOT find Firebird library")
ENDIF (FBIRD_LIBRARY)

MARK_AS_ADVANCED ( FBIRD_LIBRARY )

	
