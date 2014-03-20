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

#ifndef TEPDITPMOSAIC_HPP
  #define TEPDITPMOSAIC_HPP

  #include "TePDIAlgorithm.hpp"
  
  #include "TePDIHistogram.hpp"

  /**
   * @class TePDITPMosaic 
   * @brief Mosaic of two rasters linked by a tie-points set.
   * strategy.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIMosaicGroup
   *
   * @note The required parameters are:
   * @param input_raster1 ( TePDITypes::TePDIRasterPtrType ) - 
   *  Input raster 1 (this will be the reference raster).
   * @param input_raster2 ( TePDITypes::TePDIRasterPtrType ) - 
   *  Input raster 2.
   * @param channels1 ( std::vector< unsigned int > ) - The channels to process from 
   * input_raster1.
   * @param channels2 ( std::vector< unsigned int > ) - The channels to process from 
   * input_raster2. 
   * @param output_raster ( TePDITypes::TePDIRasterPtrType ) - Output raster.
   * @param trans_params ( TeGTParams ) - Geometric transformation parameters. 
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
   *
   * @note The output_raster parameters will take the reference
   * raster parameters.   
   * @note The output raster data type will be automatically to fit the
   * range of both input rasters.
   * @note Transformation tie-points: TeCoordPair::pt1 are raster 1 indexed 
   * (line, column) coordinates and TeCoordPair::pt2 are raster 2 indexed 
   * (line, column) coordinates.   
   * @example TePDIMosaic_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDITPMosaic : public TePDIAlgorithm {
    public :
      

      TePDITPMosaic();

      ~TePDITPMosaic();
      
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
     * @param interPolR1Ref Intersection polygon - Raster 1 projected reference 
     * coords.
     * @param raster1 Raster 2 pointer.
     * @param channels1 Raster 2 channels.
     * @param interPolR2Ref Intersection polygon - Raster 2 projected reference 
     * coords.
     * @param dummyValue Dummy value.
     * @param pixelOffsets2 The generated offsets values for raster 1.
     * @param pixelScales2 The generated scales values for raster 1.
     * @param pixelOffsets2 The generated offsets values for raster 2.
     * @param pixelScales2 The generated scales values for raster 2.
     * @return true if OK, false on errors.
     */        
      bool calcEqParams( const TePDITypes::TePDIRasterPtrType& raster1, 
        const std::vector< unsigned int >& channels1,
        TePolygon& interPolR1Ref,
        const TePDITypes::TePDIRasterPtrType& raster2,
        const std::vector< unsigned int >& channels2,
        TePolygon& interPolR2Ref,
        const double& dummyValue,
        std::vector< double >& pixelOffsets1, 
        std::vector< double >& pixelScales1,
        std::vector< double >& pixelOffsets2, 
        std::vector< double >& pixelScales2 ) const;
        
    /**
     * @brief Calc mean and variances for each raster channel.
     * @param raster Raster pointer.
     * @param channels Raster channels.
     * @param interPol The restriction polygon over input raster (projected
     * coords).
     * @param dummyValue Dummy value.
     * @param means The generated mean values.
     * @param means The generated variance values.
     * @return true if OK, false on errors.
     */           
      bool calcStats( const TePDITypes::TePDIRasterPtrType& raster,
        const std::vector< unsigned int >& channels,
        TePolygon& interPol,
        const double& dummyValue,
        std::vector< double >& means,
        std::vector< double >& variances ) const;
  };

#endif
