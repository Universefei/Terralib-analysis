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

#ifndef PDI_STRATEGY_FACTORIES_REGISTERED
  #define PDI_STRATEGY_FACTORIES_REGISTERED

#endif 

#ifndef TEPDISTRATEGYFACTORY_HPP
  #define TEPDISTRATEGYFACTORY_HPP

  #include "../kernel/TeSharedPtr.h"
  #include "TePDIStrategy.hpp"
  #include "TePDIParameters.hpp"
  
  #include "../kernel/TeFactory.h"
  
  #include <string>

  /**
   * @brief This is the class for digital image processing strategy factory.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIStrategyFactories
   */
  class PDI_DLL TePDIStrategyFactory : 
    public TeFactory< TePDIStrategy, TePDIParameters >
  {
    public :
      /** @typedef Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIStrategyFactory > pointer;
      /** @typedef Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIStrategyFactory > const_pointer;
      
      /**
       * @brief Default Destructor
       */
      virtual ~TePDIStrategyFactory();
      
    protected :
      
      /**
       * @brief Default constructor
       *
       * @param factoryName Factory name.
       */
      TePDIStrategyFactory( const std::string& factoryName );
  };
  
#endif



