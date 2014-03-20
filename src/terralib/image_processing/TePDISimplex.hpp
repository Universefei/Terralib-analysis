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

#ifndef TEPDISIMPLEX_HPP
  #define TEPDISIMPLEX_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  
  #include "../kernel/TeSharedPtr.h"

  /**
   * @brief This is the class for Simplex, an implementation of the Nelder-Mead simplex method to minimize a given function.
   * @author Alexandre Noma <alex.noma@gmail.com>
   * @ingroup TePDIGeneralAlgoGroup
   * @note Based on the original code from Michael F. Hutt (http://www.mikehutt.com)
   *
   * @note The general required parameters:
   *
   * @param objective_function (double (*func)(double*)) - pointer for a objective function which receives a double array and returns a double value.
   *
   * @param num_variables (int) - number of variables.
   *
   * @param x_array (double *) - an array with starting points.
   *
   * @param max_iterations (int) - number of maximum iterations.
   *
   * @param epsilon (double) - value used for convergence test.
   *
   */
  class PDI_DLL TePDISimplex : public TePDIAlgorithm {
    public :
      /**
       * Default Constructor.
       *
       */
      TePDISimplex();

      /**
       * Default Destructor
       */
      ~TePDISimplex();
      
      /**
       * Checks if the supplied parameters fits the requirements of each
       * PDI algorithm implementation.
       *
       * Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      bool CheckParameters( const TePDIParameters& parameters ) const;      

    protected :
    
      /**
       * Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );    
     
      /**
       * Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();      
  };
  
/** @example TePDISimplex_test.cpp
 *    Shows how to use this class.
 */    

#endif
