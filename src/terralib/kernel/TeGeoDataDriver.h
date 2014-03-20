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
/* 
	\file TeGeoDataDriver.h
	\brief This file contains contains the abstract definition of a driver to geographical data files with vectorial representation.
*/

#ifndef __TERRALIB_INTERNAL_GEODATADRIVER_H
#define __TERRALIB_INTERNAL_GEODATADRIVER_H

class TeProjection;
class TeSTElementSet;
class TeLayer;
class TeTable;
class TeSTInstance;
class TeGeoDataSet;


#include "TeDefines.h"

#include "TeBox.h"
#include "TeAttribute.h"

#include <string>

//! Abstract definition of a driver to geographical data files with vectorial representation
class TL_DLL TeGeoDataDriver
{

protected:

	bool			isDataInited_; //!< TRUE if the data has already been initialized
	bool			isMetadataInited_; //!< TRUE if the metadata has already been initialized
	unsigned int	numObjects_; //!< The number of objects in this dataSet
	TeProjection*	dataProjection_; //!< The projection associated to the data
	TeBox			box_; //!< The bounding box of the dataSet
	int				rep_; //!< The representation of the dataSet
	TeAttributeList attrList_; //!< The list of attributes of the dataSet

public:

	//! Constructor
	TeGeoDataDriver();

	//! Destructor
	virtual ~TeGeoDataDriver();

	//! Returns the complete access path associated to this driver
	virtual std::string getFileName() = 0;

	//! Returns true or false whether the data file is accessible
	virtual bool isDataAccessible() = 0;

	//! Returns the spatial reference associated to the data as an instance of TeProjection 
	virtual TeProjection* getDataProjection();

	//! Loads the list of descriptive attributes of the objetcts represented in the file
	virtual TeAttributeList& getAttributesList();

	//!< Returns the data representation of the geoDataDriver
	virtual int getRepresentation();

	//!< Returns the bounding box of the geoDataDriver
	virtual TeBox getBox();

	//!< Returns a dataSet containing all the data
	TeGeoDataSet* getData();

	//!< Returns a dataSet containing the filtered data based on the given box restriction
	TeGeoDataSet* filter(const TeBox& box);

	//!< Returns a dataSet containing the filtered data based on the given attribute restriction
	TeGeoDataSet* filter(const std::string& attrRestriction);

	//!< Returns a dataSet containing the filtered data based on the given box and attribute restrictions
	TeGeoDataSet* filter(const TeBox& box, const std::string& attrRestriction);

	//!< Initializes the access to the data
	virtual bool initData() = 0;

	//!< Returns TRUE if the data has already been initialized. FALSE otherwise.
	virtual bool isDataInited() const;

	//!< Initializes the access to the data
	virtual bool initMetadata() = 0;

	//!< Returns TRUE if the data has already been initialized. FALSE otherwise.
	virtual bool isMetadataInited() const;

	//! Creates a new TerraLib layer from a concrete geo data driver.
	/*! 
		\param layer TerraLib Layer
		\param rep TerraLib Representation: Polygon, Line, Point or Text
		\return true if the layer was successfully created and false otherwise
	*/
	virtual bool createLayer(TeLayer* layer, const int& rep) { return false; }

	//! Creates a TerraLib Attribute Table from a concrete geo data driver.
	/*! 
		\param table TerraLib Table
		\return true if the table was successfully created and false otherwise
	*/
	virtual bool createAttributeTable(TeTable& table) { return false; }

	//! Adds a TeSTInstance (TerraLib feature) to set of features handled by a concrete geo data driver.
	/*!
		\param feature A TerraLib feature.
		\return true if the feature was successfully added and false otherwise
	*/
	virtual bool addElement(TeSTInstance* e) { return false; }

	static TeGeoDataDriver*	DefaultObject() { return 0; }

protected:

	//!< Returns a dataSet containing all the data
	virtual TeGeoDataSet* getDataImpl() = 0;

	//!< Returns a dataSet containing the filtered data based on the given attribute restriction. Note that both restrictions could be invalid. In that case, a dataSet containing all the data should be returned.
	virtual TeGeoDataSet* filterImpl(const TeBox& box, const std::string& attrRestriction) = 0;
};

#endif //__TERRALIB_INTERNAL_GEODATADRIVER_H
