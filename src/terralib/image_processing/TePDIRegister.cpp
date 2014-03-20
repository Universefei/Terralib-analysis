#include "TePDIRegister.hpp"
#include "../kernel/TeAgnostic.h"
#include "TePDITypes.hpp"
#include "TePDIUtils.hpp"

#include "../kernel/TeGTFactory.h"
#include "../kernel/TeDefines.h"
#include "../kernel/TeUtils.h"

#include <string>

#include <math.h>
#include <float.h>

TePDIRegister::TePDIRegister()
{
}


TePDIRegister::~TePDIRegister()
{
}


void TePDIRegister::ResetState( const TePDIParameters& )
{
}


bool TePDIRegister::CheckParameters( const TePDIParameters& parameters ) const
{
  /* Checking adjust_raster */
    
  TePDITypes::TePDIRasterPtrType adjust_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "adjust_raster", 
    adjust_raster ),
    "Missing parameter: adjust_raster" );
  TEAGN_TRUE_OR_RETURN( adjust_raster.isActive(),
    "Invalid parameter: adjust_raster inactive" );
  TEAGN_TRUE_OR_RETURN( adjust_raster->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: adjust_raster not ready" );
    
  /* adjust_channels parameter checking */

  std::vector< int > adjust_channels;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "adjust_channels", adjust_channels ), 
    "Missing parameter: adjust_channels" );
  for( unsigned int adjust_channels_index = 0 ; 
    adjust_channels_index < adjust_channels.size() ; 
    ++adjust_channels_index ) {
    
    TEAGN_TRUE_OR_RETURN(
      ( adjust_channels[ adjust_channels_index ] < adjust_raster->nBands() ),
      "Invalid parameter: channels" );
  }
    
  /* Checking output_raster */
    
  TePDITypes::TePDIRasterPtrType output_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "output_raster", 
    output_raster ),
    "Missing parameter: output_raster" );
  TEAGN_TRUE_OR_RETURN( output_raster.isActive(),
    "Invalid parameter: output_raster inactive" );
  TEAGN_TRUE_OR_RETURN( output_raster->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: output_raster not ready" );    
    
  /* Checking the geometric transformation */

  
  if( ! parameters.CheckParameter< TeGTParams >( "transf_params" ) )
  {
    std::string gt_name;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "gt_name", 
      gt_name ), "Missing parameter: gt_name" );
      
    TeGTFactory::TeFactoryMap::iterator gt_it = 
      TeGTFactory::instance().find( gt_name );  
    TEAGN_TRUE_OR_RETURN( ( gt_it != TeGTFactory::instance().end() ),
       "Invalid parameter: gt_name" );    

    TeGTParams gt_params;
    gt_params.transformation_name_ = gt_name;          
    TeSharedPtr< TeGeometricTransformation > geotransf_ptr( 
      TeGTFactory::make( gt_params ) );
    TEAGN_TRUE_OR_THROW( geotransf_ptr.isActive(),
       "Geometric transformation factory product instatiation error" );
    
    /* Checking tie points */
    
    unsigned int adjust_raster_lines = 
      (unsigned int)adjust_raster->params().nlines_;
    unsigned int adjust_raster_cols = 
      (unsigned int)adjust_raster->params().ncols_;
  
    TePDITypes::TeCoordPairVectPtrType tie_points;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "tie_points", 
      tie_points ), "Missing parameter: tie_points" );
    TEAGN_TRUE_OR_RETURN( 
      ( tie_points->size() >= geotransf_ptr->getMinRequiredTiePoints() ),
      "Invalid parameter: insufficient tie_points for the required "
      "transformation" );
      
    for( unsigned int tie_points_index = 0 ; 
      tie_points_index < tie_points->size() ; ++ tie_points_index ) {
      
      TEAGN_TRUE_OR_RETURN( 
        ( (*tie_points)[ tie_points_index ].pt1.x() >= ( -0.5 ) ) &&
        ( (*tie_points)[ tie_points_index ].pt1.x() <= 
          ( ( (double)( adjust_raster_cols ) ) - 0.5 ) ) &&
        ( (*tie_points)[ tie_points_index ].pt1.y() >= ( -0.5 ) ) &&
        ( (*tie_points)[ tie_points_index ].pt1.y() <= 
          ( ( (double)( adjust_raster_lines  ) - 0.5 ) ) ),
        "Invalid parameter: tie_points - pt1 coords out of range" );
    }
  }
    
  /* Checking the interpolator method */
  
  TePDIInterpolator::InterpMethod interpolator;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "interpolator", 
    interpolator ), "Missing parameter: interpolator" );  
  
  TePDIInterpolator interp;
  TEAGN_TRUE_OR_RETURN( interp.reset( adjust_raster, 
    (TePDIInterpolator::InterpMethod)interpolator, 0 ), 
    "Invalid parameter: interpolator" );
    
  /* Checking proj_params parameter */
  
  TePDITypes::TeProjParamsPtrType proj_params;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "proj_params", proj_params ),
    "Missing paramter : proj_params" );
    
  TeSharedPtr< TeProjection > proj_ptr( TeProjectionFactory::make( 
    *proj_params ) );
  TEAGN_TRUE_OR_THROW( proj_ptr.isActive(), 
    "Invalid parameter : proj_params" );
    
  /* Checking res_x and res_y */
  
  double res_x = 0;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "res_x", res_x ),
    "Missing parameter : res_x" );

  double res_y = 0;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "res_y", res_y ),
    "Missing parameter : res_y" );
        
  return true;
}


bool TePDIRegister::RunImplementation()
{
  /* Retriving parameters */
  
  TePDITypes::TeProjParamsPtrType proj_params;
  params_.GetParameter( "proj_params", proj_params );
  
  double res_x = 0;
  params_.GetParameter( "res_x", res_x );

  double res_y = 0;
  params_.GetParameter( "res_y", res_y );
    
  TePDITypes::TePDIRasterPtrType adjust_raster;
  params_.GetParameter( "adjust_raster", adjust_raster );
  
  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter( "output_raster", output_raster );
  
  TeGTParams transf_params;
  if( params_.CheckParameter< TeGTParams >( "transf_params" ) ) 
  {
    params_.GetParameter( "transf_params", transf_params );
  }
  else
  {
    params_.GetParameter( "gt_name", transf_params.transformation_name_ );
    
    TePDITypes::TeCoordPairVectPtrType tie_points;
    params_.GetParameter( "tie_points", tie_points );
    transf_params.tiepoints_ = *tie_points;
  }
  
  double dummy_value = 0;
  bool use_dummy = false;
  if( params_.CheckParameter< double >( "dummy_value" ) ) {
    
    params_.GetParameter( "dummy_value", dummy_value );
    use_dummy = true;
  } else {
    if( adjust_raster->params().useDummy_ ) {
      dummy_value = adjust_raster->params().dummy_[ 0 ];
      use_dummy = true;
    }
  }
  
  std::vector< int > adjust_channels;
  params_.GetParameter( "adjust_channels", adjust_channels );
  
  /* Instantiating the interpolator */
  
  TePDIInterpolator::InterpMethod interpolator;
  params_.GetParameter( "interpolator", interpolator );
  
  TePDIInterpolator interp;
  TEAGN_TRUE_OR_RETURN( interp.reset( adjust_raster, 
    (TePDIInterpolator::InterpMethod)interpolator, dummy_value ),
    "Interpolator reset error" );
    
  /* Instantiating the geometric transformation mapping adjust raster indexes
     to projected space coordinates */
     
  TeSharedPtr< TeGeometricTransformation > geotransf_ptr(
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_RETURN( geotransf_ptr.isActive(),
    "Unable to generate the geometric transformation" );    
  TEAGN_TRUE_OR_RETURN( geotransf_ptr->reset( transf_params ),
    "Unable to generate the geometric transformation" );      
    
  /* Guessing the output raster geometry */
  
  TePolygon adjust_raster_detailed_box_wi;/* world indexes */
  TePolygon adjust_raster_detailed_box_mi;/* matricial indexes */
  TEAGN_TRUE_OR_RETURN( TePDIUtils::buildDetailedBox( adjust_raster, 
    adjust_raster_detailed_box_wi ), 
    "Unable to build detailed box from adjust raster" );
  TePDIUtils::MapCoords2RasterIndexes( adjust_raster_detailed_box_wi, 
    adjust_raster, adjust_raster_detailed_box_mi );
    
  TeCoord2D transformed_point;
  double min_x = DBL_MAX;
  double min_y = DBL_MAX;
  double max_x = ( -1.0 ) * DBL_MAX;
  double max_y = ( -1.0 ) * DBL_MAX;
  
  for( unsigned int ardb_index = 0 ; 
    ardb_index < adjust_raster_detailed_box_mi[ 0 ].size() ;
    ++ardb_index ) {
            
    geotransf_ptr->directMap( 
      adjust_raster_detailed_box_mi[ 0 ][ ardb_index ],
      transformed_point );
      
    if( transformed_point.x() < min_x ) {
      min_x = transformed_point.x();
    }
    if( transformed_point.y() < min_y ) {
      min_y = transformed_point.y();
    }
    if( transformed_point.x() > max_x ) {
      max_x = transformed_point.x();
    }
    if( transformed_point.y() > max_y ) {
      max_y = transformed_point.y();
    }
  }
  
  TeSharedPtr< TeProjection > proj_ptr( TeProjectionFactory::make( 
    *proj_params ) );
  TEAGN_TRUE_OR_THROW( proj_ptr.isActive(), 
    "Projection factory instatiation error" );
  
  TeRasterParams output_raster_params = output_raster->params();
  output_raster_params.nBands( adjust_channels.size() );
  output_raster_params.projection( proj_ptr.nakedPointer() );
  if( use_dummy )
  {
    output_raster_params.setDummy( dummy_value, -1 );
  }
  else
  {
    output_raster_params.useDummy_ = false;
  }

  TeCoord2D output_raster_box_ll( min_x, min_y );
  TeCoord2D output_raster_box_ur( max_x, max_y );
  
  output_raster_params.boxResolution( output_raster_box_ll.x(), 
    output_raster_box_ll.y(), output_raster_box_ur.x(), 
    output_raster_box_ur.y(), res_x, res_y );
    
  TEAGN_TRUE_OR_RETURN( output_raster->init( output_raster_params ),
    "output_raster init error" );
    
  /* Rendering output raster */
  
  TeGeometricTransformation& geotransf = *geotransf_ptr;
  TeRaster& output_raster_ref = *output_raster;
  unsigned int output_raster_lines = output_raster_ref.params().nlines_;
  unsigned int output_raster_cols = output_raster_ref.params().ncols_;
  unsigned int line = 0;
  unsigned int col = 0;
  TeCoord2D output_raster_pt_mi; /* matricial indexes */
  TeCoord2D output_raster_pt_pi; /* projected indexes */
  TeCoord2D adjust_raster_pt_mi;
  double interpolated_value = 0;
  double outMax = 0;
  double outMin = 0;   
  
  StartProgInt( "Rendering output", adjust_channels.size() * 
    output_raster_lines );  
  
  for( unsigned int adjust_channels_index = 0 ; 
    adjust_channels_index < adjust_channels.size() ; 
    ++adjust_channels_index ) {
    
    unsigned int adjust_channel = 
      (unsigned int)adjust_channels[ adjust_channels_index ];
      
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds( output_raster,
      adjust_channels_index, outMin, outMax ), "Internal error" );      
    
    for( line = 0 ; line < output_raster_lines ; ++line ) {
      for( col = 0 ; col < output_raster_cols ; ++col ) {
        output_raster_pt_mi.setXY( (double)col, (double)line );
        output_raster_pt_pi = output_raster_ref.index2Coord( 
          output_raster_pt_mi );
        
        geotransf.inverseMap( output_raster_pt_pi, adjust_raster_pt_mi );
      
        interp.interpolate( adjust_raster_pt_mi.y(), adjust_raster_pt_mi.x(),
          adjust_channel, interpolated_value );
          
        interpolated_value = MIN( interpolated_value, outMax );
        interpolated_value = MAX( interpolated_value, outMin );              
      
        TEAGN_TRUE_OR_RETURN( 
          output_raster_ref.setElement( 
          col, line, interpolated_value, adjust_channels_index ), 
          "output_raster write error at line=" +
          Te2String( line ) + " col=" + Te2String( col ) + " channel=" +
          Te2String( adjust_channels_index ) );
      }
      
      TEAGN_FALSE_OR_RETURN( IncProgInt(), "Canceled by the user" );
    }
  }
  
  StopProgInt();
  
  return true;
}

