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

/**
 * @file TePDIUtils.hpp
 * @brief This file contains utilitary functions used by PDI algorithms. 
 * THEY SHOLD NOT BE USED BY ANYONE. The support and interfaces 
 * can be changed in future. THIS IS FOR INTERNAL USE ONLY.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 */

#ifndef TEPDIUTILS_HPP
  #define TEPDIUTILS_HPP

  #include "TePDIRgbPalette.hpp"
  #include "TePDITypes.hpp"
  #include "TePDIMathFunctions.hpp"
  #include "TePDIInterpolator.hpp"
  
  #include "TePDITileIndexer.hpp"

  #include "../kernel/TeAgnostic.h"
  #include "../kernel/TeRaster.h"
  #include "../kernel/TeDataTypes.h"
  #include "../kernel/TeBox.h"

  #include <string>
  #include <map>

 
    /**
     * @namespace TePDIUtils This namespace contains base utilitary functions used by PDI classes.
     * @note THEY SHOLD NOT BE USED BY ANYONE. The support and interfaces 
     * can be changed in future. THIS IS FOR INTERNAL USE ONLY.     
     * @ingroup PDIModule
     */
  namespace TePDIUtils
  {
    /**
     * @enum PDIUtils memory policies.
     */
    enum TePDIUtilsMemPol {
      /** The memory policy will be choosed following the free RAM */
      TePDIUtilsAutoMemPol = 1,
      /** Use RAM memory */
      TePDIUtilsRamMemPol = 2,
      /** Use memory mapped disk files */
      TePDIUtilsDiskMemPol = 3
    };  

    /**
     * @brief Allocates one RAM raster using another raster as template base.
     * @deprecated DEPRECATED - Use another implementation of this method 
     * because it will be removed in the future.
     * @note If there are not enought memory, memory mapping to disk files 
     * will be used.
     *
     * @param template_raster Input raster.
     * @param RAMRaster Output raster.
     * @param bands Number of bands for the generated RAMRaster. If zero, the
     * number of bands will match the input raster.
     * @param force_new_dt If true, a new pixel data type will be used for 
     * all bands.
     * @param force_mm Force disk memory mapped raster creation.
     * @param pixel_type Pixel data type.
     * @return true if OK. false on error.
     */
    PDI_DLL bool TeAllocRAMRaster(
      TePDITypes::TePDIRasterPtrType& template_raster,
      TePDITypes::TePDIRasterPtrType& RAMRaster,
      unsigned int bands, bool force_new_dt, bool force_mm,
      TeDataType pixel_type );

    /**
     * @brief Allocates a brand new RAM raster.
     *
     * @note If there are not enought memory, memory mapping to disk files 
     * will be used.
     * @deprecated DEPRECATED - Use another implementation of this method 
     * because it will be removed in the future.
     * @param RAMRaster Output raster.
     * @param bands Number of bands for the generated RAMRaster.
     * @param lines Number of lines.
     * @param columns Number of Columns
     * @param force_mm Force disk memory mapped raster creation.
     * @param pixel_type Pixel data type.
     * @param palette A palette reference, for generation of a paletted
     * raster ( must be 0 if not used ).
     * @return true if OK. false on error.
     */
    PDI_DLL bool TeAllocRAMRaster(
      TePDITypes::TePDIRasterPtrType& RAMRaster,
      unsigned int bands, unsigned int lines, unsigned int columns,
      bool force_mm, TeDataType pixel_type, TePDIRgbPalette* palette );
      
    /**
     * @brief Allocates a brand new RAM raster using the supplied parameters.
     * @deprecated DEPRECATED - Use another implementation of this method 
     * because it will be removed in the future.
     * @note If there are not enought memory, memory mapping to disk files 
     * will be used.
     *
     * @param RAMRaster Output raster.
     * @param params Raster parameters.
     * @param force_mm Force disk memory mapped raster creation.
     * @return true if OK. false on error.
     */      
    PDI_DLL bool TeAllocRAMRaster( TePDITypes::TePDIRasterPtrType& RAMRaster,
      const TeRasterParams& params, bool force_mm );
      
    /**
     * @brief Allocates a brand new RAM raster using the supplied parameters.
     * @deprecated DEPRECATED - Use another implementation of this method 
     * because it will be removed in the future.
     * @param RAMRaster Output raster.
     * @param params Raster parameters.
     * @param mempol Memory policy.
     */      
    PDI_DLL bool TeAllocRAMRaster( TePDITypes::TePDIRasterPtrType& RAMRaster,
      const TeRasterParams& params, 
      TePDIUtilsMemPol mempol ); 
       
    /**
     * @brief Create a new raster instance into memory using the supplied 
     * parameters.
     *
     * @param rasterPointer Output raster pointer.
     * @param params Raster parameters.
     */      
    PDI_DLL bool TeAllocRAMRaster( 
      const TeRasterParams& params,
      TePDITypes::TePDIRasterPtrType& rasterPointer );        
      
    /**
     * @brief Save the raster to a GeoTIFF file.
     *
     * @param in_raster Input raster.
     * @param file_name Output file name.
     * @return true if OK. false on error.
     * @note DEPRECATED: Use other TeRaster2Geotiff implementation.
     */
    PDI_DLL bool TeRaster2Geotiff(
      const TePDITypes::TePDIRasterPtrType& in_raster,
      const std::string& file_name );      

    /**
     * @brief Save the raster to a GeoTIFF file.
     *
     * @param in_raster Input raster.
     * @param file_name Output file name.
     * @param pixel_type The type of pixels.
     * @return true if OK. false on error.
     * @note DEPRECATED: Use other TeRaster2Geotiff implementation.
     */
    PDI_DLL bool TeRaster2Geotiff(
      const TePDITypes::TePDIRasterPtrType& in_raster,
      const std::string& file_name,
      TeDataType pixel_type );
      
    /**
     * @brief Save the raster to a GeoTIFF file.
     *
     * @param in_raster Input raster.
     * @param file_name Output file name.
     * @param enable_progress Enable/disable progress interface.
     * @return true if OK. false on error.
     */
    PDI_DLL bool TeRaster2Geotiff(
      const TePDITypes::TePDIRasterPtrType& in_raster,
      const std::string& file_name,
      bool enable_progress );
      
    /**
     * @brief Save a 3 bands/channels raster to a RGB GeoTIFF file.
     *
     * @param in_raster 3 band/channels input raster.
     * @param file_name Output file name.
     * @param enable_progress Enable/disable progress interface.
     * @return true if OK. false on error.
     */
    PDI_DLL bool TeRaster2RGBGeotiff(
      const TePDITypes::TePDIRasterPtrType& in_raster,
      const std::string& file_name,
      bool enable_progress );      
            
    /**
     * @brief Save the raster to a Jpeg file.
     *
     * @param in_raster Input raster.
     * @param file_name Output file name.
     * @param enable_progress Enable/disable progress interface.
     * @param qualityFactor Quality factor from 0% to 100%.
     * @return true if OK. false on error.
     */
    PDI_DLL bool TeRaster2Jpeg(
      const TePDITypes::TePDIRasterPtrType& in_raster,
      const std::string& file_name,
      bool enable_progress, unsigned int qualityFactor );                

    /**
     * @brief Do pixel by pixel copy from one raster to another.
     *
     * @param source_raster Input raster.
     * @param source_band Souce band.
     * @param target_raster Output raster.
     * @param target_band Target band.
     * @param discretize_levels Levels will be rounded to the near integer.
     * @param trunc_levels Levels outside a range ( from zero
     * to the maximum Output raster allowed pixel value ) will be rounded to 
     * zero or the maximum Output raster allowed pixel value.
     * @return true if OK. false on error.
     */
    PDI_DLL bool TeCopyRasterPixels(
      TePDITypes::TePDIRasterPtrType& source_raster,
      unsigned int source_band,
      TePDITypes::TePDIRasterPtrType& target_raster,
      unsigned int target_band,
      bool discretize_levels,
      bool trunc_levels );

    /**
     * @brief Do pixel by pixel copy from one raster to another ( all bands ).
     *
     * @param source_raster Input raster.
     * @param target_raster Output raster.
     * @param discretize_levels Levels will be rounded to the near integer.
     * @param trunc_levels Levels outside a range ( from zero
     * to the maximum Output raster allowed pixel value ) will be rounded to 
     * zero or the maximum Output raster allowed pixel value.
     * @return true if OK. false on error.
     */
    PDI_DLL bool TeCopyRasterBands(
      TePDITypes::TePDIRasterPtrType& source_raster,
      TePDITypes::TePDIRasterPtrType& target_raster,
      bool discretize_levels,
      bool trunc_levels );

    /**
     * @brief Used to find the allowed maximum and minimum level values
     * that the supplied TeRaster band can have.
     *
     * @param raster Input raster.
     * @param band Band number.
     * @param min Minimum allowed value.
     * @param max Maximum allowed value..
     * @return true if OK, false on error.
     */
    PDI_DLL bool TeGetRasterMinMaxBounds( 
      const TePDITypes::TePDIRasterPtrType& raster,
      unsigned int band, double& min, double& max );
      
    /**
     * @brief Used to find the raster required memory.
     *
     * @param params Input raster parameters.
     * @return The amount of required memory.
     */
    PDI_DLL unsigned long int TeGetRasterReqMem( TeRasterParams& params );      

    /**
     * @brief Compares two rasters.
     *
     * @param raster1 Input raster 1.
     * @param raster2 Input raster 2.
     * @return true if all the pixels from both rasters have the same levels,
     * false if not.
     */
    PDI_DLL bool rasterCompare( TePDITypes::TePDIRasterPtrType& raster1,
      TePDITypes::TePDIRasterPtrType& raster2 );
      
    /**
     * @brief Given a raster band, verifies if the pixels are floating point
     * pixels.
     *
     * @param raster The input raster.
     * @param band Band number.
     * @return true if the the pixels are floating point pixels.
     */      
    PDI_DLL bool IsFloatBand( const TePDITypes::TePDIRasterPtrType& raster, 
      unsigned int band );
    
    /**
     * @brief Convert a TeCoord2D with projected coordinates to a geometry with
     * indexed coordinates ( following the raster reference of lines and
     * columns ).
     *
     * @param g_in Input geometry.
     * @param raster Reference raster.
     * @param g_out Output geometry.
     */       
    PDI_DLL void MapCoords2RasterIndexes( const TeCoord2D& g_in, 
      const TePDITypes::TePDIRasterPtrType& raster, TeCoord2D& g_out );
      
    /**
     * @brief Convert a TeBox with projected coordinates to a TeBox with
     * indexed coordinates ( following the raster reference of lines and
     * columns ).
     *
     * @param box_in Input box.
     * @param raster Reference raster.
     * @param box_out Output box.
     */       
    PDI_DLL void MapCoords2RasterIndexes( const TeBox& box_in, 
      const TePDITypes::TePDIRasterPtrType& raster, TeBox& box_out );      
      
   /**
     * @brief Convert a geometry with projected coordinates to a geometry with
     * indexed coordinates ( following the raster reference of lines and
     * columns ).
     *
     * @param g_in Input geometry.
     * @param raster Reference raster.
     * @param g_out Output geometry.
     */        
    template< typename Geometry >
    void MapCoords2RasterIndexes( const Geometry& g_in, 
      const TePDITypes::TePDIRasterPtrType& raster, Geometry& g_out )
    {
        unsigned int nItens = (unsigned int)g_in.size();
        
        for (unsigned int i=0; i < nItens ; i++ ) {
                typename Geometry::value_type e1 = g_in[i];
                typename Geometry::value_type e2;
                
                MapCoords2RasterIndexes( e1, raster, e2 );
                
                g_out.add( e2 );
        }
    }
    
    /**
     * @brief Convert TeCoord2D with matrix indexed coordinates to a TeCoord2D with 
     * projected coordinates folowing a raster reference.
     *
     * @param g_in Input geometry.
     * @param raster Reference raster.
     * @param g_out Output geometry.
     */       
    PDI_DLL void MapRasterIndexes2Coords( const TeCoord2D& g_in, 
      const TePDITypes::TePDIRasterPtrType& raster, TeCoord2D& g_out );     
      
    /**
     * @brief Convert TeBox with matrix indexed coordinates to a TeBox with 
     * projected coordinates folowing a raster reference.
     *
     * @param g_in Input geometry.
     * @param raster Reference raster.
     * @param g_out Output geometry.
     */       
    PDI_DLL void MapRasterIndexes2Coords( const TeBox& g_in, 
      const TePDITypes::TePDIRasterPtrType& raster, TeBox& g_out );       
    
   /**
     * @brief Convert geometry with matrix indexed coordinates to a geometry with 
     * projected coordinates folowing a raster reference.
     *
     * @param g_in Input geometry.
     * @param raster Reference raster.
     * @param g_out Output geometry.
     */        
    template< typename Geometry >
    void MapRasterIndexes2Coords( const Geometry& g_in, 
      const TePDITypes::TePDIRasterPtrType& raster, Geometry& g_out )
    {
        unsigned int nItens = (unsigned int)g_in.size();
        
        for (unsigned int i=0; i < nItens ; i++ ) {
                typename Geometry::value_type e1 = g_in[i];
                typename Geometry::value_type e2;
                
                MapRasterIndexes2Coords( e1, raster, e2 );
                
                g_out.add( e2 );
        }
    }
    
   /**
     * @brief Breaks a TeLinearRing geometry into an segment set ( line segments of 
     * two points ).
     *
     * @param g_in Input geometry.
     * @param g_out Output geometry.
     */        
    PDI_DLL void makeSegmentSet( const TeLinearRing& g_in, TeLineSet& g_out );
    
   /**
     * @brief Breaks a geometry into an segment set ( line segments of two points ).
     *
     * @param g_in Input geometry.
     * @param g_out Output geometry.
     */        
    template< typename Geometry >
    void makeSegmentSet( const Geometry& g_in, TeLineSet& g_out )
    {
      g_out.clear();
        
      for ( unsigned int i = 0 ; i < g_in.size() ; i++ ) {
        TeLineSet temp_ls;
        
        makeSegmentSet( g_in[i], temp_ls );
          
        const unsigned int temp_ls_size = temp_ls.size();
                
        for( unsigned int temp_ls_index = 0 ; temp_ls_index < temp_ls_size ; 
          ++temp_ls_index ) {
            
          g_out.add( temp_ls[ temp_ls_index ] );
        }
       }
    };
        
    /**
     * @brief Virifies if the lut has only discrete reference values ( integer ).
     *
     * @param lut Lut.
     * @return true if only discrete values were found, otherwise false
     * will be returned.
     */
    PDI_DLL bool IsDiscreteLut( const TePDITypes::TePDILutType& lut );
    
    /**
     * @brief Builds a detailed box (world indexes) using all raster border pixels.
     *
     * @param raster Input raster.
     * @param box_polygon The generated box (polygon representation).
     * @return true if OK, false on errors.
     */
    PDI_DLL bool buildDetailedBox( TeRaster& raster,
      TePolygon& box_polygon );
    
    /**
     * @deprecated This method will be removed in the future, please
     * use the other buildDetailedBox implementation.
     * @brief DEPRECATED - Builds a detailed box (world indexes) using all raster border pixels.
     * @param raster Input raster.
     * @param box_polygon The generated box (polygon representation).
     * @return true if OK, false on errors.
     */
    PDI_DLL bool buildDetailedBox( const TePDITypes::TePDIRasterPtrType& raster,
      TePolygon& box_polygon );
      
    /**
     * @brief Builds a detailed bounding box (world indexes) using all raster 
     * border pixels.
     *
     * @param raster Input raster.
     * @param box_polygon The generated box (polygon representation).
     * @return true if OK, false on errors.
     */
    PDI_DLL bool buildDetailedBBox( TeRaster& raster,
      TePolygon& box_polygon );      
      
    /**
     * @deprecated This method will be removed in the future, please
     * use the other buildDetailedBox implementation.    
     * @brief DEPRECATED - Builds a detailed bounding box (world indexes) using all raster 
     * border pixels.
     * @param raster Input raster.
     * @param box_polygon The generated box (polygon representation).
     * @return true if OK, false on errors.
     */
    PDI_DLL bool buildDetailedBBox( const TePDITypes::TePDIRasterPtrType& raster,
      TePolygon& box_polygon );
      
    /**
     * @brief Reproject a raster geometry.
     *
     * @param input_raster Input raster.
     * @param target_projection Target projection.
     * @param outNLines Output raster lines number.
     * @param outNCols Output raster columns number.
     * @param enableProgress Enable the progress interface.
     * @param enableOptimization Enable optimization (good for big rasters).
     * @param output_raster Output raster.
     *
     * @return true if OK, false on errors.
     */
    PDI_DLL bool reprojectRaster( TeRaster& input_raster,
      unsigned int outNLines, unsigned int outNCols, 
      TeProjection& target_projection,
      bool enableProgress, bool enableOptimization,
      TeRaster& output_raster );      
      
    /**
     * @brief Reproject a raster geometry.
     *
     * @param input_raster Input raster.
     * @param target_projection Target projection.
     * @param target_resx Target X resolution following the target projection.
     * @param target_resy Target Y resolution following the target projection.
     * @param enableProgress Enable the progress interface.
     * @param enableOptimization Enable optimization (good for big rasters).
     * @param output_raster Output raster.
     *
     * @return true if OK, false on errors.
     */
    PDI_DLL bool reprojectRaster( TeRaster& input_raster,
      TeProjection& target_projection, double target_resx, 
      double target_resy,
      bool enableProgress, bool enableOptimization,
      TeRaster& output_raster );
      
    /**
     * @brief Tells the relation between a point and a polygon by using a 
     * polygon tile indexer.
     *
     * @note The boundary case is not checked.
     *
     * @param c The input point.
     * @param indexer The polygon tile indexer.
     *
     * @return TeINSIDE or TeOUTSIDE.
     */      
    PDI_DLL short TeRelation( const TeCoord2D& c, const TePDITileIndexer& indexer );
    
    /**
     * @brief Calculates the intersection set operation between two line segments
     * sets.
     *
     * @note Each line must have just 2 points.
     *
     * @param ls1 The input line set 1.
     * @param ls2 The input line set 2.
     * @param ps The output point set.
     */     
    PDI_DLL void TeSegSetIntersection( const TeLineSet& ls1, const TeLineSet& ls2,
      TePointSet& ps );
      
    /**
     * @brief Chooses the best pixel data type betweenn rasters.
     * 
     * @param rasters Input rasters.
     * @return The best data type
     */        
    PDI_DLL TeDataType chooseBestPixelType( 
      TePDITypes::TePDIRasterVectorType& rasters );      
      
    /**
     * @brief Returns the minimum and maximum pixel values inside one raster channel.
     * 
     * @param raster Input raster.
     * @param channel Input channel.
     * @param min The minimum level.
     * @param max The maximum level.
     * @param progress_enabled Enable/disable the progress interface.
     * @param sample_step The step size between lines/columns ( to
     * use all lines/columns use sample_step = 0 ).
     */        
    PDI_DLL void getBandMinMaxValues( 
      const TePDITypes::TePDIRasterPtrType& raster, unsigned int channel,
      bool progress_enabled, unsigned int sample_step,  
      double& min, double& max );      
      
    /**
     * @brief Generates a multi-band copy from the given raster.
     * 
     * @param in_raster Input raster (must have a projection).
     * @param progress_enabled Progress interface enable/disable flag.
     * @param out_raster Output raster.
     * @return true if ok, false on errors.
     * @note If out_raster isn't active, a new raster ( SMARMEM decoder )
     * will be returned.
     * @note If no conversion was made, the returned pointer will point
     * to the input raster.
     */        
    PDI_DLL bool convert2MultiBand( const TePDITypes::TePDIRasterPtrType& in_raster,
      bool progress_enabled, TePDITypes::TePDIRasterPtrType& out_raster );
      
    /**
     * @brief Compares two TeLinerRing ignoring the points order.
     * @param geom1 The first geometry.
     * @param geom2 The second geometry.
     * @return true if both geometries has the same points, false otherwise.
     */        
    PDI_DLL bool compareGeometryPoints( const TeLinearRing& geom1, 
      const TeLinearRing& geom2 );      
      
    /**
     * @brief Compares two geometries ignoring the points order inside each
     * composite elemeent.
     * @param geom1 The first polygon.
     * @param geom2 The second polygon.
     * @return true if both polygons has the same points on each ring.
     */        
    template< typename Geometry >
    bool compareGeometryPoints( const Geometry& geom1, const Geometry& geom2 )
    {
      if( geom1.size() == geom2.size() ) {
        const unsigned geom_size = geom1.size();

        for( unsigned int elem_index = 0 ; elem_index < geom_size ; 
          ++elem_index ) {
            
          if( ! compareGeometryPoints( geom1[ elem_index ], 
            geom2[ elem_index ] ) ) {
            
            return false;
          }
        }
        
        return true;
      } else {
        return false;
      }
    };     

    /**
     * @brief Raster resampling to new resolution, 
     * keeping the original box and projection.
     * @param input_raster Input raster.
     * @param out_lines The number of output lines.
     * @param out_cols The number of output columns.
     * @param interpol Interpolation method.
     * @param enable_progress Enable/disable the progress interface.
     * @param output_raster Output raster.
     * @return true if OK, false on errors.
     * @ All bands will be resampled.
     * @note ( resolution_ratio = output_resolution / input_resolution ).
     */   
    PDI_DLL bool resampleRasterByLinsCols( 
      const TePDITypes::TePDIRasterPtrType& input_raster,
      const TePDITypes::TePDIRasterPtrType& output_raster,
      unsigned int out_lines, 
      unsigned int out_cols, 
      bool enable_progress,
      TePDIInterpolator::InterpMethod interpol );  
      
    /**
     * @brief Raster resampling to new resolution, 
     * keeping the original box and projection.
     * @param input_raster Input raster.
     * @param inputBands Input raster used bands.
     * @param out_lines The number of output lines.
     * @param out_cols The number of output columns.
     * @param interpol Interpolation method.
     * @param enable_progress Enable/disable the progress interface.
     * @param output_raster Output raster.
     * @return true if OK, false on errors.
     * 
     * @note ( resolution_ratio = output_resolution / input_resolution ).
     */      
    PDI_DLL bool resampleRasterByLinsCols( 
      const TePDITypes::TePDIRasterPtrType& input_raster,
      const std::vector< unsigned int >& inputBands,
      unsigned int out_lines, 
      unsigned int out_cols, 
      bool enable_progress,
      TePDIInterpolator::InterpMethod interpol,
      const TePDITypes::TePDIRasterPtrType& output_raster );        
    
    /**
     * @brief Raster resampling to new resolution, keeping the original box and projection.
     * @param input_raster Input raster.
     * @param output_raster Output raster.
     * @param x_resolution_ratio X resolution ratio.
     * @param y_resolution_ratio Y resolution ratio.
     * @param interpol Interpolation method.
     * @param enable_progress Enable/disable the progress interface.
     * @return true if OK, false on errors.
     * 
     * @note ( resolution_ratio = output_resolution / input_resolution ).
     */      
    PDI_DLL bool resampleRasterByRes( 
      const TePDITypes::TePDIRasterPtrType& input_raster,
      const TePDITypes::TePDIRasterPtrType& output_raster,
      double x_resolution_ratio, 
      double y_resolution_ratio, 
      TePDIInterpolator::InterpMethod interpol, 
      bool enable_progress );    
      
    /**
     * @brief Compose raster channels into one single raster.
     * @param input_rasters Input rasters vector.
     * @param input_channels Input rasters channels/bands vector.
     * @param output_raster Output raster.
     * @param enable_progress Enable/disable the progress interface.
     * @return true if OK, false on errors.
     * 
     * @note The reference (projection, resolution, box) for the output 
     * raster will be taken from the first
     * input channel (with index zero).
     * @note All input raster must have the same number of lines and columns.
     */      
    PDI_DLL bool composeRaster( 
      const TePDITypes::TePDIRasterVectorType& input_rasters,
      const std::vector< unsigned int >& input_channels,
      TePDITypes::TePDIRasterPtrType& output_raster,
      bool enable_progress );       
      
    /**
     * @brief Decompose raster channels into many single rasters with
     * one channel/band each.
     * @param inputRaster Input raster.
     * @param inputChannels Input rasters channels/bands vector.
     * @param outputRasters Output rasters vector.
     * @param enable_progress Enable/disable the progress interface.
     * @return true if OK, false on errors.
     * @note There is no need to pre-initialize the outputRasters vector.
     */      
    PDI_DLL bool decomposeRaster( 
      TeRaster& inputRaster,
      const std::vector< unsigned int >& inputChannels,
      TePDITypes::TePDIRasterVectorType& outputRasters,
      bool enableProgress );      
    
    /**
     * @brief Load a raster data file into RAM (using a SMARTMEM decoder)
     * @param fileName Raster file name
     * @param memRasterPtr The output memory raster pointer.
     * @param enable_progress Enable/disable the progress interface.
     * @return true if OK, false on errors.
     * 
     * @note The reference (projection, resolution, box) for the output 
     * raster will be taken from the first
     * input channel (with index zero).
     * @note All input raster must have the same number of lines and columns.
     */      
    PDI_DLL bool loadRaster( const std::string& fileName, 
      TePDITypes::TePDIRasterPtrType& memRasterPtr, bool enable_progress );        
      
  };
  
/** @example TePDIUtils_test.cpp
 *    Shows how to use this namespace functions.
 */    

#endif //TEPDIUTILS_HPP
