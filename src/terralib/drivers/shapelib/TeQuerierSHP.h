/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2004 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeQuerierSHP.h
	\brief This file contains a mechanism named "TeQuerierSHP" that is 
	responsible for loading geometries and its attributes from a shapefile. 
*/

#ifndef  __TERRALIB_INTERNAL_QUERIER_SHP_H
#define  __TERRALIB_INTERNAL_QUERIER_SHP_H

#include "TeQuerierImpl.h"
#include <shapefil.h>

/*! \class TeQuerierSHP
	\brief A class responsible for loading geometries and its attributes from a shape file.

	This class implements a mechanism that is responsible for loading
	geometries and its attributes from a shape file (.shp file). 
	Each entity (geometries and attributes) of the shape file is 
	represented through the TeSTInstance class.  

	\sa
	TeQuerierImpl TeQuerierParams TeSTInstance 
*/
class TeQuerierSHP : public TeQuerierImpl
{
	
protected:

	SHPHandle	hSHP;			//!< handle to the file that stores geometries (.shp) of the shapefile format 
	DBFHandle	hDBF;			//!< handle to the file that stores attributes (.dbf) of the shapefile format 
	int			nEntities;		//!< number of entities in the shape file
	int			curEntity;		//!< current entity index used to traverse all entities of the shape file
	int			nShapeType;		//!< geometry type of the shape file  
	
public:

		//! Constructor from a set of parameters
		TeQuerierSHP(TeQuerierParams* params): 
			TeQuerierImpl(params),
			nEntities(0),
			curEntity(0),
			nShapeType()
		{ }

		
		//! Returns the file name 
		string fileName () { return params_->fileName(); }

		//! Load information about the geometries and attributes of the shape file 
		bool loadInstances(TeTSEntry* ent = 0); 
		
		//! Gets the current entity and moves to the next one. Returns if there is a next entity. 
		bool fetchInstance(TeSTInstance& stoi); 

		//! Loads all geometries of the index-th geometry representation 
		bool loadGeometries(TeMultiGeometry& geometries, unsigned int& index); 
		
		//! Loads all geometries 
		bool loadGeometries(TeMultiGeometry& geometries); 
};


/*! \class TeQuerierSHPFactory
	\brief A class that define a factory to build a querier strategy from shape file.

	\sa
	TeQuerierImplFactory TeQuerierSHP
*/
class TeQuerierSHPFactory : public TeQuerierImplFactory
{
public:

	//! Constructor
	TeQuerierSHPFactory(const string& name) : TeQuerierImplFactory(name) {}

	//! Builds a shapefile querier
	virtual TeQuerierImpl* build (const TeQuerierParams& arg)
	{  
		TeQuerierParams* tempArg = new TeQuerierParams();
		*tempArg = arg;
		return new TeQuerierSHP(tempArg); 
	}
};
 
/** \example createSTElementSetFromShapeFile.cpp
	Shows how to create a new Spatial Temporal Element Set (STElementSet) from a shapefile
*/

namespace 
{
  static TeQuerierSHPFactory querierSHP("querierSHP");
};


#endif


