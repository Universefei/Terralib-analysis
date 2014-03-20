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

#ifndef TEPDIWAVELETATROUS_HPP
  #define TEPDIWAVELETATROUS_HPP

	#include "TePDILevelRemap.hpp"
	#include "TePDIAlgorithm.hpp"
	#include "TePDIParameters.hpp"
	#include "../kernel/TeSharedPtr.h"

	#define waveletPlanes 2

  /**
   * @class TePDIWaveletAtrous
   * @brief This is the class for principal components generation.
   * @author Felipe Castro da Silva <felipe@dpi.inpe.br>
   * @ingroup PDIWaveletsAlgorithms
   *
   * @note The general required parameters are:
   * @param direction (TeWaveletAtrousDirection) - Decompose or Recompose.
   *
   * @note The required parameters for the DECOMPOSE are:
   * @param input_raster (TePDITypes::TePDIRasterPtrType) - Raster to do 
   * wavelet analysis.
   * @param band (int) - Raster band.
   * @param levels (int) - Levels to decompose the raster.
   * @param output_wavelets (TePDITypes::TePDIRasterVectorType) - Raster 
   * vector to store de residual and coefficients planes - All output rasters
   * must be pre-initiated with the same number of lines and columns
   * of the input_raster and with the number of bands related to the
   * desired wavelet planes.
   *
   * @note The DECOMPOSE optional parameters are: 
   * @param filter_file (std::string) - Input filter file name (default:
   * Use internal filter bank TriangleFilter).
   * @param filter_type (TePDIWaveletAtrous::FilterType) - 
   * Filter type (default:TriangleFilter).
   *
   * @note The required parameters for the RECOMPOSE are:
   * @param input_rasters_wavelets (std::vector<TePDITypes::TePDIRasterVectorType>) - Residual and coefficient planes for each band.
   * @param rasters_levels (int) - Number of decomposition in the input_rasters_wavelets.
   * @param reference_raster_wavelets (TePDITypes::TePDIRasterVectorType) - Residual and coefficient planes for the reference band.
   * @param reference_levels (int) - Number of decomposition in the reference_raster_wavelets.
   * @param output_rasters (TePDITypes::TePDIRasterVectorType) - Output raster after recomposing the wavelet analysis.
   *
   * @example TePDIWavelets_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIWaveletAtrous : public TePDIAlgorithm {
    public :
      /** @enum TeWaveletAtrousDirection Decompose or Compose */
      enum TeWaveletAtrousDirection {DECOMPOSE, RECOMPOSE};
      
      /** @enum TeFilterBanks Filter typs available */
      enum FilterType 
      {
        B3SplineFilter, 
        TriangleFilter
      };

      /**
       * @brief Default Constructor.
       *
       */
      TePDIWaveletAtrous();

      /**
       * @brief Default Destructor
       */
      ~TePDIWaveletAtrous();

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
       * @brief Decide the direction of the analysis based on the direction parameter.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();

      /**
       * @brief Decomposition.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation_decompose();

      /**
       * @brief Recomposition.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation_recompose();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState(const TePDIParameters&);
      
      bool filterBank(string filterFile,TeMatrix &filter);
      bool filterBank(FilterType filtertype,TeMatrix &filter);
  };

#endif //TEPDIWaveletAtrous_HPP
