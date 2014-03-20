# Find the QWT installation.
# (c) Frederico A. Bedê, 2011
# ----------------------------------------------------------------------------
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # QWT
# FIND_PACKAGE(QWT)
# ...
# if( QWT_FOUND )
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
#  QWT_LIBRARY     - The QWT libraries.
#

cmake_minimum_required(VERSION 2.8)

# Find library - - tries to find *.a,*.so,*.dylib in paths hard-coded by the script
include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

IF ( APPLE )
	SET ( MACPORTS_INC_DIR /opt/local/include)
	SET ( MAC_QWT_LIB_NAME libqwt.dylib )
	SET ( MACPORTS_LIB_DIR /opt/local/lib )
ENDIF ( APPLE )

FIND_PATH(QWT_INCLUDE_DIR qwt.h
  PATHS ${MACPORTS_INC_DIR}/qwt ${T4_DP_INC}/qwt /usr/include/qwt-qt3 include/qwt-qt3
)

if(APPLE)
  FIND_LIBRARY(QWT_LIBRARY
    NAMES ${MAC_QWT_LIB_NAME} qwt libqwt qwt-qt3
    PATHS ${MACPORTS_LIB_DIR} 
  )
elseif( $ENV{QTDIR} MATCHES 3.3.8 )
	FIND_LIBRARY(QWT_LIBRARY
		NAMES qwt libqwt qwt-qt3
		PATHS ${T4_DP_LIB}/qwt_qt-3.3.8 
	)
else()
	FIND_LIBRARY(QWT_LIBRARY
		NAMES qwt libqwt qwt-qt3
		PATHS ${T4_DP_LIB}/qwt_qt-3.2.0 
	)
endif()


# Export include and library path for linking with other libraries

IF (QWT_INCLUDE_DIR AND QWT_LIBRARY)
	SET(QWT_FOUND TRUE)
	MESSAGE(" -- Found Qwt include in ${QWT_INCLUDE_DIR} and Qwt library ${QWT_LIBRARY}.")
ELSE (QWT_INCLUDE_DIR AND QWT_LIBRARY)
	SET(QWT_FOUND FALSE)
	MESSAGE("Looked for QWT libraries named ${QWT_NAMES}.")
	MESSAGE("Could NOT find QWT library")
ENDIF (QWT_INCLUDE_DIR AND QWT_LIBRARY)

MARK_AS_ADVANCED( QWT_INCLUDE_DIR QWT_LIBRARY )
