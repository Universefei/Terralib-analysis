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
  \file TeCoverageInterpolator.h
  
  \par This file defines an interpolation method for using with 
       Coverage representations
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEINTERPOLATOR_H
#define  __TERRALIB_INTERNAL_COVERAGEINTERPOLATOR_H

#include "TeCoverageDecoder.h"

/*!
  \class TeCoverageInterpolator
  \brief Provides an interpolation method for coverages.

  \par Abstract class for providing an interpolation method for generic
       Coverages.
  \par Assumes that the coverage decoder will be provided, for more efficient
       access to the the coverage data.
*/
template <class T>
class TeCoverageInterpolator
{
public:

    //! Default constructor
    TeCoverageInterpolator() :
        decoder_(NULL)
    {
    }

    //! Destructor
    virtual ~TeCoverageInterpolator()
    {
    }

    //! Set the decoder
    /*!
      \param decoder the coverage decoder
    */
    virtual void setDecoder(TeCoverageDecoder<T>* decoder)
    {
        decoder_ = decoder;
    }

    //! Return the decoder
    /*!
      \return the coverage decoder
    */
    virtual TeCoverageDecoder<T>* getDecoder() const
    {
        return decoder_;
    }

    //! Retrieve the coverage value at an arbitrary location.
    /*!
      \par Retrieve the coverage value at an arbitrary location, using the
           coverage decoder to access the coverage data.
      \param position the arbitrary location to be evaluated
      \param the value vector, to be filled by this method
    */
    virtual void evaluate(const TeCoord2D& position, std::vector<double>& value) = 0;

    //! Fill a region of a raster with interpolated coverage data.
    /*!
      \par Fill the pixels of a squared region of a raster with
           coverage data, using an interpolation method.
      \param raster the raster to be filled with coverage data
      \param rasterBand the raster band to be filled
      \param coverageDimension the coverage dimension to be evaluated
      \param box defines the raster region to be filled
    */
    virtual void evaluate(TeRaster& raster, const int rasterBand, const int coverageDimension, const TeBox& box = TeBox()) = 0;

protected:

    TeCoverageDecoder<T>* decoder_; //!< The coverage decoder.
};

#endif // __TERRALIB_INTERNAL_COVERAGEINTERPOLATOR_H