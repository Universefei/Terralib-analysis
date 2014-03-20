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

/*
** ----------------------------------------------------------------------------
** Includes:
*/

#include "TeCoverageUtils.h"

/*
** ----------------------------------------------------------------------------
** Definitions:
*/

/*
** ----------------------------------------------------------------------------
** Methods Implementation:
*/

std::string
TeDefaultCoverageLayerTable(TeLayer* layer)
{
    return "Coverage_" + Te2String(layer->id());
}

std::string
TeDefaultCoverageTable(TeLayer* layer, const std::string& coverageId)
{
    return TeDefaultCoverageLayerTable(layer) + "_" + coverageId;
}

std::string
TeDefaultCoverageMetadataTable(TeLayer* layer)
{
    return TeDefaultCoverageLayerTable(layer) + "_metadata";
}

void
TeRetrieveCoverageLayerTables(TeLayer* layer, std::vector<std::string>& coverageLayerTables)
{
    coverageLayerTables.clear();

    // Get coverage layer tables
    TeRepresPointerVector representations;
    layer->getRepresentation(TeCOVERAGE, representations);
    for (TeRepresPointerVector::iterator it = representations.begin(); it != representations.end(); it++)
    {
        coverageLayerTables.push_back((*it)->tableName_);
    }
}

void
TeRetrieveCoverageIds(TeLayer* layer, std::vector<std::string>& coverageIds)
{
    coverageIds.clear();

    // Get coverage layer table
    std::vector<std::string> coverageLayerTables;
    TeRetrieveCoverageLayerTables(layer, coverageLayerTables);

    for (std::vector<std::string>::iterator it = coverageLayerTables.begin(); it != coverageLayerTables.end(); it++)
    {
        // Find coverage IDs
        TeDatabasePortal* portal = layer->database()->getPortal();
        portal->query("SELECT coverage_id FROM " + (*it));
        while (portal->fetchRow())
        {
            coverageIds.push_back(portal->getData("coverage_id"));
        }
        portal->freeResult();
    }
}

std::string
TeRetrieveCoverageTable(TeLayer* layer, const std::string& coverageId)
{
    std::string coverageTable = "";
    
    // Get coverage layer table
    std::vector<std::string> coverageLayerTables;
    TeRetrieveCoverageLayerTables(layer, coverageLayerTables);

    for (std::vector<std::string>::iterator it = coverageLayerTables.begin(); it != coverageLayerTables.end(); it++)
    {
        // Find coverage IDs
        TeDatabasePortal* portal = layer->database()->getPortal();
        portal->query("SELECT coverage_table FROM " + (*it) + " WHERE coverage_id = '" + coverageId + "'");
        if (portal->fetchRow())
        {
            coverageTable = portal->getData("coverage_table");
            portal->freeResult();
            break;
        }
        portal->freeResult();
    }
    if (coverageTable.empty())
    {
        std::string errorMsg = "Couldn't find coverage " + coverageId + " on layer " + Te2String(layer->id()) + ".";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
    return coverageTable;
}

std::string
TeRetrieveCoverageLayerTable(TeLayer* layer, const std::string& coverageId)
{
    std::string coverageLayerTable = "";
    
    // Get coverage layer tables associated to this layer
    std::vector<std::string> coverageLayerTables;
    TeRetrieveCoverageLayerTables(layer, coverageLayerTables);

    // Find the coverage layer table to which this coverage ID belongs
    for (std::vector<std::string>::iterator it = coverageLayerTables.begin(); it != coverageLayerTables.end(); it++)
    {
        // Look for coverage ID
        TeDatabasePortal* portal = layer->database()->getPortal();
        portal->query("SELECT coverage_id FROM " + (*it) + " WHERE coverage_id = '" + coverageId + "'");
        if (portal->fetchRow())
        {
            coverageLayerTable = (*it);
            portal->freeResult();
            break;
        }
        portal->freeResult();
    }
    if (coverageLayerTable.empty())
    {
        std::string errorMsg = "Couldn't find coverage " + coverageId + " on layer " + Te2String(layer->id()) + ".";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
    return coverageLayerTable;
}

TeLayer*
TeRetrieveLayer(TeDatabase* database, const int layerId)
{
	TeLayerMap& layerMap = database->layerMap();
    TeLayerMap::iterator match = layerMap.find(layerId);
    if (match == layerMap.end())
    {
        std::string errorMsg = "Layer " + Te2String(layerId) + " not found in database.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
    return match->second;
}

void
TeRetrieveDimensions(TeLayer* layer, const std::string& coverageId, std::vector<TeCoverageDimension>& dimensions)
{
    dimensions.clear();

    TeRepresentation* rep = layer->getRepresentation(TeCOVERAGE);
    std::string metadataTable = rep->tableName_ + "_metadata";

    TeDatabasePortal* portal = layer->database()->getPortal();

    std::string sql = "SELECT * FROM " + metadataTable + " WHERE coverage_id = '" + coverageId + "'";
    if (portal->query(sql))
    {
        while (portal->fetchRow())
        {
            int dimensionId = portal->getInt("dimension_id");
            int dataType = portal->getInt("data_type");
            std::string name = portal->getData("name");

            TeCoverageDimension dimension = TeCoverageDimension(dimensionId, name, static_cast<TeDataType>(dataType));
            dimensions.push_back(dimension);
        }
    }
    else
    {
        std::string errorMsg = "Couldn't retrieve dimensions for coverage " + coverageId + " on layer " + Te2String(layer->id()) + ".";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
}

int
TeCoverageDimensionsSize(std::vector<TeCoverageDimension>& dimensions)
{
    int dimensionsSize = 0;

    std::vector<TeCoverageDimension>::iterator it = dimensions.begin();
    std::vector<TeCoverageDimension>::iterator end = dimensions.end();

    while (it != end)
    {
        switch (it->type)
        {
        case (TeUNSIGNEDSHORT):
            dimensionsSize += sizeof(unsigned short);
            break;
        case (TeSHORT):
            dimensionsSize += sizeof(short);
            break;
        case (TeINTEGER):
            dimensionsSize += sizeof(int);
            break;                
        case (TeUNSIGNEDLONG):
            dimensionsSize += sizeof(unsigned long);
            break;
        case (TeLONG):
            dimensionsSize += sizeof(long);
            break;
        case (TeFLOAT):
            dimensionsSize += sizeof(float);
            break;
        case (TeDOUBLE):
            dimensionsSize += sizeof(double);
            break;
        default:
            std::string errorMsg = "Illegal type found on dimensions vector.";
            throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
            break;
        }
        it++;
    }
    return dimensionsSize;
}

TeBox
TeRetrieveCoverageBox(TeLayer* layer, const std::string& coverageId)
{
    TeBox box;

    std::string coverageTable = TeRetrieveCoverageLayerTable(layer, coverageId);

    std::string sql = "SELECT * FROM " + coverageTable + " WHERE coverage_id = '" + coverageId + "'";

    TeDatabasePortal* portal = layer->database()->getPortal();
    if (portal->query(sql) && portal->fetchRow())
    {
        box = TeBox(portal->getDouble("lower_x"),
                    portal->getDouble("lower_y"),
                    portal->getDouble("upper_x"),
                    portal->getDouble("upper_y"));
    }
    else
    {
        std::string errorMsg = "Couldn't retrieve box for coverage " + coverageId + " on layer " + Te2String(layer->id()) + ".";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
    return box;
}

int
TeRetrieveCoverageNumElements(TeLayer* layer, const std::string& coverageId)
{
    int numElements = -1;
    std::string coverageTable = TeRetrieveCoverageTable(layer, coverageId);

    // Sum number of elements of all blocks
    std::string sql = "SELECT SUM(num_elements) as sum_num_elements FROM " + coverageTable;

    TeDatabasePortal* portal = layer->database()->getPortal();
    if (portal->query(sql) && portal->fetchRow())
    {
        numElements = portal->getInt("sum_num_elements");
    }
    else
    {
        std::string errorMsg = "Couldn't retrieve number of elements for coverage " + coverageId + " on layer " + Te2String(layer->id()) + ".";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
    portal->freeResult();
    return numElements;
}

/*
** ----------------------------------------------------------------------------
** End:
*/