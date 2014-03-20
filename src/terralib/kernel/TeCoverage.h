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
  \file TeCoverage.h

  \par This file contains structures and definitions for a generic
       Coverage representation.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGE_H
#define  __TERRALIB_INTERNAL_COVERAGE_H

#include "TeGeometry.h"
#include "TeDataTypes.h"
#include "TeCoverageParams.h"
#include "TeCoverageDecoder.h"
#include "TeCoverageDecoderCacheLRU.h"
#include "TeCoverageDecoderDatabase.h"
#include "TeCoverageInterpolator.h"
#include "TeCoverageInterpolatorNN.h"
#include <vector>

/*!
  \class TeCoverage
  \brief A generic Coverage class.

  \par Coverage is a way of representing geographic information that
       emphasizes relationships and the spatial distribution of earth
       phenomena, rather than emphasizing features of independent locations.
  \par This class represents a generic Coverage, which is a Coverage
       with a generic geographic domain, e.g. a set of points or a set
       of surfaces. Each element of the domain is associated to a value vector
       that represents the different dimensions of the coverage information.
  \par Each coverage is connected to a decoder, which provides a common
       interface for retrieving coverage data wherever it is stored.
  \par The elements of the domain can be traversed and its values can be
       consulted using the coverage iterator. Also, the values on arbitrary
       locations can be consulted using interpolation methods.
  \sa TeCoverageParams TeCoverageDecoder TeCoverageInterpolator
*/
template <class T>
class TeCoverage : public TeGeometry
{
public:

    //! Constructor from parameters and interpolator
    /*!
      Construct a coverage object, but do not initialize the internal
      structures. The interpolator provided must be for exclusive use of
      this coverage, any external use of the interpolator is discouraged.
      \param params information about the coverage
      \param interpolator pointer to a coverage iterpolator
    */
    TeCoverage(TeCoverageParams& params, TeCoverageInterpolator<T>* interpolator = NULL) :
        params_(params),
        interpolator_(interpolator),
        decoder_(NULL)
    {
    }

    //! Coverage destructor
    /*!
      Clear internal structures before destroying the coverage object.
    */
    virtual ~TeCoverage()
    {
        clear();
    }

    //! Initalize the coverage
    /*!
      Instantiate internal structures, preparing the coverage for actual use.
      Must be called before any attempt to read from the coverage.
    */
    virtual void init()
    {
        // Initialize decoder according to persistence type
        if (!decoder_)
        {
            if (params_.getPersistenceType() == TePERSISTENCE_DATABASE)
            {
                decoder_ = new TeCoverageDecoderDatabase<T>(params_);
                decoder_->init();
            }
            else if (params_.getPersistenceType() == TePERSISTENCE_DATABASE_CACHELRU)
            {
                decoder_ = new TeCoverageDecoderCacheLRU<T>(params_);
                decoder_->init();
            }
        }

        // Initialize interpolator
        if (!interpolator_)
        {
            // If undefined, create default interpolator
            interpolator_ = createDefaultInterpolator();
        }
        interpolator_->setDecoder(decoder_);

        // Set box according to params
        setBox(params_.getBoundingBox());
    }

    //! Clear internal structures
    /*!
      Clear internal structures such as de decoder and the interpolator.
    */
    virtual void clear()
    {
        if (decoder_)
        {
            delete(decoder_);
            decoder_ = NULL;
        }
        if (interpolator_)
        {
            delete(interpolator_);
            interpolator_ = NULL;
        }
    }

    //! Return the type of the geometry
    /*!
      \return the type of the geometry
    */
    TeGeomRep elemType()
    {
        return TeCOVERAGE;
    }

    //! Return the coverage parameters
    /*!
      \return the coverage parameters
    */
    TeCoverageParams& getParameters()
    {
        return params_;
    }

    /*!
      \class iterator
      \brief A generic Coverage iterator.

      \par An iterator for a Coverage representation. This iterator
           is specific for traversing a set of generic geometries and basically
           it encapsulates a generic geometry-value pair iterator.
      \par The move-forward operators are used to traverse the coverage data.
           At any point of the traversal, dereferencing operators might be used
           to access the generic geometry, while the "[]" operator might be
           used to access the values in different dimensions of the coverage.
      \sa TeCoverage
    */
    class iterator
    {
        /*!
          \typedef geom_it
          \brief Generic geometry-value pair vector iterator.
        */
        typedef typename std::vector<TeGeomValuePair<T> >::iterator geom_it;
    public:

        //! Constructor from the base iterator
        iterator(geom_it it) :
            it_(it)
        {
        }
          
        //! Destructor
        virtual ~iterator()
        {
        }

        //! Prefix move-forward operator
        iterator operator++()
        {
            it_++;
            return *this;
        }
    
        //! Posfix move-forward operator
        iterator operator++(int)
        {
            iterator temp = *this;
            ++it_;
            return temp;
        }

        //! Operator for retrieving an instance of a generic geometry
        /*!
          \return an instance of a generic geometry
        */
        T operator*()
        {
            return it_->geom;
        }

        //! Operator for retrieving a pointer to a generic geometry
        /*!
          \return a pointer to a generic geometry
        */
        T* operator->()
        {
            return &(it_->geom);
        }
        
        //! Operator for getting the value at a dimension of the coverage
        /*!
          \param dimension the dimension to be retrieved
          \return a double value
        */
        double operator[](int dimension)
        {
            return it_->value[dimension];
        }
        
        //! Return whether two iterators point to the same element
        bool operator==(const iterator& rhs)
        {
            return it_ == rhs.it_;
        }

        //! Return whether two iterators point to the different elements
        bool operator!=(const iterator& rhs)
        {
            return it_ != rhs.it_;
        }

    protected:
        geom_it it_; //!< Generic geometry-value pair vector iterator
    };   

    // ---------------------  End of class TeCoverage::iterator

    //! Return the beginning iterator
    /*!
      \par Return the beginning iterator for traversing the generic geometries
           that form the spatial domain of this coverage.
      \par The polygon parameter may be used to define a selection area. If no
           polygon is provided, the selection area includes all the domain.
      \par The relation parameter may be used to specify the kind of relation
           (e.g. intersection, crossing, overlapping) that holds between the
           selection area and the geometries to be selected . If no relation
           is provided, it defaults to intersection.
      \param poly an instance of TePolygon representing a selection area
      \param relation a spatial relation
      \return the beginning iterator
    */
    typename TeCoverage<T>::iterator begin(
            TePolygon& poly = TePolygon(),
            TeSpatialRelation relation = TeINTERSECTS)
    {
        std::vector<TeGeomValuePair<T> >& selected = selectGeomValuePairs(poly, relation);
        TeCoverage<T>::iterator beginIt = TeCoverage<T>::iterator(selected.begin());
        return beginIt;
    }

    //! Return the ending iterator
    /*!
      \par Return the beginning iterator for traversing the generic geometries
           that form the spatial domain of this coverage.
      \par The polygon parameter may be used to define a selection area. If no
           polygon is provided, the selection area includes all the domain.
      \par The relation parameter may be used to specify the kind of relation
           (e.g. intersection, crossing, overlapping) that holds between the
           selection area and the geometries to be selected . If no relation
           is provided, it defaults to intersection.
      \param poly an instance of TePolygon representing a selection area
      \param relation a spatial relation
      \return the ending iterator
    */
    typename TeCoverage<T>::iterator end(
            TePolygon& poly = TePolygon(),
            TeSpatialRelation relation = TeINTERSECTS)
    {
        std::vector<TeGeomValuePair<T> >& selected = selectGeomValuePairs(poly, relation);
        TeCoverage<T>::iterator endIt = TeCoverage<T>::iterator(selected.end());
        return endIt;
    }

    //! Evaluate the value of this coverage at an arbitrary location.
    /*!
      Evaluate the value of this coverage at an arbitrary location, using
      the auxiliary interpolator object.
      \param position the arbitrary location to be evaluated
      \param the value vector, to be filled by this method
    */
    void evaluate(const TeCoord2D& location, std::vector<double>& value)
    {
        interpolator_->evaluate(location, value);
    }

    //! Fill a region of a raster with interpolated coverage data.
    /*!
      \par Fill the pixels of a squared region of a raster with
           coverage data, using an interpolation method.
      \param raster the raster to be filled with coverage data
      \param rasterBand the raster band to be filled
      \param coverageDimension the coverage dimension to be evaluated
      \param box defines the raster region to be filled
    */
    void evaluate(TeRaster& raster, const int rasterBand, const int coverageDimension, const TeBox& box = TeBox())
    {
        interpolator_->evaluate(raster, rasterBand, coverageDimension, box);
    }

protected:

    TeCoverageParams& params_; //!< stores information about this coverage
    TeCoverageDecoder<T>* decoder_; //!< provides access to the coverage data
    TeCoverageInterpolator<T>* interpolator_; //!< provides interpolation methods

    //! Create a default interpolator
    /*!
      Create a default interpolator. This method is used in case no other
      interpolator was provided on construction.
    */
    TeCoverageInterpolator<T>* createDefaultInterpolator()
    {
        // Default Coverage interpolator is nearest neighbour
        return new TeCoverageInterpolatorNN<T>();
    }

    //! Select generic geometry-value pairs from the coverage
    /*!
      \par Select generic geometry-value pairs from the coverage, using the
           decoder to access the coverage data.
      \par The polygon parameter defines a selection area and the relation
           parameter specifies the kind of relation (e.g. intersection,
           crossing, overlapping) that holds between the selection area and
           the geometries to be selected.
      \param poly an instance of TePolygon representing a selection area
      \param relation a spatial relation
    */
    std::vector<TeGeomValuePair<T> >& selectGeomValuePairs(const TePolygon& poly, const TeSpatialRelation relation)
    {
        std::vector<TeGeomValuePair<T> >& selected = lastSelectionResult_;

        // Search decoder only if the selection is different from the last one
        if ((!TeEquals(lastSelectionPolygon_, poly)) || lastSelectionRelation_ != relation)
        {
            decoder_->selectGeomValuePairs(poly, relation, selected);

            // Save selection parameters and result
            lastSelectionPolygon_ = poly;
            lastSelectionRelation_ = relation;
            lastSelectionResult_ = selected;
        }
        return selected;
    }

private:

    TePolygon lastSelectionPolygon_; //!< the last polygon used for a selection
    TeSpatialRelation lastSelectionRelation_; //!< the last relation used for a selection
    std::vector<TeGeomValuePair<T> > lastSelectionResult_; //!< the last result of a selection
};


#endif // __TERRALIB_INTERNAL_COVERAGE_H