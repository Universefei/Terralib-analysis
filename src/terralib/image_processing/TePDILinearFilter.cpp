#include "TePDILinearFilter.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"

#include "../kernel/TeUtils.h"

#include "math.h"

TePDILinearFilter::TePDILinearFilter()
{
}


TePDILinearFilter::~TePDILinearFilter()
{
}


bool TePDILinearFilter::CheckParameters( 
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
  if( inRaster->params().status_ == TeRasterParams::TeReadyToWrite ) {

    TEAGN_LOGERR( "Invalid parameter: output_image not ready" );
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

  /* Checking for number of iterations */
  int iterations = 0;
  if( ! parameters.GetParameter( "iterations", iterations ) ) {

    TEAGN_LOGERR( "Missing parameter: iterations" );
    return false;
  }
  TEAGN_TRUE_OR_RETURN( iterations > 0, "Invalid iterations number" );

  /* Checking photometric interpretation */
  
  for( unsigned int channels_index = 0 ; channels_index < channels.size() ; 
    ++channels_index ) {
    
    TEAGN_TRUE_OR_RETURN( ( 
      ( inRaster->params().photometric_[ channels[ channels_index ] ] == 
        TeRasterParams::TeRGB ) ||
      ( inRaster->params().photometric_[ channels[ channels_index ] ] == 
        TeRasterParams::TeMultiBand ) ),
    "Invalid parameter - input_image "
    "(invalid photometric interpretation)" );
  }  
  
  return true;
}


bool TePDILinearFilter::RunImplementation()
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

  /* checking for the optional parameter level offset */

  double level_offset;
  if( params_.CheckParameter< double >( "level_offset" ) ) {
    
    params_.GetParameter( "level_offset", level_offset );
  } else {
    level_offset = 0;
  }

  /* Resetting the output raster */
  
  TeRasterParams outRaster_params = outRaster->params();
  
  outRaster_params.nBands( channels.size() );
  if( inRaster->projection() != 0 ) {
    outRaster_params.projection( inRaster->projection() );
  }
  outRaster_params.boundingBoxLinesColumns( 
    inRaster->params().boundingBox().x1(), 
    inRaster->params().boundingBox().y1(), 
    inRaster->params().boundingBox().x2(), 
    inRaster->params().boundingBox().y2(), 
    inRaster->params().nlines_,
    inRaster->params().ncols_ );
  if( inRaster->params().useDummy_ ) {
    outRaster_params.setDummy( inRaster->params().dummy_[ 0 ], -1 );
  } else {
    outRaster_params.setDummy( 0, -1 );
  }
  outRaster_params.setPhotometric( TeRasterParams::TeMultiBand );
  
  TEAGN_TRUE_OR_RETURN( outRaster->init( outRaster_params ),
    "Output raster reset error" );       

  /* Creating the temporary rasters with one band each */

  TePDITypes::TePDIRasterPtrType aux_raster1;
  TePDITypes::TePDIRasterPtrType aux_raster2;
  {
    TeRasterParams aux_raster_params = inRaster->params();
    aux_raster_params.nBands( channels.size() );
    
    if( iterations > 1 ) {
      TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( aux_raster1, 
        aux_raster_params, TePDIUtils::TePDIUtilsAutoMemPol ), 
        "Unable to create auxiliary raster 1" );
    }
  
    if( iterations > 2 ) {
      TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( aux_raster2, 
        aux_raster_params, TePDIUtils::TePDIUtilsAutoMemPol ), 
        "Unable to create auxiliary raster 2" );      
    }        
  }
  
  /* Updating the global temporary representation of mask weights */

  reset_maskmatrix( mask );

  /* Setting the convolution buffer initial state */

  unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( temp_maskmatrix_lines_, raster_columns );

  /* Convolution Loop */

  double output_level;

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

  double out_channel_min_level = 0;
  double out_channel_max_level = 0;
  
  double dummy_value = 0;

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;
  
  TePDIPIManager progress( "Linear filter", channels.size() * iterations *
     conv_line_bound, progress_enabled_ );  
     
  unsigned int current_input_channel =0;
  unsigned int curr_out_channel = 0;     

  for( int iteration = 0 ; (int)iteration < iterations ; ++iteration ) {
    /* Defining the source and target rasters */
      
    if( iteration == 0 ) {
      /* The first iteration */
      
      source_raster = inRaster;
      
      if( iterations > 1 ) {
        target_raster = aux_raster1;
      } else {
        target_raster = outRaster;
      }
    } else if ( iteration == ( iterations - 1 ) ) {
      /* The last iteration */
      
      source_raster = target_raster;        
      target_raster = outRaster;
    } else {
      /* The intermediary iteration */
      
      if( iteration == 1 ) {
        source_raster = target_raster;
        target_raster = aux_raster2;
      } else {
        TePDITypes::TePDIRasterPtrType swap_ptr = source_raster;
        source_raster = target_raster;
        target_raster = swap_ptr;
      }
    }
    
    for( unsigned int channels_index = 0 ; channels_index < channels.size() ;
      ++channels_index ) {
       
      /* Defining the source channel and target channel */
        
      if( iteration == 0 ) {
        /* The first iteration */
        
        current_input_channel = channels[ channels_index ];
        curr_out_channel = channels_index;
      } else if ( iteration == ( iterations - 1 ) ) {
        /* The last iteration */
        
        current_input_channel = channels_index;
        curr_out_channel = channels_index;
      } else {
        /* The intermediary iteration */

        current_input_channel = channels_index;
        curr_out_channel = channels_index;
      }        
          
      if( inRaster_uses_dummy ) {
        dummy_value = inRaster->params().dummy_[ current_input_channel ];
      }

      TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
        outRaster, channels_index, out_channel_min_level,
        out_channel_max_level ), "Unable to get raster channel level bounds" ); 
       

      /* Fills the convolution buffer with the first "mask_lines" from the 
         raster */

      for( unsigned int line = 0 ; line < ( temp_maskmatrix_lines_ - 1 ); 
        ++line ) {
        
        up_conv_buf( source_raster, line, current_input_channel );
      }

      /* raster convolution */

      for( raster_line = 0 ; raster_line < conv_line_bound ; ++raster_line ) {
        /* Getting one more line from the source raster and adding to buffer */
        
        TEAGN_FALSE_OR_RETURN( progress.Increment(),
          "Canceled by the user" );
          
        up_conv_buf( source_raster, raster_line + temp_maskmatrix_lines_ - 1,
          current_input_channel );

        for( conv_buf_column = 0 ; conv_buf_column < conv_column_bound ;
             ++conv_buf_column ) {

          output_level = level_offset;

          for( mask_line = 0; mask_line < temp_maskmatrix_lines_ ; ++mask_line ) {
            for( mask_column = 0; mask_column < temp_maskmatrix_columns_ ;
                 ++mask_column ) {
              output_level += temp_maskmatrix_[ mask_line ][ mask_column ] *
                conv_buf_[ mask_line ][ conv_buf_column + mask_column ];
            }
          }

          /* Level range filtering */

          if( output_level < out_channel_min_level ) {
            output_level = out_channel_min_level;
          } else if( output_level > out_channel_max_level ) {
            output_level = out_channel_max_level;
          }

          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            conv_buf_column + mask_middle_off_columns,
            raster_line +  mask_middle_off_lines, output_level, 
            curr_out_channel ),
            "Pixel mapping error at line=" + 
            Te2String( raster_line +  mask_middle_off_lines ) + " column=" +
            Te2String( conv_buf_column + mask_middle_off_columns ) 
            + " channel=" + Te2String( curr_out_channel ) +
            " iteration=" + Te2String( iteration ) );
        }
      }
    }
  }

  return true;
}


void TePDILinearFilter::ResetState( const TePDIParameters& params )
{
  TePDIBufferedFilter::ResetState( params );
}


