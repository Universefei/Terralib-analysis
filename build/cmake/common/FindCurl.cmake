# - Find mysqlclient
# Find the native CURL includes and library
#
#  CURL_INCLUDE_DIR - where to find mysql.h, etc.
#  CURL_LIBRARIES   - List of libraries when using CURL.
#  CURL_FOUND       - True if CURL found.

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

FIND_PATH(CURL_INCLUDE_DIR 
   NAMES curl/curl.h
   PATHS ${T4_DP_INC}
)

IF ( WIN32 )
  IF ( MINGW )
    SET ( CURL_LIBRARY_NAMES libcurl.a )
  ELSE ( MINGW )
    SET ( CURL_LIBRARY_NAMES libcurl.lib )
  ENDIF ( MINGW )
ELSE ( WIN32 )
  SET( CURL_LIBRARY_NAMES libcurl libcurl_r )
ENDIF ( WIN32 )

FIND_LIBRARY(CURL_LIBRARY
   NAMES ${CURL_LIBRARY_NAMES}
   PATHS ${T4_DP_LIB}
)

IF (CURL_INCLUDE_DIR AND CURL_LIBRARY)
	SET(CURL_FOUND TRUE)
	
ELSE (CURL_INCLUDE_DIR AND CURL_LIBRARY)
	SET(CURL_FOUND FALSE)
	MESSAGE("Looked for CURL libraries named ${CURL_NAMES}.")
	MESSAGE("Could NOT find CURL library")
ENDIF (CURL_INCLUDE_DIR AND CURL_LIBRARY)


