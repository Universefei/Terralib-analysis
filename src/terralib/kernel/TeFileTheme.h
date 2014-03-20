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
/** \file TeFileTheme.h
    \brief This file contains direct implementation of TeTheme in order to support themes 
	created directly from shapefiles.
*/

#ifndef  __TERRALIB_INTERNAL_FILETHEME_H
#define  __TERRALIB_INTERNAL_FILETHEME_H

#include "TeDefines.h"

// STL's include files
#include <map>
#include <vector>
#include <string>

// TerraLib's include files and forward declarations
#include "TeAbstractTheme.h"

class TeDatabase;
class TeProjection;
class TeGeoDataDriver;
class TeRaster;

class TL_DLL TeFileTheme : public TeAbstractTheme
{
protected:
	std::string			fileName_;		// location of the file	
	TeDatabase*			localDb_;		// a pointer to the database that stores the theme
	unsigned int		numObjects_;	// number of objects that the theme has access to
	TeProjection*		projection_;	// spatial reference of the objects in the file
	TeGeoDataDriver*	dataDriver_;	// a pointer to decoder to get an specific data
	TeRaster*			raster_;		// a pointer to a raster file

public:

	//! Default Constructor
	TeFileTheme(const std::string& name = "", TeViewNode* parent = 0, int view = 0, int id = 0);

	//! Constructor from specific parameters
	TeFileTheme(const std::string& name, TeGeoDataDriver* dataDriver, TeDatabase* localDb, TeViewNode* parent = 0, int view = 0);

	//! Copy constructor
	TeFileTheme(const TeFileTheme& other);

	//! Operator =
	TeFileTheme& operator=(TeFileTheme& rhs);	

	//! Destructor
	~TeFileTheme();

	//! Clones the object
	virtual TeViewNode* clone();

	//! Set the raste file name
	void setRasterFile(const std::string& fileName);

	//! Get a pointer to a raste file
	TeRaster* getRasterFile();

	//! Set the data driver that will handle this file
	void setDataDriver(TeGeoDataDriver* dataDriver);

	//! Get the data driver that will handle this file
	TeGeoDataDriver* getDataDriver();

	//! Give access to theme data as stored internally
    //TeSTElementSet* getData();
	
	//! Set the name of the file that contains the data
	void setFileName(const std::string& fileName);

	//! Get the name of the file that contains the data
	std::string getFileName();

	//! Set the pointer to the database where the theme is stored
	void setLocalDatabase(TeDatabase* db); 
	
	//! Get the pointer to the database where the theme is stored
	TeDatabase* getLocalDatabase(); 

// --- Methods responsible to the persistence of this kind of theme

	//! An static method to create a metadata table to store information about this type of theme
	static bool createFileThemeTable(TeDatabase* db);

	//! Load the basic information about the data set from the file
	bool retrieveMetadataFromFile();

	//! Load the objects from the file and fills the set of elements
	bool retrieveDataFromFile();

	//! Releases the internal data set
	void clearData(); 	

//--- Methods that reimplement the interface defined by the abstract theme

	//! Return a pointer to the projection of the geometries accessible by this theme
	TeProjection* getThemeProjection();

	//! Set the projection of the geometries accessible by this theme
	void setThemeProjection(TeProjection* proj);

	//! Get the number of objects acessible by this theme
	unsigned int getNumberOfObjects(); 

	//! Set the number of objects acessible by this theme
	void setNumberOfObjects(unsigned int); 

	//! Load metadata information about this theme
	bool loadMetadata(TeDatabase*);

	//! Save metadata information about this theme
	bool saveMetadata(TeDatabase* );

	//! Erase the theme metadata in the database
	bool eraseMetadata(TeDatabase* );

	//! This method is kept for compatibility reasons with the abstract theme it doesn't do anything
	bool save();

	//! Save the grouping parameters in memory when there is no chronon
	bool buildGrouping(const TeGrouping& g, TeSelectedObjects selectedObjects = TeAll, vector<double>* dValuesVec = 0);

	//! Save the grouping parameters in memory when there is chronon
	bool buildGrouping(const TeGrouping& g, TeChronon chr, vector<map<string, string> >& mapObjValVec);

	//! Build the grouping and associate each object to its group 
	bool saveGrouping(TeSelectedObjects selectedObjects = TeAll);

	//! Delete grouping
	bool deleteGrouping(); 

	//! Set the legend id for each object of the theme 
	void setLegendsForObjects();

	//! Set the own legend id for each object of the theme 
	void setOwnLegendsForObjects();

/** @name Locate geometries
	Returns the geometry(ies) of the theme given coordinate
*/
//@{
	bool locatePolygon(TeCoord2D &pt, TePolygon &polygon, const double& tol);

	bool locatePolygonSet(TeCoord2D &pt, double tol, TePolygonSet &polygons);
	
	bool locateLine(TeCoord2D &pt, TeLine2D &line, const double& tol);

	bool locatePoint(TeCoord2D &pt, TePoint &point, const double& tol);

	bool locateCell(TeCoord2D &pt, TeCell &c, const double& tol);
//@}

	//! Get the set of objects corresponding to the object selection criteria
	set<string> getObjects(TeSelectedObjects selectedObjects = TeAll);

	//! Get the set of objects corresponding to the list of items
	set<string> getObjects(const vector<string>& itemVec);

	//! Get the set of items corresponding to the object selection criteria
	vector<string> getItemVector(TeSelectedObjects selectedObjects);

	//! Get the set of items corresponding to the set of objects
	vector<string> getItemVector(const set<string>& oidSet);

protected:

	//! Should be reimplemented to set all the parameters and pre-requisites necessaries to copy a theme to the given database. This method will be called before the theme is saved in the database.
	virtual bool beforeCopyThemeTo(TeAbstractTheme* absThemeCopy, TeDatabase* outputDatabase);
};


//!  This class implements a factory to create remote theme objects. 
/*!  
	
*/
class TeFileThemeFactory : public TeViewNodeFactory
{
public:
	//! Constructor 
	TeFileThemeFactory() : TeViewNodeFactory((int)TeFILETHEME)
	{}

	//! Created theme objects 
	TeViewNode* build(TeViewNodeParams* params)
	{	
		TeViewNodeParams auxParams = *params;
		return new TeFileTheme(auxParams.name_, auxParams.myParent_, auxParams.viewId_, auxParams.id_);	
	}
	
	TeViewNode* build()
	{
		return new TeFileTheme();
	}
};

namespace 
{
  static TeFileThemeFactory fileThemeFactory;
}; 


#endif	// __TERRALIB_INTERNAL_FILETHEME_H

