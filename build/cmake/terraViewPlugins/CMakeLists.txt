#TerraView Plugins
# (c) André G de Oliveira, 2011
# ----------------------------------------------------------------------------
# A super project that constains all the plugins of terraView

cmake_minimum_required(VERSION 2.8)


set (ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..") # change this if terralib is somewhere else
set (TVPLGS_DIR "${ROOT}/terraViewPlugins") # change this if terralib is somewhere else

include_directories ( ${ROOT}/terraViewPlugins/base )
#Options to build each of the plugins


OPTION (BUILD_COPYDATABASE "Build the copyDatabase plugin?" OFF)
OPTION (BUILD_EXTERNALTHEME "Build the external theme plugin?" OFF)
OPTION (BUILD_FILETHEME "Build the file theme plugin?" OFF)
OPTION (BUILD_FILLCELL "Build the fillCell plugin?" OFF)
OPTION (BUILD_FLOW "Build the Flow plugin?" OFF)
OPTION (BUILD_KMLPLUGIN "Build the Kml plugin?" OFF)
OPTION (BUILD_MERCADO "Build the Mercado plugin?" OFF)
OPTION (BUILD_ORACLESPATIALADAPTER "Build the Oracle Spatial Adapter plugin?" OFF)
OPTION (BUILD_PLUGINGENERATOR "Build the plugin Generator plugin?" OFF)
OPTION (BUILD_POSTGISADAPTER "Build the PostGIS Adapter plugin?" OFF)
OPTION (BUILD_SPATIALSTAT "Build the Spatial Stat plugin?" OFF)
OPTION (BUILD_VORONOI "Build the Voronoi plugin?" OFF)
OPTION (BUILD_WMSTHEME "Build the WMS theme plugin?" OFF)
OPTION (BUILD_WFSTHEME "Build the WFS theme plugin?" OFF)
OPTION (BUILD_WCSCLIENT "Build the WCS client plugin?" OFF)

if(BUILD_COPYDATABASE)
	add_subdirectory ( copyDatabase )
endif()

if(BUILD_EXTERNALTHEME)
	add_subdirectory ( ExternalTheme )
endif()

if(BUILD_FILETHEME)
	add_subdirectory ( FileTheme )
endif()

if(BUILD_FILLCELL)
	add_subdirectory ( fillCell )
endif()

if(BUILD_FLOW)
	add_subdirectory ( Flow )
endif()

if(BUILD_KMLPLUGIN)
	add_subdirectory ( KmlPlugin )
endif()

if(BUILD_MERCADO)
	add_subdirectory ( mercado )
endif()

if(BUILD_ORACLESPATIALADAPTER)
	add_subdirectory ( oracleSpatialAdapter )
endif()

if(BUILD_PLUGINGENERATOR)
	add_subdirectory ( pluginGenerator )
endif()

if(BUILD_POSTGISADAPTER)
	add_subdirectory ( PostGISAdapter )
endif()

if(BUILD_SPATIALSTAT)
	add_subdirectory ( SpatialStat )
endif()

if(BUILD_VORONOI)
	add_subdirectory ( Voronoi )
endif()

if(BUILD_WMSTHEME)
	add_subdirectory ( wmsClient )
endif()

if(BUILD_WFSTHEME)
	add_subdirectory ( wfsClient )
endif()

if(BUILD_WCSCLIENT)
	add_subdirectory ( wcsClient )
endif()

# Export variables
set ( BUILD_COPYDATABASE ${BUILD_COPYDATABASE} PARENT_SCOPE )
set ( BUILD_EXTERNALTHEME ${BUILD_EXTERNALTHEME} PARENT_SCOPE )
set ( BUILD_FILETHEME ${BUILD_FILETHEME} PARENT_SCOPE )
set ( BUILD_FILLCELL ${BUILD_FILLCELL} PARENT_SCOPE )
set ( BUILD_FLOW ${BUILD_FLOW} PARENT_SCOPE )
set ( BUILD_MERCADO ${BUILD_MERCADO} PARENT_SCOPE )
set ( BUILD_ORACLESPATIALADAPTER ${BUILD_ORACLESPATIALADAPTER} PARENT_SCOPE )
set ( BUILD_PLUGINGENERATOR ${BUILD_PLUGINGENERATOR} PARENT_SCOPE )
set ( BUILD_POSTGISADAPTER ${BUILD_POSTGISADAPTER} PARENT_SCOPE )
set ( BUILD_VORONOI ${BUILD_VORONOI} PARENT_SCOPE )
set ( BUILD_WMSTHEME ${BUILD_WMSTHEME} PARENT_SCOPE )
set ( BUILD_WFSTHEME ${BUILD_WFSTHEME} PARENT_SCOPE )
set ( BUILD_WCSCLIENT ${BUILD_WCSCLIENT} PARENT_SCOPE )
set (TEOGC_BINARIES ${TEOGC_BINARIES} PARENT_SCOPE)
set ( PACKAGE_DEPENDS ${PACKAGE_DEPENDS} PARENT_SCOPE)
