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

#ifndef TEPDIPARASEGBAATZSTRATEGY_HPP
  #define TEPDIPARASEGBAATZSTRATEGY_HPP

  #include "TePDIParaSegStrategy.hpp"
  #include "TePDIBaatz.hpp"
  
  #include <vector>

  /*!
      \class TePDIParaSegBaatzStrategy
      \brief Multi-threaded image segmenter Region Growing strategy.
      \author Thales Sehn Korting <tkorting@dpi.inpe.br>.
      \ingroup PDIStrategies
      \note The strategy name that must be used to construct an instance
      of this class is "Baatz".
      
      \note The general required parameters :
      \param scale (float) Parameter scale is > 0 (Has direct influence
      over the generated segments size).
      \param compactness (float) Parameter compactness is > 0 and <= 1 (
      compactness = perimeter / sqrt( area ) ).
      \param color (float) Parameter color is > 0 and <= 1 (the relative ratio
      between spacial components and spectral components).
      \param input_weights (std::vector<float>) one weight for each input image channel
      \param euc_treshold (double) - Maximum eclidean distance between each
      segment (This parameter is used to merge adjacent segments from
      adjacent image blocks).
*/
  class PDI_DLL TePDIParaSegBaatzStrategy : public TePDIParaSegStrategy
  {
    public :
         
      // This method has the same parameters as the 
      // TePDIParaSegStrategy constructor
      TePDIParaSegBaatzStrategy( const TePDIParaSegStrategyParams& params );
  
      ~TePDIParaSegBaatzStrategy();
      
      // overloaded
      bool execute( const RasterDataVecT& rasterDataVector, 
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& outSegsMatrix );      
        
      //overloaded
      bool setParameters( const TePDIParameters& params );         
    
      //overloaded
      bool mergeSegments( 
       TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& centerMatrix,
       TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* topMatrixPtr,
       TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* leftMatrixPtr ); 
       
      //overloaded
      double getMemUsageFactor() const { return 1.5; };
      
    protected :
      
      /*! A raster generated from the input raster data vector. */      
      TePDITypes::TePDIRasterPtrType inputRasterPtr_;
      
      /*! A raster generated from the segmentation. */      
      TePDITypes::TePDIRasterPtrType outputRasterPtr_;
      
      /*! Segmentation algorithm parameters */      
      TePDIParameters segParams_;
      
      /*! Segmenter instance */
      TePDIBaatz segmenter_;
      
      /*! Maximum allowed euclidean distance parameter */
      double eucTreshold_;

  };
  
#endif
