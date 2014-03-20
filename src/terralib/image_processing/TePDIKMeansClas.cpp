#include "TePDIKMeansClas.hpp"
#include "TePDIStrategyFactory.hpp"
#include "TePDIAlgorithmFactory.hpp"
#include "TePDIUtils.hpp"

#include "../kernel/TeAgnostic.h"

TePDIKMeansClas::TePDIKMeansClas()
{
};

TePDIKMeansClas::~TePDIKMeansClas()
{
  tuple.clear();
  clusters.clear();
};

void TePDIKMeansClas::ResetState( const TePDIParameters& )
{

}

bool TePDIKMeansClas::CheckParameters(const TePDIParameters& parameters) const
{
// Checking input_rasters
  TePDITypes::TePDIRasterPtrType input_raster;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_raster", input_raster), "Missing parameter: input_raster");
  TEAGN_TRUE_OR_RETURN(input_raster->params().nBands() > 0, "Invalid input rasters number");

// Checking output_raster
  TePDITypes::TePDIRasterPtrType output_raster;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_raster", output_raster), "Missing parameter: output_raster");
  TEAGN_TRUE_OR_RETURN(output_raster.isActive(), "Invalid parameter: output_raster inactive");
  TEAGN_TRUE_OR_RETURN(output_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: output_raster not ready");

// Checking nClass
  int nClass = 0;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("classes_number", nClass), "Missing parameter: classes_number");
  TEAGN_TRUE_OR_RETURN( ( nClass > 0 ) , "Invalid parameter - classes_number" );

// Checking bands
  vector<int> bands;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("bands", bands), "Missing parameter: bands");
  TEAGN_TRUE_OR_RETURN(bands.size() > 0, " Invalid parameter: bands number ");
  for( unsigned int bands_index = 0 ; bands_index < bands.size() ; 
    ++bands_index ) {
    
    TEAGN_TRUE_OR_RETURN( 
      ( bands[ bands_index ] < input_raster->params().nBands() ),
      "Invalid parameter : bands[" + Te2String( bands_index ) + "]" )  
  }
  
  return true;
}

bool TePDIKMeansClas::RunImplementation()
{
// Getting raster vector
  params_.GetParameter("input_raster", input_raster);
  
// Getting output raster
  params_.GetParameter("output_raster", output_raster);

  /* Setting the output raster */
  //TeRasterParams output_raster_params = output_raster->params();

  //output_raster_params.setDataType( TeUNSIGNEDCHAR );
  //output_raster_params.nBands( 1 );
  //if( input_raster->projection() != 0 ) 
  //{
  //  output_raster_params.projection( input_raster->projection() );
  //}

  //output_raster_params.boxResolution( input_raster->params().box().x1(), 
  //  input_raster->params().box().y1(), input_raster->params().box().x2(), 
  //  input_raster->params().box().y2(), input_raster->params().resx_, 
  //  input_raster->params().resy_ ); 
  
  TEAGN_TRUE_OR_RETURN( output_raster->init( ), 
    "Output raster reset error" );

  output_raster->params().status_ = TeRasterParams::TeReadyToWrite;

// Getting bands
  params_.GetParameter("bands", bands);
  n_Bands = bands.size();

// Getting number of classe
  params_.GetParameter("classes_number", n_Class);

// Checking nIter
  int nIter = 0;
  if( params_.CheckParameter< int >( "iterations_number" ) ) 
  {
    params_.GetParameter( "iterations_number", nIter );
  }
  n_Iter = nIter <= 0 ? 10 : nIter;

// Checking initial line
  int lineBegin = 0;
  if( params_.CheckParameter< int >( "line_begin" ) ) 
  {
    params_.GetParameter( "line_begin", lineBegin );
  }
  initial_line = lineBegin <= 0 || lineBegin >=  input_raster->params().nlines_ ? 0 : lineBegin;
  
// Checking end line
  int lineEnd = 0;
  if( params_.CheckParameter< int >( "line_end" ) ) 
  {
    params_.GetParameter( "line_end", lineEnd );
  }
  end_line = lineEnd <= 0 || lineEnd >= input_raster->params().nlines_ ? input_raster->params().nlines_ - 1 : lineEnd;

// Checking initial column
  int columnBegin = 0;
  if( params_.CheckParameter< int >( "column_begin" ) ) 
  {
    params_.GetParameter( "column_begin", columnBegin );
  }
  initial_column = columnBegin <= 0 || columnBegin >= input_raster->params().ncols_ ? 0 : columnBegin;

// Checking end column
  int columnEnd = 0;
  if( params_.CheckParameter< int >( "column_end" ) ) 
  {
    params_.GetParameter( "column_end", columnEnd );
  }
  end_column = columnEnd <= 0 || columnEnd >= input_raster->params().ncols_ ? input_raster->params().ncols_ - 1 : columnEnd;

// Checking fSamp
  int fSamp = 0;
  if( params_.CheckParameter< int >( "sample" ) ) 
  {
    params_.GetParameter( "sample", fSamp );
  }
  
  int defaultSamp = ( (end_line - initial_line + 1)*( end_column - initial_column + 1) ) / (256*256);
  
  if( defaultSamp <= 0 )
    defaultSamp = 1;

  f_Samp = fSamp == 0 ? defaultSamp : fSamp;

//initialization of cluster vector
  for( int i = 0; i < 256; ++i )
  {
    clusters.push_back(0);
  }
  
  if(!classifyPixels(initial_line, end_line, columnBegin, end_column))
  {
	  return false;
  }


  return true;
}

bool TePDIKMeansClas::GenerateClassificationParameters()
{  
// Creates first Cluster
  clusters[0] =  new TePDICluster( n_Bands );

  tuple.clear();

  TePDIPIManager progress("Generating Classification Parameters", end_line,
    progress_enabled_ );

  for( int lin = initial_line; lin <= end_line; lin += f_Samp )
  {
    for( int col = initial_column; col <= end_column; col += f_Samp )
    {
      for( int band = 0; band < n_Bands; ++band )
      {
        double value;
        input_raster->getElement(col,lin, value, bands[band]);
        tuple.push_back(value);
      }

      clusters[0]->add(tuple);

      tuple.clear();
    }

    TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
  }

  progress.Toggle(false);

  clusters[0]->updateStatistics();

// Generates other clusters 
  class_count = 1;
  while( class_count < n_Class )
  {
    if( !split() )
      break;
  }

  if(!kmeans())
  {
	  return false;
  }

  return true;
}

  
bool TePDIKMeansClas::split()
{
//Selects class with biggest variance
  TePDICluster* splitCluster = clusters[0];
  for( int indclass = 1; indclass < class_count; ++indclass )
  {
    if( splitCluster->maxVar() < clusters[indclass]->maxVar() )
    {
      splitCluster = clusters[indclass];
    }
  }

  if( splitCluster->splitRight().GetNban() <= 0 )
    return false;

// Splits Cluster
  clusters[class_count++] = new TePDICluster( splitCluster->splitRight() );
  *splitCluster = splitCluster->splitLeft();

// Classify Samples
  classifySamples();

  return true;
}
  
bool TePDIKMeansClas::kmeans()
{
  if( class_count <= 1 )
    return false;

  TePDIPIManager progress("Classifying Samples", n_Iter, progress_enabled_ );

  for( int iter = 0; iter < n_Iter; ++iter )
  {
    classifySamples();

    TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
  }
  
  progress.Toggle(false);

  return true;
}

void TePDIKMeansClas::classifySamples()
{
  int indclass;

// Reset Counts
  for( indclass = 0; indclass < class_count; ++indclass )
  {
    clusters[indclass]->resetCount();
  }

  tuple.clear();


// Classification of samples
  for( int lin = initial_line; lin <= end_line; lin += f_Samp )
  {
    for( int col = initial_column; col <= end_column; col += f_Samp )
    {
      for( int band = 0; band < n_Bands; ++band )
      {
        double value;
        input_raster->getElement(col, lin, value, bands[band]);
        tuple.push_back(value);
      }

      TePDICluster* bestCluster = clusters[0];
      
      double distance = bestCluster->dist( tuple );

      for( int indclass = 1; indclass < class_count; ++indclass )
      {
        if( clusters[indclass]->dist( tuple ) < distance )
        {
          bestCluster = clusters[indclass];
          distance = clusters[indclass]->dist( tuple );
        }
      }

      bestCluster->add( tuple );

      tuple.clear();
    }
  }

// Update Statistics
  for( indclass = 0; indclass < class_count; ++indclass )
  {
    clusters[indclass]->updateStatistics();
  }

  return;
}


bool TePDIKMeansClas::classifyPixels( int l1, int l2, int c1, int c2)
{
  vector<double> pixel;

  if(!GenerateClassificationParameters())
  {
	  return false;
  }

  TePDIPIManager progress("Classifying Pixels", l2, progress_enabled_ );

  for( int lin = l1; lin <= l2; lin++ )
  {
    for( int col = c1; col <= c2; col++ )
    {
	  bool dummy = true;

      for( int ban = 0; ban < n_Bands; ban++ )
      {
        double value;
        
		if(input_raster->getElement(col, lin, value, bands[ban]))
		{
			dummy = false;
		}

		pixel.push_back(value);
      }

	  if(!dummy)
	  {
		  double dist = 0;
		  double distance = clusters[0]->dist( pixel );
		  int besttheme = 0;
		  for(int indclass = 0; indclass < class_count; ++indclass )
		  {
			if( ( dist = clusters[indclass]->dist( pixel ) ) < distance )
			{
			  distance  = dist;
			  besttheme = indclass;
			}
		  }

		  pixel.clear();

		  output_raster->setElement(col, lin, besttheme);
	  }
    }

	TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
  }

  progress.Toggle(false);

  pixel.clear();

  return true;
}
