/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/** \file TeDecoderPAM.h
 *  \brief Implements a decoder to read/write PAM (Portable Arbitrary Maps) 
 * image files ( PBM, PGM, or PPM formats).
 */
#ifndef  TEDECODERPAM_H
#define  TEDECODERPAM_H

#include "../../kernel/TeDecoderSmartMem.h"

class TeDecoderPAMFactory;

/** 
 * @brief Implements a decoder to read/write PAM (Portable Arbitrary Maps) 
 * image files (PBM, PGM, or PPM formats).
 * @note No interleaving support.
 * @note libnetpbm devel 10.26 linking required.
 * @note Both TeDecoderPAM.h and TeDecoderPAM.cpp must be compiled into 
 * application code.
 * @note TeDecoderPAM.h must be included into application main.cpp to
 * register the decoder with the TerraLib decoder factory.
 * @note Only TeUNSIGNEDCHAR and TeUNSIGNEDSHORT supported formats.
 */
class TeDecoderPAM : public TeDecoderSmartMem
{

public:

  //! @brief Empty constructor
  TeDecoderPAM ();

  //! @brief Constructor from parameters
  TeDecoderPAM ( const TeRasterParams& par );

  //! @brief Destructor
  virtual ~TeDecoderPAM ();

  //! @brief Initializes the internal structures of the decoder
  virtual void  init  ();

};

//! Implements a factory to build decoder to MEMORY raster
class TeDecoderPAMFactory : public TeDecoderFactory
{
public:

  //! Factory constructor
  TeDecoderPAMFactory(const string& name) : TeDecoderFactory(name) {}

  //! Builds the object
  virtual TeDecoder* build (const TeRasterParams& arg)
  {  return new TeDecoderPAM(arg); }
};

bool registerPAMDecoder();

namespace {
  static bool register_result = registerPAMDecoder();
};  
  
#endif

