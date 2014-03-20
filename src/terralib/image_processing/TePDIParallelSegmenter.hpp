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

#ifndef TEPDIPARALELLSEGMENTER_HPP
  #define TEPDIPARALELLSEGMENTER_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIMatrix.hpp"
  #include "TePDIParaSegSegmentsBlock.hpp"
  #include "TePDIParaSegSegment.hpp"
  #include "TePDIParaSegStrategy.hpp"
  
  #include "../kernel/TeMutex.h"
  #include "../kernel/TeThreadSignal.h"

  /*!
      \class TePDIParallelSegmenter
      \brief Multi-threaded image segmenter.
      \details This class acts just like a threaded engine that splits the 
      input image into small blocks to be  processed by the choosen 
      segmentation strategy. All blocks are segmented independently and the 
      result will be merged (if possible) at the end.
      \author Emiliano F. Castejon <castejon@dpi.inpe.br>.
      \ingroup TePDISegmentationGroup
     
      \note The required parameters are:
     
      \param input_image_ptr (TePDITypes::TePDIRasterPtrType) - 
      The input image pointer.
      \param input_channels ( std::vector< unsigned int > ) - Bands/channels
      to process from input_image.
      \param strategy_name (std::string) - Parallel segmenter strategy name
      (see each TePDIParaSegStrategy inherited classes
      documentation reference).
      \param output_image_ptr (TePDITypes::TePDIRasterPtrType) - 
      The output labeled image.      
     
      \note The optional parameters are:
      
      \param strategy_params (TePDIParameters) - Segmenter strategy specific
      parameters (see each TePDIParaSegStrategy inherited classes
      documentation reference).
      \param max_block_size (unsigned int) - The input image will be
      split into blocks with this for processing, this parameter
      tells the maximum block lateral size (width/height), the default: 
      automatic width following the current system resources and physical processors
      number).
      \param max_seg_threads (unsigned int) - The maximum number of concurrent
      segmenter threads (default:the number of physical processors,
      minimum:0 - multi-thread disabled).    
      \param merge_adjacent_blocks (bool) - All image adjacent blocks boder 
      segments will be merged, when possible (default:true)
      
      \note Since this algorithm uses multi-thread processing the user must be 
      aware that some image decoders do not offer support for multi-threaded 
      processing.
      
      \example TePDIParallelSegmenter_test.cpp Shows how to use this class.
  */
  class PDI_DLL TePDIParallelSegmenter : public TePDIAlgorithm 
  {
    public :
    
      TePDIParallelSegmenter();

      ~TePDIParallelSegmenter();

      // Overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;

    protected :
    
      /*! Segments blocks matrix type definition */
      typedef TePDIMatrix< TePDIParaSegSegmentsBlock > SegmentsBlocksMatrixT;    
    
      /*! 
        \brief The parameters passed to the segmenterThreadEntry method.
      */     
      class SegmenterThreadEntryParams
      {
        public :
          /*! Pointer to the input raster (default:0)*/
          TeRaster* inputRasterPtr_;      
          
          /*! Pointer to the input raster used channels vector (default:0)*/
          std::vector< unsigned int > const* inputChannelsVecPtr_;
        
          /*! Pointer to the segments blocks matrix (default:0)*/
          SegmentsBlocksMatrixT* segsBlocksMatrixPtr_;
          
          /*! Pointer to the global mutex (default:0)*/
          TeMutex* mutexPtr_;
          
          /*! Pointer to the segmenter strategy parameters (default:0)*/
          TePDIParameters const* stratParamsPtr_;
          
          /*! Pointer to the segmenter strategy name (default:0)*/
          std::string const* stratNamePtr_;
          
          /*! Pointer to the abort segmentation flag (default:0)*/
          bool volatile* abortSegmentationFlagPtr_;
          
          /*! Pointer to the segments Ids counter - (must be used with 
          mutexPtr_ - default 0) */
          TePDIParaSegSegment::SegIdDataType volatile* segmentsIdsCounterPtr_;
          
          /*! A signal to be emited when a segments block was processed (default:0)*/
          TeThreadSignal* signalPtr_;
          
          /*! Progress interface enabled/disabled flag (default:false). */ 
          bool progressEnabled_;  
                  
          /*! Merge adjacent blocks border segments (default:false). */ 
          bool mergeAdjacentBlocks_;                  
          
          SegmenterThreadEntryParams()
          {
            inputRasterPtr_ = 0;
            inputChannelsVecPtr_ = 0;
            segsBlocksMatrixPtr_ = 0;
            mutexPtr_ = 0;
            stratParamsPtr_ = 0;
            stratNamePtr_ = 0;
            abortSegmentationFlagPtr_ = 0;
            segmentsIdsCounterPtr_ = 0;
            signalPtr_ = 0;
            progressEnabled_ = false;
            mergeAdjacentBlocks_ = false;
          }
          
          ~SegmenterThreadEntryParams() {};
      };
      
      /*! 
        \brief The parameters passed to the groupDataThreadEntry method.
      */     
      class GroupDataThreadEntryParams
      {
        public :
        
          /*! Pointer to the pre-initiated output raster */
          TeRaster* outputRasterPtr_;    
                
          /*! Pointer to the segments blocks matrix */
          SegmentsBlocksMatrixT* segsBlocksMatrixPtr_;
          
          /*! Pointer to the global mutex */
          TeMutex* mutexPtr_;
          
          /*! Pointer to the segmenter strategy parameters */
          TePDIParameters const* stratParamsPtr_;
          
          /*! Pointer to the segmenter strategy name */
          std::string const* stratNamePtr_;
          
          /*! Pointer to the abort segmentation flag */
          bool volatile* abortSegmentationFlagPtr_;
          
          /*! A signal to be listen when a segments block was processed 
          or errors aborting the segmentation process (default:0)*/
          TeThreadSignal* signalPtr_;      
              
          /*! Pointer to the segments Ids counter - (must be used with 
          mutexPtr_ - default 0) */
          TePDIParaSegSegment::SegIdDataType volatile* segmentsIdsCounterPtr_;
          
          /*! If true, the thread will wait for signals emitted by signalPtr_
          (default true) */          
          bool waitForSignals_;
          
          /*! Progress interface enabled/disabled flag (default:false). */ 
          bool progressEnabled_;         
                   
          /*! Merge adjacent blocks border segments (default:false). */ 
          bool mergeAdjacentBlocks_;
          
          GroupDataThreadEntryParams()
          {
            outputRasterPtr_ = 0;
            segsBlocksMatrixPtr_ = 0;
            mutexPtr_ = 0;
            stratParamsPtr_ = 0;
            stratNamePtr_ = 0;
            abortSegmentationFlagPtr_ = 0;
            signalPtr_ = 0;
            segmentsIdsCounterPtr_ = 0;
            waitForSignals_ = true;
            progressEnabled_ = false;
            mergeAdjacentBlocks_ = false;
          }
          
          ~GroupDataThreadEntryParams() {};
      };      
      
      // Overloaded
      bool RunImplementation();

      // Overloaded
      void ResetState( const TePDIParameters& params );
      
      /*! 
        \brief Segmenter thread entry.
        \param paramsPtr A pointer to a casted SegmenterThreadEntryParams.
        \return true if OK, false on errors.
      */      
      static bool segmenterThreadEntry(void* paramsPtr);
      
      /*! 
        \brief Group segmented data thread entry.
        \param paramsPtr A pointer to a casted GroupDataThreadEntryParams.
        \return true if OK, false on errors.
      */      
      static bool groupDataThreadEntry(void* paramsPtr);      
        
      /*! 
        \brief Flush block to output raster.
        \param currBlock The current block that will be flushed.
        \param outRaster The output raster.
        \param progress Progress interface instance to increment.
        \return true if OK, false on errors.
        \note The block unused memory will be freed as well segments
        with no more references left.
      */      
      static bool flushBlock( TePDIParaSegSegmentsBlock& currBlock,
        TeRaster& outRaster, TePDIPIManager& progress ); 
               
      /*! 
        \brief Calc the best sub-image block size for each thread to
        process.
        \param totalImageLines The total original full image lines.
        \param totalImageCols The total original full image columns.
        \param minBlockPixels The minimun allowed pixels number for each block.
        \param maxBlockPixels The maximum allowed pixels number for each block.
        \param blockWidth The calculated block width.
        \param blockHeight The calculated block height.
        \return true if OK, false on errors.
      */                
      bool calcBestBlockSize( const unsigned int totalImageLines, 
        const unsigned int totalImageCols, const unsigned int minBlockPixels,
        const unsigned int maxBlockPixels, unsigned int& blockWidth,
        unsigned int& blockHeight ) const;
      
  };


#endif
