#include "TePDIDecorrelationEnhancement.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"
#include "TePDITypes.hpp"
#include "TePDIStatistic.hpp"
#include "TePDIPrincipalComponents.hpp"
#include "../kernel/TeMatrix.h"
#include "../kernel/TeUtils.h"
#include <math.h>

TePDIDecorrelationEnhancement::TePDIDecorrelationEnhancement()
{
}

TePDIDecorrelationEnhancement::~TePDIDecorrelationEnhancement()
{
}

void TePDIDecorrelationEnhancement::ResetState(const TePDIParameters&)
{
}

bool TePDIDecorrelationEnhancement::CheckParameters(const TePDIParameters& parameters) const
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
		TEAGN_TRUE_OR_RETURN(bands[input_rasters_index] < input_rasters[input_rasters_index]->nBands(), "Invalid parameter: bands[ " + Te2String(input_rasters_index) + " ]");
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().nlines_ == input_rasters[0]->params().nlines_, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " with imcompatible number of lines");
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().ncols_ == input_rasters[0]->params().ncols_, "Invalid parameter: input_raster " + Te2String( input_rasters_index ) + " with imcompatible number of columns" );
		TEAGN_TRUE_OR_RETURN(((input_rasters[input_rasters_index]->params().photometric_[bands[input_rasters_index]] == TeRasterParams::TeRGB) || (input_rasters[input_rasters_index]->params().photometric_[bands[input_rasters_index]] == TeRasterParams::TeMultiBand)), "Invalid parameter - input_rasters (invalid photometric interpretation)");
	}

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

	return true;
}

bool TePDIDecorrelationEnhancement::RunImplementation()
{
/********* Direct analysis *********/

	TePDIPrincipalComponents::TePDIPCAType analysis_type = 
	  TePDIPrincipalComponents::TePDIPCADirect;

	TePDIParameters params_direct;

	TePDITypes::TePDIRasterVectorType input_rasters;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");

	std::vector<int> bands_direct;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("bands", bands_direct), "Missing parameter: bands");

	TePDITypes::TePDIRasterVectorType output_rasters;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("output_rasters", output_rasters), "Missing parameter: output_rasters");

	TePDITypes::TePDIRasterVectorType output_rasters_direct;
	for (unsigned b = 0; b < output_rasters.size(); b++)
	{
		TePDITypes::TePDIRasterPtrType outRaster_direct;
		TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(outRaster_direct, 1, 1, 1, false, TeDOUBLE, 0), "RAM Raster " + Te2String(b) + " Alloc error");
		output_rasters_direct.push_back(outRaster_direct);
	}
	
	TeSharedPtr<TeMatrix> covariance_matrix(new TeMatrix);
	
	params_direct.SetParameter("analysis_type", analysis_type);
	params_direct.SetParameter("input_rasters", input_rasters);
	params_direct.SetParameter("bands", bands_direct);
	params_direct.SetParameter("output_rasters", output_rasters_direct);
	params_direct.SetParameter("covariance_matrix", covariance_matrix);
	
	TePDIPrincipalComponents pc_direct;
	pc_direct.ToggleProgInt( progress_enabled_ );
	TEAGN_TRUE_OR_THROW(pc_direct.Reset(params_direct), "Invalid Parameters");
	TEAGN_TRUE_OR_THROW(pc_direct.Apply(), "Apply error");
	
/********* Enhancing pca components **********/
	std::vector< int > bands_enhancement;
	for (unsigned b = 0; b < output_rasters_direct.size(); b++)
		bands_enhancement.push_back(0);

	TePDIStatistic stat;
	stat.ToggleProgInt( progress_enabled_ );
	
	TePDIParameters stat_pars;
	stat_pars.SetParameter("rasters", output_rasters_direct);
	stat_pars.SetParameter("bands", bands_enhancement);
	
	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");

	TeMatrix	std_matrix = stat.getStdDevMatrix(),
			mean_matrix = stat.getMeanMatrix();
	double	pixel,
		gain,
		offset;
		
  TePDIPIManager progress( "Enhancing PCA components", 
    output_rasters_direct.size(), 
    progress_enabled_ );

	for (unsigned int b = 1; b < output_rasters_direct.size(); b++)
	{
		gain = std_matrix(0, 0) / std_matrix(b, 0);
		offset = mean_matrix(0, b) - (gain * mean_matrix(0, b));
		for (int i = 0; i < output_rasters_direct[0]->params().ncols_; i++)
		{
			for (int j = 0; j < output_rasters_direct[0]->params().nlines_; j++)
				if (output_rasters_direct[b]->getElement(i, j, pixel))
				{
					pixel = gain * pixel + offset;
					output_rasters_direct[b]->setElement(i, j, pixel);
				}
	  }
	  
    TEAGN_FALSE_OR_RETURN( progress.Increment(),
      "Canceled by the user" );	  
	}
	
	progress.Reset();

/********* Inverse analysis *********/
	analysis_type = TePDIPrincipalComponents::TePDIPCAInverse;
	
	TePDIParameters params_inverse;

	std::vector< int > bands_inverse;
	for (unsigned b = 0; b < output_rasters.size(); b++)
		bands_inverse.push_back(0);

	params_inverse.SetParameter("analysis_type", analysis_type);
	params_inverse.SetParameter("input_rasters", output_rasters_direct);
	params_inverse.SetParameter("bands", bands_inverse);
	params_inverse.SetParameter("output_rasters", output_rasters);
	params_inverse.SetParameter("covariance_matrix", covariance_matrix);
	
	TePDIPrincipalComponents pc_inverse;
	pc_inverse.ToggleProgInt( progress_enabled_ );
	TEAGN_TRUE_OR_THROW(pc_inverse.Reset(params_inverse), "Invalid Parameters");
	TEAGN_TRUE_OR_THROW(pc_inverse.Apply(), "Apply error");

	
	return true;
}
