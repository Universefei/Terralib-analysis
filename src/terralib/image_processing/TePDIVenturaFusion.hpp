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

#ifndef TEPDIVENTURAFUSION_HPP
  #define TEPDIVENTURAFUSION_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIMatrix.hpp"

  /**
   * @class TePDIVenturaFusion 
   * @brief This is the class for Ventura fusion.
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
   * @note The reference_raster lines number must be a multiple
   * of two multiplied by lowres_raster lines number.
   * @note The reference_raster columns number must be a multiple
   * of two multiplied by lowres_raster columns number.
   * @note This algorithm is base on the work: Fernando N. Ventura, 
   * Leila M. G. Fonseca. Remotely Sensed Image Fusion Using The Wavelet 
   * Transform. International Symposium on Remote Sensing of Environment. 
   * Buenos Aires, Argentina. 2002.
   *
   * @example TePDIFusion_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIVenturaFusion : public TePDIAlgorithm
  {
    public :
      
      TePDIVenturaFusion();      

      ~TePDIVenturaFusion();

      //Overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;   
            
    protected :
    
      //Overloaded
      void ResetState( const TePDIParameters& params );    
     
      //Overloaded
      bool RunImplementation();      
  
      /**
       * @brief Extracts the band 0 from the raster into the matrix.
       *
       * @param raster Input raster.
       * @param matrix Input matrix.
       * @return true if ok, valse on errors.
       */      
      bool RasterBand2Matrix( const TePDITypes::TePDIRasterPtrType& raster,
        TePDIMatrix< double >& matrix );
        
      /**
      * @brief Find the best decimation levels bringging the two rasters to
      * closer sizes.
      *
      * @param raster1 Input raster1.
      * @param raster2 Input raster2.
      * @param r1_level Raster 1 best decomposition level.
      * @param r2_level Raster 2 best decomposition level.
      * @param decim_error The error ( pixels number ) for the best level found.
      * @return true if OK. false on errors.
      */
      bool findBestDecimLevels( 
        const TePDITypes::TePDIRasterPtrType& raster1,
        const TePDITypes::TePDIRasterPtrType& raster2,
        unsigned long int& r1_level, unsigned long int& r2_level,
        unsigned long int& decim_error );    
  };

#endif
