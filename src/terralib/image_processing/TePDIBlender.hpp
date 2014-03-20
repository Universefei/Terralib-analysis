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

#ifndef TEPDIBLENDER_HPP
  #define TEPDIBLENDER_HPP

  #include "TePDIInterpolator.hpp"
  #include "TePDIDefines.hpp"
  #include "TePDITypes.hpp"

  #include "../kernel/TeGeometry.h"
  #include "../kernel/TeRaster.h"
  #include "../kernel/TeGeometricTransformation.h"
  
  #include <vector>
  
  /**
   * @class TePDIBlender
   * @brief Blended pixel value calculation for two overlaped rasters.
   * @details The overlap between the two images is modeled by the given
   * geometric transformation direct mapping input raster 1 indexed
   * coords (line, column) to input raster 2 indexed coords.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIAux
   *
   * @example TePDIBlender_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIBlender 
  {
    public :
      /** @typedef TeSharedPtr< TePDIBlender > pointer 
          Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIBlender > pointer;
      
      /**
       * @enum Allowed blender methods.
       */      
      enum BlendMethod {
        /** No blending performed. */
        NoBlendMethod = 1,
        /** Mean of overlapped pixels method. */
        MeanBlendMethod = 2,        
        /** Euclidean distance based blending method. */
        EuclideanBlendMethod = 3
      };

      TePDIBlender();

      ~TePDIBlender();
      
      /**
       * @brief Initiate this instance.
       *
       * @param raster1Ptr Input raster 1 pointer.
       * @param raster1ChannelsVec Input raster 1 bands/channels.
       * @param raster2Ptr Input raster 2 pointer.
       * @param raster2ChannelsVec Input raster 2 bands/channels.
       * @param blendMethod Blending method.
       * @param interpMethod Interpolation method.  
       * @param transParams Transformation parameters. 
       * @param dummyValue The value to be returned for cases where there is no
       * raster data to blend.    
       * @param pixelOffsets1 The values offset to be applied to raster 1 pixel
       * values before the blended value calcule (one element for each
       * used raster channel/band).
       * @param pixelScales1 The values scale to be applied to raster 1 pixel
       * values before the blended value calcule (one element for each
       * used raster channel/band).   
       * @param pixelOffsets2 The values offset to be applied to raster 2 pixel
       * values before the blended value calcule (one element for each
       * used raster channel/band).
       * @param pixelScales2 The values scale to be applied to raster 2 pixel
       * values before the blended value calcule (one element for each
       * used raster channel/band).   
       * @param raster1Pol A polygon (raster 1 indexed coords - line, col)
       * delimiting the raster region with valid data (if an empty polygon
       * is given the entire raster will be considered).
       * @param raster2Pol A polygon (raster 2 indexed coords - line, col)
       * delimiting the raster region with valid data (if an empty polygon
       * is given the entire raster will be considered).       
       * @return true if OK, false on errors.
       * @note Transformation tie-points: TeCoordPair::pt1 are raster 1 indexed 
       * (line, column) coordinates and TeCoordPair::pt2 are raster 2 indexed 
       * (line, column) coordinates.
       * @note raster1ChannelsVec, raster2ChannelsVec, pixelOffsets1,
       * pixelScales1, pixelOffsets2 and pixelScales2 must have the same
       * number of elements.
       *
       */
      bool init( const TePDITypes::TePDIRasterPtrType& raster1Ptr,
        const std::vector< unsigned int > raster1ChannelsVec,
        const TePDITypes::TePDIRasterPtrType& raster2Ptr,
        const std::vector< unsigned int > raster2ChannelsVec,
        BlendMethod blendMethod,
        TePDIInterpolator::InterpMethod interpMethod,
        const TeGTParams& transParams,
        const double& dummyValue,
        const std::vector< double >& pixelOffsets1,
        const std::vector< double >& pixelScales1,
        const std::vector< double >& pixelOffsets2,
        const std::vector< double >& pixelScales2,
        const TePolygon& raster1Pol = TePolygon(), 
        const TePolygon& raster2Pol = TePolygon() );
        
      /**
       * @brief Blend a pixel value using the current parameters.
       *
       * @param line Line (raster 1 reference).
       * @param col Column (raster 1 reference).
       * @param rasterChannelsVecsIdx Vector index (the index
       * to search the correct band/channel for each input raster
       * from raster1ChannelsVec and raster2ChannelsVec).
       * @param value Blended value.
       * @note The caller of this method must be aware that the returned 
       * blended value may be outside the original input rasters valid values 
       * range.
       *
       */
      inline void blend( const double& line, const double& col, 
        const unsigned int& rasterChannelsVecsIdx , double& value )
      {
        return (this->*blendFunctPtr_)( line, col, rasterChannelsVecsIdx, 
          value );
      };

    protected :
    
      /**
       * @brief Type definition for the a bleding function pointer.
       *
       * @param line Line.
       * @param col Column.
       * @param band Band.
       * @param value Interpolated value.
       */      
      typedef void (TePDIBlender::*BlendFunctPtr)( const double& line, 
        const double& col, const unsigned int& band, double& value );    
        
      /** @brief Current dummy value (default:0). */      
      double dummyValue_;          
        
      /**
       * @brief The current blending method pointer (default:noBlendMethod).
       */      
      BlendFunctPtr blendFunctPtr_;        
       
      /** @brief Current blending method (default:NoBlendMethod). */      
      BlendMethod blendMethod_;  
           
      /** @brief The relation between the two rasters boinding box -
      raster 1 indexed reference (default:0) */
      short relation_;
      
      /** @brief Current geometrical transformation pointer. */      
      TeGeometricTransformation::pointer transPrt_;      
    
      /** @brief Input raster 1 pointer. */      
      TePDITypes::TePDIRasterPtrType raster1Ptr_;
      
      /** @brief Input raster 1 used channels vector. */      
      std::vector< unsigned int > raster1ChannelsVec_;
      
      /** @brief Input raster 2 pointer. */      
      TePDITypes::TePDIRasterPtrType raster2Ptr_;
      
      /** @brief Input raster 2 used channels vector. */      
      std::vector< unsigned int > raster2ChannelsVec_;
      
      /** @brief Input raster 1 internal interpolator instance. */      
      TePDIInterpolator interp1Instance_;      
      
      /** @brief Input raster 2 internal interpolator instance. */      
      TePDIInterpolator interp2Instance_;      
      
      // noBlendMethodImp temporary used variables
      TeCoord2D noBlendMethodImp_coord1_;
      TeCoord2D noBlendMethodImp_coord2_;
      
      // eucBlendMethodImp temporary used variables
      TeCoord2D eucBlendMethodImp_coord1_;
      TeCoord2D eucBlendMethodImp_coord2_;
      double eucBlendMethodImp_value1_; 
      double eucBlendMethodImp_value2_;
      double eucBlendMethodImp_weight1_;
      double eucBlendMethodImp_weight2_;
      double eucBlendMethodImp_minDistToR1_;
      double eucBlendMethodImp_minDistToR2_;
      TeCoord2D eucBlendMethodImp_curCoord_;
      TeCoord2D eucBlendMethodImp_dummyCoord_;
      double eucBlendMethodImp_currDist_;
      unsigned int eucBlendMethodImp_iLSIdx_;
      
      // meanBlendMethodImp temporary used variables
      TeCoord2D meanBlendMethodImp_coord1_;
      TeCoord2D meanBlendMethodImp_coord2_;
      double meanBlendMethodImp_value1_; 
      double meanBlendMethodImp_value2_;
      
      /** @brief Intersection polygon between the two images 
      (indexed - raster 1 indexed reference ) */
      TePolygon interPolR1IdxRef_;
      
      /** @brief Intersection polygon line sets that are inside the
       * raster 1 bounding box (raster 1 indexed reference) */    
      TeLineSet interLineSetInR1_;
      
      /** @brief Intersection polygon line sets that are inside the
       * raster 2 bounding box (raster 1 indexed reference) */    
      TeLineSet interLineSetInR2_;

      /** @brief The values offsets to be applied to raster 1 pixel
       * values before the blended value calcule (one element for each
       * used raster channel/band).
       */       
      std::vector< double > pixelOffsets1_;
      
      /** @brief pixelScale1 The values scale to be applied to raster 1 pixel
       * values before the blended value calcule (one element for each
       * used raster channel/band).
       */
      std::vector< double > pixelScales1_;
      
      /** @brief pixelOff2 The values offset to be applied to raster 2 pixel
       * values before the blended value calcule (one element for each
       * used raster channel/band).
       */
      std::vector< double > pixelOffsets2_;
      
      /** @param pixelScale2 The values scale to be applied to raster 2 pixel
       * values before the blended value calcule (one element for each
       * used raster channel/band).        
       */
      std::vector< double > pixelScales2_;
      
      /**
       * @brief Reset this instance cleaning all allocated resources.
       */      
      void reset();
      
      // just like the method blend above
      void noBlendMethodImp( const double& line, const double& col,
        const unsigned int& rasterChannelsVecsIdx, double& value ); 
        
      // just like the method blend above
      void eucBlendMethodImp( const double& line, const double& col,
        const unsigned int& rasterChannelsVecsIdx, double& value );
            
      // just like the method blend above
      void meanBlendMethodImp( const double& line, const double& col,
        const unsigned int& rasterChannelsVecsIdx, double& value );            
  };

#endif
