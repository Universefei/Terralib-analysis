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

/**
 * @defgroup PDIModule Image Processing.
 */  
 
/**
 * @defgroup PDIAlgorithms Image Processing Algorithms.
 * @ingroup PDIModule
 */ 
 
/**
 * @defgroup PDIAlgorithmsFactories Algorithms factories.
 * @ingroup PDIModule
 */   
 
/**
 * @defgroup TePDIMosaicGroup Mosaic algorithms.
 * @ingroup PDIAlgorithms
 */   
 
/**
 * @defgroup TePDISegmentationGroup Segmentation algorithms.
 * @ingroup PDIAlgorithms
 */
 
/**
 * @defgroup TePDIFiltersGroup Filtering algorithms.
 * @ingroup PDIAlgorithms
 */ 
 
/**
 * @defgroup TePDIEnhancementGroup Image enhancement algorithms.
 * @ingroup PDIAlgorithms
 */  
 
/**
 * @defgroup TePDIGeneralAlgoGroup General purpose algorithms.
 * @ingroup PDIAlgorithms
 */ 
 
/**
 * @defgroup PDIStrategies Algorithm strategies.
 * @ingroup PDIModule
 */   
 
/**
 * @defgroup PDIStrategiesFactories Strategies factories.
 * @ingroup PDIModule
 */   
 
/**
 * @defgroup PDIClassificationAlgorithms Classification algorithms.
 * @ingroup PDIAlgorithms
 */  

/**
 * @defgroup PDIWaveletsAlgorithms Wavelets algorithms.
 * @ingroup PDIAlgorithms
 */  
  
/**
 * @defgroup PDIBlendingStrategies Blending strategies.
 * @ingroup PDIStrategies
 */   
 
/**
 * @defgroup PDIAux Image processing auxiliary classes/methods.
 * @ingroup PDIModule
 */    
 
/**
 * @defgroup PDIFusionAlgorithms Image Fusion algorithms.
 * @ingroup PDIAlgorithms
 */   
 
/**
 * @defgroup PDIMatchingAlgorithms Image matching algorithms.
 * @ingroup PDIAlgorithms
 */  

/**
 * @file TePDIDefines.hpp
 * @brief This file contains a set of definitions used by PDI module.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 */

#ifndef TEPDIDEFINES_HPP
  #define TEPDIDEFINES_HPP
  
  #include "../kernel/TeDefines.h"

  /** @defgroup PDI_AS_DLL macros.
    @{
  */

  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    #ifdef PDI_AS_DLL
      #define PDI_DLL __declspec(dllexport)
    #else
      #define PDI_DLL __declspec(dllimport)
    #endif  
  #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_APPLE
    #define PDI_DLL
  #else
    #error "ERROR: Unsupported platform"
  #endif

  /** @} */ 

#endif
