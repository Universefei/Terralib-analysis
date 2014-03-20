#include "TePDITPMosaic.hpp"

#include "TePDIBlender.hpp"
#include "TePDIUtils.hpp"
#include "TePDIPIManager.hpp"

#include "../kernel/TeGTFactory.h"
#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeGeometricTransformation.h"
#include "../kernel/TeOverlay.h"
#include "../kernel/TeAgnostic.h"


TePDITPMosaic::TePDITPMosaic()
{
}


TePDITPMosaic::~TePDITPMosaic()
{
}


void TePDITPMosaic::ResetState( const TePDIParameters& )
{
}


bool TePDITPMosaic::CheckParameters( const TePDIParameters& parameters ) const
{
  /* Checking input_raster1 */
  
  TePDITypes::TePDIRasterPtrType input_raster1;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_raster1", 
    input_raster1 ),
    "Missing parameter: input_raster1" );
  TEAGN_TRUE_OR_RETURN( input_raster1.isActive(),
    "Invalid parameter: input_raster1 inactive" );
  TEAGN_TRUE_OR_RETURN( input_raster1->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: input_raster1 not ready" );    
    
  /* Checking input_raster2 */
  
  TePDITypes::TePDIRasterPtrType input_raster2;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_raster2", 
    input_raster2 ),
    "Missing parameter: input_raster2" );
  TEAGN_TRUE_OR_RETURN( input_raster2.isActive(),
    "Invalid parameter: input_raster2 inactive" );
  TEAGN_TRUE_OR_RETURN( input_raster2->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: input_raster2 not ready" );    
    
  /* channels1 parameter checking */

  std::vector< unsigned int > channels1;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "channels1", channels1 ), 
    "Missing parameter: channels1" );
  for( unsigned int channels1_index = 0 ; 
    channels1_index < channels1.size() ; 
    ++channels1_index ) {
    
    TEAGN_TRUE_OR_RETURN( ( (int)channels1[ channels1_index ] < 
      input_raster1->nBands() ),
      "Invalid parameter: channels1" );
  }
  
  /* channels2 parameter checking */

  std::vector< unsigned int > channels2;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "channels2", channels2 ), 
    "Missing parameter: channels2" );
  TEAGN_TRUE_OR_RETURN( ( channels2.size() == channels1.size() ),
    "Size mismatch between channels1 and channels2" );
  for( unsigned int channels2_index = 0 ; 
    channels2_index < channels2.size() ; 
    ++channels2_index ) {
    
    TEAGN_TRUE_OR_RETURN( ( (int)channels2[ channels2_index ] < 
      input_raster2->nBands() ),
      "Invalid parameter: channels2" );
  }     
    
  /* Checking output_raster */
  
  TePDITypes::TePDIRasterPtrType output_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "output_raster", 
    output_raster ),
    "Missing parameter: output_raster" );
  TEAGN_TRUE_OR_RETURN( output_raster.isActive(),
    "Invalid parameter: output_raster inactive" );
  TEAGN_TRUE_OR_RETURN( output_raster->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: output_raster not ready" );
    
  // Checking transParams
  
  TeGTParams trans_params;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "trans_params", 
    trans_params ),
    "Missing parameter: trans_params" );  
    
  return true;
}


bool TePDITPMosaic::RunImplementation()
{
  // Extracting user supplied parameters
  
  TePDITypes::TePDIRasterPtrType input_raster1;
  params_.GetParameter( "input_raster1", input_raster1 );

  TePDITypes::TePDIRasterPtrType input_raster2;
  params_.GetParameter( "input_raster2", input_raster2 );
  
  std::vector< unsigned int > channels1;
  params_.GetParameter( "channels1", channels1 );

  std::vector< unsigned int > channels2;
  params_.GetParameter( "channels2", channels2 ); 

  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter( "output_raster", output_raster );
  
  TeGTParams trans_params;
  params_.GetParameter( "trans_params", trans_params );   
  
  TePDIBlender::BlendMethod blend_method = TePDIBlender::NoBlendMethod;
  if( params_.CheckParameter< TePDIBlender::BlendMethod >(
    "blend_method" ) )
  {
    params_.GetParameter( "blend_method", blend_method );
  }  
  
  TePDIInterpolator::InterpMethod interp_method = TePDIInterpolator::NNMethod;
  if( params_.CheckParameter< TePDIInterpolator::InterpMethod >(
    "interp_method" ) )
  {
    params_.GetParameter( "interp_method", interp_method );
  }
  
  bool auto_equalize = false;
  if( params_.CheckParameter< int >( "auto_equalize" ) )
  {
    auto_equalize = true;
  }
  
  /* Dumyy value definition */
 
  double dummy_value = 0;
  if( params_.CheckParameter< double >( "dummy_value" ) ) 
  {
    params_.GetParameter( "dummy_value", dummy_value );
  }
  else if( output_raster->params().useDummy_ )
  {
    dummy_value = output_raster->params().dummy_[ 0 ];
  }
  else if( input_raster1->params().useDummy_ )
  {
    dummy_value = input_raster1->params().dummy_[ 0 ];
  }  
  else if( input_raster2->params().useDummy_ )
  {
    dummy_value = input_raster2->params().dummy_[ 0 ];
  }    
  
  // Creating the geometric transformation instance
  // Direct mapping raster 1 indexed space into raster 2 indexed space
  
  TeGeometricTransformation::pointer transPrt( 
    TeGTFactory::make( trans_params ) );
  TEAGN_TRUE_OR_RETURN( transPrt.isActive(), 
    "Invalid transformation parameters" );  
    
  if( ! transPrt->reset( trans_params ) )
  {
    TEAGN_LOG_AND_RETURN( "Invalid transformation parameters" );
  }   
  
  // Guessing intersection polygon on each raster reference
  // and the global bounding box on raster 1 projected reference
  
  TePolygon interPolR1Ref;
  TePolygon interPolR2Ref;
  TeBox globalBBoxR1Ref;
  
  {
    // Raster 2 bounding box - raster 2 reference
    TePolygon r2BBoxR2Ref;
    TEAGN_TRUE_OR_RETURN( TePDIUtils::buildDetailedBBox( input_raster2,
      r2BBoxR2Ref ), "Internal error" );
      
    // Raster 2 bounding box - raster 2 indexed reference
    TePolygon r2BBoxR2IdxRef;
    TePDIUtils::MapCoords2RasterIndexes( r2BBoxR2Ref, input_raster2, 
      r2BBoxR2IdxRef );
      
    // Raster 2 bounding box - raster 1 indexed reference
    TePolygon r2BBoxR1IdxRef;
    {
      TeLine2D line;
      TeCoord2D pt1;
      TeCoord2D pt2;
      
      TEAGN_DEBUG_CONDITION( r2BBoxR2IdxRef.size() == 1,
        "Invalid number of polygon rings" );
      
      for( unsigned int pointIdx = 0 ; pointIdx < r2BBoxR2IdxRef[ 0 ].size() ;
        ++pointIdx )
      {
        pt2 = r2BBoxR2IdxRef[ 0 ][ pointIdx ];
        
        transPrt->inverseMap( pt2, pt1 );
        
        line.add( pt1 );
      }
      
      r2BBoxR1IdxRef.add( TeLinearRing( line ) );
    }  
    
    // Raster 1 bounding box - raster 1 reference
    TePolygon r1BBoxR1Ref;
    TEAGN_TRUE_OR_RETURN( TePDIUtils::buildDetailedBBox( input_raster1,
      r1BBoxR1Ref ), "Internal error" ); 
       
    // Raster 1 bounding box - raster 1 indexed reference
    TePolygon r1BBoxR1IdxRef;
    TePDIUtils::MapCoords2RasterIndexes( r1BBoxR1Ref, input_raster1, 
      r1BBoxR1IdxRef ); 
      
    // Raster 1 bounding box PS - raster 1 indexed reference
    TePolygonSet r1BBoxPSR1IdxRef;
    r1BBoxPSR1IdxRef.add( r1BBoxR1IdxRef );
    
    // Raster 2 bounding box PS - raster 1 indexed reference
    TePolygonSet r2BBoxPSR1IdxRef;
    r2BBoxPSR1IdxRef.add( r2BBoxR1IdxRef );    
          
    // Calculating the intersection polygon - raster 1 indexed reference 
    TePolygon interPolR1IdxRef;
    {
      TePolygonSet auxPs;
      
      TeOVERLAY::TeIntersection( r1BBoxPSR1IdxRef, r2BBoxPSR1IdxRef, 
        auxPs );
        
      if( auxPs.size() )
      {
        TEAGN_DEBUG_CONDITION( auxPs.size() == 1, "Invalid PS size" );
        
        interPolR1IdxRef = auxPs[ 0 ];
      }
    }
      
    // Calculating the intersection polygon - raster 2 indexed reference 
    TePolygon interPolR2IdxRef;
    
    if( interPolR1IdxRef.size() )
    {
      TeLine2D line;
      TeCoord2D pt1;
      TeCoord2D pt2;
      
      TEAGN_DEBUG_CONDITION( interPolR1IdxRef.size() == 1, 
        "Invalid intersection polygon rings number" );
      
      for( unsigned int pointIdx = 0 ; pointIdx < interPolR1IdxRef[ 0 ].size() ;
        ++pointIdx )
      {
        pt1 = interPolR1IdxRef[ 0 ][ pointIdx ];
        
        transPrt->directMap( pt1, pt2 );
        
        line.add( pt2 );
      }
      
      interPolR2IdxRef.add( TeLinearRing( line ) );
    }    
    
    // Intersection polygon - raster 1 projected reference 
    TePDIUtils::MapRasterIndexes2Coords( interPolR1IdxRef, input_raster1, 
      interPolR1Ref );   
      
    // Intersection polygon - raster 2 projected reference 
    TePDIUtils::MapRasterIndexes2Coords( interPolR2IdxRef, input_raster2, 
      interPolR2Ref ); 
      
    // Raster 2 bounding box - Raster 1 projected reference
    TePolygon r2BBoxR1Ref;
    TePDIUtils::MapRasterIndexes2Coords( r2BBoxR1IdxRef, input_raster1, 
      r2BBoxR1Ref );
           
    // Global bounding box - raster 1 projected reference
    
    globalBBoxR1Ref = r1BBoxR1Ref.box();
    updateBox( globalBBoxR1Ref, r2BBoxR1Ref.box() );
  }
  
  // Calc auto equalizing parameters
  
  std::vector< double > pixelOffsets1;
  std::vector< double > pixelScales1;
  std::vector< double > pixelOffsets2;
  std::vector< double > pixelScales2;
  
  if( auto_equalize && ( interPolR1Ref.size() > 0 ) )
  {
    TEAGN_TRUE_OR_RETURN( calcEqParams( input_raster1, channels1,
      interPolR1Ref, input_raster2, channels2, interPolR2Ref,
      dummy_value, pixelOffsets1, pixelScales1, pixelOffsets2, pixelScales2 ), 
      "Error calculating equalization parameters" );
  }
  else
  {
    for( unsigned int idx = 0 ; idx < channels1.size() ; ++idx )
    {
      pixelOffsets1.push_back( 0 );
      pixelScales1.push_back( 1 );
      pixelOffsets2.push_back( 0 );
      pixelScales2.push_back( 1 );      
    }
  }
  
  // Initiating blender
  
  TePDIBlender blender;

  TEAGN_TRUE_OR_RETURN( blender.init( input_raster1, channels1,
    input_raster2, channels2, blend_method,
    interp_method, transPrt->getParameters(), dummy_value, pixelOffsets1, pixelScales1, 
    pixelOffsets2, pixelScales2 ),
    "Blender initiation error" );
    
  // Updating output image geometry
  
  TeRasterParams outputRasterParams = output_raster->params();
  
  outputRasterParams.nBands( channels1.size() );
  
  outputRasterParams.boundingBoxResolution( globalBBoxR1Ref.x1(),
    globalBBoxR1Ref.y1(), globalBBoxR1Ref.x2(),
    globalBBoxR1Ref.y2(), input_raster1->params().resx_,
    input_raster1->params().resy_ );
    
  outputRasterParams.projection( input_raster1->projection() );
    
  outputRasterParams.setDummy( dummy_value, -1 );
  
  TePDITypes::TePDIRasterVectorType rastersVec;
  rastersVec.push_back( input_raster1 );
  rastersVec.push_back( input_raster2 );
  outputRasterParams.setDataType( TePDIUtils::chooseBestPixelType(
    rastersVec ), -1 );
  
  TEAGN_TRUE_OR_RETURN( output_raster->init( outputRasterParams ), 
    "Output raster init error" );    
  
  // Generate output raster blended values
  
  const unsigned int outNLines = (unsigned int)output_raster->params().nlines_;
  const unsigned int outNCols = (unsigned int)output_raster->params().ncols_;
  const unsigned int outNBands = (unsigned int)output_raster->params().nBands();
  double value = 0;
  unsigned int band = 0;
  unsigned int line = 0;
  unsigned int col = 0;
  TeRaster& outRasterRef = *output_raster;
  const double colOff = ( outRasterRef.params().boundingBox().x1() -
    input_raster1->params().boundingBox().x1() ) /
    outRasterRef.params().resx_;
  const double lineOff = -1.0 * ( outRasterRef.params().boundingBox().y2() -
    input_raster1->params().boundingBox().y2() ) /
    outRasterRef.params().resy_;    
  double outMax = 0;
  double outMin = 0;
  
  TePDIPIManager progress( "Rendering mosaic", outNBands * outNLines,
    progress_enabled_ );
  
  for( band = 0 ; band < outNBands ; ++band )
  {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds( output_raster,
      band, outMin, outMax ), "Internal error" );
       
    for( line = 0 ; line < outNLines ; ++line )  
    {
      for( col = 0 ; col < outNCols ; ++col )
      {
        blender.blend( ((double)line) + lineOff, ((double)col) + colOff, band, 
          value );
          
        value = MIN( value, outMax );
        value = MAX( value, outMin );
        
        TEAGN_TRUE_OR_RETURN( outRasterRef.setElement( col, line, value, 
          band ), "Error writing value to output raster" );
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
    }
  }
  
  return true;
}

bool TePDITPMosaic::calcEqParams( 
  const TePDITypes::TePDIRasterPtrType& raster1, 
  const std::vector< unsigned int >& channels1,
  TePolygon& interPolR1Ref,
  const TePDITypes::TePDIRasterPtrType& raster2,
  const std::vector< unsigned int >& channels2,
  TePolygon& interPolR2Ref,
  const double& dummyValue,
  std::vector< double >& pixelOffsets1, 
  std::vector< double >& pixelScales1,
  std::vector< double >& pixelOffsets2, 
  std::vector< double >& pixelScales2 ) const
{
  TEAGN_DEBUG_CONDITION( raster1.isActive(), "Invalid pointer" );
  TEAGN_DEBUG_CONDITION( channels1.size(), "Invalid vector" );
  TEAGN_DEBUG_CONDITION( interPolR1Ref.size() == 1, "Invalid interpol 1" )
  TEAGN_DEBUG_CONDITION( raster2.isActive(), "Invalid pointer" );
  TEAGN_DEBUG_CONDITION( channels2.size(), "Invalid vector" );
  TEAGN_DEBUG_CONDITION( channels1.size() == channels2.size(), 
    "Vectors size mismatch" );
  TEAGN_DEBUG_CONDITION( interPolR2Ref.size() == 1, "Invalid interpol 2" )    
  
  pixelOffsets1.clear();
  pixelScales1.clear();       
  pixelOffsets2.clear();
  pixelScales2.clear();      

  // calculating statistic attributes
  
  std::vector< double > r1Means;
  std::vector< double > r1Variances;
  TEAGN_TRUE_OR_RETURN( calcStats( raster1, channels1, interPolR1Ref,
    dummyValue, r1Means, r1Variances ), "Error calculating statistics" );

  std::vector< double > r2Means;
  std::vector< double > r2Variances;
  TEAGN_TRUE_OR_RETURN( calcStats( raster2, channels2, interPolR2Ref,
    dummyValue, r2Means, r2Variances ), "Error calculating statistics" );
    
  // generating output values

  for( unsigned int idx = 0 ; idx < r1Means.size() ; ++idx )
  {
    if( r1Variances[ idx ] > r2Variances[ idx ] )
    {
      pixelOffsets1.push_back( 0 );
      pixelScales1.push_back( 1 );         
      pixelScales2.push_back( sqrt( r1Variances[ idx ] / r2Variances[ idx ] ) );
      pixelOffsets2.push_back( r1Means[ idx ] - ( pixelScales2[ idx ] * 
        r2Means[ idx ] ) );      
    }
    else
    {
      pixelOffsets2.push_back( 0 );
      pixelScales2.push_back( 1 );         
      pixelScales1.push_back( sqrt( r2Variances[ idx ] / r1Variances[ idx ] ) );
      pixelOffsets1.push_back( r2Means[ idx ] - ( pixelScales1[ idx ] * 
        r1Means[ idx ] ) ); 
    }
  }

  return true;
}

bool TePDITPMosaic::calcStats( const TePDITypes::TePDIRasterPtrType& raster,
  const std::vector< unsigned int >& channels,
  TePolygon& interPol,
  const double& dummyValue,
  std::vector< double >& means,
  std::vector< double >& variances ) const
{
  TEAGN_DEBUG_CONDITION( raster.isActive(), "Invalid pointer" );
  TEAGN_DEBUG_CONDITION( channels.size(), "Invalid vector" );
  TEAGN_DEBUG_CONDITION( interPol.size() == 1, "Invalid polygon" );
    
  means.clear();
  variances.clear();
  
  // Creating the statistic algorithm parameters
  
  TePDITypes::TePDIPolygonSetPtrType polsetptr( new TePolygonSet );
  polsetptr->add( interPol );
  
  for( unsigned int cIdx = 0 ; cIdx < channels.size() ; ++cIdx )
  {
    TePDIHistogram histo;
    histo.ToggleProgressInt( progress_enabled_ );
    TEAGN_TRUE_OR_RETURN( histo.reset( raster, channels[ cIdx ], 0,
      TeBoxPixelIn, polsetptr ),
      "Histogram reset error" );
      
    TePDIHistogram::const_iterator it = histo.begin();
    const TePDIHistogram::const_iterator itEnd = histo.end();
    
    double pixelsNumber = 0;
    double sum = 0;
    
    while( it != itEnd )
    {
      if( it->first != dummyValue )
      {
        sum += ( it->first * ((double)it->second) );
        pixelsNumber += it->second;
      }
      
      ++it;
    }
    
    double mean = sum / pixelsNumber;      
    
    it = histo.begin();
    
    double variance = 0;
      
    while( it != itEnd )
    {
      if( it->first != dummyValue )
      {
        variance += ( ( (double)it->second ) / pixelsNumber ) *
          ( it->first - mean ) * ( it->first - mean );
      }
      
      ++it;
    }      
  
    means.push_back( mean );
    variances.push_back( variance );
  }

  return true;
}


