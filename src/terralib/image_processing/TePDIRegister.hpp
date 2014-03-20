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

#ifndef TEPDIREGISTER_HPP
  #define TEPDIREGISTER_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  #include "../kernel/TeSharedPtr.h"
  #include "TePDIInterpolator.hpp"

  /**
   * @brief This is the class for digital image registering.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIGeneralAlgoGroup
   *
   * @note The general required parameters:
   *
   * @param proj_params ( TePDITypes::TeProjParamsPtrType ) - The 
   * output projection parameters.
   * @param res_x ( double ) - X Axis output resolution.
   * @param res_y ( double ) - Y Axis output resolution.
   * @param adjust_raster ( TePDITypes::TePDIRasterPtrType ) - The 
   * input raster that needs to be registered.
   * @param output_raster ( TePDITypes::TePDIRasterPtrType ) - The 
   * output raster.
   * @param interpolator ( TePDIInterpolator::InterpMethod ) - Interpolator
   * method to use when rendering output_raster ( For rasters with
   * internal palletes only TePDIInterpolator::NNMethod can be used ).
   * @param adjust_channels (std::vector<int>) - Band(s) to process from
   * adjust_raster.
   *
   * @note The optional parameters are:
   *
   * @param dummy_value (double) - A dummy pixel value for use 
   * in pixels where no data is available.  
   *
   * @param tie_points ( TePDITypes::TeCoordPairVectPtrType ) -
   * The input tie-points where TeCoordPair.pt1 are adjust_rater matricial
   * indexes (They must be inside the respective raster to be valid) and
   * TeCoordPair.pt2 are the respective projected coordinates (this
   * parameter is not necessary if the parameter "transf_params" is given).
   *
   * @param gt_name ( std::string ) - Geometric transformation name ( see
   * TeGTFactory documentation for reference; this
   * parameter is not necessary if the parameter "transf_params" is given ).
   *
   * @param transf_params ( TeGTParams ) - The geometric transformation 
   * parameters (if this parameter is given, the "tie_points" and
   * "gt_name" parameters will be ignored).
   */
  class PDI_DLL TePDIRegister : public TePDIAlgorithm {
    public :
      
      /**
       * @brief Default Constructor.
       *
       */
      TePDIRegister();

      /**
       * @brief Default Destructor
       */
      ~TePDIRegister();
      
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
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );    
     
      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();      
  };
  
/** @example TePDIRegister_test.cpp
 *    Shows how to use this class.
 */    

#endif
