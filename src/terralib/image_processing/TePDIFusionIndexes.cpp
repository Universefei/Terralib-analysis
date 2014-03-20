#include "TePDIFusionIndexes.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"
#include "TePDITypes.hpp"
#include "TePDIStatistic.hpp"
#include "../kernel/TeRasterRemap.h"
#include "../kernel/TeMatrix.h"
#include "../kernel/TeUtils.h"
#include <math.h>
#include <queue>

#define waveletPlanes 2

TePDIFusionIndexes::TePDIFusionIndexes()
{
}

TePDIFusionIndexes::~TePDIFusionIndexes()
{
}

void TePDIFusionIndexes::ResetState(const TePDIParameters&)
{
}

bool TePDIFusionIndexes::CheckParameters(const TePDIParameters& parameters) const
{
	int index_type;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("index_type", index_type), "Missing parameter: index_type");

	if ((index_type == TePDIFusionIndexes::CC) || (index_type == TePDIFusionIndexes::VARDIF) || (index_type == TePDIFusionIndexes::SDD) || (index_type == TePDIFusionIndexes::BIAS) || (index_type == TePDIFusionIndexes::UIQI) || (index_type == TePDIFusionIndexes::RMSE) || (index_type == TePDIFusionIndexes::SIMM))
	{
		TePDITypes::TePDIRasterPtrType original_raster;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("original_raster", original_raster), "Missing parameter: original_raster");
		TEAGN_TRUE_OR_RETURN(original_raster.isActive(), "Invalid parameter: original_raster inactive");
		TEAGN_TRUE_OR_RETURN(original_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: original_raster not ready");
	
		int original_band;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("original_band", original_band), "Missing parameter: original_band");
		TEAGN_TRUE_OR_THROW(original_raster->params().nBands() > original_band, "Original band out of the bands bounds");
	
		TePDITypes::TePDIRasterPtrType fused_raster;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fused_raster", fused_raster), "Missing parameter: fused_raster");
		TEAGN_TRUE_OR_RETURN(fused_raster.isActive(), "Invalid parameter: fused_raster inactive");
		TEAGN_TRUE_OR_RETURN(fused_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: fused_raster not ready");
	
		int fused_band;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fused_band", fused_band), "Missing parameter: fused_band");
		TEAGN_TRUE_OR_THROW(fused_raster->params().nBands() > fused_band, "Fused band out of the bands bounds");
	
		TeSharedPtr<double> index_value;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("index_value", index_value), "Missing parameter: index_value");

		TEAGN_TRUE_OR_THROW((original_raster->params().nlines_ == fused_raster->params().nlines_) && (original_raster->params().ncols_ == fused_raster->params().ncols_), "Original raster lines and/or columns differs from Fused raster");
	}
	else if ((index_type == TePDIFusionIndexes::ERGAS) || (index_type == TePDIFusionIndexes::SERGAS))
	{
		TePDITypes::TePDIRasterVectorType original_rasters;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("original_rasters", original_rasters), "Missing parameter: original_rasters");

		std::vector<int> original_bands;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("original_bands", original_bands), "Missing parameter: original_bands");
		TEAGN_TRUE_OR_THROW(original_bands.size() == original_rasters.size(), "Original raster and Original bands difference in size");

		for (unsigned b = 0; b < original_rasters.size(); b++)
		{
			TEAGN_TRUE_OR_RETURN(original_rasters[b].isActive(), "Invalid parameter: original_rasters inactive");
			TEAGN_TRUE_OR_RETURN(original_rasters[b]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: original_rasters not ready");
			TEAGN_TRUE_OR_THROW((original_rasters[0]->params().nlines_ == original_rasters[b]->params().nlines_) && (original_rasters[0]->params().ncols_ == original_rasters[b]->params().ncols_), "Original raster lines and/or columns differs");
			TEAGN_TRUE_OR_THROW(original_rasters[b]->params().nBands() > original_bands[b], "Original bands out of the bands bounds");
		}

		if (index_type == TePDIFusionIndexes::SERGAS)
		{
			TePDITypes::TePDIRasterPtrType pan_raster;
			TEAGN_TRUE_OR_RETURN(parameters.GetParameter("pan_raster", pan_raster), "Missing parameter: pan_raster");
			TEAGN_TRUE_OR_RETURN(pan_raster.isActive(), "Invalid parameter: pan_raster inactive");
			TEAGN_TRUE_OR_RETURN(pan_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: pan_raster not ready");

			int pan_band;
			TEAGN_TRUE_OR_RETURN(parameters.GetParameter("pan_band", pan_band), "Missing parameter: pan_band");
			TEAGN_TRUE_OR_THROW(pan_raster->params().nBands() > pan_band, "Pan band out of the band bounds");
		}

		TePDITypes::TePDIRasterVectorType fused_rasters;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fused_rasters", fused_rasters), "Missing parameter: fused_rasters");
		TEAGN_TRUE_OR_THROW(fused_rasters.size() == original_rasters.size(), "Fused and Original rasters numbers are different");

		std::vector<int> fused_bands;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fused_bands", fused_bands), "Missing parameter: fused_bands");
		TEAGN_TRUE_OR_THROW(fused_bands.size() == fused_rasters.size(), "Fused raster and Fused bands difference in size");

		for (unsigned b = 0; b < fused_rasters.size(); b++)
		{
			TEAGN_TRUE_OR_RETURN(fused_rasters[b].isActive(), "Invalid parameter: fused_rasters inactive");
			TEAGN_TRUE_OR_RETURN(fused_rasters[b]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: fused_rasters not ready");
			TEAGN_TRUE_OR_THROW(fused_rasters[b]->params().nBands() > fused_bands[b], "Fused bands out of the bands bounds");
			TEAGN_TRUE_OR_THROW((fused_rasters[0]->params().nlines_ == fused_rasters[b]->params().nlines_) && (fused_rasters[0]->params().ncols_ == fused_rasters[b]->params().ncols_), "Fused raster lines and/or columns differs");
			TEAGN_TRUE_OR_THROW((original_rasters[b]->params().nlines_ == fused_rasters[b]->params().nlines_) && (original_rasters[b]->params().ncols_ == fused_rasters[b]->params().ncols_), "Original raster lines and/or columns differs from Fused raster");
		}

		double low_resolution;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("low_resolution", low_resolution), "Missing parameter: low_resolution");

		double high_resolution;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("high_resolution", high_resolution), "Missing parameter: high_resolution");

		TeSharedPtr<double> index_value;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("index_value", index_value), "Missing parameter: index_value");
	}
	else if (index_type == TePDIFusionIndexes::SCC)
	{
		TePDITypes::TePDIRasterPtrType fused_raster;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fused_raster", fused_raster), "Missing parameter: fused_raster");
		TEAGN_TRUE_OR_RETURN(fused_raster.isActive(), "Invalid parameter: fused_raster inactive");
		TEAGN_TRUE_OR_RETURN(fused_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: fused_raster not ready");
	
		int fused_band;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("fused_band", fused_band), "Missing parameter: fused_band");
		TEAGN_TRUE_OR_THROW(fused_raster->params().nBands() > fused_band, "Fused band out of the bands bounds");

		TePDITypes::TePDIRasterPtrType pan_raster;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("pan_raster", pan_raster), "Missing parameter: pan_raster");
		TEAGN_TRUE_OR_RETURN(pan_raster.isActive(), "Invalid parameter: pan_raster inactive");
		TEAGN_TRUE_OR_RETURN(pan_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: pan_raster not ready");
	
		int pan_band;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("pan_band", pan_band), "Missing parameter: pan_band");
		TEAGN_TRUE_OR_THROW(pan_raster->params().nBands() > pan_band, "Pan band out of the bands bounds");
	
		TeSharedPtr<double> index_value;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("index_value", index_value), "Missing parameter: index_value");

		TEAGN_TRUE_OR_THROW((fused_raster->params().nlines_ == pan_raster->params().nlines_) && (fused_raster->params().ncols_ == pan_raster->params().ncols_), "Fused raster lines and/or columns differs from Pan raster");
	}
	else
	{
		TEAGN_TRUE_OR_RETURN(false, "Invalid parameter: index type");
	}


	return true;
}

bool TePDIFusionIndexes::raster_mean(TePDITypes::TePDIRasterPtrType raster, int band, double& mean)
{
	double	pixel;
	mean = 0.0;

	TePDIPIManager progress("Computing raster mean", raster->params().nlines_, progress_enabled_);
	for (int j = 0; j < raster->params().nlines_; j++)
	{
		for (int i = 0; i < raster->params().ncols_; i++)
		{
			raster->getElement(i, j, pixel, band);
			mean += pixel;
		}
		progress.Increment();
	}
	progress.Toggle(progress_enabled_);

	mean = mean / ((double)raster->params().nlines_ * (double)raster->params().ncols_);

	return true;
}

bool TePDIFusionIndexes::rasters_difference(TePDITypes::TePDIRasterPtrType raster1, int band1, TePDITypes::TePDIRasterPtrType raster2, int band2, TePDITypes::TePDIRasterPtrType raster_diff, int band_diff)
{
	double	pixel1,
		pixel2,
		diff;

	TePDIPIManager progress("Computing rasters difference", raster1->params().nlines_, progress_enabled_);
	for (int j = 0; j < raster1->params().nlines_; j++)
	{
		for (int i = 0; i < raster1->params().ncols_; i++)
		{
			raster1->getElement(i, j, pixel1, band1);
			raster2->getElement(i, j, pixel2, band2);
			diff = pixel1-pixel2;
			raster_diff->setElement(i, j, (diff < 0?-diff:diff), band_diff);
		}
		progress.Increment();
	}
	progress.Toggle(progress_enabled_);

	return true;
}

// CORRELATION COEFFICIENT INDEX -> NUNEZ:1999 with a modifications in the correlation coefficient equation
bool TePDIFusionIndexes::correlation_coefficient_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> cc)
{
	/*double	original_mean,
		fused_mean,
		original_pixel,
		fused_pixel,
		sigma_original_fused = 0.0,
		sigma_original = 0.0,
		sigma_fused = 0.0,
		diff_original,
		diff_fused;

	TEAGN_TRUE_OR_THROW(raster_mean(original_raster, original_band, original_mean), "Unable to compute Original raster mean");
	TEAGN_TRUE_OR_THROW(raster_mean(fused_raster, fused_band, fused_mean), "Unable to compute Fused raster mean");	

	TePDIPIManager progress("Computing correlation coefficient index", original_raster->params().nlines_, progress_enabled_);
	for (int j = 0; j < original_raster->params().nlines_; j++)
	{
		for (int i = 0; i < original_raster->params().ncols_; i++)
		{
			original_raster->getElement(i, j, original_pixel, original_band);
			fused_raster->getElement(i, j, fused_pixel, fused_band);
			diff_original = original_pixel - original_mean;
			diff_fused = fused_pixel - fused_mean;
			sigma_original_fused += diff_original * diff_fused;
			sigma_original += diff_original * diff_original;
			sigma_fused += diff_fused * diff_fused;
		}
		progress.Increment();
	}
	progress.Toggle(false);

	*cc = sigma_original_fused / sqrt(sigma_original * sigma_fused);*/

	TePDIStatistic stat;
	
	TePDIParameters stat_pars;
	TePDITypes::TePDIRasterVectorType stat_rasters;
	stat_rasters.push_back(original_raster);
	stat_rasters.push_back(fused_raster);
	stat_pars.SetParameter("rasters", stat_rasters);
	std::vector<int> stat_bands;
	stat_bands.push_back(original_band);
	stat_bands.push_back(fused_band);
	stat_pars.SetParameter("bands", stat_bands);

	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(progress_enabled_);
	
	*cc = stat.getCorrelation(0, 1);

	return true;
}

// BIAS INDEX -> OTAZU:2005
bool TePDIFusionIndexes::bias_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> bias)
{
	double	original_mean,
			fused_mean;
	/*TEAGN_TRUE_OR_THROW(raster_mean(original_raster, original_band, original_mean), "Unable to compute Original raster mean");
	TEAGN_TRUE_OR_THROW(raster_mean(fused_raster, fused_band, fused_mean), "Unable to compute Fused raster mean");

	*bias = (((original_mean - fused_mean) >= 0)?(original_mean - fused_mean):(fused_mean - original_mean));*/

	TePDIStatistic stat;
	
	TePDIParameters stat_pars;
	TePDITypes::TePDIRasterVectorType stat_rasters;
	stat_rasters.push_back(original_raster);
	stat_rasters.push_back(fused_raster);
	stat_pars.SetParameter("rasters", stat_rasters);
	std::vector<int> stat_bands;
	stat_bands.push_back(original_band);
	stat_bands.push_back(fused_band);
	stat_pars.SetParameter("bands", stat_bands);

	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(progress_enabled_);
	
	original_mean = stat.getMean(0);
	fused_mean = stat.getMean(1);

	*bias = original_mean - fused_mean;

	return true;
}
// THESIS MIGUEL TELLES 2008
bool TePDIFusionIndexes::variance_difference_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> vardif)
{
	double	original_variance,
			fused_variance;

	TePDIStatistic stat;
	
	TePDIParameters stat_pars;
	TePDITypes::TePDIRasterVectorType stat_rasters;
	stat_rasters.push_back(original_raster);
	stat_rasters.push_back(fused_raster);
	stat_pars.SetParameter("rasters", stat_rasters);
	std::vector<int> stat_bands;
	stat_bands.push_back(original_band);
	stat_bands.push_back(fused_band);
	stat_pars.SetParameter("bands", stat_bands);

	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(progress_enabled_);

	original_variance = stat.getVariance(0);	
	fused_variance = stat.getVariance(1);	

	*vardif = (original_variance - fused_variance) / original_variance;

	return true;
}

// STANDARD DEVIATION DIFFERENCE INDEX -> GONZALEZ-AUDICANA:2004
bool TePDIFusionIndexes::standard_deviation_diff_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> sdd)
{
	TeRasterParams diff_raster_params = original_raster->params();
	TePDITypes::TePDIRasterPtrType diff_raster;
	TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(diff_raster,  diff_raster_params, TePDIUtils::TePDIUtilsAutoMemPol), "Unable create the resampled input rasters");

	int diff_raster_band = 0;
	/*double	diff_raster_mean,
		diff_raster_pixel;
	*sdd = 0.0;*/

	TEAGN_TRUE_OR_THROW(rasters_difference(original_raster, original_band, fused_raster, fused_band, diff_raster, diff_raster_band), "Unable to compute rasters difference");
	/*TEAGN_TRUE_OR_THROW(raster_mean(diff_raster, diff_raster_band, diff_raster_mean), "Unable to compute Diff raster mean");

	TePDIPIManager progress("Computing difference standard deviation index", diff_raster->params().nlines_, progress_enabled_);
	for (int j = 0; j < diff_raster->params().nlines_; j++)
	{
		for (int i = 0; i < diff_raster->params().ncols_; i++)
		{
			diff_raster->getElement(i, j, diff_raster_pixel, diff_raster_band);
 			*sdd += (diff_raster_pixel - diff_raster_mean) * (diff_raster_pixel - diff_raster_mean);
		}
		progress.Increment();
	}
	progress.Toggle(false);

	*sdd = sqrt(*sdd / (double)(diff_raster->params().nlines_ * diff_raster->params().ncols_));*/

	TePDIStatistic stat;
	
	TePDIParameters stat_pars;
	TePDITypes::TePDIRasterVectorType stat_rasters;
	stat_rasters.push_back(diff_raster);
	stat_pars.SetParameter("rasters", stat_rasters);
	std::vector<int> stat_bands;
	stat_bands.push_back(diff_raster_band);
	stat_pars.SetParameter("bands", stat_bands);

	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(progress_enabled_);
	
	*sdd = stat.getStdDev(0);

	return true;
}

bool TePDIFusionIndexes::uiqi_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> uiqi)
{
	TePDIStatistic stat;
	
	TePDIParameters stat_pars;
	TePDITypes::TePDIRasterVectorType stat_rasters;
	stat_rasters.push_back(original_raster);
	stat_rasters.push_back(fused_raster);
	stat_pars.SetParameter("rasters", stat_rasters);
	std::vector<int> stat_bands;
	stat_bands.push_back(original_band);
	stat_bands.push_back(fused_band);
	stat_pars.SetParameter("bands", stat_bands);

	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(progress_enabled_);
	
	TeMatrix	mean_matrix = stat.getMeanMatrix(),
				stddev_matrix = stat.getStdDevMatrix();
	double	covariance = stat.getCovariance(0, 1);
	
	*uiqi = (covariance / (stddev_matrix(0, 0) * stddev_matrix(1, 0))) * ((2 * mean_matrix(0, 0) * mean_matrix(0, 1)) / ((mean_matrix(0, 0) * mean_matrix(0, 0)) + (mean_matrix(0, 1) * mean_matrix(0, 1)))) * ((2 * stddev_matrix(0, 0) * stddev_matrix(1, 0)) / ((stddev_matrix(0, 0) * stddev_matrix(0, 0)) + (stddev_matrix(1, 0) * stddev_matrix(1, 0))));

	return true;
}

// RMSE INDEX -> OTAZU:2005
bool TePDIFusionIndexes::rmse_index(TePDITypes::TePDIRasterPtrType original, int original_band, TePDITypes::TePDIRasterPtrType fused, int fused_band, TeSharedPtr<double> rmse)
{
	TeSharedPtr<double>	bias(new double),
						sdd(new double);
	TEAGN_TRUE_OR_THROW(bias_index(original, original_band, fused, fused_band, bias), "Unable to compute the bias index");
 	TEAGN_TRUE_OR_THROW(standard_deviation_diff_index(original, original_band, fused, fused_band, sdd), "Unable to compute the standart deviation index");

	*rmse = sqrt((*bias * *bias) + (*sdd * *sdd));

	return true;
}

// ERGAS INDEX -> OTAZU:2005
bool TePDIFusionIndexes::ergas_index(TePDITypes::TePDIRasterVectorType original_rasters, std::vector<int> original_bands, TePDITypes::TePDIRasterVectorType fused_rasters, std::vector<int> fused_bands, double low_resolution, double high_resolution, TeSharedPtr<double> ergas)
{
	TeSharedPtr<double> rmse(new double);
	double	original_mean,
			sum = 0.0;

	TePDIStatistic stat;
	
	TePDIParameters stat_pars;
	TePDITypes::TePDIRasterVectorType stat_rasters;
	std::vector<int> stat_bands;
	for (unsigned i = 0; i < original_rasters.size(); i++)
	{
		stat_rasters.push_back(original_rasters[i]);
		stat_bands.push_back(original_bands[i]);
	}
	stat_pars.SetParameter("rasters", stat_rasters);
	stat_pars.SetParameter("bands", stat_bands);

	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(progress_enabled_);

	TePDIPIManager progress("Computing ERGAS index", original_rasters.size(), progress_enabled_);
	for (unsigned i = 0; i < original_rasters.size(); i++)
	{
 		TEAGN_TRUE_OR_THROW(rmse_index(original_rasters[i], original_bands[i], fused_rasters[i], fused_bands[i], rmse), "Unable to compute the RMSE index");
		original_mean = stat.getMean(i);
		sum += (*rmse * *rmse) / (original_mean * original_mean);
		progress.Increment();
	}
	progress.Toggle(progress_enabled_);

	*ergas = 100.0 * (high_resolution / low_resolution) * sqrt((1.0 / (double)original_rasters.size()) * sum);

	return true;
}

bool TePDIFusionIndexes::structural_similarity_index(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TeSharedPtr<double> simm)
{
	double	C1 = 0.01,
			C2 = 0.03;

	TePDIStatistic stat;
	
	TePDIParameters stat_pars;
	TePDITypes::TePDIRasterVectorType stat_rasters;
	stat_rasters.push_back(original_raster);
	stat_rasters.push_back(fused_raster);
	stat_pars.SetParameter("rasters", stat_rasters);
	std::vector<int> stat_bands;
	stat_bands.push_back(original_band);
	stat_bands.push_back(fused_band);
	stat_pars.SetParameter("bands", stat_bands);

	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(progress_enabled_);
	
	TeMatrix	mean_matrix = stat.getMeanMatrix(),
				stddev_matrix = stat.getStdDevMatrix();
	double	covariance = stat.getCovariance(0, 1);

	*simm = ((2 * mean_matrix(0, 0) * mean_matrix(0, 1) + C1) * (2 * covariance + C2)) / ((mean_matrix(0, 0) * mean_matrix(0, 0) + mean_matrix(0, 1) * mean_matrix(0, 1) + C1) * (stddev_matrix(0, 0) * stddev_matrix(0, 0) + stddev_matrix(1, 0) * stddev_matrix(1, 0) + C2));

	return true;
}

bool TePDIFusionIndexes::fit_histogram(TePDITypes::TePDIRasterPtrType original_raster, int original_band, TePDITypes::TePDIRasterPtrType pan_raster, int pan_band, TePDITypes::TePDIRasterPtrType temp_raster)
{
	TePDIStatistic stat;
	
	TePDIParameters stat_pars;
	TePDITypes::TePDIRasterVectorType stat_rasters;
	stat_rasters.push_back(original_raster);
	stat_rasters.push_back(pan_raster);
	stat_pars.SetParameter("rasters", stat_rasters);
	std::vector<int> stat_bands;
	stat_bands.push_back(original_band);
	stat_bands.push_back(pan_band);
	stat_pars.SetParameter("bands", stat_bands);

	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(false);

	TeMatrix	std_matrix = stat.getStdDevMatrix(),
			mean_matrix = stat.getMeanMatrix();

	double	pixel,
		gain = 1.0,
		offset = 0.0;

 	gain = std_matrix(0, 0) / std_matrix(1, 0);
 	offset = mean_matrix(0, 0) - (gain * mean_matrix(0, 1));

	TePDIPIManager progress("Fitting histograms", pan_raster->params().nlines_, progress_enabled_);
	for (int j = 0; j < pan_raster->params().nlines_; j++)
	{
		for (int i = 0; i < pan_raster->params().ncols_; i++)
			if (pan_raster->getElement(i, j, pixel))
			{
				pixel = gain * pixel + offset;
				temp_raster->setElement(i, j, pixel, 0);
			}
		progress.Increment();
	}
	progress.Toggle(false);

	return true;
}

bool TePDIFusionIndexes::sergas_index(TePDITypes::TePDIRasterVectorType original_rasters, std::vector<int> original_bands, TePDITypes::TePDIRasterPtrType pan_raster, int pan_band, TePDITypes::TePDIRasterVectorType fused_rasters, std::vector<int> fused_bands, double low_resolution, double high_resolution, TeSharedPtr<double> ergas)
{
	TeSharedPtr<double> rmse(new double);
	double	temp_mean,
		sum = 0.0;

	TePDIPIManager progress("Computing ERGAS index", original_rasters.size(), progress_enabled_);
	for (unsigned i = 0; i < original_rasters.size(); i++)
	{
		TePDITypes::TePDIRasterPtrType tempRaster;
		TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(tempRaster, pan_raster->params(), TePDIUtils::TePDIUtilsAutoMemPol), "Unable create tempRaster");
		fit_histogram(original_rasters[i], original_bands[i], pan_raster, pan_band, tempRaster);
 		TEAGN_TRUE_OR_THROW(rmse_index(tempRaster, 0, fused_rasters[i], fused_bands[i], rmse), "Unable to compute the RMSE index");
		TEAGN_TRUE_OR_THROW(raster_mean(tempRaster, 0, temp_mean), "Unable to compute Temp raster mean");
		sum += (*rmse * *rmse) / (temp_mean * temp_mean);
		progress.Increment();
	}
	progress.Toggle(false);

	*ergas = 100.0 * (high_resolution / low_resolution) * sqrt((1 / (double)original_rasters.size()) * sum);

	return true;
}

bool TePDIFusionIndexes::spatial_correlation_coefficient_index(TePDITypes::TePDIRasterPtrType fused_raster, int fused_band, TePDITypes::TePDIRasterPtrType pan_raster, int pan_band, TeSharedPtr<double> scc)
{
	int x,
		y;
	int m[3][3];
	double	pf,
			pp,
			sumf,
			sump;

	m[0][0] = -1; m[0][1] = -1; m[0][2] = -1;
	m[1][0] = -1; m[1][1] = 8;  m[1][2] = -1;
	m[2][0] = -1; m[2][1] = -1; m[2][2] = -1;

	TeRasterParams fused_raster_params = fused_raster->params();
	TePDITypes::TePDIRasterPtrType fused_raster_lap;
	TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(fused_raster_lap,  fused_raster_params, TePDIUtils::TePDIUtilsAutoMemPol), "Unable create the fused_raster_lap raster");

	TeRasterParams pan_raster_params = pan_raster->params();
	TePDITypes::TePDIRasterPtrType pan_raster_lap;
	TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(pan_raster_lap, pan_raster_params, TePDIUtils::TePDIUtilsAutoMemPol), "Unable create the pan_raster_lap raster");
	
	TePDIPIManager progress("Computing SCC index", fused_raster->params().nlines_, progress_enabled_);
	for (int j = 0; j < fused_raster->params().nlines_; j++)
	{
		for (int i = 0; i < fused_raster->params().ncols_; i++)
		{
			sumf = sump = 0.0;
			for (int l = 0; l < 3; l++)
			{
				for (int k = 0; k < 3; k++)
				{
					y = j+l-1;
					x = i+k-1;
					if (y < 0)
						y = 0;
					else if (y >= fused_raster->params().nlines_)
						y = fused_raster->params().nlines_-1;
					if (x < 0)
						x = 0;
					else if (x >= fused_raster->params().ncols_)
						x = fused_raster->params().ncols_-1;
					fused_raster->getElement(x, y, pf, fused_band);
					pan_raster->getElement(x, y, pp, pan_band);
					sumf += (pf * (double)m[k][l]);
					sump += (pp * (double)m[k][l]);
				}
			}
			sumf = (sumf<0.0?0.0:(sumf>256.0?256.0:sumf));
			sump = (sump<0.0?0.0:(sump>256.0?256.0:sump));
			fused_raster_lap->setElement(i, j, sumf, 0);
			pan_raster_lap->setElement(i, j, sump, 0);
		}
		progress.Increment();
	}
	progress.Toggle(false);

	//TEAGN_TRUE_OR_THROW(TePDIUtils::TeRaster2Geotiff(fused_raster_lap, "o1.tif"), "GeoTIFF generation error");
	//TEAGN_TRUE_OR_THROW(TePDIUtils::TeRaster2Geotiff(pan_raster_lap, "o2.tif"), "GeoTIFF generation error");

	TeSharedPtr<double> cc(new double);
	correlation_coefficient_index(fused_raster_lap, 0, pan_raster_lap, 0, cc);

	*scc = *cc;

	return true;
}

bool TePDIFusionIndexes::RunImplementation()
{
/* Getting parameters */
	int index_type;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("index_type", index_type), "Missing parameter: index_type");

	if ((index_type == TePDIFusionIndexes::CC) || (index_type == TePDIFusionIndexes::VARDIF) || (index_type == TePDIFusionIndexes::SDD) || (index_type == TePDIFusionIndexes::BIAS) || (index_type == TePDIFusionIndexes::UIQI) || (index_type == TePDIFusionIndexes::RMSE) || (index_type == TePDIFusionIndexes::SIMM))
	{
		TePDITypes::TePDIRasterPtrType original_raster;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("original_raster", original_raster), "Missing parameter: original_raster");
	
		int original_band;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("original_band", original_band), "Missing parameter: original_band");
	
		TePDITypes::TePDIRasterPtrType fused_raster;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("fused_raster", fused_raster), "Missing parameter: fused_raster");
	
		int fused_band;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("fused_band", fused_band), "Missing parameter: fused_band");
	
		TeSharedPtr<double> index_value;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("index_value", index_value), "Missing parameter: index_value");

		if (index_type == TePDIFusionIndexes::CC)
			correlation_coefficient_index(original_raster, original_band, fused_raster, fused_band, index_value);
		else if (index_type == TePDIFusionIndexes::BIAS)
			bias_index(original_raster, original_band, fused_raster, fused_band, index_value);
		else if (index_type == TePDIFusionIndexes::VARDIF)
			variance_difference_index(original_raster, original_band, fused_raster, fused_band, index_value);
		else if (index_type == TePDIFusionIndexes::SDD)
			standard_deviation_diff_index(original_raster, original_band, fused_raster, fused_band, index_value);
		else if (index_type == TePDIFusionIndexes::UIQI)
			uiqi_index(original_raster, original_band, fused_raster, fused_band, index_value);
		else if (index_type == TePDIFusionIndexes::RMSE)
			rmse_index(original_raster, original_band, fused_raster, fused_band, index_value);
		else if (index_type == TePDIFusionIndexes::SIMM)
			structural_similarity_index(original_raster, original_band, fused_raster, fused_band, index_value);
	}
	else if ((index_type == TePDIFusionIndexes::ERGAS) || (index_type == TePDIFusionIndexes::SERGAS))
	{
		TePDITypes::TePDIRasterVectorType original_rasters;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("original_rasters", original_rasters), "Missing parameter: original_rasters");
	
		std::vector<int> original_bands;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("original_bands", original_bands), "Missing parameter: original_bands");

		TePDITypes::TePDIRasterVectorType fused_rasters;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("fused_rasters", fused_rasters), "Missing parameter: fused_rasters");
	
		std::vector<int> fused_bands;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("fused_bands", fused_bands), "Missing parameter: fused_bands");

		double low_resolution;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("low_resolution", low_resolution), "Missing parameter: low_resolution");

		double high_resolution;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("high_resolution", high_resolution), "Missing parameter: high_resolution");
	
		TeSharedPtr<double> index_value;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("index_value", index_value), "Missing parameter: index_value");

		if (index_type == TePDIFusionIndexes::ERGAS)
			ergas_index(original_rasters, original_bands, fused_rasters, fused_bands, low_resolution, high_resolution, index_value);
		else if (index_type == TePDIFusionIndexes::SERGAS)
		{
			TePDITypes::TePDIRasterPtrType pan_raster;
			TEAGN_TRUE_OR_RETURN(params_.GetParameter("pan_raster", pan_raster), "Missing parameter: pan_raster");
			int pan_band;
			TEAGN_TRUE_OR_RETURN(params_.GetParameter("pan_band", pan_band), "Missing parameter: pan_band");
			sergas_index(original_rasters, original_bands, pan_raster, pan_band, fused_rasters, fused_bands, low_resolution, high_resolution, index_value);
		}
	}
	else if (index_type == TePDIFusionIndexes::SCC)
	{
		TePDITypes::TePDIRasterPtrType fused_raster;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("fused_raster", fused_raster), "Missing parameter: fused_raster");
	
		int fused_band;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("fused_band", fused_band), "Missing parameter: fused_band");

		TePDITypes::TePDIRasterPtrType pan_raster;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("pan_raster", pan_raster), "Missing parameter: pan_raster");
	
		int pan_band;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("pan_band", pan_band), "Missing parameter: pan_band");
	
		TeSharedPtr<double> index_value;
		TEAGN_TRUE_OR_RETURN(params_.GetParameter("index_value", index_value), "Missing parameter: index_value");

		spatial_correlation_coefficient_index(fused_raster, fused_band, pan_raster, pan_band, index_value);
	}
		
	return true;
}
