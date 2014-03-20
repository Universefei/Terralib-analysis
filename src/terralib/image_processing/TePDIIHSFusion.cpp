#include "TePDIIHSFusion.hpp"
#include "TePDIUtils.hpp"
#include "TePDIColorTransform.hpp"
#include "TePDIStatistic.hpp"
#include "TePDIContrast.hpp"


TePDIIHSFusion::TePDIIHSFusion()
{
}


TePDIIHSFusion::~TePDIIHSFusion()
{
}


void TePDIIHSFusion::ResetState( const TePDIParameters& )
{
}


bool TePDIIHSFusion::CheckParameters( const TePDIParameters& parameters ) const
{
  /* Checking input_raster1 */
  
  TePDITypes::TePDIRasterPtrType reference_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "reference_raster", 
    reference_raster ),
    "Missing parameter: reference_raster" );
  TEAGN_TRUE_OR_RETURN( reference_raster.isActive(),
    "Invalid parameter: reference_raster inactive" );
  TEAGN_TRUE_OR_RETURN( reference_raster->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: reference_raster not ready" );
    
  /* Checking input_raster2 */
  
  TePDITypes::TePDIRasterVectorType lowres_rasters;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "lowres_rasters", lowres_rasters ),
    "Missing parameter: lowres_rasters" );
    
  TEAGN_TRUE_OR_RETURN( ( lowres_rasters.size() == 3 ),
    "Invalid number of lower resolution rasters" );    
    
  std::vector< int > lowres_channels;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "lowres_channels", lowres_channels ),
    "Missing parameter: lowres_channels" );
    
  TEAGN_TRUE_OR_RETURN( ( lowres_channels.size() == 3 ), 
    "Invalid number of resolution channels" );
    
  for( unsigned int index = 0 ; index < lowres_rasters.size() ; ++index ) {
    TEAGN_TRUE_OR_RETURN( lowres_rasters[ index ].isActive(),
      "Invalid parameter: raster " + 
      Te2String( index ) + " inactive" );
      
    TEAGN_TRUE_OR_RETURN( 
      lowres_rasters[ index ]->params().status_ != TeRasterParams::TeNotReady,
      "Invalid parameter: raster " + 
      Te2String( index ) + " not ready" );
        
            
    TEAGN_TRUE_OR_RETURN( 
      lowres_channels[ index ] >= 0, "Invalid channel number (" + 
      Te2String( index ) + ")" );
    TEAGN_TRUE_OR_RETURN( 
      lowres_channels[ index ] < lowres_rasters[ index ]->nBands(), 
      "Invalid channel number (" + 
      Te2String( index ) + ")" );
      
    /* Checking photometric interpretation */
    
    TEAGN_TRUE_OR_RETURN( ( 
      ( lowres_rasters[ index ]->params().photometric_[ 
        lowres_channels[ index ] ] == TeRasterParams::TeRGB ) ||
      ( lowres_rasters[ index ]->params().photometric_[ 
        lowres_channels[ index ] ] == TeRasterParams::TeMultiBand ) ),
      "Invalid parameter - rasters (invalid photometric "
      "interpretation)" );      
  }  

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
    
  /* Checking photometric interpretation */
  
  TEAGN_TRUE_OR_RETURN( ( 
    ( reference_raster->params().photometric_[ reference_raster_band ] == 
      TeRasterParams::TeRGB ) ||
    ( reference_raster->params().photometric_[ reference_raster_band ] == 
      TeRasterParams::TeMultiBand ) ),
  "Invalid parameter - reference_raster (invalid photometric interpretation)" );   

  return true;
}


bool TePDIIHSFusion::RunImplementation()
{
  TePDITypes::TePDIRasterPtrType reference_raster;
  params_.GetParameter( "reference_raster", reference_raster );

  TePDITypes::TePDIRasterVectorType lowres_rasters;
  params_.GetParameter( "lowres_rasters", lowres_rasters );
    
  std::vector< int > lowres_channels;
  params_.GetParameter( "lowres_channels", lowres_channels );
  
  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter( "output_raster", output_raster );
    
  int reference_raster_band = 0;    
  params_.GetParameter( "reference_raster_band", reference_raster_band );
  
  TePDIInterpolator::InterpMethod interpol_method = 
    TePDIInterpolator::BicubicMethod;
  if( params_.CheckParameter<TePDIInterpolator::InterpMethod>( 
    "interpol_method" ) )
  {
    params_.GetParameter( "interpol_method", interpol_method );
  }
  
  /* Bringing lowres_rasters to reference_raster size */

  TePDITypes::TePDIRasterVectorType input_rasters_ihs;

  for( unsigned int index = 0 ; index < lowres_rasters.size() ; ++index ) {
    
	TeRaster& lowres_raster = *( lowres_rasters[ index ].nakedPointer() );
	TePDITypes::TePDIRasterPtrType upsampled_raster;
    
	TeRasterParams upsampled_raster_params = lowres_raster.params();
    upsampled_raster_params.setNLinesNColumns( 1, 1 );
    
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( upsampled_raster,
      upsampled_raster_params, TePDIUtils::TePDIUtilsAutoMemPol ),
      "Unable create the new upsampled raster RGB raster" );
      
    TEAGN_TRUE_OR_RETURN( TePDIUtils::resampleRasterByLinsCols(
      lowres_rasters[ index ], 
      upsampled_raster, 
      (unsigned int)reference_raster->params().nlines_, 
      (unsigned int)reference_raster->params().ncols_, 
      progress_enabled_,
      interpol_method), "Raster resample error" ); 

	input_rasters_ihs.push_back(upsampled_raster);
/*    
TEAGN_TRUE_OR_THROW( TePDIUtils::TeRaster2Geotiff( upsampled_raster,
  "TePDIIHSFusion_upsampled_raster.tif" ), 
  "GeoTIF generation error" );     
*/      
  }
   
  /* Generating the IHS raster */
  
  TePDITypes::TePDIRasterPtrType ihs_raster;
  TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( ihs_raster,
    1, 10, 10, false, TeFLOAT, 0 ),
    "Unable create the new ihs_raster raster RGB raster" );  
    
  double rgb_channels_min = 0;
  double rgb_channels_max = 0;
  TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds( 
    input_rasters_ihs[0],
    0, rgb_channels_min, rgb_channels_max ),
    "Unable to get channel range" )
     
  {
    TePDIParameters rgb2ihs_params;
    
    rgb2ihs_params.SetParameter( "input_rasters", input_rasters_ihs );
    
    rgb2ihs_params.SetParameter( "input_channels", lowres_channels );
    
    TePDITypes::TePDIRasterVectorType output_rasters;
    output_rasters.push_back( ihs_raster );
    rgb2ihs_params.SetParameter( "output_rasters", output_rasters );
    
    rgb2ihs_params.SetParameter( "transf_type", TePDIColorTransform::Rgb2Ihs );  

    rgb2ihs_params.SetParameter( "rgb_channels_min", rgb_channels_min );
    rgb2ihs_params.SetParameter( "rgb_channels_max", rgb_channels_max );    
    
    TePDIColorTransform transform_instance;
    TEAGN_TRUE_OR_RETURN( transform_instance.Reset( rgb2ihs_params ),
      "Error in IHS parameters" )
      
    transform_instance.ToggleProgInt( progress_enabled_ );
      
    TEAGN_TRUE_OR_RETURN( transform_instance.Apply(), 
      "Unable to build IHS color space" );
  }
  
/*  
TEAGN_TRUE_OR_THROW( TePDIUtils::TeRaster2Geotiff( ihs_raster,
  "TePDIIHSFusion_ihs_raster.tif" ), 
  "GeoTIF generation error" );    
*/

  /* Free the anused raster */
  for( unsigned int index = 0 ; index < lowres_rasters.size() ; ++index ) {
    lowres_rasters[ index ].reset();
  }
  
  /* Bring the reference raster to the mean and variance of
     the I channel */
  
  TePDITypes::TePDIRasterPtrType ref_rast_contrast;
  
  {
    {
      /* Get stat params */
      
      double target_mean = 0;
      double target_variance = 0;
      double current_mean = 0;
      double current_variance = 0; 
      
      {
        TePDIParameters statparams;
        
        TePDITypes::TePDIRasterVectorType rasters;
        rasters.push_back( ihs_raster );
        rasters.push_back( reference_raster );
        statparams.SetParameter( "rasters", rasters );
        
        std::vector< int > bands;
        bands.push_back( 0 ); // IHS raster I band
        bands.push_back( reference_raster_band ); // reference_raster band
        statparams.SetParameter( "bands", bands );
        
        TePDIStatistic stat;
        stat.ToggleProgInt( progress_enabled_ );
        TEAGN_TRUE_OR_THROW( stat.Reset( statparams ),
          "Invalid statistic algorithm parameters" )
          
        target_mean = stat.getMean( 0 );
        target_variance = stat.getVariance( 0 );
        current_mean = stat.getMean( 1 );
        current_variance = stat.getVariance( 1 );        
      }
      
      double gain = ( ( current_variance == 0.0 ) ? 1.0 :
        sqrt( target_variance / current_variance ) );
      double offset = target_mean - ( gain * current_mean );      

      /* Applying contrast to the reference raster */
      
      TeRasterParams ref_rast_cont_params = reference_raster->params();
      ref_rast_cont_params.nBands( 1 );
      ref_rast_cont_params.setDataType( TeFLOAT, -1 );
            
      TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( ref_rast_contrast,
        ref_rast_cont_params, TePDIUtils::TePDIUtilsAutoMemPol ),
        "Unable create the new reference raster (contrast)" ); 
         
      const unsigned int nLines = reference_raster->params().nlines_;
      const unsigned int nCols = reference_raster->params().ncols_;
      unsigned line = 0;
      unsigned col = 0;
      double curr_value = 0;
      double new_value = 0;
      TeRaster& reference_raster_ref = (*reference_raster);
      TeRaster& ref_rast_contrast_ref = (*ref_rast_contrast);
      
      TePDIPIManager progress( "Applying contrast", nLines,
        progress_enabled_ );      
              
      for( line = 0; line < nLines ; ++line )
      {
        for( col = 0; col < nCols ; ++col )
        {
          reference_raster_ref.getElement( col, line, curr_value, 
            reference_raster_band );
            
          new_value = ( curr_value * gain ) + offset;
          new_value = MAX( new_value, 0.0 );
          new_value = MIN( new_value, 1.0 );
            
          TEAGN_TRUE_OR_THROW( ref_rast_contrast_ref.setElement( col, line, 
            new_value, 0 ), "Error writing value" );            
        }
        
        TEAGN_FALSE_OR_RETURN( progress.Increment(), 
          "Canceled by the user" );
      }
    }
  }
/*  
TEAGN_TRUE_OR_THROW( TePDIUtils::TeRaster2Geotiff( ref_rast_contrast,
  "TePDIIHSFusion_ref_rast_contrast.tif" ), 
  "GeoTIF generation error" );   
*/    
  /* Swapping reference_raster into ihs_raster I component */
  {
    const unsigned int lines_bound = ihs_raster->params().nlines_;
    const unsigned int cols_bound = ihs_raster->params().ncols_;
    unsigned int line = 0;
    unsigned int col = 0;
    TeRaster* inraster = ref_rast_contrast.nakedPointer();
    TeRaster* outraster = ihs_raster.nakedPointer();
    double value = 0;
    
    TePDIPIManager progress( "Swapping Intensity channel...", lines_bound,
      progress_enabled_ );
    
    for( line = 0; line < lines_bound ; ++line ) {
      TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
    
      for( col = 0; col < cols_bound ; ++col ) {
        inraster->getElement( col, line, value, 0 );
        
        outraster->setElement( col, line, value, 0 );
      }
    }
  }
  
/* 
TEAGN_TRUE_OR_THROW( TePDIUtils::TeRaster2Geotiff( ihs_raster,
  "TePDIIHSFusion_ihs_raster_swapped.tif" ), 
  "GeoTIF generation error" );     
*/  

  /* Switching back to RBG space */
  {
    TePDIParameters ihs2rgb_params;
    
    TePDITypes::TePDIRasterVectorType input_rasters;
    input_rasters.push_back( ihs_raster );
    input_rasters.push_back( ihs_raster );
    input_rasters.push_back( ihs_raster );
    ihs2rgb_params.SetParameter( "input_rasters", input_rasters );
    
    std::vector< int >input_channels;
    input_channels.push_back( 0 );
    input_channels.push_back( 1 );
    input_channels.push_back( 2 );
    ihs2rgb_params.SetParameter( "input_channels", input_channels );
    
    TePDITypes::TePDIRasterVectorType output_rasters;
    output_rasters.push_back( output_raster );    
    ihs2rgb_params.SetParameter( "output_rasters", output_rasters );
    
    ihs2rgb_params.SetParameter( "transf_type", TePDIColorTransform::Ihs2Rgb );  
    
    ihs2rgb_params.SetParameter( "rgb_channels_min", rgb_channels_min );
    ihs2rgb_params.SetParameter( "rgb_channels_max", rgb_channels_max );
    
    TePDIColorTransform transform_instance;
    TEAGN_TRUE_OR_RETURN( transform_instance.Reset( ihs2rgb_params ),
      "Error in IHS parameters" )    
    transform_instance.ToggleProgInt( progress_enabled_ );
    TEAGN_TRUE_OR_RETURN( transform_instance.Apply(), 
      "Unable to build RGB color space" );      
  }
   
  return true;
}


