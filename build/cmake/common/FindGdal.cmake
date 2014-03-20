# Find the GDAL installation.
# (c) Douglas Uba, 2011
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

include ( ${CMAKE_MODULE_PATH}/CMakeUtils.cmake )
getTePath( T4_DP_INC T4_DP_LIB )

IF ( APPLE )
	SET ( MACPORTS_INC /opt/local/include )
	SET ( MACPORTS_LIB /opt/local/lib )
	SET ( GDAL_NAME_MAC libgdal.dylib )
ENDIF ( APPLE )

# Find path - tries to find *.h in paths hard-coded by the script
FIND_PATH(GDAL_CORE_INCLUDE_DIR gdal.h
   PATHS ${MACPORTS_INC} ${T4_DP_INC}/gdal/gcore )
FIND_PATH(GDAL_OGR_INCLUDE_DIR ogr_core.h
   PATHS ${MACPORTS_INC} ${T4_DP_INC}/gdal/ogr )
FIND_PATH(GDAL_OGR_FRMTS_INCLUDE_DIR ogrsf_frmts.h
   PATHS ${MACPORTS_INC} ${T4_DP_INC}/gdal/ogr/ogrsf_frmts )
FIND_PATH(GDAL_PORT_INCLUDE_DIR cpl_port.h  
   PATHS ${MACPORTS_INC} ${T4_DP_INC}/gdal/port )
   
# Find library - - tries to find *.a,*.so,*.dylib in paths hard-coded by the script
SET(GDAL_NAMES gdal_i)


FIND_LIBRARY(GDAL_LIBRARY
   NAMES ${GDAL_NAME_MAC} ${GDAL_NAMES}
   PATHS ${MACPORTS_LIB} ${T4_DP_LIB}
)

# Export include and library path for linking with other libraries

IF (GDAL_CORE_INCLUDE_DIR AND GDAL_OGR_INCLUDE_DIR 
	AND GDAL_OGR_FRMTS_INCLUDE_DIR AND GDAL_PORT_INCLUDE_DIR
	AND GDAL_LIBRARY)
	SET(GDAL_FOUND TRUE)
ELSE (GDAL_CORE_INCLUDE_DIR AND GDAL_OGR_INCLUDE_DIR 
	AND GDAL_OGR_FRMTS_INCLUDE_DIR AND GDAL_PORT_INCLUDE_DIR
	AND GDAL_LIBRARY)
	SET(GDAL_FOUND FALSE)
	MESSAGE("Could NOT find GDAL library")
ENDIF (GDAL_CORE_INCLUDE_DIR AND GDAL_OGR_INCLUDE_DIR 
	AND GDAL_OGR_FRMTS_INCLUDE_DIR AND GDAL_PORT_INCLUDE_DIR
	AND GDAL_LIBRARY)

MARK_AS_ADVANCED(  GDAL_LIBRARY  GDAL_INCLUDE_DIR )
