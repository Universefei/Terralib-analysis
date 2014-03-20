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

#ifndef TEPDIWAVELETATROUSFUSION_HPP
  #define TEPDIWAVELETATROUSFUSION_HPP

  #include "TePDILevelRemap.hpp"
  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  #include "TePDIWaveletAtrous.hpp"
  #include "TePDIInterpolator.hpp"
  
  #include "../kernel/TeSharedPtr.h"

  /**
   * @class TePDIWaveletAtrousFusion 
   * @brief Wavelets Atrous Fusion.
   * @author Felipe Castro da Silva <felipe@dpi.inpe.br>
   * @ingroup PDIFusionAlgorithms
   *
   * @note The required parameters are:
   *
   * @param input_rasters (TePDITypes::TePDIRasterVectorType) - Low resolution rasters.
   * @param bands (std::vector< int >) - The bands from each low resolution raster.
   * @param output_rasters (TePDITypes::TePDIRasterVectorType) - High resolution fused rasters.
   * @param reference_raster (TePDITypes::TePDIRasterPtrType) - High resolution raster.
   * @param reference_raster_band (int) - Reference raster band number.
   * @param rasters_levels (int) - Decomposition levels for each low resolution raster.
   * @param reference_levels (int) - Decomposition levels for each reference raster.
   * @param resampling_type (TePDIWaveletAtrousFusion::InterpMethod) - Resampling type.
   * @param fit_histogram (bool) - Fit histogram before the substitution.
   *
   * @note The optional parameters are:
   * @param filter_file (std::string) - Input filter file name (default:
   * Use internal default TePDIWaveletAtrous filter bank).
   * @param filter_type (TePDIWaveletAtrous::FilterType) - Filter type
   * (default:Use internal default TePDIWaveletAtrous filter bank).   
   *
   * @example TePDIFusion_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIWaveletAtrousFusion : public TePDIAlgorithm {
    public :
      /**
       * @enum Allowed interpolation methods.
       */      
      enum InterpMethod {
        /** No interpolation method. */
        Nothing = 0,
        /** Near neighborhood interpolation method. */
        NNMethod = 1,
        /** Bilinear interpolation method. */
        BilinearMethod = 2,
        /** Bicubic interpolation method. */
        BicubicMethod = 3      
      };

      /**
       * @brief Default Constructor.
       *
       */
      TePDIWaveletAtrousFusion();

      /**
       * @brief Default Destructor
       */
      ~TePDIWaveletAtrousFusion();

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
       * @brief Run the wavelet fusion method.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState(const TePDIParameters&);
      
      /**
       * @brief Raster resampling to new resolution, forcing lines and columns.
       * @param input_raster Input raster.
       * @param output_raster Output raster.
       * @param x_resolution_ratio X resolution ratio.
       * @param y_resolution_ratio Y resolution ratio.
       * @param l Output raster Lines.
       * @param c Output raster Columns.
       * @param interpol Interpolation method.
       * @param enable_progress Enable/disable the progress interface.
       * @return true if OK, false on errors.
       * 
       * @note ( resolution_ratio = output_resolution / input_resolution ).
       */      
      bool resampleRasterByResLinsCols(
        const TePDITypes::TePDIRasterPtrType& input_raster,
        const TePDITypes::TePDIRasterPtrType& output_raster,
        double x_resolution_ratio,
        double y_resolution_ratio,
        int l,
        int c,
        TePDIInterpolator::InterpMethod interpol,
        bool enable_progress );  
  };


#endif //TEPDIWaveletAtrous_HPP
