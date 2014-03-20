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
/*! \file TeSPRINGDriver.h
    \brief This file contains functions to deal with the SPRING GEO/TAB format
*/
#ifndef __TERRALIB_INTERNAL_SPRINGDriver_H
#define __TERRALIB_INTERNAL_SPRINGDriver_H

#include "../kernel/TeTable.h"
#include "../kernel/TeBox.h"
#include "../kernel/TeDataTypes.h"
#include "../kernel/TeGeometry.h" 

#include "TeFunctionsDefines.h"

#include <string>
using namespace std;

class TeLayer;
class TeSPRFile;
class TeAsciiFile;
class TeProjection;
class TeDatabase;
class TeTheme;
class TeQuerier;

/** @defgroup SPR SPRING GEO/TAB format
   @ingroup Formats
   Functions related to SPRING format
   @{
 */ 
/** Imports a geo data in SPRING geo/tab format to a TerraLib database
	\param geoFileName	GEO/TAB file name
	\param db			pointer to a TerraLib database
	\param layerName	name of the layer that will contain the data (optional)
	\return a pointer to created layer if the data was imported successfully and 0 otherwise
*/
TLFUNCTIONS_DLL TeLayer* TeImportGEO(const string& geoFileName, TeDatabase* db, const string& layerName="");

/** Imports a GEO/TAB data to a layer TerraLib
    \param layer			pointer to a layer
	\param geoFileName		GEO/TAB file name
	\param attrTableName	the name that the attribute table will be saved
	\param chunkSize the number of objects in memory before save them in the layer
*/
TLFUNCTIONS_DLL bool TeImportGEO(TeLayer* layer, const string& geoFileName, string attrTableName="", unsigned int chunkSize=60);

/** Reads the list of attributes  of GEO/TAB file
	\param geoFileName	DBF input file name
	\param attList		to return the DBF list of attributes
	\param labelName	to return the name of the column that is the link with geometries
	\return true if the list of attributes were was successfully read and false otherwise
*/
TLFUNCTIONS_DLL void TeReadGeoAttributeList(const string& geoFileName, TeAttributeList& attList, string& labelName);

/** Exports a layer in a TerraLib database to a file in ASCII SPRING format
    \param layer		pointer to the layer
    \param sprFileName	name of the output SPRING file
    \param tableName	name of the attribute table to be exported
    \param cat			Spring's category
	\param name			name of the column that has the object identifier or class indication
    \return TRUE if the data was successfully exported and FALSE otherwise
    \note Parameter 'name' is the Object name if exporting to categories Cadastral or Network .
   Is the name of the column that has the quote value if Numeric category or the name
   of the column that has the class information if category Thematic.
*/
TLFUNCTIONS_DLL bool TeExportSPR(TeLayer* layer, const string& sprFileName, const string& tableName, 
			 TeSpringModels cat, const string& name);

/** Exports a theme in a TerraLib database to a file in MID/MIF format
	\param theme			pointer to the theme
	\param cat			Spring's category
	\param name			name of the column that has the object identifier or class indication
	\param selOb			selection of the objects that should be exported
	\param baseName		name of the output MID/MIF file
   \return TRUE if the data was successfully exported and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeExportThemeToSPRING(TeTheme* theme, TeSpringModels cat, const string& name, TeSelectedObjects selOb = TeAll, const string& baseName="");


/** Exports a querier to a SCII Spring file
	\param querier		pointer to a valid querier
	\param baseName		name of the output SPRING file
    \param cat			Spring's category
	\param name			name of the column that has the object identifier or class indication
	\param proj			projection of the data
	\param objname		name of object category
	\return TRUE if the data was successfully exported and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeExportQuerierToSPRING(TeQuerier* querier, const std::string& base, TeSpringModels cat, const string& name, TeProjection* proj=0);
/** @} */ // End of subgroup SPRING format

/** \example importGeoTab.cpp
 Shows how to import geographical data in SPRING GEO/TAB format
 */

#endif

