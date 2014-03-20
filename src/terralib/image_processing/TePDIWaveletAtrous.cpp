#include "TePDIWaveletAtrous.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"
#include "TePDITypes.hpp"
#include "TePDIStatistic.hpp"
#include "TePDIPrincipalComponents.hpp"
#include "../kernel/TeRasterRemap.h"
#include "../kernel/TeMatrix.h"
#include "../kernel/TeUtils.h"

#include <math.h>
#include <queue>

TePDIWaveletAtrous::TePDIWaveletAtrous()
{
}

TePDIWaveletAtrous::~TePDIWaveletAtrous()
{
}

void TePDIWaveletAtrous::ResetState(const TePDIParameters&)
{
}

bool TePDIWaveletAtrous::CheckParameters(const TePDIParameters& parameters) const
{
	int direction;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("direction", direction), "Missing parameter: direction");

	if (direction == DECOMPOSE)
	{
		TePDITypes::TePDIRasterPtrType input_raster;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_raster", input_raster), "Missing parameter: input_raster");
		TEAGN_TRUE_OR_RETURN(input_raster.isActive(), "Invalid parameter: input_raster inactive");
		TEAGN_TRUE_OR_RETURN(input_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_raster not ready");
	
		int band;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("band", band), "Missing parameter: band");
		TEAGN_TRUE_OR_RETURN(band < input_raster->nBands(), "Invalid parameter: band number");
	
		int levels;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("levels", levels), "Missing parameter: levels");
	
		TePDITypes::TePDIRasterVectorType output_wavelets;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_wavelets", output_wavelets), "Missing parameter: output_wavelets");

		TEAGN_TRUE_OR_RETURN((int)output_wavelets.size() == (levels + 1), "Invalid output rasters number");
	
		for(unsigned int b = 0; b < output_wavelets.size(); b++)
		{
			TEAGN_TRUE_OR_RETURN(output_wavelets[b].isActive(), "Invalid parameter: output_wavelets " + Te2String(b) + " inactive");
			TEAGN_TRUE_OR_RETURN(output_wavelets[b]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: output_wavelets " + Te2String(b) + " not ready");
		}
	}
	else if (direction == RECOMPOSE)
	{
		std::vector<TePDITypes::TePDIRasterVectorType> input_rasters_wavelets;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_rasters_wavelets", input_rasters_wavelets), "Missing parameter: input_rasters_wavelets");
		for(unsigned int w = 0; w < input_rasters_wavelets.size(); w++)
			for(unsigned int b = 0; b < input_rasters_wavelets[w].size(); b++)
			{
				TEAGN_TRUE_OR_RETURN(input_rasters_wavelets[w][b].isActive(), "Invalid parameter: input_rasters_wavelets inactive");
				TEAGN_TRUE_OR_RETURN(input_rasters_wavelets[w][b]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_rasters_wavelets not ready");
			}
	
		int rasters_levels;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("rasters_levels", rasters_levels), "Missing parameter: rasters_levels");
		for(unsigned int w = 0; w < input_rasters_wavelets.size(); w++)
			TEAGN_TRUE_OR_RETURN((rasters_levels + 1) == (int)input_rasters_wavelets[w].size(), "Invalid parameter: rasters_levels not ready");
	
		TePDITypes::TePDIRasterVectorType reference_raster_wavelets;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_raster_wavelets", reference_raster_wavelets), "Missing parameter: reference_raster_wavelets");
		for(unsigned int b = 0; b < reference_raster_wavelets.size(); b++)
		{
			TEAGN_TRUE_OR_RETURN(reference_raster_wavelets[b].isActive(), "Invalid parameter: reference_raster_wavelets inactive");
			TEAGN_TRUE_OR_RETURN(reference_raster_wavelets[b]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: reference_raster_wavelets not ready");
		}
	
		int reference_levels;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("reference_levels", reference_levels), "Missing parameter: reference_levels");
		TEAGN_TRUE_OR_RETURN((reference_levels + 1) == (int)reference_raster_wavelets.size(), "Invalid parameter: reference_levels not ready");
	
		TePDITypes::TePDIRasterVectorType output_rasters;
		TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_rasters", output_rasters), "Missing parameter: output_rasters");
	
		for(unsigned int b = 0; b < output_rasters.size(); b++)
		{
			TEAGN_TRUE_OR_RETURN(output_rasters[b].isActive(), "Invalid parameter: output_rasters " + Te2String(b) + " inactive");
			TEAGN_TRUE_OR_RETURN(output_rasters[b]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: output_rasters " + Te2String(b) + " not ready");
		}
	}
	else
		return false;

	return true;
}

bool TePDIWaveletAtrous::filterBank(string filterFile,TeMatrix &filter)
{
	FILE *ff;
	int l,
		c;
	double	t,
			w;
	

	if ((ff = fopen(filterFile.c_str(), "r")) != NULL)
	{
		fscanf(ff, "%d %d %lf", &l, &c, &t);
		filter.Init(l, c, 0.0);

		std::queue<double> maskWeights;
		while(fscanf(ff, "%lf", &w) == 1)
			maskWeights.push(w/t);

		for (int i = 0; i < l; i++)
			for (int j = 0; j < c; j++)
			{
				filter(i, j) = maskWeights.front();
				maskWeights.pop();
			}

		fclose(ff);
	}
	else
		return false;

	return true;
}

bool TePDIWaveletAtrous::filterBank(FilterType filtertype,TeMatrix &filter)
{
  switch( filtertype )
  {
    case B3SplineFilter :
    {
      TEAGN_TRUE_OR_RETURN( filter.Init( 5, 5, 0.0 ), "Filter init error" );
      const double weight = 256;
      
      filter(0,0) = 1/weight; filter(0,1) = 4/weight; filter(0,2) = 6/weight; filter(0,3) = 4/weight; filter(0,4) = 1/weight;
      filter(1,0) = 4/weight; filter(1,1) = 16/weight; filter(1,2) = 24/weight; filter(1,3) = 16/weight; filter(1,4) = 4/weight;
      filter(2,0) = 6/weight; filter(2,1) = 24/weight; filter(2,2) = 36/weight; filter(2,3) = 24/weight; filter(2,4) = 6/weight;
      filter(3,0) = 4/weight; filter(3,1) = 16/weight; filter(3,2) = 24/weight; filter(3,3) = 16/weight; filter(3,4) = 4/weight;
      filter(4,0) = 1/weight; filter(4,1) = 4/weight; filter(4,2) = 6/weight; filter(4,3) = 4/weight; filter(4,4) = 1/weight;
      
      break;
    }
    case TriangleFilter :
    {
      TEAGN_TRUE_OR_RETURN( filter.Init( 3, 3, 0.0 ), "Filter init error" );
      const double weight = 16;
      
      filter(0,0) = 1/weight; filter(0,1) = 2/weight; filter(0,2) = 1/weight;
      filter(1,0) = 2/weight; filter(1,1) = 4/weight; filter(1,2) = 2/weight;
      filter(2,0) = 1/weight; filter(2,1) = 2/weight; filter(2,2) = 1/weight;
      
      break;
    }  
    default :
    {
      TEAGN_LOG_AND_RETURN( "Invalid filter type" )
      break;
    }
  }
  
  return true;
}

bool TePDIWaveletAtrous::RunImplementation_decompose()
{
/* Getting parameters */
	TePDITypes::TePDIRasterPtrType input_raster;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("input_raster", input_raster), "Missing parameter: input_raster");

	int band;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("band", band), "Missing parameter: band");

	int levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("levels", levels), "Missing parameter: levels");

	TePDITypes::TePDIRasterVectorType output_wavelets;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("output_wavelets", output_wavelets), "Missing parameter: output_wavelets");
  
  // Retriving filter matrix

	TeMatrix filter;
  if( params_.CheckParameter< FilterType > ( "filter_type" ) )
  {
    FilterType filterType;
    TEAGN_TRUE_OR_RETURN( params_.GetParameter( "filter_type",
      filterType ), "Missin parameter filter_type" );
      
    TEAGN_TRUE_OR_THROW( filterBank( filterType, filter ), 
      "Filter matrix generation failed");
  }
  else if( params_.CheckParameter< std::string > ( "filter_file" ) )
  {
    std::string filter_file;
    TEAGN_TRUE_OR_RETURN( params_.GetParameter( "filter_file",
      filter_file ), "Missin parameter filter_file" );
  
	  TEAGN_TRUE_OR_THROW( filterBank( filter_file, filter), 
      "Filter matrix generation failed");
  }
  else
  {
    TEAGN_TRUE_OR_THROW( filterBank( TriangleFilter, filter ), 
      "Filter matrix generation failed");
  }

/* Allocating output rasters */	
	/*TeRasterParams base_raster_params = input_raster->params();
	for(int l = 1; l <= levels; l++)
	{
		TeRasterParams input_raster_params = base_raster_params;
		input_raster_params.nBands(waveletPlanes);
		if (input_raster_params.projection() != 0)
		{
			TeSharedPtr<TeProjection> proj(TeProjectionFactory::make(base_raster_params.projection()->params()));
			input_raster_params.projection(proj.nakedPointer());
		}
		input_raster_params.boxResolution(base_raster_params.box().x1(), base_raster_params.box().y1(), base_raster_params.box().x2(), base_raster_params.box().y2(), base_raster_params.resx_, base_raster_params.resy_);
		input_raster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
		input_raster_params.setDataType(TeFLOAT, -1);
		input_raster_params.setDummy(0.0, -1);
		for(int n = 0; n < waveletPlanes; n++)
			input_raster_params.nbitsperPixel_[n] = base_raster_params.nbitsperPixel_[band];
		TEAGN_TRUE_OR_RETURN(output_wavelets[l]->init(input_raster_params), "Output wavelets reset error " + Te2String(l));
	}*/

/* Setting variables */
	int	l,
		multi,
		x,
		y,
		i,
		j,
		filter_dim = filter.Nrow(),
		offset = (int)(filter_dim / 2),
		k,
		m,
		lines = input_raster->params().nlines_,
		columns = input_raster->params().ncols_;
	double	p_ori,
			p_ant,
			p_new;

/* Computing wavelet decomposition */
	for(l = 1; l <= levels; l++)
	{
		multi = (int)pow(2.0, l-1);
		TePDIPIManager progress("Decomposing Wavelet Level " + Te2String(l), input_raster->params().nlines_, progress_enabled_);
		for (j = 0; j < lines; j++)
		{
			for (i = 0; i < columns; i++)
 			{
				p_ori = p_ant = p_new = 0.0;
				for (k = 0; k < filter_dim; k++)
				{
					for (m = 0; m < filter_dim; m++)
					{
						x = i+(k-offset)*multi;
						y = j+(m-offset)*multi;
						if (x < 0)
							x = columns + x;
						else if (x >= columns)
							x = x - columns;
						if (y < 0)
							y = lines + y;
						else if (y >= lines)
							y = y - lines;
						output_wavelets[l-1]->getElement(x, y, p_ori, 0);
						p_new += p_ori * filter(k, m);
					}
				}
				output_wavelets[l]->setElement(i, j, p_new, 0);
				output_wavelets[l-1]->getElement(i, j, p_ori, 0);
				output_wavelets[l]->setElement(i, j, p_ori-p_new, 1);
			}
			progress.Increment();
		}
	}

	return true;
}

bool TePDIWaveletAtrous::RunImplementation_recompose()
{
/* Getting parameters */
	std::vector<TePDITypes::TePDIRasterVectorType> input_rasters_wavelets;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("input_rasters_wavelets", input_rasters_wavelets), "Missing parameter: input_rasters_wavelets");

	int rasters_levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("rasters_levels", rasters_levels), "Missing parameter: rasters_levels");

	TePDITypes::TePDIRasterVectorType reference_raster_wavelets;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_raster_wavelets", reference_raster_wavelets), "Missing parameter: reference_raster_wavelets");

	int reference_levels;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("reference_levels", reference_levels), "Missing parameter: reference_levels");

	TePDITypes::TePDIRasterVectorType output_rasters;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("output_rasters", output_rasters), "Missing parameter: output_rasters");

/* Setting variables */
	unsigned int b;
	int	i,
		j,
		l;
	double	pixel,
			p_pan;
	std::vector<double> channelMinLevel,
						channelMaxLevel;

	for(b = 0; b < input_rasters_wavelets.size(); b++)
	{
		channelMinLevel.push_back(0.0);
		//channelMaxLevel.push_back(pow(2.0, (double)input_rasters_wavelets[b][0]->params().nbitsperPixel_[0])-1.0);
		channelMaxLevel.push_back(255.0);
	}

/* Allocating output rasters */
	for(b = 0; b < input_rasters_wavelets.size(); b++)
	{
		TeRasterParams base_raster_params = input_rasters_wavelets[b][0]->params();
		TeRasterParams output_rasters_params = base_raster_params;
		output_rasters_params.nBands(1);
    output_rasters_params.projection( base_raster_params.projection() );
    output_rasters_params.boxResolution(base_raster_params.box().x1(), base_raster_params.box().y1(), base_raster_params.box().x2(), base_raster_params.box().y2(), base_raster_params.resx_, base_raster_params.resy_);
		output_rasters_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
		output_rasters_params.setDummy(0.0, -1);
    output_rasters_params.setDataType(TeFLOAT, -1);  
		TEAGN_TRUE_OR_RETURN(output_rasters[b]->init(output_rasters_params), "Output rasters reset error " + Te2String(b));
	}

/* Computing wavelet recomposition */
	for(b = 0; b < input_rasters_wavelets.size(); b++)
	{
		int	lines = input_rasters_wavelets[b][0]->params().nlines_,
			columns = input_rasters_wavelets[b][0]->params().ncols_;
      
    double maxOutValue = 0;
    double minOutValue = 0;
    TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
      output_rasters[b], 0, minOutValue, maxOutValue ),
      "Unable to determine the min/max output values" )
      
		TePDIPIManager progress("Recomposing Wavelet Band " + Te2String(b+1), input_rasters_wavelets[0][0]->params().nlines_, progress_enabled_);
		for (j = 0; j < lines; j++)
		{
			for (i = 0; i < columns; i++)
			{
				input_rasters_wavelets[b][rasters_levels]->getElement(i, j, pixel, 0);
				for(l = 1; l <= reference_levels; l++)
				{
					reference_raster_wavelets[l]->getElement(i, j, p_pan, 1);
					pixel += p_pan;
				}
				//pixel = (pixel > channelMaxLevel[b]?channelMaxLevel[b]:(pixel < channelMinLevel[b]?channelMinLevel[b]:pixel));
        
        pixel = MIN( pixel, maxOutValue );
        pixel = MAX( pixel, minOutValue );
        
				output_rasters[b]->setElement(i, j, pixel, 0);
			}
			progress.Increment();
		}
	}

	return true;
}

bool TePDIWaveletAtrous::RunImplementation()
{
/* Getting parameters */

	int direction;
	TEAGN_TRUE_OR_RETURN(params_.GetParameter("direction", direction), "Missing parameter: direction");

	if (direction == DECOMPOSE)
		return RunImplementation_decompose();
	else if (direction == RECOMPOSE)
		return RunImplementation_recompose();

	return false;
}
