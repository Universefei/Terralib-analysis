/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeGeoProcessingFunctions.h
    \brief This file contains functions to generate new layers from spatial operations among themes 
*/

#ifndef __TERRALIB_INTERNAL_GEOPROCESSING_H
#define __TERRALIB_INTERNAL_GEOPROCESSING_H 

#include <vector>
#include <string>

using std::vector;
using std::string;

#include "../kernel/TeDataTypes.h"
#include "../kernel/TeAttribute.h"
#include "../kernel/TeBufferRegion.h"

#include "TeFunctionsDefines.h"

class TeAsciiFile;
class TeTheme;
class TeLayer;

/** @defgroup GeoOp GeoProcessing functions
	A set of functions that execute some common operations when dealing with geographical data
 *  @{
 */
/** Agreggates the objects of a theme generating a new layer
	\param newLayer			pointer to a valid layer already created in the database to store the result of the operation
	\param theme			pointer to the theme being aggregated
	\param agregAttrVect	a vector with the names of the attributes that should have the same value in order to aggregate two objects.
	\param attrMM			the definition of the attributes (with the measures over it) that should passed to the new layer
	\param selOb			a selection of the objects of the theme that should be aggregated
	\param logFile			Optional: a pointer to a text file where error messages will be saved
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpAggregation(TeLayer* newLayer, TeTheme* theme, vector<string>& agregAttrVec, TeGroupingAttr& attrMM, TeSelectedObjects selOb, TeAsciiFile* logFile=0);		

/** Adds a set of themes generating a new layer
	\param newLayer			pointer to a valid layer already created in the database to store the result of the operation
	\param theme			pointer to the first theme being added: the new layer will have the same attributes as the this theme
	\param themeVec			a vector of themes that should be added to the first one
	\param selOb			a selection of the objects of the theme that should be added
	\param logFile			Optional: a pointer to a text file where error messages will be saved
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpAdd(TeLayer* newLayer, TeTheme* theme, vector<TeTheme*> themeVec, TeSelectedObjects selObj = TeAll, TeAsciiFile* logFile=0);

/** Clips a theme generating a new layer
	\param newLayer			pointer to a valid layer already created in the database to store the result of the operation
	\param theme			pointer to the theme being clipped
	\param themeTrim		pointer to the theme that has objects with polygon geometry that will be used as the mask for the clipping
	\param selOb			a selection of the objects of the theme that should be clipped
	\param selObTrim		a selection of the objects of the theme that should be used as mask
	\param attrTrim			flag to indicate that the attributes of the mask theme should be included in the new layer
	\param logFile			Optional: a pointer to a text file where error messages will be saved
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpOverlayIntersection(TeLayer* newLayer, TeTheme* theme, TeTheme* themeTrim, TeSelectedObjects selOb, TeSelectedObjects selObTrim, bool attrTrim, TeAsciiFile* logFile=0);

/** Clips a theme generated from a layer with raster representation generating a new layer
	\param newLayer			pointer to a valid layer already created in the database to store the result of the operation
	\param theme			pointer to the theme being clipped
	\param themeTrim		pointer to the theme that has objects with polygon geometry that will be used as the mask for the clipping
	\param selObTrim		a selection of the objects of the theme that should be used as mask
	\param backValue		the value that should be used as the background
	\param logFile			Optional: a pointer to a text file where error messages will be saved
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpOverlayIntersection(TeLayer* newLayer, TeTheme* theme, TeTheme* themeTrim, TeSelectedObjects selObTrim, double backValue, TeAsciiFile* logFile=0);



/** Generates the geometrical difference between two themes (t1 - t2), generating a new layer
	\param newLayer			pointer to a valid layer already created in the database to store the result of the operation
	\param theme1			pointer to the first theme
	\param theme2		    pointer to the second theme 
	\param selOb			a selection of the objects of the first theme
	\param selOb			a selection of the objects of the second theme
	\param logFile			Optional: a pointer to a text file where error messages will be saved
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpOverlayDifference(TeLayer* newLayer, TeTheme* theme1, TeTheme* theme2, TeSelectedObjects selOb1, TeSelectedObjects selOb2, TeAsciiFile* logFile=0);

/** Assign Data By Location - Distribute: passes the attributes of the objects of a theme to the objects of a second theme based on a spatial relation
	\param themeMod			pointer to theme that has the attributes that will be passed
	\param theme			pointer to the theme that will receive the attributes
	\param tableName		name of the table that will store the attributes passed from the first theme
	\param spatialRelation	a spatial relation (or a combination of) that will be used as the criteria to pass the attributes between the themes
	\param attributes		a list of attributes from the first theme that should be distributes to the second theme
	\param logFile			Optional: a pointer to a text file where error messages will be saved
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpAssignDataLocationDistribute(TeTheme* themeMod, TeTheme* theme, const string& tableName, const int& spatialRelation, const vector<string>& attributes = vector<string>(), TeAsciiFile* logFile=0);

/** Assign Data By Location - Collect: collects the attributes of the objects of a theme and assign it to second theme based on a spatial relation
	\param restrTheme		pointer to theme that has the attributes that will be passed
	\param srcTheme			pointer to the theme that has the attributes that will be collected 
	\param newTableName		name of the table that will store the attributes collected
	\param measuresColl		the definition of the attributes (with the measures over it) that should passed to first theme
	\param spatialRelation	a spatial relation (or a combination of) that will be used as the criteria to pass the attributes between the themes
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpAssignByLocationCollect(TeTheme* restrTheme, TeTheme* srcTheme, const string& newTableName, TeGroupingAttr& measuresColl, const int& spatialRelation);

/** Generates a new layer with the buffers of the objects of a theme
	\param newLayer			pointer to a new layer that will receive the buffers generated
	\param themeIn			pointer to a source theme that will provide the objects to generate the buffers  
	\param selOb			define which objects of the themeIn will be used
	\param bufferType		define the buffer type: inside/outside/inside+outside
	\param bufferDistance	the buffer distance in the same unit of the object geometries 
	\param bufferLevels		the number of buffer rings
	\param numPoints		the number of points that should be used to describe the curved corners of the buffer zone
	\param doUnion			a flag to indicate if this function will dissolve barries among buffers
	\param tableName		name of the attribute table that will provide an attribute that defines the buffer distance for each object
	\param attrName			attribute name of the attribute table "tableName" that will provide the buffer distance for each object
	\param logFile			Optional: a pointer to a text file where error messages will be saved
	\param copyInputColumns If TRUE, all the columns from the iput table will be also copied to the result layer
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpBuffer(TeLayer* newLayer, TeTheme* themeIn, const TeSelectedObjects& selOb, const TeBUFFERREGION::TeBufferType& bufferType, const double& bufferDistance, 
				   const unsigned int& bufferLevels, const unsigned int& numPoints, const bool& doUnion=false, const string& tableName="", const string& attrName="", 
				   TeAsciiFile* logFile=0, const bool& copyInputColumns = true);

/** \brief Calculates the distance to the nearest neighbour (NN)
	This function calculates the distance of objetcs in the input theme to the nearest object in a neighbour candidates theme.
	For complex geometries such as polygons, cells, lines, sets of geometries or multigeometries the NN is found
	based on thedistance between the centroids of the geometries (for both input and for neighbour candidates objects).
	If themes have different projections the distance will be computed using input theme projection.
	
	\param inputTheme		pointer to the theme that contains the objects to which the nearest neighbour are being searched
	\param iselObj			a specific selection of the input theme  
	\param neighTheme		pointer to the theme that contains the candidates to NN
	\param nnselObj			a specific selection of NN theme
	\param distColName		the name of the column that will contain the distance to the NN object. If empty, the name "DIST" will be used. If
	the column should be created in a specific table, use the convention "tableName.colName", otherwise the first static table will be
	sued. If the column exists its values will be updated, if it don't the column will be created.
	\param NNIDColName		the name of the column that will contain the identification of the NN object. If empty the column won't be generated.
	\return true if it succeed and false otherwise
*/
TLFUNCTIONS_DLL bool TeGeoOpNearestNeighbour(TeTheme* inputTheme, TeSelectedObjects iselObj, 
							 TeTheme* neighTheme, TeSelectedObjects nnselObj, 
							 const string& distColName = "", const string& NNIDColName = "");


/** \internal */
TLFUNCTIONS_DLL bool TeGeoOpOverlayUnion(TeLayer* newLayer, TeTheme* theme, TeTheme* themeOverlay, TeSelectedObjects selOb, TeSelectedObjects selObOverlay, TeAsciiFile* logFile=0);
/** @} */ // End of group GeoOp 

#endif

