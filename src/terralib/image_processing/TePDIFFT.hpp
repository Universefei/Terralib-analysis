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

#ifndef TEPDIFFT_HPP
  #define TEPDIFFT_HPP

  #include "TePDIAlgorithm.hpp"

  /**
   * @brief Fast Fourier Transform.
   * Given an input matrix (m x 2n), it outputs another matrix (m x 2n) with its FFT.
   * Notation: input/output[i][2*j] = real term of complex number; input/output[i][2*j+1] = imaginary term of complex number.
   * Requirements: 
   * * FFTW library (available at http://www.fftw.org/ ).
   * * The symbol TE_USE_FFTW must be defined (use the global configuration file (terralib/kernel/Te********PlatformDefines.h).
   * * For windows developers: Update project files to link against the FFTW library.
   *
   * @author Alexandre Noma <alex.noma@gmail.com>
   * @ingroup TePDIGeneralAlgoGroup
   *
   * @note The general required parameters:
   *
   * @param input_raster ( TePDITypes::TePDIRasterPtrType ) - The input data. Channel 0 is used for real part, while channel 1 is used for imaginary part of a complex number.
   * @param output_raster ( TePDITypes::TePDIRasterPtrType ) - The output data.
   *
   * @note Optional parameters:
   *
   * @param fft_ifft_flag (+1 or -1) - Default +1 for FFT. To execute IFFT, set this variable to -1.
   *
   * @example TePDIFFT_test.cpp Shows how to use this class.
   *
   */
  class PDI_DLL TePDIFFT : public TePDIAlgorithm {
    public :
      
      TePDIFFT();

      ~TePDIFFT();
      
      //overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;      

    protected :
    
      //overloaded
      void ResetState( const TePDIParameters& params );    
     
      //overloaded
      bool RunImplementation();      
  };

#endif
