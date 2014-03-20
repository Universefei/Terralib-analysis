/*
TerraLib - a library for developing GIS applications.
Copyright  2001, 2002, 2003 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular
purpose. The library provided hereunder is on an "as is" basis, and the
authors have no obligation to provide maintenance, support, updates,
enhancements, or modifications.
In no event shall INPE be held liable to any party
for direct, indirect, special, incidental, or consequential damages arising
out of the use of this library and its documentation.
*/

#ifndef TEPDIIHSFUSION_HPP
  #define TEPDIIHSFUSION_HPP

  #include "TePDIAlgorithm.hpp"

  /**
   * @class TePDIIHSFusion
   * @brief This is the class for IHS fusion.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIFusionAlgorithms
   *
   * @note The required parameters are:
   * @param reference_raster ( TePDITypes::TePDIRasterPtrType ) - 
   *  Reference raster ( higher quality ).
   * @param lowres_rasters( TePDITypes::TePDIRasterVectorType ) - 
   * A vector of all used low resolution raster ( NOTE: When two or more bands
   * from the same raster are used, this raster must be inserted 
   * two or three times inside the vector.
   * @param lowres_channels ( std::vector< int > ) - A vector of all used 
   * low resolution raster rasters channels/bands.
   * @param output_raster ( TePDITypes::TePDIRasterPtrType ) - Output raster.
   * @param reference_raster_band ( int ) - The band to process from 
   * reference_raster.
   *
   * @note The optional parameters are:
   * @param interpol_method (TePDIInterpolator::InterpMethod) - Upsampling
   * interpolation method (default:BicubicMethod);
   *
   * @example TePDIFusion_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIIHSFusion : public TePDIAlgorithm{
  
    public :
      
      TePDIIHSFusion();      

      ~TePDIIHSFusion();

      //Overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;   
            
    protected :
    
      //Overloaded
      void ResetState( const TePDIParameters& params );    
     
      //Overloaded
      bool RunImplementation();      
  
  };

#endif
