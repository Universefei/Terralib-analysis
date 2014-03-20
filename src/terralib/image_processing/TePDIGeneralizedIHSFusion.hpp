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

#ifndef TEPDIGENERALIZEDIHSFUSION_HPP
  #define TEPDIGENERALIZEDIHSFUSION_HPP

  #include "../kernel/TeSharedPtr.h"
  #include "TePDIParameters.hpp"
  #include "TePDIAlgorithm.hpp"

  /**
   * @class TePDIGeneralizedIHSFusion
   * @brief This is the class for Multi Bands IHS fusion strategy.
   * @warning This class is under development and may produce incorrect 
   * results or generate execution errors, please wait for the next version.   
   * @author Felipe Castro da Silva <felipe@dpi.inpe.br>
   * @ingroup PDIFusionAlgorithms
   * @param pp_type (TePDIGeneralizedIHSFusion::PreProcessingType) - 
   * Input rasters pre-processing type.
   * @param mean (double) - ???
   * @param variance (double) - ???
   * @param bands_center (vector<double>) - Bands center angles 
   * (decimal degrees).
   * @param input_rasters (TePDITypes::TePDIRasterVectorType) - Input
   * low resolution rasters (only the first band/channel will be
   * used).
   * @param pansharpening (bool) - ???
   * @param reference_raster ( TePDITypes::TePDIRasterPtrType ) - 
   *  Reference raster ( higher resolution ). 
   * @param reference_raster_band (int) - Reference raster band/channel. 
   * @param ihs_output_rasters (TePDITypes::TePDIRasterVectorType) - Pointers
   * to the output generated IHS components rasters.
   * @param rgb_output_raster (TePDITypes::TePDIRasterPtrType) - A pointer
   * to the output generated RGB raster.
   *
   * @example TePDIFusion_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIGeneralizedIHSFusion : public TePDIAlgorithm {
    public :
      typedef TeSharedPtr< TePDIGeneralizedIHSFusion > pointer;
      typedef const TeSharedPtr< TePDIGeneralizedIHSFusion > const_pointer;
      
      /** Input rasters pre processing type */
      enum PreProcessingType
      {
        NoPreProcessing = 0,
        VariancePreProcessing = 1,
        MeanPreProcessing = 2
      };
      
      /**
       * Default constructor
       *
       * @param factoryName Factory name.
       */
      TePDIGeneralizedIHSFusion();

      /**
       * Default Destructor
       */
      ~TePDIGeneralizedIHSFusion();

      /**
       * Checks if the supplied parameters fits the requirements of each
       * PDI strategy.
       *
       * Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      bool CheckParameters( 
        const TePDIParameters& parameters ) const;       

    protected :
      /**
       * Runs the algorithm implementation.
       *
       * @return true if OK. false on errors.
       */
      bool RunImplementation();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );
  };
#endif
