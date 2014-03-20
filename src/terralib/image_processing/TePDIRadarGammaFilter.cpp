#include "TePDIRadarGammaFilter.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"

#include "math.h"

TePDIRadarGammaFilter::TePDIRadarGammaFilter()
{
}


TePDIRadarGammaFilter::~TePDIRadarGammaFilter()
{
}


void TePDIRadarGammaFilter::ResetState( const TePDIParameters& params )
{
  TePDIRadarFilter::ResetState( params );
}


bool TePDIRadarGammaFilter::CheckParameters( 
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

  /* Filter type checking */
  TePDIGammaFType filter_type;
  if( ! parameters.GetParameter( "filter_type", filter_type ) ) {

    TEAGN_LOGERR( "Missing parameter: filter_type" );
    return false;
  }
  if( ( filter_type != TePDIGammaFixedType ) &&
      ( filter_type != TePDIGammaAdaptType ) ) {

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

  /* Checking for detection type parameter */

  TePDIGammaDetType det_type;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "det_type", det_type ),
    "Missing parameter: det_type" );
  TEAGN_TRUE_OR_RETURN(
    ( ( det_type == TePDIGammaDTLinear ) || ( det_type == TePDIGammaDTQuadratic ) ),
    "Invalid detection type" );

  /* Checking for image look number */

  double look_number = 0;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "look_number", look_number ),
    "Missing parameter: look_number" );
  TEAGN_TRUE_OR_RETURN( look_number > 0,  "Invalid image look number" );

  /* Checking for detection type parameter */

  int mask_width = 0;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "mask_width", mask_width ),
    "Missing parameter: mask_width" );
  TEAGN_TRUE_OR_RETURN( ( ( mask_width > 2 ) && ( ( mask_width % 2 ) != 0 ) ),
    "Invalid mask width" );
  if( filter_type != TePDIGammaAdaptType ) {
    TEAGN_TRUE_OR_RETURN( ( mask_width <=  (int)max_adapt_mask_width_ ),
      "Invalid mask width" );
  }

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


bool TePDIRadarGammaFilter::RunImplementation()
{
  TePDIGammaFType filter_type;
  params_.GetParameter( "filter_type", filter_type );

  switch( filter_type ) {
    case TePDIGammaFixedType :
    {
      return RunFixedImplementation();
      break;
    }
    default :
    {
      TEAGN_LOG_AND_RETURN( "Invalid algorithm type" );
    }
  }
}

bool TePDIRadarGammaFilter::RunFixedImplementation()
{
  TePDITypes::TePDIRasterPtrType inRaster;
  params_.GetParameter( "input_image", inRaster );

  TePDITypes::TePDIRasterPtrType outRaster;
  params_.GetParameter( "output_image", outRaster );

  std::vector< int > channels;
  params_.GetParameter( "channels", channels );

  int mask_width;
  params_.GetParameter( "mask_width", mask_width );

  int iterations = 0;
  params_.GetParameter( "iterations", iterations );

  TePDIGammaDetType det_type;
  params_.GetParameter( "det_type", det_type );

  double look_number = 0;
  params_.GetParameter( "look_number", look_number );

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

  TEAGN_TRUE_OR_RETURN( outRaster->init( outRaster_params ),
    "Output raster reset error" );         

  /* Creating the temporary rasters with one band each */

  TePDITypes::TePDIRasterPtrType aux_raster1;
  TePDITypes::TePDIRasterPtrType aux_raster2;
  
  TeRasterParams auxRasterParams = inRaster->params();
  auxRasterParams.nBands( 1 );
  auxRasterParams.setDataType( TeDOUBLE, -1 );
  
  if( iterations > 1  )
  {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( auxRasterParams,
      aux_raster1 ), "Unable to create auxiliary raster 1" );
  }

  if( iterations > 2 )
  {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( auxRasterParams,
      aux_raster2 ), "Unable to create auxiliary raster 1" );
  }

  /* Noise statistics */

  double noise_deviation = 0;

  //if( det_type == TePDIGammaDTLinear ) {
  //  noise_deviation = 0.522723008 / sqrt( look_number );
  //} else if( det_type == TePDIGammaDTQuadratic ) {
    noise_deviation = 1.0 / sqrt( look_number );
  //}

  const double noise_variance = noise_deviation * noise_deviation;

  /* Setting the convolution buffer initial state */

  const unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  const unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( mask_width, raster_columns );

  /* Convolution Loop */

  double output_level; /* the value generated by each pixel mask calcule */

  unsigned int mask_middle_off_lines =
    (unsigned int) floor( ((double)mask_width) / 2. );
  unsigned int mask_middle_off_columns = mask_middle_off_lines;

  unsigned int conv_column_bound = raster_columns - mask_width + 1;
  unsigned int conv_line_bound = raster_lines - mask_width + 1;

  unsigned int raster_line; /* mask top-left line */
  unsigned int conv_buf_column; /* mask top-left column */

  unsigned int current_pixel_column; /* raster reference for the current pixel being
                                        processed */

  double channel_min_level;
  double channel_max_level;

  double mean; /* for the current window covolution inside convolution buffer */
  double variance; /* for the current window convolution inside convolution buffer */
  int sourceChannel = 0;
  int targetChannel = 0;  

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;
  
  StartProgInt( "Gamma fixed filter", channels.size() * iterations * 
    conv_line_bound ); 

  for( unsigned int channels_index = 0 ; channels_index < channels.size() ;
       ++channels_index ) 
  {
    for( int iteration = 0 ; (int)iteration < iterations ; 
      ++iteration ) 
    {
      /* Switching rasters */

      if( iteration == 0 ) 
      {
        source_raster = inRaster;
        sourceChannel = channels[ channels_index ];
        
        if( iterations > 1 )
        {
          target_raster = aux_raster1;
          targetChannel = 0;
        }
        else
        {
          target_raster = outRaster;
          targetChannel = channels_index;
        }
      }
      else if( iteration == ( iterations - 1 ) )
      {
        source_raster = target_raster;
        sourceChannel = targetChannel;
        
        target_raster = outRaster;
        targetChannel = channels_index;
      }
      else 
      {
        source_raster = target_raster;
        sourceChannel = targetChannel;
        
        if( target_raster == aux_raster1 )
        {
          target_raster = aux_raster2;
        }
        else
        {
          target_raster = aux_raster1;
        }
        targetChannel = 0;
      }
      
      TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
        target_raster, targetChannel, channel_min_level,
        channel_max_level ), "Unable to get raster channel level bounds" );      

      /* Fills the convolution buffer with the first "mask_lines" from the raster */

      for( int line = 0 ; line < ( mask_width - 1) ; ++line ) {
        up_conv_buf( source_raster, line, sourceChannel );
      }

      /* window convolution over raster */

      for( raster_line = 0 ; raster_line < conv_line_bound ; ++raster_line ) {
        /* Getting one more line from the source raster and adding to buffer */
        
        TEAGN_FALSE_OR_RETURN( UpdateProgInt( ( channels_index * iterations * 
          conv_line_bound ) +
          ( iteration * conv_line_bound ) + raster_line ),
          "Canceled by the user" );

        up_conv_buf( source_raster, raster_line + mask_width - 1,
          sourceChannel );

        for( conv_buf_column = 0 ; conv_buf_column < conv_column_bound ;
             ++conv_buf_column ) 
        {
          current_pixel_column = conv_buf_column +  mask_middle_off_columns;

          conv_buf_estatistics( 0, conv_buf_column, mask_width, mask_width, mean,
            variance );

		  double zval = conv_buf_[ mask_middle_off_lines ][ current_pixel_column ];

		  double c = sqrt(2.0) * noise_deviation;
		  double ci = sqrt(variance) / mean;

		  if( ci <= noise_deviation ) {
            output_level = mean;
		  //} else if( ( noise_deviation < ci ) && ( ci < c ) ) {
		  } else if( ( noise_deviation < ci ) && ( noise_deviation < c ) ) {
			double alpha = ( 1 + noise_variance ) / ( ( ci * ci ) - noise_variance );
			double b = alpha - look_number - 1;
			double d = ( ( mean * mean ) * ( b * b ) ) + ( 4 * alpha * look_number * mean *
			  zval );
			
			output_level = ( ( b * mean ) + sqrt( d ) ) / ( 2 * alpha );

		  } else {
			output_level = zval;
		  }

          /* Level range filtering */

          if( output_level < channel_min_level ) {
            output_level = channel_min_level;
          } else if( output_level > channel_max_level ) {
            output_level = channel_max_level;
          }

          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            current_pixel_column,
            raster_line +  mask_middle_off_lines, output_level, targetChannel ),
            "Pixel mapping error" );
        }
      }
    }
  }

  return true;
}
