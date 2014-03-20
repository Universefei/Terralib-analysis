#include "TePDIVenturaFusion.hpp"
#include "TePDIUtils.hpp"
#include "TePDIMallatWavelets.hpp"
#include "TePDIMatrixFunctions.hpp"

TePDIVenturaFusion::TePDIVenturaFusion()
{
}

TePDIVenturaFusion::~TePDIVenturaFusion()
{
}


void TePDIVenturaFusion::ResetState( const TePDIParameters& )
{
}


bool TePDIVenturaFusion::CheckParameters( const TePDIParameters& parameters ) 
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


bool TePDIVenturaFusion::RunImplementation()
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
    
  /* Finding the best decomposition levels for both rasters */
  
  unsigned long int reference_raster_best_level = 0;
  unsigned long int lowres_raster_best_level = 0;
  unsigned long int decim_error = 0;
  findBestDecimLevels( reference_raster, lowres_raster, 
    reference_raster_best_level, lowres_raster_best_level, decim_error );
  
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
  
  /* Step 1 - pyramid1 generation  */
  
  TePDITypes::TePDIRasterVectorPtrType pyramid1( new
    TePDITypes::TePDIRasterVectorType );
  
  TePDIParameters step1_params;
  step1_params.SetParameter( "wavelets_type", std::string( "mallat" ) );
  step1_params.SetParameter( "filter_task", std::string( "GetPyramid" ) );
  step1_params.SetParameter( "input_image", lowres_raster );
  step1_params.SetParameter( "band", lowres_raster_band );
  step1_params.SetParameter( "a_filter_l", a_filter_l );
  step1_params.SetParameter( "a_filter_h", a_filter_h );
  step1_params.SetParameter( "s_filter_l", s_filter_l );
  step1_params.SetParameter( "s_filter_h", s_filter_h );
  step1_params.SetParameter( "filters_scale", filters_scale );         
  step1_params.SetParameter( "levels", (int)lowres_raster_best_level );
  step1_params.SetParameter( "pyramid", pyramid1 );
  
  TePDIMallatWavelets wavelet_filter;
  wavelet_filter.ToggleProgInt( progress_enabled_ );  
  
  TEAGN_TRUE_OR_THROW( wavelet_filter.Reset( step1_params ),
    "Unable to set wavelet filter Parameters ( step 1 )" );
  TEAGN_TRUE_OR_THROW( wavelet_filter.Apply(), 
    "Wavelet Filter apply error ( step 1 )" );  
  
  step1_params.Clear();
  
  /* Step 2 - pyramid2 generation */
  
  TePDITypes::TePDIRasterVectorPtrType pyramid2( new
    TePDITypes::TePDIRasterVectorType );
  
  TePDIParameters step2_params;
  step2_params.SetParameter( "wavelets_type", std::string( "mallat" ) );
  step2_params.SetParameter( "filter_task", std::string( "GetPyramid" ) );
  step2_params.SetParameter( "input_image", reference_raster );
  step2_params.SetParameter( "band", (int)reference_raster_band );
  step2_params.SetParameter( "a_filter_l", a_filter_l );
  step2_params.SetParameter( "a_filter_h", a_filter_h );
  step2_params.SetParameter( "s_filter_l", s_filter_l );
  step2_params.SetParameter( "s_filter_h", s_filter_h );
  step2_params.SetParameter( "filters_scale", filters_scale );         
  step2_params.SetParameter( "levels", (int)reference_raster_best_level );      
  step2_params.SetParameter( "pyramid", pyramid2 );
  
  TEAGN_TRUE_OR_THROW( wavelet_filter.Reset( step2_params ),
    "Unable to set wavelet filter Parameters ( step 2 )" );
  TEAGN_TRUE_OR_THROW( wavelet_filter.Apply(), 
    "Wavelet Filter apply error ( step 2 )" );  
  
  step2_params.Clear();
  
  TEAGN_DEBUG_CONDITION( ( pyramid1->size() / 4 ) == 
    lowres_raster_best_level, 
    "Invalid pyramid" );
  TEAGN_DEBUG_CONDITION( ( pyramid2->size() / 4 ) == 
    reference_raster_best_level, 
    "Invalid pyramid" );
  TEAGN_DEBUG_CONDITION(
    ( (*pyramid1)[ pyramid1->size() - 4 ]->params().nlines_ ==
    (*pyramid2)[ pyramid2->size() - 4 ]->params().nlines_ ),
    "Pyramids lines mismatch" );
  TEAGN_DEBUG_CONDITION(
    ( (*pyramid1)[ pyramid1->size() - 4 ]->params().ncols_ ==
    (*pyramid2)[ pyramid2->size() - 4 ]->params().ncols_ ),
    "Pyramids columns mismatch" );  
  
  /* Step 3 - Pyramid2 enhencement */
  
  TePDIMatrix< double > sb1matrix;
  TEAGN_TRUE_OR_RETURN( sb1matrix.Reset( 0, 0, 
    TePDIMatrix< double >::AutoMemPol ), "Matrix reset error" );
  
  TePDIMatrix< double > sb2matrix;
  TEAGN_TRUE_OR_RETURN( sb2matrix.Reset( 0, 0, 
    TePDIMatrix< double >::AutoMemPol ), "Matrix reset error" );    
    
  const unsigned int pys_levels_bound = MIN( reference_raster_best_level,
    lowres_raster_best_level );
  unsigned int py1_sb00_index = 0;
  unsigned int py2_sb00_index = 0;
  
  TePDIPIManager progress( "Pyramid enhencement", pys_levels_bound, 
    progress_enabled_ );
  
  for( unsigned int pys_levels_off = 0 ; /* low-low skipping */
       pys_levels_off < pys_levels_bound ;
       ++pys_levels_off ) {
       
    py1_sb00_index = pyramid1->size() - ( 4 * ( pys_levels_off + 1 ) );
    py2_sb00_index = pyramid2->size() - ( 4 * ( pys_levels_off + 1 ) );
    
    TEAGN_TRUE_OR_RETURN( RasterBand2Matrix( (*pyramid1)[ py1_sb00_index ], 
      sb1matrix ), "Cannot convert sub-band from pyramid1" );
    TEAGN_TRUE_OR_RETURN( RasterBand2Matrix( (*pyramid2)[ py2_sb00_index ], 
      sb2matrix ), "Cannot convert sub-band from pyramid2" );    
       
    double sb00_correlation = TePDIMatrixFunctions::GetCorrelation( sb1matrix,
      sb2matrix );
      
    TePDIMatrix< double > absmatrix1;
    TEAGN_TRUE_OR_RETURN( absmatrix1.Reset( 0, 0, 
      TePDIMatrix< double >::AutoMemPol ), "Matrix reset error" );         
      
    TePDIMatrix< double > absmatrix2; 
    TEAGN_TRUE_OR_RETURN( absmatrix2.Reset( 0, 0, 
      TePDIMatrix< double >::AutoMemPol ), "Matrix reset error" );           
           
    TePDIMatrix< double > gtmatrix;
    TEAGN_TRUE_OR_RETURN( gtmatrix.Reset( 0, 0, 
      TePDIMatrix< double >::AutoMemPol ), "Matrix reset error" );
                  
    TePDIMatrix< double > notmatrix;  
    TEAGN_TRUE_OR_RETURN( notmatrix.Reset( 0, 0, 
      TePDIMatrix< double >::AutoMemPol ), "Matrix reset error" );          
      
    for( unsigned int subband = 1 ; subband < 4 ; ++subband ) {
      TEAGN_TRUE_OR_RETURN( RasterBand2Matrix( 
        (*pyramid1)[ py1_sb00_index + subband ], 
        sb1matrix ), "Cannot convert sub-band from pyramid1" );
      TEAGN_TRUE_OR_RETURN( RasterBand2Matrix( 
        (*pyramid2)[ py2_sb00_index + subband ], 
        sb2matrix ), "Cannot convert sub-band from pyramid2" );       
        
      TEAGN_TRUE_OR_RETURN( 
        TePDIMatrixFunctions::Abs( sb1matrix, absmatrix1 ), 
        "Abs1 matrix calcule error" );
      TEAGN_TRUE_OR_RETURN( 
        TePDIMatrixFunctions::Abs( sb2matrix, absmatrix2 ), 
        "Abs2 matrix calcule error" );
      TEAGN_TRUE_OR_RETURN( 
        TePDIMatrixFunctions::WhereIsGreater( absmatrix2, absmatrix1, 
        gtmatrix ), "GT matrix calcule error" );        
      TEAGN_TRUE_OR_RETURN( 
        TePDIMatrixFunctions::Negate( gtmatrix, notmatrix ), 
        "NOT matrix calcule error" );
        
      const unsigned int valid_lines = MIN( sb1matrix.GetLines(),
        sb2matrix.GetLines() );
      const unsigned int valid_cols = MIN( sb2matrix.GetColumns(),
        sb2matrix.GetColumns() );
      const unsigned int sb2matrix_lines = sb2matrix.GetLines();
      const unsigned int sb2matrix_cols = sb2matrix.GetColumns();
      unsigned int line = 0;
      unsigned int col = 0;
      TeRaster* outraster_ptr = 
        (*pyramid2)[ py2_sb00_index + subband ].nakedPointer();
      bool result = false;
      double value1 = 0;
      double value2 = 0;
      
      for( line = 0 ; line < sb2matrix_lines ; ++line ) {
        for( col = 0 ; col < sb2matrix_cols ; ++col ) {
          if( ( line < valid_lines ) && ( col < valid_cols ) ) {
            value1 = ( gtmatrix( line, col ) * sb1matrix( line, col ) ) +
              ( notmatrix( line, col ) * sb2matrix( line, col ) );
            value2 = ( notmatrix( line, col ) * sb1matrix( line, col ) ) +
              ( gtmatrix( line, col ) * sb2matrix( line, col ) );              
          
            result = outraster_ptr->setElement( col, line, 
              ( value1 + ( ABS( 1 - sb00_correlation ) * value2 ) ), 0 );
            TEAGN_DEBUG_CONDITION( result, "Raster write error" );
          } else {
            result = outraster_ptr->setElement( col, line, 0.0, 0 );
            TEAGN_DEBUG_CONDITION( result, "Raster write error" );
          }
        }
      }
    }

    TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
  }
  
  progress.Toggle( false );
  
  sb1matrix.clear();
  sb2matrix.clear();
  
  /* Step 4 - Low-low swap */
  
  TePDITypes::TePDIRasterPtrType py1_raster = (*pyramid1)[ pyramid1->size() - 4 ];
  TePDITypes::TePDIRasterPtrType py2_raster = 
    (*pyramid2)[ pyramid2->size() - 4 ];
  const unsigned int py_lines_bound = py1_raster->params().nlines_;
  const unsigned int py_cols_bound = py1_raster->params().ncols_;
  double value = 0;
  unsigned int py_line;
  unsigned int py_col;
  bool get_result = false;
  bool set_result = false;
  
  for( py_line = 0 ; py_line < py_lines_bound ; ++py_line ) {
    for( py_col = 0 ; py_col < py_cols_bound ; ++py_col ) {
      get_result = py1_raster->getElement( py_col, py_line, value, 0 );
      
      TEAGN_DEBUG_CONDITION( get_result, 
        "Unable to read from low resolution pyramid" );
        
      set_result = py2_raster->setElement( py_col, py_line, value, 0 );
        
      TEAGN_DEBUG_CONDITION( set_result, 
        "Unable to write to high resolution pyramid" );
    }
  }
  
  pyramid1->clear();
  
  /* Step 5 - Pyramid 2 recomposition */
  
  TePDIParameters step5_params;
  step5_params.SetParameter( "wavelets_type", std::string( "mallat" ) );
  step5_params.SetParameter( "filter_task", std::string( "RecomposePyramid" ) );
  step5_params.SetParameter( "input_image", reference_raster );
  step5_params.SetParameter( "band", (int)reference_raster_band );
  step5_params.SetParameter( "output_image", output_raster );
  step5_params.SetParameter( "a_filter_l", a_filter_l );
  step5_params.SetParameter( "a_filter_h", a_filter_h );
  step5_params.SetParameter( "s_filter_l", s_filter_l );
  step5_params.SetParameter( "s_filter_h", s_filter_h );
  step5_params.SetParameter( "filters_scale", filters_scale );         
  step5_params.SetParameter( "pyramid", pyramid2 );      
  
  TEAGN_TRUE_OR_THROW( wavelet_filter.Reset( step5_params ),
    "Unable to set wavelet filter Parameters ( step 5 )" );
  TEAGN_TRUE_OR_THROW( wavelet_filter.Apply(), 
    "Wavelet Filter apply error ( step 5 )" );
     
  return true;
}

bool TePDIVenturaFusion::RasterBand2Matrix( 
  const TePDITypes::TePDIRasterPtrType& raster,
  TePDIMatrix< double >& matrix )
{
  TEAGN_TRUE_OR_RETURN( raster.isActive(), "Inactive raster" );
  
  unsigned int lines_bound = raster->params().nlines_;
  unsigned int cols_bound = raster->params().ncols_;
  unsigned int line = 0;
  unsigned int col = 0;
  double value = 0;
  bool result = false;
  
  TEAGN_TRUE_OR_RETURN( matrix.Reset( lines_bound, cols_bound ),
    "Matrix reset error" );
    
  TeRaster* raster_ptr = raster.nakedPointer();
  
  for( line = 0 ; line < lines_bound ; ++line ) {
    for( col = 0 ; col < cols_bound ; ++col ) {
      result = raster_ptr->getElement( col, line, value, 0 );
      TEAGN_DEBUG_CONDITION( result, "Unable to read from raster" );
      
      matrix( line, col ) = value;
    }
  }
  
  return true;
}
        
     
bool TePDIVenturaFusion::findBestDecimLevels( 
  const TePDITypes::TePDIRasterPtrType& raster1,
  const TePDITypes::TePDIRasterPtrType& raster2,
  unsigned long int& r1_level, unsigned long int& r2_level,
  unsigned long int& decim_error )
{
  TEAGN_TRUE_OR_RETURN( raster1.isActive(), "raster1 inactive" );
  TEAGN_TRUE_OR_RETURN( raster2.isActive(), "raster2 inactive" );

  const unsigned long int r1_lines = raster1->params().nlines_;
  const unsigned long int r1_cols = raster1->params().ncols_;
  const unsigned long int r2_lines = raster2->params().nlines_;
  const unsigned long int r2_cols = raster2->params().ncols_;

  const unsigned long int r1_max_levels = MIN(
    TePDIMathFunctions::DecimLevels( r1_lines ),
    TePDIMathFunctions::DecimLevels( r1_cols ) );  
  const unsigned long int r2_max_levels = MIN(
    TePDIMathFunctions::DecimLevels( r2_lines ),
    TePDIMathFunctions::DecimLevels( r2_cols ) );
  
  if( ( r1_max_levels < 1 ) || ( r2_max_levels < 1 ) ) {
    r1_level = r2_level = 0;
    return false;
  } else {
    bool best_level_diff_found = false;
    
    unsigned long int curr_diff = 0;
    long int curr_r1_lines = r1_lines;
    long int curr_r1_cols = r1_cols;
    long int curr_r2_lines = 0;
    long int curr_r2_cols = 0;
    
    for( unsigned long int curr_r1_level = 1 ; 
      curr_r1_level <= r1_max_levels ; 
      ++curr_r1_level ) {
      
      curr_r1_lines = ( long int )
        ceil( ( (double)curr_r1_lines ) / 2 );
      curr_r1_cols = ( long int )
        ceil( ( (double)curr_r1_cols ) / 2 );      
        
      curr_r2_lines = r2_lines;
      curr_r2_cols = r2_cols;        
    
      for( unsigned long int curr_r2_level = 1 ; 
        curr_r2_level <= r2_max_levels ; 
        ++curr_r2_level ) {
      
        curr_r2_lines = ( long int )
          ceil( ( (double)curr_r2_lines ) / 2 );
        curr_r2_cols = ( long int )
          ceil( ( (double)curr_r2_cols ) / 2 );
          
        curr_diff = ABS( curr_r1_lines - curr_r2_lines ) +
          ABS( curr_r1_cols - curr_r2_cols );
          
        if( best_level_diff_found ) {
          if( curr_diff < decim_error ) {
            r1_level = curr_r1_level;
            r2_level = curr_r2_level;
            decim_error = curr_diff;
          }
        } else {
          r1_level = curr_r1_level;
          r2_level = curr_r2_level;
          decim_error = curr_diff;
          best_level_diff_found = true;
        }
      }
    }
    
    return true;
  }
}        

