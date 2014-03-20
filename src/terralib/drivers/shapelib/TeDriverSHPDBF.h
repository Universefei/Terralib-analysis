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
/*! \file TeSHPDBFDriver.h
    \brief This file contains the header of the functions that deal with shapefile and dbf formats
	using the shapelib library
*/
#ifndef __TERRALIB_INTERNAL_DBFSHPDRIVER_H
#define __TERRALIB_INTERNAL_DBFSHPDRIVER_H

#include "../../kernel/TeTable.h"
#include "../../kernel/TeSTElementSet.h"
#include "../../kernel/TeDataTypes.h"
#include "../../kernel/TeMultiGeometry.h"

// Using the local modified Frank-Shapelib
#include "../../../shapelib/shapefil.h"

#include "TeSHPDefines.h"

#include <string>
using namespace std;

class TeLayer;
class TeAsciiFile;
class TeProjection;
class TeDatabase;
class TePolygonSet;
class TeQuerier;
class TeTheme;
class TeSTElementSet;


/** @defgroup Formats Geographical data formats
 *  @ingroup Formats
 *  Functions related to encoding/decoding of files in the formats supported by TerraLib
 *  @{
 */

/** @defgroup DBF DBF tables
 *  @ingroup Formats
 *  Functions related to DBF table format
 *  @{
 */

/** Reads the list of attributes specified in DBF file 
   \param dbfFileName	DBF input file name
   \param attList		returns the DBF list of attributes
   \return TRUE if the list of attributes were was successfully read and FALSE otherwise
*/
SHP_DLL bool TeReadDBFAttributeList(const string& dbfFileName, TeAttributeList& attList);

/** Reads a set of records from a shapelib to a structure in memory
	\param dbfFileName	DBF file name
	\param table		to return the set of records in memory
	\param nrecords		number of records to be read
	\param rinitial		number of the initial record
	\param createAuto	flag used to indicate if an auto number columns should be added to each record
*/
SHP_DLL bool TeReadDBFAttributes (const string& dbfFileName, TeTable& table, 
						  int nrecords=-1, int rinitial=0, bool createAuto=false);

/** Imports a DBF table to a TerraLib database
	This function imports a DBF to a TerraLib database. An attribute table can be associated to a
	particular layer, or as an external table that will be linked to any layer when genereated a 
	theme. A column called object_id_ will be added with unique values automatically generated.
	\param dbfFileName	DBF file name
	\param db			pointer to a TerraLib database
	\param layer		pointer to a layer to which the table will associated
	\param tableType	type of the attribute table
*/
SHP_DLL bool TeImportDBFTable(const string& dbfFileName, TeDatabase* db, TeLayer* layer=0,TeAttrTableType tableType=TeAttrExternal, string objectIdAtt = "object_id_");

//! Decodify a row of a DBF into a TeTableRow structure 
/*! 
	\param dbfHandle pointer to a DBF handler
	\param nr number of the row
	\param row reference to a row that will store the data
*/
SHP_DLL void TeDBFRowDecode		(DBFHandle& dbfHandle, int nr, TeTableRow& row);
/** @} */ // End of subgroup DBF format

/** @defgroup Shape decode 
 *  @ingroup SHP
 *  Auxiliary functions to decode one shape using shapelib
 *  @{
 */
//! Decodify a ShapeObject into a TerraLib point set
/*! 
	\param psShape pointer to a shapelib shape object 
	\param ps TerraLib point set to receive the geometries
	\param objectId TerraLib identifier to this object
*/
SHP_DLL void TeSHPPointDecode	(SHPObject* psShape, TePointSet& ps, string& objectId);

//! Decodify a ShapeObject into a TerraLib line set
/*! 
	\param psShape pointer to a shapelib shape object 
	\param ps TerraLib line set to receive the geometries
	\param objectId TerraLib identifier to this object
*/
SHP_DLL void TeSHPPolyLineDecode(SHPObject* psShape, TeLineSet& ls, string& objectId);

//! Decodify a ShapeObject into a TerraLib polygon set
/*! 
	\param psShape pointer to a shapelib shape object 
	\param ps TerraLib polygon set to receive the geometries
	\param objectId TerraLib identifier to this object
*/
SHP_DLL void TeSHPPolygonDecode	(SHPObject* psShape, TePolygonSet& pols, string& objectId);

//! Decodify a ShapeObject into a TerraLib multigeometry set
/*! 
	\param psShape pointer to a shapelib shape object 
	\param geometries TerraLib multi geometry set to receive the geometries
	\param objectId TerraLib identifier to this object
*/
SHP_DLL bool TeDecodeShape(SHPObject* psShape, TeMultiGeometry& geomestries, string& objectId);
/** @} */ // End of Shape decode group


/** @defgroup SHP Shapefile format
 *  @ingroup Formats
 *  Functions related to Shapefile format. Uses some functions related to DBF attribute table format.
 *  @{
 */ 

/** Get the metadata information of a shapefile
	\param shpFileName	Shapefile name
	\param nShapes		to return the number of shapes in the shapefile
	\param box			to return the boundary box of the shapes contained in the shapefile
	\param rep			to return the type of geometry exist in the shapefile
	\return true if succeeds and and false otherwise
*/
SHP_DLL bool
TeGetSHPInfo(std::string shpfileName, unsigned int& nShapes, TeBox& box, TeGeomRep& rep);


/** Imports a geo data in a shapefile format to a TerraLib database
	\param shpFileName	Shapefile name
	\param db			pointer to a TerraLib database
	\param layerName	name of the layer that will contain the data (optional)
	\return a pointer to created layer if the data was imported successfully and 0 otherwise
*/
SHP_DLL TeLayer* TeImportShape(const string& shpFileName, TeDatabase* db, const string& layerName="");

/** Imports a shapefile to a layer TerraLib
	\param layer			pointer to a layer
	\param shpFileName		shapefile file name
	\param attrTableName	the name that the attribute table will be saved
	\param objectIdAttr		name of the attribute that has the identification of objects
	\param chunkSize		the number of objects in memory before save them in the layer
*/
SHP_DLL bool TeImportShape (TeLayer* layer, const string& shpFileName, string attrTableName="",  
					string objectIdAttr="", unsigned int  chunkSize = 60, const bool& useTransaction = true);

/** Exports a querier to a shapefile
	\param	querier		pointer to a valid querier
	\param baseName		name of the output shapefile
	\return TRUE if the data was successfully exported and FALSE otherwise
*/
SHP_DLL bool TeExportQuerierToShapefile(TeQuerier* querier, const std::string& baseName);

/** Exports a theme in a TerraLib database to a shapefile
	\param theme		pointer to the layer
	\param selOb		the objetct selection: TeALL, TePOINTED, TeQUERIED...
	\param baseName		name of the output shapefile. If empty, the file will have the same name as the Theme.
	\param attributes	list of attributes to be exported. If empty, the file will have all the attributes.
	\return TRUE if the data was successfully exported and FALSE otherwise
*/
SHP_DLL bool TeExportThemeToShapefile(TeTheme* theme, TeSelectedObjects selOb = TeAll, const std::string& baseName="", 
									  const std::vector<std::string> attributes = std::vector<std::string>());

/** Exports a layer in a TerraLib database to a shapefile
   \param layer			pointer to the layer
   \param baseName		name of the output shapefile. If empty, the file will have the same name as the Layer.
   \return TRUE if the data was successfully exported and FALSE otherwise
*/
SHP_DLL bool TeExportLayerToShapefile(TeLayer* layer, const string& baseName="");

/** Exports a layer in a TerraLib database to a file in MID/MIF format
   \deprecated This is an overloaded function, provided for compatibility. It behaves essentially like the above function.
*/
SHP_DLL bool TeExportShapefile(TeLayer* layer, const string& shpFileName, const string& tableName, const string& restriction = "");

/** Exports a TerraLib polygon set to a shapefile
	Each polygon will be a shape. Only one attribute will be created: the object id of the polygon.
   \param ps				the polygon set
   \param base_file_name	the base file name used to build the shapefile
   \return TRUE if the data was successfully exported and FALSE otherwise
*/
SHP_DLL bool TeExportPolygonSet2SHP( const TePolygonSet& ps,const string& base_file_name  );

//! Builds the spatial object set from database according to the restrictions previously defined
/*! 
	\param stoset the STOSet that will be filled 
	\param fileName shape file name
*/
SHP_DLL bool TeSTOSetBuildSHP(TeSTElementSet& stoset, const string& fileName);

SHP_DLL DBFHandle TeCreateDBFFile (const string& dbfFilename, TeAttributeList& attList);

SHP_DLL  void TeWriteDataProjectionToFile(TeProjection* projection, const std::string& fileName);


/** @} */ // End of subgroup SHAPEFILE format


#include "../../kernel/TeSTGeoDataDriver.h"

//! Concrete implementation of a driver shapefiles
class SHP_DLL TeShapefileDriver: public TeSTGeoDataDriver
{
	std::string shpFileName_;
	std::string dbfFileName_;
	SHPHandle	hSHP_;
	DBFHandle	hDBF_;

public:

	//! Constructor from a file name
	TeShapefileDriver(std::string fileName);

	//! Destructor
	~TeShapefileDriver();

	std::string getFileName();

	//! Returns true or false whether the data file is accessible
	bool isDataAccessible();

	//!< Initializes the access to the data
	virtual bool initData();

	//!< Initializes the access to the data
	virtual bool initMetadata();

	//!< Loads the data into the elementSet
	bool loadData();

	//! Releases internal structures and invalidate the access to the data file
	void close();
};


/** \example importShape.cpp
 Shows how to import geographical data in shapefile format
 */

/** \example importDBF.cpp
 Shows how to import a DBF table
 */

/** @} */ // End of group ImpExp format

#endif

