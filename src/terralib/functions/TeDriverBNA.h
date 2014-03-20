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
/*! \file TeBNADriver.h
    \brief This file contains functions to deal with the BNA format
*/
#ifndef __TERRALIB_INTERNAL_BNADRIVER_H
#define __TERRALIB_INTERNAL_BNADRIVER_H

#include "../kernel/TeTable.h"
#include "../kernel/TeBox.h"
#include "../kernel/TeDataTypes.h"
#include "../kernel/TeGeometry.h" 

#include "TeFunctionsDefines.h"

#include <string>
using namespace std;

class TeLayer;
class TeAsciiFile;
class TeProjection;
class TeDatabase;


/** @defgroup BNA BNA format
 *  @ingroup Formats
 *  Functions related to BNA format (Atlas GIS BNA)
 *  @{
 */
 
/** Imports a geo data in BNA format to a TerraLib database
	\param bnaFileName	BNA file name
	\param db			pointer to a TerraLib database
	\param layerName	name of the layer that will contain the data (optional)
	\return a pointer to created layer if the data was imported successfully and 0 otherwise
*/
TLFUNCTIONS_DLL TeLayer* TeImportBNA(const string& bnaFileName, TeDatabase* db, const std::string& layerName="", const std::string& linkName = "");

/** Imports a BNA file to a specific layer TerraLib
	\param layer			pointer to a layer
	\param bnaFileName		BNA file name
	\param attrTableName	the name that the attribute table will be saved
	\param attList			to return the list of attributes defined in the bna file, used only to name the fields
	\param chunkSize		the number of objects in memory before save them to the layer
	\param linkCol			name of the column that will contain the link to geometries
	\return TRUE if the data was imported successfully and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeImportBNA(TeLayer* layer, const string& bnaFileName, string attrTableName,  
								 TeAttributeList& attList, int unsigned chunkSize, std::string& linkName);

/** Reads the attributes definition of a data in BNA format 
	\param bnaFileName	BNA file name
	\param attList		to return the attribute list
	\return TRUE if the attribute list is successfully read and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeReadBNAAttributeList(const string& bnaFileName, TeAttributeList& attList);

/** @} */ // End of subgroup BNA format
#endif

