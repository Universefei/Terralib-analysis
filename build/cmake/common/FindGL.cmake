# Find the GL and GLU installation.
# (c) Frederico A. Bedê, 2011
# ----------------------------------------------------------------------------
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # GL
# FIND_PACKAGE(GL)
# ...
# if( GL_FOUND )
#   include_directories(${QWT_INCLUDE_DIRS})
#   link_directories(${QWT_LIBRARY_DIRS})
# endif( QWT_FOUND )
# ...
# Remember to include ${QWT_LIBRARIES} in the target_link_libraries() statement.
#
# ----------------------------------------------------------------------------
# IMPORTANT - You may need to manually set:
#  PATHS in line 35  - path to where the QWT include files are.
#  PATHS in line 40  - path to where the QWT library files are.
#  in case FindQWT.cmake cannot find the include files or the library files.
#
# ----------------------------------------------------------------------------
# The following variables are set if QWT is found:
#  QWT_FOUND         - Set to true when QWT is found.
#  QWT_INCLUDE_DIR  - Include directories for QWT
#  QWT_LIBRARIES     - The QWT libraries.
#

cmake_minimum_required(VERSION 2.8)

# Find library - - tries to find *.a,*.so,*.dylib in paths hard-coded by the script

FIND_LIBRARY(GL_LIBRARY 
  NAMES GL  
  HINTS  /opt/local/lib /usr/local/lib /usr/lib
)

FIND_LIBRARY(GLU_LIBRARY
   NAMES GLU
   PATHS /usr/lib /usr/local/lib /opt/local/lib
)

# Export include and library path for linking with other libraries

IF (GL_LIBRARY AND GLU_LIBRARY)
	SET(GL_FOUND TRUE)
	MESSAGE(" -- Found library GL in ${GL_LIBRARY}.")
	MESSAGE(" -- Found library GLU in ${GLU_LIBRARY}.")
ELSE (GL_LIBRARY AND GLU_LIBRARY)
	SET(GL_FOUND FALSE)
	MESSAGE("Looked for GL libraries named ${GL_NAMES}.")
	MESSAGE("Could NOT find GL and GLU libraries")
ENDIF (GL_LIBRARY AND GLU_LIBRARY)

MARK_AS_ADVANCED( GL_LIBRARY GLU_LIBRARY )
