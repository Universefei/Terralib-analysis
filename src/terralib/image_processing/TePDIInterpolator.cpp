#include "TePDIInterpolator.hpp"

#include "TePDIUtils.hpp"

#include "../kernel/TeAgnostic.h"
#include "../kernel/TeUtils.h"

#include <math.h>

#define BICUBIC_MODULE( x ) ( ( x < 0 ) ? ( -1 * x ) : x )
#define BICUBIC_K1( x , a ) ( ( ( a + 2 ) * x * x * x ) - \
  ( ( a + 3 ) * x * x ) + 1 )
#define BICUBIC_K2( x , a ) ( ( a * x * x * x ) - ( 5 * a * x * x ) + \
  ( 8 * a * x ) - ( 4 * a ) )
#define BICUBIC_RANGES(x,a) \
  ( ( ( 0 <= x ) && ( x <= 1 ) ) ? \
    BICUBIC_K1(x,a) \
  : ( ( ( 1 < x ) && ( x <= 2 ) ) ? \
      BICUBIC_K2(x,a) \
    : 0 ) )
#define BICUBIC_KERNEL( x , a ) BICUBIC_RANGES( BICUBIC_MODULE(x) , a )

TePDIInterpolator::TePDIInterpolator()
{
  init();
}


TePDIInterpolator::~TePDIInterpolator()
{
  clear();
}


void TePDIInterpolator::init()
{
  input_raster_params_nptr_ = 0;
  interp_funct_ptr_ = 0;
  bicubic_lines_bound_ = 0;
  bicubic_columns_bound_ = 0;
  bicubic_kernel_parameter_ = -1.0;
  last_valid_line_ = 0;
  last_valid_col_ = 0;
  raster_uses_dummy_ = false;
  nbands_ = 0;
  dummy_value_ = 0;
  input_raster_ptr_.reset();
  input_raster_nptr_ = 0;
}


void TePDIInterpolator::clear()
{
  input_raster_ptr_.reset();

  init();
}


bool TePDIInterpolator::reset( const TeSharedPtr< TeRaster >& input_raster,
  InterpMethod method, double dummy_value )
{
  clear();
  init();

  /* Checking input */

  TEAGN_TRUE_OR_RETURN( input_raster.isActive(), "Inactive input raster" );
  TEAGN_TRUE_OR_RETURN( input_raster->status(), "Input raster not ready" );
  
  if( method != NNMethod ) 
  {
    for( int band = 0 ; band < input_raster->params().nBands() ;
      ++band ) 
    {
      TEAGN_TRUE_OR_RETURN( ( 
        ( input_raster->params().photometric_[ band ] == 
        TeRasterParams::TeMultiBand ) || 
        ( input_raster->params().photometric_[ band ] == 
        TeRasterParams::TeRGB ) ),
      "Only NNMethod can be used on this raster" );
    }
  }

  /* Switching interpolation function pointer */
  
  switch( method ) {
    case BicubicMethod :
    {
      TEAGN_TRUE_OR_RETURN( ( input_raster->params().nlines_ > 3 ), 
        "Invalid raster lines number" );
      TEAGN_TRUE_OR_RETURN( ( input_raster->params().ncols_ > 3 ), 
        "Invalid raster columns number" );
    
      interp_funct_ptr_ = &TePDIInterpolator::bicubicInterpolation;
      break;
    }
    case BilinearMethod :
    {
      TEAGN_TRUE_OR_RETURN( ( input_raster->params().nlines_ > 1 ), 
        "Invalid raster lines number" );
      TEAGN_TRUE_OR_RETURN( ( input_raster->params().ncols_ > 1 ), 
        "Invalid raster columns number" );
    
      interp_funct_ptr_ = &TePDIInterpolator::bilinearInterpolation;
      break;
    }
    case NNMethod :
    {
      TEAGN_TRUE_OR_RETURN( ( input_raster->params().nlines_ > 0 ), 
        "Invalid raster lines number" );
      TEAGN_TRUE_OR_RETURN( ( input_raster->params().ncols_ > 0 ), 
        "Invalid raster columns number" );
    
      interp_funct_ptr_ = &TePDIInterpolator::nNInterpolation;
      break;
    }
    default :
    {
      TEAGN_LOG_AND_RETURN( "Invalid interpolation method" );
      break;
    }
  }
  
  input_raster_ptr_ = input_raster;
  input_raster_nptr_ = input_raster_ptr_.nakedPointer();
  input_raster_params_nptr_ = &(input_raster->params());
  raster_uses_dummy_ = input_raster_params_nptr_->useDummy_;
  nbands_ = input_raster_params_nptr_->nBands();
  dummy_value_ = dummy_value;
  
  last_valid_line_ = (double)( input_raster_params_nptr_->nlines_ - 1 );
  last_valid_col_ = (double)( input_raster_params_nptr_->ncols_ - 1 );
  
  bicubic_lines_bound_ = (double)( input_raster_params_nptr_->nlines_ - 2 );
  bicubic_columns_bound_ = (double)( input_raster_params_nptr_->ncols_ - 2 );
  
  return true;
}

void TePDIInterpolator::nNInterpolation( const double& line, const double& col, 
  const unsigned int& band, double& value )
{
  TEAGN_DEBUG_CONDITION( interp_funct_ptr_, 
    "Invalid interpolation function pointer" );
  TEAGN_DEBUG_CONDITION( ( band < nbands_ ), "Invalid band" );
    
  value = dummy_value_;
  input_raster_nptr_->getElement( TeRound( col ), TeRound( line ), value, 
    band );
}


void TePDIInterpolator::bilinearInterpolation( const double& line, 
  const double& col, const unsigned int& band, double& value )
{
  TEAGN_DEBUG_CONDITION( interp_funct_ptr_, 
    "Invalid interpolation function pointer" );
  TEAGN_DEBUG_CONDITION( ( band < nbands_ ), "Invalid band" );
  
  if( ( line < 0.0 ) || ( col < 0.0 ) || ( line > last_valid_line_ ) ||
    ( col > last_valid_col_ ) )
  {
    value = dummy_value_;
    input_raster_nptr_->getElement( TeRound( col ), TeRound( line ), 
      value, band );
  }
  else
  {
    bilinear_line_min_ = floor( line );
    bilinear_line_max_ = ( ( bilinear_line_min_ == line ) ? bilinear_line_min_ : 
      ( bilinear_line_min_ + 1.0 ) );
    bilinear_col_min_ = floor( col );
    bilinear_col_max_ = ( ( bilinear_col_min_ == col ) ? bilinear_col_min_ : 
      ( bilinear_col_min_ + 1.0 ) );
    
    bilinear_diff_line_min_ = line - bilinear_line_min_;
    bilinear_diff_line_max_ = line - bilinear_line_max_;
    bilinear_diff_col_min_ = col - bilinear_col_min_;
    bilinear_diff_col_max_ = col - bilinear_col_max_;
    
    bilinear_dist1_ = sqrt( 
      ( bilinear_diff_line_min_ * bilinear_diff_line_min_ )
      + ( bilinear_diff_col_min_ * bilinear_diff_col_min_ ) );
    bilinear_dist2_ = sqrt( 
      ( bilinear_diff_line_min_ * bilinear_diff_line_min_ )
      + ( bilinear_diff_col_max_ * bilinear_diff_col_max_ ) );
    bilinear_dist3_ = sqrt( 
      ( bilinear_diff_line_max_ * bilinear_diff_line_max_ )
      + ( bilinear_diff_col_min_ * bilinear_diff_col_min_ ) );
    bilinear_dist4_ = sqrt( 
      ( bilinear_diff_line_max_ * bilinear_diff_line_max_ )
      + ( bilinear_diff_col_max_ * bilinear_diff_col_max_ ) );
    
    bilinear_weigh1_ = ( ( bilinear_dist1_ == 0 ) ? 1 : 
      ( 1 / bilinear_dist1_ ) );
    bilinear_weigh2_ = ( ( bilinear_dist2_ == 0 ) ? 1 : 
      ( 1 / bilinear_dist2_ ) );
    bilinear_weigh3_ = ( ( bilinear_dist3_ == 0 ) ? 1 : 
      ( 1 / bilinear_dist3_ ) );
    bilinear_weigh4_ = ( ( bilinear_dist4_ == 0 ) ? 1 : 
      ( 1 / bilinear_dist4_ ) );
    
    bilinear_elem1_ = 0;
    bilinear_elem2_ = 0;
    bilinear_elem3_ = 0;
    bilinear_elem4_ = 0;
    
    if( input_raster_nptr_->getElement( (unsigned int)bilinear_col_min_, 
          (unsigned int)bilinear_line_min_, bilinear_elem1_, band ) &&
        input_raster_nptr_->getElement( (unsigned int)bilinear_col_max_, 
          (unsigned int)bilinear_line_min_, bilinear_elem2_, band ) &&        
        input_raster_nptr_->getElement( (unsigned int)bilinear_col_min_, 
          (unsigned int)bilinear_line_max_, bilinear_elem3_, band ) &&        
        input_raster_nptr_->getElement( (unsigned int)bilinear_col_max_, 
          (unsigned int)bilinear_line_max_, bilinear_elem4_, band ) ) 
    {
      value = ( ( bilinear_elem1_ * bilinear_weigh1_ ) + 
        ( bilinear_elem2_ * bilinear_weigh2_ ) +
        ( bilinear_elem3_ * bilinear_weigh3_ ) + 
        ( bilinear_elem4_ * bilinear_weigh4_ ) ) / 
        ( bilinear_weigh1_ + bilinear_weigh2_ + bilinear_weigh3_ + 
        bilinear_weigh4_ );
          
      // if the interpolated value is dummy
      // fall-back to NN interpolation          
        
      if( value == dummy_value_)
      {
        input_raster_nptr_->getElement( TeRound( col ), TeRound( line ), 
          value, band );         
      }
    } else {
      TEAGN_TRUE_OR_THROW( raster_uses_dummy_, 
        "Unable to read raster element" );  
    
      value = dummy_value_;
      input_raster_nptr_->getElement( TeRound( col ), TeRound( line ), 
        value, band );     
    }
  }
}


void TePDIInterpolator::bicubicInterpolation( const double& line, 
  const double& col, const unsigned int& band, double& value )
{
  TEAGN_DEBUG_CONDITION( interp_funct_ptr_, 
    "Invalid interpolation function pointer" );
  TEAGN_DEBUG_CONDITION( ( band < nbands_ ), "Invalid band" );
  
  if( ( col <= 1.0 ) || ( line <= 1.0 ) || ( col >= bicubic_columns_bound_ ) || 
    ( line >= bicubic_lines_bound_ ) ) 
  {
    value = dummy_value_;
    input_raster_nptr_->getElement( TeRound( col ), TeRound( line ), 
      value, band );  
  } 
  else 
  {
    bicubic_grid_raster_line_ = ( (unsigned int)floor( line ) ) - 1;
    bicubic_grid_raster_col_ = ( (unsigned int)floor( col ) ) - 1;
    
    /* retriving raster elements */
    
    for( bicubic_buffer_line_ = 0 ; bicubic_buffer_line_ < 4 ; 
      ++bicubic_buffer_line_) 
    {
      for( bicubic_buffer_col_ = 0 ; bicubic_buffer_col_ < 4 ; 
        ++bicubic_buffer_col_) 
      {
        if( ! input_raster_nptr_->getElement( 
          bicubic_grid_raster_col_ + bicubic_buffer_col_, 
          bicubic_grid_raster_line_ + bicubic_buffer_line_, 
          bicubic_buffer_[bicubic_buffer_line_][bicubic_buffer_col_], band ) )
        {
          TEAGN_DEBUG_CONDITION( raster_uses_dummy_,
            "Unable to read raster data at [" +
            Te2String( bicubic_grid_raster_col_ + bicubic_buffer_col_, 1 ) 
            + "," 
            + Te2String( bicubic_grid_raster_line_ + bicubic_buffer_line_ ) 
            + "]" );
            
          value = dummy_value_;
          input_raster_nptr_->getElement( TeRound( col ), TeRound( line ), 
            value, band );           
            
          return;
        }
      }
    }
    
    /* Bicubic weights calcule for the required position */
    
    bicubic_offset_x_ = col - (double)( bicubic_grid_raster_col_ + 1 );
    bicubic_offset_y_ = line - (double)( bicubic_grid_raster_line_ + 1 );    
    
    bicubic_h_weights_[0] = BICUBIC_KERNEL( 1.0 + bicubic_offset_x_, 
      bicubic_kernel_parameter_ );
    bicubic_h_weights_[1] = BICUBIC_KERNEL( bicubic_offset_x_, 
      bicubic_kernel_parameter_ );
    bicubic_h_weights_[2] = BICUBIC_KERNEL( 1.0 - bicubic_offset_x_, 
      bicubic_kernel_parameter_ );
    bicubic_h_weights_[3] = BICUBIC_KERNEL( 2.0 - bicubic_offset_x_, 
      bicubic_kernel_parameter_ );
      
    bicubic_v_weights_[0] = BICUBIC_KERNEL( 1.0 + bicubic_offset_y_, 
      bicubic_kernel_parameter_ );
    bicubic_v_weights_[1] = BICUBIC_KERNEL( bicubic_offset_y_, 
      bicubic_kernel_parameter_ );
    bicubic_v_weights_[2] = BICUBIC_KERNEL( 1.0 - bicubic_offset_y_, 
      bicubic_kernel_parameter_ );
    bicubic_v_weights_[3] = BICUBIC_KERNEL( 2.0 - bicubic_offset_y_, 
      bicubic_kernel_parameter_ );
      
    bicubic_h_weights_sum_ = bicubic_h_weights_[0] + bicubic_h_weights_[1] +
      bicubic_h_weights_[2] + bicubic_h_weights_[3];
    bicubic_v_weights_sum_ = bicubic_v_weights_[0] + bicubic_v_weights_[1] +
      bicubic_v_weights_[2] + bicubic_v_weights_[3];
    
    /* interpolating the value */
    
    for( bicubic_buffer_line_ = 0 ; bicubic_buffer_line_ < 4 ; 
      ++bicubic_buffer_line_) {
      
      bicubic_int_line_accum_ = 0;
      
      for( bicubic_buffer_col_ = 0 ; bicubic_buffer_col_ < 4 ; 
        ++bicubic_buffer_col_) {
        
        bicubic_int_line_accum_ += 
          bicubic_buffer_[bicubic_buffer_line_][bicubic_buffer_col_] *
          bicubic_h_weights_[ bicubic_buffer_col_ ];
      }
      
      bicubic_int_lines_values_[ bicubic_buffer_line_ ] = 
        bicubic_int_line_accum_ / bicubic_h_weights_sum_;
    }
    
    value = bicubic_int_lines_values_[ 0 ] * bicubic_v_weights_[ 0 ] +
      bicubic_int_lines_values_[ 1 ] * bicubic_v_weights_[ 1 ] +
      bicubic_int_lines_values_[ 2 ] * bicubic_v_weights_[ 2 ] +
      bicubic_int_lines_values_[ 3 ] * bicubic_v_weights_[ 3 ];
    value = value / bicubic_v_weights_sum_;
    
    // if the interpolated value is dummy
    // fall-back to NN interpolation
    
    if( value == dummy_value_)
    {
      input_raster_nptr_->getElement( TeRound( col ), TeRound( line ), 
        value, band );         
    }
  }
}


