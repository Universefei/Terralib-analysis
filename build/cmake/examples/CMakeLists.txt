# Examples cmake project.
# (c) Frederico A. Bede, 2011
# ----------------------------------------------------------------------------
# A super project that constains all the examples of terralib.

cmake_minimum_required(VERSION 2.8)

#Build the image processing examples.
OPTION ( BUILD_IMG_EXAMPLES "Build also the image examples?" ON )


add_subdirectory ( addGeomRepresentation )
add_subdirectory ( connectionPool )
add_subdirectory ( convertCoordinates )
add_subdirectory ( copyLayer )

if (WIN32 AND NOT MINGW)
	add_subdirectory ( createCoverage )
endif()

add_subdirectory ( createDatabase )
add_subdirectory ( createLayer )
add_subdirectory ( createProximityMatrix )
add_subdirectory ( createSTElementSet )
add_subdirectory ( createSTElementSetFromLayer )
add_subdirectory ( createSTElementSetFromShapeFile )
add_subdirectory ( createSTElementSetFromTheme )

add_subdirectory ( createTable )
add_subdirectory ( createTheme )
add_subdirectory ( databaseQuery )
add_subdirectory ( databaseSQLQuery )
  if( BUILD_IMG_EXAMPLES )
    add_subdirectory ( image_processing )
  endif( BUILD_IMG_EXAMPLES )
add_subdirectory ( importCSV )
add_subdirectory ( importDBF )
add_subdirectory ( importGeoTab )
add_subdirectory ( importGridData )
add_subdirectory ( importJPEG )
add_subdirectory ( importMIDMIF )
add_subdirectory ( importShape )


add_subdirectory ( mosaicTIFFImages )
add_subdirectory ( proxMatrixAndSpatialStatistics )
add_subdirectory ( querierFromLayer )
add_subdirectory ( querierFromTheme )
add_subdirectory ( querierGroupChronon )
add_subdirectory ( querierGroupElement )
add_subdirectory ( querierGroupSpatialRest )
add_subdirectory ( querierWithSpatialRestBox )
add_subdirectory ( querierWithSpatialRestGeometry )
add_subdirectory ( rasterSlicing )
add_subdirectory ( spatialQuery )
add_subdirectory ( spatialQueryAndBuffer )
add_subdirectory ( themeGrouping )
add_subdirectory ( GDALDecoder )
