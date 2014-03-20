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
	\file TeSTGeoDataDriver.h
	\brief This file contains contains the implementation of a driver to geographical data with vectorial representation based of the TeSTElementSet class.
*/

#ifndef __TERRALIB_INTERNAL_STGEODATASET_H
#define __TERRALIB_INTERNAL_STGEODATASET_H

#include "TeGeoDataSet.h"

class TeSTElementSet;

//! Abstract definition of a driver to geographical data files with vectorial representation
class TL_DLL TeSTGeoDataSet : public TeGeoDataSet
{
public:

	//!< Construtor
	TeSTGeoDataSet(TeSTElementSet* dataSet);

	//!< Construtor
	TeSTGeoDataSet(TeSTElementSet* dataSet, std::vector<int>& vecFilter);

	//!< Destrutor
	virtual ~TeSTGeoDataSet();

	//!< Returns the 'col'-th attribute value of the 'index'-th element
	virtual bool getAttribute(const int& index, const int& col, std::string& value);
	
	//!< Returns the attrbute values of the 'index'-th element
	virtual bool getAttributes(const int& index, std::vector<std::string>& vecAtributes);

	//!< Returns the 'index'-th point
	virtual bool getGeometry(const int& index, TePointSet& ps);
	
	//!< Returns the 'index'-th line
	virtual bool getGeometry(const int& index, TeLineSet& ls);

	//!< Returns the 'index'-th polygon
	virtual bool getGeometry(const int& index, TePolygonSet& ps);

protected:

	virtual int getRealIndex(const int& index);

protected: 
	
	TeSTElementSet*		dataSet_;
	std::vector<int>	vecFilter_;
};

#endif //__TERRALIB_INTERNAL_STGEODATASET_H
