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

#ifndef TEPDIPRINCIPALCOMPONENTS_HPP
  #define TEPDIPRINCIPALCOMPONENTS_HPP

  #include "TePDIAlgorithm.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
   * @brief This is the class for principal components generation.
   * @author Felipe Castro da Silva <felipe@dpi.inpe.br>
   * @ingroup TePDIGeneralAlgoGroup
   *
   * @note The required parameters are:
   *
   * @param analysis_type ( TePDIPrincipalComponents::TePDIPCAType ) - The analysis type.
   * @param input_rasters ( TePDITypes::TePDIRasterVectorType ) - The input rasters.
   * @param output_raster ( TePDITypes::TePDIRasterVectorType ) - The output raster.
   * @param bands ( std::vector< int > ) - The bands to use from each input raster.
   * @param covariance_matrix ( TeSharedPtr<TeMatrix> ) - This
   * is the output covariance matrix generated when TePDIPCADirect
   * is used, or, this is the input covariance matrix when
   * TePDIPCAInverse is used.
   *
   * @note The optional parameters are:
   *
   * @param output_rasters  ( TePDITypes::TePDIRasterVectorType ) - The output raters, each one with just one band/channel, if this parameter is set the output_raster parameter will be ignored.
   */
  class PDI_DLL TePDIPrincipalComponents : public TePDIAlgorithm {
    public :
      /** @enum TePDIPCAType Direction of the PCA analisys */
      enum TePDIPCAType
      {
        TePDIPCADirect, 
        TePDIPCAInverse
      };

      /**
       * @brief Default Constructor.
       *
       */
      TePDIPrincipalComponents();

      /**
       * @brief Default Destructor
       */
      ~TePDIPrincipalComponents();

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
       * @brief Runs the direct algorithm.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation_direct();

      /**
       * @brief Runs the inverse algorithm.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation_inverse();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );
  };
  
/** @example TePDIPrincipalComponents_test.cpp
 *    Shows how to use this class.
 */    

#endif //TEPDIPRINCIPALCOMPONENTS_HPP
