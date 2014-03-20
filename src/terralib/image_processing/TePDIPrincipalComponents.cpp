#include "TePDIPrincipalComponents.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"
#include "TePDITypes.hpp"
#include "TePDIStatistic.hpp"

#include "../kernel/TeMatrix.h"
#include "../kernel/TeUtils.h"

#include <math.h>



TePDIPrincipalComponents::TePDIPrincipalComponents()
{
}


TePDIPrincipalComponents::~TePDIPrincipalComponents()
{
}


void TePDIPrincipalComponents::ResetState(
  const TePDIParameters&)
{
}


bool TePDIPrincipalComponents::CheckParameters(
  const TePDIParameters& parameters) const
{
/* Analysis type checking */
	TePDIPCAType analysis_type;

	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("analysis_type", 
	  analysis_type), "Missing parameter: analysis_type");
	TEAGN_TRUE_OR_RETURN(
	  ((analysis_type == TePDIPrincipalComponents::TePDIPCADirect) || 
	  (analysis_type == TePDIPrincipalComponents::TePDIPCAInverse)), 
	  "Invalid analysis type");

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
		TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().ncols_ == input_rasters[ 0 ]->params().ncols_, "Invalid parameter: input_raster " + Te2String( input_rasters_index ) + " with imcompatible number of columns" );
/* Checking photometric interpretation */
		TEAGN_TRUE_OR_RETURN(((input_rasters[input_rasters_index]->params().photometric_[bands[input_rasters_index]] == TeRasterParams::TeRGB) || (input_rasters[input_rasters_index]->params().photometric_[bands[input_rasters_index]] == TeRasterParams::TeMultiBand)), "Invalid parameter - input_rasters (invalid photometric interpretation)" );
	}

/* Output rasters checking */

	TePDITypes::TePDIRasterVectorType output_rasters;

  if( parameters.GetParameter("output_rasters", output_rasters) )
  {
    TEAGN_TRUE_OR_RETURN(output_rasters.size() > 1, "Invalid output rasters number");
    TEAGN_TRUE_OR_RETURN(output_rasters.size() == input_rasters.size(), "Invalid parameter: output rasters size");

    for( unsigned int output_rasters_index = 0; output_rasters_index < 
      output_rasters.size(); ++output_rasters_index )
    {
      TEAGN_TRUE_OR_RETURN(output_rasters[output_rasters_index].isActive(), 
        "Invalid parameter: output_rasters " + Te2String(output_rasters_index) + 
        " inactive");
      TEAGN_TRUE_OR_RETURN(output_rasters[output_rasters_index]->params().status_ 
        != TeRasterParams::TeNotReady, "Invalid parameter: output_rasters " + 
        Te2String(output_rasters_index) + " not ready");
    }
  }
  else
  {
    TePDITypes::TePDIRasterPtrType output_raster;
    TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_raster", output_raster), 
      "Missing parameter: output_raster or output_rasters");

    TEAGN_TRUE_OR_RETURN(output_raster.isActive(), 
      "Invalid parameter: output_raster inactive");
    TEAGN_TRUE_OR_RETURN(output_raster->params().status_ != 
      TeRasterParams::TeNotReady, "Invalid parameter: output_raster not ready");
  }
	
	/* Checking covariance_matrix */

	TeSharedPtr<TeMatrix> covariance_matrix;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter("covariance_matrix", 
	  covariance_matrix), "Missing parameter: covariance_matrix");

/* Inverse type checking */

	if (analysis_type == TePDIPrincipalComponents::TePDIPCAInverse)
		TEAGN_TRUE_OR_RETURN((((unsigned)covariance_matrix->Nrow() == input_rasters.size()) && ((unsigned)covariance_matrix->Ncol() == input_rasters.size())), "Covariance matrix out of rasters bounds");
		
	return true;
}

bool TePDIPrincipalComponents::RunImplementation_direct()
{
	TePDITypes::TePDIRasterVectorType input_rasters;
	params_.GetParameter("input_rasters", input_rasters);
	
	std::vector<int> bands;
	params_.GetParameter("bands", bands);

	TeSharedPtr<TeMatrix> covariance_matrix;
	params_.GetParameter("covariance_matrix", covariance_matrix);
	
	TeRasterParams base_raster_params = input_rasters[0]->params();
	
/* Setting the output rasters */

  TePDITypes::TePDIRasterVectorType output_rasters;
  std::vector< unsigned int > output_rasters_bands;
  
  {
    TeRasterParams base_raster_params = input_rasters[ 0 ]->params();
  
    if( params_.GetParameter("output_rasters", output_rasters) )
    {
      for(unsigned int outrasterindex = 0; outrasterindex < output_rasters.size(); 
        ++outrasterindex)
      {
        TeRasterParams outRaster_params = 
          output_rasters[outrasterindex]->params();
          
        outRaster_params.nBands(1);
        
        outRaster_params.projection(base_raster_params.projection());

        outRaster_params.boundingBoxLinesColumns(
          base_raster_params.boundingBox().x1(), 
          base_raster_params.boundingBox().y1(), 
          base_raster_params.boundingBox().x2(), 
          base_raster_params.boundingBox().y2(), 
          base_raster_params.nlines_, 
          base_raster_params.ncols_);
          
        outRaster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
      
        TEAGN_TRUE_OR_RETURN(output_rasters[outrasterindex]->init(
          outRaster_params), "Raster reset error - Raster index " + 
          Te2String(outrasterindex));
      }
      
      output_rasters_bands.resize( output_rasters.size(), 0 );
    }
    else
    {
      TePDITypes::TePDIRasterPtrType output_raster;
      TEAGN_TRUE_OR_THROW(params_.GetParameter("output_raster", output_raster), 
        "Missing parameter: output_raster or output_rasters");
        
      TeRasterParams outRaster_params = output_raster->params();
        
      outRaster_params.nBands( input_rasters.size() );
      
      outRaster_params.projection(base_raster_params.projection());

      outRaster_params.boundingBoxLinesColumns(
        base_raster_params.boundingBox().x1(), 
        base_raster_params.boundingBox().y1(), 
        base_raster_params.boundingBox().x2(), 
        base_raster_params.boundingBox().y2(), 
        base_raster_params.nlines_, 
        base_raster_params.ncols_);
        
      outRaster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
    
      TEAGN_TRUE_OR_RETURN( output_raster->init( outRaster_params ), 
        "Output raster reset error" );      
        
      for( unsigned int output_raster_band = 0 ; output_raster_band < 
        input_rasters.size() ; ++output_raster_band )
      {
        output_rasters[ output_raster_band ] = output_raster;
        output_rasters_bands.push_back( output_raster_band );
      }
    }
  }
	
/* Finding the mininum and maximum allowed pixel values */
	
	double channel_min_level = 0;
	double channel_max_level = 0;
	
	TEAGN_TRUE_OR_RETURN(TePDIUtils::TeGetRasterMinMaxBounds(
    output_rasters[0], 0, channel_min_level, channel_max_level), 
    "Unable to get raster channel level bounds");
	
/* Initializing statistic module */
	
	TePDIStatistic stat;
  stat.ToggleProgInt( progress_enabled_ );
	
	TePDIParameters stat_pars;
	stat_pars.SetParameter("rasters", input_rasters);
	stat_pars.SetParameter("bands", bands);
	
	TEAGN_TRUE_OR_RETURN(stat.Reset(stat_pars), "Unable to inialize the statistc module");
	stat.ToggleProgInt(progress_enabled_);

	
/* Building covariance matrix from all used bands ( lines == columns ) */

	TeSharedPtr<TeMatrix> x(new TeMatrix);
	*covariance_matrix = stat.getCovMatrix();
/* Building eigenvalues matrix from all used bands ( 1 column ) */

	TeMatrix eigenvaluesmatrix;
	TEAGN_TRUE_OR_RETURN(covariance_matrix->EigenValues(eigenvaluesmatrix), "Unable to build eigenvalue matrix");
	
/* Building eigenvectors matrix from all used bands (lines == columns) */

	TeMatrix tempmatrix;
	TEAGN_TRUE_OR_RETURN(covariance_matrix->EigenVectors(tempmatrix), "Unable to build eigenvectors matrix");
	TeMatrix eigenvectors;
	TEAGN_TRUE_OR_RETURN(tempmatrix.Transpose(eigenvectors), "Unable to build eigenvectors matrix");
	
/* Building auxiliary vectors for optimization */

	std::vector< bool > in_dummy_use_vector;
	for (unsigned int in_dummy_use_vector_index = 0; in_dummy_use_vector_index < input_rasters.size(); ++in_dummy_use_vector_index)
	{
		in_dummy_use_vector.push_back(input_rasters[in_dummy_use_vector_index]->params().useDummy_);
	}

	std::vector< double > out_dummy_vector;
	for (unsigned int out_dummy_vector_index = 0; out_dummy_vector_index < output_rasters.size(); ++out_dummy_vector_index)
	{
		if (output_rasters[out_dummy_vector_index]->params().useDummy_)
		{
			out_dummy_vector.push_back(output_rasters[out_dummy_vector_index]->params().dummy_[0]);
		}
		else
		{
			out_dummy_vector.push_back(0.0);
		}
	}
	
/* Do level remapping based on each matrix above */
	TeMatrix in_values; /* input levels */
	in_values.Init(bands.size(), 1, 0.0);
	
	TeMatrix intermediary_values;
	intermediary_values.Init(bands.size(), 1, 0.0);
	
	int line;
	int column;
	unsigned int index;
	unsigned int index_bound = bands.size();
	double value;
	bool missing_pixel = false;

	TePDIPIManager progress("Generating principal components", 
    base_raster_params.nlines_, progress_enabled_);
	for(line = 0 ; line < base_raster_params.nlines_ ; line++)
	{
		for(column = 0 ; column < base_raster_params.ncols_; ++column)
		{
/* Retriving the levels from all bands for the current line and column */
			missing_pixel = false;
	
			for(index = 0 ; index < index_bound ; ++index)
			{
				if (input_rasters[index]->getElement(column, line, value, bands[index]))
				{
					in_values(index, 0) = value;
				}
				else
				{
					TEAGN_TRUE_OR_RETURN(in_dummy_use_vector[index], "Raster read error");
					missing_pixel = true;
					break;
				}
			}
	
			if (missing_pixel)
			{
				for(index = 0 ; index < index_bound; ++index)
				{
					TEAGN_TRUE_OR_RETURN(output_rasters[index]->setElement(column, line, 
            out_dummy_vector[index], output_rasters_bands[ index ]), 
            "Unable to set raster element - raster index " + Te2String(index));
				}
			}
			else
			{
				intermediary_values = eigenvectors * in_values;
        
        /* Remapping levels */
				for(index = 0; index < index_bound; ++index)
				{
					value = intermediary_values(index, 0);
          
          /* Level range filtering */
 					if (value < channel_min_level)
 					{
 						value = channel_min_level;
 					}
 					if(value > channel_max_level)
 					{
 						value = channel_max_level;
 					}

					TEAGN_TRUE_OR_RETURN(output_rasters[index]->setElement(column, line, 
            value, output_rasters_bands[ index ]), 
            "Unable to write raster element - raster index " + Te2String(index));
				}
			}
		}
		progress.Increment();
	}
	progress.Toggle(false);

	return true;
}

bool TePDIPrincipalComponents::RunImplementation_inverse()
{
	TePDITypes::TePDIRasterVectorType input_rasters;
	params_.GetParameter("input_rasters", input_rasters);
	

	std::vector<int> bands;
	params_.GetParameter("bands", bands);

	TeSharedPtr<TeMatrix> covariance_matrix;
	params_.GetParameter("covariance_matrix", covariance_matrix);
	
	TeRasterParams base_raster_params = input_rasters[0]->params();
	
/* Setting the output rasters */

  TePDITypes::TePDIRasterVectorType output_rasters;
  std::vector< unsigned int > output_rasters_bands;
  
  {
    TeRasterParams base_raster_params = input_rasters[ 0 ]->params();
  
    if( params_.GetParameter("output_rasters", output_rasters) )
    {
      for(unsigned int outrasterindex = 0; outrasterindex < output_rasters.size(); 
        ++outrasterindex)
      {
        TeRasterParams outRaster_params = 
          output_rasters[outrasterindex]->params();
          
        outRaster_params.nBands(1);
        
        outRaster_params.projection(base_raster_params.projection());

        outRaster_params.boundingBoxLinesColumns(
          base_raster_params.boundingBox().x1(), 
          base_raster_params.boundingBox().y1(), 
          base_raster_params.boundingBox().x2(), 
          base_raster_params.boundingBox().y2(), 
          base_raster_params.nlines_, 
          base_raster_params.ncols_);
          
        outRaster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
      
        TEAGN_TRUE_OR_RETURN(output_rasters[outrasterindex]->init(
          outRaster_params), "Raster reset error - Raster index " + 
          Te2String(outrasterindex));
      }
      
      output_rasters_bands.resize( output_rasters.size(), 0 );
    }
    else
    {
      TePDITypes::TePDIRasterPtrType output_raster;
      TEAGN_TRUE_OR_THROW(params_.GetParameter("output_raster", output_raster), 
        "Missing parameter: output_raster or output_rasters");
        
      TeRasterParams outRaster_params = output_raster->params();
        
      outRaster_params.nBands( input_rasters.size() );
      
      outRaster_params.projection(base_raster_params.projection());

      outRaster_params.boundingBoxLinesColumns(
        base_raster_params.boundingBox().x1(), 
        base_raster_params.boundingBox().y1(), 
        base_raster_params.boundingBox().x2(), 
        base_raster_params.boundingBox().y2(), 
        base_raster_params.nlines_, 
        base_raster_params.ncols_);
        
      outRaster_params.setPhotometric(TeRasterParams::TeMultiBand, -1);
    
      TEAGN_TRUE_OR_RETURN( output_raster->init( outRaster_params ), 
        "Output raster reset error" );      
        
      for( unsigned int output_raster_band = 0 ; output_raster_band < 
        input_rasters.size() ; ++output_raster_band )
      {
        output_rasters.push_back( output_raster );
        output_rasters_bands.push_back( output_raster_band );
      }
    }
  }
	
/* Finding the mininum and maximum allowed pixel values */
	
	double channel_min_level = 0;
	double channel_max_level = 0;
	
	TEAGN_TRUE_OR_RETURN(TePDIUtils::TeGetRasterMinMaxBounds(
    output_rasters[0], 0, channel_min_level, channel_max_level), 
    "Unable to get raster channel level bounds");
	
/* Initializing statistic module */
	
	TeMatrix eigenvaluesmatrix;
	TEAGN_TRUE_OR_RETURN(covariance_matrix->EigenValues(eigenvaluesmatrix), "Unable to build eigenvalue matrix");
	
/* Building eigenvectors matrix from all used bands (lines == columns) */

	TeMatrix eigenvectors;
	TEAGN_TRUE_OR_RETURN(covariance_matrix->EigenVectors(eigenvectors), "Unable to build eigenvectors matrix");
	
/* Building auxiliary vectors for optimization */

	std::vector< bool > in_dummy_use_vector;
	for (unsigned int in_dummy_use_vector_index = 0; in_dummy_use_vector_index < input_rasters.size(); ++in_dummy_use_vector_index)
	{
		in_dummy_use_vector.push_back(input_rasters[in_dummy_use_vector_index]->params().useDummy_);
	}

	std::vector< double > out_dummy_vector;
	for (unsigned int out_dummy_vector_index = 0; out_dummy_vector_index < output_rasters.size(); ++out_dummy_vector_index)
	{
		if (output_rasters[out_dummy_vector_index]->params().useDummy_)
		{
			out_dummy_vector.push_back(output_rasters[out_dummy_vector_index]->params().dummy_[0]);
		}
		else
		{
			out_dummy_vector.push_back(0.0);
		}
	}
	
/* Do level remapping based on each matrix above */
	TeMatrix in_values; /* input levels */
	in_values.Init(bands.size(), 1, 0.0);
	
	TeMatrix intermediary_values;
	intermediary_values.Init(bands.size(), 1, 0.0);
	
	int line;
	int column;
	unsigned int index;
	unsigned int index_bound = bands.size();
	double value;
	bool missing_pixel = false;
	
	TePDIPIManager progress("Generating original images", base_raster_params.nlines_, progress_enabled_);
	for(line = 0 ; line < base_raster_params.nlines_ ; line++)
	{
		for(column = 0 ; column < base_raster_params.ncols_; ++column)
		{
/* Retriving the levels from all bands for the current line and column */
			missing_pixel = false;
			for(index = 0 ; index < index_bound ; ++index)
			{
				if (input_rasters[index]->getElement(column, line, value, bands[index]))
					in_values(index, 0) = value;
				else
				{
					TEAGN_TRUE_OR_RETURN(in_dummy_use_vector[index], "Raster read error");
					missing_pixel = true;
					break;
				}
			}
	
			if (missing_pixel)
			{
				for(index = 0 ; index < index_bound; ++index)
				{
					TEAGN_TRUE_OR_RETURN(output_rasters[index]->setElement(column, line, out_dummy_vector[index], 0), "Unable to set raster element - raster index " + Te2String(index));
				}
			}
			else
			{
				intermediary_values = eigenvectors * in_values;
        /* Remapping levels */
				for(index = 0; index < index_bound; ++index)
				{
					value = intermediary_values(index, 0);

          /* Level range filtering */
					if (value < channel_min_level)
					{
						value = channel_min_level;
					}
					if(value > channel_max_level)
					{
						value = channel_max_level;
					}
					TEAGN_TRUE_OR_RETURN(output_rasters[index]->setElement(column, line, 
            value, output_rasters_bands[ index ]), 
            "Unable to set raster element - raster index " + Te2String(index));
				}
			}
		}
		progress.Increment();
	}
	progress.Toggle(false);

	return true;
}

bool TePDIPrincipalComponents::RunImplementation()
{
	TePDIPCAType analysis_type;
	params_.GetParameter("analysis_type", analysis_type);

	if (analysis_type == TePDIPrincipalComponents::TePDIPCADirect)
		return RunImplementation_direct();
	else if (analysis_type == TePDIPrincipalComponents::TePDIPCAInverse)
		return RunImplementation_inverse();

	return true;
}


