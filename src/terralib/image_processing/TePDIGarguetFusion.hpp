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

#ifndef TEPDIGARGUETFUSION_HPP
  #define TEPDIGARGUETFUSION_HPP

  #include "TePDIAlgorithm.hpp"

  /**
   * @class TePDIGarguetFusion
   * @brief This is the class for garguet fusion.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIFusionAlgorithms
   *
   * @note The required parameters are:
   * @param reference_raster ( TePDITypes::TePDIRasterPtrType ) - 
   *  Reference raster ( higher quality ).
   * @param lowres_raster ( TePDITypes::TePDIRasterPtrType ) - 
   *  Low resolution raster ( will be improved ).
   * @param output_raster ( TePDITypes::TePDIRasterPtrType ) - Output raster.
   * @param reference_raster_band ( int ) - The band to process for 
   * reference_raster.
   * @param lowres_raster_band ( int ) - The band to process for lowres_raster.
   *
   * @note The following parameters are optional, and will be used if present:
   *
   * @param filters_scale ( double ) - The wavelet filters scale.
   * @param a_filter_l ( std::vector< double > ) - Wavelet Analysis filter, 
   * low pass, non
   * normalized ( used in decomposition ).
   * @param a_filter_h ( std::vector< double > ) - Wavelet Analysis filter, 
   * high pass, non
   * normalized ( used in decomposition ).
   * @param s_filter_l ( std::vector< double > ) - Wavelet Synthesis filter, 
   * low pass, non
   * normalized ( used in recomposition ).
   * @param s_filter_h ( std::vector< double > ) - Wavelet Synthesis filter, 
   * high pass, non
   * normalized ( used in recomposition ). 
   *
   * @example TePDIFusion_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIGarguetFusion : public TePDIAlgorithm
  {
    public :
      
      TePDIGarguetFusion();      

      ~TePDIGarguetFusion();

      //Overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;   
            
    protected :
    
      //Overloaded
      void ResetState( const TePDIParameters& params );    
     
      //Overloaded
      bool RunImplementation();      
  
      /**
      * @brief Find the best reference_raster decimation level 
      * that comes close to lowres_raster.
      *
      * @param reference_raster Reference raster.
      * @param lowres_raster Low resolution raster.
      * @param rr_level Reference raster best decomposition level.
      * @param rr_level_width Reference raster best decomposition level
      * width (starting from level 1 - at least 1 decimation
      * done)
      * @param rr_level_height Reference raster best decomposition level
      * height.      
      * @return true if OK. false on errors.
      */
      bool findBestDecimLevel( 
        const TePDITypes::TePDIRasterPtrType& reference_raster,
        const TePDITypes::TePDIRasterPtrType& lowres_raster,
        unsigned int& rr_level, 
        unsigned int& rr_level_width,
        unsigned int& rr_level_height );    
  };

#endif
