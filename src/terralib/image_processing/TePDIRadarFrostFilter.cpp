#include "TePDIRadarFrostFilter.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"

#include "math.h"

TePDIRadarFrostFilter::TePDIRadarFrostFilter()
{
}


TePDIRadarFrostFilter::~TePDIRadarFrostFilter()
{
}


void TePDIRadarFrostFilter::ResetState( const TePDIParameters& params )
{
  TePDIRadarFilter::ResetState( params );
}


bool TePDIRadarFrostFilter::CheckParameters( 
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

  /* Checking for number of iterations */

  int iterations = 0;
  if( ! parameters.GetParameter( "iterations", iterations ) ) {

    TEAGN_LOGERR( "Missing parameter: iterations" );
    return false;
  }
  TEAGN_TRUE_OR_RETURN( iterations > 0, "Invalid iterations number" );

  /* Checking for detection type parameter */

  TePDIFrostDetType det_type;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "det_type", det_type ),
    "Missing parameter: det_type" );
  TEAGN_TRUE_OR_RETURN(
    ( ( det_type == TePDIFrostDTLinear ) || ( det_type == TePDIFrostDTQuadratic ) ),
    "Invalid detection type" );

  /* Checking for image look number */

  double look_number = 0;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "look_number", look_number ),
    "Missing parameter: look_number" );
  TEAGN_TRUE_OR_RETURN( look_number > 0,  "Invalid image look number" );

  /* Checking for corr_coef parameter */

  double corr_coef = 0;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "corr_coef",
    corr_coef ),
    "Missing parameter: corr_coef" );
  TEAGN_TRUE_OR_RETURN( ( corr_coef >= 0 ) && ( corr_coef <= 1 ),
    "Invalid curr_coef" );

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


bool TePDIRadarFrostFilter::RunImplementation()
{
  TePDITypes::TePDIRasterPtrType inRaster;
  params_.GetParameter( "input_image", inRaster );

  TePDITypes::TePDIRasterPtrType outRaster;
  params_.GetParameter( "output_image", outRaster );

  std::vector< int > channels;
  params_.GetParameter( "channels", channels );

  int iterations = 0;
  params_.GetParameter( "iterations", iterations );

  TePDIFrostDetType det_type;
  params_.GetParameter( "det_type", det_type );

  double look_number = 0;
  params_.GetParameter( "look_number", look_number );

  double corr_coef;
  params_.GetParameter( "corr_coef", corr_coef );

  const unsigned int mask_width = 5;
  /* fixed size - affects convolution weights matrix dimentions */

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

  if( det_type == TePDIFrostDTLinear ) {
    noise_deviation = 0.522723008 / sqrt( look_number );
  } else if( det_type == TePDIFrostDTQuadratic ) {
    noise_deviation = 1.0 / sqrt( look_number );
  }

  const double noise_variance = noise_deviation * noise_deviation;
  const double noise_variance_plus_one = noise_variance + 1.;

  /* Setting the convolution buffer initial state */

  const unsigned int raster_lines = (unsigned int)outRaster->params().nlines_;
  const unsigned int raster_columns = (unsigned int)outRaster->params().ncols_;

  reset_conv_buf( mask_width, raster_columns );

  /* Convolution Loop */

  double output_level; /* the value generated by each pixel mask calcule */

  const unsigned int mask_middle_off =
    (unsigned int) floor( ((double)mask_width) / 2. );

  const unsigned int conv_column_bound = raster_columns - mask_width + 1;
  const unsigned int conv_line_bound = raster_lines - mask_width + 1;

  unsigned int raster_line; /* mask top-left line */
  unsigned int conv_buf_column; /* mask top-left column */

  double channel_min_level;
  double channel_max_level;

  double mean = 0; /* for the current window covolution inside convolution 
    buffer */
  double variance = 0; /* for the current window convolution inside 
    convolution buffer */
  double mean_2 = 0;; /* mean ^ 2 */
  double vx = 0;;
  double H = 0;;
  double m = 0;;
  double m_cumulative = 0;;

  TePDITypes::TePDIRasterPtrType source_raster;
  TePDITypes::TePDIRasterPtrType target_raster;

  double mtab[ 33 ];

  unsigned int mask_line;
  unsigned int mask_column;

  unsigned int dist_x;
  unsigned int dist_y;
  unsigned int dist_2;
  int sourceChannel = 0;
  int targetChannel = 0;  
  
  StartProgInt( "Frost filter", channels.size() * iterations * 
    conv_line_bound );  

  for( unsigned int channels_index = 0 ; channels_index < channels.size() ;
       ++channels_index ) 
  {
    for( int iteration = 0 ; iteration < iterations ; ++iteration ) 
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

      for( unsigned int line = 0 ; line < ( mask_width - 1) ; ++line ) {
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
             ++conv_buf_column ) {

          conv_buf_estatistics( 0, conv_buf_column, mask_width, mask_width, mean,
            variance );

          mean_2 = mean * mean;

          vx = ( variance  - ( noise_variance *  mean_2 ) ) /
               noise_variance_plus_one;

          H = vx + mean_2;
          if( H != 0 ) {
            H = corr_coef + ( 2 * corr_coef * vx / noise_variance / H );
          }
          if( H <= 0 ) {
            H = corr_coef;
          }
          H = sqrt( H );

          /* Weights matrix creation */

          mtab[0] = 1.;
          mtab[1] = exp(-H * 1.);
          mtab[2] = exp(-H * 1.414213562);
          mtab[4] = exp(-H * 2.);
          mtab[5] = exp(-H * 2.236067977);
          mtab[8] = exp(-H * 2.828427125);
          mtab[9] = exp(-H * 3.);
          mtab[10] = exp(-H * 3.16227766);
          mtab[13] = exp(-H * 3.605551276);
          mtab[16] = exp(-H * 4.);
          mtab[17] = exp(-H * 4.123105626);
          mtab[18] = exp(-H * 4.242640687);
          mtab[20] = exp(-H * 4.472135955);
          mtab[25] = exp(-H * 5.);
          mtab[32] = exp(-H * 5.65685425);

          /* Mask Convolution */

          output_level = 0;
          m_cumulative = 0;

          for( mask_line = 0 ; mask_line < mask_width ; ++mask_line ) {
            dist_y = mask_line - mask_middle_off - mask_width;

            for( mask_column = 0 ; mask_column < mask_width ; ++mask_column ) {
              dist_x = mask_column - mask_middle_off - mask_width;

              dist_2 = ( dist_y * dist_y ) + ( dist_x * dist_x );

              if( ( dist_2 < 32 ) && ( mtab[ dist_2 ] != 0 ) ) {
                m = m * mtab[ dist_2 ];
              } else {
                m = H;
              }
            }

            output_level +=
              m * conv_buf_[ mask_line ][ mask_column + conv_buf_column];
            m_cumulative += m;
          }

          output_level = output_level / m_cumulative;


          /* Level range filtering */

          if( output_level < channel_min_level ) {
            output_level = channel_min_level;
          } else if( output_level > channel_max_level ) {
            output_level = channel_max_level;
          }

          TEAGN_TRUE_OR_RETURN( target_raster->setElement(
            conv_buf_column +  mask_middle_off,
            raster_line +  mask_middle_off, output_level, targetChannel ),
            "Pixel mapping error" );

        }
      }
    }
  }

  return true;
}
