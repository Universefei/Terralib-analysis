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
  \file TeCoverageDecoderDatabase.h

  \par This file contains definitions and algorithms for accessing a
       generic Coverage from a TerraLib database.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEDECODERDATABASE_H
#define  __TERRALIB_INTERNAL_COVERAGEDECODERDATABASE_H

#include "TeCoverageDecoder.h"

/*!
  \class TeCoverageDecoderDatabase
  \brief Class to decode a generic Coverage from a TerraLib database.
*/
template <class T>
class TeCoverageDecoderDatabase : public TeCoverageDecoder<T>
{
public:

    //! Constructor from parameters
    TeCoverageDecoderDatabase(TeCoverageParams& params) :
        TeCoverageDecoder<T>(params)
    {
    }

    //! Initialize internal structures
    /*!
      Instantiate internal structures, must be called before any
      attempt to access the coverage data.
    */
    virtual void init()
    {
        TeLayer* layer = TeRetrieveLayer(params_.getDatabase(), params_.getLayerId());

        // Check if the coverage ID is specified
        if (params_.getCoverageId().empty())
        {
            // Otherwise, get the first coverage (if any)
            std::vector<std::string> coverageIds;
            TeRetrieveCoverageIds(layer, coverageIds);
            if (coverageIds.empty())
            {
                std::string errorMsg = "No coverage defined on layer " + Te2String(layer->id()) + ".";
                throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
            }
            // New coverage ID is the first coverage found on layer
            params_.setCoverageId(coverageIds.at(0));
        }

        // Set coverage table name
        params_.setCoverageTable(TeRetrieveCoverageTable(layer, params_.getCoverageId()));

        // Retrieve coverage dimensions information from the database
        std::vector<TeCoverageDimension> dimensions;
        TeRetrieveDimensions(layer, params_.getCoverageId(), dimensions);
        params_.setDimensions(dimensions);

        // Set coverage projection
        params_.setProjection(layer->projection());

        // Set coverage bounding box
        params_.setBoundingBox(TeRetrieveCoverageBox(layer, params_.getCoverageId()));

        // Set number of elements of the coverage
        params_.setNumElements(TeRetrieveCoverageNumElements(layer, params_.getCoverageId()));
    }

    //! Select generic coverage blocks from the database.
    /*!
      \par Select generic coverage blocks from a TerraLib database. Uses
           a TeBlockLoader object for retrieving blocks from the database.
      \par The polygon parameter defines a selection area and the relation
           parameter specifies the kind of relation (e.g. intersection,
           crossing, overlapping) that holds between the selection area and
           at least one geometry in each block to be selected.
      \param poly an instance of TePolygon representing a selection area
      \param relation a spatial relation
      \param dontSelect list IDs of blocks that must not be retrieved
      \sa TeCoverageBlock TeSpatialRelation TeBlockLoader
    */
    void selectBlocks(const TePolygon& poly, const TeSpatialRelation relation, std::vector<TeCoverageBlock<T> >& selected, std::set<int>& dontSelect)
    {
        selected.clear();

        TeBox queryBox = poly.box();

        // Set block loader parameters.
        TeBlockLoaderParams loaderParams;
        loaderParams.db = params_.getDatabase();
        loaderParams.table_name = params_.getCoverageTable();
        loaderParams.selection_box = queryBox;
        loaderParams.load_all_attrs = true;
        loaderParams.dont_load_ids = dontSelect;

        // Construct loader and load blocks.
        TeBlockLoader loader(loaderParams);
        loader.loadBlocks();

        // Iterate over blocks loaded
        while (loader.fetchNext()) {

            TeBox blockBox(loader.getDouble("lower_x"),
                        loader.getDouble("lower_y"),
                        loader.getDouble("upper_x"),
                        loader.getDouble("upper_y"));

            TePolygon& blockPoly = polygonFromBox(blockBox);

            // If the spatial relation doesn't hold, proceed to the next block
            if (!TeTopologicalRelation(&blockPoly, &poly, relation))
            {
                continue;
            }

            // Extract block contents.
            long blockSize = 0;
            unsigned char* data = NULL;
            loader.getSpatialData(data, blockSize);
            
            // Decode block contents.
            TeCoverageBlock<T>& block = decodeCoverageBlock(data, blockSize, params_.getDimensions());

            // Set other block attributes
            block.id = loader.getID();
            block.box = blockBox;

            // Add block to result
            selected.push_back(block);
        }
    }

protected:

    //! Decode a generic geometry-value pair
    /*!
      \par Decode a generic geometry-value pair from a block of raw data.
           As a side effect, the pointer to the data is moved forward to the
           end of the decoded chunk.
        \param data pointer to the memory location where the raw data starts
        \param dimensions information about the coverage dimensions
        \return a generic geometry-value pair
    */
    TeGeomValuePair<T> decodeGeomValuePair(unsigned char*& data, std::vector<TeCoverageDimension>& dimensions);

    //! Decode a generic coverage block
    /*!
        \par Decode coverage block contents that are stored in the database
             as raw binary data. As a side effect, the pointer to the data
             is moved forward to the end of the decoded chunk.
        \param data pointer to the memory location where the raw data starts
        \param dataSize size of the raw data block to be decoded
        \param dimensions information about the coverage dimensions
        \return a generic coverage block
    */
    TeCoverageBlock<T> decodeCoverageBlock(unsigned char*& data, const long dataSize, std::vector<TeCoverageDimension>& dimensions)
    {
        TeCoverageBlock<T> block;

        unsigned char* dataEnd = data + dataSize;

        // Decode geom-value pairs from chunks of the block
        unsigned int count = 0;
        while (data < dataEnd)
        {
            TeGeomValuePair<T>& gvPair = decodeGeomValuePair(data, dimensions);
            gvPair.geom.objectId(Te2String(count++));

            block.pairs.push_back(gvPair); // Insert geom-value pair in block
        }
        return block;
    }
};

//! Specialization for decoding a TePoint instead of a generic geometry
TeGeomValuePair<TePoint>
TeCoverageDecoderDatabase<TePoint>::decodeGeomValuePair(unsigned char*& data, std::vector<TeCoverageDimension>& dimensions)
{
    TeGeomValuePair<TePoint> gvPair = TeGeomValuePair<TePoint>();

    // Set coordinates
    double x = READ(data, double);
    double y = READ(data, double);

    gvPair.geom = TePoint(x, y);
    
    // Set values
    gvPair.value = std::vector<double>();
    std::vector<TeCoverageDimension>::iterator it = dimensions.begin();
    std::vector<TeCoverageDimension>::iterator end = dimensions.end();
    while (it != end)
    {
        double val = 0;

        switch (it->type)
        {
        case (TeUNSIGNEDSHORT):
            val = READ(data, unsigned short);
            break;
        case (TeSHORT):
            val = READ(data, short);
            break;
        case (TeINTEGER):
            val = READ(data, int);
            break;                
        case (TeUNSIGNEDLONG):
            val = READ(data, unsigned long);
            break;
        case (TeLONG):
            val = READ(data, long);
            break;
        case (TeFLOAT):
            val = READ(data, float);
            break;
        case (TeDOUBLE):
            val = READ(data, double);
            break;
        default:
            // Exception (invalid type)
            break;
        }

        gvPair.value.push_back(val);
        it++;
    }

    return gvPair;
}

//! Specialization for decoding a TeLine2D instead of a generic geometry
TeGeomValuePair<TeLine2D>
TeCoverageDecoderDatabase<TeLine2D>::decodeGeomValuePair(unsigned char*& data, std::vector<TeCoverageDimension>& dimensions)
{
    TeGeomValuePair<TeLine2D> gvPair = TeGeomValuePair<TeLine2D>();

    TeLine2D line = TeLine2D();

    // Check number of points in the line
    unsigned int numPoints = READ(data, unsigned int);

    // Decode points and add to the line
    for (unsigned int i = 0; i < numPoints; ++i)
    {
        double x = READ(data, double);
        double y = READ(data, double);

        line.add(TeCoord2D(x, y));
    }

    gvPair.geom = line;
    
    // Set values
    gvPair.value = std::vector<double>();
    std::vector<TeCoverageDimension>::iterator it = dimensions.begin();
    std::vector<TeCoverageDimension>::iterator end = dimensions.end();
    while (it != end)
    {
        double val = 0;

        switch (it->type)
        {
        case (TeUNSIGNEDSHORT):
            val = READ(data, unsigned short);
            break;
        case (TeSHORT):
            val = READ(data, short);
            break;
        case (TeINTEGER):
            val = READ(data, int);
            break;                
        case (TeUNSIGNEDLONG):
            val = READ(data, unsigned long);
            break;
        case (TeLONG):
            val = READ(data, long);
            break;
        case (TeFLOAT):
            val = READ(data, float);
            break;
        case (TeDOUBLE):
            val = READ(data, double);
            break;
        default:
            // Exception (invalid type)
            break;
        }

        gvPair.value.push_back(val);
        it++;
    }

    return gvPair;
}

//! Specialization for decoding a TePolygon instead of a generic geometry
TeGeomValuePair<TePolygon>
TeCoverageDecoderDatabase<TePolygon>::decodeGeomValuePair(unsigned char*& data, std::vector<TeCoverageDimension>& dimensions)
{
    TeGeomValuePair<TePolygon> gvPair = TeGeomValuePair<TePolygon>();

    TePolygon poly = TePolygon();
    // Check number of linear rings in the polygon
    unsigned int numLines = READ(data, unsigned int);

    for (unsigned int i = 0; i < numLines; ++i)
    {
        TeLine2D line = TeLine2D();

        // Check number of points in the line
        unsigned int numPoints = READ(data, unsigned int);

        // Decode points and add to the line
        for (unsigned int i = 0; i < numPoints; ++i)
        {
            double x = READ(data, double);
            double y = READ(data, double);

            line.add(TeCoord2D(x, y));
        }

        TeLinearRing ring = TeLinearRing(line);
        poly.add(ring);
    }
    gvPair.geom = poly;
    
    // Set values
    gvPair.value = std::vector<double>();
    std::vector<TeCoverageDimension>::iterator it = dimensions.begin();
    std::vector<TeCoverageDimension>::iterator end = dimensions.end();
    while (it != end)
    {
        double val = 0;

        switch (it->type)
        {
        case (TeUNSIGNEDSHORT):
            val = READ(data, unsigned short);
            break;
        case (TeSHORT):
            val = READ(data, short);
            break;
        case (TeINTEGER):
            val = READ(data, int);
            break;                
        case (TeUNSIGNEDLONG):
            val = READ(data, unsigned long);
            break;
        case (TeLONG):
            val = READ(data, long);
            break;
        case (TeFLOAT):
            val = READ(data, float);
            break;
        case (TeDOUBLE):
            val = READ(data, double);
            break;
        default:
            // Exception (invalid type)
            break;
        }

        gvPair.value.push_back(val);
        it++;
    }

    return gvPair;
}

#endif // __TERRALIB_INTERNAL_COVERAGEDECODERDATABASE_H