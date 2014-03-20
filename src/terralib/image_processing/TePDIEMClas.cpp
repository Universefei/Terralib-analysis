#include "TePDIEMClas.hpp"
#include "TePDIMathFunctions.hpp"
#include "TePDIStrategyFactory.hpp"
#include "TePDIAlgorithmFactory.hpp"
#include "TePDIUtils.hpp"

#include "../kernel/TeBox.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeRasterParams.h"
#include "../kernel/TeRaster.h"
#include "../kernel/TeAgnostic.h"

void TePDIEMClas::ResetState( const TePDIParameters& )
{

}

TePDIEMClas::TePDIEMClas()
{
};

TePDIEMClas::~TePDIEMClas()
{
};

bool TePDIEMClas::CheckParameters(const TePDIParameters& parameters) const
{
  // Checking input_rasters
  TePDITypes::TePDIRasterVectorType input_rasters;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_rasters", input_rasters), 
    "Missing parameter: input_rasters");
  TEAGN_TRUE_OR_RETURN(input_rasters.size() > 0, 
    "Invalid input rasters number");

  vector<int> bands;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("bands", bands), "Missing parameter: bands");
  TEAGN_TRUE_OR_RETURN(bands.size() == input_rasters.size(), Te2String(input_rasters.size()) + " Invalid parameter: bands number " + Te2String(bands.size()));

  for(unsigned int input_rasters_index = 0 ; input_rasters_index < input_rasters.size(); input_rasters_index++)
  {
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index].isActive(), "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " inactive");
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " not ready");
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().nlines_ == input_rasters[0]->params().nlines_, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " with imcompatible number of lines");
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().ncols_ == input_rasters[0]->params().ncols_, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " with imcompatible number of columns");
  }

  int M;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("classes_to_find", M), "Missing parameter: classes_to_find");
  double e;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("epsilon", e), "Missing parameter: epsilon");
  TEAGN_TRUE_OR_RETURN(e > 0, "Parameter epsilon must be > 0");
  double s;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("shift_threshold", s), "Missing parameter: shift_threshold");
  TEAGN_TRUE_OR_RETURN(e > 0, "Parameter shift_threshold must be > 0");

  // Checking output_raster
  TePDITypes::TePDIRasterPtrType output_raster;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_raster", output_raster), "Missing parameter: output_raster");
  TEAGN_TRUE_OR_RETURN(output_raster.isActive(), "Invalid parameter: output_raster inactive");
  TEAGN_TRUE_OR_RETURN(output_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: output_raster not ready");
  return true;
}

bool TePDIEMClas::RunImplementation()
{
  // Setting the parameters
  TePDITypes::TePDIRasterVectorType input_rasters;
  params_.GetParameter("input_rasters", input_rasters);

  vector<int> bands;
  params_.GetParameter("bands", bands);

  W = input_rasters[0]->params().ncols_;
  H = input_rasters[0]->params().nlines_;
  N = 0;
  TOTAL_ATTRIBUTES = bands.size();
  params_.GetParameter("classes_to_find", M);
  
  if( params_.CheckParameter< int >( "sx" ) )
    params_.GetParameter("sx", SX);
  else
    SX = 1;
    
  if( params_.CheckParameter< int >( "sy" ) )
    params_.GetParameter("sy", SY);
  else
    SY = 1;
    
  if( params_.CheckParameter< int >( "max_iterations" ) )
    params_.GetParameter("max_iterations", MAX_ITERATIONS);
  else
    MAX_ITERATIONS = 3;
    
  if( params_.CheckParameter< double >( "epsilon" ) )
    params_.GetParameter("epsilon", EPSILON);
  else
    EPSILON = 15.0;
    
  if( params_.CheckParameter< double >( "shift_threshold" ) )
    params_.GetParameter("shift_threshold", SHIFT_THRESHOLD);
  else
    SHIFT_THRESHOLD = 40.0;

  bool no_means = true;
  TeSharedPtr<TeMatrix> input_means;
  if( params_.CheckParameter< TeSharedPtr<TeMatrix> >( "input_means" ) ) {
    params_.GetParameter("input_means", input_means);
    no_means = false;
  }

  for (int i = 0; i < W; i+=SX)
    for (int j = 0; j < H; j+=SY)
      N++;
  double  START_COVAR = 1000.0,
      *variations = (double *) malloc(M * sizeof(double));

  // Start the algorithm
  vector<TeMatrix>  covars, // covariance matrix
            old_covars, // to prevent problems in inverse function
            means, // mean vectors
            old_means, // to compare difference during the E-M process
            X, // image points
            original_X; // original image points (hole set)
  vector<double>  P; // Probability for each cluster
  double **P_C_x = (double **) malloc(M * sizeof(double)), // Probability a posteriori
      **original_P_C_x = (double **) malloc(M * sizeof(double));

  // Initializing data...
  for (int i = 0; i < M; i++)
  {
    P_C_x[i] = (double *) malloc(N * sizeof(double));
    original_P_C_x[i] = (double *) malloc(W * H * sizeof(double));
    for (int j = 0; j < N; j++)
      P_C_x[i][j] = 0.0;
    for (int j = 0; j < W * H; j++)
      original_P_C_x[i][j] = 0.0;
  }
  // initialize covariances, means and P
  TeMatrix tmp_covar;
  tmp_covar.Init(TOTAL_ATTRIBUTES, START_COVAR);
  TeMatrix tmp_mean;

  unsigned seed = (unsigned)time(0);
  srand(seed);
  for (int j = 0; j < M; j++)
  {
    double *random_means = (double *)malloc(TOTAL_ATTRIBUTES * sizeof(double));
    if (no_means)
    {
      for (int r = 0; r < TOTAL_ATTRIBUTES; r++)
        random_means[r] = rand() % 255;
    }
    else
    {
      for (int r = 0; r < TOTAL_ATTRIBUTES; r++)
        random_means[r] = input_means->operator()(r, j);
    }

    tmp_mean.Init(TOTAL_ATTRIBUTES, 1, random_means);
    means.push_back(tmp_mean);
    old_means.push_back(tmp_mean);
    covars.push_back(tmp_covar);
    old_covars.push_back(tmp_covar);
    P.push_back((double) 1 / M);
	free(random_means);
  }

  // Generates the attributes vector
  TeMatrix tmp_X;

  double *tmp = (double*) malloc(TOTAL_ATTRIBUTES * sizeof(double));
  for (int i = 0; i < W; i += SX)
    for (int j = 0; j < H; j += SY)
    {
      for (int b = 0; b < TOTAL_ATTRIBUTES; b++)
        input_rasters[b]->getElement(i, j, tmp[b], bands[b]);
      tmp_X.Init(TOTAL_ATTRIBUTES, 1, tmp);
      X.push_back(tmp_X);
    }

  TePDIPIManager p("Generating Attributes Vector", W, progress_enabled_);
  // Store the hole set of pixels
  for (int i = 0; i < W; i++)
  {
    for (int j = 0; j < H; j++)
    {
      for (int b = 0; b < TOTAL_ATTRIBUTES; b++)
        input_rasters[b]->getElement(i, j, tmp[b], bands[b]);

      tmp_X.Init(TOTAL_ATTRIBUTES, 1, tmp);
      original_X.push_back(tmp_X);
    }
    p.Increment();
  }
  p.Toggle(false);
  free(tmp);

  // Start to estimate the data
  int num_iterations = 0;
  bool stop_condition = false;
  TeMatrix  tmp_matrix,
        tmp_inv,
        tmp_trans,
        tmp_prod;

  TePDIPIManager p2("Estimating data", MAX_ITERATIONS, progress_enabled_);
  while (stop_condition == false)
  {
    // Calculate P_C_x
    for (int j = 0; j < M; j++)
    {
      for (int k = 0; k < N; k++)
      {
        double denom = 0.0;
        for (int k1 = 0; k1 < M; k1++)
        {
          tmp_matrix = X[k] - means[k1];
          if (!covars[k1].Inverse(tmp_inv))
          {
            covars[k1].Init(TOTAL_ATTRIBUTES, START_COVAR);
            covars[k1].Inverse(tmp_inv);
          }
          tmp_matrix.Transpose(tmp_trans);
          tmp_prod.Init();
          tmp_prod = tmp_trans * tmp_inv * tmp_matrix;

          denom += pow(covars[k1].Determinant(), -0.5) * exp(-0.5 * tmp_prod(0, 0)) * P[k1];
        }

        tmp_matrix = X[k] - means[j];


        if (!covars[j].Inverse(tmp_inv))
        {
          covars[j].Init(TOTAL_ATTRIBUTES, START_COVAR);
          covars[j].Inverse(tmp_inv);
        }

        tmp_matrix.Transpose(tmp_trans);
        tmp_prod = tmp_trans * tmp_inv * tmp_matrix;

        P_C_x[j][k] = pow(covars[j].Determinant(), -0.5) * exp(-0.5 * tmp_prod(0,0)) * P[j] / denom;
      }
    }

    // Update means, covariances and Probabilities
    for (int j = 0; j < M; j++)
    {
      double tmp_denom = 0.0;
      TeMatrix  tmp_num_mean,
            tmp_num_covar,
            tmp_sub;
      tmp_num_mean.Init(TOTAL_ATTRIBUTES, 1, 0.0);
      tmp_num_covar.Init(TOTAL_ATTRIBUTES, 0.0);

      for (int k = 1; k < N; k++)
      {
        // sum for mean numerator
        tmp_num_mean = tmp_num_mean + P_C_x[j][k] * X[k];
        // sum for both denominators
        tmp_denom += P_C_x[j][k];
        // sum for covar numerator
        tmp_matrix = X[k] - means[j];
        tmp_matrix.Transpose(tmp_trans);
        tmp_prod = tmp_matrix * tmp_trans;
        tmp_num_covar = tmp_num_covar + P_C_x[j][k] * tmp_prod;
      }

      means[j] =  (1 / tmp_denom) * tmp_num_mean;
      covars[j] = (1 / tmp_denom) * tmp_num_covar;
      P[j] = tmp_denom / N;

      tmp_sub = means[j] - old_means[j];
      tmp_sub.Transpose(tmp_trans);
      tmp_prod = tmp_trans * tmp_sub;
      variations[j] = sqrt(tmp_prod(0, 0));
    }

    TeMatrix  tmp_sub,
          shift;

    shift.Init(TOTAL_ATTRIBUTES, 1);
    for (int a = 0; a < TOTAL_ATTRIBUTES; a++)
      shift(a, 0) = rand() % 255;
    for (int j = 0; j < M; j++)
      for (int j1 = (j + 1); j1 < M; j1++)
      {
        tmp_sub = means[j] - means[j1];
        tmp_sub.Transpose(tmp_trans);
        tmp_prod = tmp_trans * tmp_sub;
        if (sqrt(tmp_prod(0, 0)) < SHIFT_THRESHOLD)
        {
          for (int a = 0; a < TOTAL_ATTRIBUTES; a++)
            shift(a, 0) = rand() % 255;

          means[j1] = shift;
          covars[j1] = tmp_covar;
        }
      }

    double variation = 0.0;
    for (int j = 0; j < M; j++)
      if (variations[j] > variation)
        variation = variations[j];

    old_means = means;
    old_covars = covars;
    if (++num_iterations >= MAX_ITERATIONS)
      stop_condition = true;
	p2.Increment();
    if (variation < EPSILON)
      stop_condition = true;
  }
  p2.Toggle(false);
  free(variations);

  // Classifying input image
  TePDIPIManager p3("Classifying input image", M, progress_enabled_);
  double *class_covars_determinant = (double *) malloc(M * sizeof(double));
  vector<TeMatrix> class_covars_inverse;
  for (int j = 0; j < M; j++)
  {
    class_covars_determinant[j] = covars[j].Determinant();
    covars[j].Inverse(tmp_inv);
    class_covars_inverse.push_back(tmp_inv);
  }
  for (int j = 0; j < M; j++)
  {
    for (int k = 0; k < H * W; k++)
    {
      double denom = 0.0;

      for (int k1 = 0; k1 < M; k1++)
      {
        tmp_matrix = original_X[k] - means[k1];
        tmp_matrix.Transpose(tmp_trans);
        tmp_prod = tmp_trans * class_covars_inverse[k1] * tmp_matrix;

        denom += pow(class_covars_determinant[k1], -0.5) * exp(-0.5 * tmp_prod(0, 0)) * P[k1];
      }

      tmp_matrix = original_X[k] - means[j];
      tmp_matrix.Transpose(tmp_trans);
      tmp_prod = tmp_trans * class_covars_inverse[j] * tmp_matrix;

      original_P_C_x[j][k] = pow(class_covars_determinant[j], -0.5) * exp(-0.5 * tmp_prod(0,0)) * P[j] / denom;
    }
    p3.Increment();
  }
  p3.Toggle(false);
  free(class_covars_determinant);

  // Setting the output raster 
  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter("output_raster", output_raster);

  TeRasterParams output_raster_params = output_raster->params();
  output_raster_params.setDataType( TeUNSIGNEDCHAR );
  output_raster_params.nBands( 1 );
  if( input_rasters[0]->projection() != 0 ) 
    output_raster_params.projection( input_rasters[0]->projection() );

  output_raster_params.boxResolution( input_rasters[0]->params().box().x1(), 
    input_rasters[0]->params().box().y1(), input_rasters[0]->params().box().x2(), 
    input_rasters[0]->params().box().y2(), input_rasters[0]->params().resx_, 
    input_rasters[0]->params().resy_ ); 

  // Lut
  output_raster_params.setPhotometric( TeRasterParams::TePallete );
  output_raster_params.lutr_.clear();
  output_raster_params.lutg_.clear();
  output_raster_params.lutb_.clear();

  vector<unsigned short> colors_R, colors_G, colors_B;
  colors_R.push_back(0); colors_G.push_back(0); colors_B.push_back(255);
  colors_R.push_back(0); colors_G.push_back(255); colors_B.push_back(0);
  colors_R.push_back(255); colors_G.push_back(0); colors_B.push_back(0);
  colors_R.push_back(0); colors_G.push_back(255); colors_B.push_back(255);
  colors_R.push_back(255); colors_G.push_back(0); colors_B.push_back(255);
  colors_R.push_back(255); colors_G.push_back(255); colors_B.push_back(0);
  colors_R.push_back(255); colors_G.push_back(255); colors_B.push_back(255);
  colors_R.push_back(0); colors_G.push_back(0); colors_B.push_back(0);
  colors_R.push_back(200); colors_G.push_back(0); colors_B.push_back(0);
  colors_R.push_back(0); colors_G.push_back(200); colors_B.push_back(0);
  colors_R.push_back(0); colors_G.push_back(0); colors_B.push_back(200);
  colors_R.push_back(0); colors_G.push_back(200); colors_B.push_back(200);
  colors_R.push_back(200); colors_G.push_back(200); colors_B.push_back(0);
  colors_R.push_back(200); colors_G.push_back(0); colors_G.push_back(200);
  colors_R.push_back(184); colors_G.push_back(115); colors_B.push_back(51);
  colors_R.push_back(217); colors_G.push_back(135); colors_B.push_back(25);
  colors_R.push_back(133); colors_G.push_back(99); colors_B.push_back(99);
  colors_R.push_back(217); colors_G.push_back(217); colors_B.push_back(25);
  colors_R.push_back(207); colors_G.push_back(181); colors_B.push_back(59);
  colors_R.push_back(205); colors_G.push_back(127); colors_B.push_back(50);
  colors_R.push_back(230); colors_G.push_back(232); colors_B.push_back(250);
  
  unsigned color_index = 0;
  for( int lut_index = 0 ; lut_index < M ; ++lut_index ) 
  {
    color_index = ( (unsigned) lut_index < colors_R.size() ? lut_index : 0 );
    output_raster_params.lutr_.push_back( (unsigned short) colors_R[color_index] );
    output_raster_params.lutg_.push_back( (unsigned short) colors_G[color_index] );
    output_raster_params.lutb_.push_back( (unsigned short) colors_B[color_index] );
  }

  TEAGN_TRUE_OR_RETURN( output_raster->init( output_raster_params ), "Output raster reset error" );
  output_raster->params().status_ = TeRasterParams::TeReadyToWrite;

  TePDIPIManager p4("Generating output image", H * W, progress_enabled_);
  int k = 0;
  for (int c = 0; c < W; c++)
    for (int l = 0; l < H; l++)
    {
      int  max = 0;
      
      for (int j = 0; j < M; j++)
        if (original_P_C_x[j][k] > original_P_C_x[max][k])
          max = j;
      output_raster->setElement(c, l, max);
      k++;
      p4.Increment();
    }
  p4.Toggle(false);
  free(original_P_C_x);
  free(P_C_x);

  return true;
}
