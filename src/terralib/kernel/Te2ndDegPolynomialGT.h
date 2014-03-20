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
/*! \file Te2ndDegPolynomialGT.h
   \brief Second degree polynomial model geometric trasformation.
*/


#ifndef TE2NDDEGPOLYNOMIALGT_H
  #define TE2NDDEGPOLYNOMIALGT_H
  
  #include "TeDefines.h"
  #include "TeGeometricTransformation.h"

  /**
   * @brief Second degree polynomial model geometric trasformation
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup GeometricTransformationGroup
   *
   * @note The pseudo-inverse aproach is used to find the equation system parameters.
   */
  class TL_DLL Te2ndDegPolynomialGT : public TeGeometricTransformation {
    public :

      Te2ndDegPolynomialGT();
    
      ~Te2ndDegPolynomialGT();
      
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



