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
 * @file TePDITypes.hpp
 * @brief This file contains a set of type definitions used by PDI module
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 */

#ifndef TEPDITYPES_HPP
  #define TEPDITYPES_HPP

  #include "TePDIDefines.hpp"
  #include "../kernel/TeSharedPtr.h"

  #include "../kernel/TeRaster.h"
  #include "../kernel/TeMatrix.h"
  #include "../kernel/TeLayer.h"
  #include "../kernel/TeGeometry.h"
  #include "../kernel/TeCoord2D.h"
  #include "../kernel/TeProjection.h"

  #include <map>
  #include <vector>

  /**
   * @brief This namespace contains the global types used by all PDI classes.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIModule
   */
  namespace TePDITypes
  {
    /**
     * @brief Type definition for a simple LUT - Look-Up Table.
     *
     * @param double (First) input level.
     * @param double (Second) output level.
     */
    typedef std::map< double, double > TePDILutType;
    
    /**
     * @brief Type definition for a simple LUT pointer.
     */
    typedef TeSharedPtr< TePDILutType > TePDILutPtrType;    

    /**
     * @brief Type definition for a two dimentional LUT - Look-Up Table.
     *
     * @param double Input level from the first source image.
     * @param TePDILutType A lut with input levels from the second source image.
     */
    typedef std::map< double, TePDILutType > TePDI2DLutType;

    /**
     * @brief Look-Up Table vector alias definition.
     */
    typedef std::vector< TePDILutType > TePDILutsVectorType;

    /**
     * @brief TeRaster pointer definition.
     */
    typedef TeSharedPtr< TeRaster > TePDIRasterPtrType;

    /**
     * @brief TeRasters vector definition.
     */
    typedef std::vector< TePDIRasterPtrType > TePDIRasterVectorType;
    
    /**
     * @brief TeRasters vector pointer definition.
     */
    typedef TeSharedPtr< TePDIRasterVectorType > TePDIRasterVectorPtrType;    

    /**
     * @brief TeLayer pointer definition.
     */
    typedef TeSharedPtr< TeLayer > TePDILayerPtrType;
    
    /**
     * @brief TePolygon pointer definition.
     */
    typedef TeSharedPtr< TePolygon > TePDIPolygonPtrType;    

    /**
     * @brief TePolygonSet pointer definition.
     */
    typedef TeSharedPtr< TePolygonSet > TePDIPolygonSetPtrType; 
        
    /**
     * @brief TePolygonSet map definition.
     */    
    typedef std::map< double, TePolygonSet > TePDIPolSetMapType;      
    
    /**
     * @brief TePolygonSet map pointer definition.
     */
    typedef TeSharedPtr< TePDIPolSetMapType > TePDIPolSetMapPtrType;      

    /**
     * @brief TeMatrix pointer definition.
     */
    typedef TeSharedPtr< TeMatrix > TePDIMatrixPtrType;
    
    /**
     * @brief TeCoordPairVect pointer definition.
     */
    typedef TeSharedPtr< TeCoordPairVect > TeCoordPairVectPtrType;    
    
    /**
     * @brief TeProjectionParams pointer definition.
     */
    typedef TeSharedPtr< TeProjectionParams > TeProjParamsPtrType;    
  };

#endif //TEPDITYPES_HPP
