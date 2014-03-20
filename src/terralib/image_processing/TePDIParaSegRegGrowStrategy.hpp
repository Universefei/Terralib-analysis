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

#ifndef TEPDIPARASEGREGGROWSTRATEGY_HPP
  #define TEPDIPARASEGREGGROWSTRATEGY_HPP

  #include "TePDIParaSegStrategy.hpp"
  #include "TePDIParaSegSegment.hpp"
  #include "TePDIRegGrowSeg.hpp"
  
  #include <vector>

  /*!
      \class TePDIParaSegRegGrowStrategy
      \brief Multi-threaded image segmenter Region Growing strategy.
      \author Emiliano F. Castejon <castejon@dpi.inpe.br>.
      \ingroup PDIStrategies
      \note The strategy name that must be used to construct an instance
      of this class is "RegionGrowing".
      
      \note The general required parameters :
      \param euc_treshold (double) - Maximum eclidean distance between each
      segment (This parameter is used to merge adjacent segments from
      adjacent image blocks).
      \param area_min (int) - cell pixel size min value.      
  */
  class PDI_DLL TePDIParaSegRegGrowStrategy : public TePDIParaSegStrategy
  {
    public :
    
      /*!
          \class Segment
          \brief Information about one segment.
          \author Emiliano F. Castejon <castejon@dpi.inpe.br>.
          \ingroup PDIAux
      */
      class PDI_DLL Segment : public TePDIParaSegSegment
      {
        public :
        
          /*! Raster bands means values for this segment (default:empty 
          vector). */ 
          std::vector< double > bandsMeansVec_;
          
          /*! The number of pixels inside this region (default:0). */ 
          double pixelsNumber_;       
             
          /*! True if this segment was merged with another and the current
          segment Id corresponds to the merging result. (default:false)*/
          bool wasMerged_;             
        
          Segment();
          
          ~Segment();
      };    
    
      // This method has the same parameters as the 
      // TePDIParaSegStrategy constructor
      TePDIParaSegRegGrowStrategy( const TePDIParaSegStrategyParams& params );

      ~TePDIParaSegRegGrowStrategy();

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
          
      // just as the mergeSegments above
      static bool staticMergeSegments( 
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& centerMatrix,
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* topMatrixPtr,
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* leftMatrixPtr,
        double eucTreshold );  
                
      /*!
        \brief Create a segments matrix from a label image.
        \param rasterDataVector The original input raster data vector.
        \param labelRaster The label image raster.
        \param stratPtr A pointer to the strategy calling this method..
        \param outSegsMatrix A reference to the output generated segments
        matrix.
        \return true if OK, false on errors.
        \note This method is templated by the strategy type inherithed
        from TePDIParaSegStrategy.
      */                
      template< class StrategyTemplateT >
      static bool createSegsBlkFromLImg( const RasterDataVecT& rasterDataVector,
        TeRaster& labelRaster, StrategyTemplateT* stratPtr,
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& outSegsMatrix );
        
      //overloaded
      double getMemUsageFactor() const { return 1.0; };

    protected :
      
      /*!
          \class MergingSegmentInfo
          \brief Information about a candidate segment for merging.
          \ingroup PDIAux
      */          
      class PDI_DLL MergingSegmentInfo
      {
        public:
        
        /*! The candidate segment pointer* (default:0) */
        TePDIParaSegSegment const* segPtr_;
        
        /*! The candidate segment matrix pointer (default:0) */
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT const* mtxPtr_;
        
        /*! The lenght of the joint border (pixels number) (default:0) */
        unsigned int jointBorderLenght_;
        
        MergingSegmentInfo();
        
        ~MergingSegmentInfo();
      
      };      
      
      /*!
          \brief Type definition for a vector of canditate merging segments
          information vector.
      */     
      typedef std::vector< MergingSegmentInfo >  MergingSegmentInfoVecT;       
      
      /*! A raster generated from the input raster data vector. */      
      TePDITypes::TePDIRasterPtrType inputRasterPtr_;
      
      /*! A raster generated from the segmentation. */      
      TePDITypes::TePDIRasterPtrType outputRasterPtr_;
      
      /*! Segmentation algorithm parameters */      
      TePDIParameters segParams_;
      
      /*! Segmenter instance */
      TePDIRegGrowSeg segmenter_;
      
      /*! Maximum allowed euclidean distance parameter */
      double eucTreshold_;
      
      /*!
        \brief Locate, among the given candidate segments, those wich
        can be merged with the segment inputSegPtr.
        \param inputSegPtr A pointer to the segment to merge with the located candidate.
        \param inputSegMtxPtr Input segment matrix pointer.
        \param candidateSegmentsVec The candidate segments information
        vector.
        \param locatedSegsIndexes The selected candidate segment indexes
        inside candidateSegmentsVec ordered from the best candidate (first
        locatedSegInfoIndexes vector element ) to the worse canditate (last
        locatedSegInfoIndexes vector element).
        \param eucTreshold Max euclidean distance between each segment.
        \return true if one cadidate was located, false if no candidate
        was located.
      */
      static bool staticLocateMergingSegments( Segment const* inputSegPtr,
        TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT const* inputSegMtxPtr,
        const MergingSegmentInfoVecT& candidateSegmentsVec,
        std::vector< unsigned int >& locatedSegsIndexes, double eucTreshold );      

  };
    
template< class StrategyTemplateT >    
bool TePDIParaSegRegGrowStrategy::createSegsBlkFromLImg( 
  const RasterDataVecT& rasterDataVector, TeRaster& labelRaster,
  StrategyTemplateT* stratPtr,
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& outSegsMatrix )
{

  const unsigned int nLines = (unsigned int)labelRaster.params().nlines_;
  const unsigned int nCols = (unsigned int)labelRaster.params().ncols_;      
  const unsigned int nBands = (unsigned int)labelRaster.params().nBands(); 
  
  // Initiating the output matrix
  
  TEAGN_TRUE_OR_RETURN( outSegsMatrix.Reset( rasterDataVector[ 0 ].GetLines(),
    rasterDataVector[ 0 ].GetColumns() ),
    "output segments matrix reset error " );  
  
  unsigned int band = 0;
  unsigned int line = 0;
  unsigned int col = 0;
  TePDIParaSegSegment** segsMtxLinePtr = 0;
  TeDecoder& labelRasterDecoder = *(labelRaster.decoder());
  double value = 0;
  Segment* segPtr = 0;
  Segment* borderSegPtr = 0;
  const unsigned int lastLineIdx = nLines - 1;
  const unsigned int lastColIdx = nCols - 1;
  double oldSegPixelsNmb = 0;
  double newBandMeanValue = 0;
  std::map< double, Segment* > allSegsMap;
  std::map< double, Segment* > borderSegsMap;
  bool blockMergingEnabled = stratPtr->isBlockMergingEnabled();

  for( line = 0 ; line < nLines ; ++line )
  {
    segsMtxLinePtr = outSegsMatrix[ line ];
  
    for( col = 0 ; col < nCols ; ++col )
    {
      TEAGN_TRUE_OR_THROW( labelRasterDecoder.getElement( col, line, 
        value, 0 ), "Error writing input raster" )
      
      // Does this segment already was created ??
        
      segPtr = allSegsMap[ value ];
      if( segPtr == 0 )
      {
        segPtr = new Segment;
        segPtr->id_ = stratPtr->acquireNewSegID();
        
        allSegsMap[ value ] = segPtr;
      }
      
      segsMtxLinePtr[ col ] = segPtr;
      
      // Is this a border segment ??
      
      if( blockMergingEnabled && ( ( line == 0 ) || ( line == lastLineIdx ) || ( col == 0 ) || 
        ( col == lastColIdx ) ) )
      {
        borderSegPtr = borderSegsMap[ value ];
        
        if( borderSegPtr == 0 )
        {
          borderSegsMap[ value ] = segPtr;
          borderSegPtr = segPtr;
          
          borderSegPtr->pixelsNumber_ = 1.0;
          
          borderSegPtr->bandsMeansVec_.resize( nBands );
          for( band = 0 ; band < nBands ; ++band )
          {
            borderSegPtr->bandsMeansVec_[ band ] = rasterDataVector[ 
              band ][ line ][ col ];
          }
        }
        else
        {
          oldSegPixelsNmb = borderSegPtr->pixelsNumber_;
          ++(borderSegPtr->pixelsNumber_);
          
          for( band = 0 ; band < nBands ; ++band )
          {
            newBandMeanValue = ( borderSegPtr->bandsMeansVec_[ band ] * 
              oldSegPixelsNmb );
            newBandMeanValue += rasterDataVector[ band ][ line ][ col ];
            newBandMeanValue /= borderSegPtr->pixelsNumber_;
          
            borderSegPtr->bandsMeansVec_[ band ] = newBandMeanValue;
          }   
        }
      }
    }
  }
  
  return true;
}    
    
#endif
