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

#ifndef TEPDIPARASEGSTRATEGY_HPP
  #define TEPDIPARASEGSTRATEGY_HPP

  #include "TePDIParaSegStrategyParams.hpp"
  #include "TePDIParaSegSegmentsBlock.hpp"
  #include "TePDIParaSegStrategyFactoryParams.hpp"
  #include "TePDIMatrix.hpp"
  #include "TePDIDefines.hpp"
  #include "TePDIParameters.hpp"
  
  #include <vector>

  /*!
      \class TePDIParaSegStrategy
      \brief Multi-threaded image segmenter strategy base class.
      \author Emiliano F. Castejon <castejon@dpi.inpe.br>.
      \ingroup PDIStrategies
  */
  class PDI_DLL TePDIParaSegStrategy
  {
    public :
        

      /*! Raster data block (one band/channel)*/
      typedef TePDIMatrix< double > RasterBandDataT;          
         
      /*! Raster data block type definition (one vector element for each
      raster band)*/
      typedef std::vector< RasterBandDataT > RasterDataVecT;           
    
      virtual ~TePDIParaSegStrategy();

      /*!
          \brief Execute segmentation following the supplied strategy
          specific parameters.
          \details This method will be called many times as required
          to the same strategy instance to execute segmentation on each
          image block.
          \param rasterDataVector The input raster data vector (each
          vector element represents one raster band/channel).
          \param outSegsMatrix The generated output segments matrix.
          \return true if OK, false on errors.
          \note The generated segments IDs must be acquired by using the
           TePDIParaSegStrategy::acquireNewSegID() method.
          \note The generated segments reference counters must be initiated
           with value 1.           
      */ 
      virtual bool execute( const RasterDataVecT& rasterDataVector, 
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& outSegsMatrix ) = 0 ;
        
      /*!
          \brief Set the specific strategy parameters.
          \details This method will be called once for each strategy
          instance created.
          \param params A reference to the new parameters.
          \return true if OK, false on errors.
      */         
      virtual bool setParameters( const TePDIParameters& params ) = 0;        
        
      /*!
          \brief Merge adjacent matrix border segments.
          \details This method will be called many times as required to the 
          same strategy instance to merge the entire label image  
          incrementally; on each call references to 3 adjacent image segments 
          blocks will be passed as shown ahead.
          \code
          ________________________________
          |     |     |      |     |     |
          ________________________________
          |     |     |top   |     |     |
          ________________________________
          |     |left |center|     |     |          
          _______________________________
          |     |     |      |     |     |          
          ________________________________          
          \endcode
          \param centerMatrix A reference to the image block matrix located
          at the merging region center.
          \param topMatrixPtr A pointer to the image block matrix located
          at the merging region top or null if this block does not exists.          
          \param leftMatrixPtr A pointer to the image block matrix located
          at the merging region left or null if this block does not exists.          
          \return true if OK, false on errors.
      */         
      virtual bool mergeSegments( 
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& centerMatrix,
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* topMatrixPtr,
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* leftMatrixPtr ) = 0;
        
      /*!
          \brief Returns a positive memory use weight factor.
          \details This parameter is used by the parallel segmenter to
          guess the sub-image blocks size.
          \return A memory use weight factor (default:1,0).
      */          
      virtual double getMemUsageFactor() const = 0;
        
      /*!
          \brief Acquire an unique segment ID to be associated with a new
          generated segment.
          \return An unique segment ID to be associated with a new
          generated segment or zero if an error occurred.
          \note The returned ID is unique, meaning that other calls
          to this method (by this or by other threads) will never generate
          the same values.
      */         
      TePDIParaSegSegment::SegIdDataType acquireNewSegID();
      
      /*!
          \brief Return true if adjacent segments blocks merging
          will be performed.
          \return Return true if adjacent segments blocks merging
          will be performed.
      */      
      inline bool isBlockMergingEnabled() const
      {
        return params_.blockMergingEnabled_;
      };      
      
      // Overloaded
      static TePDIParaSegStrategy* DefaultObject( 
        const TePDIParaSegStrategyFactoryParams& params );
        
    protected :
    
      /*!
          \brief The default constructor.
          \param params The strategy parameters.
      */    
      TePDIParaSegStrategy( const TePDIParaSegStrategyParams& params );    
             
    private :
    
      /*! The last generated segment ID (default:0). */
      volatile TePDIParaSegSegment::SegIdDataType lastGenSegID_;    
      
      /*! This strategy instance parameters. */
      TePDIParaSegStrategyParams params_;
      
  };


#endif
