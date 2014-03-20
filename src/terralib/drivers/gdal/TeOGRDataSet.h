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

/** \file  TeOGRDataSet.h
    \brief This file contains a concrete geo data set based on OGR Library.
    \author Douglas Uba <douglas@dpi.inpe.br>
   */

#ifndef _TEOGRDATASET_H
#define _TEOGRDATASET_H

// Defines
#include "TeGDALDefines.h"
#include "ogr_core.h"

// TerraLib include files
#include "../../kernel/TeGeoDataSet.h"
#include "../../kernel/TeBox.h"
#include "../../kernel/TeDataTypes.h"
#include "../../kernel/TeAttribute.h"
#include "../../kernel/TeTable.h"
#include "../../kernel/TeAttribute.h"
#include "TeSTInstance.h"

// STL include files
#include <vector>
#include <string>

// forward declarations;
class TeProjection;
class TeSTElementSet;
class OGRDataSource;
class OGRLayer;
class TeLayer;
class OGRFeature;

class TEGDAL_DLL TeOGRDataSet : public TeGeoDataSet 
{
public:
  
	TeOGRDataSet(OGRLayer* ogrLayer);

    virtual ~TeOGRDataSet();

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

	bool moveTo(const int& index);

protected:

	TeSTInstance	currentInstance_; //!, The current read ogr feature
	OGRLayer*		ogrLayer_;	//!< A pointer to first OGR Layer on DataSource. // TODO: revision is necessary.
	int				currentIndex_; //!< The current index of the ogrLayer

private:
  
    TeOGRDataSet(const TeOGRDataSet& source);

    TeOGRDataSet & operator=(const TeOGRDataSet & source);
};

#endif // _TEOGRDATASET_H
