/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeCoverageImportUtils.h
    \brief This file contains utility functions for importing coverages.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEIMPORTUTILS_H
#define  __TERRALIB_INTERNAL_COVERAGEIMPORTUTILS_H

#include "TeCoverageParams.h"
#include "TeCoverageUtils.h"

TL_DLL void attributeListToCoverageDimensions(const TeAttributeList& attributes, std::vector<TeCoverageDimension>& dimensions);

TL_DLL void createCoverageTable(TeLayer* layer, std::string& tableName);

TL_DLL void insertToCoverageTable(TeLayer* layer, std::string& tableName, const unsigned int blockId, const TeBox& blockBox, const unsigned int numElements, unsigned char* data, const unsigned long dataSize);

TL_DLL void createCoverageLayerTable(TeLayer* layer, std::string& tableName);

TL_DLL void insertToCoverageLayerTable(TeLayer* layer, std::string& coverageLayerTableName, std::string& coverageTableName, const std::string& coverageId, TeBox& coverageBox);

TL_DLL void createCoverageMetadataTable(TeLayer* layer, std::string& metadataTableName);

TL_DLL void fillCoverageMetadataTable(TeLayer* layer, std::string& metadataTableName, const std::string& coverageId, std::vector<TeCoverageDimension>& dimensions);

TL_DLL TeLayer* createNewCoverageLayer(TeDatabase* database, const std::string& layerName, TeProjection* projection = NULL);

#endif // __TERRALIB_INTERNAL_COVERAGEIMPORTUTILS_H