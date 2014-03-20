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

#ifndef TEPDIFUSIONINDEXES_HPP
  #define TEPDIFUSIONINDEXES_HPP

  #include "TePDILevelRemap.hpp"
  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
   * @brief This is the class for principal components generation.
   * @author Felipe Castro da Silva <felipe@dpi.inpe.br>
   * @ingroup PDIAlgorithms
   *
   * @note The required parameters for band to band indexes are:
   *
   * @param index_type (TePDIFusionIndexes::TeFusionIndexes) - CC, SDD, BIAS, RMSE
   * @param original_raster (TePDITypes::TePDIRasterPtrType) - Original raster.
   * @param original_band (int) - Band of the Original raster.
   * @param fused_raster (TePDITypes::TePDIRasterPtrType) - Fused raster.
   * @param fused_band (int) - Band of the Fused raster.
   * @param index_value (TeSharedPtr<double>) - Index value.
   *
   * @note The required parameters for all bands indexes are:
   *
   * @param index_type (TePDIFusionIndexes::TeFusionIndexes) - ERGAS
   * @param original_rasters (TePDITypes::TePDIRasterVectorType) - Original rasters.
   * @param original_bands (std::vector<int>) - Bands of the Original rasters.
   * @param fused_rasters (TePDITypes::TePDIRasterVectorType) - Fused rasters.
   * @param fused_bands (std::vector<int>) - Band of the Fused rasters.
   * @param index_value (TeSharedPtr<double>) - Index value.
   */
  class PDI_DLL TePDIFusionIndexes : public TePDIAlgorithm {
    public :
      /** @enum TeFusionIndexes Fusion indexes available */
      enum TeFusionIndexes {CC, BIAS, VARDIF, SDD, UIQI, RMSE, ERGAS, SIMM, SERGAS, SCC};
      /** @typedef Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIFusionIndexes > pointer;
      /** @typedef Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIFusionIndexes > const_pointer;

      /**
       * @brief Default Constructor.
       *
       */
      TePDIFusionIndexes();

      /**
       * @brief Default Destructor
       */
      ~TePDIFusionIndexes();

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
	* @brief Decide the direction of the analysis based on the analysis_type parameter.
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
	* @brief Compute the raster meam.
	*
	* @return true if OK. false on error.
	*/
	bool raster_mean(TePDITypes::TePDIRasterPtrType raster, int band, double& mean);

	/**
	* @brief Compute the difference between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool rasters_difference(TePDITypes::TePDIRasterPtrType raster1, int band1, TePDITypes::TePDIRasterPtrType raster2, int band2, TePDITypes::TePDIRasterPtrType raster_diff, int band_diff);

	/**
	* @brief Compute the CORRELATION COEFFICIENT INDEX between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool correlation_coefficient_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> bias);

	/**
	* @brief Compute the BIAS index between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool bias_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> bias);

	/**
	* @brief Compute the VARIANCE DIFFERENCE INDEX between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool variance_difference_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> vardif);

	/**
	* @brief Compute the STANDARD DEVIATION INDEX of the raster produce by the difference between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool standard_deviation_diff_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> bias);

	/**
	* @brief Compute the UIQI between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool uiqi_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> uiqi);

	/**
	* @brief Compute the RMSE index between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool rmse_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> rmse);

	/**
	* @brief Compute the ERGAS index between two raster set.
	*
	* @return true if OK. false on error.
	*/
	bool ergas_index(TePDITypes::TePDIRasterVectorType original_rasters, std::vector<int> original_bands, TePDITypes::TePDIRasterVectorType fused_rasters, std::vector<int> fused_bands, double low_resolution, double high_resolution, TeSharedPtr<double> ergas);

	/**
	* @brief Compute the STRUCTURAL SIMILARITY index between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool structural_similarity_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> simm);

	/**
	* @brief Fit histogram between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool fit_histogram(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType pan_raster, int pan_band, TePDITypes::TePDIRasterPtrType temp_raster);

	/**
	* @brief Compute the Spatial ERGAS index between two raster set.
	*
	* @return true if OK. false on error.
	*/
	bool sergas_index(TePDITypes::TePDIRasterVectorType original_rasters, std::vector<int> original_bands, TePDITypes::TePDIRasterPtrType pan_raster, int pan_band, TePDITypes::TePDIRasterVectorType fused_rasters, std::vector<int> fused_bands, double low_resolution, double high_resolution, TeSharedPtr<double> ergas);

	/**
	* @brief Compute the Spacial Correlation Coefficient index between two rasters.
	*
	* @return true if OK. false on error.
	*/
	bool spatial_correlation_coefficient_index(TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TePDITypes::TePDIRasterPtrType pan_raster, int pan_band, TeSharedPtr<double> scc);
  };

#endif //TEPDIFUSIONINDEXES_HPP
