# - Find mysqlclient
# Find the native MySQL includes and library
#
#  MySQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MySQL_LIBRARIES   - List of libraries when using MySQL.
#  MySQL_FOUND       - True if MySQL found.

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

FIND_PATH(MySQL_INCLUDE_DIR 
   NAMES mysql/mysql.h
   PATHS ${T4_DP_INC}
)

IF ( WIN32 )
  IF ( MINGW )
    SET ( MySQL_LIBRARY_NAMES libmysql.a )
  ELSE ( MINGW )
    SET ( MySQL_LIBRARY_NAMES libmysql.lib )
  ENDIF ( MINGW )
ELSE ( WIN32 )
  SET( MySQL_LIBRARY_NAMES mysqlclient mysqlclient_r )
ENDIF ( WIN32 )

FIND_LIBRARY(MySQL_LIBRARY
   NAMES mysql mysqlclient mysqlclient_r
   NAMES ${MySQL_LIBRARY_NAMES}
   PATHS ${T4_DP_LIB}
)

IF (MySQL_INCLUDE_DIR AND MySQL_LIBRARY)
	SET(MySQL_FOUND TRUE)
	
ELSE (MySQL_INCLUDE_DIR AND MySQL_LIBRARY)
	SET(MySQL_FOUND FALSE)
	MESSAGE("Looked for MySQL libraries named ${MySQL_NAMES}.")
	MESSAGE("Could NOT find MySQL library")
ENDIF (MySQL_INCLUDE_DIR AND MySQL_LIBRARY)


