/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/
/*! \file TeApplicationUtils.h
    \brief This contains some utilitary functions to deal with TerraLib databases
*/
#ifndef  __TERRALIB_INTERNAL_APPLICATIONUTILS_H
#define  __TERRALIB_INTERNAL_APPLICATIONUTILS_H

#include "TeAppTheme.h"
#include "../../kernel/TeVisual.h"
#include "../../kernel/TeDatabase.h"
#include "../../kernel/TeGeneralizedProxMatrix.h"

#include "../../kernel/TeDefines.h"

#include <string>
#include <vector>
#include <set>
using namespace std;

struct TeKernelParams;


struct TL_DLL ObjectInfo
{
	int	status_;						// collection: c_object_status
	map<string, int> uniqueMap_;	// uniqueId to grid status (collection: grid_status)
};

/** @defgroup ExtTheme Extended Theme Functions
    @ingroup  DatabaseUtils
	A set of functions to manipulate the extension of a Theme as proposed by TerraView application. 
	It can be used as a model for other TerraLib based applications.
	\sa TeAppTheme.h
@{
*/
/**	Creates a table to store extra information about the extended Theme
	\param db			a pointer to a TerraLib database
	\param integrity	a flag to indicate that the a integrity relation should be created (with Theme)
	\param tableName	name of the table that persists the extended theme
*/
TL_DLL bool createAppThemeTable(TeDatabase* db, bool integrity=false, const string& tableName="te_theme_application");

/** Populates the extended theme table with default parameters
	\param db			a pointer to a TerraLib database
	\param tableName	name of the table that persists the extended theme
*/
TL_DLL bool populateAppThemeTable(TeDatabase* db, const string& tableName="te_theme_application");


/** Delete an extended theme

	\param db			a pointer to a TerraLib database
	\param theme		pointer to a theme to be deleted 
	\param tableName	name of the table that persists the extended theme
*/
TL_DLL bool deleteAppTheme (TeDatabase* db, TeAbstractTheme* theme, const string& tableName="te_theme_application");

/** @} */ 

/** @defgroup Media Media Persistence
   @ingroup  DatabaseUtils
   A set of functions to manipulate the media data persistence model proposed by TerraLib  
@{
*/
/** Creates a table to store multimedia documents 
	\param db		pointer do a TerraLib database
	\param tname	name of the table to be created 
*/
TL_DLL bool createMediaTable(TeDatabase* db, const string& tname);

/** Creates a table to store the association between objects of a layer and its multimedia attributes
	\param db		pointer do a TerraLib database
	\param layerId	identifier of the layer that will contain the media
	\param tname	name of the table to be created 
*/
TL_DLL bool createLayerMediaTable(TeDatabase* db, int layerId, const string& tname);

/** Inserts a new media file in a database
	\param db			pointer do a TerraLib database
	\param tableName	identifier of the layer that will contain the media
	\param fileName		name of the file that contains the media 
	\param description	description of the media
	\param type			type of the media
	\param blob			flag to indicate that the media should be turned to a blob in the databse
*/
TL_DLL bool insertMedia(TeDatabase* db, const string& tableName, string& fileName, const string& description, const string& type, bool blob=true);

/** Associates a media data to an object of a layer
	\param db			pointer do a TerraLib database
	\param layer		layer that contains the object
	\param objectId		identifier of the object 
	\param mediaName	name of the media
	\param mediaTable	name of the table tha contains the media
*/
TL_DLL bool insertMediaObjLayer (TeDatabase* db, TeLayer* layer, const string& objectId, const string& mediaName, const string& mediaTable);

/** Sets a media as a default media of an object of a layer
	\param db			pointer do a TerraLib database
	\param layer		layer that contains the object
	\param objectId		identifier of the object 
	\param mediaName	name of the media
	\param mediaTable	name of the table tha contains the media
*/
TL_DLL bool setDefaultMedia(TeDatabase* db, TeLayer* layer, const string& objectId, const string& mediaName, const string& mediaTable);

/** Updates a media description in a database
	\param db			pointer do a TerraLib database
	\param mediaName	name of the media
	\param mediaTable	name of the table tha contains the media
	\param desc			description of the media
*/
TL_DLL bool updateMediaDescription(TeDatabase* db, const string& mediaName, const string& mediaTable, const string& desc);

//! Inserts a new general information of a layer or database
/*
	\param db			pointer do a TerraLib database
	\param layerId		if < 0 it is database information, else it is layer information
	\param tableName	table name
	\param fileName		name of the file that contains the media information
	\param type			file type. It can be ".TXT", ".DOC", ".MP3", etc. If URL it should be ".URL"
*/
TL_DLL bool insertInfo2Blob(TeDatabase* db, int layerId, const string& tableName,  const string& fileName,  const string& type);

//! read blob and copy to temporary file
/*
	\param db			pointer do a TerraLib database
	\param layerId		if < 0 it is database information, else it is layer information
	\param tableName	table name
	\param type			file type. It can be ".TXT", ".DOC", ".MP3", etc. If URL it should be ".URL"
	\return the temporary file that contains the blob
*/
TL_DLL string getInfoBlob2TemporaryFile(TeDatabase* db, int layerId, const string& tableName, string& type);
/** @} */ 

/** @defgroup PBCharts Pie/Bar Charts
  @ingroup  DatabaseUtils
  A set of functions to manipulate the charts (pie and/or bar) persistence model used by TerraView application.
  It can be used as a model for other TerraLib based applications.
  @{
 */
/** Creates a table to store the parameters of bar/pie charts 
	\param db			pointer do a TerraLib database
	\param integrity	flag to indicate a integrity relation (with the theme) should be created 
*/
TL_DLL bool createChartParamsTable(TeDatabase* db, bool integrity=false);

/** Updates the color associated to a selection mode for a theme
	\param db		pointer do a TerraLib database
	\param themeId	theme extended identifier
	\param sel		selection mode
	\param color	color definition
*/
TL_DLL bool updateGridColor(TeDatabase* db, int themeId, TeObjectSelectionMode sel, TeColor& color);
/** @} */ 

/** @defgroup Kernel Map Kernel Metadata Functions
    @ingroup  DatabaseUtils
 *  A set of functions to persist in a TerraLib database the metadata associated to Kernel Map algorithm.
 *	\sa TeKernelParams.h
 *  @{
 */
/** Creates a table to store information about the results of the Kernel map algorithm
	\param db pointer do a TerraLib database
*/
TL_DLL bool createKernelMetadataTable(TeDatabase* db);

/** Saves the parameters associated to the execution of a Kernel Map algorithm in a TerraLib database
	\param db			pointer do a TerraLib database
	\param kernelPar	reference to a structure of parameters associated to a Kernel Map 
*/
TL_DLL bool insertKernelMetadata(TeDatabase* db, TeKernelParams& kernelPar);

/** Updates in the database the parameters associated to the execution of a Kernel Map  -- inserts if it does not exist
	\param db			pointer do a TerraLib database
	\param kernelPar	reference to a structure of parameters associated to a Kernel Map 
*/
TL_DLL bool updateKernelMetadata(TeDatabase* db, TeKernelParams& kernelPar);

/** Loads some stored Kernel parameters associated to a theme used as the support to the Kernel Map
	\param db			pointer do a TerraLib database
	\param supThemeId	identifier of the theme that has the support to the Kernel Map
	\param supTableName	name of the attribute table has stores the Kernel result
	\param supAttrName	name of the column table has stores the Kernel result
	\param KernelPar	reference to a structure of parameters associated to a Kernel Map	
*/
TL_DLL bool loadKernelParams (TeDatabase* db, int supThemeId, const string& supTableName, const string& supAttrName, TeKernelParams& KernelPar);

/** Loads some stored Kernel parameters associated to a layer 
	\param db			pointer do a TerraLib database
	\param LayerId		identifier layer
	\param KernelPar	reference to a structure of parameters associated to a Kernel Map	
*/
TL_DLL bool loadKernelParams (TeDatabase* db, int LayerId, TeKernelParams& KernelPar);
/** @} */ 

/** @defgroup Metadata Layer and Attribute table metadata
    @ingroup  DatabaseUtils
    A set of functions to persist application dependent metadata about layers and attribute tables
  @{
 */
/** Creates a table to store some information about a layer layers 
	The table is called te_layer_metadata and has the attributes: [layer_id, name, author, source, description, date_, hour_, transf]
	\param db	pointer do a TerraLib database
*/
TL_DLL int createTeLayerMetadata(TeDatabase* db);

/** Creates a table to store metadata about attribute tables 
	The table is called te_table_metadata and has the attributes: [table_id, name, author, source, quality, description, date_, hour_, transf]
	\param db	pointer do a TerraLib database
*/
TL_DLL int createTeTableMetadata(TeDatabase* db);
/** @} */ 


/** Removes tables that contains the visual of texts associated to a layer
	\param layer	pointer do a TerraLib layer
*/
TL_DLL bool deleteTextVisual(TeLayer* layer);


/** Delete the objects of a layer that are pointed in a theme
	\param theme	pointer to the extended theme were the objects are pointed
	\param box		to return the bounding box that contains the objects that were deleted
	\return the error message of the database when the function fails
*/
TL_DLL string deletePointedObjects(TeTheme* theme, TeBox& box);

/** Inserts a new object into a theme
	\param theme	pointer to a theme (the object is already in the layer)
	\param p		labels position location
	\param newId	the identifier of the object
	\param newTId	the identifier of the instance of the object (for temporal objects)
*/
TL_DLL string insertObjectIntoCollection(TeTheme* theme, TeCoord2D p, string newId, string newTId = "");
/** @} */ 

/** @defgroup GPM Generalized Proximity Matrix
    @ingroup  DatabaseUtils
    Functions to persist a generalized proximity matrix
	\sa TeGeneralizedProxMatrix
 *  @{
*/
/** Creates a table to store information about the generalized proximity matrix
	\param db			pointer do a TerraLib database
	\param integrity	flag to indicate a integrity relation (with the theme) should be created 
*/
TL_DLL bool createGPMMetadataTable(TeDatabase* db, const bool& integrity=true);

/** Creates a table to store information about the neighbourhood of a the objects selected in a theme
	\param db			pointer do a TerraLib database
	\param tableName	name of the table 
*/
TL_DLL bool createNeighbourhoodTable(TeDatabase* db, const string& tableName);

/** Persists a generalized proximity matrix in a TerraLib database
	\param db	pointer do a TerraLib database
	\param gpm	a proximity matrix
	\param objs	vector with the identifier of the objects in the matrix
*/
template<typename set> 
bool insertGPM(TeDatabase* db, TeGeneralizedProxMatrix<set>* gpm, vector<string>& objs);

/** Loads from the database the dafault GPM associated to a theme
	\param db		pointer do a TerraLib database
	\param themeId	identification of the theme
	\param gpm		a structure of a GPM in memory
*/
template<typename set> 
bool loadDefaultGPM(TeDatabase* db, const int& themeId, TeGeneralizedProxMatrix<set>* &gpm);

/** Delete all generalized proximity matrix of a theme (neighbourhoodTable and metadata) or only the gpm with a specific id (gpmId)
	\param db		pointer do a TerraLib database
	\param themeId	identification of the theme
	\param gpmId	GPM identifier
*/
TL_DLL bool deleteGPMTable(TeDatabase* db, const int& themeId, const int& gpmId=-1);
/** @} */ 

/** @} */ // end of group  DatabaseUtils


/** Loads the parameters of an extended theme
	\param db			a pointer to a TerraLib database
	\param theme		pointer to an instance of an extended theme
	\param tableName	name of the table that persists the extended theme
*/
TL_DLL bool loadAppTheme (TeDatabase* db, TeAppTheme *theme, const string& tableName="te_theme_application");

/** Saves the parameters of an extended theme 
	\param db			a pointer to a TerraLib database
	\param theme		pointer to an instance of an extended theme
	\param tableName	name of the table that persists the extended theme 
*/
TL_DLL bool insertAppTheme (TeDatabase* db, TeAppTheme *theme, const string& tableName="te_theme_application");

/** Updates the parameters of an extended theme
	\param db			a pointer to a TerraLib database
	\param theme		pointer to an instance of an extended theme
	\param tableName	name of the table that persists the extended theme 
*/
TL_DLL bool updateAppTheme (TeDatabase* db, TeAppTheme *theme, const string& tableName="te_theme_application");

/** Saves in the database the color parameters associated to a theme
	\param db			pointer do a TerraLib database
	\param theme	pointer to a theme as defined by TerraView
*/
TL_DLL bool insertChartParams(TeDatabase* db, TeAppTheme *theme);

/** Deletes the chart parameters of a theme in the database
	\param db			pointer do a TerraLib database
	\param theme	pointer to a theme as defined by TerraView
*/
TL_DLL bool deleteChartParams (TeDatabase* db, TeAppTheme *theme);

/** Verifies if there is a pie/bar char around a given location, for a given theme and select it
	\param db		pointer do a TerraLib database
	\param theme	pointer to a theme as defined by TerraView
	\param pt		a location 
	\param delta	a tolerance around the location
*/
TL_DLL bool locatePieBar (TeDatabase* db, TeAppTheme* theme, TeCoord2D pt, double delta);

/** Loads the charts parameters associated to a theme
	\param db		pointer do a TerraLib database
	\param theme	pointer to a theme as defined by TerraView
*/
TL_DLL bool loadChartParams (TeDatabase* db, TeAppTheme *theme);

/** Updates the location of the selected pie/bar chart of a theme
	\param db		pointer do a TerraLib database
	\param theme	pointer to a theme as defined by TerraView
	\param pt		the new location for the chart
*/
TL_DLL bool updatePieBarLocation (TeDatabase* db, TeAppTheme* theme, TeCoord2D pt);

/** Updates the location of selected object of an extended theme with TePoint geometry
	\param db		pointer do a TerraLib database
	\param theme	pointer to an extended theme
	\param pt		new location of the point
*/
TL_DLL bool updatePointLocation (TeDatabase* db, TeAppTheme* theme, TeCoord2D pt);


//! generate Count legends 
TL_DLL bool generateCountLegends(TeAppTheme* theme, string table, string field);

TL_DLL void changeObjectStatus(ObjectInfo& info, string& uniqueId, string mode, int pointed = 1);

TL_DLL bool saveObjectStatus(TeAppTheme *theme, set<string>& objectSet, map<string, ObjectInfo>& objectMap);

TL_DLL TeStatisticType getStatisticFromString(const string& s);


TL_DLL void getVisual(TeAppTheme* theme, TeGeomRep geomRep,
			   map<string, TeVisual>& objVisualMap, set<string>& oidSet, bool allObjects);


// Implementation of the template functions
template<typename set>
bool insertGPM(TeDatabase* db, TeGeneralizedProxMatrix<set>* gpm, vector<string>& objs)
{
	if(!db)
		return false; 

	TeProxMatrixConstructionParams* cParams = gpm->getConstructionParams();
	TeProxMatrixSlicingParams*	sParams = gpm->getSlicingParams();
	TeProxMatrixWeightsParams*   wParams = gpm->getWeightsParams();

	if((!cParams) || (!sParams) || (!wParams))
		return false;
		
	//update todas as gpms desse tema para default igual a zero
	string up =  " UPDATE te_gpm SET gpm_default = 0 WHERE theme_id1 = "+ Te2String(cParams->theme_id1_);
	db->execute (up);

	// ----- progress bar
	int step = 0;
	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(objs.size());
	// -----
	
	//insere na tabela de metadados 
	string ins = " INSERT INTO te_gpm (gpm_default, theme_id1, geom_rep1, theme_id2, ";
	ins += " geom_rep2, neighbours_table, construction_date, construction_strategy, ";
	ins += " const_max_distance, const_num_neighbours, const_max_net_distance, ";
	ins += " const_max_con_distance, slicing_strategy, slicing_zone_dist, ";
	ins += " slicing_zone_local, weight_strategy, weight_norm, weight_a, ";
	ins += " weight_b, weight_c, weight_factor, weight_dist_raio, weigth_max_local_dist ) ";
	ins += " VALUES ( ";
	ins += " 1 ";
	ins += ", "+ Te2String(cParams->theme_id1_);
	ins += ", "+ Te2String((int)cParams->geom_rep1_);
	
	if(cParams->theme_id2_<0)
		ins += ", NULL ";
	else
        ins += ", "+ Te2String(cParams->theme_id2_);
	
	ins += ", "+ Te2String((int)cParams->geom_rep2_);
	ins += ", '"+ gpm->neighbourhood_table_ +"'";
	ins += ", NULL ";
	ins += ", "+ Te2String((int)cParams->strategy_); 
	ins += ", "+ Te2String(cParams->max_distance_, 8);
	ins += ", "+ Te2String(cParams->num_neighbours_);
	ins += ", "+ Te2String(cParams->max_net_distance_, 8);
	ins += ", "+ Te2String(cParams->max_connection_distance_, 8);
	ins += ", "+ Te2String((int)sParams->strategy_); 
	ins += ", "+ Te2String(sParams->zone_dist_, 8);
	ins += ", "+ Te2String((int)sParams->zone_local_);
	ins += ", "+ Te2String((int)wParams->strategy_);
	ins += ", "+ Te2String((int)wParams->norm_);
	ins += ", "+ Te2String(wParams->a_, 8);
	ins += ", "+ Te2String(wParams->b_, 8);
	ins += ", "+ Te2String(wParams->c_, 8);
	ins += ", "+ Te2String(wParams->factor_, 8);
	ins += ", "+ Te2String(wParams->dist_ratio_, 8);
	ins += ", "+ Te2String(wParams->max_local_dist_, 8);
	ins += " )";

	if(!db->execute (ins))
	{
		if (TeProgress::instance())
			TeProgress::instance()->reset();
		return false;
	}

	//insere na tabela de vizinhos
	ins = " INSERT INTO "+ gpm->neighbourhood_table_;
	ins += " (object_id1, object_id2, centroid_distance, weight, ";
	ins += " slice, order_neig, borders_length, net_objects_distance, ";
	ins += " net_minimum_path ) VALUES ( ";

	vector<string>::iterator it = objs.begin(); 
	while(it != objs.end())
	{
		string objId1 = (*it);

		//get the neighbours
		TeNeighboursMap neigs = gpm->getMapNeighbours(objId1);
		TeNeighboursMap::iterator itNeig = neigs.begin();
		while(itNeig != neigs.end())
		{
			string objId2 = itNeig->first;
			TeProxMatrixAttributes attributes = itNeig->second;
			//sql
			string sql = "'"+ objId1 +"'";
			sql += ", '"+ objId2 +"'";
			sql += ", "+ Te2String(attributes.CentroidDistance(), 8);
			sql += ", "+ Te2String(attributes.Weight(), 8);
			sql += ", "+ Te2String(attributes.Slice());
			sql += ", "+ Te2String(attributes.Order());
			sql += ", "+ Te2String(attributes.BorderLength(), 8);
			sql += ", "+ Te2String(attributes.NetworkObjectsDistance(), 8);
			sql += ", "+ Te2String(attributes.NetworkMinimumPath(), 8);
			sql += " )";

			if(!db->execute(ins + sql))
			{
				if (TeProgress::instance())
					TeProgress::instance()->reset();
				return false;
			}
			++itNeig;
		}

		//progress bar
		if(TeProgress::instance())
		{
			if (TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			else
				TeProgress::instance()->setProgress(step);
		}	
		++step;
		++it;
	}
	if (TeProgress::instance())
		TeProgress::instance()->reset();
	return true;
}

template<typename set>
bool loadDefaultGPM(TeDatabase* db, const int& themeId, TeGeneralizedProxMatrix<set>* &gpm)
{
	if(!db)
		return false;

	TeDatabasePortal* portal = db->getPortal();
	if(!portal)
		return false;

	//load the table name
	string sql = " SELECT gpm_id, neighbours_table, weight_strategy, weight_norm, ";
	sql += " weight_a, weight_b, weight_c, weight_factor, weight_dist_raio, ";
	sql += " weigth_max_local_dist FROM te_gpm ";
	sql += " WHERE theme_id1 = "+ Te2String(themeId) +" AND gpm_default = 1";
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	string table = portal->getData(1);
	int gpmId = portal->getInt(0);
	int weightStrategy = portal->getInt(2); 

	//load weight strategy
	TeProxMatrixWeightsStrategy* ws = 0;
	if((TeGPMWeightsStrategy)weightStrategy==TeNoWeightsStrategy)
		ws = new TeProxMatrixNoWeightsStrategy((bool)portal->getInt(3));

	else if((TeGPMWeightsStrategy)weightStrategy==TeInverseDistanceStrategy)
		ws = new TeProxMatrixInverseDistanceStrategy (portal->getDouble(4), portal->getDouble(5), 
				portal->getDouble(6), portal->getDouble(7), (bool)portal->getInt(3));

	else if((TeGPMWeightsStrategy)weightStrategy==TeSquaredInverseDistStrategy)
		ws = new TeProxMatrixSquaredInverseDistanceStrategy (portal->getDouble(4), portal->getDouble(5), 
				portal->getDouble(6), portal->getDouble(7), (bool)portal->getInt(3));

	else if((TeGPMWeightsStrategy)weightStrategy==TeConnectionStrenghtStrategy)
		ws = new TeProxMatrixConnectionStrenghtStrategy(portal->getDouble(8), portal->getDouble(9), 
				portal->getDouble(7),(bool)portal->getInt(3));

	portal->freeResult();

	sql= " SELECT count(*) FROM "+ table;
	if(!portal->query(sql))
	{
		delete portal;
		return false;
	}
	int numSteps = atoi(portal->getData(0));
	int step = 0;
	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(numSteps);
	
	portal->freeResult();

	sql= " SELECT object_id1, object_id2, centroid_distance, weight, ";
	sql += " slice, order_neig, borders_length, net_objects_distance, ";
	sql += " net_minimum_path FROM "+ table;
	if(!portal->query(sql))
	{
		delete portal;
		return false;
	}

	TeProxMatrixImplementation* impl = TeProxMatrixAbstractFactory::MakeConcreteImplementation();
	while(portal->fetchRow())
	{
		TeProxMatrixAttributes attr(portal->getDouble(3), portal->getInt(4), 
			portal->getInt(5),  portal->getDouble(2), portal->getDouble(6), 
			portal->getDouble(7), portal->getDouble(8));
		
		impl->connectObjects (string(portal->getData(0)), string(portal->getData(1)), attr);

		if(TeProgress::instance())
		{
			if (TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				delete portal;
				return false;
			}
			else
				TeProgress::instance()->setProgress(step);
		}	
		++step;
	}
	if (TeProgress::instance())
		TeProgress::instance()->reset();

	gpm = new TeGeneralizedProxMatrix<set>(gpmId, table, impl, ws);

	delete portal;
	return true;
}

template<typename T1>
TeCoord2D calculateLabelPosition(const T1& geomSet)
{
	return TeCoord2D(0., 0.);
};

template<> TeCoord2D calculateLabelPosition(const TePolygonSet& geomSet);

template<> TeCoord2D calculateLabelPosition(const TeLine2D& geomSet);

/** @brief
 */
TL_DLL bool getNextObject(TeDatabasePortal* portal, TePolygonSet& polys, TePolygon& nextPoly);

/** @brief This function generates the position of text labels.
 * 
 * It calculates the label position using the center of geometry box. This coordinate is calculated
 * as follows:
 * <ul>
 *   <li> If the geometry is a polygon, the label is located at the polygon box center. If this coordinate
 *	isn't within the polygon, recalcultates coordinate. The new label position will be the polygon centroid.
 *   </li>
 *   <li> If the polygon has islands, the label position will be located within the polygon with greater
 *  area.
 *   </li>
 *   <li> If the geometry is a line, the label is located ate the line box center. If this coordinate isn't
 * located at the line, recalculates the coordinate. The new label position will be the center point of the line.
 *   </li>
 *   <li> If the geometry is a point, the label will be the point. </li>
 * </ul>
 *
 * @param theme The theme to generate labels positions.
 * @param objectId Object identifier. This parameter is used, if the label position must be generated for
 * a particular object.
 * @return True if everything is ok, false if something fails.
 */
TL_DLL bool generateTextUsingCentroid(TeTheme* theme, const std::string& objectId = "");

#endif



