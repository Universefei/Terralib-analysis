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

#ifndef TEPDICONTRAST_HPP
  #define TEPDICONTRAST_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIRgbPalette.hpp"
  #include "TePDITypes.hpp"
  #include "../kernel/TeSharedPtr.h"
  #include "TePDIHistogram.hpp"

  /**
   * @brief  This is the class for contrast image processing algorithms.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIEnhancementGroup
   *
   * @note The general input parameters are:
   *
   * @param contrast_type ( TePDIContrast::TePDIContrastType ) - The contrast 
   * to be generated.
   * @param input_image (TePDITypes::TePDIRasterPtrType) - The input
   * image.
   * @param input_band ( int ) - Band to process from input_image.
   *
   * @note The specific input parameters ( by parameter type ):
   *
   * @param min_level (double) - Minimal level for Linear Luts.
   * Required by: TePDIContrastLinear, TePDIContrastSquareRoot,
   * TePDIContrastSquare, TePDIContrastLog, TePDIContrastNegative,
   * TePDIContrastSimpleSlicer.
   *
   * @param max_level (double) - Minimal level for Linear Luts.
   * Required by: TePDIContrastLinear, TePDIContrastSquareRoot,
   * TePDIContrastSquare, TePDIContrastLog, TePDIContrastNegative,
   * TePDIContrastSimpleSlicer.
   *
   * @param rgb_palette (TePDIRgbPalette::pointer) -
   * Reference RGB palette.
   * Required by: TePDIContrastSimpleSlicer.
   *
   * @param target_mean ( double ) - The target mean value.
   * Required by: TePDIContrastStat.
   *
   * @param target_variance ( double ) - The target variance value.
   * Required by: TePDIContrastStat.
   *
   * @note The following parameters are optional and will be used if present.
   *
   * @param output_image ( TePDITypes::TePDIRasterPtrType ) -
   * The output image ( if not present, only the output LUT will be
   * generated ).   
   *
   * @param output_band ( int ) - Band to save the result inside output_image
   * (required if output_image is present).
   *
   * @param restrict_out_reset (int) - If present (any value),
   * a restrictive output_image reset will be done
   * ( output_image reset will not be done if all of the following 
   * conditions are satisfied - Exact number of lines, exact number
   * of columns, sufficient number of bands, same projection, same box,
   * dummy value ), 
   * this allows output_image data keeping since
   * unused bands are not changed.
   *
   * @param outlut ( TePDITypes::TePDILutPtrType ) - The generated look-up
   * table. NOTE: If the current input_image has floating point pixel values,
   * the generated look-up table will have interpolated values.
   *
   * @param input_histogram ( TePDIHistogram::pointer ) - A pointer to an
   * active histogram instance that will be used when generating the
   * internal lut - The internal histogram generation will be disabled.
   *
   * @param output_original_histogram ( TePDIHistogram::pointer ) - A 
   * pointer to an active output histogram instance that will be filled 
   * with the internally generated histogram from the input image or
   * using the supplied input histogram.
   *
   * @param output_enhanced_histogram ( TePDIHistogram::pointer ) - A 
   * pointer to an active output histogram instance that will be filled 
   * with the internally generated histogram from the input image or
   * using the supplied input histogram.   
   *
   * @param histo_levels (int) - Number of histogram levels when internal
   * histogram generation is needed ( if zero, the histogram levels
   * will be automatically found. In this case, for floating point rasters the
   * number of levels will be always 256 ).
   *
   * @param dummy_value (double) - A dummy pixel value for use 
   * in pixels where no data is available.    
   *
   * @param input_mean ( double ) - Force the algorithm to use the supplied
   * value as mean for the current raster (The internal mean calcule will
   * be disabled ).
   * Used by: TePDIContrastStat.
   *
   * @param input_variance ( double ) - Force the algorithm to use the supplied
   * value as variance for the current raster ( The internal variance calcule
   * will be disabled ).
   * Used by: TePDIContrastStat.      
   *
   * @param output_channel_min_level ( double ) - Force the miminum output
   * gray level to be the supplied value (required if 
   * output_image is not present).
   *
   * @param output_channel_max_level ( double ) - Force the maximum output
   * gray level to be the supplied value (required if 
   * output_image is not present).
   */
  class PDI_DLL TePDIContrast : public TePDIAlgorithm {
    public :

      /**
       * @enum Allowed contrast types.
       */
      enum TePDIContrastType{
        /** The histogram range will be changed to the supplied 
            min/max range ( linear function ). */
        TePDIContrastMinMax = 1,
        /** The histogram will be remapped using a linear function. */
        TePDIContrastLinear = 2,
        /** The histogram will be remapped using a square root function. */
        TePDIContrastSquareRoot = 3,
        /** The histogram will be remapped using a square function. */
        TePDIContrastSquare = 4,
        /** The histogram will be remapped using a logarithmic function. */
        TePDIContrastLog = 5,
        /** The histogram will be remapped using a linear inverse function. */
        TePDIContrastNegative = 6,
        /** The uniform histogram aproximation. */
        TePDIContrastHistEqualizer = 7,
        /** The histogram values will be splitted into slices and linked with
            a RGB color table. */
        TePDIContrastSimpleSlicer = 8,
        /** Image mean and variance uniformization. */
        TePDIContrastStat = 9
      };

      /**
       * @brief Default Constructor.
       *
       */
      TePDIContrast();

      /**
       * @brief Default Destructor
       */
      ~TePDIContrast();

      /**
       * @brief Checks if the supplied parameters fits the requirements of each
       * PDI algorithm implementation.
       *
       * @note Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      bool CheckParameters( const TePDIParameters& parameters ) const;

    protected :
    
      /**
       * @brief A pointer to the current generated histogram.
       */    
      TePDIHistogram::pointer histo_ptr_;

      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );

      /**
       * @brief Build a min-max look-up table.
       *
       * @param channel_min_level The minimum channel possible level value.
       * @param channel_max_level The maximum channel possible level value.
       * @param base_lut Base lut.
       * @return  The look-up table.
       */
      TePDITypes::TePDILutType GetMinMaxLut(
        double channel_min_level, double channel_max_level,
        TePDITypes::TePDILutType& base_lut );

      /**
       * @brief Build a Linear look-up table.
       *
       * @param min Minimal level value.
       * @param max Maximum level value.
       * @param channel_min_level The minimum channel possible level value.
       * @param channel_max_level The maximum channel possible level value.
       * @param base_lut Base lut.
       * @return The look-up table.
       */
      TePDITypes::TePDILutType GetLinearLut(
        TePDITypes::TePDILutType& base_lut,
        double min, double max,
        double channel_min_level, double channel_max_level );

      /**
       * @brief Build a Square Root look-up table.
       *
       * @param min Minimal level value.
       * @param max Maximum level value.
       * @param channel_min_level The minimum channel possible level value.
       * @param channel_max_level The maximum channel possible level value.
       * @param base_lut Base lut.
       * @return The look-up table.
       */
      TePDITypes::TePDILutType GetSquareRootLut(
        TePDITypes::TePDILutType& base_lut,
        double min, double max,
        double channel_min_level, double channel_max_level );

      /**
       * @brief Build a Square look-up table.
       *
       * @param min Minimal level value.
       * @param max Maximum level value.
       * @param channel_min_level The minimum channel possible level value.
       * @param channel_max_level The maximum channel possible level value.
       * @param base_lut Base lut.
       * @return The look-up table.
       */
      TePDITypes::TePDILutType GetSquareLut(
        TePDITypes::TePDILutType& base_lut,
        double min, double max,
        double channel_min_level, double channel_max_level );

      /**
       * @brief Build a Log look-up table.
       *
       * @param min Minimal level value.
       * @param max Maximum level value.
       * @param channel_min_level The minimum channel possible level value.
       * @param channel_max_level The maximum channel possible level value.
       * @param base_lut Base lut.
       * @return The look-up table.
       */
      TePDITypes::TePDILutType GetLogLut(
        TePDITypes::TePDILutType& base_lut,
        double min, double max,
        double channel_min_level, double channel_max_level );

      /**
       * @brief Build a Negative look-up table.
       *
       * @param min Minimal level value.
       * @param max Maximum level value.
       * @param channel_min_level The minimum channel possible level value.
       * @param channel_max_level The maximum channel possible level value.
       * @param base_lut Base lut.
       * @return The look-up table.
       */
      TePDITypes::TePDILutType GetNegativeLut(
        TePDITypes::TePDILutType& base_lut,
        double min, double max,
        double channel_min_level, double channel_max_level );

      /**
       * @brief Build a Histogram equalizer look-up table.
       *
       * @param channel_min_level The minimum channel possible level value.
       * @param channel_max_level The maximum channel possible level value.       
       * @param hist A LEVEL-ORDERED Base histogram.
       * @return The look-up table.
       */
      TePDITypes::TePDILutType GetHistEqualizerLut(
        TePDIHistogram::pointer hist,
        double channel_min_level, double channel_max_level );

      /**
       * @brief Builds a Simple slicer look-up table.
       *
       * @param hist A LEVEL-ORDERED Base histogram.
       * @param in_palette The reference RGB palette.
       * @param min Minimal level value.
       * @param max Maximum level value.
       * @param out_lut The look-up table.
       */
      void GetSimpleSlicerLut(
        TePDIHistogram::pointer hist,
        TePDIRgbPalette::pointer in_palette,
        double min,
        double max,
        TePDITypes::TePDILutType& out_lut );
        
      /**
       * @brief Builds a mean and variance normalization lut.
       *
       * @param hist A LEVEL-ORDERED Base histogram.
       * @param target_mean The target mean value.
       * @param target_variance The target variance value.
       * @param channel_min_level The minimum channel possible level value.
       * @param channel_max_level The maximum channel possible level value.       
       * @param out_lut The look-up table.
       */
      void GetStatLut( TePDIHistogram::pointer hist, double target_mean,
        double target_variance, double channel_min_level, 
        double channel_max_level, TePDITypes::TePDILutType& out_lut );        
        
      /**
       * @brief Do level remapping from one raster to another.
       *
       * @param inRaster The input Raster.
       * @param outRaster The Output Raster.
       * @param lut The increasing level-ordered look-up table.
       * @param in_channel in_raster channel to remap.
       * @param out_channel out_raster channel to write on.
       * @param use_dummy Force a new dummy value use ( input pixels with
       * dummy values will be ignored ).
       * @param dummy_value A dummy pixel value for use in pixels where no data 
       * is avaliable.
       * @param fixed_step_lut A flag indication of a fixed step lut ( the
       * internal optimized fixed step algorithm will be used )
       * @return true if ok, false on errors.
       */
      bool RemapLevels(
        TePDITypes::TePDIRasterPtrType& inRaster,
        TePDITypes::TePDILutType& lut,
        int in_channel,
        int out_channel,
        TePDITypes::TePDIRasterPtrType& outRaster,
        bool use_dummy, double dummy_value,
        bool fixed_step_lut );
        
      /**
       * @brief Do level remapping from one raster to another.
       *
       * @note For luts having full channel range (contains mappings
       * for all possible ordered gray scale values, starting from zero ).
       *
       * @param inRaster The input Raster.
       * @param outRaster The Output Raster.
       * @param lut The increasing level-ordered look-up table.
       * @param in_channel in_raster channel to remap.
       * @param out_channel out_raster channel to write on.
       * @param use_dummy Force a new dummy value use ( input pixels with
       * dummy values will be ignored ).
       * @param dummy_value A dummy pixel value for use in pixels where no data 
       * is available.
       * @return true if ok, false on errors.
       */
      bool FullRangeLutRemapLevels(
        TePDITypes::TePDIRasterPtrType& inRaster,
        TePDITypes::TePDILutType& lut,
        int in_channel,
        int out_channel,
        TePDITypes::TePDIRasterPtrType& outRaster,
        bool use_dummy, double dummy_value );        
        
      /**
       * @brief Returns a valid histogram using the given algorithm input 
       * parameters.
       *
       * @param hist The generated Histogram.
       * @param useDummy A flag indicating when a dummy value must be used.
       * @param dummyValue The dummy value that must be used.
       * @return true if ok, false on errors.
       */        
      bool getHistogram( TePDIHistogram::pointer& hist, bool useDummy,
        double dummyValue );
      
      /**
       * @brief Returns valid dummy base lut from the given algorithm input 
       * parameters.
       * @note The dummy lut contains dummy mappings for all grey levels
       * following the algorithm input parameters.
       *
       * @param lut The generated dummy lut.
       * @param hist_based_lut This parameter will be false if the generated
       * lut has all possible gray levels ( including the unused ones ), 
       * true value means that the lut was build using the image histogram 
       * wich may contain only the used gray values or the interpolated values.
       * @param useDummy A flag indicating when a dummy value must be used.
       * @param dummyValue The dummy value that must be used.       
       * @return true if ok, false on errors.
       */        
      bool getBaseLut( TePDITypes::TePDILutType& lut, bool& hist_based_lut,
        bool useDummy, double dummyValue );      
  };
  
/** @example TePDIContrast_test.cpp
 *    Shows how to use this class.
 */    

#endif //TEPDICONTRAST_HPP
