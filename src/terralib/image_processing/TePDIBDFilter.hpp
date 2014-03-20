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

#ifndef TEPDIBDFILTER_HPP
  #define TEPDIBDFILTER_HPP

  #include "TePDIBufferedFilter.hpp"
  #include "TePDIParameters.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
   * @brief This is the class for border detection filter.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   *
   * @note The general required parameters are:
   *
   * @param filter_type ( TePDIBDType ) - Edge detection type.
   * @param input_image (TePDITypes::TePDIRasterPtrType) - Input image.
   * @param output_image (TePDITypes::TePDIRasterPtrType) - Output image.
   * @param channels (std::vector<int>) - Band(s) to process. For
   * @param iterations (int) - Iterations number.
   *
   * @ingroup TePDIFiltersGroup
   */
  class PDI_DLL TePDIBDFilter : public TePDIBufferedFilter{
    public :

      /**
       * @brief Edge detection types.
       */  
      enum TePDIBDType{
        /** Sobel edge detector */
        TePDISobel = 1,
        /** Roberts edge detector */
        TePDIRoberts = 2
      };

      /**
       * @brief Default Constructor.
       *
       */
      TePDIBDFilter();

      /**
       * @brief Default Destructor
       */
      ~TePDIBDFilter();

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
       * @brief Runs the Sobel algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunSobel();

      /**
       * @brief Runs the Roberts algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunRoberts();
  };
  
/** @example TePDIBDFilter_test.cpp
 *    Shows how to use this class.
 */    

#endif //TEPDIBDFILTER_HPP
