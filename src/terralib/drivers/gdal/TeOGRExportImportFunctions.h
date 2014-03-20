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

/** \file  TeDriverOGR.h
    \brief This file contains functions to deal with export/import data using the OGR Library support.
   */

#ifndef _TEOGREXPORTIMPORTFUNCTIONS_H
#define _TEOGREXPORTIMPORTFUNCTIONS_H

// TerraLib
#include <TeFunctionsDefines.h>
#include <TeAttribute.h>
#include <TeTable.h>

// STL
#include <string>
#include <vector>

// forward declarations
class TeLayer;

TLFUNCTIONS_DLL bool populateTable(TeTable& tb, std::vector<std::string> props);

TLFUNCTIONS_DLL TeAttribute getAutoLinkName(const TeAttributeList& attList);

TLFUNCTIONS_DLL bool TeImportOGR(TeLayer* lrInput, const std::string& fileName, const std::string& linkName = "", const int& chunkSize = 100);

TLFUNCTIONS_DLL bool TeExportToOGR(TeLayer* lrInput, const std::string& fileName, const std::string& ogrType);

#endif
