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

#ifndef TEPDIHANTS_HPP
  #define TEPDIHANTS_HPP

  #include "TePDIAlgorithm.hpp"

  /**
   * @brief This is the class for Harmonic Analysis of NDVI Time-Series (HANTS). 
   * Given the input raster with NDVI values (nb = time-series size), it computes its corresponding smoothed NDVI values in the output raster.
   * Note: The original program was developed in IDL-ENVI by Allard de Wit (http://www.ittvis.com/language/en-US/UserCommunity/CodeLibrary/id/1166.aspx) 
   * and modified by Julio Cesar D. M. Esquerdo (julio@cnptia.embrapa.br).
   * Requirements: FFTW library for fast Fourier transform.
   * @author Alexandre Noma <alex.noma@gmail.com>
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIGeneralAlgoGroup
   *
   * @note The general required parameters:
   *
   * @param input_raster ( TePDITypes::TePDIRasterPtrType ) - The input data with NDVI values, where the number of bands corresponds to the time-series size.
   * @param output_raster ( TePDITypes::TePDIRasterPtrType ) - The output data with the smoothed NDVI values.
   * @param mask_raster ( TePDITypes::TePDIRasterPtrType ) - Mask for the input data. 0: do not perform algorithm on the current pixel.
   * @param freqs (int[]) - 1D array with frequences that should be selected from the fourier spectrum. i.e. freqs[]={0,1,2,3} to use the fourier compoments 0 (mean), 1 (1 sine wave), 2 (2 sine waves) and 3.
   * @param nfreqs (int) - number of selected frequencies.
   * @param range (double[]) - Array of size 2 to specify the minimum and maximum valid data values, i.e. range[]={1000,10000}.
   * @param FET (double) - Fit Error Tolerance, is maximum tolerable downward deviation between fourier fit and NDVI data values (in DN values).
   * @param TAT (int) - Throw-Away-Threshold, maximum number of points that may be thrown away by the FFT fitting.
   * @param iMAX (int) - maximum number of iterations to be performed.
   *
   */
  class PDI_DLL TePDIHants : public TePDIAlgorithm {
    public :
      
      /**
       * Default Constructor.
       *
       */
      TePDIHants();

      /**
       * Default Destructor
       */
      ~TePDIHants();
      
      /**
       * Checks if the supplied parameters fits the requirements of each
       * PDI algorithm implementation.
       *
       * Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      bool CheckParameters( const TePDIParameters& parameters ) const;      

    protected :
    
      /**
       * Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );    
     
      /**
       * Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();      
  };
  
/** @example TePDIHants_test.cpp
 *    Shows how to use this class.
 */    

#endif
