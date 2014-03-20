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

#ifndef PDI_ALGORITHM_FACTORIES_REGISTERED
  #define PDI_ALGORITHM_FACTORIES_REGISTERED

  #include "TePDIArithmeticFactory.hpp"
  #include "TePDIBDFilterFactory.hpp"
  #include "TePDIColorTransformFactory.hpp"
  #include "TePDIContrastFactory.hpp"
  #include "TePDIMorfFilterFactory.hpp"
  #include "TePDILinearFilterFactory.hpp"
  #include "TePDIPrincipalComponentsFactory.hpp"
  #include "TePDIRadarFrostFilterFactory.hpp"
  #include "TePDIRadarKuanFilterFactory.hpp"
  #include "TePDIRadarLeeFilterFactory.hpp"
  #include "TePDIRaster2VectorFactory.hpp"
  #include "TePDIRegGrowSegFactory.hpp"
  #include "TePDIRegisterFactory.hpp"
  #include "TePDIIsosegClasFactory.hpp"
  #include "TePDIMixModelFactory.hpp"
  #include "TePDIKMeansClasFactory.hpp"
  #include "TePDIEMClasFactory.hpp"
  #include "TePDISensorSimulator.hpp"
  
#endif 

#ifndef TEPDIALGORITHMFACTORY_HPP
  #define TEPDIALGORITHMFACTORY_HPP

  #include "../kernel/TeSharedPtr.h"
  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  #include "TePDIDefines.hpp"
  
  #include "../kernel/TeFactory.h"
  
  #include <string>
  
  /**
   * @brief This is the class for digital image processing algorithms factory.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIAlgorithmsFactories
   */
  class PDI_DLL TePDIAlgorithmFactory : 
    public TeFactory< TePDIAlgorithm, TePDIParameters >
  {
    public :
      typedef TeSharedPtr< TePDIAlgorithmFactory > pointer;
      typedef const TeSharedPtr< TePDIAlgorithmFactory > const_pointer;
      
      /**
       * @brief Default Destructor
       */
      virtual ~TePDIAlgorithmFactory();
      
    protected :
      
      /**
       * @brief Default constructor
       *
       * @param factoryName Factory name.
       */
      TePDIAlgorithmFactory( const std::string& factoryName );
  };
  
#endif



