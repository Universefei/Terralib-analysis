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

#ifndef TEPDIMORFFILTER_HPP
  #define TEPDIMORFFILTER_HPP

  #include "TePDIBufferedFilter.hpp"
  #include "TePDIParameters.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
   * @brief This is the class for morfological filter.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIFiltersGroup
   *
   * @note The general required parameters are:
   *
   * @param filter_type ( TePDIMorfType ),
   * @param input_image (TePDITypes::TePDIRasterPtrType),
   * @param output_image (TePDITypes::TePDIRasterPtrType),
   * @param channels (std::vector<int>) - input_image Band(s) to process.
   * @param iterations (int) - Iterations number.
   * @param filter_mask ( TePDIFilterMask::pointer ) Morfological filter mask.
   */
  class PDI_DLL TePDIMorfFilter : public TePDIBufferedFilter {
    public :

      /** @enum Morfological filters types */
      enum TePDIMorfType{
        /** @brief Dilation */
        TePDIMDilationType = 1,
        /** @brief Erosion */
        TePDIMErosionType = 2,
        /** @brief Median */        
        TePDIMMedianType = 3,
        /** @brief Mode */        
        TePDIMModeType = 4
      };

      /**
       * @brief Default Constructor.
       *
       */
      TePDIMorfFilter();

      /**
       * @brief Default Destructor
       */
      ~TePDIMorfFilter();

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
       * @brief A buffer to store temporary double values.
       *
       */
      double* double_buffer_;

      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();

      /**
       * @brief Runs the dilation algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunDilation();

      /**
       * @brief Runs the erosion algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunErosion();

      /**
       * @brief Runs the median algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunMedian();
      
      /**
       * @brief Runs the mode algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunMode();      

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );

  };

/** @example TePDIMorfFilter_test.cpp
 *    Shows how to use this class.
 */  
   
#endif //TEPDIMORFFILTER_HPP
