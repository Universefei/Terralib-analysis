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

#ifndef TEPDIPRINCIPALCOMPONENTSFUSION_HPP
  #define TEPDIPRINCIPALCOMPONENTSFUSION_HPP

  #include "TePDIAlgorithm.hpp"

  /**
   * @brief This is the class for principal components generation.
   * @author Felipe Castro da Silva <felipe@dpi.inpe.br>
   * @ingroup PDIFusionAlgorithms
   *
   * @note The required parameters are:
   *
   * @param input_rasters (TePDITypes::TePDIRasterVectorType) - Low resolution rasters.
   * @param bands (std::vector< int >) - The bands from each low resolution raster.
   * @param output_raster (TePDITypes::TePDIRasterPtrType) - High resolution fused raster.
   * @param reference_raster (TePDITypes::TePDIRasterPtrType) - High resolution raster.
   * @param reference_raster_band (int) - Reference raster band number.
   * @param resampling_type (TePDIInterpolator::InterpMethod) - 
   * Resampling type.
   * @param fit_histogram (bool) - Fit the reference histogram to the
   * low resolution rasters histograms (better collor quality).
   *
   * @note The optional parameters are:
   *
   * @param output_rasters (TePDITypes::TePDIRasterVectorType) - High resolution fused rasters, each one represents one band/channel, if this parameter is set the output_raster parameter will be ignored.
   *
   */
  class PDI_DLL TePDIPrincipalComponentsFusion : public TePDIAlgorithm {
    public :

      /**
       * @brief Default Constructor.
       *
       */
      TePDIPrincipalComponentsFusion();

      /**
       * @brief Default Destructor
       */
      ~TePDIPrincipalComponentsFusion();

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
      void ResetState( const TePDIParameters& params );
  };
  
/** @example TePDIFusion_test.cpp
 *    Fusion algorithms test.
 */    

#endif //TEPDIPRINCIPALCOMPONENTS_HPP
