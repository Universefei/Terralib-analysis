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
/* 
	\file TeGeoFileInfo.h
	\brief This file contains a simple class to represent the geo data file informations that can be handled by TerraLib.
*/

#ifndef __TERRALIB_INTERNAL_GEOFILEINFO_H
#define __TERRALIB_INTERNAL_GEOFILEINFO_H

// TerraLib
#include "TeDefines.h"

// STL
#include <string>

class TL_DLL TeGeoFileInfo
{
public:

	TeGeoFileInfo();
	~TeGeoFileInfo();

	const std::string& getId() const;
	const std::string& getDescription() const;
	const std::string& getExtension() const;
	bool hasCreateSupport() const;

	void setId(const std::string& id);
	void setDescription(const std::string& d);
	void setExtension(const std::string& e);
	void setCreateSupport(bool b);

	void print() const;

	std::string getFilter() const;

private:

	std::string _id;            //!< The TerraLib internal driver identify. (ex.: SHP, DXF)
	std::string _description;   //!< The file description. (ex.: ESRI Shapefile)
	std::string _extension;     //!< The extension file associated with the driver. (empty allowed)
	bool _hasCreateSupport;     //!< Indicates if the TerarLib internal driver has creation support to this geo file type.
};

#endif //__TERRALIB_INTERNAL_GEOFILEINFO_H
