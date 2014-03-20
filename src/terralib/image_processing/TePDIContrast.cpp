#include "TePDIContrast.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDITypes.hpp"
#include "TePDIUtils.hpp"
#include "TePDIHistogram.hpp"
#include "TePDIMatrix.hpp"

#include <math.h>
#include <float.h>


TePDIContrast::TePDIContrast()
{
}


TePDIContrast::~TePDIContrast()
{
}


bool TePDIContrast::RunImplementation()
{
  /* Getting general parameters */

  TePDIContrastType contrast_type;
  params_.GetParameter( "contrast_type", contrast_type );

  TePDITypes::TePDIRasterPtrType inRaster;
  int input_band = 0;
  if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "input_image" ) ) {
    
    params_.GetParameter( "input_image", inRaster );
    params_.GetParameter( "input_band", input_band );
  }

  int output_band = 0;
  TePDITypes::TePDIRasterPtrType outRaster;
  if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "output_image" ) ) {
    
    params_.GetParameter( "output_image", outRaster );
    params_.GetParameter( "output_band", output_band );  
  }  
  
  double min_level = 0;
  double max_level = 0;
  if( params_.CheckParameter< double >( "min_level" ) ) {
    
    params_.GetParameter( "min_level", min_level );
  }
  if( params_.CheckParameter< double >( "max_level" ) ) {
    
    params_.GetParameter( "max_level", max_level );
  }
  
  TePDIRgbPalette::pointer palette;
  if( params_.CheckParameter< TePDIRgbPalette::pointer >( "rgb_palette" ) ) {
  
    params_.GetParameter( "rgb_palette", palette );
  }
  
  /* output dumyy value definition */
 
  bool output_raster_uses_dummy = false;
  double output_raster_dummy = 0;
  
  if( outRaster.isActive() ) {
    output_raster_uses_dummy = outRaster->params().useDummy_;
    
    if( output_raster_uses_dummy ) {
      output_raster_dummy = outRaster->params().dummy_[ 0 ];
    }    
  }
  
  if( params_.CheckParameter< double >( "dummy_value" ) ) {
    
    params_.GetParameter( "dummy_value", output_raster_dummy );
    
    output_raster_uses_dummy = true;
  }  
  
  /* Reseting output raster */
  
  if( outRaster.isActive() ) {
    switch( contrast_type ) {
      case TePDIContrastSimpleSlicer :
      {
        bool output_reset_not_needed = true;
        
        if( ! params_.CheckParameter< int >( "restrict_out_reset" ) ) {
          output_reset_not_needed = false;
        }
        
        if( output_reset_not_needed &&
          ( inRaster->params().nlines_ != outRaster->params().nlines_ ) ) {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed &&
          ( inRaster->params().ncols_ != outRaster->params().ncols_ ) ) {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed &&
          ( output_band >= outRaster->params().nBands() ) ) {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed )
        { 
          if( inRaster->projection() )
          {
            if( outRaster->projection() )
            {
              if( !( (*inRaster->projection()) == (*outRaster->projection()) ) )
              {
                output_reset_not_needed = false;
              }
            }
            else
            {
              output_reset_not_needed = false;
            }
          }
          else
          {
            if( outRaster->projection() )
            {
              output_reset_not_needed = false;
            }
          }
        }
        
        if( output_reset_not_needed &&
          ( inRaster->params().box() != outRaster->params().box() ) ) 
        {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed &&
          ( inRaster->params().photometric_[ 0 ] != TeRasterParams::TePallete 
          ) ) 
        {
           
          output_reset_not_needed = false; 
        }        
        
        if( output_reset_not_needed &&
            ( ( outRaster->params().useDummy_ == output_raster_uses_dummy ) ?
              false :
              ( outRaster->params().useDummy_ ? 
                ( outRaster->params().dummy_[ 0 ] != output_raster_dummy ) :
                false
              ) 
            )
          ) {
           
          output_reset_not_needed = false; 
        }   
              
        if( ! output_reset_not_needed ) 
        {
          TeRasterParams new_outRaster_params = outRaster->params();
          new_outRaster_params.nBands( output_band + 1 );
          if( inRaster->projection() == 0 ) {
            new_outRaster_params.projection( 0 );
          } else {
            new_outRaster_params.projection( inRaster->projection() );
          }
          new_outRaster_params.boxResolution( inRaster->params().box().x1(), 
            inRaster->params().box().y1(), inRaster->params().box().x2(), 
            inRaster->params().box().y2(), 
            inRaster->params().resx_, inRaster->params().resy_ );
            
          new_outRaster_params.setPhotometric( TeRasterParams::TePallete, -1 );
          
          new_outRaster_params.lutr_.clear();
          new_outRaster_params.lutg_.clear();
          new_outRaster_params.lutb_.clear();
		  new_outRaster_params.lutClassName_.clear();

          TePDIRgbPalette::iterator pal_it = palette->begin();
          TePDIRgbPalette::iterator pal_it_end = palette->end();
          for( unsigned int lut_index = 0 ; lut_index < palette->size() ;
              ++lut_index ) {
    
            new_outRaster_params.lutr_.push_back( 
              (unsigned short)pal_it->second.red_ );
            new_outRaster_params.lutg_.push_back( 
              (unsigned short)pal_it->second.green_ );
            new_outRaster_params.lutb_.push_back( 
              (unsigned short)pal_it->second.blue_ );

            ++pal_it;
          }

		  new_outRaster_params.lutClassName_.resize(palette->size());
          
          if( output_raster_uses_dummy ) {
            new_outRaster_params.setDummy( output_raster_dummy, -1 );
          } else {
            new_outRaster_params.useDummy_ = false;
          }
            
          TEAGN_TRUE_OR_RETURN( outRaster->init( new_outRaster_params ),            
            "Output raster reset error" );           
        }
      
        break;
      }  
      default :
      {
        /* Reseting output raster */
        
        bool output_reset_not_needed = true;
        
        if( ! params_.CheckParameter< int >( "restrict_out_reset" ) ) {
          output_reset_not_needed = false;
        }
        
        if( output_reset_not_needed &&
          ( inRaster->params().nlines_ != outRaster->params().nlines_ ) ) {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed &&
          ( inRaster->params().ncols_ != outRaster->params().ncols_ ) ) {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed &&
          ( output_band >= outRaster->params().nBands() ) ) {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed &&
          ( 
            ( inRaster->projection() == outRaster->projection() ) ? 
            false : 
            ( 
              ( inRaster->projection() == 0 ) ?
              true :
              (
                ( outRaster->projection() == 0 ) ? 
                true :
                ( inRaster->projection()->name() == 
                  outRaster->projection()->name() ) ? false : true
              )
            ) 
          )        
         ) {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed &&
          ( inRaster->params().box() != outRaster->params().box() ) ) {
           
          output_reset_not_needed = false; 
        }
        
        if( output_reset_not_needed &&
            ( ( outRaster->params().useDummy_ == output_raster_uses_dummy ) ?
              false :
              ( outRaster->params().useDummy_ ? 
                ( outRaster->params().dummy_[ 0 ] != output_raster_dummy ) :
                false
              ) 
            )
          ) {
           
          output_reset_not_needed = false; 
        }          
        
        if( ! output_reset_not_needed ) {
        
          TeRasterParams new_outRaster_params = outRaster->params();
          new_outRaster_params.nBands( output_band + 1 );
          if( inRaster->projection() == 0 ) {
            new_outRaster_params.projection( 0 );
          } else {
            TeSharedPtr< TeProjection > proj( TeProjectionFactory::make( 
              inRaster->projection()->params() ) );          
            new_outRaster_params.projection( proj.nakedPointer() );
          }
          new_outRaster_params.boxResolution( inRaster->params().box().x1(), 
            inRaster->params().box().y1(), inRaster->params().box().x2(), 
            inRaster->params().box().y2(), 
            inRaster->params().resx_, inRaster->params().resy_ );            
            
          if( output_raster_uses_dummy ) {
            new_outRaster_params.setDummy( output_raster_dummy, -1 );
          } else {
            new_outRaster_params.useDummy_ = false;
          }            
          
          new_outRaster_params.setPhotometric( TeRasterParams::TeMultiBand );
            
          TEAGN_TRUE_OR_RETURN( outRaster->init( new_outRaster_params ),
            "Output raster reset error" );             
        }
        
        break;
      }
    }
  }
  
  /* Getting output channel range */
  
  double output_channel_min_level = 0;
  double output_channel_max_level = 0;
  
  if( params_.CheckParameter< double >( "output_channel_min_level" ) ) {
    params_.GetParameter( "output_channel_min_level",
      output_channel_min_level );
    params_.GetParameter( "output_channel_max_level",
      output_channel_max_level );
  } else {
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
      outRaster, output_band, output_channel_min_level,
      output_channel_max_level ), "Unable to get raster channel level bounds" );  
  }
      
  /* Building a lut suitable for each algorithm */
    
  TePDITypes::TePDILutType lut;
  bool hist_based_lut = false;
  bool fixed_step_lut = false;
  
  switch( contrast_type ) {
    case TePDIContrastMinMax :
    {
      TePDITypes::TePDILutType baselut;
      TEAGN_TRUE_OR_RETURN( getBaseLut( baselut, hist_based_lut,
        output_raster_uses_dummy, output_raster_dummy ),
        "Error getting base lut" );
      
      lut = GetMinMaxLut( output_channel_min_level, output_channel_max_level,
        baselut );
  
      break;
    }
    case TePDIContrastLinear :
    {
      TePDITypes::TePDILutType baselut;
      TEAGN_TRUE_OR_RETURN( getBaseLut( baselut, hist_based_lut,
        output_raster_uses_dummy, output_raster_dummy ),
        "Error getting base lut" );
            
      lut = GetLinearLut( baselut,
        min_level, max_level, output_channel_min_level, output_channel_max_level );
  
      break;
    }
    case TePDIContrastSquareRoot :
    {
      TePDITypes::TePDILutType baselut;
      TEAGN_TRUE_OR_RETURN( getBaseLut( baselut, hist_based_lut,
        output_raster_uses_dummy, output_raster_dummy ),
        "Error getting base lut" );    
    
      lut = GetSquareRootLut( baselut,
        min_level, max_level, output_channel_min_level, output_channel_max_level );
  
      break;
    }
    case TePDIContrastSquare :
    {
      TePDITypes::TePDILutType baselut;
      TEAGN_TRUE_OR_RETURN( getBaseLut( baselut, hist_based_lut,
        output_raster_uses_dummy, output_raster_dummy ),
        "Error getting base lut" ); 
            
      lut = GetSquareLut( baselut,
        min_level, max_level, output_channel_min_level, output_channel_max_level );
  
      break;
    }
    case TePDIContrastLog :
    {
      TePDITypes::TePDILutType baselut;
      TEAGN_TRUE_OR_RETURN( getBaseLut( baselut, hist_based_lut,
        output_raster_uses_dummy, output_raster_dummy ),
        "Error getting base lut" ); 
            
      lut = GetLogLut( baselut,
        min_level, max_level, output_channel_min_level, output_channel_max_level );
  
      break;
    }
    case TePDIContrastNegative :
    {
      TePDITypes::TePDILutType baselut;
      TEAGN_TRUE_OR_RETURN( getBaseLut( baselut, hist_based_lut,
        output_raster_uses_dummy, output_raster_dummy ),
        "Error getting base lut" ); 
            
      lut = GetNegativeLut( baselut,
        min_level, max_level, output_channel_min_level, output_channel_max_level );
 
      break;
    }
    case TePDIContrastHistEqualizer :
    {
      TePDIHistogram::pointer histogram;
      TEAGN_TRUE_OR_RETURN( getHistogram( histogram, output_raster_uses_dummy, 
        output_raster_dummy ), "Unable to get histogram" );
        
      fixed_step_lut = histogram->hasFixedStep();
      hist_based_lut = true;
    
      lut = GetHistEqualizerLut( histogram,
        output_channel_min_level, output_channel_max_level );
  
      break;
    }
    case TePDIContrastSimpleSlicer :
    {
      TePDIHistogram::pointer histogram;
      TEAGN_TRUE_OR_RETURN( getHistogram( histogram, output_raster_uses_dummy, 
        output_raster_dummy ), "Unable to get histogram" );
        
      fixed_step_lut = histogram->hasFixedStep();
      hist_based_lut = true;
            
      GetSimpleSlicerLut( histogram,
        palette,  min_level, max_level, lut );
  
      break;
    }
    case TePDIContrastStat :
    {
      TePDIHistogram::pointer histogram;
      TEAGN_TRUE_OR_RETURN( getHistogram( histogram, output_raster_uses_dummy, 
        output_raster_dummy ), "Unable to get histogram" );
        
      fixed_step_lut = histogram->hasFixedStep();
      hist_based_lut = true;
    
      double target_mean = 0;
      params_.GetParameter( "target_mean", target_mean );
      
      double target_variance = 0;
      params_.GetParameter( "target_variance", target_variance );
      GetStatLut( histogram, target_mean, target_variance,
        output_channel_min_level, output_channel_max_level, lut );
  
      break;
    }    
    default :
    {
      TEAGN_LOG_AND_THROW( "Unsuported contrast type" );
      break;
    }
  }
  
  /* Updating the output lut, if present */
  
  if( params_.CheckParameter< TePDITypes::TePDILutPtrType >( 
    "outlut" ) ) {

    TePDITypes::TePDILutPtrType outlut;  
    params_.GetParameter( "outlut", outlut );
    
    *outlut = lut;
  }    

  /* Rendering output raster */
  
  if( outRaster.isActive() ) {
    if( hist_based_lut ) {
      TEAGN_TRUE_OR_RETURN( RemapLevels( inRaster, lut, input_band, 
        output_band, outRaster, output_raster_uses_dummy, output_raster_dummy, 
        fixed_step_lut ), "Level remapping error" );
    } else {
      TEAGN_TRUE_OR_RETURN( FullRangeLutRemapLevels( inRaster, lut, 
        input_band, output_band, outRaster, output_raster_uses_dummy, 
        output_raster_dummy ), "Level remapping error" );    
    }
  }
  
  /* Returning the generated histogram, if required */
  
  if( params_.CheckParameter< TePDIHistogram::pointer >( 
    "output_original_histogram" ) ) {
    
    TePDIHistogram::pointer curr_histo_ptr;
    TEAGN_TRUE_OR_RETURN( getHistogram( curr_histo_ptr,
      output_raster_uses_dummy, output_raster_dummy ), 
        "Unable to get histogram" );      
  
    TePDIHistogram::pointer output_original_histogram;
    params_.GetParameter( "output_original_histogram", 
      output_original_histogram );
    
    (*output_original_histogram) = (*curr_histo_ptr);
  }
  
  /* Returning output_enhanced_histogram, if required */
  
  if( params_.CheckParameter< TePDIHistogram::pointer >( 
    "output_enhanced_histogram" ) ) {
    
    TePDIHistogram::pointer curr_histo_ptr;
    TEAGN_TRUE_OR_RETURN( getHistogram( curr_histo_ptr,
      output_raster_uses_dummy, output_raster_dummy ), 
        "Unable to get histogram" );      
  
    TePDIHistogram::pointer output_enhanced_histogram;
    params_.GetParameter( "output_enhanced_histogram", 
      output_enhanced_histogram );
    
    TePDIHistogram::iterator curr_histo_it = curr_histo_ptr->begin();
    TePDIHistogram::iterator curr_histo_it_end = curr_histo_ptr->end();
    TePDITypes::TePDILutType::iterator lut_it_end = lut.end();
    TePDITypes::TePDILutType::iterator found_lut_mapping_it;
    
    while( curr_histo_it != curr_histo_it_end ) {
      found_lut_mapping_it = lut.find( curr_histo_it->first );
      
      if( found_lut_mapping_it != lut_it_end ) {
        (*output_enhanced_histogram)[ found_lut_mapping_it->second ]
          += curr_histo_it->second;
      }
    
      ++curr_histo_it;
    }
  }  
  
  return true;
}


bool TePDIContrast::CheckParameters( 
  const TePDIParameters& parameters ) const
{
  /* Checking input raster */

  if( parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "input_image" ) ) {
    
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
    
    /* Checking input band */
    
    int input_band;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_band", input_band ),
      "Missing parameter: input_band" );
    TEAGN_TRUE_OR_RETURN( ( ( input_band >= 0 ) && 
      ( input_band < inRaster->nBands() ) ),
      "Invalid parameter: input_band" );     
      
    /* Checking photometric interpretation */
    
    TEAGN_TRUE_OR_RETURN( ( 
      ( inRaster->params().photometric_[ input_band ] == 
        TeRasterParams::TeRGB ) ||
      ( inRaster->params().photometric_[ input_band ] == 
        TeRasterParams::TeMultiBand ) ),
    "Invalid parameter - input_image (invalid photometric interpretation)" );    
  }
        
  /* checking output raster, if present */
  
  TePDITypes::TePDIRasterPtrType output_image;
  if( parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "output_image" ) ) {
    
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "output_image", 
      output_image ),  "Missing parameter: output_image" );
    TEAGN_TRUE_OR_RETURN( output_image.isActive(),
      "Invalid parameter: output_image inactive" );
    TEAGN_TRUE_OR_RETURN( output_image->params().status_ != 
      TeRasterParams::TeNotReady, 
      "Invalid parameter: output_image not ready" );
      
    /* Checking output_band */
    
    int output_band;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "output_band", 
      output_band ), "Missing parameter: output_band" );
    TEAGN_TRUE_OR_RETURN( ( output_band >= 0 ),
      "Invalid parameter: output_band" ); 
      
    /* Input raster needed if output_image is present */
    
    TEAGN_TRUE_OR_RETURN( 
      parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
      "input_image" ), "Missing parameter: input_image" );
  }

  /* Checking for the correct allowed contrast types */

  TePDIContrastType contrast_type;
  if( ! parameters.GetParameter( "contrast_type", contrast_type ) ) {
    TEAGN_LOGERR( "Missing parameter: contrast_type" );
    return false;
  }
  TEAGN_CHECK_NOTEQUAL( contrast_type, 0, "Invalid Contrast type" );
  if( ( contrast_type != TePDIContrastMinMax ) &&
      ( contrast_type != TePDIContrastLinear ) &&
      ( contrast_type != TePDIContrastSquareRoot ) &&
      ( contrast_type != TePDIContrastSquare ) &&
      ( contrast_type != TePDIContrastLog ) &&
      ( contrast_type != TePDIContrastNegative ) &&
      ( contrast_type != TePDIContrastHistEqualizer ) &&
      ( contrast_type != TePDIContrastSimpleSlicer ) &&
      ( contrast_type != TePDIContrastStat ) ) {

    TEAGN_LOGERR( "Invalid parameter: contrast_type" );
    return false;
  }

  /* Checking for min and max required parameters */

  if( ( contrast_type == TePDIContrastLinear ) ||
      ( contrast_type == TePDIContrastSquareRoot ) ||
      ( contrast_type == TePDIContrastSquare ) ||
      ( contrast_type == TePDIContrastLog ) ||
      ( contrast_type == TePDIContrastNegative ) ||
      ( contrast_type == TePDIContrastSimpleSlicer ) ) {

    if( ! parameters.CheckParameter< double >( "min_level" ) ) {

      TEAGN_LOGERR( "Missing parameter: min_level" );
      return false;
    }
    if( ! parameters.CheckParameter< double >( "max_level" ) ) {

      TEAGN_LOGERR( "Missing parameter: max_level" );
      return false;
    }
  }

  /* Checking for RGB Palette required parameters */

  if( ( contrast_type == TePDIContrastSimpleSlicer ) ) {
    TePDIRgbPalette::pointer rgb_palette;

    if( ( ! parameters.GetParameter( "rgb_palette", rgb_palette ) ) ||
        ( ! rgb_palette.isActive() ) ) {

      TEAGN_LOGERR( "Missing parameter: rgb_palette" );
      return false;
    }
  }
  
  /* checking outlut parameter */
  
  if( parameters.CheckParameter< TePDITypes::TePDILutPtrType >( 
    "outlut" ) ) {

    TePDITypes::TePDILutPtrType outlut;  
    parameters.GetParameter( "outlut", outlut );
    
    TEAGN_TRUE_OR_RETURN( outlut.isActive(),
      "Invalid parameter: outlut" );
  }
  
  /* checking input_histogram */
  
  TePDIHistogram::pointer input_histogram;
  
  if( parameters.CheckParameter< TePDIHistogram::pointer >( 
    "input_histogram" ) ) {
  
    parameters.GetParameter( "input_histogram", input_histogram );
    TEAGN_TRUE_OR_RETURN( input_histogram.isActive(),
      "Invalid parameter: input_histogram" );
    TEAGN_TRUE_OR_RETURN( ( input_histogram->size() > 0 ),
      "Invalid parameter: input_histogram" );
  } else {
    /* Input raster needed if input_histogram isn't present */
    
    TEAGN_TRUE_OR_RETURN( 
      parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
      "input_image" ), "Missing parameter: input_image" );
  }
  
  /* checking output_original_histogram */
  
  TePDIHistogram::pointer output_original_histogram;
  
  if( parameters.CheckParameter< TePDIHistogram::pointer >( 
    "output_original_histogram" ) ) {
  
    parameters.GetParameter( "output_original_histogram", 
      output_original_histogram );
    TEAGN_TRUE_OR_RETURN( output_original_histogram.isActive(),
      "Invalid parameter: output_original_histogram" );
  }

  /* checking output_enhanced_histogram */
  
  TePDIHistogram::pointer output_enhanced_histogram;
  
  if( parameters.CheckParameter< TePDIHistogram::pointer >( 
    "output_enhanced_histogram" ) ) {
  
    parameters.GetParameter( "output_enhanced_histogram", 
      output_enhanced_histogram );
    TEAGN_TRUE_OR_RETURN( output_enhanced_histogram.isActive(),
      "Invalid parameter: output_enhanced_histogram" );
  }  
  
  /* Checking target_mean and target_variance */
  
  if( ( contrast_type == TePDIContrastStat ) ) {
    double target_mean = 0;
    TEAGN_TRUE_OR_RETURN( 
      parameters.GetParameter( "target_mean", target_mean ),
      "Missing parameter: target_mean" );
      
    double target_variance = 0;
    TEAGN_TRUE_OR_RETURN( 
      parameters.GetParameter( "target_variance", target_variance ),
      "Missing parameter: target_variance" );      
  }
  
  /* Checking input_variance */
  
  /* if input_variance is zero the algoritm will fail in GetStatLut */
  
  double input_variance = 0;
  
  if( parameters.CheckParameter< double >( "input_variance" ) ) {
    parameters.GetParameter( "input_variance", input_variance );
    
    TEAGN_TRUE_OR_RETURN( ( input_variance != 0 ),
      "Invalid parameter - input_variance" )
  }
  
  /* Checking output_channel_min_level and output_channel_max_level */
  
  if( ! output_image.isActive() ) {
  
    TEAGN_TRUE_OR_RETURN( 
      parameters.CheckParameter< double >( "output_channel_min_level" ),
      "Missing parameter: output_channel_min_level" );
    TEAGN_TRUE_OR_RETURN( 
      parameters.CheckParameter< double >( "output_channel_max_level" ),
      "Missing parameter: output_channel_max_level" );
  }
  
  return true;
}


void TePDIContrast::ResetState( const TePDIParameters& params )
{
  if( params != params_ ) {
    histo_ptr_.reset();
  }
}


TePDITypes::TePDILutType TePDIContrast::GetMinMaxLut(
  double output_channel_min_level, double output_channel_max_level,
  TePDITypes::TePDILutType& base_lut )
{
  TEAGN_TRUE_OR_THROW( base_lut.size() != 0, "Invalid base_lut" );
  
  double lut_max = (-1.0) * DBL_MAX;
  double lut_min = DBL_MAX;
  
  TePDITypes::TePDILutType::iterator it = base_lut.begin();
  TePDITypes::TePDILutType::iterator it_end = base_lut.end();
  
  while( it != it_end ) {
    if( it->first < lut_min ) {
      lut_min = it->first;
    }
    if( it->first > lut_max ) {
      lut_max = it->first;
    }
  
    ++it;
  }

  return GetLinearLut( base_lut, lut_min,
    lut_max, output_channel_min_level, output_channel_max_level );
}


TePDITypes::TePDILutType TePDIContrast::GetLinearLut(
  TePDITypes::TePDILutType& base_lut,
  double min, double max,
  double output_channel_min_level, double output_channel_max_level )
{
  TEAGN_CHECK_NOTEQUAL( base_lut.size(), 0, "Invalid base_lut size" );
  TEAGN_TRUE_OR_THROW( max > min, "Invalid max and min values" );

  /* Calculating parameters */

  unsigned int levels = base_lut.size();

  double a = 0;
  double b = 0;

  if( max == min ) {
    a = (double)levels;
    b = -1. * ((double)levels) * min;
  } else {
    a = ((double)levels) / ( max - min );
    b = ( -1. * ((double)levels) * min ) / ( max - min );
  }

  /* Generating LUT map using the existing histogram levels */

  TePDITypes::TePDILutType out_lut;

  TePDITypes::TePDILutType::iterator base_lut_it = base_lut.begin();
  TePDITypes::TePDILutType::iterator base_lut_it_end = base_lut.end();
  
  unsigned int progress = 0;
  double mapped_level = 0;
  StartProgInt( "Building Linear Lut...", base_lut.size() );

  while( base_lut_it != base_lut_it_end ) {
    UpdateProgInt( progress );
    
    if( base_lut_it->first <= min ) {
      out_lut[ base_lut_it->first ] = output_channel_min_level;
    } else if( base_lut_it->first >= max ) {
      out_lut[ base_lut_it->first ] = output_channel_max_level;
    } else {
      mapped_level = ( a * base_lut_it->first ) + b;
      
      if( mapped_level < output_channel_min_level ) {
        mapped_level = output_channel_min_level;
      } else if( mapped_level > output_channel_max_level ) {
        mapped_level = output_channel_max_level;
      }
      
      out_lut[ base_lut_it->first ] = mapped_level;
    }

    ++progress;
    ++base_lut_it;
  }

  return out_lut;
}


TePDITypes::TePDILutType TePDIContrast::GetSquareRootLut(
  TePDITypes::TePDILutType& base_lut,
  double min, double max,
  double output_channel_min_level, double output_channel_max_level )
{
  TEAGN_CHECK_NOTEQUAL( base_lut.size(), 0, "Invalid base_lut size" );
  TEAGN_TRUE_OR_THROW( min < max, "Invalid min and max values" );

  unsigned int levels = base_lut.size();

  double factor = ((double)levels) / sqrt( max - min );

  /* Generating LUT map using the existing base lut levels */

  TePDITypes::TePDILutType out_lut;

  TePDITypes::TePDILutType::iterator base_lut_it = base_lut.begin();
  TePDITypes::TePDILutType::iterator base_lut_it_end = base_lut.end();
  
  unsigned int progress = 0;
  double mapped_level = 0;
  StartProgInt( "Building Square Root Lut...", base_lut.size() );  

  while( base_lut_it != base_lut_it_end ) {
    UpdateProgInt( progress );
    
    if( base_lut_it->first <= min ) {
      out_lut[ base_lut_it->first ] = output_channel_min_level;
    } else if( base_lut_it->first >= max ) {
      out_lut[ base_lut_it->first ] = output_channel_max_level;
    } else {
      mapped_level = factor * sqrt( base_lut_it->first - min );
      
      if( mapped_level < output_channel_min_level ) {
        mapped_level = output_channel_min_level;
      } else if( mapped_level > output_channel_max_level ) {
        mapped_level = output_channel_max_level;
      }      
      
      out_lut[ base_lut_it->first ] = mapped_level;
    }

    ++progress;
    ++base_lut_it;
  }

  return out_lut;
}


TePDITypes::TePDILutType TePDIContrast::GetSquareLut(
  TePDITypes::TePDILutType& base_lut,
  double min, double max,
  double output_channel_min_level, double output_channel_max_level )
{
  TEAGN_CHECK_NOTEQUAL( base_lut.size(), 0, "Invalid base_lut size" );
  TEAGN_TRUE_OR_THROW( min < max, "Invalid min and max values" );

  unsigned int levels = base_lut.size();

  double factor = ((double)levels) / pow( (max - min), 2 );

  /* Generating LUT map using the existing base lut levels */

  TePDITypes::TePDILutType out_lut;

  TePDITypes::TePDILutType::iterator base_lut_it = base_lut.begin();
  TePDITypes::TePDILutType::iterator base_lut_it_end = base_lut.end();

  unsigned int progress = 0;
  double mapped_level = 0;
  StartProgInt( "Building Square Lut...", base_lut.size() ); 
    
  while( base_lut_it != base_lut_it_end ) {
    UpdateProgInt( progress );
    
    if( base_lut_it->first <= min ) {
      out_lut[ base_lut_it->first ] = output_channel_min_level;
    } else if( base_lut_it->first >= max ) {
      out_lut[ base_lut_it->first ] = output_channel_max_level;
    } else {
      mapped_level = factor * pow( base_lut_it->first - min, 2 );
      
      if( mapped_level < output_channel_min_level ) {
        mapped_level = output_channel_min_level;
      } else if( mapped_level > output_channel_max_level ) {
        mapped_level = output_channel_max_level;
      }        
      
      out_lut[ base_lut_it->first ] = mapped_level;
    }

    ++progress;
    ++base_lut_it;
  }

  return out_lut;
}


TePDITypes::TePDILutType TePDIContrast::GetLogLut(
  TePDITypes::TePDILutType& base_lut,
  double min, double max,
  double output_channel_min_level, double output_channel_max_level )
{
  TEAGN_CHECK_NOTEQUAL( base_lut.size(), 0, "Invalid base_lut size" );
  TEAGN_TRUE_OR_THROW( max > ( min+1 ), "Invalid min and max values" );

  unsigned int levels = base_lut.size();

  double factor = ((double)levels) / log10( max - min + 1 );

  /* Generating LUT map using the existing base_lut levels */

  TePDITypes::TePDILutType out_lut;

  TePDITypes::TePDILutType::iterator base_lut_it = base_lut.begin();
  TePDITypes::TePDILutType::iterator base_lut_it_end = base_lut.end();

  unsigned int progress = 0;
  double mapped_level = 0;
  StartProgInt( "Building Log Lut...", base_lut.size() ); 
    
  while( base_lut_it != base_lut_it_end ) {
    UpdateProgInt( progress );
    
    if( base_lut_it->first <= min ) {
      out_lut[ base_lut_it->first ] = output_channel_min_level;
    } else if( base_lut_it->first >= max ) {
      out_lut[ base_lut_it->first ] = output_channel_max_level;
    } else {
      mapped_level = factor * log10( base_lut_it->first - min + 1 );
      
      if( mapped_level < output_channel_min_level ) {
        mapped_level = output_channel_min_level;
      } else if( mapped_level > output_channel_max_level ) {
        mapped_level = output_channel_max_level;
      }        
    
      out_lut[ base_lut_it->first ] = mapped_level;
    }

    ++progress;
    ++base_lut_it;
  }

  return out_lut;
}


TePDITypes::TePDILutType TePDIContrast::GetNegativeLut(
  TePDITypes::TePDILutType& base_lut,
  double min, double max,
  double output_channel_min_level, double output_channel_max_level )
{
  TEAGN_CHECK_NOTEQUAL( base_lut.size(), 0, "Invalid base_lut size" );
  TEAGN_TRUE_OR_THROW( max > min, "Invalid max and min values" );

  /* Calculating parameters */

  unsigned int levels = base_lut.size();

  double a = -1. * ((double)levels) / ( max - min );
  double b = ( ((double)levels) * max ) / ( max - min );

  /* Generating LUT map using the existing base_lut levels */

  TePDITypes::TePDILutType out_lut;

  TePDITypes::TePDILutType::iterator base_lut_it = base_lut.begin();
  TePDITypes::TePDILutType::iterator base_lut_it_end = base_lut.end();

  unsigned int progress = 0;
  double mapped_level = 0;
  StartProgInt( "Building Negative Lut...", base_lut.size() ); 
    
  while( base_lut_it != base_lut_it_end ) {
    UpdateProgInt( progress );
    
    if( base_lut_it->first <= min ) {
      out_lut[ base_lut_it->first ] = output_channel_max_level;
    } else if( base_lut_it->first >= max ) {
      out_lut[ base_lut_it->first ] = output_channel_min_level;
    } else {
      mapped_level = ( a * base_lut_it->first ) + b;
      
      if( mapped_level < output_channel_min_level ) {
        mapped_level = output_channel_min_level;
      } else if( mapped_level > output_channel_max_level ) {
        mapped_level = output_channel_max_level;
      }
      
      out_lut[ base_lut_it->first ] = mapped_level;
    }

    ++progress;
    ++base_lut_it;
  }

  return out_lut;
}

TePDITypes::TePDILutType TePDIContrast::GetHistEqualizerLut(
  TePDIHistogram::pointer hist,
  double output_channel_min_level, double output_channel_max_level )
{
  TEAGN_TRUE_OR_THROW( hist->size() > 1, "Invalid histogram size" );
  TEAGN_TRUE_OR_THROW( ( output_channel_max_level > output_channel_min_level ),
    "Invalid paramters output_channel_max_level <= output_channel_min_level" );
  TEAGN_TRUE_OR_THROW( ( hist->GetMaxCount() > 0 ),
    "Invalid histogram" );
    
  /* Generating the accumulative matrix */

  TePDIHistogram::iterator in_hist_it = hist->begin();
  TePDIHistogram::iterator in_hist_it_end = hist->end();

  TePDIMatrix< double > accumulative_matrix;
  TEAGN_TRUE_OR_THROW( accumulative_matrix.Reset( hist->size(), 2 ),
    "Matrix reset error" );
    
  accumulative_matrix( 0, 0 ) = in_hist_it->first;
  accumulative_matrix( 0, 1 ) = (double)in_hist_it->second;
  ++in_hist_it;
  
  unsigned int accumulative_matrix_line = 1;
  double hist_population = (double)in_hist_it->second;
  
  StartProgInt( "Building Histogram Equalizer Lut...", 2 * hist->size() );
  
  while( in_hist_it != in_hist_it_end ) { 
    accumulative_matrix( accumulative_matrix_line, 0 ) = in_hist_it->first;
    accumulative_matrix( accumulative_matrix_line, 1 ) =
      accumulative_matrix( accumulative_matrix_line - 1, 1 ) +
      (double)in_hist_it->second;
      
     hist_population += (double)in_hist_it->second;
  
    ++accumulative_matrix_line;
    ++in_hist_it;
    
    IncProgInt();
  }
  
  /* Creating the look-up table */

  double total_levels_nmb = (double)hist->size();
  double mapped_level = 0;
  TePDITypes::TePDILutType out_lut;
  
  for( accumulative_matrix_line = 0 ; 
    accumulative_matrix_line < accumulative_matrix.GetLines() ;
    ++accumulative_matrix_line ) {
    
    mapped_level = ( accumulative_matrix( accumulative_matrix_line, 1 ) *
      total_levels_nmb ) / hist_population;
      
    if( mapped_level < output_channel_min_level ) {
      mapped_level = output_channel_min_level;
    } else if( mapped_level > output_channel_max_level ) {
      mapped_level = output_channel_max_level;
    }
    
    out_lut[ accumulative_matrix( accumulative_matrix_line, 0 ) ] = 
      mapped_level;
      
    IncProgInt();  
  }
  
  StopProgInt();
  
  return out_lut;
}


void TePDIContrast::GetSimpleSlicerLut(
  TePDIHistogram::pointer hist,
  TePDIRgbPalette::pointer in_palette,
  double min,
  double max,
  TePDITypes::TePDILutType& out_lut )
{
  TEAGN_TRUE_OR_THROW( ( hist->size() > 0 ), "Invalid histogram size" );
  TEAGN_TRUE_OR_THROW( max > min, "Invalid max and min values" );
  TEAGN_TRUE_OR_THROW( in_palette->size() > 0,
    "Invalid input palette size" );

  out_lut.clear();

  /* Extracting palette levels */

  std::vector< double > palette_levels;
  TePDIRgbPalette::iterator pal_it = in_palette->begin();
  TePDIRgbPalette::iterator pal_it_end = in_palette->end();
  
  unsigned int progress = 0;
  StartProgInt( "Building Simple Slicer Lut...", hist->size() +
    in_palette->size() );    

  while( pal_it != pal_it_end ) {
    UpdateProgInt( progress );
    
    palette_levels.push_back( pal_it->first );

    ++progress;
    ++pal_it;
  }

  /* min and max adjusting to the supplied histogram range */

  double in_hist_max = hist->GetMaxLevel();
  double in_hist_min = hist->GetMinLevel();

  min = ( min < in_hist_min ) ? in_hist_min : min;
  max = ( max > in_hist_max ) ? in_hist_max : max;

  /* Output LUT generation */

  double slice_size = ( max - min ) / ((double)in_palette->size());

  double first_pal_level = palette_levels[ 0 ];
  double last_pal_level = palette_levels[ palette_levels.size() - 1 ];

  TePDIHistogram::iterator in_hist_it = hist->begin();
  TePDIHistogram::iterator in_hist_it_end = hist->end();

  double current_level;
  unsigned int current_slice;
  
  while( in_hist_it != in_hist_it_end ) {
    UpdateProgInt( progress );
    
    current_level = in_hist_it->first;

    if( current_level <= min ) {
      out_lut[ current_level ] = first_pal_level;
    } else if ( current_level >= max ) {
      out_lut[ current_level ] = last_pal_level;
    } else {
      current_slice =
        (unsigned int) floor( ( current_level - min ) / slice_size );
        
      TEAGN_DEBUG_CONDITION( ( current_slice < palette_levels.size() ),
        "Invalid current_slice=" + Te2String( current_slice ) +
        " for pallete_levels size=" + Te2String( palette_levels.size() ) );

      out_lut[ current_level ] = palette_levels[ current_slice ];
    }

    ++progress;
    ++in_hist_it;
  }
}


void TePDIContrast::GetStatLut( TePDIHistogram::pointer hist,
  double target_mean, double target_variance, double output_channel_min_level, 
  double output_channel_max_level, TePDITypes::TePDILutType& out_lut )
{
  TEAGN_CHECK_NOTEQUAL( hist->size(), 0, "Invalid histogram size" );

  out_lut.clear();
  
  TePDIHistogram::iterator in_hist_it;
  TePDIHistogram::iterator in_hist_it_end = hist->end(); 
  
  /* Calculating the total pixels number */
  
  unsigned int total_pixels = 0;
  
  in_hist_it = hist->begin();
  
  while( in_hist_it != in_hist_it_end ) {
    total_pixels += in_hist_it->second;
        
    ++in_hist_it;
  }
  
  /* Calculating the current mean */ 
  
  double current_mean = 0;
  
  if( params_.CheckParameter< double >( "input_mean" ) ) {
    params_.GetParameter( "input_mean", current_mean );
  } else {
    in_hist_it = hist->begin();
    
    while( in_hist_it != in_hist_it_end ) {
      current_mean += ( in_hist_it->first * ((double)in_hist_it->second) );
        
      ++in_hist_it;
    }
  
    current_mean = current_mean / ((double)total_pixels);
  }
    
  /* Calculating the current variance */ 
        
  double current_variance = 0;
  
  if( params_.CheckParameter< double >( "input_variance" ) ) {
    params_.GetParameter( "input_variance", current_variance );
  } else {
    double temp_double = 0;
    
    in_hist_it = hist->begin();
    
    while( in_hist_it != in_hist_it_end ) {
      temp_double = ( in_hist_it->first - current_mean );
      temp_double = temp_double * temp_double * ((double)in_hist_it->second);
      
      current_variance += temp_double;
        
      ++in_hist_it;
    }
    
    current_variance = current_variance / ( (double) total_pixels );
  }
  
  /* Creating levels map */
  
  double gain = sqrt( target_variance / current_variance );
  double offset = target_mean - ( gain * current_mean );
  
  in_hist_it = hist->begin();
  
  double current_level = 0;
  double mapped_level = 0;
  
  while( in_hist_it != in_hist_it_end ) {
    current_level = in_hist_it->first;
    mapped_level = ( current_level * gain ) + offset;

    if( mapped_level < output_channel_min_level ) {
      out_lut[ current_level ] = output_channel_min_level;
    } else if ( mapped_level > output_channel_max_level ) {
      out_lut[ current_level ] = output_channel_max_level;
    } else {
      out_lut[ current_level ] = mapped_level;
    }    
  
    ++in_hist_it;
  }
}


bool TePDIContrast::RemapLevels(
  TePDITypes::TePDIRasterPtrType& inRaster,
  TePDITypes::TePDILutType& lut,
  int in_channel,
  int out_channel,
  TePDITypes::TePDIRasterPtrType& outRaster,
  bool use_dummy, double dummy_value, bool fixed_step_lut )
{
  TEAGN_TRUE_OR_RETURN( inRaster.isActive(),
    "inRaster inactive" );
  TEAGN_TRUE_OR_RETURN( outRaster.isActive(),
    "outRaster inactive" );
  TEAGN_TRUE_OR_RETURN( 
    inRaster->params().status_ != TeRasterParams::TeNotReady,
    "inRaster not ready" );
  TEAGN_TRUE_OR_RETURN( 
    ( outRaster->params().status_ != TeRasterParams::TeNotReady ),
    "outRaster not ready" );
    
  TEAGN_TRUE_OR_RETURN( ( inRaster->params().nlines_ ==
    outRaster->params().nlines_ ),
    "Lines number mismatch between input and output image" );
  TEAGN_TRUE_OR_RETURN( ( inRaster->params().ncols_ ==
    outRaster->params().ncols_ ),
    "Columns number mismatch between input and output image" );
  TEAGN_TRUE_OR_RETURN( in_channel < inRaster->nBands(), 
    "Invalid input band" );
  TEAGN_TRUE_OR_RETURN( out_channel < outRaster->nBands(), 
    "Invalid output band" );
  TEAGN_TRUE_OR_RETURN( ( lut.size() > 1 ), "Invalid lut" );

  const int raster_lines = inRaster->params().nlines_;
  const int raster_columns = inRaster->params().ncols_;
  
  /* Guessing dummy use */
  
  bool inRaster_uses_dummy = inRaster->params().useDummy_;
  
  bool outRaster_uses_dummy = outRaster->params().useDummy_;
  double outRaster_dummy = 0;
  if( outRaster_uses_dummy ) {
    outRaster_dummy = outRaster->params().dummy_[ out_channel ];
  } else {
    outRaster_dummy = dummy_value;
  }
  
  /* Loading lut */
  
  TePDIMatrix< double > lutmatrix;
  TEAGN_TRUE_OR_RETURN( lutmatrix.Reset( 2, lut.size(), 
    TePDIMatrix< double >::AutoMemPol ),
    "Unable to create lut matrix" );
  {
    TePDITypes::TePDILutType::iterator it = lut.begin();;
  
    for( unsigned int lutcol = 0 ; lutcol < lutmatrix.GetColumns() ; 
      ++lutcol ) {
      
      lutmatrix( 0, lutcol ) = it->first;
      lutmatrix( 1, lutcol ) = it->second;
      
      ++it;
    }
  }
  
  /* Remapping levels */
  
  StartProgInt( "Remapping Levels...", raster_lines );

  if( fixed_step_lut ) {
    double current_level = 0;
    const double lut_min_level = lutmatrix( 0, 0 );
    const double lut_max_level = lutmatrix( 0, lutmatrix.GetColumns() - 1 );
    unsigned int best_lut_index = 0;
    const unsigned int lutmatrix_last_index = ( lutmatrix.GetColumns() - 1 );
    
    double lut_step = 0;
    if( lutmatrix.GetColumns() > 1 ) {
      lut_step = lutmatrix( 0, 1 ) - lutmatrix( 0, 0 );
    }
  
    for( int line = 0 ; line < raster_lines ; ++line ) {
      TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
    
      for( int column = 0 ; column < raster_columns ; ++column ) {
        if( inRaster->getElement( column, line, current_level,
            in_channel ) ) {
            
          if( use_dummy && ( current_level == dummy_value ) ) {
            TEAGN_TRUE_OR_RETURN( outRaster->setElement( column, line,
              outRaster_dummy, out_channel ),
              "Level remmaping error at " + Te2String( line ) +
              "," + Te2String( column ) );           
          } else {
            /* Finding the mapping level from lut */
            
            if( current_level < lut_min_level ) {
              best_lut_index = 0;
            } else if( current_level > lut_max_level ) {
              best_lut_index = lutmatrix_last_index;
            } else {
              best_lut_index = ( unsigned int ) ( TeRound( ( current_level - 
                lut_min_level ) / lut_step ) );             
            }
            
            /* Pixel Output level remapping */
            
            TEAGN_TRUE_OR_RETURN( outRaster->setElement( column, line,
              lutmatrix( 1, best_lut_index ), out_channel ),
              "Level remmaping error at " + Te2String( line ) +
              "," + Te2String( column ) );
          }
        } else {
          TEAGN_TRUE_OR_RETURN( inRaster_uses_dummy, "Raster read error" );
            
          TEAGN_TRUE_OR_RETURN( outRaster->setElement( column, line,
            outRaster_dummy, out_channel ),
            "Level remmaping error at " + Te2String( line ) +
            "," + Te2String( column ) );          
        }
      }
    }
  } else {
    double current_level;
    unsigned int left_element_index;
    unsigned int middle_element_index;
    unsigned int right_element_index;
    const unsigned int last_valid_index = lutmatrix.GetColumns() - 1;
    const unsigned int lut_size = lutmatrix.GetColumns();
    double middle_element_level;
  
    for( int line = 0 ; line < raster_lines ; ++line ) {
      TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
    
      for( int column = 0 ; column < raster_columns ; ++column ) {
        if( inRaster->getElement( column, line, current_level,
            in_channel ) ) {
            
          if( use_dummy && ( current_level == dummy_value ) ) {
            TEAGN_TRUE_OR_RETURN( outRaster->setElement( column, line,
              outRaster_dummy, out_channel ),
              "Level remmaping error at " + Te2String( line ) +
              "," + Te2String( column ) );           
          } else {            
            /* Finding the two best mapping levels from lut */
            
            if( lut_size == 1 ) {
              right_element_index = left_element_index = 1;
            } else {
              left_element_index = 0;
              right_element_index = last_valid_index;
            
              do {
                middle_element_index =  left_element_index + 
                  ( ( right_element_index - left_element_index ) / 2 );
                middle_element_level = lutmatrix( 0, middle_element_index );
                          
                if( current_level == middle_element_level ) {
                  left_element_index = right_element_index = 
                    middle_element_index;
                  break;
                } else if( current_level < middle_element_level ) {
                  right_element_index = middle_element_index;
                } else {
                  left_element_index = middle_element_index;
                }          
              } while( ( right_element_index - left_element_index ) > 1 );
            }
            
            /* Pixel Output level aproximation and mapping */
            
            if( ( current_level - lutmatrix( 0, left_element_index ) ) <
                ( lutmatrix( 0, right_element_index ) - current_level ) ) {
    
              TEAGN_TRUE_OR_RETURN( outRaster->setElement( column, line,
                lutmatrix( 1, left_element_index ), out_channel ),
                "Level remmaping error at " + Te2String( line ) +
                "," + Te2String( column ) );
            } else {
              TEAGN_TRUE_OR_RETURN( outRaster->setElement( column, line,
                lutmatrix( 1, right_element_index ), out_channel ),
                "Level remmaping error at " + Te2String( line ) +
                "," + Te2String( column ) );
            }
          }
        } else {
          TEAGN_TRUE_OR_RETURN( inRaster_uses_dummy, "Raster read error" );
          
          TEAGN_TRUE_OR_RETURN( outRaster->setElement( column, line,
            outRaster_dummy, out_channel ),
            "Level remmaping error at " + Te2String( line ) +
            "," + Te2String( column ) );          
        }
      }
    }
  }
  
  StopProgInt();
  
  return true;
}


bool TePDIContrast::FullRangeLutRemapLevels( 
  TePDITypes::TePDIRasterPtrType& inRaster, TePDITypes::TePDILutType& lut,
  int in_channel, int out_channel, TePDITypes::TePDIRasterPtrType& outRaster, 
  bool use_dummy, double dummy_value )
{
  TEAGN_TRUE_OR_RETURN( inRaster.isActive(),
    "inRaster inactive" );
  TEAGN_TRUE_OR_RETURN( outRaster.isActive(),
    "outRaster inactive" );
  TEAGN_TRUE_OR_RETURN( 
    inRaster->params().status_ != TeRasterParams::TeNotReady,
    "inRaster not ready" );
  TEAGN_TRUE_OR_RETURN( 
    ( outRaster->params().status_ != TeRasterParams::TeNotReady ),
    "outRaster not ready" );
    
  TEAGN_TRUE_OR_RETURN( ( inRaster->params().nlines_ ==
    outRaster->params().nlines_ ),
    "Lines number mismatch between input and output image" );
  TEAGN_TRUE_OR_RETURN( ( inRaster->params().ncols_ ==
    outRaster->params().ncols_ ),
    "Columns number mismatch between input and output image" );
  TEAGN_TRUE_OR_RETURN( in_channel < inRaster->nBands(), 
    "Invalid input band" );
  TEAGN_TRUE_OR_RETURN( out_channel < outRaster->nBands(), 
    "Invalid output band" );
  TEAGN_TRUE_OR_RETURN( ( lut.size() > 1 ), "Invalid lut" );

  const int raster_lines = inRaster->params().nlines_;
  const int raster_columns = inRaster->params().ncols_;
  
  /* Guessing dummy use */
  
  bool inRaster_uses_dummy = inRaster->params().useDummy_;
  
  bool outRaster_uses_dummy = outRaster->params().useDummy_;
  double outRaster_dummy = 0;
  if( outRaster_uses_dummy ) {
    outRaster_dummy = outRaster->params().dummy_[ out_channel ];
  } else {
    outRaster_dummy = dummy_value;
  }
  
  /* Loading lut */
  
  TePDIMatrix< double > lutmatrix;
  TEAGN_TRUE_OR_RETURN( lutmatrix.Reset( 2, lut.size(), 
    TePDIMatrix< double >::AutoMemPol ),
    "Unable to create lut matrix" );
  {
    TePDITypes::TePDILutType::iterator it = lut.begin();;
  
    for( unsigned int lutcol = 0 ; lutcol < lutmatrix.GetColumns() ; 
      ++lutcol ) {
      
      lutmatrix( 0, lutcol ) = it->first;
      lutmatrix( 1, lutcol ) = it->second;
      
      ++it;
    }
  }
  
  /* Remapping levels */
  
  TeRaster& inRaster_ref = *inRaster;
  TeRaster& outRaster_ref = *outRaster;
  int line = 0;
  int column = 0 ;
  double current_level = 0;
  
  StartProgInt( "Remapping Levels...", raster_lines );
  
  for( line = 0 ; line < raster_lines ; ++line ) {
    TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
    
    for( column = 0 ; column < raster_columns ; ++column ) {
      if( inRaster_ref.getElement( column, line, current_level,
        in_channel ) ) { 
        
        if( use_dummy && ( current_level == dummy_value ) ) {
          TEAGN_TRUE_OR_RETURN( outRaster_ref.setElement( column, line,
            outRaster_dummy, out_channel ),
            "Level remmaping error at " + Te2String( line ) +
            "," + Te2String( column ) );       
        } else {
          TEAGN_DEBUG_CONDITION( ( ( (unsigned int)current_level ) <
            lutmatrix.GetColumns() ), "Level out of lut range" );
          TEAGN_DEBUG_CONDITION( 
            ( lutmatrix( 0, ( (unsigned int)current_level ) ) ==
            ( (unsigned int)current_level ) ), 
            "Requested value not found inside lut" );
          
          TEAGN_TRUE_OR_RETURN( outRaster_ref.setElement( column, line,
            lutmatrix( 1, ( (unsigned int)current_level ) ), out_channel ),
            "Level remmaping error at " + Te2String( line ) +
            "," + Te2String( column ) );          
        }
      } else {
        TEAGN_TRUE_OR_RETURN( inRaster_uses_dummy, "Raster read error" );
            
        TEAGN_TRUE_OR_RETURN( outRaster_ref.setElement( column, line,
          outRaster_dummy, out_channel ),
          "Level remmaping error at " + Te2String( line ) +
          "," + Te2String( column ) );       
      }
    }
  }
  
  StopProgInt();
  
  return true;  
}


bool TePDIContrast::getHistogram( TePDIHistogram::pointer& hist,
  bool useDummy, double dummyValue )
{
  if( ! histo_ptr_.isActive() ) {
    if( params_.CheckParameter< TePDIHistogram::pointer >( 
      "input_histogram" ) ) {
    
      params_.GetParameter( "input_histogram", histo_ptr_ );
    } else {
      /* No histogram supplied, we need to generate it */
  
      TePDITypes::TePDIRasterPtrType inRaster;
      TEAGN_TRUE_OR_RETURN( params_.GetParameter( "input_image", inRaster ),
        "Missing parameter : input_image" );
      
      int input_band = 0;
      TEAGN_TRUE_OR_RETURN( params_.GetParameter( "input_band", input_band ),
        "Missing parameter : input_band" );
      
      int histo_levels = 0;
      if( ( inRaster->params().dataType_[ input_band ] == TeFLOAT ) ||
        ( inRaster->params().dataType_[ input_band ] == TeDOUBLE ) ) {
        
        histo_levels = 256;
      }
      if( params_.CheckParameter< int >( "histo_levels" ) ) {
        params_.GetParameter( "histo_levels", histo_levels );
      }
      
      histo_ptr_.reset( new TePDIHistogram );
      TEAGN_TRUE_OR_RETURN( 
        histo_ptr_->reset( inRaster, input_band, 
        (unsigned int)histo_levels, 
        TeBoxPixelIn ), "Histogram generation error" );
        
      // Removing the dummy value from the generated histrogram
      
      if( useDummy )
      {
        histo_ptr_->erase( dummyValue );
      }
    }
  }
  
  hist = histo_ptr_;
  
  return true;
}

bool TePDIContrast::getBaseLut( TePDITypes::TePDILutType& lut,
  bool& hist_based_lut, bool useDummy, double dummyValue )
{
  lut.clear();
  
  if( params_.CheckParameter< TePDIHistogram::pointer >( 
    "input_histogram" ) ) 
  {
  
    TePDIHistogram::pointer hist;
    TEAGN_TRUE_OR_RETURN( getHistogram( hist, useDummy, dummyValue ), 
      "Unable to get histogram" );    
    
    TePDIHistogram::iterator it = hist->begin();
    TePDIHistogram::iterator it_end = hist->end();
      
    while( it != it_end ) {
      lut[ it->first ] = it->first;
    
      ++it;
    }
      
    hist_based_lut = true;    
  } else {
    TePDITypes::TePDIRasterPtrType inRaster;
    params_.GetParameter( "input_image", inRaster );
    
    int input_band = 0;
    params_.GetParameter( "input_band", input_band );
      
    switch( inRaster->params().dataType_[ input_band ] ) {
      case TeBIT :
      case TeUNSIGNEDCHAR :
      case TeUNSIGNEDSHORT :
      {
        unsigned int lut_size = (unsigned int)( 
          pow( 2.0, (double)( inRaster->params().elementSize( input_band ) * 
          8 ) ) );
      
        for( unsigned int index = 0 ; index < lut_size ; ++index ) {
          lut[ index ] = index;
        }
            
        hist_based_lut = false;
        
        break;
      }
      case TeCHAR :
      case TeSHORT :      
      case TeINTEGER :
      case TeLONG :
      case TeUNSIGNEDLONG :
      case TeFLOAT :
      case TeDOUBLE :
      {
        TePDIHistogram::pointer hist;
        TEAGN_TRUE_OR_RETURN( getHistogram( hist, useDummy, dummyValue ), 
          "Unable to get histogram" );
        
        TePDIHistogram::iterator it = hist->begin();
        TePDIHistogram::iterator it_end = hist->end();
        
        while( it != it_end ) {
          lut[ it->first ] = it->first;
      
          ++it;
        }
        
        hist_based_lut = true;
        
        break;
      }
      default :
      {
        TEAGN_LOG_AND_THROW( "Invalid raster pixel type" );
        break;
      }
    }
  }
  
  return true;
}

