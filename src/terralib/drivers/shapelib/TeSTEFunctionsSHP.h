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
/*! \file TeSTEFunctionsSHP.h
    This file is a wrapper aroun shapelib to provide funcions to decodify shapefiles
*/
#ifndef  __TERRALIB_INTERNAL_TeSTEFUNCTIONSSHP_H
#define  __TERRALIB_INTERNAL_TeSTEFUNCTIONSSHP_H

#include <string>
#include "../../kernel/TeAttribute.h"
#include "../../../shapelib/shapefil.h"

class TeSTElementSet; 
class TeLineSet; 
class TePolygonSet; 
class TePointSet; 
class TeMultiGeometry; 

using namespace std;

//! Builds the spatial object set from database according to the restrictions previously defined
/*! 
	\param stoset the STOSet that will be filled 
	\param fileName shape file name
*/
bool TeSTOSetBuildSHP(TeSTElementSet& stoset, const string& fileName);


//! Decodify a ShapeObject into a TerraLib point set
/*! 
	\param psShape pointer to a shapelib shape object 
	\param ps TerraLib point set to receive the geometries
	\param objectId TerraLib identifier to this object
*/
void TeSHPPointDecode	(SHPObject* psShape, TePointSet& ps, string& objectId);

//! Decodify a ShapeObject into a TerraLib line set
/*! 
	\param psShape pointer to a shapelib shape object 
	\param ps TerraLib line set to receive the geometries
	\param objectId TerraLib identifier to this object
*/
void TeSHPPolyLineDecode(SHPObject* psShape, TeLineSet& ls, string& objectId);

//! Decodify a ShapeObject into a TerraLib polygon set
/*! 
	\param psShape pointer to a shapelib shape object 
	\param ps TerraLib polygon set to receive the geometries
	\param objectId TerraLib identifier to this object
*/
void TeSHPPolygonDecode	(SHPObject* psShape, TePolygonSet& pols, string& objectId);

//! Decodify a ShapeObject into a TerraLib multigeometry set
/*! 
	\param psShape pointer to a shapelib shape object 
	\param geomestries TerraLib multi geometry set to receive the geometries
	\param objectId TerraLib identifier to this object
*/
bool TeDecodeShape(SHPObject* psShape, TeMultiGeometry& geomestries, string& objectId);

//! Decodify the attribute list definition from a dbf file into a TerraLib attribute list
/*! 
	\param dbfFilename name of the DBF file 
	\param attList to return the list of attributes
*/
bool TeReadDBFAttributeList(const string& dbfFileName, TeAttributeList& attList);
#endif

