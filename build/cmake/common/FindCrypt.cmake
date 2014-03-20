# Find the Crypt installation.
# (c) Frederico A. Bedê, 2011
# ----------------------------------------------------------------------------
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # Crypt
# FIND_PACKAGE(Crypt)
# ...
# if( CRYPT_FOUND )
#   include_directories(${CRYPT_INCLUDE_DIRS})
#   link_directories(${CRYPT_LIBRARY_DIRS})
# endif( CRYPT_FOUND )
# ...
# ----------------------------------------------------------------------------
# IMPORTANT - You may need to manually set:
#  PATHS in line 35  - path to where the Crypt include files are.
#  PATHS in line 40  - path to where the Crypt library files are.
#  in case FindCrypt.cmake cannot find the include files or the library files.
#
# ----------------------------------------------------------------------------
# The following variables are set if Crypt is found:
#  CRYPT_FOUND         - Set to true when CRYPT is found.
#  CRYPT_INCLUDE_DIR  - Include directories for CRYPT
#  CRYPT_LIBRARIES     - The CRYPT libraries.
#

cmake_minimum_required(VERSION 2.8)

# Find library - - tries to find *.a,*.so,*.dylib in paths hard-coded by the script

FIND_PATH(CRYPT_INCLUDE_DIR crypt.h
   HINTS  /opt/local/include /usr/local/include /usr/include
)

FIND_LIBRARY(CRYPT_LIBRARY
   NAMES libcrypt.a libcrypt.so
   PATHS /usr/lib /usr/local/lib /opt/local/lib
)

# Export include and library path for linking with other libraries

IF (CRYPT_INCLUDE_DIR AND CRYPT_LIBRARY)
	SET(CRYPT_FOUND TRUE)
	MESSAGE(" -- Found library crypt in ${CRYPT_LIBRARY}.")
	MESSAGE(" -- Found crypt header files in ${CRYPT_INCLUDE_DIR}.")
ELSE (CRYPT_INCLUDE_DIR AND CRYPT_LIBRARY)
	SET(CRYPT_FOUND FALSE)
	MESSAGE("Looked for Crypt libraries named ${CRYPT_NAMES}.")
	MESSAGE("Could NOT find Crypt library")
ENDIF (CRYPT_INCLUDE_DIR AND CRYPT_LIBRARY)

MARK_AS_ADVANCED( CRYPT_INCLUDE_DIR CRYPT_LIBRARY )

