/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2004 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeCellAlgorithms.h
    This file contains algorithms and operations to deal with on cell geometry
*/

#ifndef  __TERRALIB_INTERNAL_CELLALGORITHMS_H
#define  __TERRALIB_INTERNAL_CELLALGORITHMS_H

#include "../kernel/TeSTElementSet.h"
#include "../kernel/TeComputeAttributeStrategies.h"
#include "../kernel/TeRaster.h"

#include "TeFunctionsDefines.h"

class TeDatabase; 
class TeDatabasePortal;
class TeLayer;
class TeGraphNetwork;
class TeTable;

using namespace std;

//! This class represents a raster iterator using a box as spatial restriction
class TLFUNCTIONS_DLL TeBoxRasterIterator
{
	public:
		//! Constructor
		TeBoxRasterIterator(TeRaster* raster_, TeBox box); 

	    //! Prefix move forward operator
	    TeBoxRasterIterator& operator++();

	    //! Prefix move forward operator
	    TeBoxRasterIterator operator++(int);

	    //! Dereferencing operator
	    double operator*();

		//! Equal operator
        bool operator==(const TeBoxRasterIterator& rhs) const;

		//! Differ operator
        bool operator!=(const TeBoxRasterIterator& rhs) const; 

		//! getProperty
		bool getProperty(TeProperty& prop);
        
		//! Returns an iterator that points to the first position of the raster inside the box
		TeBoxRasterIterator begin(); 
		
		//! Returns an iterator that points to the last position of the raster inside the box
		TeBoxRasterIterator end();

	private:
		int xo, yo, xf, yf, x, y;
		bool end_;
		TeRaster* raster;
};

/** @defgroup CellSpaces Cell Spaces 
  @{
 */

/** @defgroup CreateCells Cells generation
    @ingroup CellSpaces
  @{
 */
//! Creates a new layer with the cells that covers the polygons of a theme  
/*	\param layerName	name of the new layer with cell representations
	\param theme		pointer to the theme that has the polygons
	\param resX			X resolution of each cell
	\param resY			Y resolution of each cell
	\param box			bounding box of interest
	\return a pointer to the new layer created if success or null if fail
*/
TLFUNCTIONS_DLL TeLayer* TeCreateCells(const string& layerName,TeTheme* theme, double resX, double resY, TeBox& box);

//! Creates a new layer with the cells that covers the entire bounding box, or the polygons  of a layer 
/*	\param layerName	name of the new layer with cell representations
	\param layerBase	pointer to the base layer
	\param resX			X resolution of each cell
	\param resY			Y resolution of each cell
	\param mask			flag that indicates if the cells should be created only over the polygons of the layer
	\param box			bounding box of interest
	\return a pointer to the new layer created if success or null if fail
*/
TLFUNCTIONS_DLL TeLayer* TeCreateCells( const string& layerName, TeLayer* layerBase, 
			   double resX, double resY, TeBox& box, bool mask = true);

/** @} */ 

/** @defgroup CellStatistics Cells statistics 
    @ingroup CellSpaces
  @{
 */
//! Calculates a set of statistics of cells of a theme 
/*	\param themeOut			cell theme 
	\param newAttrTable		new attribute table where the generated statistics will be kept 
	\param stat				the structure that contains the statistics which will be generated 
	\param themeIn			point or cell theme
	\param db				a pointer to the database
	\return true if the operation was successfully done  
*/
TLFUNCTIONS_DLL bool 
TeCellStatistics(TeTheme* themeOut, TeTable& newAttrTable, TeGroupingAttr& stat, 
				 TeTheme* themeIn, TeDatabase* db);

//! Saves the statistics accessible through a TeDatabasePortal in a database table 
/*	\param tableAttrCell	attribute table name where the generated statistics will be kept 
	\param portal			a pointer to the database portal that contains the generated statistics  
	\return true if the operation was successfully done  
*/
TLFUNCTIONS_DLL bool TeKeepStatistics(TeTable& tableAttrCell, TeDatabasePortal* portal);

/** @} */ 

/** @defgroup FillCellAlg Cells attributes generation
    @ingroup CellSpaces
	Fill one attribute (or group of related attributes) for all cells in the output layer 
	based on a single input layer attribute for a given time interval. Specific operation to be applied is a parameter (Strategy pattern).
  @{
 */

//! Calculates an attribute based on the exact Euclidean distance. This strategy is separated from the others because it 
//! calculates the distance using an already calculated distance of one of its neighbours.
/*	\param input_theme				a pointer to the input theme to calculate the distances
	\param rep						input theme representation to be used
	\param cell_layer				a pointer to the output cell layer  
	\param cell_tablename			output cell attribute dynamic table; if does not exist, the table will be created by the function  
	\param output_columnname		output attribute name   
	\param time_interval			time interval of output cells attributes
	\return true if the operation was successfully done  
*/
TLFUNCTIONS_DLL bool TeFillCellDistanceOperation(TeTheme* input_theme,
								 TeGeomRep rep,
								 TeLayer* cell_layer,
								 const string& cell_tablename,
								 const string& output_columnname,
								 TeTimeInterval time_interval = TeTimeInterval());

//! suppose that each cell is completely filled by the used geometry, and there is no overlapping geometries. Creates one attribute 
//! for each unique value, and this values are used in the name of the created attributes, in the form attrName concatenated 
//! with the value
/*	\param input_theme				a pointer to the input theme to calculate the distances
	\param rep						input theme representation to be used
	\param cell_layer				a pointer to the output cell layer  
	\param cell_tablename			output cell attribute table; if does not exist, the table will be created by the function  
	\param output_columnName		output attribute name   
	\param time_interval			time interval of output cells attributes
	\return true if the operation was successfully done  
*/

TLFUNCTIONS_DLL bool TeFillCellCategoryCountPercentageRasterOperation(TeDatabase* database,
													  string raster_layername,
													  string cell_layername,
													  const string& cell_tablename,
													  const string attrName,
													  TeTimeInterval t);



TLFUNCTIONS_DLL bool TeFillCellCategoryAreaPercentageOperation(TeTheme* theme,
											   const string attrName,
								 			   TeLayer* cell_layer,
											   const string& cell_tablename,
											   TeTimeInterval t = TeTimeInterval());


TLFUNCTIONS_DLL bool TeFillCellCategoryAreaPercentageOperation(TeTheme* input_theme,
											   const string attr_name,
								 			   TeLayer* cell_layer,
											   const string& cell_tablename,
											   map<string, string>& classesMap,
											   TeTimeInterval time_interval = TeTimeInterval());

//! Computes an attribute based on a TeComputeSpatialStrategy 
/*	\param db						the database which contains the layer of cells
	\patam operation				the operation used to compute the attribute
	\param cell_layername			the name of the layer of cells
	\param cell_tablename			output cell attribute table; if does not exist, the table will be created by the function  
	\param output_columnname		output attribute name
	\param time_interval			time interval of output cells attributes
	\return true if the operation was successfully done  
*/
TLFUNCTIONS_DLL bool TeFillCellSpatialOperation(TeDatabase* db,
                        		TeComputeSpatialStrategy* operation,
								const string& cell_layername,
                        		const string& cell_tablename,
                        		const string& output_columnname,
								TeTimeInterval time_interval = TeTimeInterval());

//! Computes the cell attribute based on a non-spatial operation (TeComputeAttrStrategy) over a vector or cell input layer.
/*	\param db						a pointer to the database in which input and output layers are stored
	\param input_layername			name of input layer 
	\param rep						input layer representation to be used
	\param input_tablename			input layer attribute table to be used  
	\param input_attrname			input layer attribute to be used to compute output attribute
	\param operation				operation to be applied (see TeComputeAttrStrategy hierarchy)  
	\param cell_layername			output cell layer  
	\param cell_tablename			output cell attribute dynamic table; if does not exist, the table will be created by the function  
	\param output_columnName		output attribute name   
	\param time_interval			time interval of output cells attributes
	\return true if the operation was successfully done  
*/
TLFUNCTIONS_DLL bool TeFillCellNonSpatialOperation(TeDatabase* db,
									const string& input_layername, 
									TeGeomRep rep,
									const string& input_tablename, 
									const string& input_attrname,
									TeComputeAttrStrategy<TeSTElementSet::propertyIterator>* operation,
									const string& cell_layername, 
									const string& cell_tablename, 
									const string& output_columnName,
									TeTimeInterval time_interval = TeTimeInterval());

//! Computes the cell attribute based on a non-spatial operation (TeComputeAttrStrategy) over a vector or cell input layer.
/*	\param theme					a pointer to an input theme
	\param rep						input theme representation to be used
	\param input_tablename			input theme attribute table to be used  
	\param input_attrname			input theme attribute to be used to compute output attribute
	\param operation				operation to be applied (see TeComputeAttrStrategy hierarchy)  
	\param cell_layername			output cell layer  
	\param cell_tablename			output cell attribute dynamic table; if does not exist, the table will be created by the function  
	\param output_columnName		output attribute name   
	\param time_interval			time interval of output cells attributes
	\return true if the operation was successfully done  
*/
TLFUNCTIONS_DLL bool TeFillCellNonSpatialOperation (TeTheme* theme,
									TeGeomRep rep,
									const string& input_attrname,
									TeComputeAttrStrategy<TeSTElementSet::propertyIterator>* operation,
									TeLayer* cell_layer,
									const string& cell_tablename, 
									const string& output_columnName,
									TeTimeInterval time_interval = TeTimeInterval());

//! Computes the cell attribute based on a non-spatial operation (TeComputeSpatialStrategy) over a raster input layer.
/*	\param db						a pointer to the database in which input and output layers are stored
	\param input_raster_layername	name of input layer that contains a raster representation 
	\param operation				operation to be applied (see TeComputeAttrStrategy hierarchy)  
	\param cell_layername			output cell layer  
	\param cell_tablename			output cell attribute dynamic table; if does not exist, the table will be created by the function  
	\param output_columnName		output attribute name   
	\param time_interval			time interval of output cells attributes
	\return true if the operation was successfully done  
*/
TLFUNCTIONS_DLL bool TeFillCellNonSpatialRasterOperation(TeDatabase* db,
										const string& input_raster_layername, 
										TeComputeAttrStrategy<TeBoxRasterIterator>* operation,
										const string& cell_layername, 
										const string& cell_tablename, 
										const string& output_columnName,
										TeTimeInterval time_interval = TeTimeInterval());

//! Computes the cell attribute based on a non-spatial operation (TeComputeSpatialStrategy) over a certain attribute of a set of objects connected to each cell by a network.
//! This function will change in the next version of TerraLib
/*  \param db                       a pointer to the database in which input and output layers are stored
    \param objects                  set of objects (e.g., ports) 
    \param rep                      objects representation type
    \param net                      netork to be used to verify and quantify cell connectivity to each cell
    \param input_attrname           connected objects attribute to be used to compute output attribute
    \param t                        time interval of output cells attributes
    \param cell_layername           output cell layer  
    \param cell_tablename           output cell attribute dynamic table; if does not exist, the table will be created by the function  
    \param output_columnName        output attribute name   
    \param operation                operation to be applied (see TeComputeAttrStrategy hierarchy)  
    \param local_distance_factor    multiplier used to compute connection strenght weights: local distance factor (see GPM documentation) 
    \param net_distance_factor      multiplier used to compute connection strenght weights: distance to network factor (see GPM documentation) 
    \param net_conn_factor          multiplier used to compute connection strenght weights: connection through network factor (see GPM documentation) 
    \param mult_factor              multiplier used to compute connection strenght weights: weight multiplier factor (see GPM documentation) 
*/
TLFUNCTIONS_DLL bool TeFillCellConnectionOperation (TeDatabase* db,
                                    TeSTElementSet objects,
                                    TeGeomRep rep,
                                    TeGraphNetwork* net,
                                    const string& /*input_attrName*/,
                                    TeTimeInterval t,
                                    const string& cell_layername,
                                    const string& cell_tablename,
                                    const string& /*output_columnName*/,
                                    TeComputeAttrStrategy<TeSTElementSet::propertyIterator>* operation,
                                    double /*local_distance_factor = 0.0*/,
                                    double /*net_distance_factor = 0.25*/,
                                    double /*net_conn_factor = 1.0*/,
                                    double /*mult_factor = 1000000.0*/);



//! Computes a set of attributes for the output cell layer based on a non-spatial operation (TeComputeAttrSpatialStrategy) over an input cell layer.
/*	\param db						a pointer to the database in which input and output layers are stored
	\param input_layername			name of input cell layer 
	\param input_tablename			input layer attribute table to be used  
	\param operation				operation to be applied (see TeComputeAttrStrategy hierarchy)  
	\param attrNames				names of input/output attribute names to be aggregated   
	\param cell_layername			output cell layer  
	\param cell_tablename			output cell attribute dynamic table; if does not exist, the table will be created by the function  
	\param time_interval			time interval of output cells attributes
	\return true if the operation was successfully done  
*/
TLFUNCTIONS_DLL bool TeFillCellAggregateOperation(TeDatabase* db,
									const string& input_layername, 
									const string& input_tablename, 
									TeComputeAttrStrategy<TeSTElementSet::propertyIterator>* operation,
									vector<string>& attrNames,
									const string& cell_layername, 
									const string& cell_tablename, 
									TeTimeInterval time_interval = TeTimeInterval());
/** @} */ 

/** @defgroup FillCellAux Fill cells auxiliary functions
    @ingroup FillCellAlg
*  @{
*/
//! Load cells geometries and verify/create dynamic attribute table if does not exists
/*	\param cell_layer		a pointer to a layer of cells
	\param cell_tablename	the name of the table in the layer of cells
	\param cells			the object to be filled
	\return if the function was successfully executed
*/
TLFUNCTIONS_DLL bool TeFillCellInitLoad (TeLayer* cell_layer, const string& cell_tablename, TeCellSet& cells);


//! Fills the cell object set for a temporal table 
/*	\param cell			the cell which owns the attribute(s)
	\param result		the result of an operation to be used to fill the set
	\param cellObjSet	the set to be filled
	\param atttype		the type of the table to be filled
	\patam t			the time interval of the attribute(s)
	\return if the function was successfully executed
*/
TLFUNCTIONS_DLL void TeFillCellInitSTO (const TeCell& cell, TePropertyVector& result, TeSTElementSet& cellObjSet, TeAttrTableType atttype, TeTimeInterval& t);


//! Fills the cell object set for a temporal table 
/*	\param cell			the cell which owns the attribute(s)
	\param result		the result of an operation to be used to fill the set
	\param cellObjSet	the set to be filled
	\param atttype		the type of the table to be filled
	\patam t			the time interval of the attribute(s)
	\return if the function was successfully executed
*/
TLFUNCTIONS_DLL void TeFillCellInitSTO (const TeCell& cell, vector<string>& result, TeSTElementSet& cellObjSet, TeAttrTableType atttype, TeTimeInterval& t);


//! Creates dymanic table
/*	\param cell_layer			the layer where the table will be created
	\param cell_table_name		the name of the new table
	\return if the function was successfully executed
*/
TLFUNCTIONS_DLL bool TeCreateBasicDynamicCellTable (TeLayer* cell_layer, const string cell_tablename);

/** @} */
/** @} */
#endif

