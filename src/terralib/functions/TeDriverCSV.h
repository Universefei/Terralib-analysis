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
/*! \file TeDriverCSV.h
    \brief This file contains functions to deal with a file in CSV format
*/
#ifndef __TERRALIB_INTERNAL_DRIVERCSV_H
#define __TERRALIB_INTERNAL_DRIVERCSV_H

#include "../kernel/TeTable.h"
#include "../kernel/TeBox.h"
#include "../kernel/TeDataTypes.h"

#include "TeFunctionsDefines.h"

#include <string>
using namespace std;

class TeLayer;
class TeAsciiFile;
class TeProjection;
class TeDatabase;
class TeQuerier;
class TeTheme;

/** @defgroup CSV format
 *  @ingroup Formats
 *  Functions related to CSV format
 *  @{
 */
/** Creates an attribute table in a TerraLib database and imports a CSV file to this table 
	\param csvFileName			CSV file name
	\param attrTable			attribute table specifications
	\param db					pointer to a TerraLib database
	\param layer				pointer to a layer to which the table will associated
	\param separator			character used to separate the data in the CSV file 
	\param firstLineIsHeader	indicates if the first line is a header
	\param chunkSize			number of objects in memory before save them in the database
	\param createAutoNum		indicates if the table first attribute is an autonumber which must be generated  
	\return TRUE if the data was imported successfully and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeImportCSVFile(const string& csvFileName, TeTable& attrTable, TeDatabase* db, TeLayer* layer=0, 
					 const char& separator=',', const bool& firstLineIsHeader=false,   
					 const unsigned int& chunkSize=100, const bool& createAutoNum = false);

/** Imports a CSV file to a layer in a TerraLib database, its geometries (points) and attributes
	\param csvFileName			CSV file name pointer to a layer
	\param attributeX			attribute index which provides the coordinate X
	\param attributeY			attribute index which provides the coordinate Y
	\param attrTable			attribute table specifications
	\param db					pointer to a TerraLib database
	\param layer				pointer to an empty layer which will be filled from CSV file
	\param separator			character used to separate the data in the CSV file 
	\param firstLineIsHeader	indicates if the first line is a header
	\param chunkSize			number of objects in memory before save them in the database
	\param createAutoNum		indicates if the table first attribute is an autonumber which must be generated  
	\return TRUE if the data was imported successfully and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeImportCSVFile(const string& csvFileName, const int& attributeX, const int& attributeY, 
					 TeTable& attrTable, TeDatabase* db, TeLayer* layer, 
					 const char& separator=',', const bool& firstLineIsHeader=false,   
					 const unsigned int& chunkSize=100, const bool& createAutoNum = false);

/** Exports a querier to a Comma-Separated Values data file
	\param querier		pointer to a valid querier
	\param filename		name of the output CSV file
	\param sep			separator to be used
	\return TRUE if the data was successfully exported and FALSE otherwise
	This function can be used only when the querier access point representation data. First line contains the name
	of the attributes.
*/
TLFUNCTIONS_DLL bool TeExportQuerierToCSV(TeQuerier* querier, const std::string& filename, const char& sep=';');

/** Exports a layer in a TerraLib database to a file in ASCII CSV format
	This function can be used only when the layers contains POINT representation. First line contains the name
	of the attributes.
   \param layer			pointer to the layer
   \param filename		name of the output MID/MIF file
   \param sep			separator to be used
   \return TRUE if the data was successfully exported and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeExportLayerToCSV(TeLayer* layer, const string& filename="", const char& sep=';');

/** Exports a theme in a TerraLib database to a file in ASCII CSV format
 	This function can be used only when the theme acontains POINT representation. First line contains the name
	of the attributes.
   \param theme			pointer to the theme
   \param filename		name of the output MID/MIF file
   \param selOb			selection of the objects that should be exported
   \param sep			separator to be used
   \return TRUE if the data was successfully exported and FALSE otherwise
*/
TLFUNCTIONS_DLL bool TeExportThemeToCSV(TeTheme* theme, TeSelectedObjects selOb, const std::string& filename="", const char& sep=';');

/** @} */ // End of subgroup CSV format

#endif

