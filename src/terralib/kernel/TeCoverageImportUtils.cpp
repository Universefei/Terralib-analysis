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

#include "TeCoverageImportUtils.h"

/*
** ----------------------------------------------------------------------------
** Definitions:
*/

/*
** ----------------------------------------------------------------------------
** Methods Implementation:
*/

void attributeListToCoverageDimensions(const TeAttributeList& attributes, std::vector<TeCoverageDimension>& dimensions)
{
    dimensions.clear();

    unsigned int count = 0;
    for (TeAttributeList::const_iterator it = attributes.begin(); it != attributes.end(); it++)
    {
        TeDataType dimensionType = (it->rep_.type_ == TeINT) ? TeINTEGER : TeDOUBLE;
        dimensions.push_back(TeCoverageDimension(count, it->rep_.name_, dimensionType));
        count++;
    }
}

void
createCoverageTable(TeLayer* layer, std::string& tableName)
{
    TeAttributeList attList;
    
    TeAttribute blockIdAtt;

    blockIdAtt.rep_.type_ = TeINT;
    blockIdAtt.rep_.name_ = "block_id";
    blockIdAtt.rep_.isPrimaryKey_ = true;
    attList.push_back(blockIdAtt);

    TeAttribute lowerXAtt;
    lowerXAtt.rep_.type_ = TeREAL;
    lowerXAtt.rep_.name_ = "lower_x";
    attList.push_back(lowerXAtt);

    TeAttribute lowerYAtt;
    lowerYAtt.rep_.type_ = TeREAL;
    lowerYAtt.rep_.name_ = "lower_y";
    attList.push_back(lowerYAtt);

    TeAttribute upperXAtt;
    upperXAtt.rep_.type_ = TeREAL;
    upperXAtt.rep_.name_ = "upper_x";
    attList.push_back(upperXAtt);

    TeAttribute upperYAtt;
    upperYAtt.rep_.type_ = TeREAL;
    upperYAtt.rep_.name_ = "upper_y";
    attList.push_back(upperYAtt);

    TeAttribute numElementsAtt;
    numElementsAtt.rep_.type_ = TeINT;
    numElementsAtt.rep_.name_ = "num_elements";
    attList.push_back(numElementsAtt);

    TeAttribute spatialDataAtt;
    spatialDataAtt.rep_.type_ = TeBLOB;
    spatialDataAtt.rep_.name_ = "spatial_data";
    attList.push_back(spatialDataAtt);

    if (!layer->database()->createTable(tableName, attList))
    {
        std::string errorMsg = "Couldn't create table '" + tableName + "'.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

}

void
insertToCoverageTable(TeLayer* layer, std::string& tableName, const unsigned int blockId, const TeBox& blockBox, const unsigned int numElements, unsigned char* data, const unsigned long dataSize)
{
    // Insert block info into the coverage table
    std::string sqlInsert = "INSERT INTO " + tableName + " (block_id, lower_x, lower_y, upper_x, upper_y, num_elements) VALUES ('" + Te2String(blockId) + "', " + Te2String(blockBox.x1_) + ", " + Te2String(blockBox.y1_) + ", " + Te2String(blockBox.x2_) + ", " + Te2String(blockBox.y2_) + ", " + Te2String(numElements) + ")";
    if (!layer->database()->execute(sqlInsert))
    {
        std::string errorMsg = "Couldn't insert block '" + Te2String(blockId) + "' to coverage table '" + tableName + "'.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

    // Insert block data into the coverage table
    std::string whereClause = "block_id = " + Te2String(blockId);
    if (!layer->database()->insertBlob(tableName, "spatial_data", whereClause , data, dataSize))
    {
        std::string errorMsg = "Couldn't insert block '" + Te2String(blockId) + "' to coverage table '" + tableName + "'.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
}

void
createCoverageLayerTable(TeLayer* layer, std::string& tableName)
{
    TeAttributeList attList;
    
    TeAttribute geomIdAtt;
    geomIdAtt.rep_.type_ = TeINT;
    geomIdAtt.rep_.name_ = "geom_id";
    geomIdAtt.rep_.isPrimaryKey_ = true;
    geomIdAtt.rep_.isAutoNumber_ = true;
    attList.push_back(geomIdAtt);

    TeAttribute coverageIdAtt;
    coverageIdAtt.rep_.type_ = TeSTRING;
    coverageIdAtt.rep_.numChar_ = 25;
    coverageIdAtt.rep_.name_ = "coverage_id";
    coverageIdAtt.rep_.isPrimaryKey_ = false;
	attList.push_back(coverageIdAtt);

    TeAttribute coverageTableAtt;
    coverageTableAtt.rep_.type_ = TeSTRING;
    coverageTableAtt.rep_.numChar_ = 255;
    coverageTableAtt.rep_.name_ = "coverage_table";
    coverageTableAtt.rep_.isPrimaryKey_ = false;
	attList.push_back(coverageTableAtt);

    TeAttribute lowerXAtt;
    lowerXAtt.rep_.type_ = TeREAL;
    lowerXAtt.rep_.name_ = "lower_x";
    attList.push_back(lowerXAtt);

    TeAttribute lowerYAtt;
    lowerYAtt.rep_.type_ = TeREAL;
    lowerYAtt.rep_.name_ = "lower_y";
    attList.push_back(lowerYAtt);

    TeAttribute upperXAtt;
    upperXAtt.rep_.type_ = TeREAL;
    upperXAtt.rep_.name_ = "upper_x";
    attList.push_back(upperXAtt);

    TeAttribute upperYAtt;
    upperYAtt.rep_.type_ = TeREAL;
    upperYAtt.rep_.name_ = "upper_y";
    attList.push_back(upperYAtt);

    if (!layer->database()->createTable(tableName, attList))
    {
        std::string errorMsg = "Couldn't create table '" + tableName + "'.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
}

void
insertToCoverageLayerTable(TeLayer* layer, std::string& coverageLayerTableName, std::string& coverageTableName, const std::string& coverageId, TeBox& coverageBox)
{
    // Insert entry in the coverage layer table
    std::string sqlInsert = "INSERT INTO " + coverageLayerTableName + " (coverage_id, coverage_table, lower_x, lower_y, upper_x, upper_y) VALUES ('" + coverageId + "', '" + coverageTableName + "', " + Te2String(coverageBox.x1_) + ", " + Te2String(coverageBox.y1_) + ", " + Te2String(coverageBox.x2_) + ", " + Te2String(coverageBox.y2_) + ")";
    if (!layer->database()->execute(sqlInsert))
    {
        std::string errorMsg = "Couldn't insert coverage '" + coverageId + "' to layer table '" + coverageLayerTableName + "'.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
}

void
createCoverageMetadataTable(TeLayer* layer, std::string& metadataTableName)
{
    TeAttributeList attList;

    TeAttribute coverageIdAtt;
    coverageIdAtt.rep_.type_ = TeSTRING;
    coverageIdAtt.rep_.numChar_ = 25;
    coverageIdAtt.rep_.name_ = "coverage_id";
    coverageIdAtt.rep_.isPrimaryKey_ = true;
	attList.push_back(coverageIdAtt);

    TeAttribute dimensionIdAtt;
    dimensionIdAtt.rep_.type_ = TeINT;
    dimensionIdAtt.rep_.name_ = "dimension_id";
    dimensionIdAtt.rep_.isPrimaryKey_ = true;
    attList.push_back(dimensionIdAtt);

    TeAttribute dataTypeAtt;
    dataTypeAtt.rep_.type_ = TeINT;
    dataTypeAtt.rep_.name_ = "data_type";
    dataTypeAtt.rep_.isPrimaryKey_ = false;
    attList.push_back(dataTypeAtt);

    TeAttribute nameAtt;
    nameAtt.rep_.type_ = TeSTRING;
    nameAtt.rep_.numChar_ = 25;
    nameAtt.rep_.name_ = "name";
    nameAtt.rep_.isPrimaryKey_ = false;
	attList.push_back(nameAtt);

    if (!layer->database()->createTable(metadataTableName, attList))
    {
        std::string errorMsg = "Couldn't create table '" + metadataTableName + "'.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
}

void
fillCoverageMetadataTable(TeLayer* layer, std::string& metadataTableName, const std::string& coverageId, std::vector<TeCoverageDimension>& dimensions)
{
    TeDatabase* db = layer->database();
    if (!db->tableExist(metadataTableName))
    {   
        createCoverageMetadataTable(layer, metadataTableName);
    }

    for (std::vector<TeCoverageDimension>::iterator it = dimensions.begin(); it != dimensions.end(); it++)
    {
        const std::string& coverageIdStr = coverageId;
        const std::string dimensionIdStr = Te2String(it->dimension_id);
        const std::string dataTypeStr = Te2String(static_cast<TeDataType>(it->type));
        const std::string& nameStr = it->name;

        std::string sqlInsert = "INSERT INTO " + metadataTableName + " (coverage_id, dimension_id, data_type, name) VALUES ('" + coverageIdStr + "', '" + dimensionIdStr + "', '" + dataTypeStr + "', '" + nameStr + "')";

        if (!db->execute(sqlInsert))
        {
            std::string errorMsg = "Couldn't insert values to metadata table '" + metadataTableName + "'.";
            throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
        }
    }
}

TeLayer* createNewCoverageLayer(TeDatabase* database, const std::string& layerName, TeProjection* projection)
{
    // Find a valid layer name
    std::string newLayerName = layerName;
    TeLayerMap& layerMap = database->layerMap();
    TeLayerMap::iterator it = layerMap.begin();
    int n = 0;
    while (it != layerMap.end())
    {
        if (TeStringCompare(it->second->name(),newLayerName))
        {
            // Try another name and restart search
            newLayerName = layerName + "_" + Te2String(n++);
            it = layerMap.begin();
            continue;
        }
        it++;
    }

    // Create new layer
	TeLayer* newLayer = new TeLayer(newLayerName, database, projection);
	if (!newLayer || newLayer->id() <= 0)
    {
        std::string errorMsg = "Layer creation failed.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

    return newLayer;
}

/*
** ----------------------------------------------------------------------------
** End:
*/