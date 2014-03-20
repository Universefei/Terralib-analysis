#include "TePDIGeneralizedIHSFusion.hpp"

#include "TePDIUtils.hpp"
#include "../kernel/TeAgnostic.h"
#include "TePDIColorTransform.hpp"
#include "TePDIPIManager.hpp"
#include "../kernel/TeRasterRemap.h"
#include "TePDIStatistic.hpp"
#include "../kernel/TeMatrix.h"

#define max_gray_levels 255.0
#define max_angle 360.0
#define PI 3.141592653589793238462643383279502884197169399375105820974944592
#define PIdiv2 (PI / 2)
#define PImult3div2 (3 * PI / 2)
#define PImult2 (2 * PI)
#define AlmostZero 0.0001

TePDIGeneralizedIHSFusion::TePDIGeneralizedIHSFusion()
{
}

TePDIGeneralizedIHSFusion::~TePDIGeneralizedIHSFusion()
{
}

void TePDIGeneralizedIHSFusion::ResetState(const TePDIParameters& )
{
}

bool TePDIGeneralizedIHSFusion::CheckParameters(const TePDIParameters& parameters) const
{
	PreProcessingType pp_type;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("pp_type", pp_type), "Missing parameter: pp_type");

	double mean;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("mean", mean), "Missing parameter: mean");

	double variance;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("variance", variance), "Missing parameter: variance");

	TePDITypes::TePDIRasterVectorType input_rasters;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_rasters" , input_rasters), "Missing parameter: input_rasters");
	for (unsigned b = 0; b < input_rasters.size(); b++)
	{
		TEAGN_TRUE_OR_RETURN(input_rasters[b].isActive(), "Invalid parameter: input_rasters " + Te2String(b+1) + " inactive");
  		TEAGN_TRUE_OR_RETURN(input_rasters[b]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_rasters " + Te2String(b+1) + " not ready");
	}

	vector<double> bands_center;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("bands_center", bands_center), "Missing parameter: bands_center");
	TEAGN_TRUE_OR_RETURN(bands_center.size() == input_rasters.size(), "Invalid Parameter: number os bands center angles")

	bool pansharpening;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("pansharpening", pansharpening), "Missing parameter: pansharpening");

	if (pansharpening)
	{
		TePDITypes::TePDIRasterPtrType reference_raster;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_raster", reference_raster), "Missing parameter: reference_raster");
		TEAGN_TRUE_OR_RETURN(reference_raster.isActive(), "Invalid parameter: reference_raster inactive");
		TEAGN_TRUE_OR_RETURN(reference_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: reference_raster not ready");
	}

	int reference_raster_band;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_raster_band", reference_raster_band), "Missing parameter: reference_raster_band");
	
	TePDITypes::TePDIRasterVectorType ihs_output_rasters;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("ihs_output_rasters" , ihs_output_rasters), "Missing parameter: ihs_output_rasters");
	for (unsigned b = 0; b < ihs_output_rasters.size(); b++)
	{
		TEAGN_TRUE_OR_RETURN(ihs_output_rasters[b].isActive(), "Invalid parameter: ihs_output_rasters " + Te2String(b+1) + " inactive");
  		TEAGN_TRUE_OR_RETURN(ihs_output_rasters[b]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: ihs_output_rasters " + Te2String(b+1) + " not ready");
	}

	TePDITypes::TePDIRasterPtrType rgb_output_raster;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("rgb_output_raster", rgb_output_raster), "Missing parameter: rgb_output_raster");
	TEAGN_TRUE_OR_RETURN(rgb_output_raster.isActive(), "Invalid parameter: rgb_output_raster inactive");
	TEAGN_TRUE_OR_RETURN(rgb_output_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: rgb_output_raster not ready");

	return true;
}

double max_three(std::vector<double> list)
{
	double max_value = -1;
	for (unsigned int i = 0; i < list.size(); i++)
		if (list[i] > max_value)
			max_value = list[i];
	//return (max_value == 0.0? AlmostZero: max_value);
	return max_value;
}

double min_three(std::vector<double> list)
{
	double min_value = max_gray_levels + 1.0;
	for (unsigned int i = 0; i < list.size(); i++)
		if (list[i] < min_value)
			min_value = list[i];
	//return (min_value == 0.0? AlmostZero: min_value);
	return min_value;
}

double compute_raster_mean(TePDITypes::TePDIRasterPtrType input_raster)
{
	double mean = 0.0;
	for(int i = 0; i < input_raster->params().ncols_; i++)
		for(int j = 0; j < input_raster->params().nlines_; j++)
		{
			double pixel;
			input_raster->getElement(i, j, pixel);
			mean += pixel;
		}
	mean /= (double)(input_raster->params().ncols_ * input_raster->params().nlines_);
	return mean;
}


double compute_raster_variance(TePDITypes::TePDIRasterPtrType input_raster, double mean)
{
	double variance = 0.0;
	for(int i = 0; i < input_raster->params().ncols_; i++)
		for(int j = 0; j < input_raster->params().nlines_; j++)
		{
			double pixel;
			input_raster->getElement(i, j, pixel);
			variance += ((pixel - mean) * (pixel - mean));
		}
	variance /= (double)(input_raster->params().ncols_ * input_raster->params().nlines_);
	return variance;
}

bool TePDIGeneralizedIHSFusion::RunImplementation()
{
	PreProcessingType pp_type;
	params_.GetParameter("pp_type", pp_type);

	double mean;
	params_.GetParameter("mean", mean);

	double variance;
	params_.GetParameter("variance", variance);

	vector<double> bands_center;
	params_.GetParameter("bands_center", bands_center);

	TePDITypes::TePDIRasterVectorType input_rasters;
	params_.GetParameter("input_rasters" , input_rasters);

// Compute covariance matrix before the transformation
	TePDIStatistic stat_before;

	TePDIParameters stat_before_pars;
	std::vector<int> bands(input_rasters.size(), 0);
	stat_before_pars.SetParameter("rasters", input_rasters);
	stat_before_pars.SetParameter("bands", bands);
	TEAGN_TRUE_OR_RETURN(stat_before.Reset(stat_before_pars), "Unable to inialize the statistc module");
	TeMatrix covariance_input_rasters = stat_before.getCovMatrix();
	covariance_input_rasters.Print();

	bool pansharpening;
	params_.GetParameter("pansharpening", pansharpening);

	TePDITypes::TePDIRasterPtrType reference_raster;
	params_.GetParameter("reference_raster", reference_raster);

	int reference_raster_band;
	params_.GetParameter("reference_raster_band", reference_raster_band);
	
	TePDITypes::TePDIRasterVectorType ihs_output_rasters;
	params_.GetParameter("ihs_output_rasters" , ihs_output_rasters);

	TePDITypes::TePDIRasterPtrType rgb_output_raster;
	params_.GetParameter("rgb_output_raster" , rgb_output_raster);

	int input_rasters_number = input_rasters.size();
	int output_rasters_number = 3;

	/* Base rasters params */
	TeRasterParams base_raster_params = input_rasters[0]->params();

	/* Preprocessing input multirasters */
	TePDITypes::TePDIRasterVectorType pp_input_rasters;
	for(int b = 0; b < input_rasters_number; b++)
	{
		TePDITypes::TePDIRasterPtrType ppInRaster;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(ppInRaster, 1, base_raster_params.nlines_, base_raster_params.ncols_, false, TeDOUBLE, 0), "pp_input_raster Alloc error");
		pp_input_rasters.push_back(ppInRaster);
	}

/* Preprocessing input rasters */
	std::vector<double>	means,
				variances,
				gains,
				offsets;

	TePDIPIManager progress("Pre-processing input rasters", 0, progress_enabled_);
	if (pp_type == VariancePreProcessing)
	{
		progress.Reset("Pre-processing input rasters - Type imposed", input_rasters_number);
		for(int b = 0; b < input_rasters_number; b++)
		{
			means.push_back(compute_raster_mean(input_rasters[b]));
			variances.push_back(compute_raster_variance(input_rasters[b], means[b]));
			gains.push_back(sqrt(variance / variances[b]));
			offsets.push_back(mean - (gains[b] * means[b]));
			for(int i = 0; i < input_rasters[b]->params().ncols_; i++)
				for(int j = 0; j < input_rasters[b]->params().nlines_; j++)
				{
					double pixel;
					input_rasters[b]->getElement(i, j, pixel);
					pixel = gains[b] * pixel + offsets[b];
					pixel = (pixel>255.0?255.0:pixel);
					pp_input_rasters[b]->setElement(i, j, pixel);
				}
			progress.Increment();
		}
	}
	else if (pp_type == MeanPreProcessing)
	{
		for(int b = 0; b < input_rasters_number; b++)
		{
			
			means.push_back(compute_raster_mean(input_rasters[b]));
			mean += means[b];
			variances.push_back(compute_raster_variance(input_rasters[b], means[b]));
			variance += variances[b];
		}
		mean /= (double)input_rasters_number;
		variance /= (double)input_rasters_number;
		progress.Reset("Pre-processing input rasters - Type means", input_rasters_number);
		for(int b = 0; b < input_rasters_number; b++)
		{
			gains.push_back(sqrt(variance / variances[b]));
			offsets.push_back(mean - (gains[b] * means[b]));
			for(int i = 0; i < input_rasters[b]->params().ncols_; i++)
				for(int j = 0; j < input_rasters[b]->params().nlines_; j++)
				{
					double pixel;
					input_rasters[b]->getElement(i, j, pixel);
					pixel = pixel * gains[b] + offsets[b];
					pixel = (pixel>255.0?255.0:pixel);
					pp_input_rasters[b]->setElement(i, j, pixel);
				}
			progress.Increment();
		}	
	}
	else if (pp_type == NoPreProcessing)
	{
		progress.Reset("Pre-processing input rasters - Type nothing", input_rasters_number);
		for(int b = 0; b < input_rasters_number; b++)
		{
			pp_input_rasters[b] = input_rasters[b];
			progress.Increment();
		}
	}
 else
 {
  TEAGN_LOG_AND_THROW( "Invalid pre-processing type");
 } 
 
/* Writing Preprocessing input rasters in disk */
//	for (int b = 0; b < input_rasters_number; b++)
//		TEAGN_TRUE_OR_THROW(TePDIUtils::TeRaster2Geotiff(pp_input_rasters[b], "PPInputRaster_test" + Te2String(b+1) + ".tif"), "GeoTIF generation error");

/* nbands2ihs */
	std::vector<double>	bands_center_sin,
				bands_center_cos;
	for (int b = 0; b < input_rasters_number; b++)
	{
		bands_center[b] = (bands_center[b] * PI) / 180.0;
		bands_center_sin.push_back(sin(bands_center[b]));
		bands_center_cos.push_back(cos(bands_center[b]));
	}

	progress.Reset("Non Orthogonal IHS Transformation", base_raster_params.ncols_);
	for (int i = 0; i < base_raster_params.ncols_; i++)
	{
		for (int j = 0; j < base_raster_params.nlines_; j++)
		{
			std::vector<double> pixel_vector(input_rasters_number, 0.0);
			for (int b = 0; b < input_rasters_number; b++)
				pp_input_rasters[b]->getElement(i, j, pixel_vector[b]);
			double	max_value = max_three(pixel_vector),
				min_value = min_three(pixel_vector);
			double	product_sin = 0.0,
				product_cos = 0.0;
			for (int b = 0; b < input_rasters_number; b++)
			{
				product_sin += (pixel_vector[b] - min_value) * bands_center_sin[b];
				product_cos += (pixel_vector[b] - min_value) * bands_center_cos[b];
			}
			std::vector<double> pixel_output;
			pixel_output.push_back(max_value);
			if (product_cos == 0.0)
				product_cos = AlmostZero;
			if (product_cos < 0.0)
 				pixel_output.push_back(max_angle * (atan(product_sin / product_cos) + PI) / PImult2);
			else
				if (product_sin < 0.0)
					pixel_output.push_back(max_angle * (PImult2 + atan(product_sin / product_cos)) / PImult2);
				else
					pixel_output.push_back(max_angle * atan(product_sin / product_cos) / PImult2);
			pixel_output[1] = (max_gray_levels * pixel_output[1]) / max_angle;
			pixel_output.push_back((max_value > 0.0?(max_gray_levels* (1.0-(min_value/max_value))):0.0));
			for (int b = 0; b < output_rasters_number; b++)
				ihs_output_rasters[b]->setElement(i, j, pixel_output[b]);
		}
		progress.Increment();
	}

/* ihs vector to ihs 3 bands */

	TeRasterParams ihs_temp_raster_params = input_rasters[0]->params();
	ihs_temp_raster_params.nBands(output_rasters_number);
	ihs_temp_raster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
	TePDITypes::TePDIRasterPtrType ihs_temp_raster;
	TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(ihs_temp_raster, ihs_temp_raster_params, TePDIUtils::TePDIUtilsAutoMemPol), "Unable create ihs_temp_raster");

	progress.Reset("Transforming raster vector in one raster", base_raster_params.ncols_);
	for (int i = 0; i < base_raster_params.ncols_; i++)
	{
		for (int j = 0; j < base_raster_params.nlines_; j++)
			for (int b = 0; b < output_rasters_number; b++)
			{
				double pixel;
				ihs_output_rasters[b]->getElement(i, j, pixel);
				ihs_temp_raster->setElement(i, j, pixel, b);
			}
		progress.Increment();
	}

	if (!pansharpening)
	{
		TePDIParameters ihs2rgb_params;
		ihs2rgb_params.SetParameter("input_image", ihs_temp_raster);
		ihs2rgb_params.SetParameter("output_image", rgb_output_raster);
		ihs2rgb_params.SetParameter("transf_type", TePDIColorTransform::Ihs2Rgb);
		TePDIColorTransform IHSTransform;
		TEAGN_TRUE_OR_THROW(IHSTransform.Apply(), "Unable to build IHS2RGB color space");

// Compute covariance matrix after the transformation
		TePDIStatistic stat_after;
	
		TePDIParameters stat_after_pars;
		std::vector<int> bands(output_rasters_number, 0);
		TePDITypes::TePDIRasterVectorType temp_output_rasters;
		for (int b = 0; b < output_rasters_number; b++)
		{
			temp_output_rasters.push_back(rgb_output_raster);
			bands[b] = b;
		}
		stat_after_pars.SetParameter("rasters", temp_output_rasters);
		stat_after_pars.SetParameter("bands", bands);
		TEAGN_TRUE_OR_RETURN(stat_after.Reset(stat_after_pars), "Unable to inialize the statistc module");
		TeMatrix covariance_output_rasters = stat_after.getCovMatrix();
		covariance_output_rasters.Print();

		return true;
	}

/* Bringing ihs_temp_raster to reference_raster size generating upsampled_raster */
	double dummy_value = 0;
	if (ihs_temp_raster->params().useDummy_)
		dummy_value = ihs_temp_raster->params().dummy_[0];
	
	TeRasterParams upsampled_raster_params;
	upsampled_raster_params.nBands(output_rasters_number);
	if (reference_raster->projection() != 0)
		upsampled_raster_params.projection(TeProjectionFactory::make(reference_raster->projection()->params()));
	upsampled_raster_params.boxResolution(reference_raster->params().box().x1(), reference_raster->params().box().y1(), reference_raster->params().box().x2(), reference_raster->params().box().y2(), reference_raster->params().resx_, reference_raster->params().resy_);
	upsampled_raster_params.setDataType(ihs_temp_raster->params().dataType_[0], -1);
	if (ihs_temp_raster->params().useDummy_)
		upsampled_raster_params.setDummy(dummy_value, -1);
	else
		upsampled_raster_params.useDummy_ = false;
	upsampled_raster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
	
	TePDITypes::TePDIRasterPtrType upsampled_raster;
	TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(upsampled_raster, upsampled_raster_params, TePDIUtils::TePDIUtilsAutoMemPol), "Unable create the new upsampled raster RGB raster");
	
	TeRasterRemap remap_instance(ihs_temp_raster.nakedPointer(), upsampled_raster.nakedPointer());
	TEAGN_TRUE_OR_RETURN(remap_instance.apply(true), "Unable to remap ihs_temp_raster");

/* Swapping intensity band */
	progress.Reset("Swapping panchromatic and intensity channel", reference_raster->params().ncols_);
	for (int i = 0; i < reference_raster->params().ncols_; i++)
	{
		for (int j = 0; j < reference_raster->params().nlines_; j++)
		{
			double pixel;
			reference_raster->getElement(i, j, pixel, reference_raster_band);
			upsampled_raster->setElement(i, j, pixel, 0);

		}
		progress.Increment();
	}

//	TEAGN_TRUE_OR_THROW(TePDIUtils::TeRaster2Geotiff(upsampled_raster, "out.tif"), "GeoTIF generation error");

/* ihs2rgb */
	TePDIParameters ihs2rgb_params;
	ihs2rgb_params.SetParameter("input_image", upsampled_raster);
	ihs2rgb_params.SetParameter("output_image", rgb_output_raster);
	ihs2rgb_params.SetParameter("transf_type", TePDIColorTransform::Ihs2Rgb);
	TePDIColorTransform IHSTransform;
	TEAGN_TRUE_OR_THROW(IHSTransform.Apply( ihs2rgb_params ), 
    "Unable to build IHS2RGB color space");

	return true;
}
