#include "TePDIColorTransform.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"
#include "TePDIMatrix.hpp"
#include "../kernel/TeDefines.h"

#ifndef M_PI
  #define M_PI       3.14159265358979323846
#endif


TePDIColorTransform::TePDIColorTransform()
{
}


TePDIColorTransform::~TePDIColorTransform()
{
}


void TePDIColorTransform::ResetState( const TePDIParameters& )
{
}


bool TePDIColorTransform::CheckParameters( 
  const TePDIParameters& parameters ) const
{
  /* Checking transformation type */
  
  ColorTransfTypes transf_type;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "transf_type", transf_type ),
    "Missing parameter: transf_type" );
  TEAGN_TRUE_OR_RETURN(
    ( ( transf_type == Rgb2Ihs ) || ( transf_type == Ihs2Rgb ) ||
	  ( transf_type == Rgb2Hls ) || ( transf_type == Hls2Rgb ) ),
    "Invalid parameter: transf_type" );
    
  /* Checking input_rasters and input_channels */
  
  TePDITypes::TePDIRasterVectorType input_rasters;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "input_rasters", input_rasters ),
    "Missing parameter: input_rasters" );
    
  TEAGN_TRUE_OR_RETURN( ( input_rasters.size() == 3 ),
    "Invalid number of input rasters" );    
    
  std::vector< int > input_channels;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "input_channels", input_channels ),
    "Missing parameter: input_channels" );
    
  TEAGN_TRUE_OR_RETURN( ( input_channels.size() == 3 ), 
    "Invalid number of input channels" );
    
  for( unsigned int index = 0 ; index < input_rasters.size() ; ++index ) {
    TEAGN_TRUE_OR_RETURN( input_rasters[ index ].isActive(),
      "Invalid parameter: raster " + 
      Te2String( index ) + " inactive" );
      
    TEAGN_TRUE_OR_RETURN( 
      input_rasters[ index ]->params().status_ != TeRasterParams::TeNotReady,
      "Invalid parameter: raster " + 
      Te2String( index ) + " not ready" );
        
    TEAGN_TRUE_OR_RETURN( 
      input_rasters[ 0 ]->params().nlines_ == 
      input_rasters[ index ]->params().nlines_,
      "Lines number mismatch between raster 0 and raster " +
      Te2String( index ) );
      
    TEAGN_TRUE_OR_RETURN( 
      input_rasters[ 0 ]->params().ncols_ == 
      input_rasters[ index ]->params().ncols_,
      "Columns number mismatch between raster 0 and raster " +
      Te2String( index ) );
            
    TEAGN_TRUE_OR_RETURN( 
      input_channels[ index ] >= 0, "Invalid channel number (" + 
      Te2String( index ) + ")" );
    TEAGN_TRUE_OR_RETURN( 
      input_channels[ index ] < input_rasters[ index ]->nBands(), 
      "Invalid channel number (" + 
      Te2String( index ) + ")" );
      
    /* Checking photometric interpretation */
    
    TEAGN_TRUE_OR_RETURN( ( 
      ( input_rasters[ index ]->params().photometric_[ 
        input_channels[ index ] ] == TeRasterParams::TeRGB ) ||
      ( input_rasters[ index ]->params().photometric_[ 
        input_channels[ index ] ] == TeRasterParams::TeMultiBand ) ),
      "Invalid parameter - rasters (invalid photometric "
      "interpretation)" );      
  }    
  
  /* Checking output_rasters */
  
  TePDITypes::TePDIRasterVectorType output_rasters;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "output_rasters", output_rasters ),
    "Missing parameter: output_rasters" );
    
  TEAGN_TRUE_OR_RETURN( ( ( output_rasters.size() == 3 ) ||
    ( output_rasters.size() == 1 ) ), 
    "Invalid number of output rasters" );
    
  for( unsigned int index = 0 ; index < output_rasters.size() ; ++index ) {
    TEAGN_TRUE_OR_RETURN( output_rasters[ index ].isActive(),
      "Invalid parameter: output raster " + 
      Te2String( index ) + " inactive" );
      
    TEAGN_TRUE_OR_RETURN( 
      output_rasters[ index ]->params().status_ != TeRasterParams::TeNotReady,
      "Invalid parameter: output raster " + 
      Te2String( index ) + " not ready" );
      
    if( ( index != 0 ) && ( output_rasters.size() != 1 ) ) {
      TEAGN_TRUE_OR_RETURN( 
        ( output_rasters[ 0 ] != output_rasters[ index ] ),
        "Cannot use the same output raster two more times" );    
    }
    
    /* Checking input data type */
    
    if( transf_type == Rgb2Ihs ) {
      TEAGN_TRUE_OR_RETURN( 
        (
          ( 
            output_rasters[ index ]->params().dataType_[ 0 ] == TeDOUBLE 
          )
          ||
          (
            output_rasters[ index ]->params().dataType_[ 0 ] == TeFLOAT 
          )
        ), "Invalid output rasters data type" );  
    }      
  }     
  
  /* Checking  rgb_channels_min and rgbs_channel_max */
  
  if( ( transf_type == Ihs2Rgb ) || ( transf_type == Rgb2Ihs ) ||
	  ( transf_type == Hls2Rgb ) || ( transf_type == Rgb2Hls )) {
    double rgb_channels_min = 0;
    TEAGN_TRUE_OR_RETURN( 
      parameters.GetParameter( "rgb_channels_min", rgb_channels_min ),
      "Missing parameter: rgbs_channels_min" );  
      
    double rgb_channels_max = 0;
    TEAGN_TRUE_OR_RETURN( 
      parameters.GetParameter( "rgb_channels_max", rgb_channels_max ),
      "Missing parameter: rgb_channels_max" );    
      
    TEAGN_TRUE_OR_RETURN( 
      ( rgb_channels_max > rgb_channels_min ),
      "Invalid parameters: rgb_channels_max - rgb_channels_min" );       
  }
  
  return true;
}


bool TePDIColorTransform::RunImplementation()
{
  TePDITypes::TePDIRasterVectorType input_rasters;
  params_.GetParameter( "input_rasters", input_rasters );
    
  std::vector< int > input_channels;
  params_.GetParameter( "input_channels", input_channels );
  
  TePDITypes::TePDIRasterVectorType output_rasters;
  params_.GetParameter( "output_rasters", output_rasters );

  ColorTransfTypes transf_type;
  params_.GetParameter( "transf_type", transf_type );
  
  std::vector< int > output_channels;
  
  /* Updating output raster geometries */
  
  TeRaster& ref_input_raster = *(input_rasters[ 0 ].nakedPointer() );
  
  if( output_rasters.size() == 1 ) {
    TeRaster& output_raster = *( output_rasters[ 0 ].nakedPointer() );
    
    /* Generating the new output raster parameters */
    
    TeRasterParams output_raster_params = output_raster.params();
    
    output_raster_params.nBands( 3 );
    if( ref_input_raster.projection() != 0 ) {
      output_raster_params.projection( ref_input_raster.projection() );
    }
    output_raster_params.boxLinesColumns( 
      ref_input_raster.params().box().x1(), 
      ref_input_raster.params().box().y1(), 
      ref_input_raster.params().box().x2(), 
      ref_input_raster.params().box().y2(), 
      ref_input_raster.params().nlines_, 
      ref_input_raster.params().ncols_ );
      
    switch( transf_type ) {
      case Rgb2Ihs :
      {
        output_raster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 );

        break;
      }
      case Ihs2Rgb :
      {
        output_raster_params.setPhotometric( TeRasterParams::TeRGB, -1 );

        break;
      }
	  case Rgb2Hls :
		  {
			  output_raster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 );

			  break;
		  }
	  case Hls2Rgb :
		  {
			  output_raster_params.setPhotometric( TeRasterParams::TeRGB, -1 );

			  break;
		  }
      default :
      {
        TEAGN_LOG_AND_RETURN( "Invalid transformation type" );
        break;
      }
    }      
      
    TEAGN_TRUE_OR_RETURN( output_raster.init( output_raster_params ),
      "Output raster reset error" );  
      
    /* updating the output_channels vector */
      
    output_channels.push_back( 0 );
    output_channels.push_back( 1 );
    output_channels.push_back( 2 );     
    
     /* updating the output_channels vector */

    output_rasters.push_back( output_rasters[ 0 ] );   
    output_rasters.push_back( output_rasters[ 0 ] );
  } else {
    /* output_rasters.size() == 3 */
    
    for( unsigned int index = 0 ; index < output_rasters.size() ; ++index ) {
      TeRaster& output_raster = *( output_rasters[ index ].nakedPointer() );
      
      /* Generating the new output raster parameters */
      
      TeRasterParams output_raster_params = output_raster.params();
      
      output_raster_params.nBands( 1 );
      if( ref_input_raster.projection() != 0 ) {
        output_raster_params.projection( ref_input_raster.projection() );
      }
      output_raster_params.boxLinesColumns( 
        ref_input_raster.params().box().x1(), 
        ref_input_raster.params().box().y1(), 
        ref_input_raster.params().box().x2(), 
        ref_input_raster.params().box().y2(), 
        ref_input_raster.params().nlines_, 
        ref_input_raster.params().ncols_ );  
      switch( transf_type ) {
        case Rgb2Ihs :
        {
          output_raster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 );

          break;
        }
        case Ihs2Rgb :
        {
          output_raster_params.setPhotometric( TeRasterParams::TeRGB, -1 );

          break;
        }
		case Rgb2Hls :
			{
				output_raster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 );

				break;
			}
		case Hls2Rgb :
			{
				output_raster_params.setPhotometric( TeRasterParams::TeRGB, -1 );

				break;
			}
        default :
        {
          TEAGN_LOG_AND_RETURN( "Invalid transformation type" );
          break;
        }
      }           
        
      TEAGN_TRUE_OR_RETURN( output_raster.init( output_raster_params ),
        "Output raster reset error" );  
    }

    /* updating the output_channels vector */
      
    output_channels.push_back( 0 );
    output_channels.push_back( 0 );
    output_channels.push_back( 0 );       
  }
    
  /* Calling the required color conversion */  

  switch( transf_type ) {
    case Rgb2Ihs :
    {
      double rgb_channels_min = 0;
      params_.GetParameter( "rgb_channels_min", rgb_channels_min );
        
      double rgb_channels_max = 0;
      params_.GetParameter( "rgb_channels_max", rgb_channels_max );
          
      return RunRgb2Ihs( input_rasters, input_channels, output_rasters,
        output_channels, rgb_channels_min, rgb_channels_max );
      break;
    }
    case Ihs2Rgb :
    {
      double rgb_channels_min = 0;
      params_.GetParameter( "rgb_channels_min", rgb_channels_min );
        
      double rgb_channels_max = 0;
      params_.GetParameter( "rgb_channels_max", rgb_channels_max );
         
      return RunIhs2Rgb( input_rasters, input_channels, output_rasters,
        output_channels, rgb_channels_min, rgb_channels_max );
      break;
    }
	case Rgb2Hls :
	{
		double rgb_channels_min = 0;
		params_.GetParameter( "rgb_channels_min", rgb_channels_min );

		double rgb_channels_max = 0;
		params_.GetParameter( "rgb_channels_max", rgb_channels_max );

		return RunRgb2Hls( input_rasters, input_channels, output_rasters,
			output_channels, rgb_channels_min, rgb_channels_max );
		break;
	}
	case Hls2Rgb :
	{
		double rgb_channels_min = 0;
		params_.GetParameter( "rgb_channels_min", rgb_channels_min );

		double rgb_channels_max = 0;
		params_.GetParameter( "rgb_channels_max", rgb_channels_max );

		return RunHls2Rgb( input_rasters, input_channels, output_rasters,
			output_channels, rgb_channels_min, rgb_channels_max );
		break;
	}
    default :
    {
      TEAGN_LOG_AND_RETURN( "Invalid transformation type" );
      break;
    }
  }

  return false;
}


bool TePDIColorTransform::RunRgb2Ihs(
  TePDITypes::TePDIRasterVectorType& input_rasters,
  std::vector< int >& input_channels,
  TePDITypes::TePDIRasterVectorType& output_rasters,
  std::vector< int >& output_channels,
  const double rgb_channels_min, const double rgb_channels_max )
{
  TEAGN_DEBUG_CONDITION( ( input_rasters.size() == 3 ),
    "Invalid vector size" )
  TEAGN_DEBUG_CONDITION( ( input_channels.size() == 3 ),
    "Invalid vector size" )
  TEAGN_DEBUG_CONDITION( ( output_rasters.size() == 3 ),
    "Invalid vector size" )
  TEAGN_DEBUG_CONDITION( ( output_channels.size() == 3 ),
    "Invalid vector size" )
  TEAGN_DEBUG_CONDITION( ( rgb_channels_max >= rgb_channels_min ),
    "Invalid rgb channels max/min" )    
                
  /* Extracting local references */
  
  TeRaster& input_raster0 = *( input_rasters[ 0 ].nakedPointer() ); 
  TeRaster& input_raster1 = *( input_rasters[ 1 ].nakedPointer() ); 
  TeRaster& input_raster2 = *( input_rasters[ 2 ].nakedPointer() );
  
  const int input_channel0 = input_channels[ 0 ];
  const int input_channel1 = input_channels[ 1 ];
  const int input_channel2 = input_channels[ 2 ];
  
  TeRaster& output_raster0 = *( output_rasters[ 0 ].nakedPointer() ); 
  TeRaster& output_raster1 = *( output_rasters[ 1 ].nakedPointer() ); 
  TeRaster& output_raster2 = *( output_rasters[ 2 ].nakedPointer() );
  
  const int output_channel0 = output_channels[ 0 ];
  const int output_channel1 = output_channels[ 1 ];
  const int output_channel2 = output_channels[ 2 ];    
  
  const unsigned int lines = ( unsigned int ) input_raster0.params().nlines_;
  const unsigned int columns = ( unsigned int ) 
    input_raster0.params().ncols_;
    
  const double rgb_channels_diff = rgb_channels_max - rgb_channels_min;
  const double rgb_channels_norm_fac = ( rgb_channels_diff != 0.0 ) ? 
    rgb_channels_diff : 1.0;
  
  /* Dummy use definition */
  
  double out_raster_dummy = 0;
  if( output_raster0.params().useDummy_ ) {
    out_raster_dummy = output_raster0.params().dummy_[ 0 ];
  }
  
  /* Generating the non-normalized ihs matrixes */
  
  double red = 0, green = 0, blue = 0;
  double hue = 0, sat = 0, light = 0;  
  unsigned int line = 0;
  unsigned int column = 0;
  double teta = 0;
  double red_norm = 0, green_norm = 0, blue_norm = 0;
  double r_minus_g = 0, r_minus_b = 0;
  double rgb_sum = 0;
  double cosvalue = 0;
  const double two_pi = 2.0 * ((double)M_PI);
  const double pi_rat_2 = ((double)M_PI) / 2.0;
  
  TePDIPIManager progress( "Converting RGB -> IHS...", lines,
    progress_enabled_ ); 

  for( line = 0 ; line < lines ; ++line ) {
    for( column = 0 ; column < columns ; ++column ) {
      if( input_raster0.getElement( column, line, red, input_channel0 ) && 
        input_raster1.getElement( column, line, green, input_channel1 ) &&
        input_raster2.getElement( column, line, blue, input_channel2 ) ) 
      {
        if( ( red == green ) && ( green == blue ) ) 
        { // Gray scale case
          // From Wikipedia:
          // h = 0 is used for grays though the hue has no geometric 
          // meaning there, where the saturation s = 0. Similarly, 
          // the choice of 0 as the value for s when l is equal to 0 or 1 
          // is arbitrary.        
          
          hue = sat = 0.0;
          light = ( red / rgb_channels_norm_fac ); // or green or blue since they all are the same.
        }
        else
        { // Color case
          red_norm = ( red - rgb_channels_min ) / rgb_channels_norm_fac;
          green_norm = ( green - rgb_channels_min ) / rgb_channels_norm_fac;
          blue_norm = ( blue - rgb_channels_min ) / rgb_channels_norm_fac;
          
          r_minus_g = red_norm - green_norm;
          r_minus_b = red_norm - blue_norm;
          
          cosvalue = sqrt( ( r_minus_g * r_minus_g ) + ( r_minus_b * 
            ( green_norm - blue_norm ) ) );
            
          if( cosvalue == 0.0 )
          {
            teta = pi_rat_2;
          }
          else
          {
            cosvalue =  ( 0.5 * ( r_minus_g + r_minus_b )  ) /
              cosvalue;
            teta = acos( cosvalue );  
          }
            
          TEAGN_DEBUG_CONDITION( ( cosvalue >= (-1.0) ) &&
            ( cosvalue <= (1.0) ), "Invalid cosvalue value"
            " cosvalue=" + Te2String( cosvalue, 9 )
            + " red_norm=" + Te2String( red_norm, 9 )
            + " green_norm=" + Te2String( green_norm, 9 )
            + " blue_norm=" + Te2String( blue_norm, 9 )
            + " r_minus_g=" + Te2String( r_minus_g, 9 )
            + " r_minus_b=" + Te2String( r_minus_b, 9 )
            + " rgb_channels_min=" + Te2String( rgb_channels_min, 9 ) 
            + " rgb_channels_norm_fac=" + Te2String( rgb_channels_norm_fac, 9 )  
            );
            
          if( blue_norm > green_norm )
          {
            hue = two_pi - teta;
          }
          else
          {
            hue = teta;
          }
            
          rgb_sum = red_norm + green_norm + blue_norm;
          
          sat = 1.0 - ( 3 * MIN( MIN( red_norm, green_norm ), blue_norm ) /
            rgb_sum );
            
          light = rgb_sum / 3.0;
        }
      } else {
        hue = sat = light = 0.0;
      }
      
      TEAGN_TRUE_OR_RETURN( output_raster0.setElement(
        column, line, light, output_channel0 ),
        "Unable to write intensity channel for output_image" );
      TEAGN_TRUE_OR_RETURN( output_raster1.setElement(
        column, line, hue, output_channel1 ),
        "Unable to write hue channel for output_image" );
      TEAGN_TRUE_OR_RETURN( output_raster2.setElement(
        column, line, sat, output_channel2 ),
        "Unable to write saturation channel for output_image" ); 
    }
    
    TEAGN_FALSE_OR_RETURN( progress.Increment(), 
      "Canceled by the user" );    
  }
  
  return true;
}


bool TePDIColorTransform::RunIhs2Rgb(
  TePDITypes::TePDIRasterVectorType& input_rasters,
  std::vector< int >& input_channels,
  TePDITypes::TePDIRasterVectorType& output_rasters,
  std::vector< int >& output_channels,
  const double rgb_channels_min, const double rgb_channels_max )
{
  TEAGN_TRUE_OR_THROW( ( input_rasters.size() == 3 ),
    "Invalid vector size" )
  TEAGN_TRUE_OR_THROW( ( input_channels.size() == 3 ),
    "Invalid vector size" )
  TEAGN_TRUE_OR_THROW( ( output_rasters.size() == 3 ),
    "Invalid vector size" )
  TEAGN_TRUE_OR_THROW( ( output_channels.size() == 3 ),
    "Invalid vector size" )
                
  /* Extracting local references */
  
  TeRaster& input_raster0 = *( input_rasters[ 0 ].nakedPointer() ); 
  TeRaster& input_raster1 = *( input_rasters[ 1 ].nakedPointer() ); 
  TeRaster& input_raster2 = *( input_rasters[ 2 ].nakedPointer() );
  
  const int input_channel0 = input_channels[ 0 ];
  const int input_channel1 = input_channels[ 1 ];
  const int input_channel2 = input_channels[ 2 ];
  
  TeRaster& output_raster0 = *( output_rasters[ 0 ].nakedPointer() ); 
  TeRaster& output_raster1 = *( output_rasters[ 1 ].nakedPointer() ); 
  TeRaster& output_raster2 = *( output_rasters[ 2 ].nakedPointer() );
  
  int output_channel0 = output_channels[ 0 ];
  int output_channel1 = output_channels[ 1 ];
  int output_channel2 = output_channels[ 2 ];  
    
  unsigned int lines = ( unsigned int ) input_raster0.params().nlines_;
  unsigned int columns = ( unsigned int ) input_raster0.params().ncols_;

  const double rgb_channels_diff = rgb_channels_max - rgb_channels_min;
  const double rgb_channels_norm_fac = ( rgb_channels_diff != 0.0 ) ? 
    rgb_channels_diff : 1.0;  
 
  /* Dummy use definition */
  
  double out_raster_dummy = 0;
  if( output_raster0.params().useDummy_ ) {
    out_raster_dummy = output_raster0.params().dummy_[ 0 ];
  }
  
  /* Raster convertion */
  
  const double pi_rat3 = M_PI / 3.0; // 60
  const double two_pi_rat3 = 2.0 * M_PI / 3.0; // 120
  const double four_pi_rat3 = 4.0 * M_PI / 3.0; // 240
  
  double red = 0, green = 0, blue = 0;
  double hue = 0, sat = 0, lig = 0;
  
  StartProgInt( "Converting IHS -> RGB...", lines );
  
  for( unsigned int line = 0 ; line < lines ; ++line ) {
    TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
  
    for( unsigned int column = 0 ; column < columns ; ++column ) {
      if( input_raster0.getElement( column, line, lig, input_channel0 ) &&
          input_raster1.getElement( column, line, hue, input_channel1 ) &&
          input_raster2.getElement( column, line, sat, input_channel2 ) ) 
      {
        if( ( hue == 0.0 ) && ( sat == 0.0 ) )
        { // Gray scale case
          red = green = blue = ( lig * rgb_channels_norm_fac );
        }
        else
        { // color case
          /* Hue inside RG sector */
          if( hue < two_pi_rat3 )
          {
            blue = lig * ( 1.0 - sat );
            red = lig * ( 1.0 + ( sat * cos( hue ) / 
              cos( pi_rat3 - hue ) ) );
            green = ( 3.0 * lig ) - ( red + blue );
          }
          else if( hue < four_pi_rat3 )
          { /* Hue inside GB sector */
          
            hue -= two_pi_rat3;
            
            red = lig * ( 1.0 - sat );
            green = lig * ( 1.0 + ( sat * cos( hue ) / 
              cos( pi_rat3 - hue ) ) );
            blue = ( 3.0 * lig ) - ( red + green );
          }
          else
          { /* Hue inside BR sector */
          
            hue -= four_pi_rat3;
            
            green = lig * ( 1.0 - sat );
            blue = lig * ( 1.0 + ( sat * cos( hue ) / 
              cos( pi_rat3 - hue ) ) );
            red = ( 3.0 * lig ) - ( green + blue );
          }
          
          red = ( red * rgb_channels_norm_fac ) + rgb_channels_min;
          green = ( green * rgb_channels_norm_fac ) + rgb_channels_min;
          blue = ( blue * rgb_channels_norm_fac ) + rgb_channels_min;
        }
        
        red = MIN( red, rgb_channels_max );
        green = MIN( green, rgb_channels_max );
        blue = MIN( blue, rgb_channels_max );
        
        red = MAX( red, rgb_channels_min );
        green = MAX( green, rgb_channels_min );
        blue = MAX( blue, rgb_channels_min );        
  
        TEAGN_TRUE_OR_RETURN( output_raster0.setElement(
          column, line, red, output_channel0 ),
          "Unable to write red channel for output_image" );
        TEAGN_TRUE_OR_RETURN( output_raster1.setElement(
          column, line, green, output_channel1 ),
          "Unable to write green channel for output_image" );
        TEAGN_TRUE_OR_RETURN( output_raster2.setElement(
          column, line, blue, output_channel2 ),
          "Unable to write blue channel for output_image" );
      } else {                    
        TEAGN_TRUE_OR_RETURN( output_raster0.setElement(
          column, line, out_raster_dummy, output_channel0 ),
          "Unable to write red channel for output_image" );
        TEAGN_TRUE_OR_RETURN( output_raster1.setElement(
          column, line, out_raster_dummy, output_channel1 ),
          "Unable to write green channel for output_image" );
        TEAGN_TRUE_OR_RETURN( output_raster2.setElement(
          column, line, out_raster_dummy, output_channel2 ),
          "Unable to write blue channel for output_image" );
      }
    }
  }

  return true;
}

bool TePDIColorTransform::RunRgb2Hls(
	TePDITypes::TePDIRasterVectorType& input_rasters,
	std::vector< int >& input_channels,
	TePDITypes::TePDIRasterVectorType& output_rasters,
	std::vector< int >& output_channels,
	const double rgb_channels_min, const double rgb_channels_max )
{
	TEAGN_DEBUG_CONDITION( ( input_rasters.size() == 3 ),
	"Invalid vector size" )
	TEAGN_DEBUG_CONDITION( ( input_channels.size() == 3 ),
	"Invalid vector size" )
	TEAGN_DEBUG_CONDITION( ( output_rasters.size() == 3 ),
	"Invalid vector size" )
	TEAGN_DEBUG_CONDITION( ( output_channels.size() == 3 ),
	"Invalid vector size" )
	TEAGN_DEBUG_CONDITION( ( rgb_channels_max >= rgb_channels_min ),
	"Invalid rgb channels max/min" )    

	/* Extracting local references */

	TeRaster& input_raster0 = *( input_rasters[ 0 ].nakedPointer() ); 
	TeRaster& input_raster1 = *( input_rasters[ 1 ].nakedPointer() ); 
	TeRaster& input_raster2 = *( input_rasters[ 2 ].nakedPointer() );

	const int input_channel0 = input_channels[ 0 ];
	const int input_channel1 = input_channels[ 1 ];
	const int input_channel2 = input_channels[ 2 ];

	TeRaster& output_raster0 = *( output_rasters[ 0 ].nakedPointer() ); 
	TeRaster& output_raster1 = *( output_rasters[ 1 ].nakedPointer() ); 
	TeRaster& output_raster2 = *( output_rasters[ 2 ].nakedPointer() );

	const int output_channel0 = output_channels[ 0 ];
	const int output_channel1 = output_channels[ 1 ];
	const int output_channel2 = output_channels[ 2 ];    

	const unsigned int lines = ( unsigned int ) input_raster0.params().nlines_;
	const unsigned int columns = ( unsigned int ) 
		input_raster0.params().ncols_;

	/* Dummy use definition */

	double out_raster_dummy = 0;
	if( output_raster0.params().useDummy_ ) {
		out_raster_dummy = output_raster0.params().dummy_[ 0 ];
	}

	/* Generating the non-normalized ihs matrixes */

	double red = 0, green = 0, blue = 0;
	double hue = 0, sat = 0, light = 0;  
	unsigned int line = 0;
	unsigned int column = 0;

	TePDIPIManager progress( "Converting RGB -> HLS...", lines,
		progress_enabled_ ); 

	for( line = 0 ; line < lines ; ++line ) {
		for( column = 0 ; column < columns ; ++column ) {
			input_raster0.getElement( column, line, red, input_channel0 ); 
			input_raster1.getElement( column, line, green, input_channel1 );
			input_raster2.getElement( column, line, blue, input_channel2 );

			red /= rgb_channels_max;
			green /= rgb_channels_max;
			blue /= rgb_channels_max;

			double vmax = MAX(red, green);
			vmax = MAX(vmax, blue);

			double vmin = MIN(red, green);
			vmin = MIN(vmin, blue);

			hue = (vmax + vmin) / 2.;
			sat = (vmax + vmin) / 2.;
			light = (vmax + vmin) / 2.;

			if(vmax == vmin)
			{// achromatic
				hue = 0.;
				sat = 0.;
			}
			else
			{
				double diff = vmax - vmin;

				sat = light > 0.5 ? diff / (2. - vmax - vmin) : diff / (vmax + vmin);
					
				if(vmax == red)
				{
					hue = (green - blue) / diff + (green < blue ? 6. : 0.);
				}
				else if(vmax == green)
				{
					hue = (blue - red) / diff + 2.;
				}
				else if(vmax == blue)
				{
					hue = (red - green) / diff + 4.;
				}

				hue *= 60.;
			}

			TEAGN_TRUE_OR_RETURN( output_raster0.setElement(
				column, line, hue, output_channel0 ),
				"Unable to write intensity channel for output_image" );
			TEAGN_TRUE_OR_RETURN( output_raster1.setElement(
				column, line, light, output_channel1 ),
				"Unable to write hue channel for output_image" );
			TEAGN_TRUE_OR_RETURN( output_raster2.setElement(
				column, line, sat, output_channel2 ),
				"Unable to write saturation channel for output_image" ); 
		}

		TEAGN_FALSE_OR_RETURN( progress.Increment(), 
			"Canceled by the user" );    
	}

	return true;
}

bool TePDIColorTransform::RunHls2Rgb(
	TePDITypes::TePDIRasterVectorType& input_rasters,
	std::vector< int >& input_channels,
	TePDITypes::TePDIRasterVectorType& output_rasters,
	std::vector< int >& output_channels,
	const double rgb_channels_min, const double rgb_channels_max )
{
	TEAGN_TRUE_OR_THROW( ( input_rasters.size() == 3 ),
	"Invalid vector size" )
	TEAGN_TRUE_OR_THROW( ( input_channels.size() == 3 ),
	"Invalid vector size" )
	TEAGN_TRUE_OR_THROW( ( output_rasters.size() == 3 ),
	"Invalid vector size" )
	TEAGN_TRUE_OR_THROW( ( output_channels.size() == 3 ),
	"Invalid vector size" )

	/* Extracting local references */

	TeRaster& input_raster0 = *( input_rasters[ 0 ].nakedPointer() ); 
	TeRaster& input_raster1 = *( input_rasters[ 1 ].nakedPointer() ); 
	TeRaster& input_raster2 = *( input_rasters[ 2 ].nakedPointer() );

	const int input_channel0 = input_channels[ 0 ];
	const int input_channel1 = input_channels[ 1 ];
	const int input_channel2 = input_channels[ 2 ];

	TeRaster& output_raster0 = *( output_rasters[ 0 ].nakedPointer() ); 
	TeRaster& output_raster1 = *( output_rasters[ 1 ].nakedPointer() ); 
	TeRaster& output_raster2 = *( output_rasters[ 2 ].nakedPointer() );

	int output_channel0 = output_channels[ 0 ];
	int output_channel1 = output_channels[ 1 ];
	int output_channel2 = output_channels[ 2 ];  

	unsigned int lines = ( unsigned int ) input_raster0.params().nlines_;
	unsigned int columns = ( unsigned int ) input_raster0.params().ncols_;

	/* Dummy use definition */

	double out_raster_dummy = 0;
	if( output_raster0.params().useDummy_ ) {
		out_raster_dummy = output_raster0.params().dummy_[ 0 ];
	}

	/* Raster convertion */

	double hue = 0, sat = 0, lig = 0;

	StartProgInt( "Converting HLS -> RGB...", lines );

	for( unsigned int line = 0 ; line < lines ; ++line ) {
		TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );

		for( unsigned int column = 0 ; column < columns ; ++column ) {
			if( input_raster0.getElement( column, line, hue, input_channel0 ) &&
				input_raster1.getElement( column, line, lig, input_channel1 ) &&
				input_raster2.getElement( column, line, sat, input_channel2 ) ) 
			{
				std::vector< double > rgb;
				hue = hue / 360.;

				if( sat == 0 )
				{// achromatic
					rgb.push_back(lig * rgb_channels_max);
					rgb.push_back(lig * rgb_channels_max);
					rgb.push_back(lig * rgb_channels_max);
				}
				else
				{
					for( unsigned int i = 0; i < 3; ++i )
					{
						double q = lig < 0.5 ? lig * (1. + sat) : lig + sat - lig * sat;
						double p = 2. * lig - q;

						double t;

						switch(i)
						{
							case 0://red
							{
								t = hue + 1./3.;
								break;
							}
							case 1://green
							{
								t = hue;
								break;
							}
							case 2://blue
							{
								t = hue - 1./3.;
								break;
							}
						}

						if( t < 0 )
						{
							t += 1.;
						}
						if( t > 1 )
						{
							t -= 1.;
						}

						if( t < (1./6.) )
						{
							p = (p + (q - p) * 6. * t);
						}
						else if( t < (1./2.) )
						{
							p = q;
						}
						else if( t < (2./3.) )
						{
							p = (p + (q - p) * (2./3. - t) * 6.);
						}

						rgb.push_back(p * rgb_channels_max);
					}
				}        

				TEAGN_TRUE_OR_RETURN( output_raster0.setElement(
					column, line, rgb[0], output_channel0 ),
					"Unable to write red channel for output_image" );
				TEAGN_TRUE_OR_RETURN( output_raster1.setElement(
					column, line, rgb[1], output_channel1 ),
					"Unable to write green channel for output_image" );
				TEAGN_TRUE_OR_RETURN( output_raster2.setElement(
					column, line, rgb[2], output_channel2 ),
					"Unable to write blue channel for output_image" );
			} else {                    
				TEAGN_TRUE_OR_RETURN( output_raster0.setElement(
					column, line, out_raster_dummy, output_channel0 ),
					"Unable to write red channel for output_image" );
				TEAGN_TRUE_OR_RETURN( output_raster1.setElement(
					column, line, out_raster_dummy, output_channel1 ),
					"Unable to write green channel for output_image" );
				TEAGN_TRUE_OR_RETURN( output_raster2.setElement(
					column, line, out_raster_dummy, output_channel2 ),
					"Unable to write blue channel for output_image" );
			}
		}
	}

	return true;
}

