# Find the PostGIS installation.
# (c) Gilberto Camara, 2011
# ----------------------------------------------------------------------------
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # PostGIS
# FIND_PACKAGE(PostGIS)
# ...
# if( PostGIS_FOUND )
#   include_directories(${PostGIS_INCLUDE_DIRS})
#   link_directories(${PostGIS_LIBRARY_DIRS})
# endif( PostGIS_FOUND )
# ...
# Remember to include ${PostGIS_LIBRARIES} in the target_link_libraries() statement.
#
# ----------------------------------------------------------------------------
# IMPORTANT - You may need to manually set:
#  PATHS in line 54  - path to where the PostGIS include files are.
#  PATHS in line 62  - path to where the PostGIS library files are.
#  in case FindPostGIS.cmake cannot find the include files or the library files.
#
# ----------------------------------------------------------------------------
# The following variables are set if PostGIS is found:
#  PostGIS_FOUND         - Set to true when PostGIS is found.
#  PostGIS_INCLUDE_DIR  - Include directories for PostGIS
#  PostGIS_LIBRARIES     - The PostGIS libraries.
#

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )

getTePath( T4_DP_INC T4_DP_LIB )

IF ( WIN32 )
  IF ( MINGW )
    SET ( PQ_LIBRARY_NAMES libpq.a )
  ELSE ( MINGW )
    SET ( PQ_LIBRARY_NAMES libpq.lib )
  ENDIF ( MINGW )
ELSE ( WIN32 )
  SET( PQ_LIBRARY_NAMES pq )
ENDIF ( WIN32 )

# Find path - tries to find *.h in paths hard-coded by the script
FIND_PATH(PostGIS_INCLUDE_DIR pg_config.h
   PATHS "${T4_DP_INC}/PostgreSQL"
   PATH_SUFFIXES postgresql90
)

FIND_LIBRARY(PostgreSQL_LIBRARY
   NAMES ${PQ_LIBRARY_NAMES}
   PATHS ${T4_DP_LIB}
   PATH_SUFFIXES postgresql90
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  FIND_LIBRARY(Crypt_LIBRARY
    NAMES libcrypt.a libcrypt.so
    PATHS /usr/lib /usr/local/lib /opt/local/lib
    PATH_SUFFIXES postgresql90
  )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


# Export include and library path for linking with other libraries

IF (PostGIS_INCLUDE_DIR  AND PostgreSQL_LIBRARY )
	SET(PostGIS_FOUND TRUE)
ELSE (PostGIS_INCLUDE_DIR AND PostgreSQL_LIBRARY )
	MESSAGE("Could NOT find PostgreSQL library")
	SET(PostGIS_FOUND FALSE)
ENDIF (PostGIS_INCLUDE_DIR AND PostgreSQL_LIBRARY )

list (APPEND PGres_LIBS ${PostgreSQL_LIBRARY})

if(Crypt_LIBRARY)
  list ( APPEND PGres_LIBS ${Crypt_LIBRARY} )
endif(Crypt_LIBRARY)

set ( PGres_LIBS ${PGres_LIBS} PARENT_SCOPE )

MARK_AS_ADVANCED(  PostgreSQL_LIBRARY PostGIS_INCLUDE_DIR )
