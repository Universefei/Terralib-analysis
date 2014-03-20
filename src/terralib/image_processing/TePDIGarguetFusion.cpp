#include "TePDIGarguetFusion.hpp"
#include "TePDIUtils.hpp"
#include "TePDIMallatWavelets.hpp"

TePDIGarguetFusion::TePDIGarguetFusion()
{
}

TePDIGarguetFusion::~TePDIGarguetFusion()
{
}


void TePDIGarguetFusion::ResetState( const TePDIParameters& )
{
}


bool TePDIGarguetFusion::CheckParameters( const TePDIParameters& parameters ) 
  const
{
  /* Checking reference_raster */
  
  TePDITypes::TePDIRasterPtrType reference_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "reference_raster", 
    reference_raster ),
    "Missing parameter: reference_raster" );
  TEAGN_TRUE_OR_RETURN( reference_raster.isActive(),
    "Invalid parameter: reference_raster inactive" );
  TEAGN_TRUE_OR_RETURN( reference_raster->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: reference_raster not ready" );    
    
  /* Checking lowres_raster */
  
  TePDITypes::TePDIRasterPtrType lowres_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "lowres_raster", 
    lowres_raster ),
    "Missing parameter: lowres_raster" );
  TEAGN_TRUE_OR_RETURN( lowres_raster.isActive(),
    "Invalid parameter: lowres_raster inactive" );
  TEAGN_TRUE_OR_RETURN( lowres_raster->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: lowres_raster not ready" );    
    
  double max_low_res_raster_res =
    MAX( lowres_raster->params().resx_, 
    lowres_raster->params().resy_ );
  double max_reference_raster_res =
    MAX( reference_raster->params().resx_, 
    reference_raster->params().resy_ );
    
  TEAGN_TRUE_OR_RETURN(     
    ( max_low_res_raster_res > max_reference_raster_res ),
    "Invalid low resolution raster resolution" );
        
  /* Checking output_raster */
  
  TePDITypes::TePDIRasterPtrType output_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "output_raster", 
    output_raster ),
    "Missing parameter: output_raster" );
  TEAGN_TRUE_OR_RETURN( output_raster.isActive(),
    "Invalid parameter: output_raster inactive" );
  TEAGN_TRUE_OR_RETURN( output_raster->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: output_raster not ready" );    
    
  /* Checking bands */    
    
  int reference_raster_band = 0;    
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "reference_raster_band", 
    reference_raster_band ), "Missing parameter : reference_raster_band" );
  TEAGN_TRUE_OR_RETURN( ( reference_raster->nBands() > reference_raster_band ),
    "Invalid parameter : reference_raster_band" );
    
  int lowres_raster_band = 0;    
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "lowres_raster_band", 
    lowres_raster_band ), "Missing parameter : lowres_raster_band" );
  TEAGN_TRUE_OR_RETURN( ( lowres_raster->nBands() > lowres_raster_band ),
    "Invalid parameter : lowres_raster_band" ); 
    
  /* Checking photometric interpretation */
  
  TEAGN_TRUE_OR_RETURN( ( 
    ( reference_raster->params().photometric_[ reference_raster_band ] == 
      TeRasterParams::TeRGB ) ||
    ( reference_raster->params().photometric_[ reference_raster_band ] == 
      TeRasterParams::TeMultiBand ) ),
  "Invalid parameter - reference_raster (invalid photometric interpretation)" );   

  TEAGN_TRUE_OR_RETURN( ( 
    ( lowres_raster->params().photometric_[ lowres_raster_band ] == 
      TeRasterParams::TeRGB ) ||
    ( lowres_raster->params().photometric_[ lowres_raster_band ] == 
      TeRasterParams::TeMultiBand ) ),
  "Invalid parameter - lowres_raster (invalid photometric interpretation)" );   
      
  return true;
}


bool TePDIGarguetFusion::RunImplementation()
{
  TePDITypes::TePDIRasterPtrType reference_raster;
  params_.GetParameter( "reference_raster", reference_raster );

  TePDITypes::TePDIRasterPtrType lowres_raster;
  params_.GetParameter( "lowres_raster", lowres_raster );

  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter( "output_raster", output_raster );
  
  int reference_raster_band = 0;
  params_.GetParameter( "reference_raster_band", reference_raster_band );

  int lowres_raster_band = 0;
  params_.GetParameter( "lowres_raster_band", lowres_raster_band );
  
  TePDITypes::TePDIRasterPtrType lowlow;
  TEAGN_TRUE_OR_THROW( TePDIUtils::TeAllocRAMRaster( lowlow, 1, 1, 1, false,
    TeDOUBLE, 0 ), "Unable to create temporary low-low raster" );  
  
  /* Finding the best decomposition levels for reference raster */
  
  unsigned int rr_best_level = 0;
  unsigned int rr_best_level_w = 0;
  unsigned int rr_best_level_h = 0;
  
  TEAGN_TRUE_OR_RETURN( findBestDecimLevel( reference_raster,
    lowres_raster, rr_best_level, rr_best_level_w,
    rr_best_level_h ), 
    "Unable to find the best decomposition level" )
    
 /* Building default wavelet filters  - Reference coef 07 */
 
  double filters_scale = 0.71;
  if( params_.CheckParameter< double >( "filters_scale" ) ) {
      
    params_.GetParameter( "filters_scale", filters_scale );
  }
    
  std::vector< double > a_filter_l;
  a_filter_l.push_back( -0.05 );
  a_filter_l.push_back( 0.25 );
  a_filter_l.push_back( 0.6 );
  a_filter_l.push_back( 0.25 );
  a_filter_l.push_back( -0.05 );
  
  std::vector< double > a_filter_h;
  a_filter_h.push_back( 0.0107143 );
  a_filter_h.push_back( -0.0535714 );
  a_filter_h.push_back( -0.2607143 );
  a_filter_h.push_back( 0.6071429 );
  a_filter_h.push_back( -0.2607143 );
  a_filter_h.push_back( -0.0535714 );
  a_filter_h.push_back( 0.0107143 );
  
  std::vector< double > s_filter_l;
  s_filter_l.push_back( -0.0107143 );
  s_filter_l.push_back( -0.0535714 );
  s_filter_l.push_back( 0.2607143 );
  s_filter_l.push_back( 0.6071429 );
  s_filter_l.push_back( 0.2607143 );
  s_filter_l.push_back( -0.0535714 );
  s_filter_l.push_back( -0.0107143 );
  
  std::vector< double > s_filter_h;
  s_filter_h.push_back( -0.05 );
  s_filter_h.push_back( -0.25 );
  s_filter_h.push_back( 0.6 );
  s_filter_h.push_back( -0.25 );
  s_filter_h.push_back( -0.05 );
  
  /* Checking for user supplied wavelet filters */
  
  if( params_.CheckParameter< std::vector< double > >( "a_filter_l" ) ) {
      
    params_.GetParameter( "a_filter_l", a_filter_l );
  }
  if( params_.CheckParameter< std::vector< double > >( "a_filter_h" ) ) {
      
    params_.GetParameter( "a_filter_h", a_filter_h );
  }  
  if( params_.CheckParameter< std::vector< double > >( "s_filter_l" ) ) {
      
    params_.GetParameter( "s_filter_l", s_filter_l );
  }
  if( params_.CheckParameter< std::vector< double > >( "s_filter_h" ) ) {
      
    params_.GetParameter( "s_filter_h", s_filter_h );
  }  

  /* Bringing the low res raster to the correct size */

  if( ( lowres_raster->params().nlines_ != (int)rr_best_level_h ) ||
    ( lowres_raster->params().ncols_ != (int)rr_best_level_w ) ) {

    TePDITypes::TePDIRasterPtrType resampled_raster;

    TeRasterParams dummy_pars;
    dummy_pars.setNLinesNColumns( 1, 1 );

    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( 
      resampled_raster,
      dummy_pars, TePDIUtils::TePDIUtilsAutoMemPol ),
      "Unable create the new resampled raster" );

    TePDIInterpolator::InterpMethod int_method = 
      TePDIInterpolator::NNMethod;
    if( 
      ( ( (int)rr_best_level_h ) < 
      lowres_raster->params().nlines_ ) ||
      ( ( (int)rr_best_level_w ) < 
      lowres_raster->params().ncols_ ) ) {

      int_method = TePDIInterpolator::BicubicMethod;
    }

    TEAGN_TRUE_OR_RETURN( TePDIUtils::resampleRasterByLinsCols(
      lowres_raster, resampled_raster, rr_best_level_h,
      rr_best_level_w, progress_enabled_,
      TePDIInterpolator::NNMethod ),
      "Error resampling low resolution raster" );

    lowres_raster = resampled_raster;
  }


  /* starting filter */
  
  TePDIParameters params2;
  params2.SetParameter( "filter_task", std::string( "SBSwap" ) );
  params2.SetParameter( "input_image", reference_raster );
  params2.SetParameter( "band", reference_raster_band );
  params2.SetParameter( "output_image", output_raster );
  params2.SetParameter( "a_filter_l", a_filter_l );
  params2.SetParameter( "a_filter_h", a_filter_h );
  params2.SetParameter( "s_filter_l", s_filter_l );
  params2.SetParameter( "s_filter_h", s_filter_h );    
  params2.SetParameter( "filters_scale", filters_scale );         
  params2.SetParameter( "levels", (int)rr_best_level );  
  params2.SetParameter( "sub_band", lowres_raster ); 
  params2.SetParameter( "sub_band_index", (int)0 );  
    
  TePDIMallatWavelets filter2;
  filter2.ToggleProgInt( progress_enabled_ );
  TEAGN_TRUE_OR_RETURN( filter2.Reset( params2 ),
    "Unable to set wavelet filter 2 Parameters" );  
  TEAGN_TRUE_OR_RETURN( filter2.Apply(), 
    "Wavelet Filter apply error" );  
   
  return true;
}

bool TePDIGarguetFusion::findBestDecimLevel( 
  const TePDITypes::TePDIRasterPtrType& reference_raster,
  const TePDITypes::TePDIRasterPtrType& lowres_raster,
  unsigned int& rr_level, 
  unsigned int& rr_level_width,
  unsigned int& rr_level_height )
{
  /* Default output values */

  rr_level = rr_level_width = rr_level_height = 0;

  /* Input checking */

  TEAGN_TRUE_OR_THROW( reference_raster.isActive(), 
    "reference_raster inactive" );
  TEAGN_TRUE_OR_THROW( lowres_raster.isActive(), 
    "lowres_raster inactive" );
    
  unsigned int rr_lines =   
    reference_raster->params().nlines_ ;
  unsigned int rr_cols = 
    reference_raster->params().ncols_;

  const unsigned long int rr_max_levels = MIN(
    TePDIMathFunctions::DecimLevels( rr_lines ),
    TePDIMathFunctions::DecimLevels( rr_cols ) );  
  
  if( rr_max_levels > 0 ) {
    double rr_resx = reference_raster->params().resx_;
    double rr_resy = reference_raster->params().resy_;
    double lr_resx = lowres_raster->params().resx_;
    double lr_resy = lowres_raster->params().resy_;
    double curr_diff_y = 0;
    double curr_diff_x = 0;
    double curr_err = 0;
    double best_err = DBL_MAX;
    
    for( unsigned long int curr_rr_level = 1 ; 
      curr_rr_level <= rr_max_levels ; 
      ++curr_rr_level ) {
      
      rr_lines = (unsigned int)
        ceil( ( (double)rr_lines ) / 2 );
      rr_cols = (unsigned int)
        ceil( ( (double)rr_cols ) / 2 );
      rr_resy *= 2.0;
      rr_resx *= 2.0;   
        
      curr_diff_y = ABS( lr_resy - rr_resy );
      curr_diff_x = ABS( lr_resx - rr_resx );

      curr_err = MAX( curr_diff_x, curr_diff_y );
          
      if( curr_err < best_err ) {
        best_err = curr_err;

        rr_level = curr_rr_level;
        rr_level_width = rr_cols;
        rr_level_height = rr_lines;
      }
    }
  }

  if( rr_level > 0 ) {
    return true;
  } else {
    return false;
  }
}

