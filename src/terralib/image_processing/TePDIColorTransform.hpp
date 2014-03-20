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

#ifndef TEPDICOLORTRANSFORM_HPP
  #define TEPDICOLORTRANSFORM_HPP

  #include "TePDIAlgorithm.hpp"

  /**
   * @brief This is the class for raster color transforms.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIGeneralAlgoGroup
   *
   * @note The general required parameters are:
   *
   * @param input_rasters ( TePDITypes::TePDIRasterVectorType ) - 
   * A vector of all used input rasters ( NOTE: When two or more bands
   * from the same raster are used, this raster must be inserted 
   * two or three times inside the vector.
   * @param input_channels ( std::vector< int > ) - A vector of all used 
   * input rasters channels/bands.
   * @param output_rasters ( TePDITypes::TePDIRasterVectorType ) - 
   * A vector of all used output rasters ( Possible combinations:
   * One raster with 3 channels, 3 rasters with one channel per raster. ).
   * @param transf_type ( TePDIColorTransform::ColorTransfTypes ) - 
   * Transformation type.
   * @param rgb_channels_min (double) - The output RGB channels min level.
   * @param rgb_channels_max (double) - The output RGB channels max level.
   *
   * @note Channels disposition for RGB images:
   * input_rasters[ 0 ]=Red , input_rasters[ 1 ]=Green , 
   * input_rasters[ 2 ]=Blue
   *   
   * @note Channels disposition for IHS images:
   * input_rasters[ 0 ]=Intensity , input_rasters[ 1 ]=Hue , 
   * input_rasters[ 2 ]=Saturation
   *
   * @note Channels disposition for HLS images:
   * input_rasters[ 0 ]=Hue , input_rasters[ 1 ]=Lightness , 
   * input_rasters[ 2 ]=Saturation
   *
   * @note This algorithm generates and expects IHS images with the 
   * following channels ranges: I:[0,1] H:[0,2pi] (radians) S:[0,1]
   * and HSL images with the 
   * following channels ranges: H:[0,360] L:[0,1] S:[0,1]
   */
  class PDI_DLL TePDIColorTransform : public TePDIAlgorithm {
    public :

      /**
       * @enum ColorTransfTypes Color transformation types.
       * @note Reference: Digital Image Processing - Gonzales.
       */
      enum ColorTransfTypes {
        /** RGB -> IHS */
        Rgb2Ihs = 1,
        /** IHS -> RGB */
        Ihs2Rgb = 2,
		/** RGB -> HLS */
		Rgb2Hls = 3,
		/** HLS -> RGB */
		Hls2Rgb = 4
      };

      /**
       * @brief Default Constructor.
       *
       */
      TePDIColorTransform();

      /**
       * @briefDefault Destructor
       */
      ~TePDIColorTransform();

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
       * @brief Runs the RGB -> IHS implementation.
       * @param input_rasters Input rasters.
       * @param input_channels Input rasters channels.
       * @param output_rasters Output rasters.
       * @param output_channels Output rasters channels.
       * @param rgb_channels_min The output RGB channels min value.
       * @param rgb_channels_max The output RGB channels max value.       
       * @return true if OK. false if not.
       */
      bool RunRgb2Ihs( TePDITypes::TePDIRasterVectorType& input_rasters,
        std::vector< int >& input_channels,
        TePDITypes::TePDIRasterVectorType& output_rasters,
        std::vector< int >& output_channels,
        const double rgb_channels_min, const double rgb_channels_max );

      /**
       * @brief Runs the IHS -> RGB implementation.
       * @param input_rasters Input rasters.
       * @param input_channels Input rasters channels.
       * @param output_rasters Output rasters.
       * @param output_channels Output rasters channels.
       * @param rgb_channels_min The output RGB channels min value.
       * @param rgb_channels_max The output RGB channels max value.
       * @return true if OK. false if not.
       */
      bool RunIhs2Rgb( TePDITypes::TePDIRasterVectorType& input_rasters,
        std::vector< int >& input_channels,
        TePDITypes::TePDIRasterVectorType& output_rasters,
        std::vector< int >& output_channels,
        const double rgb_channels_min, const double rgb_channels_max );

	  /**
       * @brief Runs the RGB -> HLS implementation.
       * @param input_rasters Input rasters.
       * @param input_channels Input rasters channels.
       * @param output_rasters Output rasters.
       * @param output_channels Output rasters channels.
       * @param rgb_channels_min The output RGB channels min value.
       * @param rgb_channels_max The output RGB channels max value.       
       * @return true if OK. false if not.
       */
      bool RunRgb2Hls( TePDITypes::TePDIRasterVectorType& input_rasters,
        std::vector< int >& input_channels,
        TePDITypes::TePDIRasterVectorType& output_rasters,
        std::vector< int >& output_channels,
        const double rgb_channels_min, const double rgb_channels_max );

	  /**
       * @brief Runs the HLS -> RGB implementation.
       * @param input_rasters Input rasters.
       * @param input_channels Input rasters channels.
       * @param output_rasters Output rasters.
       * @param output_channels Output rasters channels.
       * @param rgb_channels_min The output RGB channels min value.
       * @param rgb_channels_max The output RGB channels max value.
       * @return true if OK. false if not.
       */
      bool RunHls2Rgb( TePDITypes::TePDIRasterVectorType& input_rasters,
        std::vector< int >& input_channels,
        TePDITypes::TePDIRasterVectorType& output_rasters,
        std::vector< int >& output_channels,
        const double rgb_channels_min, const double rgb_channels_max );

  };
  
/** @example TePDIColorTransform_test.cpp
 *    Shows how to use this class.
 */    

#endif //TEPDISTATISTIC_HPP
