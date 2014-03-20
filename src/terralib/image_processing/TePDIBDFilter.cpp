#include "TePDIBDFilter.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"

#include "math.h"

TePDIBDFilter::TePDIBDFilter()
{
}


TePDIBDFilter::~TePDIBDFilter()
{
}


bool TePDIBDFilter::CheckParameters( 
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
  TePDIBDType filter_type;
  if( ! parameters.GetParameter( "filter_type", filter_type ) ) {

    TEAGN_LOGERR( "Missing parameter: filter_type" );
    return false;
  }
  if( ( filter_type != TePDISobel ) &&
      ( filter_type != TePDIRoberts ) ) {

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

  /* Checking for number of iterations */
  int iterations = 0;
  if( ! parameters.GetParameter( "iterations", iterations ) ) {

    TEAGN_LOGERR( "Missing parameter: iterations" );
    return false;
  }
  TEAGN_TRUE_OR_RETURN( iterations > 0, "Invalid iterations number" );

  /* Checking photometric interpretation */
  
  for( unsigned int channel = 0 ; channel < channels.size() ; ++channel ) {
    TEAGN_TRUE_OR_RETURN( ( 
      ( inRaster->params().photometric_[ channel ] == 
        TeRasterParams::TeRGB ) ||
      ( inRaster->params().photometric_[ channel ] == 
        TeRasterParams::TeMultiBand ) ),
    "Invalid parameter - input_image (invalid photometric interpretation)" );
  }
    
  return true;
}


bool TePDIBDFilter::RunImplementation()
{
  TePDIBDType filter_type;
  params_.GetParameter( "filter_type", filter_type );

  switch( filter_type ) {
    case TePDISobel :
    {
      return RunSobel();
    }
    case TePDIRoberts :
    {
      return RunRoberts();
      break;
    }
    default :
    {
      TEAGN_LOG_AND_RETURN( "Invalid algorithm type" );
    }
  }
}


bool TePDIBDFilter::RunSobel()
{
  TePDITypes::TePDIRasterPtrType inRaster;
  params_.GetParameter( "input_image", inRaster );

  TePDITypes::TePDIRasterPtrType outRaster;
  params_.GetParameter( "output_image", outRaster );

  std::vector< int > channels;
  params_.GetParameter( "channels", channels );

  int iterations = 0;
  params_.GetParameter( "iterations", iterations );

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
  outRaster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 );
  if( inRaster->params().useDummy_ ) {
    outRaster_params.setDummy( inRaster->params().dummy_[ 0 ], -1 );
  } else {
    outRaster_params.setDummy( 0, -1 );
  }
    
  TEAGN_TRUE_OR_RETURN( outRaster->init( outRaster_params ),
    "Output raster reset error" );    
    
  /* Creating the temporary rasters */

  TePDITypes::TePDIRasterPtrType aux_raster1;
  TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( aux_raster1, 
    outRaster->params(), TePDIUtils::TePDIUtilsAutoMemPol ),
    "Unable to create auxiliary raster 1" );

  TePDITypes::TePDIRasterPtrType aux_raster2;  
  TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( aux_raster2, 
    outRaster->params(), TePDIUtils::TePDIUtilsAutoMemPol ),
    "Unable to create auxiliary raster 2" );
    
  /* Setting the convolution buffer initial state */

  unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( 3, raster_columns );

  /* Convolution Loop */

  double output_level;
  double vx, vy;

  unsigned int conv_column_bound = raster_columns - 2;
  unsigned int conv_line_bound = raster_lines - 2;

  unsigned int raster_line;
  unsigned int conv_buf_column;

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;

  double channel_min_level;
  double channel_max_level;
  
  StartProgInt( "Sobel filter", channels.size() * iterations *
     conv_line_bound );

  for( unsigned int channels_index = 0 ;
       channels_index < channels.size() ;
       ++channels_index ) {

    TEAGN_TRUE_OR_THROW(
      TePDIUtils::TeCopyRasterPixels( inRaster, channels[ channels_index ],
      aux_raster1, 0, false, false ), "Auxiliar Raster fill error" );

    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
      aux_raster1, 0, channel_min_level,
      channel_max_level ), "Unable to get raster channel level bounds" );

    for( unsigned int iteration = 0 ; (int)iteration < iterations ; ++iteration ) {
      /* Switching from input and output raster for more than 1 iteration */

      if( iteration % 2 == 0 ) {
        source_raster = aux_raster1;
        target_raster = aux_raster2;
      } else {
        source_raster = aux_raster2;
        target_raster = aux_raster1;
      }

      /* Fills the convolution buffer with the first "mask_lines" from the raster */

      for( unsigned int line = 0 ; line < 2 ; ++line ) {
        up_conv_buf( source_raster, line, 0 );
      }

      /* raster convolution */

      for( raster_line = 0 ; raster_line < conv_line_bound ; ++raster_line ) {
        /* Getting one more line from the source raster and adding to buffer */
        
        TEAGN_FALSE_OR_RETURN( UpdateProgInt( ( channels_index * iterations * 
          conv_line_bound ) + ( iteration * conv_line_bound ) + raster_line ),
          "Canceled by the user" );

        up_conv_buf( source_raster, raster_line + 2, 0 );

        for( conv_buf_column = 0 ; conv_buf_column < conv_column_bound ;
             ++conv_buf_column ) {

          vx = conv_buf_[ 2 ][ conv_buf_column ] +
            ( 2 * conv_buf_[ 2 ][ conv_buf_column + 1 ] ) +
            conv_buf_[ 2 ][ conv_buf_column + 2 ] -
            conv_buf_[ 0 ][ conv_buf_column ] -
            ( 2 * conv_buf_[ 0 ][ conv_buf_column + 1 ] ) -
            conv_buf_[ 0 ][ conv_buf_column + 2 ];

          vy = conv_buf_[ 0 ][ conv_buf_column + 2 ] +
            ( 2 * conv_buf_[ 1 ][ conv_buf_column + 2 ] ) +
            conv_buf_[ 2 ][ conv_buf_column + 2 ] -
            conv_buf_[ 0 ][ conv_buf_column ] -
            ( 2 * conv_buf_[ 1 ][ conv_buf_column ] ) -
            conv_buf_[ 2 ][ conv_buf_column ];

          output_level = sqrt( ( vx * vx ) + ( vy * vy ) );

          /* Level range filtering */

          if( output_level < channel_min_level ) {
            output_level = channel_min_level;
          } else if( output_level > channel_max_level ) {
            output_level = channel_max_level;
          }

          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            conv_buf_column + 1, raster_line +  1, output_level, 0 ),
            "Pixel mapping error" );
        }

      }
    }

    /* Content copy from the target raster to the output raster */

    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeCopyRasterPixels( target_raster, 0,
      outRaster, channels_index, false, false ), "Output raster fill error" );
  }

  return true;
}


bool TePDIBDFilter::RunRoberts()
{
  TePDITypes::TePDIRasterPtrType inRaster;
  params_.GetParameter( "input_image", inRaster );

  TePDITypes::TePDIRasterPtrType outRaster;
  params_.GetParameter( "output_image", outRaster );

  std::vector< int > channels;
  params_.GetParameter( "channels", channels );

  int iterations = 0;
  params_.GetParameter( "iterations", iterations );

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
  outRaster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 );    
  if( inRaster->params().useDummy_ ) {
    outRaster_params.setDummy( inRaster->params().dummy_[ 0 ], -1 );
  } else {
    outRaster_params.setDummy( 0, -1 );
  }
  
  TEAGN_TRUE_OR_RETURN( outRaster->init( outRaster_params ),
    "Output raster reset error" );     
    
  /* Creating the temporary rasters */

  TePDITypes::TePDIRasterPtrType aux_raster1;
  TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( aux_raster1, 
    outRaster->params(), TePDIUtils::TePDIUtilsAutoMemPol ),
    "Unable to create auxiliary raster 1" );

  TePDITypes::TePDIRasterPtrType aux_raster2;  
  TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( aux_raster2, 
    outRaster->params(), TePDIUtils::TePDIUtilsAutoMemPol ),
    "Unable to create auxiliary raster 2" );    

  /* Setting the convolution buffer initial state */

  unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( 3, raster_columns );

  /* Convolution Loop */

  double output_level;
  double diff1, diff2;

  unsigned int conv_column_bound = raster_columns - 2;
  unsigned int conv_line_bound = raster_lines - 2;

  unsigned int raster_line;
  unsigned int conv_buf_column;

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;
  
  StartProgInt( "Roberts filter", channels.size() * iterations *
     conv_line_bound );  

  for( unsigned int channels_index = 0 ;
       channels_index < channels.size() ;
       ++channels_index ) {

    TEAGN_TRUE_OR_THROW(
      TePDIUtils::TeCopyRasterPixels( inRaster, channels[ channels_index ],
      aux_raster1, 0, false, false ), "Auxiliar Raster fill error" );

    double channel_min_level;
    double channel_max_level;
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
      aux_raster1, 0, channel_min_level,
      channel_max_level ), "Unable to get raster channel level bounds" );

    for( unsigned int iteration = 0 ; (int)iteration < iterations ; ++iteration ) {
      /* Switching from input and output raster for more than 1 iteration */

      if( iteration % 2 == 0 ) {
        source_raster = aux_raster1;
        target_raster = aux_raster2;
      } else {
        source_raster = aux_raster2;
        target_raster = aux_raster1;
      }

      /* Fills the convolution buffer with the first "mask_lines" from the raster */

      for( unsigned int line = 0 ; line < 2 ; ++line ) {
        up_conv_buf( source_raster, line, 0 );
      }

      /* raster convolution */

      for( raster_line = 0 ; raster_line < conv_line_bound ; ++raster_line ) {
        /* Getting one more line from the source raster and adding to buffer */
        
        TEAGN_FALSE_OR_RETURN( UpdateProgInt( ( channels_index * iterations * 
          conv_line_bound ) + ( iteration * conv_line_bound ) + raster_line ),
          "Canceled by the user" );

        up_conv_buf( source_raster, raster_line + 2, 0 );

        for( conv_buf_column = 0 ; conv_buf_column < conv_column_bound ;
             ++conv_buf_column ) {

          output_level = 0;

          diff1 = conv_buf_[ 1 ][ conv_buf_column + 1 ] -
            conv_buf_[ 2 ][ conv_buf_column + 2 ];

          diff2 = conv_buf_[ 2 ][ conv_buf_column + 1 ] -
            conv_buf_[ 1 ][ conv_buf_column + 2 ];

          output_level = sqrt( ( diff1 * diff1 ) + ( diff2 * diff2 ) );

          /* Level range filtering */

          if( output_level < channel_min_level ) {
            output_level = channel_min_level;
          } else if( output_level > channel_max_level ) {
            output_level = channel_max_level;
          }

          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            conv_buf_column + 1, raster_line +  1, output_level, 0 ),
            "Pixel mapping error" );
        }
      }
    }

    /* Content copy from the target raster to the output raster */

    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeCopyRasterPixels( target_raster, 0,
      outRaster, channels_index, false, false ), "Output raster fill error" );
  }

  return true;
}


void TePDIBDFilter::ResetState( const TePDIParameters& params )
{
  TePDIBufferedFilter::ResetState( params );
}


