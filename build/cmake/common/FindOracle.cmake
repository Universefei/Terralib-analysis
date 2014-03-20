# Find the Oracle installation.
# (c) Frederico A. Bedê, 2011
# ----------------------------------------------------------------------------
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # Oracle
# FIND_PACKAGE(Oracle)
# ...
# if( ORACLE_FOUND )
#   include_directories(${ORACLE_INCLUDE_DIRS})
#   link_directories(${ORACLE_LIBRARY_DIRS})
# endif( ORACLE_FOUND )
# ...
# Remember to include ${ORACLE_LIBRARIES} in the target_link_libraries() statement.
#
# ----------------------------------------------------------------------------
# IMPORTANT - You may need to manually set:
#  PATHS in line 54  - path to where the Oracle include files are.
#  PATHS in line 62  - path to where the Oracle library files are.
#  in case FindOracle.cmake cannot find the include files or the library files.
#
# ----------------------------------------------------------------------------
# The following variables are set if Oracle is found:
#  ORACLE_FOUND         - Set to true when Oracle is found.
#  ORACLE_INCLUDE_DIR  - Include directories for Oracle
#  ORACLE_LIBRARIES     - The Oracle libraries.
#

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

# Find path - tries to find *.h in paths hard-coded by the script
FIND_PATH (
  ORACLE_INCLUDE_DIR 
  NAMES oci.h
  PATHS ${T4_DP_INC}/Oracle
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	FIND_LIBRARY(
	  ORACLE_LIBRARY
	  NAMES oci.lib
	  PATHS ${T4_DP_LIB}
	)
ELSE()
	FIND_LIBRARY(
	  ORACLE_LIBRARY
	  NAMES clntsh
	  PATHS ${T4_DP_LIB}
	)
	
	FIND_LIBRARY(
	  NNZ_LIBRARY
	  NAMES nnz10 nnz11
	  PATHS ${T4_DP_LIB}
	)
ENDIF()


# Export include and library path for linking with other libraries

IF (ORACLE_INCLUDE_DIR  AND ORACLE_LIBRARY)
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		SET(ORACLE_FOUND TRUE)
	ELSE()
		IF(NNZ_LIBRARY)
			SET(ORACLE_FOUND TRUE)	
		ELSE()
			MESSAGE("Could NOT find NNZ library")
		ENDIF()
	ENDIF()
ELSE (ORACLE_INCLUDE_DIR AND ORACLE_LIBRARY)
	MESSAGE("Could NOT find Oracle library")
	SET(ORACLE_FOUND FALSE)
ENDIF (ORACLE_INCLUDE_DIR AND ORACLE_LIBRARY)

set ( ORACLE_LIBS ${ORACLE_LIBRARY} ${NNZ_LIBRARY} ) #PARENT_SCOPE )

MARK_AS_ADVANCED( ORACLE_LIBRARY ORACLE_INCLUDE_DIR )
