#include "TePDIGeoMosaic.hpp"

#include "TePDIBlender.hpp"
#include "TePDIUtils.hpp"
#include "TePDIPIManager.hpp"

#include "../kernel/TeOverlay.h"
#include "../kernel/TeVectorRemap.h"
#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeGeometricTransformation.h"
#include "../kernel/TeAgnostic.h"


TePDIGeoMosaic::TePDIGeoMosaic()
{
}


TePDIGeoMosaic::~TePDIGeoMosaic()
{
}


void TePDIGeoMosaic::ResetState( const TePDIParameters& )
{
}


bool TePDIGeoMosaic::CheckParameters( const TePDIParameters& parameters ) const
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
    
  /* Checking rasters projections */
  
  TEAGN_TRUE_OR_RETURN( ( input_raster1->params().projection() != 0 ),
    "Missing input_raster1 projection" );
  TEAGN_TRUE_OR_RETURN( 
    ( input_raster1->params().projection()->name() != "NoProjection" ),
    "Invalid input_raster1 projection" );  
  TEAGN_TRUE_OR_RETURN( ( input_raster2->params().projection() != 0 ),
    "Missing input_raster2 projection" );
  TEAGN_TRUE_OR_RETURN( 
    ( input_raster2->params().projection()->name() != "NoProjection" ),
    "Invalid input_raster2 projection" );      
    
  return true;
}


bool TePDIGeoMosaic::RunImplementation()
{
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
  
  TePolygon raster1_pol;
  if( params_.CheckParameter< TePolygon >( "raster1_pol" ) )
  {
    params_.GetParameter( "raster1_pol", raster1_pol );
  }  
  
  TePolygon raster2_pol;
  if( params_.CheckParameter< TePolygon >( "raster2_pol" ) )
  {
    params_.GetParameter( "raster2_pol", raster2_pol );
  }  
  
  /* Input rasters swap using pixel resolution as reference 
    Input raster 1 will always have the better resolution
  */
  
  {
    // Fiding the raster 2 bounding box ( raster 1 proj )
    
    TeBox r2BBoxR1Ref = input_raster2->params().boundingBox();
    TePolygon r2PolR2Ref = polygonFromBox( r2BBoxR1Ref );
    
    TePolygon r2PolR1Ref;  
    TeVectorRemap( r2PolR2Ref, input_raster2->params().projection(),
      r2PolR1Ref, input_raster1->params().projection() ); 
      
    // finding the rasters resolutions
    // raster 1 projected reference
          
    double raster1_x_res = input_raster1->params().resx_;
    double raster1_y_res = input_raster1->params().resy_;
      
    double raster2_x_res = r2PolR1Ref.box().width() / 
      ((double)input_raster2->params().ncols_);
    double raster2_y_res = r2PolR1Ref.box().height() / 
      ((double)input_raster2->params().nlines_);
    
    if( ( ( raster2_x_res <= raster1_x_res ) &&
        ( raster2_y_res < raster1_y_res ) ) ||
      ( ( raster2_x_res < raster1_x_res ) &&
        ( raster2_y_res <= raster1_y_res ) ) ) 
    {
      // swapping rasters pointers 
    
      TePDITypes::TePDIRasterPtrType temp_raster_ptr = input_raster1;
      input_raster1 = input_raster2;
      input_raster2 = temp_raster_ptr;
      
      // swapping rasters bands vectors
      
      std::vector< unsigned int > auxVec = channels1;
      channels1 = channels2;
      channels2 = auxVec;
      
      // swapping rasters polygons
      
      TePolygon auxPol = raster1_pol;
      raster1_pol = raster2_pol;
      raster2_pol = auxPol;
    }
  }  
  
  /* Bringing input_raster2 to the same projection and resolution */
  
  if( ( ! ( ( *input_raster1->params().projection() ) == 
      ( *input_raster2->params().projection() ) ) ) )
  {
    // reprojecting the raster 2 polygon 
    
    TePolygon reprojR2Pol;  
    
    TeVectorRemap( raster2_pol, input_raster2->params().projection(),
      reprojR2Pol, input_raster1->projection() );
      
    raster2_pol = reprojR2Pol;
  
    /* input_raster2 reprojection */
        
    TePDITypes::TePDIRasterPtrType new_input_raster2;
    TeRasterParams new_input_raster2_params = input_raster2->params();	
    new_input_raster2_params.setNLinesNColumns( 1, 1 );
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( 
      new_input_raster2_params, new_input_raster2 ), 
      "Unable to allocate new_input_raster2" );
      
	 unsigned int nlines =  (unsigned int) input_raster2->params().nlines_;
	 unsigned int ncols = (unsigned int) input_raster2->params().ncols_;
    TEAGN_TRUE_OR_RETURN( TePDIUtils::reprojectRaster( *input_raster2,
      nlines,
      ncols, 
	  *( input_raster1->projection() ), 
      progress_enabled_, true,
      *new_input_raster2 ),
      "Raster reprojection error" );
      
    input_raster2 = new_input_raster2;
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
  
  // Calc auto equalizing parameters
  
  std::vector< double > pixelOffsets1;
  std::vector< double > pixelScales1;
  std::vector< double > pixelOffsets2;
  std::vector< double > pixelScales2;
  
  if( auto_equalize )
  {
    TePolygon r1AreaPol;
    if( raster1_pol.size() )
    {
      r1AreaPol = raster1_pol;
    }
    else
    {
      TeBox r1BBox = input_raster1->params().boundingBox();
      
      r1AreaPol = polygonFromBox( r1BBox );
    }
    
    TePolygon r2AreaPol;
    if( raster2_pol.size() )
    {
      r2AreaPol = raster2_pol;
    }
    else
    {
      TeBox r2BBox = input_raster2->params().boundingBox();
      
      r2AreaPol = polygonFromBox( r2BBox );
    }    
  
    TEAGN_TRUE_OR_RETURN( calcEqParams( input_raster1, channels1, r1AreaPol,
      input_raster2, channels2, r2AreaPol, dummy_value, pixelOffsets1,
      pixelScales1, pixelOffsets2, pixelScales2 ), 
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
  
  {
    // Creating the indexed polygons of each raster
    
    TePolygon r1IdxPolR1Ref;
    TePDIUtils::MapCoords2RasterIndexes( raster1_pol, input_raster1,
      r1IdxPolR1Ref );    
      
    TePolygon r2IdxPolR2Ref;
    TePDIUtils::MapCoords2RasterIndexes( raster2_pol, input_raster2,
      r2IdxPolR2Ref );          
  
    // Creating the transformation parameters
    
    TeCoord2D r1ULI( 0, 0 );
    TeCoord2D r1ULP = input_raster1->params().index2Coord( r1ULI );
    TeCoord2D r1ULIOverR2I = input_raster2->params().coord2Index( r1ULP );
    
    TeCoord2D r1LLI( 0, ( (double)input_raster1->params().nlines_ ) - 1.0 );
    TeCoord2D r1LLP = input_raster1->params().index2Coord( r1LLI );
    TeCoord2D r1LLIOverR2I = input_raster2->params().coord2Index( r1LLP );    
    
    TeCoord2D r1URI( ( (double)input_raster1->params().ncols_ ) - 1.0, 0 );
    TeCoord2D r1URP = input_raster1->params().index2Coord( r1URI );
    TeCoord2D r1URIOverR2I = input_raster2->params().coord2Index( r1URP );      
    
    TeGTParams transParams;
    transParams.transformation_name_ = "affine";
    transParams.out_rem_strat_ = TeGTParams::NoOutRemotion;
    transParams.tiepoints_.push_back( TeCoordPair( r1ULI, r1ULIOverR2I ) );
    transParams.tiepoints_.push_back( TeCoordPair( r1LLI, r1LLIOverR2I ) );
    transParams.tiepoints_.push_back( TeCoordPair( r1URI, r1URIOverR2I ) );
/*    
    TEAGN_TRUE_OR_RETURN( transParams.direct_parameters_.Init( 3, 3, 0.0 ),
      "Matrix init error" );
    TEAGN_TRUE_OR_RETURN( transParams.inverse_parameters_.Init( 3, 3, 0.0 ),
      "Matrix init error" );    
    transParams.direct_parameters_( 0, 0 ) = 1.0;
    transParams.direct_parameters_( 0, 2 ) = r1ULIOverR2I.x();
    transParams.direct_parameters_( 1, 1 ) = 1.0;
    transParams.direct_parameters_( 1, 2 ) = r1ULIOverR2I.y();
    transParams.inverse_parameters_( 0, 0 ) = 1.0;
    transParams.inverse_parameters_( 0, 2 ) = -r1ULIOverR2I.x();
    transParams.inverse_parameters_( 1, 1 ) = 1.0;
    transParams.inverse_parameters_( 1, 2 ) = -r1ULIOverR2I.y();    
*/    
    // Initiating blender
    
    TEAGN_TRUE_OR_RETURN( blender.init( input_raster1, channels1,
      input_raster2, channels2, blend_method,
      interp_method, transParams, dummy_value, pixelOffsets1, pixelScales1, 
      pixelOffsets2, pixelScales2, r1IdxPolR1Ref, r2IdxPolR2Ref ),
      "Blender initiation error" );
  }
    
  // Updating output image geometry
  
  TeRasterParams outputRasterParams = output_raster->params();
  
  outputRasterParams.nBands( channels1.size() );
  
  TeBox outputRasterBoundingBox = input_raster1->params().boundingBox();
  updateBox( outputRasterBoundingBox, input_raster2->params().boundingBox() );
  
  outputRasterParams.boundingBoxResolution( outputRasterBoundingBox.x1(),
    outputRasterBoundingBox.y1(), outputRasterBoundingBox.x2(),
    outputRasterBoundingBox.y2(), input_raster1->params().resx_,
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

bool TePDIGeoMosaic::calcEqParams( 
  const TePDITypes::TePDIRasterPtrType& raster1, 
  const std::vector< unsigned int >& channels1,
  TePolygon& r1Pol,
  const TePDITypes::TePDIRasterPtrType& raster2,
  const std::vector< unsigned int >& channels2,
  TePolygon& r2Pol,
  const double& dummyValue,
  std::vector< double >& pixelOffsets1, 
  std::vector< double >& pixelScales1,
  std::vector< double >& pixelOffsets2, 
  std::vector< double >& pixelScales2 ) const
{
  TEAGN_DEBUG_CONDITION( raster1.isActive(), "Invalid pointer" );
  TEAGN_DEBUG_CONDITION( channels1.size(), "Invalid vector" );
  TEAGN_DEBUG_CONDITION( r1Pol.size(), "Invalid r1Pol" );
  TEAGN_DEBUG_CONDITION( raster2.isActive(), "Invalid pointer" );
  TEAGN_DEBUG_CONDITION( channels2.size(), "Invalid vector" );
  TEAGN_DEBUG_CONDITION( r2Pol.size(), "Invalid r2Pol" );
  TEAGN_DEBUG_CONDITION( channels1.size() == channels2.size(), 
    "Vectors size mismatch" );
    
  pixelOffsets1.clear();
  pixelScales1.clear();       
  pixelOffsets2.clear();
  pixelScales2.clear();    
    
  TePolygonSet r1Ps;
  r1Ps.add( r1Pol );
  
  TePolygonSet r2Ps;
  r2Ps.add( r2Pol );  
  
  TePolygonSet interPs;
  
  if( TeOVERLAY::TeIntersection( r1Ps, r2Ps, interPs ) && 
    ( interPs.size() > 0 ) )
  {
    // calculating statistic attributes
    
    std::vector< double > r1Means;
    std::vector< double > r1Variances;
    TEAGN_TRUE_OR_RETURN( calcStats( raster1, channels1, interPs,
      dummyValue, r1Means, r1Variances ), "Error calculating statistics" );
  
    std::vector< double > r2Means;
    std::vector< double > r2Variances;
    TEAGN_TRUE_OR_RETURN( calcStats( raster2, channels2, interPs,
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
  }
  else
  {
    for( unsigned int idx = 0 ; idx < channels2.size() ; ++idx )
    {
      pixelOffsets1.push_back( 0 );
      pixelScales1.push_back( 1 );     
      pixelOffsets2.push_back( 0 );
      pixelScales2.push_back( 1 );      
    }  
  }
  
  return true;
}

bool TePDIGeoMosaic::calcStats( const TePDITypes::TePDIRasterPtrType& raster,
  const std::vector< unsigned int >& channels,
  TePolygonSet& polSet,
  const double& dummyValue,
  std::vector< double >& means,
  std::vector< double >& variances ) const
{
  TEAGN_DEBUG_CONDITION( raster.isActive(), "Invalid pointer" );
  TEAGN_DEBUG_CONDITION( channels.size(), "Invalid vector" );
  TEAGN_DEBUG_CONDITION( polSet.size() == 1, "Invalid PS" );
  TEAGN_DEBUG_CONDITION( polSet[ 0 ].size() > 0, "Invalid PS" );
    
  means.clear();
  variances.clear();
  
  // Creating the statistic algorithm parameters
  
  TePDITypes::TePDIPolygonSetPtrType polsetptr( &polSet, true );
  
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


