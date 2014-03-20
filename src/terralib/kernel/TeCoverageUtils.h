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
/*! \file TeCoverageUtils.h
    \brief This file contains utility functions for working with coverages.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEUTILS_H
#define  __TERRALIB_INTERNAL_COVERAGEUTILS_H

#include "TeCoverageParams.h"
#include "TeException.h"

TL_DLL std::string TeDefaultCoverageLayerTable(TeLayer* layer);

TL_DLL std::string TeDefaultCoverageTable(TeLayer* layer, const std::string& coverageId);

TL_DLL std::string TeDefaultCoverageMetadataTable(TeLayer* layer);

TL_DLL void TeRetrieveCoverageLayerTables(TeLayer* layer, std::vector<std::string>& coverageLayerTables);

TL_DLL void TeRetrieveCoverageIds(TeLayer* layer, std::vector<std::string>& coverageIds);

TL_DLL std::string TeRetrieveCoverageTable(TeLayer* layer, const std::string& coverageId);

TL_DLL std::string TeRetrieveCoverageLayerTable(TeLayer* layer, const std::string& coverageId);

TL_DLL TeLayer* TeRetrieveLayer(TeDatabase* database, const int layerId);

TL_DLL void TeRetrieveDimensions(TeLayer* layer, const std::string& coverageId, std::vector<TeCoverageDimension>& dimensions);

TL_DLL int TeCoverageDimensionsSize(std::vector<TeCoverageDimension>& dimensions);

TL_DLL TeBox TeRetrieveCoverageBox(TeLayer* layer, const std::string& coverageId);

TL_DLL int TeRetrieveCoverageNumElements(TeLayer* layer, const std::string& coverageId);

#endif // __TERRALIB_INTERNAL_COVERAGEUTILS_H