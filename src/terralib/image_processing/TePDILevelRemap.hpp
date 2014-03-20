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

#ifndef TEPDILEVELREMAP_HPP
  #define TEPDILEVELREMAP_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIHistogram.hpp"
  #include "TePDIParameters.hpp"
  #include "../kernel/TeSharedPtr.h"
  #include "TePDITypes.hpp"

  /**
   * @brief This is the base class for level remapping algorithms.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIGeneralAlgoGroup
   */
  class PDI_DLL TePDILevelRemap : public TePDIAlgorithm {
    public :
      /** @typedef Type definition for a instance pointer */
      typedef TeSharedPtr< TePDILevelRemap > pointer;
      /** @typedef Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDILevelRemap > const_pointer;

      /**
       * @brief Default Destructor
       */
      virtual ~TePDILevelRemap();

    protected :
      /**
       * @brief A level remap function pointer ( 3 arguments ).
       *
       * @param double Level to be remapped.
       * @param double Gain.
       * @param double Offset.
       * @return The remapped level.
       */
      typedef double (*remap_func_3_ptr_type)( double, double, double );

      /**
       * @brief A level remap function pointer ( 4 arguments ).
       *
       * @param double Input Level 1 ( from image 1 ) to be remapped.
       * @param double Input Level 2 ( from image 2 ) to be remapped.
       * @param double Gain.
       * @param double Offset.
       * @return The remapped level.
       */
      typedef double (*remap_func_4_ptr_type)( double, double, double,
        double );

      /**
       * @brief A cache of level ordered histograms to avoid rebuilding
       * histograms all the times.
       *
       * @param TeRaster A raster naked pointer reference.
       * @param unsigned int Raster band.
       * @param TePDIHistogram::pointer A Histogram shared pointer reference.
       */
      std::map< std::pair< TeRaster*, unsigned int  >,
                TePDIHistogram::pointer > histo_cache_;

      /**
       * @brief Default Constructor.
       *
       */
      TePDILevelRemap();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      virtual void ResetState( const TePDIParameters& params );

      /**
       * @brief Do level remapping from one raster to another.
       *
       * @param inRaster The input Raster.
       * @param outRaster The Output Raster.
       * @param remapp_func The level remapping function pointer.
       * @param in_channel in_raster channel to remap.
       * @param out_channel out_raster channel to write on.
       * @param gain Level Gain.
       * @param offset Level Offset.
       * @param normalize_output Do output normalization following the output
       * raster channel allowed values range.
       * @return true if ok, false on errors.
       */
      bool RemapLevels(
        TePDITypes::TePDIRasterPtrType& inRaster,
        remap_func_3_ptr_type remapp_func,
        int in_channel,
        int out_channel,
        double gain,
        double offset,
        bool normalize_output,
        TePDITypes::TePDIRasterPtrType& outRaster );

      /**
       * @brief Do level remapping from two input rasters to another.
       *
       * @param inRaster1 The input Raster 1.
       * @param inRaster2 The input Raster 2.
       * @param outRaster The Output Raster.
       * @param remapp_func The level remapping function pointer.
       * @param in_channel1 in_raster1 channel to remap.
       * @param in_channel2 in_raster2 channel to remap.
       * @param out_channel out_raster channel to write on.
       * @param gain Level Gain.
       * @param offset Level Offset.
       * @param normalize_output Do output normalization following the output
       * raster channel allowed values range
       * @return true if ok, false on errors.
       */
      bool RemapLevels(
        TePDITypes::TePDIRasterPtrType& inRaster1,
        TePDITypes::TePDIRasterPtrType& inRaster2,
        remap_func_4_ptr_type remapp_func,
        int in_channel1,
        int in_channel2,
        int out_channel,
        double gain,
        double offset,
        bool normalize_output,
        TePDITypes::TePDIRasterPtrType& outRaster );

      /**
       * @brief Build level ordered histograms from the required raster channels.
       *
       * @note The result histograms will remain inside the protected
       * object histo_cache_
       *
       * @param inRaster The input Raster.
       * @param histo_levels Number of histogram interpolated levels.
       * @param channels Channels to use for histogram generation.
       * @param force Force histogram regeneratin even if already in cache.
       */
      void BuildHistograms(
        TePDITypes::TePDIRasterPtrType& inRaster,
        unsigned int histo_levels,
        std::vector< int >& channels,
        bool force = false );

      /**
       * @brief Returns the histogram reference stored inside the histogram cache.
       *
       * @note The result histograms will remain inside the protected
       * object histo_cache_
       *
       * @param inRaster The input Raster.
       * @param band Band number.
       * @return The histogram reference.
       */
      TePDIHistogram::pointer GetHistRef(
        TePDITypes::TePDIRasterPtrType& inRaster,
        unsigned int band );

  };

#endif //TEPDILEVELREMAP_HPP
