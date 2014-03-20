#include "TePDIWiSpeRFusion.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"
#include "TePDITypes.hpp"
#include "TePDIStatistic.hpp"
#include "TePDIWaveletAtrous.hpp"
#include "../kernel/TeRasterRemap.h"
#include "../kernel/TeMatrix.h"
#include "../kernel/TeUtils.h"

#include <math.h>
#include <queue>

TePDIWiSpeRFusion::TePDIWiSpeRFusion()
{
}

TePDIWiSpeRFusion::~TePDIWiSpeRFusion()
{
}

void TePDIWiSpeRFusion::ResetState(const TePDIParameters&)
{
}

bool TePDIWiSpeRFusion::CheckParameters(const TePDIParameters& parameters) const
{
/* Input rasters and bands checking */

	TePDITypes::TePDIRasterVectorType input_rasters;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");
	TEAGN_TRUE_OR_RETURN(input_rasters.size() > 1, "Invalid input rasters number");
	
	std::vector<int> bands;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("bands", bands), "Missing parameter: bands");
	TEAGN_TRUE_OR_RETURN(bands.size() == input_rasters.size(), "Invalid parameter: bands number");

	for(unsigned int input_rasters_index = 0; input_rasters_index < input_rasters.size(); input_rasters_index++)
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

	for(unsigned int output_rasters_index = 0; output_rasters_index < output_rasters.size(); output_rasters_index++)
	{
		TEAGN_TRUE_OR_RETURN(output_rasters[output_rasters_index].isActive(), "Invalid parameter: output_rasters " + Te2String(output_rasters_index) + " inactive");
		TEAGN_TRUE_OR_RETURN(output_rasters[output_rasters_index]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: output_rasters " + Te2String(output_rasters_index) + " not ready");
	}

	int rasters_levels;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("rasters_levels", rasters_levels), "Missing parameter: rasters_levels");

	int reference_levels;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_levels", reference_levels), "Missing parameter: reference_levels");

	string filter_file;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("filter_file", filter_file), "Missing parameter: filter_file");

	TePDIWiSpeRFusion::InterpMethod resampling_type;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("resampling_type", resampling_type), "Missing parameter: resampling_type");

	bool fit_histogram;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fit_histogram", fit_histogram), "Missing parameter: fit_histogram");

	string satelliteName;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("satelliteName", satelliteName), "Missing parameter: satelliteName");

	std::vector<int> bands_order;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("bands_order", bands_order), "Missing parameter: bands_order");
	TEAGN_TRUE_OR_RETURN(bands_order.size() == input_rasters.size(), "Invalid parameter: bands_order size");

	return true;
}

bool TePDIWiSpeRFusion::fractions(TeMatrix &p_multi_pan, string satellite, int &pan_index)
{
	FILE *srf_file = fopen(satellite.c_str(), "r");
  TEAGN_TRUE_OR_RETURN( srf_file, "Error opening sitelite SRF file" );

	int	srfs_points,
		bands_number;
	fscanf(srf_file,"%d %d %d", &srfs_points, &bands_number, &pan_index);
	pan_index--;

	double	wavelength;
	TeMatrix srfs;
	srfs.Init(bands_number, srfs_points);
	p_multi_pan.Init(bands_number, bands_number, 0.0);

	for (int i = 0; i < srfs_points; i++)
	{
		fscanf(srf_file,"%lf", &wavelength);
		for (int b = 0; b < bands_number; b++)
			fscanf(srf_file,"%lf", &srfs(b, i));
		for (int x = 0; x < bands_number; x++)
		{
			for (int y = x; y < bands_number; y++)
			{
				if ((x == pan_index) && (y == pan_index))
				{
					double max = 0.0;
					for (int z = 0; z < bands_number; z++)
					{
						if (z != pan_index)
							if (srfs(z, i) > max)
								max = srfs(z, i);
					}
					p_multi_pan(pan_index, pan_index) += MIN(srfs(pan_index, i), max);
				}
				else
				{
					p_multi_pan(x, y) += MIN(srfs(x, i), srfs(y, i));
					p_multi_pan(y, x) = p_multi_pan(x, y);
 				}
			}
		}
	}
	fclose(srf_file);

	return true;
}

bool TePDIWiSpeRFusion::RunImplementation()
{
/* Getting parameters */
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

	int rasters_levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("rasters_levels", rasters_levels), "Missing parameter: rasters_levels");

	int reference_levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_levels", reference_levels), "Missing parameter: reference_levels");

	string filter_file;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("filter_file", filter_file), "Missing parameter: filter_file");

	TePDIWiSpeRFusion::InterpMethod resampling_type;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("resampling_type", resampling_type), "Missing parameter: resampling_type");

	bool fit_histogram;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("fit_histogram", fit_histogram), "Missing parameter: fit_histogram");

	string satelliteName;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("satelliteName", satelliteName), "Missing parameter: satelliteName");

	std::vector<int> bands_order;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("bands_order", bands_order), "Missing parameter: bands_order");

/* Setting variables */
	unsigned int b;
	int	i,
		j,
		l;
	double	pixel;
	std::vector<double> channelMinLevel,
						channelMaxLevel;

	for(b = 0; b < input_rasters.size(); b++)
	{
		channelMinLevel.push_back(0.0);
		//channelMaxLevel.push_back(pow(2.0, (double)input_rasters_wavelets[b][0]->params().nbitsperPixel_[0])-1.0);
		channelMaxLevel.push_back(255.0);
	}

/* Allocating output rasters */
	TeRasterParams base_raster_params = reference_raster->params();
	for(b = 0; b < output_rasters.size(); b++)
	{
		TeRasterParams outRaster_params = output_rasters[b]->params();
		outRaster_params.nBands(1);
		if (base_raster_params.projection() != 0)
		{
			TeSharedPtr<TeProjection> proj(TeProjectionFactory::make(base_raster_params.projection()->params())); 
			outRaster_params.projection(proj.nakedPointer());
		}
		outRaster_params.boxResolution(base_raster_params.box().x1(), base_raster_params.box().y1(), base_raster_params.box().x2(), base_raster_params.box().y2(), base_raster_params.resx_, base_raster_params.resy_);
		outRaster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
		outRaster_params.setDummy(0.0, -1);
		outRaster_params.setDataType(TeUNSIGNEDCHAR, -1);
		TEAGN_TRUE_OR_RETURN(output_rasters[b]->init(outRaster_params), "Raster reset error - Raster index " + Te2String(b));
	}

/* Resampling the input raster */
	TeRasterParams reference_params = reference_raster->params();
  
	TePDITypes::TePDIRasterVectorType resampled_input_rasters;
	for(b = 0; b < input_rasters.size(); b++)
	{
		if (resampling_type != TePDIWiSpeRFusion::Nothing)
		{
		  TePDIInterpolator::InterpMethod intMethod;
		  switch( resampling_type )
		  {
		    case TePDIWiSpeRFusion::NNMethod :
		    {
		      intMethod =  TePDIInterpolator::NNMethod;
		      break;
		    }
		    case TePDIWiSpeRFusion::BilinearMethod :
		    {
		      intMethod =  TePDIInterpolator::BilinearMethod;
		      break;
		    }		  
		    case TePDIWiSpeRFusion::BicubicMethod :
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
			resampleRasterByResLinsCols(input_rasters[b], resampled_input_rasters_temp, (double)reference_params.resx_ / (double)input_rasters[b]->params().resx_, (double)reference_params.resy_ / (double)input_rasters[b]->params().resy_, reference_params.nlines_,  reference_params.ncols_, intMethod, progress_enabled_);
			resampled_input_rasters.push_back(resampled_input_rasters_temp);
		}
		else
			resampled_input_rasters.push_back(input_rasters[b]);
	}

// Creating wavelet planes
	std::vector<TePDITypes::TePDIRasterVectorType> input_rasters_wavelets;
	for(b = 0; b < resampled_input_rasters.size(); b++)
	{
		TePDITypes::TePDIRasterVectorType input_rasters_wavelets_vector_temp;
		input_rasters_wavelets_vector_temp.push_back(resampled_input_rasters[b]);
		for(l = 1; l <= rasters_levels; l++)
		{
			TePDITypes::TePDIRasterPtrType input_rasters_wavelets_temp;
			TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(input_rasters_wavelets_temp, waveletPlanes, reference_params.nlines_, reference_params.ncols_, false, TeFLOAT, 0), "Unable to alloc input rasters wavelet planes");
			input_rasters_wavelets_vector_temp.push_back(input_rasters_wavelets_temp);
		}
		input_rasters_wavelets.push_back(input_rasters_wavelets_vector_temp);
	}

// Extracting input rasters wavelets planes
	for(b = 0; b < resampled_input_rasters.size(); b++)
	{
		TePDIParameters waveletAtrousParams;
		waveletAtrousParams.SetParameter("direction", (int)TePDIWaveletAtrous::DECOMPOSE);
		waveletAtrousParams.SetParameter("input_raster", resampled_input_rasters[b]);
		waveletAtrousParams.SetParameter("band", bands[b]);
		waveletAtrousParams.SetParameter("levels", rasters_levels);
		waveletAtrousParams.SetParameter("filter_file", filter_file);
		waveletAtrousParams.SetParameter("output_wavelets", input_rasters_wavelets[b]);
		waveletAtrousParams.SetParameter("fit_histogram", false);
		TePDIWaveletAtrous wa;
		TEAGN_TRUE_OR_THROW(wa.Reset(waveletAtrousParams), "Invalid Parameters");
		wa.ToggleProgInt(progress_enabled_);
		TEAGN_TRUE_OR_THROW(wa.Apply(), "Apply error");
	}

// Creating reference raster wavelet planes
	TePDITypes::TePDIRasterVectorType reference_raster_wavelets;
	for(l = 0; l <= reference_levels; l++)
	{
		TePDITypes::TePDIRasterPtrType reference_raster_wavelets_temp;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(reference_raster_wavelets_temp, waveletPlanes, reference_params.nlines_, reference_params.ncols_, false, TeFLOAT, 0), "Reference raster wavelets Alloc error");
		if (l == 0)
		{
			for (int j = 0; j < reference_params.nlines_; j++)
			{
				for (int i = 0; i < reference_params.ncols_; i++)
				{
					reference_raster->getElement(i, j, pixel, reference_raster_band);
					reference_raster_wavelets_temp->setElement(i, j, pixel, 0);
				}
			}
		}
		reference_raster_wavelets.push_back(reference_raster_wavelets_temp);
	}

// Extracting reference raster wavelet planes
	TePDIParameters waveletAtrousParams;
	waveletAtrousParams.SetParameter("direction", (int)TePDIWaveletAtrous::DECOMPOSE);
	waveletAtrousParams.SetParameter("input_raster", reference_raster);
	waveletAtrousParams.SetParameter("band", reference_raster_band);
	waveletAtrousParams.SetParameter("levels", reference_levels);
	waveletAtrousParams.SetParameter("filter_file", filter_file);
	waveletAtrousParams.SetParameter("output_wavelets", reference_raster_wavelets);
	TePDIWaveletAtrous wa;
	TEAGN_TRUE_OR_THROW(wa.Reset(waveletAtrousParams), "Invalid Parameters");
	wa.ToggleProgInt(progress_enabled_);
	TEAGN_TRUE_OR_THROW(wa.Apply(), "Apply error");

// Creating weights planes
	TePDITypes::TePDIRasterVectorType input_rasters_weights;
	for(b = 0; b < resampled_input_rasters.size(); b++)
	{
		TePDITypes::TePDIRasterPtrType input_rasters_weights_temp;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(input_rasters_weights_temp, reference_levels+1, reference_params.nlines_, reference_params.ncols_, false, TeFLOAT, 0), "Input rasters weights alloc error");
		input_rasters_weights.push_back(input_rasters_weights_temp);
	}

// WiSpeR
	TeMatrix p_multi_pan;
	int pan_index;
	fractions(p_multi_pan, satelliteName, pan_index);

	for (b = 0; b < bands_order.size(); b++)
	{
		bands_order[b]--;
	}

	TePDIPIManager progress("Pondering coefficients", resampled_input_rasters[0]->params().ncols_, progress_enabled_);
	progress.Toggle(progress_enabled_);
	for (j = 0; j < reference_params.nlines_; j++)
	{
		for (i = 0; i < reference_params.ncols_; i++)
		{
			std::vector<double>	p_multi(resampled_input_rasters.size(), 0.0),
						n_p_i(resampled_input_rasters.size(), 0.0),
						pho_p_i(resampled_input_rasters.size(), 0.0);
			double	p_pan,
				n_p_i_sum = 0.0,
				pho_p_mean = 0.0;
				//alpha = p_multi_pan(pan_index, pan_index) / 82.808534682;
			//reference_raster->getElement(i, j, p_pan, 0);
			reference_raster_wavelets[0]->getElement(i, j, p_pan, 0);
			for(b = 0; b < resampled_input_rasters.size(); b++)
			{
				resampled_input_rasters[b]->getElement(i, j, p_multi[b], 0);
				n_p_i[b] = (p_multi_pan(bands_order[b], pan_index) / p_multi_pan(bands_order[b], bands_order[b])) * p_multi[b];
				//n_p_i[b] = (p_multi_pan(bands_order[b], pan_index) / p_multi_pan(pan_index, pan_index)) * alpha * p_pan;
				n_p_i_sum += n_p_i[b];
				pho_p_i[b] = n_p_i[b] / p_multi_pan(bands_order[b], pan_index);
				pho_p_mean += pho_p_i[b];
			}
			pho_p_mean /= (double)resampled_input_rasters.size();
			for(b = 0; b < resampled_input_rasters.size(); b++)
			{
				double	ponder = 0.0,
						beta = p_multi_pan(bands_order[b], (bands_order[b]+1 < (int)resampled_input_rasters.size())?bands_order[b]+1:bands_order[b]-1) / p_multi_pan(bands_order[b], bands_order[b]);
				//reference_raster_wavelets[b][reference_levels]->getElement(i, j, p_pan_res, 0);
				for (int l = 1; l <= reference_levels; l++)
				{
					double	p_pan_wavelet,
						p_pan_res;
					reference_raster_wavelets[l]->getElement(i, j, p_pan_res, 0);
					reference_raster_wavelets[l]->getElement(i, j, p_pan_wavelet, 1);
					//ponder += (pho_p_i[b] / pho_p_mean) * (((n_p_i_sum / p_pan_res) * (p_multi_pan(bands_order[b], pan_index) / p_multi_pan(pan_index, pan_index))) / (p_multi_pan(b, pan_index) / p_multi_pan(bands_order[b], bands_order[b]))) * (1.0 - (beta/2.0)) * p_pan_wavelet;
					ponder += (pho_p_i[b] / pho_p_mean) * (((n_p_i_sum / p_pan_res) * (p_multi_pan(bands_order[b], pan_index) / p_multi_pan(pan_index, pan_index))) / ((p_multi_pan(bands_order[b], pan_index) / p_multi_pan(bands_order[b], bands_order[b])) * (1.0 - (beta/2.0)))) * p_pan_wavelet;
				}
				input_rasters_weights[b]->setElement(i, j, ponder, 0);
			}
		}
		progress.Increment();
	}
	progress.Toggle(false);


// Fusing input rasters and reference raster
	progress.Reset("Fusing input raster and reference raster", resampled_input_rasters.size()*reference_params.nlines_);
	progress.Toggle(progress_enabled_);
	double	p_pon = 0.0;
	for(b = 0; b < resampled_input_rasters.size(); b++)
	{
		for (j = 0; j < reference_params.nlines_; j++)
		{
			for (i = 0; i < reference_params.ncols_; i++)
			{
				input_rasters_wavelets[b][rasters_levels]->getElement(i, j, pixel, 0);
				input_rasters_weights[b]->getElement(i, j, p_pon, 0);
				pixel += p_pon;
				pixel = (pixel > channelMaxLevel[b]?channelMaxLevel[b]:(pixel < channelMinLevel[b]?channelMinLevel[b]:pixel));
				output_rasters[b]->setElement(i, j, pixel, 0);
			}
			progress.Increment();
		}
	}
	progress.Toggle(false);

	return true;
}

bool TePDIWiSpeRFusion::resampleRasterByResLinsCols( 
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
