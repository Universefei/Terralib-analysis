#include "TePDIPCAWaveletFusion.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"
#include "TePDITypes.hpp"
#include "TePDIStatistic.hpp"
#include "TePDIPrincipalComponents.hpp"
#include "TePDIWaveletAtrous.hpp"
#include "../kernel/TeRasterRemap.h"
#include "../kernel/TeMatrix.h"
#include "../kernel/TeUtils.h"
#include <math.h>

#define waveletPlanes 2

TePDIPCAWaveletFusion::TePDIPCAWaveletFusion()
{
}

TePDIPCAWaveletFusion::~TePDIPCAWaveletFusion()
{
}

void TePDIPCAWaveletFusion::ResetState(const TePDIParameters&)
{
}

bool TePDIPCAWaveletFusion::CheckParameters(const TePDIParameters& parameters) const
{
/* Input rasters and bands checking */
	TePDITypes::TePDIRasterVectorType input_rasters;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");
	TEAGN_TRUE_OR_RETURN(input_rasters.size() > 1, "Invalid input rasters number");
	
	std::vector<int> bands;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("bands", bands), "Missing parameter: bands");
	TEAGN_TRUE_OR_RETURN(bands.size() == input_rasters.size(), "Invalid parameter: bands number");

	for( unsigned int input_rasters_index = 0 ; input_rasters_index < input_rasters.size(); ++input_rasters_index )
	{
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index].isActive(), "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " inactive");
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " not ready");
		TEAGN_TRUE_OR_RETURN(bands[input_rasters_index] < input_rasters[input_rasters_index]->nBands(), "Invalid parameter: bands[" + Te2String(input_rasters_index) + "]");
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().nlines_ == input_rasters[0]->params().nlines_, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " with imcompatible number of lines");
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().ncols_ == input_rasters[0]->params().ncols_, "Invalid parameter: input_raster " + Te2String( input_rasters_index ) + " with imcompatible number of columns" );
		TEAGN_TRUE_OR_RETURN(((input_rasters[input_rasters_index]->params().photometric_[bands[input_rasters_index]] == TeRasterParams::TeRGB) || (input_rasters[input_rasters_index]->params().photometric_[bands[input_rasters_index]] == TeRasterParams::TeMultiBand)), "Invalid parameter - input_rasters (invalid photometric interpretation)");
	}

/* Reference raster checking */
	int reference_raster_band;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_raster_band", reference_raster_band), "Missing parameter: reference_raster_band");

	TePDITypes::TePDIRasterPtrType reference_raster;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_raster", reference_raster), "Missing parameter: reference_raster");
	TEAGN_TRUE_OR_RETURN(((reference_raster->params().photometric_[reference_raster_band] == TeRasterParams::TeRGB ) || (reference_raster->params().photometric_[reference_raster_band] == TeRasterParams::TeMultiBand)), "Invalid parameter - reference_raster (invalid photometric interpretation)");

	TEAGN_TRUE_OR_RETURN(reference_raster_band < reference_raster->nBands(), "Invalid parameter - reference_raster_band");

/* Output rasters checking */
	TePDITypes::TePDIRasterVectorType output_rasters;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_rasters", output_rasters), "Missing parameter: output_rasters");
	TEAGN_TRUE_OR_RETURN(output_rasters.size() > 1, "Invalid output rasters number");
	TEAGN_TRUE_OR_RETURN(output_rasters.size() == input_rasters.size(), "Invalid parameter: output rasters size");

	for(unsigned int input_rasters_index2 = 0; input_rasters_index2 < input_rasters.size(); ++input_rasters_index2)
	{
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index2].isActive(), "Invalid parameter: input_raster " + Te2String(input_rasters_index2) + " inactive");
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index2]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_raster " + Te2String(input_rasters_index2) + " not ready");
	}

	int pca1_levels;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("pca1_levels", pca1_levels), "Missing parameter: pca1_levels");

	int reference_levels;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_levels", reference_levels), "Missing parameter: reference_levels");

	string filter_file;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("filter_file", filter_file), "Missing parameter: filter_file");

	TePDIPCAWaveletFusion::InterpMethod resampling_type;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("resampling_type", resampling_type), "Missing parameter: resampling_type");

	bool fit_histogram;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fit_histogram", fit_histogram), "Missing parameter: fit_histogram");

	return true;
}

bool TePDIPCAWaveletFusion::RunImplementation()
{
// Getting parameters
	TePDITypes::TePDIRasterVectorType input_rasters;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");

	std::vector<int> bands_direct;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("bands", bands_direct), "Missing parameter: bands");

	TePDITypes::TePDIRasterVectorType output_rasters;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("output_rasters", output_rasters), "Missing parameter: output_rasters");

	TePDITypes::TePDIRasterPtrType reference_raster;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_raster", reference_raster), "Missing parameter: reference_raster");

	int reference_raster_band;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_raster_band", reference_raster_band), "Missing parameter: reference_raster_band");

	int pca1_levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("pca1_levels", pca1_levels), "Missing parameter: pca1_levels");

	int reference_levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_levels", reference_levels), "Missing parameter: reference_levels");

	string filter_file;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("filter_file", filter_file), "Missing parameter: filter_file");

	TePDIPCAWaveletFusion::InterpMethod resampling_type;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("resampling_type", resampling_type), "Missing parameter: resampling_type");

	bool fit_histogram;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("fit_histogram", fit_histogram), "Missing parameter: fit_histogram");

// Resampling the input raster
	TeRasterParams reference_params = reference_raster->params();
  
	TePDITypes::TePDIRasterVectorType resampled_input_rasters;
	for(unsigned int b = 0; b < input_rasters.size(); b++)
	{
		if (resampling_type != TePDIPCAWaveletFusion::Nothing)
		{
		  TePDIInterpolator::InterpMethod intMethod;
		  switch( resampling_type )
		  {
		    case TePDIPCAWaveletFusion::NNMethod :
		    {
		      intMethod =  TePDIInterpolator::NNMethod;
		      break;
		    }
		    case TePDIPCAWaveletFusion::BilinearMethod :
		    {
		      intMethod =  TePDIInterpolator::BilinearMethod;
		      break;
		    }		  
		    case TePDIPCAWaveletFusion::BicubicMethod :
		    {
		      intMethod =  TePDIInterpolator::BicubicMethod;
		      break;
		    }				      
		    default :
		    {
		      TEAGN_LOG_AND_THROW( "Invalid resampling type" );
		    }
		  }
		  
			TePDITypes::TePDIRasterPtrType resampled_input_rasters_temp;
			TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(resampled_input_rasters_temp,  reference_params, TePDIUtils::TePDIUtilsAutoMemPol), "Unable create the resampled input rasters");
			resampleRasterByResLinsCols(input_rasters[b], resampled_input_rasters_temp, reference_params.resx_ / input_rasters[b]->params().resx_, reference_params.resy_ / input_rasters[b]->params().resy_, reference_params.nlines_,  reference_params.ncols_, intMethod, progress_enabled_);
			resampled_input_rasters.push_back(resampled_input_rasters_temp);
		}
		else
			resampled_input_rasters.push_back(input_rasters[b]);
	}

// PCA Direct analysis
	TePDITypes::TePDIRasterVectorType output_rasters_direct;
	for (unsigned b = 0; b < output_rasters.size(); b++)
	{
		TePDITypes::TePDIRasterPtrType outRaster_direct;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(outRaster_direct, 1, 1, 1, false, TeDOUBLE, 0), "RAM Raster " + Te2String(b) + " Alloc error");
		output_rasters_direct.push_back(outRaster_direct);
	}

	TeSharedPtr<TeMatrix> covariance_matrix(new TeMatrix);

	TePDIParameters params_direct;

	params_direct.SetParameter("analysis_type", 
    TePDIPrincipalComponents::TePDIPCADirect);
	params_direct.SetParameter("input_rasters", resampled_input_rasters);
	params_direct.SetParameter("bands", bands_direct);
	params_direct.SetParameter("output_rasters", output_rasters_direct);
	params_direct.SetParameter("covariance_matrix", covariance_matrix);
	
	TePDIPrincipalComponents pc_direct;
	TEAGN_TRUE_OR_THROW(pc_direct.Reset(params_direct), "Invalid Parameters");
	pc_direct.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(pc_direct.Apply(), "Apply error");

// Creating PCA 1 wavelet plane
	std::vector<TePDITypes::TePDIRasterVectorType> output_rasters_direct_wavelets;
	TePDITypes::TePDIRasterVectorType input_rasters_wavelets_vector_temp;
	input_rasters_wavelets_vector_temp.push_back(output_rasters_direct[0]);
	for(int l = 1; l <= pca1_levels; l++)
	{
		TePDITypes::TePDIRasterPtrType output_rasters_direct_wavelets_temp;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(output_rasters_direct_wavelets_temp, waveletPlanes, output_rasters_direct[0]->params().nlines_, output_rasters_direct[0]->params().ncols_, false, TeFLOAT, 0), "Unable to alloc input rasters wavelet planes");
		input_rasters_wavelets_vector_temp.push_back(output_rasters_direct_wavelets_temp);
	}
	output_rasters_direct_wavelets.push_back(input_rasters_wavelets_vector_temp);

// Extracting PCA 1 wavelet planes
	TePDIParameters pca1WaveletAtrousParams;
	pca1WaveletAtrousParams.SetParameter("direction", (int)TePDIWaveletAtrous::DECOMPOSE);
	pca1WaveletAtrousParams.SetParameter("input_raster", output_rasters_direct[0]);
	pca1WaveletAtrousParams.SetParameter("band", 0);
	pca1WaveletAtrousParams.SetParameter("levels", pca1_levels);
	pca1WaveletAtrousParams.SetParameter("filter_file", filter_file);
	pca1WaveletAtrousParams.SetParameter("output_wavelets", output_rasters_direct_wavelets[0]);
	TePDIWaveletAtrous wa_pca1;
	TEAGN_TRUE_OR_THROW(wa_pca1.Reset(pca1WaveletAtrousParams), "Invalid Parameters");
	wa_pca1.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(wa_pca1.Apply(), "Apply error");

// Creating reference raster wavelet planes
	double p;
	TePDITypes::TePDIRasterVectorType reference_raster_wavelets;
	for(int l = 0; l <= reference_levels; l++)
	{
		TePDITypes::TePDIRasterPtrType reference_raster_wavelets_temp;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(reference_raster_wavelets_temp, waveletPlanes, reference_params.nlines_, reference_params.ncols_, false, TeFLOAT, 0), "Reference raster wavelets Alloc error");
		if (l == 0)
		{
			for (int j = 0; j < reference_params.nlines_; j++)
			{
				for (int i = 0; i < reference_params.ncols_; i++)
				{
					reference_raster->getElement(i, j, p, reference_raster_band);
					reference_raster_wavelets_temp->setElement(i, j, p, 0);
				}
			}
		}
		reference_raster_wavelets.push_back(reference_raster_wavelets_temp);
	}

// Extracting reference raster wavelet planes
	TePDIParameters referenceWaveletAtrousParams;
	referenceWaveletAtrousParams.SetParameter("direction", (int)TePDIWaveletAtrous::DECOMPOSE);
	referenceWaveletAtrousParams.SetParameter("input_raster", reference_raster);
	referenceWaveletAtrousParams.SetParameter("band", reference_raster_band);
	referenceWaveletAtrousParams.SetParameter("levels", reference_levels);
	referenceWaveletAtrousParams.SetParameter("filter_file", filter_file);
	referenceWaveletAtrousParams.SetParameter("output_wavelets", reference_raster_wavelets);
	referenceWaveletAtrousParams.SetParameter("fit_histogram", fit_histogram);
	TePDIWaveletAtrous wa_reference;
	TEAGN_TRUE_OR_THROW(wa_reference.Reset(referenceWaveletAtrousParams), "Invalid Parameters");
	wa_reference.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(wa_reference.Apply(), "Apply error");

// Recomposing wavelets
	TePDIParameters waveletAtrousParamsRecompose;
	waveletAtrousParamsRecompose.SetParameter("direction", (int)TePDIWaveletAtrous::RECOMPOSE);
	waveletAtrousParamsRecompose.SetParameter("input_rasters_wavelets", output_rasters_direct_wavelets);
	waveletAtrousParamsRecompose.SetParameter("rasters_levels", pca1_levels);
	waveletAtrousParamsRecompose.SetParameter("reference_raster_wavelets", reference_raster_wavelets);
	waveletAtrousParamsRecompose.SetParameter("reference_levels", reference_levels);
	waveletAtrousParamsRecompose.SetParameter("output_rasters", output_rasters_direct);
	TePDIWaveletAtrous waRecompose;
	TEAGN_TRUE_OR_THROW(waRecompose.Reset(waveletAtrousParamsRecompose), "Invalid Parameters");
	waRecompose.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(waRecompose.Apply(), "Apply error");

// Inverse analysis
	
	TePDIParameters params_inverse;

	std::vector< int > bands_inverse;
	for (unsigned b = 0; b < output_rasters.size(); b++)
		bands_inverse.push_back(0);

	params_inverse.SetParameter("analysis_type", 
   TePDIPrincipalComponents::TePDIPCAInverse);
	params_inverse.SetParameter("input_rasters", output_rasters_direct);
	params_inverse.SetParameter("bands", bands_inverse);
	params_inverse.SetParameter("output_rasters", output_rasters);
	params_inverse.SetParameter("covariance_matrix", covariance_matrix);
	
	TePDIPrincipalComponents pc_inverse;
	TEAGN_TRUE_OR_THROW(pc_inverse.Reset(params_inverse), "Invalid Parameters");
	pc_inverse.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(pc_inverse.Apply(), "Apply error");

	return true;
}

bool TePDIPCAWaveletFusion::resampleRasterByResLinsCols( 
  const TePDITypes::TePDIRasterPtrType& input_raster,
  const TePDITypes::TePDIRasterPtrType& output_raster,
  double x_resolution_ratio, double y_resolution_ratio, int l, int c,
  TePDIInterpolator::InterpMethod interpol, bool enable_progress ) 
{
    
  TEAGN_TRUE_OR_RETURN( input_raster.isActive(), "Inactive input raster" );
  TEAGN_TRUE_OR_RETURN( 
    input_raster->params().status_ != TeRasterParams::TeNotReady,
    "Input raster not ready" )
  
  TEAGN_TRUE_OR_RETURN( output_raster.isActive(), 
    "Inactive output raster" );
  TEAGN_TRUE_OR_RETURN( 
    output_raster->params().status_ != TeRasterParams::TeNotReady,
    "Output raster not ready" )
    
  TEAGN_TRUE_OR_RETURN( ( x_resolution_ratio > 0.0 ), "Invalid X ratio" )
  TEAGN_TRUE_OR_RETURN( ( y_resolution_ratio > 0.0 ), "Invalid X ratio" )
  
  /* Reseting the output raster to the new geometry */
  
  TeRasterParams& input_raster_params = input_raster->params();
  
  TeRasterParams new_output_raster_params = output_raster->params();
  
  new_output_raster_params.nBands( input_raster_params.nBands() );
  new_output_raster_params.setPhotometric( 
    input_raster_params.photometric_[ 0 ], -1 );
  if( input_raster_params.projection() != 0 ) {
    new_output_raster_params.projection( input_raster_params.projection() );
  }
  new_output_raster_params.boxResolution( 
    input_raster_params.box().x1(), 
    input_raster_params.box().y1(), input_raster_params.box().x2(), 
    input_raster_params.box().y2(), 
    ( input_raster_params.resx_ * x_resolution_ratio ), 
    ( input_raster_params.resy_ * y_resolution_ratio ) );
  new_output_raster_params.lowerLeftResolutionSize(input_raster_params.box().lowerLeft().x(), input_raster_params.box().lowerLeft().y(), ( input_raster_params.resx_ * x_resolution_ratio ), ( input_raster_params.resy_ * y_resolution_ratio ), c, l);

    
  TEAGN_TRUE_OR_RETURN( output_raster->init( new_output_raster_params ),
    "Output raster reset error" )
//cout << output_raster->params().resx_ << endl;
    
  /* interpolating pixel values */
  
  const TeRasterParams& output_raster_params = output_raster->params();
  
  const unsigned int out_lines = output_raster_params.nlines_;
  const unsigned int out_cols = output_raster_params.ncols_;
  const unsigned int out_bands = output_raster_params.nBands();
  
  double in_col = 0;
  double in_line = 0;
  unsigned int out_col = 0;
  unsigned int out_line = 0;
  unsigned int out_band = 0;
  double value = 0;
  
  double dummy_value = 0;
  if( input_raster_params.useDummy_ ) {
    dummy_value = input_raster_params.dummy_[ 0 ];
  }
  
  TeRaster& output_raster_ref = *( output_raster.nakedPointer() );
  
  TePDIInterpolator interpolator;
  TEAGN_TRUE_OR_RETURN( interpolator.reset( input_raster, interpol,
    dummy_value), "Interpolator reset error" )
    
  TePDIPIManager progress( "Resampling raster", ( out_bands * out_lines ),
    enable_progress );
  
  for( out_band = 0 ; out_band < out_bands ; ++out_band ) {
    for( out_line = 0 ; out_line < out_lines ; ++out_line ) {
      in_line = ( (double)out_line) * y_resolution_ratio;
if ((int)in_line >= (input_raster->params().nlines_-1))
  in_line = (double)(input_raster->params().nlines_-1);
    
      for( out_col = 0 ; out_col < out_cols ; ++out_col ) {
        in_col = ( (double)out_col) * x_resolution_ratio;
  if ((int)in_col >= (input_raster->params().ncols_-1))
  in_col = (double)(input_raster->params().ncols_-1);
      
        interpolator.interpolate( in_line, in_col, out_band, value );

    if (value>255)
      value=255;
    else if (value<0)
      value=0;
    
        TEAGN_TRUE_OR_RETURN( output_raster_ref.setElement( out_col, 
          out_line, value, out_band ), "Output raster writing error" )
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by user" );
    }
  }
  
  return true;
}
