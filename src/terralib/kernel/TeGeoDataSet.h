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

#ifndef __TERRALIB_INTERNAL_GEODATASET_H
#define __TERRALIB_INTERNAL_GEODATASET_H

#include "TeDefines.h"
#include <vector>
#include <string>

class TePointSet;
class TeLineSet;
class TePolygonSet;
/* 
	\file TeGeoDataDriver.h
	\brief This file contains contains the abstract definition of a set geographical data with vectorial representation.
*/
class TL_DLL TeGeoDataSet
{
public:

	//!< Construtor
	TeGeoDataSet();

	//!< Destrutor
	virtual ~TeGeoDataSet();

	//!< Returns the number of objects in the geoDataDriver
	virtual unsigned int size() const;

	//!< Returns the 'col'-th attribute value of the 'index'-th element
	virtual bool getAttribute(const int& index, const int& col, std::string& value) = 0;
	
	//!< Returns the attrbute values of the 'index'-th element
	virtual bool getAttributes(const int& index, std::vector<std::string>& vecAtributes) = 0;

	//!< Returns the 'index'-th point
	virtual bool getGeometry(const int& index, TePointSet& ps) = 0;
	
	//!< Returns the 'index'-th line
	virtual bool getGeometry(const int& index, TeLineSet& ls) = 0;

	//!< Returns the 'index'-th polygon
	virtual bool getGeometry(const int& index, TePolygonSet& ps) = 0;

protected:

	unsigned int size_; //!< The number of geographical objects on the dataSet
};

#endif //__TERRALIB_INTERNAL_GEODATASET_H
