#include "TePDISensorSimulator.hpp"
#include "TePDILinearFilter.hpp"
#include "TePDITypes.hpp"
#include "TePDIUtils.hpp"

#include <string>

#include <math.h>
#include <float.h>

//#include <string>

//#include <math.h>
//#include <float.h>


TePDISensorSimulator::TePDISensorSimulator()
{
}


TePDISensorSimulator::~TePDISensorSimulator()
{
}


void TePDISensorSimulator::ResetState( const TePDIParameters& )
{
}


bool TePDISensorSimulator::CheckParameters( const TePDIParameters& parameters ) const
{
  /* Checking input raster */

  TePDITypes::TePDIRasterPtrType input_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_raster", 
    input_raster ),
    "Missing parameter: input_raster" );
  TEAGN_TRUE_OR_RETURN( input_raster.isActive(),
    "Invalid parameter: input_raster inactive" );
  TEAGN_TRUE_OR_RETURN( input_raster->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: input_raster not ready" );


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


  /* channels parameter checking */

  std::vector< int > channels;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "channels", channels ), 
    "Missing parameter: channels" );
  for( unsigned int channels_index = 0 ; 
    channels_index < channels.size() ; 
    ++channels_index ) {

      TEAGN_TRUE_OR_RETURN(
        ( channels[ channels_index ] < input_raster->nBands() ),
        "Invalid parameter: channels" );
    }

    
    /* ifov_in parameter checking */

    double ifov_in;
    if( ! parameters.GetParameter( "ifov_in", ifov_in ) ) {

      TEAGN_LOGERR( "Missing parameter: ifov_in" );
      return false;
    }
    TEAGN_TRUE_OR_RETURN( ifov_in > 0, "Invalid input ifov" );

    /* eifov_in_across parameter checking */

    double eifov_in_across;
    if( ! parameters.GetParameter( "eifov_in_across", eifov_in_across ) ) {

      TEAGN_LOGERR( "Missing parameter: eifov_in_across" );
      return false;
    }
    TEAGN_TRUE_OR_RETURN( eifov_in_across > ifov_in, "Invalid input eifov across track" );

    /* eifov_in_long parameter checking */

    double eifov_in_long;
    if( ! parameters.GetParameter( "eifov_in_long", eifov_in_long ) ) {

      TEAGN_LOGERR( "Missing parameter: eifov_in_long" );
      return false;
    }
    TEAGN_TRUE_OR_RETURN( eifov_in_long > ifov_in, "Invalid input eifov long track" );

    /* ifov_out parameter checking */

    double ifov_out;
    if( ! parameters.GetParameter( "ifov_out", ifov_out ) ) {

      TEAGN_LOGERR( "Missing parameter: ifov_out" );
      return false;
    }
    TEAGN_TRUE_OR_RETURN( ifov_out > ifov_in, "Invalid output ifov" );

    /* eifov_out_across parameter checking */

    double eifov_out_across;
    if( ! parameters.GetParameter( "eifov_out_across", eifov_out_across ) ) {

      TEAGN_LOGERR( "Missing parameter: eifov_out_across" );
      return false;
    }
    TEAGN_TRUE_OR_RETURN( eifov_out_across > ifov_out, "Invalid output eifov across track" );

    /* eifov_out_long parameter checking */

    double eifov_out_long;
    if( ! parameters.GetParameter( "eifov_out_long", eifov_out_long ) ) {

      TEAGN_LOGERR( "Missing parameter: eifov_out_long" );
      return false;
    }
    TEAGN_TRUE_OR_RETURN( eifov_out_long > ifov_out, "Invalid output eifov long track" );

    return true;
}


bool TePDISensorSimulator::RunImplementation()
{
  /* Retriving parameters */
  
  TePDITypes::TePDIRasterPtrType input_raster;
  params_.GetParameter( "input_raster", input_raster );
  
  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter( "output_raster", output_raster );
  
  std::vector< int > channels;
  params_.GetParameter( "channels", channels );
  
  double ifov_in;
  params_.GetParameter( "ifov_in", ifov_in );

  double eifov_in_across;
  params_.GetParameter( "eifov_in_across", eifov_in_across );

  double eifov_in_long;
  params_.GetParameter( "eifov_in_long", eifov_in_long );

  double ifov_out;
  params_.GetParameter( "ifov_out", ifov_out );

  double eifov_out_across;
  params_.GetParameter( "eifov_out_across", eifov_out_across );

  double eifov_out_long;
  params_.GetParameter( "eifov_out_long", eifov_out_long );

  /* calculating images parameters */

  double sigma_in_across, sigma_in_long, sigma_out_across, sigma_out_long;
  double varF1, varF2;
  int n;
  double alfa1, alfa2, a1, a2, b1, b2;

  sigma_in_across = (1/TePI) * sqrt(2*log(2.0)) * eifov_in_across; //original sensor
  sigma_in_long = (1/TePI) * sqrt(2*log(2.0)) * eifov_in_long;

  sigma_out_across = (1/TePI) * sqrt(2*log(2.0)) * eifov_out_across; //desired output sensor
  sigma_out_long = (1/TePI) * sqrt(2*log(2.0)) * eifov_out_long;

  varF1 = pow(sigma_out_across,2) - pow(sigma_in_across,2);
  varF2 = pow(sigma_out_long,2) - pow(sigma_in_long,2);

  n = int(ceil((1.5/(pow(ifov_in,2))) * MAX(varF1, varF2)));

  alfa1 = (varF1)/(2*(n*pow(ifov_in,2)-varF1));
  alfa2 = (varF2)/(2*(n*pow(ifov_in,2)-varF2));

  a1 = 1/(1+2*alfa1);
  a2 = 1/(1+2*alfa2);

  b1 = alfa1/(1+2*alfa1);
  b2 = alfa2/(1+2*alfa2);

  TePDIFilterMask::pointer filter_mask( new TePDIFilterMask( 3, 0 ) );
  filter_mask->set(0,0,b1*b2);
  filter_mask->set(1,0,a1*b2);
  filter_mask->set(2,0,b1*b2);
  filter_mask->set(0,1,b1*a2);
  filter_mask->set(1,1,a1*a2);
  filter_mask->set(2,1,b1*a2);
  filter_mask->set(0,2,b1*b2);
  filter_mask->set(1,2,a1*b2);
  filter_mask->set(2,2,b1*b2);

  TePDIParameters new_params = params_;

  new_params.SetParameter("input_image",input_raster);
  new_params.SetParameter( "filter_mask", filter_mask );
  new_params.SetParameter( "iterations", n );

  TePDILinearFilter filter;
  
  if( ifov_in == ifov_out ) {
    /* No Ressampling required */
    
    new_params.SetParameter("output_image",output_raster);
    
    TEAGN_TRUE_OR_THROW( filter.Reset( new_params ), "Invalid Parameters" );
    TEAGN_TRUE_OR_RETURN( filter.Apply(), "Linear filter error" );
  } else {
    /* Ressampling is required */

    TePDITypes::TePDIRasterPtrType non_resssampled_output_raster;
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster(
      non_resssampled_output_raster, input_raster->params(), 
        TePDIUtils::TePDIUtilsAutoMemPol ), "Temporary raster allocation error" )
        
    new_params.SetParameter( "output_image", non_resssampled_output_raster );
    
    TEAGN_TRUE_OR_THROW( filter.Reset( new_params ), "Invalid Parameters" );
    TEAGN_TRUE_OR_RETURN( filter.Apply(), "Linear filter error" );
    
    /* Ressampling raster */
    
    TePDIInterpolator::InterpMethod interpolator = 
      TePDIInterpolator::BicubicMethod;
    
    if( params_.CheckParameter<TePDIInterpolator::InterpMethod>( 
      "interpolator" ) ) {
      
      params_.GetParameter( "interpolator", interpolator );
    }
    
    double x_resolution_ratio = ( ifov_out / ifov_in );
    double y_resolution_ratio = x_resolution_ratio;
    
    TEAGN_TRUE_OR_RETURN( TePDIUtils::resampleRasterByRes( 
      non_resssampled_output_raster, output_raster, x_resolution_ratio,
      y_resolution_ratio, interpolator, progress_enabled_ ), 
      "Output raster resample error" )
  }
  
  return true;
}

