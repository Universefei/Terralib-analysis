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
  \file TeCoverageDecoder.h

  \par This file contains definitions and algorithms for accessing persistent
       representations of a generic Coverage.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEDECODER_H
#define  __TERRALIB_INTERNAL_COVERAGEDECODER_H

#include "TeCoord2D.h"
#include "TeException.h"
#include "TeBlockLoader.h"
#include "TeCoverageParams.h"
#include "TeCoverageUtils.h"
#include "TeSpatialOperations.h"
#include <map>
#include <vector>

/*!
  \def READ
  \brief Read a value of some type from a pointer to a block of raw data.
*/
#define READ(data, t) *(t*)data; data += sizeof(t)

/*!
  \struct TeGeomValuePair
  \brief A generic geometry-value pair.
*/
template <class T>
struct TeGeomValuePair
{
    T geom; //!< A generic geometry
    std::vector<double> value; //!< a value vector

    //! Default constructor.
    TeGeomValuePair(){}

    //! Constructor for a generic geometry-value pair
    TeGeomValuePair(const T& geom,
                     const std::vector<double>& value):
            geom(geom),
            value(value){}
};



/*!
  \struct TeCoverageBlock
  \brief A generic coverage block.

  \par A generic coverage block contais a set of generic geometry-value pairs.
  \par This coverage block has a box, which is the minimum box enclosing all
       the generic geometries that are in the block.
*/
template <class T>
struct TeCoverageBlock
{
    int id; //!< The identifier of the block
    TeBox box; //!< The minimum box enclosing all elements of the block
    std::vector<TeGeomValuePair<T> > pairs; //!< vector of geometry-value pairs
};

/*!
  \class TeCoverageDecoder
  \brief Abstract class of a coverage data decoder.

  \par The decoder is used to provide a common interface for accessing
       coverage data regardless of where and how it is stored.
  \sa TeGeomValuePair TeCoverageBlock
*/
template <class T>
class TeCoverageDecoder
{
public:

    //! Constructor from coverage parameters
    TeCoverageDecoder(TeCoverageParams& params) :
        params_(params)
    {
    }

    //! Destructor
    virtual ~TeCoverageDecoder()
    {
        clear();
    }

    //! Return the coverage parameters
    virtual TeCoverageParams& getParameters()
    {
        return params_;
    }

    //! Initialize internal structures
    /*!
      Instantiate internal structures, must be called before any attempt
      to access the coverage data.
    */
    virtual void init()
    {
    }

    //! Clear internal structures
    /*!
      Clear internal structures, must be called before disposing of the
      decoder.
    */
    virtual void clear()
    {
    }

    //! Select generic geometry-value pairs from the coverage data
    /*!
      \par Select generic geometry-value pairs from the coverage data.
      \par The polygon parameter defines a selection area and the relation
           parameter specifies the kind of relation (e.g. intersection,
           crossing, overlapping) that holds between the selection area and
           the geometries to be selected.
      \param poly an instance of TePolygon representing a selection area
      \param relation a spatial relation
      \sa TeGeomValuePair
    */
    virtual void selectGeomValuePairs(const TePolygon& poly, const TeSpatialRelation relation, std::vector<TeGeomValuePair<T> >& selected)
    {
        // Select blocks which contain geometries that might respect the
        // spatial relation with the polygon
        std::vector<TeCoverageBlock<T> >& blocks = std::vector<TeCoverageBlock<T> >();
        selectBlocks(poly, TeINTERSECTS, blocks);

        for (std::vector<TeCoverageBlock<T> >::iterator itBlocks = blocks.begin(); itBlocks != blocks.end(); ++itBlocks)
        {
            // Look at all geometry-value pairs of selected blocks
            std::vector<TeGeomValuePair<T> >& pairs = itBlocks->pairs;
            for (std::vector<TeGeomValuePair<T> >::iterator it = pairs.begin(); it != pairs.end(); ++it)
            {
                // If the spatial relation holds, add geom-value pair to the result
                if (TeTopologicalRelation(&poly, &it->geom, relation))
                {
                    selected.push_back(*it);
                }
            }
        }
    }

    //! Select generic coverage blocks from the coverage data
    /*!
      \par Select generic coverage blocks from the coverage data.
      \par The polygon parameter defines a selection area and the relation
           parameter specifies the kind of relation (e.g. intersection,
           crossing, overlapping) that holds between the selection area and
           at least one geometry in each block to be selected.
      \param poly an instance of TePolygon representing a selection area
      \param relation a spatial relation
      \param dontSelect list IDs of blocks that must not be retrieved
      \sa TeCoverageBlock TeSpatialRelation
    */
    virtual void selectBlocks(const TePolygon& poly, const TeSpatialRelation relation, std::vector<TeCoverageBlock<T> >& selected, std::set<int>& dontSelect = std::set<int>()) = 0;

protected:

    TeCoverageParams& params_;  //!< Coverage parameters.
};

#endif // __TERRALIB_INTERNAL_COVERAGEDECODER_H