#include "TePDIParaSegBaatzStrategy.hpp"

#include "TePDIParaSegRegGrowStrategy.hpp"
#include "TePDIUtils.hpp"

#include "../kernel/TeAgnostic.h"

#include <map>

TePDIParaSegBaatzStrategy::TePDIParaSegBaatzStrategy( 
  const TePDIParaSegStrategyParams& params )
  : TePDIParaSegStrategy( params ), eucTreshold_( 0 )
{
  segmenter_.ToggleProgInt( false );
}


TePDIParaSegBaatzStrategy::~TePDIParaSegBaatzStrategy()
{
}

bool TePDIParaSegBaatzStrategy::execute( const RasterDataVecT& 
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
  
  std::vector<unsigned> input_channels;
  for( unsigned int band = 0 ; band < nBands ; ++band )
  {
    input_channels.push_back( band );
  }
  
  segParams_.SetParameter( "input_channels", input_channels );
  segParams_.SetParameter( "input_image", inputRasterPtr_ );
  segParams_.SetParameter( "output_image", outputRasterPtr_ );
  
  // Running the algorithm

  TEAGN_TRUE_OR_RETURN( segmenter_.Apply( segParams_ ),
    "Segmentation error" );
    
  // Creating the segment objects
  
  TEAGN_TRUE_OR_RETURN( TePDIParaSegRegGrowStrategy::createSegsBlkFromLImg<
    TePDIParaSegBaatzStrategy >( rasterDataVector, *outputRasterPtr_, 
    this, outSegsMatrix  ),
    "Error creating output segments" );    

  return true;
}

bool TePDIParaSegBaatzStrategy::mergeSegments( 
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& centerMatrix,
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* topMatrixPtr,
  TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* leftMatrixPtr )
{
  return TePDIParaSegRegGrowStrategy::staticMergeSegments( 
    centerMatrix, topMatrixPtr, leftMatrixPtr, eucTreshold_ );
}

bool TePDIParaSegBaatzStrategy::setParameters( 
  const TePDIParameters& params )
{
  TEAGN_TRUE_OR_RETURN( params.GetParameter( "euc_treshold", eucTreshold_),
    "Missing parameter euc_treshold" );
    
  float scale = 0;
  TEAGN_TRUE_OR_RETURN( params.GetParameter( "scale", scale),
    "Missing parameter scale" );
    
  float compactness = 0;
  TEAGN_TRUE_OR_RETURN( params.GetParameter( "compactness", compactness),
    "Missing parameter compactness" );    
    
  float color = 0;
  TEAGN_TRUE_OR_RETURN( params.GetParameter( "color", color),
    "Missing parameter color" );     
     
  std::vector<float> input_weights;
  TEAGN_TRUE_OR_RETURN( params.GetParameter( "input_weights", input_weights),
    "Missing parameter input_weights" );     
  
  segParams_.Clear();
  segParams_.SetParameter( "euc_treshold", eucTreshold_ );
  segParams_.SetParameter( "scale", scale );
  segParams_.SetParameter( "compactness", compactness );
  segParams_.SetParameter( "color", color );
  segParams_.SetParameter( "input_weights", input_weights );

  return true;
}


