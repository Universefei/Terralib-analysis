#include "TePDIParaSegRegGrowStrategy.hpp"

#include "TePDIUtils.hpp"

#include "../kernel/TeAgnostic.h"

#include <limits.h>

#include <map>

TePDIParaSegRegGrowStrategy::MergingSegmentInfo::MergingSegmentInfo()
: segPtr_( 0 ), mtxPtr_( 0 ), jointBorderLenght_( 0 )
{
}

TePDIParaSegRegGrowStrategy::MergingSegmentInfo::~MergingSegmentInfo()
{
}

TePDIParaSegRegGrowStrategy::TePDIParaSegRegGrowStrategy( 
  const TePDIParaSegStrategyParams& params )
  : TePDIParaSegStrategy( params ), eucTreshold_( 0 )
{
  segmenter_.ToggleProgInt( false );
}


TePDIParaSegRegGrowStrategy::~TePDIParaSegRegGrowStrategy()
{
}

bool TePDIParaSegRegGrowStrategy::execute( const RasterDataVecT& 
  rasterDataVector, TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& 
  outSegsMatrix )
{
  TEAGN_DEBUG_CONDITION( rasterDataVector.size(), "Empty data vector" )
  
  // Globals 
  
  const unsigned int nLines = rasterDataVector[ 0 ].GetLines();
  const unsigned int nCols = rasterDataVector[ 0 ].GetColumns();      
  const unsigned int nBands = (unsigned int)rasterDataVector.size();  
  
  // Copy data from data vector to the internal raster instance
  
  {
    // Initiating the input raster
    
    if( ( ! inputRasterPtr_.isActive() ) ||
      ( inputRasterPtr_->params().nBands() != (int)nBands )
      || ( inputRasterPtr_->params().nlines_ != (int)nLines ) 
      || ( inputRasterPtr_->params().ncols_ != (int)nCols ) )
    {
      TeRasterParams inRasterParams;
      inRasterParams.nBands( nBands );
      inRasterParams.setDataType( TeDOUBLE, -1 );
      inRasterParams.setNLinesNColumns( nLines, nCols );
      
      TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( inRasterParams,
       inputRasterPtr_ ), "Error creating output raster" );
    }
    
    // Copy data
    
    unsigned int line = 0;
    unsigned int col = 0;
    unsigned int band = 0;
    double const* linePtr = 0;
    TeDecoder& inputRasterDecoder = *(inputRasterPtr_->decoder());
    
    for( band = 0 ; band < nBands ; ++band )
    {
      for( line = 0 ; line < nLines ; ++line )
      {
        linePtr = rasterDataVector[ band ][ line ];
      
        for( col = 0 ; col < nCols ; ++col )
        {
          TEAGN_TRUE_OR_RETURN( inputRasterDecoder.setElement( col, line, 
            linePtr[ col ], band ), "Error writing input raster" )
        }
      }
    }
  }
    
  // Initiating the output raster
    
  if( ! outputRasterPtr_.isActive() )
  {
    TeRasterParams outRasterParams;
    outRasterParams.nBands( 1 );
    outRasterParams.setDataType( TeUNSIGNEDLONG, -1 );
    outRasterParams.setNLinesNColumns( 1, 1 );
    
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRasterParams,
     outputRasterPtr_ ), "Error creating output raster" );
  }
  
  // Updating segmenter algorithm parameters
  
  segParams_.SetParameter( "input_image", inputRasterPtr_ );
  segParams_.SetParameter( "output_image", outputRasterPtr_ );
  
  // Running the algorithm
  
  TEAGN_TRUE_OR_RETURN( segmenter_.Apply( segParams_ ),
    "Segmentation error" );
    
  // Creating the segment objects
  
  TEAGN_TRUE_OR_RETURN( TePDIParaSegRegGrowStrategy::createSegsBlkFromLImg( 
    rasterDataVector, *outputRasterPtr_, this, outSegsMatrix  ),
    "Error creating output segments" );
  
  return true;
}

TePDIParaSegRegGrowStrategy::Segment::Segment()
: pixelsNumber_( 0 ), wasMerged_( false )
{
}

TePDIParaSegRegGrowStrategy::Segment::~Segment()
{
}

bool TePDIParaSegRegGrowStrategy::mergeSegments( 
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& centerMatrix,
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* topMatrixPtr,
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* leftMatrixPtr )
{
  return TePDIParaSegRegGrowStrategy::staticMergeSegments( 
    centerMatrix, topMatrixPtr, leftMatrixPtr, eucTreshold_ );
}

bool TePDIParaSegRegGrowStrategy::staticMergeSegments( 
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& centerMatrix,
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* topMatrixPtr,
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* leftMatrixPtr,
  double eucTreshold )
{
  // Creating the merging segments map

  // Auxiliar maps map
  // first = current block segment pointer
  // second.first = other block segment pointer
  // second.second = info related to outherBlock segments touching the
  // current block segment.
  std::map< Segment const*, std::map< Segment const*,
    TePDIParaSegRegGrowStrategy::MergingSegmentInfo > > auxMapsMap;
    
  if( topMatrixPtr )
  {
    TEAGN_DEBUG_CONDITION( centerMatrix.GetColumns() == 
      topMatrixPtr->GetColumns(), 
      "Block size mismatch" );

    std::map< Segment const*, std::map< Segment const*,
      TePDIParaSegRegGrowStrategy::MergingSegmentInfo > >::iterator mapsMapIt;
    std::map< Segment const*, 
      TePDIParaSegRegGrowStrategy::MergingSegmentInfo >::iterator mapIt;
      
    TePDIParaSegRegGrowStrategy::MergingSegmentInfo auxMergSegInfo;
      
    Segment const* currBlockSegPtr = 0;
    Segment const* otherBlockSegPtr = 0;
    
    const unsigned int topMatrixLastLineIdx = topMatrixPtr->GetLines() - 1;
    const unsigned int centerMatrixWidth = centerMatrix.GetColumns();
      
    for( unsigned int bCol = 0 ; bCol < centerMatrixWidth ; ++bCol )
    {
      currBlockSegPtr = (Segment*)centerMatrix( 0, bCol );
      otherBlockSegPtr = (Segment*)
        topMatrixPtr->operator()( topMatrixLastLineIdx, bCol );
        
      mapsMapIt = auxMapsMap.find( currBlockSegPtr );
      
      if( mapsMapIt == auxMapsMap.end() )
      {
        auxMergSegInfo.segPtr_ = otherBlockSegPtr;
        auxMergSegInfo.mtxPtr_ = topMatrixPtr;
        auxMergSegInfo.jointBorderLenght_ = 1;
        
        auxMapsMap[ currBlockSegPtr ][ otherBlockSegPtr ] = auxMergSegInfo;
      }
      else
      {
        mapIt = mapsMapIt->second.find( otherBlockSegPtr );
        
        if( mapIt == mapsMapIt->second.end() )
        {
          auxMergSegInfo.segPtr_ = otherBlockSegPtr;
          auxMergSegInfo.mtxPtr_ = topMatrixPtr;
          auxMergSegInfo.jointBorderLenght_ = 1;
          
          mapsMapIt->second.operator[]( otherBlockSegPtr ) = auxMergSegInfo;
        }
        else
        {
          ++(mapIt->second.jointBorderLenght_);
        }
      }
    }
  }
  
  if( leftMatrixPtr )
  {
    TEAGN_DEBUG_CONDITION( centerMatrix.GetLines() == 
      leftMatrixPtr->GetLines(), 
      "Block size mismatch" );
           
    std::map< Segment const*, std::map< Segment const*,
      TePDIParaSegRegGrowStrategy::MergingSegmentInfo > >::iterator mapsMapIt;
    std::map< Segment const*, 
      TePDIParaSegRegGrowStrategy::MergingSegmentInfo >::iterator mapIt;
      
    TePDIParaSegRegGrowStrategy::MergingSegmentInfo auxMergSegInfo;
      
    Segment const* currBlockSegPtr = 0;
    Segment const* otherBlockSegPtr = 0;
    
    const unsigned int leftMatrixLastColIdx = leftMatrixPtr->GetColumns() - 1;
    const unsigned int centerMatrixHeight = centerMatrix.GetLines();
      
    for( unsigned int bLine = 0 ; bLine < centerMatrixHeight ; ++bLine )
    {
      currBlockSegPtr = (Segment*)centerMatrix( bLine, 0 );
      otherBlockSegPtr = (Segment*)
        leftMatrixPtr->operator()( bLine, leftMatrixLastColIdx );
        
      mapsMapIt = auxMapsMap.find( currBlockSegPtr );
      
      if( mapsMapIt == auxMapsMap.end() )
      {
        auxMergSegInfo.segPtr_ = otherBlockSegPtr;
        auxMergSegInfo.mtxPtr_ = leftMatrixPtr;
        auxMergSegInfo.jointBorderLenght_ = 1;
        
        auxMapsMap[ currBlockSegPtr ][ otherBlockSegPtr ] = auxMergSegInfo;
      }
      else
      {
        mapIt = mapsMapIt->second.find( otherBlockSegPtr );
        
        if( mapIt == mapsMapIt->second.end() )
        {
          auxMergSegInfo.segPtr_ = otherBlockSegPtr;
          auxMergSegInfo.mtxPtr_ = leftMatrixPtr;
          auxMergSegInfo.jointBorderLenght_ = 1;
          
          mapsMapIt->second.operator[]( otherBlockSegPtr ) = auxMergSegInfo;
        }
        else
        {
          ++(mapIt->second.jointBorderLenght_);
        }
      }
    }
  }
  
  // Merging each border segment, if possible
  
  {
    std::map< Segment const*, std::map< Segment const*,
      TePDIParaSegRegGrowStrategy::MergingSegmentInfo > >::iterator mapsMapIt =
      auxMapsMap.begin();      
    std::map< Segment const*, std::map< Segment const*,
      TePDIParaSegRegGrowStrategy::MergingSegmentInfo > >::iterator mapsMapItEnd =
      auxMapsMap.end();  
    std::map< Segment const*, 
      TePDIParaSegRegGrowStrategy::MergingSegmentInfo >::iterator mapIt;
    std::map< Segment const*, 
      TePDIParaSegRegGrowStrategy::MergingSegmentInfo >::iterator mapItEnd;                
    std::vector< unsigned int > locatedSegsIndexes;
    unsigned int locatedSegsIndexesIdx = 0;
    Segment* locatedSegPtr = 0;
    Segment* currentSegPtr = 0;
            
    while( mapsMapIt != mapsMapItEnd )
    {
      TEAGN_DEBUG_CONDITION( ! mapsMapIt->first->wasMerged_,
        "Trying to merge an already merged segment" );
       
      TePDIParaSegRegGrowStrategy::MergingSegmentInfoVecT auxMSVec;
      
      mapIt = mapsMapIt->second.begin();
      mapItEnd = mapsMapIt->second.end();
      
      while( mapIt != mapItEnd )
      {
        auxMSVec.push_back( mapIt->second );
      
        ++mapIt;
      }
      
      if( staticLocateMergingSegments( mapsMapIt->first,
        &(centerMatrix), auxMSVec, locatedSegsIndexes,
        eucTreshold ) )
      {
        TEAGN_DEBUG_CONDITION( locatedSegsIndexes.size(),
          "Invalid locatedSegsIndexes size" );
          
        currentSegPtr = (Segment*)mapsMapIt->first;
        
        for( locatedSegsIndexesIdx = 0 ; locatedSegsIndexesIdx < 
          locatedSegsIndexes.size() ; ++locatedSegsIndexesIdx )
        {
          TEAGN_DEBUG_CONDITION( locatedSegsIndexes[ locatedSegsIndexesIdx ] <
            auxMSVec.size(), "invalid locatedSegsIndexesIdx" );
            
          locatedSegPtr = (Segment*)
            auxMSVec[ locatedSegsIndexes[ locatedSegsIndexesIdx ] ].segPtr_;
            
          if( ! locatedSegPtr->wasMerged_ )
          {
            locatedSegPtr->wasMerged_ = true;
            locatedSegPtr->id_ = currentSegPtr->id_;
            
            currentSegPtr->wasMerged_ = true;
          }
        }
        
        if( ! currentSegPtr->wasMerged_ )
        {
          locatedSegPtr = (Segment*)
            auxMSVec[ locatedSegsIndexes[ 0 ] ].segPtr_;        
              
          currentSegPtr->wasMerged_ = true;
          currentSegPtr->id_ = locatedSegPtr->id_;          
        }
      }      
    
      ++mapsMapIt;
    }
  }
  
  return true;
}


bool TePDIParaSegRegGrowStrategy::setParameters( 
  const TePDIParameters& params )
{
  TEAGN_TRUE_OR_RETURN( params.GetParameter( "euc_treshold", eucTreshold_),
    "Missing parameter euc_treshold" )
    
  int area_min = 0;
  TEAGN_TRUE_OR_RETURN( params.GetParameter( "area_min", area_min),
    "Missing parameter area_min" )
  
  segParams_.Clear();
  segParams_.SetParameter( "euc_treshold", eucTreshold_ );
  segParams_.SetParameter( "area_min", area_min );

  return true;
}

bool TePDIParaSegRegGrowStrategy::staticLocateMergingSegments( 
  Segment const* inputSegPtr,
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT const* ,
  const MergingSegmentInfoVecT& candidateSegmentsVec,
  std::vector< unsigned int >& locatedSegsIndexes, double eucTreshold )
{
  const Segment& inputSeg = *( inputSegPtr );
  const std::vector< double >& inputSegMeansVec = inputSeg.bandsMeansVec_;
  const unsigned int cVecSize = (unsigned int)candidateSegmentsVec.size();
  
  const unsigned int meansVecSize = (unsigned int)inputSegMeansVec.size();
  TEAGN_DEBUG_CONDITION( meansVecSize, "Invalid means vector size" );  
  
  // Locating the candidates following the max euclidean ths
  
  unsigned int meansVecIdx = 0;
  double diff = 0;
  double dist = 0;
  std::multimap< unsigned int, unsigned int > auxMap;
      
  for( unsigned int cVecIdx = 0 ; cVecIdx < cVecSize ; ++cVecIdx )
  {
    const MergingSegmentInfo& candidateSegmentInfo = 
      candidateSegmentsVec[ cVecIdx ];
    TEAGN_DEBUG_CONDITION( candidateSegmentInfo.jointBorderLenght_,
      "Invalid joint border lenght" );
  
    const std::vector< double >& candSegMeansVec = 
      ((Segment const*)candidateSegmentInfo.segPtr_)->bandsMeansVec_;
    TEAGN_DEBUG_CONDITION( meansVecSize == candSegMeansVec.size(),
      "Means vector size mismatch" );        
    
    dist = 0;
    diff = 0;
      
    for( meansVecIdx = 0 ; meansVecIdx < meansVecSize ; ++meansVecIdx )
    {
      diff += candSegMeansVec[ meansVecIdx ] - inputSegMeansVec[ meansVecIdx ];
      dist += ( diff * diff );
    }
    
    dist = sqrt( dist );
    
    if( dist <= eucTreshold )
    {
      auxMap.insert( std::pair< unsigned int, unsigned int >( 
        candidateSegmentInfo.jointBorderLenght_, cVecIdx ) );
    }
  }
  
  // Generating the ordered output vector
  
  locatedSegsIndexes.clear();
  locatedSegsIndexes.reserve( auxMap.size() );
  
  std::multimap< unsigned int, unsigned int >::reverse_iterator mapIt = 
    auxMap.rbegin();
  std::multimap< unsigned int, unsigned int >::reverse_iterator mapItEnd = 
    auxMap.rend();
  
  while( mapIt != mapItEnd )
  {
    locatedSegsIndexes.push_back( mapIt->second );
    
    ++mapIt;
  }
  
  if( locatedSegsIndexes.size() )
  {
    return true;
  }
  else
  {
    return false;
  }
}

