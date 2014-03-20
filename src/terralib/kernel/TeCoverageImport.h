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
/*!
  \file TeCoverageImport.h

  \par This file contains algorithms for importing Coverage data
       to a TerraLib database.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEIMPORT_H
#define  __TERRALIB_INTERNAL_COVERAGEIMPORT_H

#include <sstream>

#include "TeCoverageParams.h"
#include "TeCoverageUtils.h"
#include "TeCoverageImportUtils.h"
#include <TeQuerier.h>
#include <TeQuerierParams.h>
//#include "TeDriverSHPDBF.h"

/*!
  \def WRITE
  \brief Write a value of some type in a block of raw data.
*/
#define WRITE(data, t, v) *((t*)(data)) = v;  data += sizeof(t)


/*!
  \struct SerializedGeomValuePair
  \brief A geometry and raw data that is the serialization of a geometry-value pair.
*/
template <class T>
struct SerializedPair {
    T geom;
    unsigned char* data;
    unsigned long size;
};

template <class T>
struct CmpSerializedX {
    bool operator() (SerializedPair<T>* s1, SerializedPair<T>* s2)
    {
        return s1->geom.box().center().x() < s2->geom.box().center().x();
    }
};

template <class T>
struct CmpSerializedY {
    bool operator() (SerializedPair<T>* s1, SerializedPair<T>* s2)
    {
        return s1->geom.box().center().y() < s2->geom.box().center().y();
    }
};

template <class T>
void
splitSerialized(std::vector<SerializedPair<T>* >& src,
                typename std::vector<SerializedPair<T>* >::iterator& begin,
                typename std::vector<SerializedPair<T>* >::iterator& end,
                std::vector<std::vector<SerializedPair<T> > >& clusters)
{
    const int CLUSTER_CAPACITY = 300;
    if ((end - begin) <= CLUSTER_CAPACITY)
    {
        std::vector<SerializedPair<T> >& cluster = std::vector<SerializedPair<T> >();
        for (std::vector<SerializedPair<T>* >::iterator it = begin; it != end; it++)
        {
            SerializedPair<T>* pair = *it;
            cluster.push_back(*pair);
        }
        clusters.push_back(cluster);
        return;
    }

    TeBox box = TeBox();
    for (std::vector<SerializedPair<T>* >::iterator it = begin; it != end; it++)
    {
        SerializedPair<T>* pair = (*it);
        updateBox(box, pair->geom.box());
    }

    // split geom set horizontaly if width is larger than height
    if ((box.x2_ - box.x1_) > (box.y2_ - box.y1_))
    {
        sort(begin, end, CmpSerializedX<T>());
    }
    // split verticaly otherwise
    else
    {
        sort(begin, end, CmpSerializedX<T>());
    }

    std::vector<SerializedPair<T> *>::iterator mid = begin + (((end - begin) + 1) / 2);
    splitSerialized(src, mid, end, clusters);
    splitSerialized(src, begin, mid, clusters);
}

template <class T>
void
clusterSerialized(std::vector<SerializedPair<T> >& srcGroup, std::vector<std::vector<SerializedPair<T> > >& clusters)
{
    std::vector<SerializedPair<T>* >& pairs = std::vector<SerializedPair<T>* >();

    for (std::vector<SerializedPair<T> >::iterator it = srcGroup.begin(); it != srcGroup.end(); it++)
    {
        SerializedPair<T>& pair = *it;
        pairs.push_back(&pair);
    }
    splitSerialized(pairs, pairs.begin(), pairs.end(), clusters);
}

template <class T>
void
populateDatabase(TeLayer* layer, const std::string& coverageId, std::vector<std::vector<SerializedPair<T> > >& groups)
{
    TeDatabase* db = layer->database();

    std::string& tableName = TeDefaultCoverageTable(layer, coverageId);
    if (!db->tableExist(tableName))
    {
        createCoverageTable(layer, tableName);
    }

    // Bounding box of the coverage, to be calculated
    TeBox coverageBox = TeBox();

    // Iterate over the  collection of grouped points
    std::vector<std::vector<SerializedPair<T> > >::iterator itGroups = groups.begin();
    std::vector<std::vector<SerializedPair<T> > >::iterator endGroups = groups.end();
    int count = 0;
    while (itGroups != endGroups)
    {
        std::vector<SerializedPair<T> >& group = *(itGroups++);
        std::vector<SerializedPair<T> >::iterator itPoints = group.begin();
        std::vector<SerializedPair<T> >::iterator endPoints = group.end();

        // Calculate block size, according to sizes of serialized points
        unsigned long blockSize = 0;
        while (itPoints != endPoints)
        {
            blockSize += itPoints->size;
            itPoints++;
        }

        // Allocate enough memory for the block data
        unsigned char* blockData = NULL;
        blockData = (unsigned char*)malloc(blockSize);

        unsigned char* dest = blockData;

        // Create bounding box of the group
        TeBox blockBox = TeBox();

        // Reset iterator and iterate over the serialized points in the group
        itPoints = group.begin();
        while (itPoints != endPoints)
        {
            // Join chunks to the block data
            unsigned char* src = itPoints->data;
            for (unsigned int i = 0; i < itPoints->size; i++)
            {
                *dest++ = *src++;
            }

            // Update block bounding box
            updateBox(blockBox, itPoints->geom.box());

            itPoints++;
        }

        // Update coverage bounding box
        updateBox(coverageBox, blockBox);

        count++;

        // Insert block data as an entry in the coverage block table
        insertToCoverageTable(layer, tableName, count, blockBox, group.size(), blockData, blockSize);
    }

    // Create coverage layer, if it does not exist
    std::string& layerTableName = TeDefaultCoverageLayerTable(layer);
    if (!db->tableExist(layerTableName))
    {   
        createCoverageLayerTable(layer, layerTableName);
    }
    insertToCoverageLayerTable(layer, layerTableName, tableName, coverageId, coverageBox);

    // Check if this coverage layer is in the representations table
    TeRepresentation* rep = layer->getRepresentation(TeCOVERAGE, layerTableName);
    if (rep != NULL)
    {
        // Update bounding box of the representation
        updateBox(rep->box_, coverageBox);
        
        if (!db->updateRepresentation(layer->id(), *rep))
        {
            std::string errorMsg = "Couldn't update representation on layer '" + Te2String(layer->id()) + "'. DB error: \"" + db->errorMessage() + "\"";
            throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
        }
    }
    else
    {
        // Insert entry in the representations table
        TeRepresentation* rep = new TeRepresentation();
        rep->box_ = coverageBox;
        rep->geomRep_ = TeCOVERAGE;
        rep->tableName_ = layerTableName;
        layer->addVectRepres(rep);

        if (!db->insertRepresentation(layer->id(), *rep))
        {
            std::string errorMsg = "Couldn't insert representation on layer '" + Te2String(layer->id()) + "'. DB error: \"" + db->errorMessage() + "\"";
            throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
        }
    }

    // Update bounding box of the layer
    layer->updateLayerBox(coverageBox);
    if (!db->updateLayer(layer))
    {
        std::string errorMsg = "Couldn't update layer '" + Te2String(layer->id()) + "'. DB error: \"" + db->errorMessage() + "\"";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
}

//! Specialization for decoding a point layer
bool
decodeLayer(TeLayer* srcLayer, const std::vector<std::string>& columns, std::vector<SerializedPair<TePoint> >& serializedVector, std::vector<TeCoverageDimension>& dimensions)
{
    TeQuerierParams querierParams(true, columns);
    querierParams.setParams(srcLayer);
    TeQuerier querier(querierParams);
    querier.loadInstances();

    // Set information about the coverage dimensions
    TeAttributeList attList = querier.getAttrList();
    attributeListToCoverageDimensions(attList, dimensions);
    
    // Size of (x, y) pair
    int chunkSize = (2 * sizeof(double)) + TeCoverageDimensionsSize(dimensions);

    // Extract coverage data
    unsigned int instanceCount = 0;
    TeSTInstance sti;
    while(querier.fetchInstance(sti))
    {
        if(!sti.hasPoints())
        {
            // If it doesn't have a point, proceed to the next instance
            continue;
        }

        SerializedPair<TePoint> serialized;
        serialized.size = chunkSize;

        // Get only the first point
        TePointSet pointSet;
        sti.getGeometry(pointSet);
        TePoint point = *(pointSet.begin());
    
        serialized.geom = point;

        serialized.data = (unsigned char*)malloc(serialized.size);

        unsigned char* temp = serialized.data;

        // Write coordinates
        WRITE(temp, double, point.location().x_);
        WRITE(temp, double, point.location().y_);

        // Write dimensions values
        TePropertyVector vec = sti.getPropertyVector();
        for(unsigned int i = 0; i < vec.size(); i++)
        {
            if (dimensions[i].type == TeINTEGER)
            {
                std::stringstream buf;
                buf << vec[i].value_;
                int val;
                buf >> val;
                WRITE(temp, int, val);
                buf.clear();
            }
            else
            {
                std::stringstream buf;
                buf << vec[i].value_;
                double val;
                buf >> val;
                WRITE(temp, double, val);
                buf.clear();
            }
        }
        serializedVector.push_back(serialized);

    }
    return true;
}

bool
decodeLayer(TeLayer* srcLayer, const std::vector<std::string>& columns, std::vector<SerializedPair<TeLine2D> >& serializedVector, std::vector<TeCoverageDimension>& dimensions)
{
    TeQuerierParams querierParams(true, columns);
    querierParams.setParams(srcLayer);
    TeQuerier querier(querierParams);
    querier.loadInstances();

    // Set information about the coverage dimensions
    TeAttributeList attList = querier.getAttrList();
    attributeListToCoverageDimensions(attList, dimensions);

    int dimensionsSize = TeCoverageDimensionsSize(dimensions);
    
    // Extract coverage data
    unsigned int instanceCount = 0;
    TeSTInstance sti;
    while(querier.fetchInstance(sti))
    {
        if(!sti.hasLines())
        {
            // If it doesn't have a line, proceed to the next instance
            continue;
        }

        SerializedPair<TeLine2D> serialized;

        // Get only the first line
        TeLineSet lineSet;
        sti.getGeometry(lineSet);
        TeLine2D line = *(lineSet.begin());

        serialized.geom = line;

        int pointSize = (2 * sizeof(double));
        serialized.size = sizeof(unsigned int) + (line.size() * pointSize) + dimensionsSize;

        serialized.data = (unsigned char*)malloc(serialized.size);

        unsigned char* temp = serialized.data;

        WRITE(temp, unsigned int, line.size());

        // Write point coordinates
        for (unsigned int i = 0; i < line.size(); ++i)
        {
            TePoint point = line[i];
            WRITE(temp, double, point.location().x_);
            WRITE(temp, double, point.location().y_);
        }

        // Write dimensions values
        TePropertyVector vec = sti.getPropertyVector();
        for(unsigned int i = 0; i < vec.size(); i++)
        {
            if (dimensions[i].type == TeINTEGER)
            {
                std::stringstream buf;
                buf << vec[i].value_;
                int val;
                buf >> val;
                WRITE(temp, int, val);
                buf.clear();
            }
            else
            {
                std::stringstream buf;
                buf << vec[i].value_;
                double val;
                buf >> val;
                WRITE(temp, double, val);
                buf.clear();
            }
        }
        serializedVector.push_back(serialized);

    }
    return true;
}

bool
decodeLayer(TeLayer* srcLayer, const std::vector<std::string>& columns, std::vector<SerializedPair<TePolygon> >& serializedVector, std::vector<TeCoverageDimension>& dimensions)
{
    TeQuerierParams querierParams(true, columns);
    querierParams.setParams(srcLayer);
    TeQuerier querier(querierParams);
    querier.loadInstances();

    // Set information about the coverage dimensions
    TeAttributeList attList = querier.getAttrList();
    attributeListToCoverageDimensions(attList, dimensions);

    int dimensionsSize = TeCoverageDimensionsSize(dimensions);
    
    // Extract coverage data
    unsigned int instanceCount = 0;
    TeSTInstance sti;
    while(querier.fetchInstance(sti))
    {
        if(!sti.hasPolygons())
        {
            // If it doesn't have a polygon, proceed to the next instance
            continue;
        }

        SerializedPair<TePolygon> serialized;

        // Get only the first polygon
        TePolygonSet polySet;
        sti.getGeometry(polySet);
        TePolygon poly = *(polySet.begin());

        serialized.geom = poly;
        
        // Count number of lines and points that make this polygon
        unsigned int numLines = 0;
        unsigned int numPoints = 0;
        for(unsigned int i = 0; i < poly.size(); ++i)
        {
            numLines += 1;
            numPoints += poly[i].size();
        }

        int coordSize = (2 * sizeof(double));
        serialized.size = sizeof(unsigned int) + (numLines * sizeof(unsigned int)) + (numPoints * coordSize) + dimensionsSize;

        serialized.data = (unsigned char*)malloc(serialized.size);

        unsigned char* temp = serialized.data;

        // Write Polygon serialization
        WRITE(temp, unsigned int, numLines);
        for (unsigned int i = 0; i < numLines; ++i)
        {
            TeLine2D line = poly[i];
            WRITE(temp, unsigned int, line.size());

            // Write point coordinates
            for (unsigned int j = 0; j < line.size(); ++j)
            {
                TePoint point = line[j];
                WRITE(temp, double, point.location().x());
                WRITE(temp, double, point.location().y());
            }
        }

        // Write dimensions serialization
        TePropertyVector vec = sti.getPropertyVector();
        for(unsigned int i = 0; i < vec.size(); i++)
        {
            if (dimensions[i].type == TeINTEGER)
            {
                std::stringstream buf;
                buf << vec[i].value_;
                int val;
                buf >> val;
                WRITE(temp, int, val);
                buf.clear();
            }
            else
            {
                std::stringstream buf;
                buf << vec[i].value_;
                double val;
                buf >> val;
                WRITE(temp, double, val);
                buf.clear();
            }
        }
        serializedVector.push_back(serialized);

    }
    return true;
}

//! Import Coverage from a layer of geometries, creating a new layer
/*!
  \par Import a Coverage from an existing layer of geometries in
       a TerraLib database, creating a new layer where the coverage is stored.
  \param database a pointer to the database to where the coverage will be imported
  \param layerName name of the layer to be created
  \param srcLayer pointer to the existing layer of geometries
  \param srcAttrTableName name of an existing attributes table
  \param columns columns of the attributes table to be imported
  \param coverageId identifier of the coverage to be imported
  \return a pointer to the generated layer
*/
template <class T>
bool
TeCoverageImportLayer(TeLayer* layer, TeLayer* srcLayer, const std::vector<std::string>& columns, const std::string& coverageId)
{
    if (!layer || !srcLayer)
    {
        std::string errorMsg = "Couldn't import Point Coverage. Illegal parameters.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

    // Check if the layer has a coverage with the same coverage id
    std::vector<std::string> coverageIds;
    TeRetrieveCoverageIds(layer, coverageIds);
    if (find(coverageIds.begin(), coverageIds.end(), coverageId) != coverageIds.end())
    {
        std::string errorMsg = "Couldn't import Coverage. Coverage ID already in use.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
    
    std::vector<SerializedPair<T> > serialized;
    std::vector<TeCoverageDimension> dimensions;

    // Get data (geometry-value pairs) and metadata (dimensions) from the source layer
    if (!decodeLayer(srcLayer, columns, serialized, dimensions))
    {
        std::string errorMsg = "Couldn't import Coverage. Error decoding source layer.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

    // Set metadata table with dimensions information
    std::string& metadataTableName = TeDefaultCoverageMetadataTable(layer);
    fillCoverageMetadataTable(layer, metadataTableName, coverageId, dimensions);

    // Partition geometries in clusters
    std::vector<std::vector<SerializedPair<T> > > clusterGroups;
    clusterSerialized<T>(serialized, clusterGroups);

    // Add clusters to the database and update layer and representations info
    populateDatabase<T>(layer, coverageId, clusterGroups);

    return true;
}

//! Import Coverage from a layer of geometries
/*!
  \par Import a Coverage from an existing layer of geometries in
       a TerraLib database to an existing layer of a TerraLib database.
  \param layer pointer to a layer already created
  \param srcLayer pointer to the existing layer of geometries
  \param srcAttrTableName name of an existing attributes table
  \param columns columns of the attributes table to be imported
  \param coverageId identifier of the coverage to be imported
  \return a pointer to the generated layer
*/
template <class T>
TeLayer*
TeCoverageImportLayer(TeDatabase* database, const std::string& layerName, TeLayer* srcLayer, const std::vector<std::string>& columns, const std::string& coverageId)
{
    if (!database || layerName.empty() || !srcLayer)
    {
        std::string errorMsg = "Couldn't import Point Coverage. Illegal parameters.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

    TeLayer* newLayer = createNewCoverageLayer(database, layerName, srcLayer->projection());
    try {
        TeCoverageImportLayer<T>(newLayer, srcLayer, columns, coverageId);
    }
    catch (TeException e)
    {
	    database->deleteLayer(newLayer->id());
        throw e;
    }

	return newLayer;
}

//! Import Point Coverage from a Shape File, creating a new layer.
/*!
  \par Import a Point Coverage from a Shape File to a TerraLib
       database, creating a new layer where the coverage is stored.
  \param database a pointer to the database to where the coverage will
         be imported
  \param layerName name of the layer to be created
  \param projection projection used in shape file
  \param fileName name of the shape file to be read
  \param columns columns of the shape file to be imported
  \param coverageId identifier of the coverage to be imported
  \return a pointer to the generated layer
*/
//TeLayer* TePointCoverageImportSHP(TeDatabase* database, const std::string& layerName, TeProjection* projection, const std::string& fileName, const std::vector<std::string>& columns, const std::string& coverageId = "0");

//! Import Point Coverage from a Shape File.
/*!
  \par Import a Point Coverage from a Shape File to an existing
       layer of a TerraLib database.
  \param layer pointer to a layer already created
  \param fileName name of the shape file to be read
  \param columns columns of the shape file to be imported
  \param coverageId identifier of the coverage to be imported
  \return whether the coverage was imported successfully
*/
//bool TePointCoverageImportSHP(TeLayer* layer, const std::string& fileName, const std::vector<std::string>& columns, const std::string& coverageId = "0");


/*
bool
TePointCoverageImportSHP(TeLayer* layer, const std::string& fileName, const std::vector<std::string>& columns, const std::string& coverageId)
{
    if (!layer || fileName.empty())
    {
        std::string errorMsg = "Couldn't import Point Coverage. Illegal parameters.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

    // Check if the layer has a coverage with the same coverage id
    std::vector<std::string> coverageIds;
    TeRetrieveCoverageIds(layer, coverageIds);
    if (find(coverageIds.begin(), coverageIds.end(), coverageId) != coverageIds.end())
    {
        std::string errorMsg = "Couldn't import Point Coverage. Coverage ID already in use.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
    
    SerializedPointVector serializedPoints;
    std::vector<TeCoverageDimension> dimensions;

    // Get data (point-value pairs) and metadata (dimensions) from a ShapeFile
    if (!decodeSHPFile(fileName, columns, serializedPoints, dimensions))
    {
        std::string errorMsg = "Couldn't import Point Coverage. Error decoding Shape File.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

    // Set metadata table with dimensions information
    std::string& metadataTableName = TeDefaultCoverageMetadataTable(layer);
    fillCoverageMetadataTable(layer, metadataTableName, coverageId, dimensions);

    // Partition points in clusters
    std::vector<SerializedPointVector> clusterGroups;
    clusterSerializedPoints(serializedPoints, clusterGroups);

    // Add clusters to the database and update layer and representations info
    populateDatabase(layer, coverageId, clusterGroups);

    return true;
}

TeLayer*
TePointCoverageImportSHP(TeDatabase* database, const std::string& layerName, TeProjection* projection, const std::string& fileName, const std::vector<std::string>& columns, const std::string& coverageId)
{	
    if (!database || !projection || layerName.empty() || fileName.empty())
    {
        std::string errorMsg = "Couldn't import Point Coverage. Illegal parameters.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

    TeLayer* newLayer = createNewCoverageLayer(database, layerName, projection);
    try {
        TePointCoverageImportSHP(newLayer, fileName, columns, coverageId);
    }
    catch (TeException e)
    {
	    database->deleteLayer(newLayer->id());
        throw e;
    }

	return newLayer;
}
*/

#endif // __TERRALIB_INTERNAL_COVERAGEIMPORT_H