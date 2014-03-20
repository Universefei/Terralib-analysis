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

#ifndef TEPDIMATHFUNCTIONS_HPP
  #define TEPDIMATHFUNCTIONS_HPP
  
  #include "TePDIDefines.hpp"
  #include "../kernel/TeCoord2D.h"
  #include "../kernel/TeBox.h"
  #include "../kernel/TeGeometry.h"
  
    /**
     * @brief This namespace contains mathematic functions used by PDI 
     * algorithms. 
     * @note THEY SHOLD NOT BE USED BY ANYONE. The support and interfaces 
     * can be changed in future. THIS IS FOR INTERNAL USE ONLY.     
     * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
     * @ingroup PDIModule
     */
  namespace TePDIMathFunctions
  {
      
    /**
     * @brief Returns the number of decimation levels for a given number of elements.
     *
     * @param elements Elements number.
     * @return The decimation levels.
     */    
    template< typename T >
    unsigned long int DecimLevels( const T& elements )
    {
      if( elements < 2 )
      {
        return 0;
      } else if( elements == 3 ) {
        return 2;
      } else {
        return ( 1 + DecimLevels( ( elements + 
          ( elements % 2 ) ) / 2 ) );
      }
    }
    
    /**
     * @brief Returns the number of elements of a decimation level.
     *
     * @param level The decimation level (starting from 1).
     * @param elements The initial elements number.
     * @return The number of elements for the required level.
     */    
    PDI_DLL unsigned long int DecimLevelSize( unsigned long int level,
      unsigned long int elements );
    
  };
  
/** @example TePDIMathFunctions_test.cpp
 *    Shows how to use this namespace functions.
 */    

#endif
