/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/
/*! \file Te3rdDegPolynomialGT.h
   \brief Third degree polynomial model geometric trasformation.
*/


#ifndef TE3RDDEGPOLYNOMIALGT_H
  #define TE3RDDEGPOLYNOMIALGT_H
  
  #include "TeDefines.h"
  #include "TeGeometricTransformation.h"

  /**
   * @brief Third degree polynomial model geometric trasformation
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup GeometricTransformationGroup
   *
   * @note The pseudo-inverse aproach is used to find the equation system parameters.
   */
  class TL_DLL Te3rdDegPolynomialGT : public TeGeometricTransformation {
    public :

      Te3rdDegPolynomialGT();
    
      ~Te3rdDegPolynomialGT();
      
      //overload       
      unsigned int getMinRequiredTiePoints() const;      
      
    protected :
    
      //overload    
      void directMap( const TeGTParams& params, const TeCoord2D& pt1, 
        TeCoord2D& pt2 ) const;
      
      //overload      
      void inverseMap( const TeGTParams& params, const TeCoord2D& pt2, 
        TeCoord2D& pt1 ) const;      
      
      //overload     
      bool computeParameters( TeGTParams& params ) const;
      
      //overload      
      bool isTransDefined( const TeGTParams& params ) const;        
  };
  
#endif



