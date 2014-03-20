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
/*! \file TeMIDMIFDriver.h
    \brief This file contains functions to deal with the MID MIF data format
*/
#ifndef __TERRALIB_INTERNAL_TeMIDMIFDriver_H
#define __TERRALIB_INTERNAL_TeMIDMIFDriver_H

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
class TePolygonSet;
class TeQuerier;
class TeTheme;

/** @defgroup MIDMIF MID/MIF format
 *  @ingroup Formats
 *  Functions related to MID MIF format
 *  @{
 */
 /** Imports a geo data in MID/MIF format to a TerraLib database
	\param mifFileName	MIF file name
	\param db			pointer to a TerraLib database
	\param layerName	name of the layer that will contain the data (optional)
	\return a pointer to created layer if the data was imported successfully and 0 otherwise
*/
TLFUNCTIONS_DLL TeLayer* TeImportMIF(const string& mifFileName, TeDatabase* db, const string& layerName="");

/** Imports a MID/MIF file to a specific layer TerraLib
	\param layer			pointer to a layer
	\param mifFileName		MIF file name
	\param attrTableName	the name that the attribute table will be saved
	\param objectIdAttr		name of the attribute that has the identification of objects
	\param chunkSize		the number of objects in memory before save them in the layer
	\return TRUE if the data was imported successfully and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeImportMIF(TeLayer* layer, const string& mifFileName, string attrTableName="",  
				 string objectIdAttr="", int unsigned chunkSize = 60);

/** Reads the attributes definition of a data in MID/MIF format
	\param mifFileName		MIF file name
	\param attList			to return the list of attributes defined in the file
*/
TLFUNCTIONS_DLL void TeReadMIFAttributeList(const string& mifFileName, TeAttributeList& attList);

/** Reads the geographical projection of a data in MID/MIF format
	\param mifFileName		MIF file name
	\return a pointer to a TeProjection
*/
TLFUNCTIONS_DLL TeProjection* TeReadMIFProjection(const string& mifFileName);

/** Exports a querier to a MID/MIF file
	\param	querier		pointer to a valid querier
	\param baseName		name of the output MID/MIF file
	\param proj			pointer to the projection of the geometries retrieved by the querier
	\return TRUE if the data was successfully exported and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeExportQuerierToMIF(TeQuerier* querier, const std::string& base, TeProjection* proj);

/** Exports a layer in a TerraLib database to a file in MID/MIF format
   \param layer			pointer to the layer
   \param baseName		name of the output MID/MIF file
   \return TRUE if the data was successfully exported and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeExportLayerToMIF(TeLayer* layer, const string& baseName="");

/** Exports a theme in a TerraLib database to a file in MID/MIF format
   \param theme			pointer to the theme
   \param baseName		name of the output MID/MIF file
   \param selOb			selection of the objects that should be exported
   \return TRUE if the data was successfully exported and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeExportThemeToMIF(TeTheme* theme, TeSelectedObjects selOb = TeAll, const string& baseName="");

/** Exports a layer in a TerraLib database to a file in MID/MIF format
   \param layer			pointer to the layer
   \param mifFileName	name of the output MID/MIF file
   \param tableName		name of the attribute table to be exported
   \return true if the data was successfully exported and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeExportMIF(TeLayer* layer, const string& mifFileName, const string& tableName, const string& restriction = "");

/** @} */ // End of subgroup MID/MIF format

/** \example importMIDMIF.cpp
 Shows how to import geographical data in MID/MIF format
 */

#endif

