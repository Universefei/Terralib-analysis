#include "TePDIMorfFilter.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIFilterMask.hpp"
#include "TePDIUtils.hpp"

#include "math.h"


TePDIMorfFilter::TePDIMorfFilter()
{
  double_buffer_ = 0;
}


TePDIMorfFilter::~TePDIMorfFilter()
{
  if( double_buffer_ ) {
    delete[] double_buffer_;
  }
}


bool TePDIMorfFilter::CheckParameters( 
  const TePDIParameters& parameters ) const
{
  /* Checking for general required parameters */

  TePDITypes::TePDIRasterPtrType inRaster;
  if( ! parameters.GetParameter( "input_image", inRaster ) ) {

    TEAGN_LOGERR( "Missing parameter: input_image" );
    return false;
  }
  if( ! inRaster.isActive() ) {

    TEAGN_LOGERR( "Invalid parameter: input_image inactive" );
    return false;
  }
  if( inRaster->params().status_ == TeRasterParams::TeNotReady ) {

    TEAGN_LOGERR( "Invalid parameter: input_image not ready" );
    return false;
  }

  TePDITypes::TePDIRasterPtrType outRaster;
  if( ! parameters.GetParameter( "output_image", outRaster ) ) {

    TEAGN_LOGERR( "Missing parameter: output_image" );
    return false;
  }
  if( ! outRaster.isActive() ) {

    TEAGN_LOGERR( "Invalid parameter: output_image inactive" );
    return false;
  }
  if( inRaster->params().status_ == TeRasterParams::TeNotReady ) {

    TEAGN_LOGERR( "Invalid parameter: output_image not ready" );
    return false;
  }

  /* Filter type checking */
  TePDIMorfType filter_type;
  if( ! parameters.GetParameter( "filter_type", filter_type ) ) {

    TEAGN_LOGERR( "Missing parameter: filter_type" );
    return false;
  }
  if( ( filter_type != TePDIMDilationType ) &&
      ( filter_type != TePDIMErosionType ) &&
      ( filter_type != TePDIMMedianType ) &&
      ( filter_type != TePDIMModeType ) ) {

    TEAGN_LOGERR( "Invalid parameter: filter_type" );
    return false;
  }

  /* channels parameter checking */

  std::vector< int > channels;
  if( ! parameters.GetParameter( "channels", channels ) ) {

    TEAGN_LOGERR( "Missing parameter: channels" );
    return false;
  }
  for( unsigned int index = 0 ; index < channels.size() ; ++index ) {
    if( channels[ index ] >= inRaster->nBands() ) {
      TEAGN_LOGERR( "Invalid parameter: channels" );
      return false;
    }
  }

  /* Filter mask checking */

  TePDIFilterMask::pointer mask;

  if( ! parameters.GetParameter( "filter_mask", mask ) ) {

    TEAGN_LOGERR( "Missing parameter: filter_mask" );
    return false;
  }
  if( ! mask.isActive() ) {

    TEAGN_LOGERR( "Invalid parameter: filter_mask inactive" );
    return false;
  }
  if( mask->columns() < 3 ) {
    TEAGN_WATCH( mask->columns() );
    TEAGN_LOGERR( "Invalid parameter: filter_mask with invalid colunms number" );
    return false;
  }
  if( mask->lines() < 3 ) {
    TEAGN_WATCH( mask->lines() );
    TEAGN_LOGERR( "Invalid parameter: filter_mask with invalid lines number" );
    return false;
  }
  if( ( (int)mask->lines() > inRaster->params().nlines_ ) ||
      ( (int)mask->columns() > inRaster->params().ncols_ ) ){
    TEAGN_LOGERR( "Invalid parameter: filter_mask do not fit inside input image" );
    return false;
  }
  TEAGN_TRUE_OR_RETURN(
    mask->isMorfMask(), "Invalid mask: Not a morfological mask" );

  /* Checking for number of iterations */

  int iterations = 0;
  if( ! parameters.GetParameter( "iterations", iterations ) ) {

    TEAGN_LOGERR( "Missing parameter: iterations" );
    return false;
  }
  TEAGN_TRUE_OR_RETURN( iterations > 0, "Invalid iterations number" );

  return true;
}


bool TePDIMorfFilter::RunImplementation()
{
  TePDIMorfType filter_type;
  params_.GetParameter( "filter_type", filter_type );

  switch( filter_type ) {
    case TePDIMDilationType :
    {
      return RunDilation();
      break;
    }
    case TePDIMErosionType :
    {
      return RunErosion();
      break;
    }
    case TePDIMMedianType :
    {
      return RunMedian();
      break;
    }
    case TePDIMModeType :
    {
      return RunMode();
      break;
    }    
    default :
    {
      TEAGN_LOG_AND_RETURN( "Invalid algorithm type" );
    }
  }
}


bool TePDIMorfFilter::RunDilation()
{
  TePDITypes::TePDIRasterPtrType inRaster;
  params_.GetParameter( "input_image", inRaster );

  TePDITypes::TePDIRasterPtrType outRaster;
  params_.GetParameter( "output_image", outRaster );

  std::vector< int > channels;
  params_.GetParameter( "channels", channels );

  TePDIFilterMask::pointer mask;
  params_.GetParameter( "filter_mask", mask );

  int iterations = 0;
  params_.GetParameter( "iterations", iterations );
  
  bool inRaster_uses_dummy = inRaster->params().useDummy_;

  /* Setting the output raster */

  TeRasterParams outRaster_params = outRaster->params();
  
  outRaster_params.nBands( channels.size() );
  if( inRaster->projection() != 0 ) {
    TeSharedPtr< TeProjection > proj( TeProjectionFactory::make( 
      inRaster->projection()->params() ) );  
    outRaster_params.projection( proj.nakedPointer() );
  }
  outRaster_params.boxResolution( inRaster->params().box().x1(), 
    inRaster->params().box().y1(), inRaster->params().box().x2(), 
    inRaster->params().box().y2(), inRaster->params().resx_, 
    inRaster->params().resy_ ); 
    
  if( inRaster->params().photometric_[0] == TeRasterParams::TePallete)
	  outRaster_params.setPhotometric( TeRasterParams::TePallete, -1 );
  else
	  outRaster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 ); 
  
  if( inRaster->params().useDummy_ ) {
    outRaster_params.setDummy( inRaster->params().dummy_[ 0 ], -1 );
  } else {
    outRaster_params.setDummy( 0, -1 );
  }  

  TEAGN_TRUE_OR_RETURN( outRaster->init( outRaster_params ),
    "Output raster reset error" );    

  /* Creating the temporary rasters with one band each */

  TePDITypes::TePDIRasterPtrType aux_raster1;
  if( iterations > 1 ) {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRaster, 
      aux_raster1, 1, true, false, TeDOUBLE ), 
      "Unable to create auxiliary raster 1" );
  }

  TePDITypes::TePDIRasterPtrType aux_raster2;
  if( iterations > 2 ) {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRaster, 
      aux_raster2, 1, true, false, TeDOUBLE ), 
      "Unable to create auxiliary raster 2" );
  }

  /* Updating the global temporary representation of mask weights */

  reset_maskmatrix( mask );

  /* Setting the convolution buffer initial state */

  unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( temp_maskmatrix_lines_, raster_columns );

  /* Convolution Loop */
  
  double output_level = 0; /* the value generated by each pixel mask calcule */

  double max_gen_level = 0;/* The maximum value generated by each mask convolution */

  unsigned int mask_middle_off_lines =
    (unsigned int) floor( ((double)temp_maskmatrix_lines_) / 2. );
  unsigned int mask_middle_off_columns =
    (unsigned int) floor( ((double)temp_maskmatrix_columns_) / 2. );

  unsigned int conv_column_bound = raster_columns - temp_maskmatrix_columns_ + 1;
  unsigned int conv_line_bound = raster_lines - temp_maskmatrix_lines_ + 1;

  unsigned int mask_line = 0;
  unsigned int mask_column = 0;
  unsigned int raster_line = 0;
  unsigned int conv_buf_column = 0;

  double out_channel_min_level = 0;
  double out_channel_max_level = 0;
  
  double dummy_value = 0;
  double curr_buf_value = 0;

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;
  
  StartProgInt( "Dilation", channels.size() * iterations *
     conv_line_bound );  

  for( unsigned int channels_index = 0 ;
       channels_index < channels.size() ;
       ++channels_index ) {
       
    unsigned int current_input_channel =0;
    unsigned int curr_out_channel = 0;
       
    if( inRaster_uses_dummy ) {
      dummy_value = inRaster->params().dummy_[ current_input_channel ];
    }

    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
      outRaster, channels_index, out_channel_min_level,
      out_channel_max_level ), "Unable to get raster channel level bounds" );

    for( int iteration = 0 ; (int)iteration < iterations ; ++iteration ) {
      if( iteration == 0 ) {
        /* The first iteration */
        
        source_raster = inRaster;
        current_input_channel = channels[ channels_index ];
        
        if( iterations > 1 ) {
          target_raster = aux_raster1;
          curr_out_channel = 0;
        } else {
          target_raster = outRaster;
          curr_out_channel = channels[ channels_index ];
        }
      } else if ( iteration == ( iterations - 1 ) ) {
        /* The last iteration */
        
        current_input_channel = 0;
        source_raster = target_raster;
        
        target_raster = outRaster;
        curr_out_channel = channels[ channels_index ];
      } else {
        /* The intermediary iteration */
        
        if( iteration == 1 ) {
          source_raster = target_raster;
          current_input_channel = 0;
        
          target_raster = aux_raster1;
          curr_out_channel = 0;
        } else {        
          TePDITypes::TePDIRasterPtrType swap_ptr = source_raster;
          source_raster = target_raster;
          current_input_channel = 0;
            
          target_raster = swap_ptr;
          curr_out_channel = 0;
        }
      }

      /* Fills the convolution buffer with the first "mask_lines" from the raster */

      for( unsigned int line = 0 ; line < ( temp_maskmatrix_lines_ - 1 ); 
        ++line ) {
        
        up_conv_buf( source_raster, line, current_input_channel );
      }

      /* raster convolution */

      for( raster_line = 0 ; raster_line < conv_line_bound ; ++raster_line ) {
        /* Getting one more line from the source raster and adding to buffer */

        TEAGN_FALSE_OR_RETURN( UpdateProgInt( ( channels_index * iterations * 
          conv_line_bound ) +
          ( iteration * conv_line_bound ) + raster_line ),
          "Canceled by the user" );
        
        up_conv_buf( source_raster, raster_line + temp_maskmatrix_lines_ - 1,
          current_input_channel );

        for( conv_buf_column = 0 ; conv_buf_column < conv_column_bound ;
             ++conv_buf_column ) {

          output_level = 0;
          
          if( inRaster_uses_dummy ) {
            max_gen_level = dummy_value;
            
            for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; 
              ++mask_line ) {
              for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                  ++mask_column ) {
                  
                curr_buf_value = conv_buf_[ mask_line ][ conv_buf_column + 
                  mask_column ];
                  
                if( curr_buf_value != dummy_value ) {
                  output_level = temp_maskmatrix_[ mask_line ][ mask_column ] *
                    curr_buf_value;
                    
                  if( max_gen_level == dummy_value ) {
                    max_gen_level = output_level;
                  } else {
                    if( output_level > max_gen_level ) {
                      max_gen_level = output_level;
                    }
                  }
                }
              }
            }
          } else {
            max_gen_level = out_channel_min_level;
            
            for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; 
              ++mask_line ) {
              
              for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                  ++mask_column ) {
                output_level = temp_maskmatrix_[ mask_line ][ mask_column ] *
                  conv_buf_[ mask_line ][ conv_buf_column + mask_column ];
  
                if( output_level > max_gen_level ) {
                  max_gen_level = output_level;
                }
              }
            }
          }

          /* Level range filtering */

          if( max_gen_level < out_channel_min_level ) {
            max_gen_level = out_channel_min_level;
          } else if( max_gen_level > out_channel_max_level ) {
            max_gen_level = out_channel_max_level;
          }

          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            conv_buf_column + mask_middle_off_columns,
            raster_line +  mask_middle_off_lines, max_gen_level, 
            curr_out_channel ), "Pixel mapping error" );
        }
      }
    }
  }

  return true;

}

bool TePDIMorfFilter::RunErosion()
{
  TePDITypes::TePDIRasterPtrType inRaster;
  params_.GetParameter( "input_image", inRaster );

  TePDITypes::TePDIRasterPtrType outRaster;
  params_.GetParameter( "output_image", outRaster );

  std::vector< int > channels;
  params_.GetParameter( "channels", channels );

  TePDIFilterMask::pointer mask;
  params_.GetParameter( "filter_mask", mask );

  int iterations = 0;
  params_.GetParameter( "iterations", iterations );
  
  bool inRaster_uses_dummy = inRaster->params().useDummy_;

  /* Setting the output raster */

  TeRasterParams outRaster_params = outRaster->params();
  
  outRaster_params.nBands( channels.size() );
  if( inRaster->projection() != 0 ) {
    TeSharedPtr< TeProjection > proj( TeProjectionFactory::make( 
      inRaster->projection()->params() ) );  
    outRaster_params.projection( proj.nakedPointer() );
  }
  outRaster_params.boxResolution( inRaster->params().box().x1(), 
    inRaster->params().box().y1(), inRaster->params().box().x2(), 
    inRaster->params().box().y2(), inRaster->params().resx_, 
    inRaster->params().resy_ );
    
  if( inRaster->params().photometric_[0] == TeRasterParams::TePallete)
	  outRaster_params.setPhotometric( TeRasterParams::TePallete, -1 );
  else
	  outRaster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 );   
  
  if( inRaster->params().useDummy_ ) {
    outRaster_params.setDummy( inRaster->params().dummy_[ 0 ], -1 );
  } else {
    outRaster_params.setDummy( 0, -1 );
  }  

  TEAGN_TRUE_OR_RETURN( outRaster->init( outRaster_params ),
    "Output raster reset error" );    

  /* Creating the temporary rasters with one band each */

  TePDITypes::TePDIRasterPtrType aux_raster1;
  if( iterations > 1 ) {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRaster, 
      aux_raster1, 1, true, false, TeDOUBLE ), 
      "Unable to create auxiliary raster 1" );
  }

  TePDITypes::TePDIRasterPtrType aux_raster2;
  if( iterations > 2 ) {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRaster, 
      aux_raster2, 1, true, false, TeDOUBLE ), 
      "Unable to create auxiliary raster 2" );
  }

  /* Updating the global temporary representation of mask weights */

  reset_maskmatrix( mask );

  /* Setting the convolution buffer initial state */

  unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( temp_maskmatrix_lines_, raster_columns );

  /* Convolution Loop */

  double output_level; /* the value generated by each pixel mask calcule */

  double min_gen_level; /* The minimal value generated by each mask convolution */

  unsigned int mask_middle_off_lines =
    (unsigned int) floor( ((double)temp_maskmatrix_lines_) / 2. );
  unsigned int mask_middle_off_columns =
    (unsigned int) floor( ((double)temp_maskmatrix_columns_) / 2. );

  unsigned int conv_column_bound = raster_columns - temp_maskmatrix_columns_ + 1;
  unsigned int conv_line_bound = raster_lines - temp_maskmatrix_lines_ + 1;

  unsigned int mask_line;
  unsigned int mask_column;
  unsigned int raster_line;
  unsigned int conv_buf_column;

  double out_channel_min_level;
  double out_channel_max_level;
  
  double dummy_value = 0;
  double curr_buf_value = 0;
  double curr_mask_value = 0;

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;

  StartProgInt( "Erosion", channels.size() * iterations *
     conv_line_bound );  

  for( unsigned int channels_index = 0 ;
       channels_index < channels.size() ;
       ++channels_index ) {
       
    unsigned int current_input_channel = channels[ channels_index ];
    unsigned int curr_out_channel = 0;
    
    if( inRaster_uses_dummy ) {
      dummy_value = inRaster->params().dummy_[ current_input_channel ];
    }    

    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
      outRaster, channels_index, out_channel_min_level,
      out_channel_max_level ), "Unable to get raster channel level bounds" );

    for( int iteration = 0 ; (int)iteration < iterations ; ++iteration ) {
      if( iteration == 0 ) {
        /* The first iteration */
        
        source_raster = inRaster;
        current_input_channel = channels[ channels_index ];
        
        if( iterations > 1 ) {
          target_raster = aux_raster1;
          curr_out_channel = 0;
        } else {
          target_raster = outRaster;
          curr_out_channel = channels[ channels_index ];
        }
      } else if ( iteration == ( iterations - 1 ) ) {
        /* The last iteration */
        
        current_input_channel = 0;
        source_raster = target_raster;
        
        target_raster = outRaster;
        curr_out_channel = channels[ channels_index ];
      } else {
        /* The intermediary iteration */
        
        if( iteration == 1 ) {
          source_raster = target_raster;
          current_input_channel = 0;
        
          target_raster = aux_raster1;
          curr_out_channel = 0;
        } else {        
          TePDITypes::TePDIRasterPtrType swap_ptr = source_raster;
          source_raster = target_raster;
          current_input_channel = 0;
            
          target_raster = swap_ptr;
          curr_out_channel = 0;
        }
      }

      /* Fills the convolution buffer with the first "mask_lines" from the raster */

      for( unsigned int line = 0 ; line < ( temp_maskmatrix_lines_ - 1 ); ++line ) {
        up_conv_buf( source_raster, line, current_input_channel );
      }

      /* raster convolution */

      for( raster_line = 0 ; raster_line < conv_line_bound ; ++raster_line ) {
        /* Getting one more line from the source raster and adding to buffer */

        TEAGN_FALSE_OR_RETURN( UpdateProgInt( ( channels_index * iterations * 
          conv_line_bound ) +
          ( iteration * conv_line_bound ) + raster_line ),
          "Canceled by the user" );
                  
        up_conv_buf( source_raster, raster_line + temp_maskmatrix_lines_ - 1,
          current_input_channel );

        for( conv_buf_column = 0 ; conv_buf_column < conv_column_bound ;
             ++conv_buf_column ) {

          output_level = 0;
          
          if( inRaster_uses_dummy ) {
            min_gen_level = dummy_value;
            
            for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; 
              ++mask_line ) {
              for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                  ++mask_column ) {
                  
                curr_buf_value = conv_buf_[ mask_line ][ conv_buf_column + 
                  mask_column ];
                curr_mask_value = temp_maskmatrix_[ mask_line ][ mask_column ];
                  
                if( ( curr_buf_value != dummy_value ) &&
                  ( curr_mask_value != 0 ) ) {
                  
                  output_level = curr_buf_value * curr_mask_value;;
                 
                  if( min_gen_level == dummy_value ) {
                    min_gen_level = output_level;
                  } else {
                    if( output_level < min_gen_level ) {
                      min_gen_level = output_level;
                    }
                  } 
                }
              }
            }
          } else {
            min_gen_level = out_channel_max_level;
            
            for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; 
              ++mask_line ) {
              for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                  ++mask_column ) {
  
                output_level = temp_maskmatrix_[ mask_line ][ mask_column ] *
                  conv_buf_[ mask_line ][ conv_buf_column + mask_column ];
  
                if( ( output_level < min_gen_level ) &&
                    ( temp_maskmatrix_[ mask_line ][ mask_column ] != 0 ) ) {
  
                  min_gen_level = output_level;
                }
              }
            }
          }

          /* Level range filtering */

          if( min_gen_level < out_channel_min_level ) {
            min_gen_level = out_channel_min_level;
          } else if( min_gen_level > out_channel_max_level ) {
            min_gen_level = out_channel_max_level;
          }

          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            conv_buf_column + mask_middle_off_columns,
            raster_line +  mask_middle_off_lines, min_gen_level, 
            curr_out_channel ), "Pixel mapping error" );
        }
      }
    }
  }

  return true;
}

bool TePDIMorfFilter::RunMedian()
{
  TePDITypes::TePDIRasterPtrType inRaster;
  params_.GetParameter( "input_image", inRaster );

  TePDITypes::TePDIRasterPtrType outRaster;
  params_.GetParameter( "output_image", outRaster );

  std::vector< int > channels;
  params_.GetParameter( "channels", channels );

  TePDIFilterMask::pointer mask;
  params_.GetParameter( "filter_mask", mask );

  int iterations = 0;
  params_.GetParameter( "iterations", iterations );
  
  bool inRaster_uses_dummy = inRaster->params().useDummy_;

  /* Setting the output raster */

  TeRasterParams outRaster_params = outRaster->params();
  
  outRaster_params.nBands( channels.size() );
  if( inRaster->projection() != 0 ) {
    TeSharedPtr< TeProjection > proj( TeProjectionFactory::make( 
      inRaster->projection()->params() ) );  
    outRaster_params.projection( proj.nakedPointer() );
  }
  outRaster_params.boxResolution( inRaster->params().box().x1(), 
    inRaster->params().box().y1(), inRaster->params().box().x2(), 
    inRaster->params().box().y2(), inRaster->params().resx_, 
    inRaster->params().resy_ );  
  
  if( inRaster->params().photometric_[0] == TeRasterParams::TePallete)
	  outRaster_params.setPhotometric( TeRasterParams::TePallete, -1 );
  else
	  outRaster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 ); 
  
  if( inRaster->params().useDummy_ ) {
    outRaster_params.setDummy( inRaster->params().dummy_[ 0 ], -1 );
  } else {
    outRaster_params.setDummy( 0, -1 );
  }  

  TEAGN_TRUE_OR_RETURN( outRaster->init( outRaster_params ),
    "Output raster reset error" );    
    

  /* Creating the temporary rasters with one band each */

  TePDITypes::TePDIRasterPtrType aux_raster1;
  if( iterations > 1 ) {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRaster, 
      aux_raster1, 1, true, false, TeDOUBLE ), 
      "Unable to create auxiliary raster 1" );
  }

  TePDITypes::TePDIRasterPtrType aux_raster2;
  if( iterations > 2 ) {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRaster, 
      aux_raster2, 1, true, false, TeDOUBLE ), 
      "Unable to create auxiliary raster 2" );
  }

  /* Updating the global temporary representation of mask weights */

  reset_maskmatrix( mask );

  /* Setting the convolution buffer initial state */

  unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( temp_maskmatrix_lines_, raster_columns );

  /* A vector to store the mask apply results for a given convolution */

  if( double_buffer_ ) {
    delete[] double_buffer_;
  }

  double_buffer_ =
    new double[ temp_maskmatrix_lines_ * temp_maskmatrix_columns_ ];

//  const unsigned int mask_apply_restult_median_pos =
//    ( temp_maskmatrix_lines_ * temp_maskmatrix_columns_ ) -
//    (int)ceil( mask->getSum() / 2 );

  /* Bubble sort alg parameters */

//  unsigned int fixed_bubble_end_bound =
//    ( temp_maskmatrix_lines_ * temp_maskmatrix_columns_ ) - 1;/* not included */
  unsigned int temp_bubble_bound; // the position before the last valid 
                                  // element index inside double buf
  unsigned int bubble_index;
  double temp_bubble_swap;

  /* Finding mask middle pixel offset */

  unsigned int mask_middle_off_lines =
    (unsigned int) floor( ((double)temp_maskmatrix_lines_) / 2. );
  unsigned int mask_middle_off_columns =
    (unsigned int) floor( ((double)temp_maskmatrix_columns_) / 2. );

  unsigned int conv_column_bound = raster_columns - temp_maskmatrix_columns_ + 1;
  unsigned int conv_line_bound = raster_lines - temp_maskmatrix_lines_ + 1;

  unsigned int mask_line;
  unsigned int mask_column;
  unsigned int raster_line;
  unsigned int conv_buf_column;

  double out_channel_min_level;
  double out_channel_max_level;
  
  unsigned int next_db_element_index = 0; // next double buffer element index,
                                          // points to the position after the
                                          // last valid buffer element.
  double curr_mask_value = 0;
  double curr_cb_value = 0; // current convolution buffer value
  double dummy_value = 0;

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;
  
  StartProgInt( "Median", channels.size() * iterations *
     conv_line_bound );    

  for( unsigned int channels_index = 0 ;
       channels_index < channels.size() ;
       ++channels_index ) {
       
    unsigned int current_input_channel = channels[ channels_index ];
    unsigned int curr_out_channel = 0;
       
    if( inRaster_uses_dummy ) {
      dummy_value = inRaster->params().dummy_[ current_input_channel ];
    }        

    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
      outRaster, channels_index, out_channel_min_level,
      out_channel_max_level ), "Unable to get raster channel level bounds" );

    for( int iteration = 0 ; (int)iteration < iterations ; ++iteration ) {
      if( iteration == 0 ) {
        /* The first iteration */
        
        source_raster = inRaster;
        current_input_channel = channels[ channels_index ];
        
        if( iterations > 1 ) {
          target_raster = aux_raster1;
          curr_out_channel = 0;
        } else {
          target_raster = outRaster;
          curr_out_channel = channels[ channels_index ];
        }
      } else if ( iteration == ( iterations - 1 ) ) {
        /* The last iteration */
        
        current_input_channel = 0;
        source_raster = target_raster;
        
        target_raster = outRaster;
        curr_out_channel = channels[ channels_index ];
      } else {
        /* The intermediary iteration */
        
        if( iteration == 1 ) {
          source_raster = target_raster;
          current_input_channel = 0;
        
          target_raster = aux_raster1;
          curr_out_channel = 0;
        } else {        
          TePDITypes::TePDIRasterPtrType swap_ptr = source_raster;
          source_raster = target_raster;
          current_input_channel = 0;
            
          target_raster = swap_ptr;
          curr_out_channel = 0;
        }
      }

      /* Fills the convolution buffer with the first "mask_lines" from the raster */

      for( unsigned int line = 0 ; line < ( temp_maskmatrix_lines_ - 1 ) ; ++line ) {
        up_conv_buf( source_raster, line, current_input_channel );
      }

      /* raster convolution */

      for( raster_line = 0 ; raster_line < conv_line_bound ; ++raster_line ) {
        /* Getting one more line from the source raster and adding to buffer */
        
        TEAGN_FALSE_OR_RETURN( UpdateProgInt( ( channels_index * iterations * 
          conv_line_bound ) +
          ( iteration * conv_line_bound ) + raster_line ),
          "Canceled by the user" );

        up_conv_buf( source_raster, raster_line + temp_maskmatrix_lines_ - 1,
          current_input_channel );

        for( conv_buf_column = 0 ; conv_buf_column < conv_column_bound ;
             ++conv_buf_column ) {

          /* Finding convolution weights apply result */
          
          next_db_element_index = 0;
          
          if( inRaster_uses_dummy ) {
            for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; ++mask_line ) {
              for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                  ++mask_column ) {
                  
                curr_mask_value = temp_maskmatrix_[ mask_line ][ mask_column ];
                curr_cb_value = conv_buf_[ mask_line ][ conv_buf_column + 
                  mask_column ];
                  
                if( ( curr_mask_value != 0 ) && 
                  ( curr_cb_value != dummy_value ) ) {
                  
                  double_buffer_[ next_db_element_index ] =
                    curr_mask_value * curr_cb_value;
                    
                  ++next_db_element_index;
                }
              }
            }
          } else {
            for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; ++mask_line ) {
              for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                  ++mask_column ) {
                  
                curr_mask_value = temp_maskmatrix_[ mask_line ][ mask_column ];
                  
                if( curr_mask_value != 0 ) {
                  double_buffer_[ next_db_element_index ] =
                    curr_mask_value *
                    conv_buf_[ mask_line ][ conv_buf_column + mask_column ];
                    
                  ++next_db_element_index;
                }
              }
            }
          }

          if( next_db_element_index == 0 ) {
            TEAGN_TRUE_OR_RETURN( target_raster->setElement(
              conv_buf_column + mask_middle_off_columns,
              raster_line +  mask_middle_off_lines, dummy_value, 
              curr_out_channel ), "Pixel mapping error" );           
          } else if( next_db_element_index == 1 ) {
            TEAGN_TRUE_OR_RETURN( target_raster->setElement(
              conv_buf_column + mask_middle_off_columns,
              raster_line +  mask_middle_off_lines,
              double_buffer_[ 0 ], curr_out_channel ),
              "Pixel mapping error" );          
          } else {
            /* Sorting the results */

            temp_bubble_bound = next_db_element_index - 1;
  
            while( temp_bubble_bound > 0 ) {
              for( bubble_index = 0 ; bubble_index < temp_bubble_bound ;
                ++bubble_index ) {
  
                if( double_buffer_[ bubble_index ] >
                    double_buffer_[ bubble_index + 1 ] ) {
  
                    temp_bubble_swap = double_buffer_[ bubble_index ];
                    double_buffer_[ bubble_index ] =
                      double_buffer_[ bubble_index + 1 ];
                    double_buffer_[ bubble_index + 1 ] = temp_bubble_swap;
                }
              }
  
              --temp_bubble_bound;
            }
  
            TEAGN_TRUE_OR_RETURN( target_raster->setElement(
              conv_buf_column + mask_middle_off_columns,
              raster_line +  mask_middle_off_lines,
              double_buffer_[ ( next_db_element_index - 1 ) / 2 ], 
              curr_out_channel ), "Pixel mapping error" );
          }
        }
      }
    }
  }

  return true;
}


bool TePDIMorfFilter::RunMode()
{
  TePDITypes::TePDIRasterPtrType inRaster;
  params_.GetParameter( "input_image", inRaster );

  TePDITypes::TePDIRasterPtrType outRaster;
  params_.GetParameter( "output_image", outRaster );

  std::vector< int > channels;
  params_.GetParameter( "channels", channels );

  TePDIFilterMask::pointer mask;
  params_.GetParameter( "filter_mask", mask );

  int iterations = 0;
  params_.GetParameter( "iterations", iterations );
  
  bool inRaster_uses_dummy = inRaster->params().useDummy_;

  /* Setting the output raster */

  TeRasterParams outRaster_params = outRaster->params();
  
  outRaster_params.nBands( channels.size() );
  if( inRaster->projection() != 0 ) {
    TeSharedPtr< TeProjection > proj( TeProjectionFactory::make( 
      inRaster->projection()->params() ) );  
    outRaster_params.projection( proj.nakedPointer() );
  }
  outRaster_params.boxResolution( inRaster->params().box().x1(), 
    inRaster->params().box().y1(), inRaster->params().box().x2(), 
    inRaster->params().box().y2(), inRaster->params().resx_, 
    inRaster->params().resy_ );  
    
  if( inRaster->params().photometric_[0] == TeRasterParams::TePallete)
	  outRaster_params.setPhotometric( TeRasterParams::TePallete, -1 );
  else
	  outRaster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 ); 
  
  if( inRaster->params().useDummy_ ) {
    outRaster_params.setDummy( inRaster->params().dummy_[ 0 ], -1 );
  } else {
    outRaster_params.setDummy( 0, -1 );
  }  

  TEAGN_TRUE_OR_RETURN( outRaster->init( outRaster_params ),
    "Output raster reset error" );    
    

  /* Creating the temporary rasters with one band each */

  TePDITypes::TePDIRasterPtrType aux_raster1;
  if( iterations > 1 ) {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRaster, 
      aux_raster1, 1, true, false, TeDOUBLE ), 
      "Unable to create auxiliary raster 1" );
  }

  TePDITypes::TePDIRasterPtrType aux_raster2;
  if( iterations > 2 ) {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( outRaster, 
      aux_raster2, 1, true, false, TeDOUBLE ), 
      "Unable to create auxiliary raster 2" );
  }

  /* Updating the global temporary representation of mask weights */

  reset_maskmatrix( mask );

  /* Setting the convolution buffer initial state */

  unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( temp_maskmatrix_lines_, raster_columns );

  /* Bubble sort alg parameters */

  std::map< double, unsigned int > freqmap;
  std::map< double, unsigned int >::iterator freqmap_it;
  std::map< double, unsigned int >::iterator freqmap_it_end;
  double freqmap_highest_freq_value = 0;
  unsigned int freqmap_highest_freq = 0;

  /* Finding mask middle pixel offset */

  unsigned int mask_middle_off_lines =
    (unsigned int) floor( ((double)temp_maskmatrix_lines_) / 2. );
  unsigned int mask_middle_off_columns =
    (unsigned int) floor( ((double)temp_maskmatrix_columns_) / 2. );

  unsigned int conv_column_bound = raster_columns - temp_maskmatrix_columns_ + 1;
  unsigned int conv_line_bound = raster_lines - temp_maskmatrix_lines_ + 1;

  unsigned int mask_line = 0;
  unsigned int mask_column = 0;
  unsigned int raster_line = 0;
  unsigned int raster_col = 0;
  unsigned int conv_buf_column = 0;

  double out_channel_min_level = 0;
  double out_channel_max_level = 0;
  
//  unsigned int next_db_element_index = 0; // next double buffer element index,
                                          // points to the position after the
                                          // last valid buffer element.
//  double curr_mask_value = 0;
//  double curr_cb_value = 0; // current convolution buffer value
  double dummy_value = 0;
  double pixel_value = 0;
  double mask_apply_result = 0.0;

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;
  
  StartProgInt( "Mede", channels.size() * iterations *
     conv_line_bound );    

  for( unsigned int channels_index = 0 ;
       channels_index < channels.size() ;
       ++channels_index ) {
       
    unsigned int current_input_channel = channels[ channels_index ];
    unsigned int curr_out_channel = 0;
       
    if( inRaster_uses_dummy ) {
      dummy_value = inRaster->params().dummy_[ current_input_channel ];
    }
    else
    {
      dummy_value = 0;
    }

    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
      outRaster, channels_index, out_channel_min_level,
      out_channel_max_level ), 
      "Unable to get raster channel level bounds" );

    for( int iteration = 0 ; (int)iteration < iterations ; 
      ++iteration ) 
    {
      if( iteration == 0 ) {
        /* The first iteration */
        
        source_raster = inRaster;
        current_input_channel = channels[ channels_index ];
        
        if( iterations > 1 ) {
          target_raster = aux_raster1;
          curr_out_channel = 0;
        } else {
          target_raster = outRaster;
          curr_out_channel = channels[ channels_index ];
        }
      } else if ( iteration == ( iterations - 1 ) ) {
        /* The last iteration */
        
        current_input_channel = 0;
        source_raster = target_raster;
        
        target_raster = outRaster;
        curr_out_channel = channels[ channels_index ];
      } else {
        /* The intermediary iteration */
        
        if( iteration == 1 ) {
          source_raster = target_raster;
          current_input_channel = 0;
        
          target_raster = aux_raster1;
          curr_out_channel = 0;
        } else {        
          TePDITypes::TePDIRasterPtrType swap_ptr = source_raster;
          source_raster = target_raster;
          current_input_channel = 0;
            
          target_raster = swap_ptr;
          curr_out_channel = 0;
        }
      }
      
      /* Copying the first and the last cols image pixels */
      
      for( raster_line = 0 ; raster_line < raster_lines ; 
        ++raster_line ) 
      {
        if( source_raster->getElement( 0, raster_line, pixel_value,
          current_input_channel ) )
        {
          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            0, raster_line, pixel_value, curr_out_channel ), 
            "Pixel writing error" );        
        }
        else
        {
          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            0, raster_line, dummy_value, curr_out_channel ), 
            "Pixel writing error" );        
        }
        
        if( source_raster->getElement( raster_columns - 1, 
          raster_line, pixel_value,
          current_input_channel ) )
        {
          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            raster_columns - 1, raster_line, pixel_value, 
            curr_out_channel ), 
            "Pixel writing error" );        
        }
        else
        {
          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            raster_columns - 1, raster_line, dummy_value, 
            curr_out_channel ), 
            "Pixel writing error" );        
        }        
      }      
      
      /* Copying the first and the last line image pixels */
      
      for( raster_col = 0 ; raster_col < raster_columns ; 
        ++raster_col ) 
      {
        if( source_raster->getElement( raster_col, 0, pixel_value,
          current_input_channel ) )
        {
          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            raster_col, 0, pixel_value, curr_out_channel ), 
            "Pixel writing error" );        
        }
        else
        {
          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            raster_col, 0, dummy_value, curr_out_channel ), 
            "Pixel writing error" );        
        }
        
        if( source_raster->getElement( raster_col, 
          raster_lines - 1, pixel_value,
          current_input_channel ) )
        {
          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            raster_col, raster_lines - 1, pixel_value, 
            curr_out_channel ), 
            "Pixel writing error" );        
        }
        else
        {
          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            raster_col, raster_lines - 1, dummy_value, 
            curr_out_channel ), 
            "Pixel writing error" );        
        }        
      }       

      /* Fills the convolution buffer with the first "mask_lines" from the raster */

      for( unsigned int line = 0 ; line < ( temp_maskmatrix_lines_ - 1 ) ; ++line ) {
        up_conv_buf( source_raster, line, current_input_channel );
      }
      
      /* raster convolution */

      for( raster_line = 0 ; raster_line < conv_line_bound ; 
        ++raster_line ) 
      {
        /* Getting one more line from the source raster and adding to buffer */
        
        TEAGN_FALSE_OR_RETURN( UpdateProgInt( 
          ( channels_index * iterations * 
          conv_line_bound ) +
          ( iteration * conv_line_bound ) + raster_line ),
          "Canceled by the user" );

        up_conv_buf( source_raster, raster_line + temp_maskmatrix_lines_ - 1,
          current_input_channel );

        for( conv_buf_column = 0 ; conv_buf_column < 
          conv_column_bound ; ++conv_buf_column ) 
        {
          freqmap.clear();
          
          /* Finding convolution weights apply result */
          
          if( inRaster_uses_dummy ) {
            for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; ++mask_line ) {
              for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                  ++mask_column ) {
                  
                const double& curr_mask_value = 
                  temp_maskmatrix_[ mask_line ][ mask_column ];
                const double& curr_cb_value = 
                  conv_buf_[ mask_line ][ conv_buf_column + 
                  mask_column ];
                
                  
                if( ( curr_mask_value != 0 ) && 
                  ( curr_cb_value != dummy_value ) ) {
                  
                  mask_apply_result = curr_mask_value *
                    curr_cb_value;
                    
                  freqmap[ mask_apply_result ] += 1;
                }
              }
            }
          } else {
            for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; ++mask_line ) {
              for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                  ++mask_column ) {
                  
                const double& curr_mask_value = 
                  temp_maskmatrix_[ mask_line ][ mask_column ];
                  
                if( curr_mask_value != 0 ) {
                  freqmap[ conv_buf_[ mask_line ][ 
                    conv_buf_column + mask_column ] ] += 1;
                }
              }
            }
          }

          if( freqmap.size() == 0 ) {
            TEAGN_TRUE_OR_RETURN( target_raster->setElement(
              conv_buf_column + mask_middle_off_columns,
              raster_line +  mask_middle_off_lines, dummy_value, 
              curr_out_channel ), "Pixel mapping error" );           
          } 
          else 
          {
            /* Finding the high frequency value */
            freqmap_highest_freq = 0;
            freqmap_it = freqmap.begin();
            freqmap_it_end = freqmap.end();
            
            while( freqmap_it != freqmap_it_end )
            {
              if( freqmap_it->second > freqmap_highest_freq )
              {
                freqmap_highest_freq = freqmap_it->second;
                freqmap_highest_freq_value = freqmap_it->first;
              }
            
              ++freqmap_it;
            }
            
            TEAGN_TRUE_OR_RETURN( target_raster->setElement(
              conv_buf_column + mask_middle_off_columns,
              raster_line +  mask_middle_off_lines,
              freqmap_highest_freq_value, curr_out_channel ),
              "Pixel mapping error" );          
          }
        }
      }
    }
  }

  return true;
}


void TePDIMorfFilter::ResetState( const TePDIParameters& params )
{
  TePDIBufferedFilter::ResetState( params );

  if( double_buffer_ ) {
    delete[] double_buffer_;
    double_buffer_ = 0;
  }
}


