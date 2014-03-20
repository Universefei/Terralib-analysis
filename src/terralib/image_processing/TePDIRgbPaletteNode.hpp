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

#ifndef TEPDIRGBPALETTENODE_HPP
  #define TEPDIRGBPALETTENODE_HPP

  #include "TePDIDefines.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
   * @class TePDIRgbPaletteNode
   * @brief This is the class for RGB Palette Node.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIAux
   */  
  class PDI_DLL TePDIRgbPaletteNode {
    public :
      /** @typedef TeSharedPtr< TePDIRgbPaletteNode > pointer 
        Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIRgbPaletteNode > pointer;
      
      /** @typedef const TeSharedPtr< TePDIRgbPaletteNode > const_pointer 
        Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIRgbPaletteNode > const_pointer;
      
      /**
       * @brief Red channel value.
       *
       */
      unsigned int red_;

      /**
       * @brief Green channel value.
       *
       */
      unsigned int green_;

      /**
       * @brief Blue channel value.
       *
       */
      unsigned int blue_;
            
      
      /**
       * @brief Default Constructor
       *
       */
      TePDIRgbPaletteNode()
      {
        red_ = green_ = blue_ = 0;
      };

      /**
       * @brief Alternative Constructor
       *
       */
      TePDIRgbPaletteNode( unsigned int red, unsigned int green,
        unsigned int blue )
      {
        red_ = red;
        green_ = green;
        blue_ = blue;
      };
            
      /**
       * @brief Default Destructor
       */
      ~TePDIRgbPaletteNode()
      {};

  };
  
#endif
