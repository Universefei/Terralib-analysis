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

#ifndef TEPDIGEOMOSAIC_HPP
  #define TEPDIGEOMOSAIC_HPP

  #include "TePDIAlgorithm.hpp"
  
  #include "TePDIHistogram.hpp"

  /**
   * @class TePDIGeoMosaic 
   * @brief Mosaic of two geo-referenced rasters.
   * strategy.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIMosaicGroup
   *
   * @note The required parameters are:
   * @param input_raster1 ( TePDITypes::TePDIRasterPtrType ) - 
   *  Input raster 1.
   * @param input_raster2 ( TePDITypes::TePDIRasterPtrType ) - 
   *  Input raster 2.
   * @param channels1 ( std::vector< unsigned int > ) - The channels to process from 
   * input_raster1.
   * @param channels2 ( std::vector< unsigned int > ) - The channels to process from 
   * input_raster2. 
   * @param output_raster ( TePDITypes::TePDIRasterPtrType ) - Output raster.
   *
   * @note The optional parameters are:
   * @param blend_method ( TePDIBlender::BlendMethod ) - Blending method
   * (default: TePDIBlender::NoBlendMethod).   
   * @param interp_method ( TePDIInterpolator::InterpMethod ) - Interpolation
   * method (default TePDIInterpolator::NNMethod) .
   * @param dummy_value (double) - A dummy pixel value for use 
   * in pixels where no data is available.     
   * @param auto_equalize ( int ) - If present ( any value )
   * auto-equalizing will be made ( using overlap area reference ).
   * @param raster1_pol (TePolygon) - A polygon (raster 1 projected coords)
   * delimiting the raster region with valid data (if no polygon
   * is given the entire raster will be considered).
   * @param raster2_pol (TePolygon) - A polygon (raster 2 projected coords)
   * delimiting the raster region with valid data (if no polygon
   * is given the entire raster will be considered).     
   *
   * @note If the projections are different input_raster2 will be reprojected.
   * @note The reference raster will be that one with better resolution.
   * @note The output_raster parameters will take the choosed reference
   * raster parameters.   
   * @note The output raster data type will be automatically to fit the
   * range of both input rasters.
   * @example TePDIMosaic_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIGeoMosaic : public TePDIAlgorithm {
    public :
      

      TePDIGeoMosaic();

      ~TePDIGeoMosaic();
      
      //overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;      

    protected :
    
      //overloaded
      void ResetState( const TePDIParameters& params );    
     
      //overloaded
      bool RunImplementation();    
        
    /**
     * @brief Calc equalization parameters.
     * @param raster1 Raster 1 pointer.
     * @param channels1 Raster 1 channels.
     * @param r1Pol A polygon (raster 1 projected coords) delimiting
     * the area over raster 1 to be considered.
     * @param raster2 Raster 2 pointer.
     * @param channels2 Raster 2 channels.
     * @param r2Pol A polygon (raster 1 projected coords) delimiting
     * the area over raster 1 to be considered.
     * @param dummyValue Dummy value.
     * @param pixelOffsets1 The generated offsets values for raster 1.
     * @param pixelScales1 The generated scales values for raster 1.
     * @param pixelOffsets2 The generated offsets values for raster 2.
     * @param pixelScales2 The generated scales values for raster 2.
     * @return true if OK, false on errors.
     * @note Both rasters must have the same projection.
     */        
      bool calcEqParams( const TePDITypes::TePDIRasterPtrType& raster1, 
        const std::vector< unsigned int >& channels1,
        TePolygon& r1Pol,
        const TePDITypes::TePDIRasterPtrType& raster2,
        const std::vector< unsigned int >& channels2,
        TePolygon& r2Pol,
        const double& dummyValue,
        std::vector< double >& pixelOffsets1, 
        std::vector< double >& pixelScales1,
        std::vector< double >& pixelOffsets2, 
        std::vector< double >& pixelScales2 ) const;
        
    /**
     * @brief Calc mean and variances for each raster channel.
     * @param raster Raster pointer.
     * @param channels Raster channels.
     * @param polSet The restriction polygon set over input raster.
     * @param dummyValue Dummy value.
     * @param means The generated mean values.
     * @param means The generated variance values.
     * @return true if OK, false on errors.
     * @note Both rasters must have the same projection.
     */           
      bool calcStats( const TePDITypes::TePDIRasterPtrType& raster,
        const std::vector< unsigned int >& channels,
        TePolygonSet& polSet,
        const double& dummyValue,
        std::vector< double >& means,
        std::vector< double >& variances ) const;
  };

#endif
