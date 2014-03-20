#include "TePDIIHSWaveletFusion.hpp"
#include "TePDIUtils.hpp"
#include "TePDITypes.hpp"
#include "TePDIWaveletAtrous.hpp"
#include "TePDIColorTransform.hpp"
#include "TePDIStatistic.hpp"

#include "../kernel/TeRasterRemap.h"
#include "../kernel/TeAgnostic.h"
#include "../kernel/TeMatrix.h"
#include "../kernel/TeUtils.h"

#include <math.h>

#define waveletPlanes 2

TePDIIHSWaveletFusion::TePDIIHSWaveletFusion()
{
}

TePDIIHSWaveletFusion::~TePDIIHSWaveletFusion()
{
}

void TePDIIHSWaveletFusion::ResetState(const TePDIParameters&)
{
}

void TePDIIHSWaveletFusion::rgb2ihs(double *rgb, double *ihs)
{
	double theta = acos((0.5 * ((rgb[0] - rgb[1]) + (rgb[0] - rgb[2]))) / sqrt((rgb[0] - rgb[1]) * (rgb[0] - rgb[1]) + (rgb[0] - rgb[2]) * (rgb[1] - rgb[2])));
	if (rgb[2] > rgb[1])
		ihs[1] = (((2 * PI) - theta) * 180) / PI;
	else
		ihs[1] = (theta * 180) / PI;
	ihs[2] = 1 - (3 / (rgb[0] + rgb[1] + rgb[2])) * MIN(rgb[0], MIN(rgb[1], rgb[2]));
	ihs[0] = (rgb[0] + rgb[1] + rgb[2]) / 3;
}

void TePDIIHSWaveletFusion::ihs2rgb(double *ihs, double *rgb)
{
	if ((ihs[1] >= 0.0) && (ihs[1] < 120.0))
	{
		ihs[1] = (ihs[1] * PI) / 180;
		rgb[2] = ihs[0] * (1 - ihs[2]);
		rgb[0] = ihs[0] * (1 + ((ihs[2] * cos(ihs[1])) / (cos((PI/3) - ihs[1]))));
		rgb[1] = (3 * ihs[0]) - (rgb[0] + rgb[2]);
	}
	else
	if ((ihs[1] >= 120.0) && (ihs[1] < 240.0))
	{
		ihs[1] = ((ihs[1] - 120) * PI) / 180;
		rgb[0] = ihs[0] * (1 - ihs[2]);
		rgb[1] = ihs[0] * (1 + ((ihs[2] * cos(ihs[1])) / (cos((PI/3) - ihs[1]))));
		rgb[2] = (3 * ihs[0]) - (rgb[0] + rgb[1]);
	}
	else
	if ((ihs[1] >= 240.0) && (ihs[1] <= 360.0))
	{
		ihs[1] = ((ihs[1] - 240) * PI) / 180;
		rgb[1] = ihs[0] * (1 - ihs[2]);
		rgb[2] = ihs[0] * (1 + ((ihs[2] * cos(ihs[1])) / (cos((PI/3) - ihs[1]))));
		rgb[0] = (3 * ihs[0]) - (rgb[1] + rgb[2]);
	}
}

bool TePDIIHSWaveletFusion::CheckParameters(const TePDIParameters& parameters) const
{
/* Input rasters and bands checking */

	TePDITypes::TePDIRasterVectorType input_rasters;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");
	TEAGN_TRUE_OR_RETURN(input_rasters.size() == 3, "Invalid input rasters number");
	
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

	int i_levels;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("i_levels", i_levels), "Missing parameter: i_levels");

	int reference_levels;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_levels", reference_levels), "Missing parameter: reference_levels");

	string filter_file;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("filter_file", filter_file), "Missing parameter: filter_file");

	TePDIIHSWaveletFusion::InterpMethod resampling_type;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("resampling_type", resampling_type), "Missing parameter: resampling_type");

	bool fit_histogram;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fit_histogram", fit_histogram), "Missing parameter: fit_histogram");

	return true;
}

bool TePDIIHSWaveletFusion::RunImplementation()
{
// Getting parameters
	TePDITypes::TePDIRasterVectorType input_rasters;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");

	std::vector<int> bands;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("bands", bands), "Missing parameter: bands");

	TePDITypes::TePDIRasterVectorType output_rasters;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("output_rasters", output_rasters), "Missing parameter: output_rasters");

	TePDITypes::TePDIRasterPtrType reference_raster;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_raster", reference_raster), "Missing parameter: reference_raster");

	int reference_raster_band;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_raster_band", reference_raster_band), "Missing parameter: reference_raster_band");

	int i_levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("i_levels", i_levels), "Missing parameter: i_levels");

	int reference_levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_levels", reference_levels), "Missing parameter: reference_levels");

	string filter_file;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("filter_file", filter_file), "Missing parameter: filter_file");

	TePDIIHSWaveletFusion::InterpMethod resampling_type;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("resampling_type", resampling_type), "Missing parameter: resampling_type");

	bool fit_histogram;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("fit_histogram", fit_histogram), "Missing parameter: fit_histogram");

// Setting variables
	unsigned int b;
	int i,
		j,
		l;
	double	rgb[NUN_BANDS_IHS],
			ihs[NUN_BANDS_IHS],
			channelMinLevel[NUN_BANDS_IHS],
			channelMaxLevel[NUN_BANDS_IHS];

	for(b = 0; b < input_rasters.size(); b++)
	{
		channelMinLevel[b] = 0.0;
		channelMaxLevel[b] = 255.0;
		//channelMaxLevel[b] = pow(2.0, (double)input_rasters[b]->params().nbitsperPixel_[bands[b]])-1.0;
	}

/* Allocating output rasters */
	TeRasterParams reference_params = reference_raster->params();
	for(b = 0; b < output_rasters.size(); b++)
	{
		TeRasterParams outRaster_params = output_rasters[b]->params();
		outRaster_params.nBands(1);
		if (reference_params.projection() != 0)
		{
			TeSharedPtr<TeProjection> proj(TeProjectionFactory::make(reference_params.projection()->params()));
			outRaster_params.projection(proj.nakedPointer());
		}
		outRaster_params.boxResolution(reference_params.box().x1(), reference_params.box().y1(), reference_params.box().x2(), reference_params.box().y2(), reference_params.resx_, reference_params.resy_);
		outRaster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
		outRaster_params.setDummy(0.0, -1);
		outRaster_params.setDataType(TeUNSIGNEDCHAR, -1);
		TEAGN_TRUE_OR_RETURN(output_rasters[b]->init(outRaster_params), "Raster reset error - Raster index " + Te2String(b));
	}

// Resampling the input raster
	TePDITypes::TePDIRasterVectorType resampled_input_rasters;
	for(b = 0; b < input_rasters.size(); b++)
	{
		if (resampling_type != TePDIIHSWaveletFusion::Nothing)
		{
		  TePDIInterpolator::InterpMethod intMethod;
		  switch( resampling_type )
		  {
		    case TePDIIHSWaveletFusion::NNMethod :
		    {
		      intMethod =  TePDIInterpolator::NNMethod;
		      break;
		    }
		    case TePDIIHSWaveletFusion::BilinearMethod :
		    {
		      intMethod =  TePDIInterpolator::BilinearMethod;
		      break;
		    }		  
		    case TePDIIHSWaveletFusion::BicubicMethod :
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

// IHS Direct transformation
	TePDITypes::TePDIRasterVectorType ihs_rasters;
	for(b = 0; b < resampled_input_rasters.size(); b++)
	{
		TePDITypes::TePDIRasterPtrType temp_raster;
		TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(temp_raster,  reference_params, TePDIUtils::TePDIUtilsAutoMemPol), "Unable create the resampled input rasters");
		ihs_rasters.push_back(temp_raster);
	}

	TePDIPIManager progress("RGB2IHS transformation", reference_params.nlines_, progress_enabled_);
	progress.Toggle(progress_enabled_);
	for (j = 0; j < reference_params.nlines_; j++)
	{
		for (i = 0; i < reference_params.ncols_; i++)
		{
			for(b = 0; b < resampled_input_rasters.size(); b++)
			{
				resampled_input_rasters[b]->getElement(i, j, rgb[b], bands[b]);
				rgb[b] = (rgb[b]-channelMinLevel[b])/(channelMaxLevel[b]-channelMinLevel[b]);
			}
			rgb2ihs(rgb, ihs);
			for(b = 0; b < resampled_input_rasters.size(); b++)
			{
				if (b!=1)
					ihs[b] = (ihs[b]*(channelMaxLevel[b]-channelMinLevel[b]))+channelMinLevel[b];
				else
					ihs[b] = (ihs[b]/360.0)*(channelMaxLevel[b]-channelMinLevel[b]);
				ihs_rasters[b]->setElement(i, j, ihs[b], 0);
			}
		}
		progress.Increment();
	}
	progress.Toggle(false);

// Creating I wavelet plane
	std::vector<TePDITypes::TePDIRasterVectorType> ihs_rasters_wavelets;
	TePDITypes::TePDIRasterVectorType ihs_rasters_wavelets_vector_temp;
	ihs_rasters_wavelets_vector_temp.push_back(ihs_rasters[0]);
	for(l = 1; l <= i_levels; l++)
	{
		TePDITypes::TePDIRasterPtrType input_rasters_wavelets_temp;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(input_rasters_wavelets_temp, waveletPlanes, reference_params.nlines_, reference_params.ncols_, false, TeFLOAT, 0), "Unable to alloc input rasters wavelet planes");
		ihs_rasters_wavelets_vector_temp.push_back(input_rasters_wavelets_temp);
	}
	ihs_rasters_wavelets.push_back(ihs_rasters_wavelets_vector_temp);

// Extracting I wavelet planes
	TePDIParameters iWaveletAtrousParams;
	iWaveletAtrousParams.SetParameter("direction", (int)TePDIWaveletAtrous::DECOMPOSE);
	iWaveletAtrousParams.SetParameter("input_raster", ihs_rasters[0]);
	iWaveletAtrousParams.SetParameter("band", 0);
	iWaveletAtrousParams.SetParameter("levels", i_levels);
	iWaveletAtrousParams.SetParameter("filter_file", filter_file);
	iWaveletAtrousParams.SetParameter("fit_histogram", false);
	iWaveletAtrousParams.SetParameter("output_wavelets", ihs_rasters_wavelets[0]);
	TePDIWaveletAtrous waI;
	TEAGN_TRUE_OR_THROW(waI.Reset(iWaveletAtrousParams), "Invalid Parameters");
	waI.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(waI.Apply(), "Apply error");

// Creating reference raster wavelet planes
	double p;
	TePDITypes::TePDIRasterVectorType reference_raster_wavelets;
	for(l = 0; l <= reference_levels; l++)
	{
		TePDITypes::TePDIRasterPtrType reference_raster_wavelets_temp;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(reference_raster_wavelets_temp, waveletPlanes, reference_params.nlines_, reference_params.ncols_, false, TeFLOAT, 0), "Reference raster wavelets Alloc error");
		if (l == 0)
		{
			for (j = 0; j < reference_params.nlines_; j++)
			{
				for (i = 0; i < reference_params.ncols_; i++)
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
	referenceWaveletAtrousParams.SetParameter("fit_histogram", fit_histogram);
	referenceWaveletAtrousParams.SetParameter("output_wavelets", reference_raster_wavelets);
	TePDIWaveletAtrous waReference;
	TEAGN_TRUE_OR_THROW(waReference.Reset(referenceWaveletAtrousParams), "Invalid Parameters");
	waReference.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(waReference.Apply(), "Apply error");

// Recomposing wavelets
	TePDIParameters waveletAtrousParamsRecompose;
	waveletAtrousParamsRecompose.SetParameter("direction", (int)TePDIWaveletAtrous::RECOMPOSE);
	waveletAtrousParamsRecompose.SetParameter("input_rasters_wavelets", ihs_rasters_wavelets);
	waveletAtrousParamsRecompose.SetParameter("rasters_levels", i_levels);
	waveletAtrousParamsRecompose.SetParameter("reference_raster_wavelets", reference_raster_wavelets);
	waveletAtrousParamsRecompose.SetParameter("reference_levels", reference_levels);
	waveletAtrousParamsRecompose.SetParameter("output_rasters", ihs_rasters);
	TePDIWaveletAtrous waRecompose;
	TEAGN_TRUE_OR_THROW(waRecompose.Reset(waveletAtrousParamsRecompose), "Invalid Parameters");
	waRecompose.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(waRecompose.Apply(), "Apply error");

// Inverse IHS transformation
	progress.Reset("IHS2RGB transformation", reference_params.nlines_);
	progress.Toggle(progress_enabled_);
	for (j = 0; j < reference_params.nlines_; j++)
	{
		for (i = 0; i < reference_params.ncols_; i++)
		{
			for(b = 0; b < resampled_input_rasters.size(); b++)
			{
				ihs_rasters[b]->getElement(i, j, ihs[b], bands[b]);
				if (b!=1)
					ihs[b] = (ihs[b]-channelMinLevel[b])/(channelMaxLevel[b]-channelMinLevel[b]);
				else
					ihs[b] = (ihs[b]/(channelMaxLevel[b]-channelMinLevel[b]))*360.0;
			}
			ihs2rgb(ihs, rgb);
			for(b = 0; b < resampled_input_rasters.size(); b++)
			{
				rgb[b] = (rgb[b]*(channelMaxLevel[b]-channelMinLevel[b]))+channelMinLevel[b];
				rgb[b] = (rgb[b]>channelMaxLevel[b]?channelMaxLevel[b]:(rgb[b]<channelMinLevel[b]?channelMinLevel[b]:rgb[b]));
				output_rasters[b]->setElement(i, j, rgb[b], 0);
			}
		}
		progress.Increment();
	}
	progress.Toggle(false);

	return true;
}

bool TePDIIHSWaveletFusion::resampleRasterByResLinsCols( 
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
