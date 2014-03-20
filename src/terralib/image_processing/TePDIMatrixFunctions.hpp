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

#ifndef TEPDIMATRIXFUNCTIONS_HPP
  #define TEPDIMATRIXFUNCTIONS_HPP
  
  #include "TePDIMatrix.hpp"
  #include "TePDITypes.hpp"
  
  /**
   * @brief This namespace contains functions for TePDIMatrix manipulation.
   * @note THEY SHOLD NOT BE USED BY ANYONE. The support and interfaces 
   * can be changed in future. THIS IS FOR INTERNAL USE ONLY.     
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIModule
   */
  namespace TePDIMatrixFunctions
  {
    /**
     * @brief Matrices correlation.
     * 
     * @param matrix1 Input matrix 1.
     * @param matrix2 Input matrix 2.
     * @return The correlation between the two matrix.
     * @note The calcule will only be made for elements that exists in both
     * matrices.
     */
    PDI_DLL double GetCorrelation( const TePDIMatrix< double >& matrix1,
      const TePDIMatrix< double >& matrix2 );
      
    /**
     * @brief Absolute values matrix generation.
     * 
     * @param inMatrix Input matrix.
     * @param absMatrix Output matrix.
     * @return true if ok, false on errors.
     */
    PDI_DLL bool Abs( const TePDIMatrix< double >& inMatrix,
      TePDIMatrix< double >& absMatrix );
      
    /**
     * @brief Locates all inMatrix1 elements there are greater than
     * inMatrix2 elements. These elements positions will be marked with
     * value 1 in gtMatrix. The other positions will be marked with 0's.
     * 
     * @param inMatrix1 Input matrix.
     * @param inMatrix2 Input matrix.
     * @param gtMatrix Output matrix.
     * @return true if ok, false on errors.
     */
   PDI_DLL bool WhereIsGreater( const TePDIMatrix< double >& inMatrix1,
      const TePDIMatrix< double >& inMatrix2,
      TePDIMatrix< double >& gtMatrix );
      
    /**
     * @brief Locates all inMatrix1 zero elements. These elements positions will be 
     * marked with value 1 in negMatrix. The other positions will be marked 
     * with 0's. 
     * @param inMatrix Input matrix.
     * @param negMatrix Output matrix.
     * @return true if ok, false on errors.
     */
   PDI_DLL bool Negate( const TePDIMatrix< double >& inMatrix,
      TePDIMatrix< double >& negMatrix );       
  };
  
/** @example TePDIMatrixFunctions_test.cpp
 *    Shows how to use this namespace functions.
 */    

#endif
