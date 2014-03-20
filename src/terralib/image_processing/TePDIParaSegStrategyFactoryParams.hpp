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

#ifndef TEPDIPARASEGSTRATEGYFACTORYPARAMS_HPP
  #define TEPDIPARASEGSTRATEGYFACTORYPARAMS_HPP

  #include "TePDIParaSegStrategyParams.hpp"
  #include "TePDIDefines.hpp"

  /*!
      \class TePDIParaSegStrategyFactoryParams
      \brief Multi-threaded image segmenter strategy factory parameters.
      \author Emiliano F. Castejon <castejon@dpi.inpe.br>.
      \ingroup PDIAux
  */
  class PDI_DLL TePDIParaSegStrategyFactoryParams
  {
    public :
   
      TePDIParaSegStrategyFactoryParams();

      ~TePDIParaSegStrategyFactoryParams();
      
      /*! Segmenter strategy parameters*/
      TePDIParaSegStrategyParams stratParams_;      
      
      // Factory compatibility
      std::string decName() const;
 
  };


#endif
