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
  \file TeCoverageInterpolatorNN.h

  \par This file defines a Nearest Neighbour interpolation method for
       using with generic Coverage representations.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEINTERPOLATORNN_H
#define  __TERRALIB_INTERNAL_COVERAGEINTERPOLATORNN_H

#include "TeCoverageInterpolator.h"

/*!
  \class TeCoverageInterpolatorNN
  \brief Provides a Nearest Neighbour interpolation method for coverages.
*/
template <class T>
class TeCoverageInterpolatorNN : public TeCoverageInterpolator<T>
{
public:

    //! Set the decoder
    /*!
      \param decoder the coverage decoder
    */
    virtual void setDecoder(TeCoverageDecoder<T>* decoder)
    {
        decoder_ = decoder;

        // Get global bounding box and total number of coverage elements
        int numElements = decoder->getParameters().getNumElements();
        TeBox& decoderBox = decoder->getParameters().getBoundingBox();
        double boxArea = decoderBox.width() * decoderBox.height();

        // Calculate the initial radius for nearest neighbour interpolation
        // There is no need to be precise, this is just an heuristic.

        // search area = total area / number of elements
        // pi * r^2 = search area  -->  r = sqrt(search area / pi)
        initialNeighbourhoodRadius_ = sqrt((boxArea / numElements) / 3.14159);
    }


    //! Retrieve the coverage value at an arbitrary location.
    /*!
      \par Retrieve the coverage value at an arbitrary location, using the
           coverage decoder to access the coverage data.
      \par This implementation looks for the generic geometry that is the
           nearest neighbour of the position passed as parameter and returns
           the value associated to it.
      \param position the arbitrary location to be evaluated
      \param the value vector, to be filled by this method
    */
    virtual void evaluate(const TeCoord2D& position, std::vector<double>& value)
    {
        TeGeomValuePair<T> nearestPoint = nearestNeighbour(position);
        value = nearestPoint.value;
    }

    //! Fill a region of a raster with interpolated coverage data.
    /*!
      \par Fill the pixels of a squared region of a raster with coverage
           data. The value of each pixel coordinate is retrieved from
           the coverage, using an interpolation method (nearest neighbour).
      \param raster the raster to be filled with coverage data
      \param rasterBand the raster band to be filled
      \param coverageDimension the coverage dimension to be evaluated
      \param box defines the raster region to be filled
    */
    virtual void evaluate(TeRaster& raster, const int rasterBand, const int coverageDimension, const TeBox& box = TeBox())
    {
        // Setup iterators for traversing raster pixels
        TeRaster::iterator it, end;
        if (box == TeBox())
        {
            it = raster.begin();
            end = raster.end();
        }
        else
        {
            it = raster.begin(TeMakePolygon(box), TeBBoxPixelInters, rasterBand);
            end = raster.end(TeMakePolygon(box), TeBBoxPixelInters, rasterBand);
        }
        
        while(it != end)
        {
            // Get raster and coverage projections
            TeProjection* coverageProj = decoder_->getParameters().getProjection();
            TeProjection* rasterProj = raster.projection();
            rasterProj->setDestinationProjection(coverageProj);

            // Get pixel coordinate
            TeCoord2D coord = raster.index2Coord(
                    TeCoord2D(it.currentColumn(), it.currentLine()));
            
            // Convert to the intermediate projection
            coord = rasterProj->PC2LL(coord);

            // Convert to the coverage projection
            coord = coverageProj->LL2PC(coord);

            // Find coverage value and set raster element
            std::vector<double> value = std::vector<double>();
            evaluate(coord, value);
            raster.setElement(it.currentColumn(), it.currentLine(), value[coverageDimension], rasterBand);

            it++;
        }
    }

protected:

    double initialNeighbourhoodRadius_; //!< The initial radius used for interpolation.

    //! Find nearest generic geometry-value pair.
    /*!
      \par Retrieve the generic geometry value-pair that is the nearest
           neighbour of the arbitrary location passed as parameter.
      \par The radius parameter is used to specify the size of the
           neighbourhood to be checked first. If no neighbours can be
           found, the radius is increased until at least one neighbour
           is found.
      \param position an arbitrary location
      \param radius the size of the neighbourhood to be checked initially.
    */
    virtual TeGeomValuePair<T> nearestNeighbour(const TeCoord2D& position, double radius = 0)
    {
        if (!decoder_)
        {
            std::string errorMsg = "Couldn't evaluate position in coverage. Decoder is NULL.";
            throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
        }
        if (!radius)
        {
            radius = initialNeighbourhoodRadius_;
        }

        TeBox box = TeBox(position.x_ - radius,
                        position.y_ - radius,
                        position.x_ + radius,
                        position.y_ + radius);

        std::vector<TeGeomValuePair<T> >& selected = std::vector<TeGeomValuePair<T> >();
        decoder_->selectGeomValuePairs(TeMakePolygon(box), TeINTERSECTS, selected);

        if (selected.size() > 0)
        {
            // Find nearest geometry, among the selected set
            double minDist = TeMAXFLOAT;
            double dist = TeMAXFLOAT;
            std::vector<TeGeomValuePair<T> >::iterator itNearest = selected.begin();
            for(std::vector<TeGeomValuePair<T> >::iterator it = selected.begin(); it != selected.end(); ++it)
            {
                dist = distanceToGeom(position, it->geom);
                if (dist < minDist)
                {
                    minDist = dist;
                    itNearest = it;
                }
            }
            
            // Check if the distance is less than the radius
            // Otherwise, there might be a closer geometry, outside the box
            if (minDist <= radius)
            {
                return (*itNearest);
            }
            else
            {
                return nearestNeighbour(position, minDist);
            }
        }
        else
        {
            // If coudn't find any geometries, search in a larger area
            return nearestNeighbour(position, radius * 2);
        }
    }

    //! Evaluate distance from a position to a point
    double distanceToGeom(const TeCoord2D& position, TePoint point)
    {
        return TeDistance(position, point.location());
    }

    //! Evaluate distance from a position to a line
    double distanceToGeom(const TeCoord2D& position, TeLine2D line)
    {
        double minDist = TeMAXFLOAT;
        double dist = TeMAXFLOAT;

        // Evaluate the distance to the line segment-by-segment
        for (unsigned int i = 0; i < (line.size() - 1); ++i)
        {
            dist = TeMinimumDistance (line[i], line[i + 1], position, TeCoord2D(), 0);
            if (dist < minDist)
            {
                minDist = dist;
            }
        }
        return minDist;
    }

    //! Evaluate distance from a position to a polygon
    double distanceToGeom(const TeCoord2D& position, TePolygon poly)
    {
        // If the position is inside the polygon, return zero
        if (TeWithin(position, poly))
        {
            return 0.0;
        }

        double minDist = TeMAXFLOAT;
        double dist = TeMAXFLOAT;

        // Evaluate the distance to each ring of the polygon segment-by-segment
        for(TePolygon::iterator itPoly = poly.begin(); itPoly != poly.end(); ++itPoly)
        {
            TeLine2D line = *itPoly;

            for (unsigned int i = 0; i < (line.size() - 1); ++i)
            {
                dist = TeMinimumDistance (line[i], line[i + 1], position, TeCoord2D(), 0);
                if (dist < minDist)
                {
                    minDist = dist;
                }
            }
        }
        return minDist;
    }

};

#endif // __TERRALIB_INTERNAL_COVERAGEINTERPOLATORNN_H