#include "TePDIPrincoMixModelStrategy.hpp"

TePDIPrincoMixModelStrategy::TePDIPrincoMixModelStrategy()
{
};

TePDIPrincoMixModelStrategy::~TePDIPrincoMixModelStrategy()
{
};

bool TePDIPrincoMixModelStrategy::CheckParameters(const TePDIParameters& parameters) const
{
// Components e spectralbands checking

  TePDIMixModelComponentList componentList;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("component_list", componentList), "Missing parameter: component_list");

  TePDIMixModelSpectralBandList spectralBandList;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("spectral_band_list", spectralBandList), "Missing parameter: spectral_band_list");

  TEAGN_TRUE_OR_RETURN(componentList.getSize() <= spectralBandList.getSize(), "Invalid parameter: componentsNumber greater than spectralBandsNumber");
  for (unsigned int nc = 0; nc < componentList.getSize(); nc++)
    TEAGN_TRUE_OR_RETURN(componentList.getComponentSize(nc) == 
      (int)spectralBandList.getSize(), 
      "Invalid parameter: components bands diferred from spectralBandsNumber");
    

// Input rasters and bands checking

  TePDITypes::TePDIRasterVectorType input_rasters;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");
  TEAGN_TRUE_OR_RETURN(input_rasters.size() > 1, "Invalid input rasters number");

  TEAGN_TRUE_OR_RETURN(input_rasters.size() == spectralBandList.getSize(), "Invalid parameter: input_raster number diferred from spectralBandsNumber");

  std::vector<int> bands;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("bands", bands), "Missing parameter: bands");
  TEAGN_TRUE_OR_RETURN(bands.size() == input_rasters.size(), Te2String(input_rasters.size()) + " Invalid parameter: bands number " + Te2String(bands.size()));

  for (unsigned int input_rasters_index = 0 ; input_rasters_index < input_rasters.size(); input_rasters_index++)
  {
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index].isActive(), "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " inactive");
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " not ready");
    TEAGN_TRUE_OR_RETURN(bands[input_rasters_index] < input_rasters[input_rasters_index]->nBands(), "Invalid parameter: bands[" + Te2String(input_rasters_index) + "]");
        TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().nlines_ == input_rasters[0]->params().nlines_, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " with imcompatible number of lines");
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().ncols_ == input_rasters[0]->params().ncols_, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " with imcompatible number of columns" );
  }
 
// Output rasters checking

  TePDITypes::TePDIRasterVectorType output_rasters;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_rasters", output_rasters), "Missing parameter: output_rasters");
  TEAGN_TRUE_OR_RETURN(output_rasters.size() > 1, "Invalid output rasters number");

// Compute error raster flag checking
  int computeErrorRasters;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("compute_error_rasters", computeErrorRasters), "Missing parameter: compute error rasters");
  if (computeErrorRasters)
  {
    TePDITypes::TePDIRasterVectorType output_error_rasters;
    TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_error_rasters", output_error_rasters), "Missing parameter: output_error_rasters");
    TEAGN_TRUE_OR_RETURN(output_error_rasters.size() > 1, "Invalid output error rasters number");
  }

// Normalize flag checking
  int normalize;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("normalize", normalize), "Missing parameter: normalize");
  TEAGN_TRUE_OR_RETURN((normalize >= 0) && (normalize <= 1) , "Inavlid parameter value: normalize");
  
  return true;
}

int SGaussElimination (TeMatrix& Z, std::vector<int>& line, unsigned int componentsNumber)
{
  int     i, j, k,
              aux;
  double  auxd, m;
 
  int nrows = Z.Nrow();

  // Verify matrix and vector sizes
  TEAGN_TRUE_OR_RETURN(nrows >= Z.Ncol(), "Matrix not square");
   TEAGN_TRUE_OR_RETURN( ((int)componentsNumber) >= nrows, "Vector Size");

  // Initialize changed lines indicator
  for (i = 0; i < nrows; i++)
          line[i] = i;

  // Diagonalization Process
  for (k = 0; k < nrows; k++)
  {
    i = k;
    while (Z(i, k) == 0.0)
    {
      TEAGN_TRUE_OR_RETURN(i != nrows-1, "Singular Matrix");
      i++;
    }

    if (k != i)
    {
      // Update changed lines indicator 
      aux = line[i];
      line[i] = line[k];              
      line[k] = aux;

      // Change lines 
      for (j=0; j < nrows; j ++)
      {
        auxd = Z(k,j);
        Z(k,j) = Z (i, j);  
        Z (i, j) = auxd;
      }
    }

    // Recompute lines i = k + 1,..., componentsNumber - 1 
    for (i = k + 1; i < nrows; i++)
    {
      m = Z(i, k)/Z(k, k);
      Z(i, k) = m;
      for (j = k + 1; j < nrows; j++)
        Z (i, j) = Z (i, j) - m * Z(k,j);
    }

       } // End of diagonalization process

  TEAGN_TRUE_OR_RETURN(Z(nrows-1, nrows-1) != 0.0, "Singular Matrix");

  return true;
}

int SFowardBackSubstitution (TeMatrix& Z, std::vector<double>& y, int ys, std::vector<int>& line, int lines, std::vector<double>& x, int xs)
{
  int  nrows = Z.Nrow(); 

  // Verify matrix and vector sizes
  TEAGN_TRUE_OR_RETURN(nrows >= Z.Ncol(), "Matriz not square");
  TEAGN_TRUE_OR_RETURN(!((lines < nrows) || (ys < nrows) || (xs < nrows)), "Vector Size");

  int k, j;
  double aux;
  std::vector<double> F(nrows, 0.0);

  // Foward substuitution 
  for (k = 0; k < nrows; k++)
  {
    aux = 0.;
    for (j = 0; j <= k - 1; j ++)
      aux = aux + Z(k,j) * F[j];
    F[k] = y[line[k]] - aux;
  }

  // Backward substitution  
  for (k= nrows - 1; k >= 0; k--)
  {
    aux = 0.;
    for (j = k + 1; j < nrows; j++)
      aux = aux + Z(k,j) * x[j];
    x[k] = (F[k] - aux)/Z(k,k);
  }

  return true;
} 

bool TePDIPrincoMixModelStrategy::Implementation(const TePDIParameters& params)
{
// Check components e spectralbands parameters
  TePDIMixModelComponentList componentList;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("component_list", componentList), "Missing parameter: component_list");

  TePDIMixModelSpectralBandList spectralBandList;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("spectral_band_list", spectralBandList), "Missing parameter: spectral_band_list");

  unsigned int  componentsNumber = componentList.getSize(),
      spectralBandsNumber = spectralBandList.getSize(),
      componentsNumberLessOne = componentsNumber - 1;

// Check input_rasters parameter
  TePDITypes::TePDIRasterVectorType input_rasters;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");
  
  std::vector<int> bands;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("bands", bands), 
    "Missing parameter: bands");

// Check output_rasters parameter
  TePDITypes::TePDIRasterVectorType output_rasters;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("output_rasters", output_rasters), "Missing parameter: output_rasters");

// Initialize output_rasters based on first input raster

  TeRasterParams base_raster_params = input_rasters[0]->params();
  base_raster_params.setDataType( TeDOUBLE, -1 );
  base_raster_params.nBands( 1 );
  
  for(unsigned int outrasterindex = 0; outrasterindex < output_rasters.size(); 
    ++outrasterindex )
  {
    TeRasterParams outRasterParams = output_rasters[outrasterindex]->params();
    outRasterParams.nBands( 1 );
    outRasterParams.boundingBoxLinesColumns(
      base_raster_params.boundingBox().x1(),
      base_raster_params.boundingBox().y1(),
      base_raster_params.boundingBox().x2(),
      base_raster_params.boundingBox().y2(),
      base_raster_params.nlines_,
      base_raster_params.ncols_,
      TeBox::TeUPPERLEFT );
    outRasterParams.projection( base_raster_params.projection() );
    
    TEAGN_TRUE_OR_THROW( 
      output_rasters[outrasterindex]->init( outRasterParams ), 
        "Output raster init error");
  }

// Check compute error raster parameter and outout error rasters if it's necessary
  int computeErrorRasters;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("compute_error_rasters", computeErrorRasters), "Missing parameter: compute_error_asters");
  TePDITypes::TePDIRasterVectorType output_error_rasters;
  if (computeErrorRasters)
  {
    TEAGN_TRUE_OR_RETURN(params.GetParameter("output_error_rasters", 
      output_error_rasters), "Missing parameter: output_error_rasters");
      
    for(unsigned int outrasterindex = 0; outrasterindex < 
      output_error_rasters.size(); ++outrasterindex )
    {
      TeRasterParams outRasterParams = 
        output_error_rasters[outrasterindex]->params();
      outRasterParams.nBands( 1 );
      outRasterParams.boundingBoxLinesColumns(
        base_raster_params.boundingBox().x1(),
        base_raster_params.boundingBox().y1(),
        base_raster_params.boundingBox().x2(),
        base_raster_params.boundingBox().y2(),
        base_raster_params.nlines_,
        base_raster_params.ncols_,
        TeBox::TeUPPERLEFT );
      outRasterParams.projection( base_raster_params.projection() );    
      
      TEAGN_TRUE_OR_THROW( 
        output_error_rasters[outrasterindex]->init( outRasterParams ), 
          "Output raster init error");      
    }
  }

// Check compute average error
  int computeAverageError;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("compute_average_error", computeAverageError), "Missing parameter: compute_average_error_");
  double averageError;
  if (computeAverageError)
    TEAGN_TRUE_OR_RETURN(params.GetParameter("average_error", averageError), "Missing parameter: average_error");

// Check normalize
  int normalize;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("normalize", normalize), "Missing parameter: normalize");

// Getting number of lines and columns from the first image
  int rastersLines = base_raster_params.nlines_;
  int rastersColumns = base_raster_params.ncols_;

// Indexes variables
  unsigned int  i,
      j,
      k,
      m;

// Buildind the SpectralBandsComponents Matrix base on input componentsList and spectralBandList
  TeMatrix SpectralBandsComponents;
  SpectralBandsComponents.Init(spectralBandsNumber, componentsNumber, 0.0);
  for(i = 0; i < spectralBandsNumber; i++)
    for (j = 0; j < componentsNumber; j++)
      SpectralBandsComponents(i, j) = componentList.getPixel(j, i);
//Printing
//cout << "SpectralBandsComponents" << endl;
//SpectralBandsComponents.Print();

//----- Part I : mathematical processing independend on image data -----//

// SpectralBandsComponentsTransposed stores SpectralBandsComponents and other things after
   TeMatrix SpectralBandsComponentsTransposed;
// Initializing SpectralBandsComponentsTransposed Matrix
  TEAGN_TRUE_OR_RETURN(SpectralBandsComponentsTransposed.Init(componentsNumber, spectralBandsNumber, 0.0), "Error initializing SpectralBandsComponentsTransposed Matrix");
// Initializing SpectralBandsComponentsTransposed Matrix = Transpose of SpectralBandsComponents
   TEAGN_TRUE_OR_RETURN(SpectralBandsComponents.Transpose(SpectralBandsComponentsTransposed), "Error transposing SpectralBandsComponents to SpectralBandsComponentsTransposed");

//Printing
//cout << "SpectralBandsComponentsTransposed - After transposing" << endl;
//SpectralBandsComponentsTransposed.Print();
 
// Compute componentsMean vector as the mean coefficient value per band and subtract the componentsMeanAfter from the coefficients matrix
// Creating two double vectors to store de means of the spectralBands of each component
   std::vector<double> componentsMean(spectralBandsNumber, 0.0),
     componentsMeanAfter(spectralBandsNumber, 0.0);
  for (j = 0; j < spectralBandsNumber; j++)
  {
// Compute original mean (componentsMean)
    componentsMean[j] = 0;
    for(i = 0; i < componentsNumber; i++)
      componentsMean[j] = componentsMean[j] + SpectralBandsComponentsTransposed(i,j);
    componentsMean[j] = componentsMean[j] / (double)componentsNumber;

// Subtract mean from SpectralBandsComponents and compute new matrix mean (componentsMeanAfter)
    componentsMeanAfter[j] = 0;
    for(i = 0; i < componentsNumber; i++)
    {
      SpectralBandsComponentsTransposed(i,j) = SpectralBandsComponentsTransposed(i,j) - componentsMean[j];
      componentsMeanAfter[j] = componentsMeanAfter[j] + SpectralBandsComponentsTransposed(i,j);
    }
    componentsMeanAfter[j] = componentsMeanAfter[j]/ (double)componentsNumber;
  }

//Printing
//cout << "SpectralBandsComponentsTransposed - After less mean" << endl;
//SpectralBandsComponentsTransposed.Print();

// Compute covarianceVector vector
  std::vector<double> covarianceVector(spectralBandsNumber*spectralBandsNumber, 0.0);
  for(k = 0; k < spectralBandsNumber*spectralBandsNumber; k++)
    covarianceVector[k] = 0;
   k = 0 ;
   for(j = 0; j < spectralBandsNumber; j++)
   {
     for(m = 0; m < j + 1; m++)
     {
       for(i = 0; i < componentsNumber; i++)
         covarianceVector[k] = covarianceVector[k] + (SpectralBandsComponentsTransposed(i, j) - componentsMeanAfter[j]) * (SpectralBandsComponentsTransposed(i, m) - componentsMeanAfter[m]);
       covarianceVector[k] = covarianceVector[k]/(double)componentsNumber ;
       k++;
     }
   }
// Printing
//  for(k = 0; k < spectralBandsNumber*spectralBandsNumber; k++)
//  {
//    cout << covarianceVector[k] << " ";
//  }

   TeMatrix covarianceMatrix;
  TEAGN_TRUE_OR_RETURN(covarianceMatrix.Init(spectralBandsNumber, spectralBandsNumber, 0.0), "Error initializing covarianceMatrix");

   k = 0;
   for (i = 0; i< spectralBandsNumber; i++)
     for (j = 0; j <= i; j++)
       covarianceMatrix(i, j) = covarianceVector[k++];
// Printing
//cout << "covarianceMatrix" << endl;
//covarianceMatrix.Print();

// Compute eigenvectors : results aux
   TeMatrix auxMatrix;
   TEAGN_TRUE_OR_RETURN(covarianceMatrix.EigenVectors(auxMatrix), "Error in eigenvectors calcule of auxMatrix")

// Printing
//cout << "auxMatrix - eigenvectors of covarianceMatrix" << endl;
//auxMatrix.Print();

// Keep only significant eigenvectors (componentsNumber - 1): results eigenreducted
   TeMatrix eigenreducted;
   TEAGN_TRUE_OR_RETURN(eigenreducted.Init(spectralBandsNumber, componentsNumberLessOne, 0.0), "Error initializing eigenreducted matrix")
   for (j = 0; j <  spectralBandsNumber; j++)
     for (i = 0; i <  componentsNumberLessOne; i++)
       eigenreducted(j, i) = auxMatrix(j, i);

//Printing
//cout << "eigenreducted" << endl;
//eigenreducted.Print();

// Clear auxMatrix for future use
   auxMatrix.Clear(); 

// Rotate SpectralBandsComponentsTransposed to PCA space;  result SpectralBandsComponentsPCA
  TeMatrix SpectralBandsComponentsPCA;
  TEAGN_TRUE_OR_RETURN(SpectralBandsComponentsPCA.Init(SpectralBandsComponentsTransposed.Nrow(), eigenreducted.Ncol()), "Error initializing SpectralBandsComponentsPCA");
 
  SpectralBandsComponentsPCA = SpectralBandsComponentsTransposed * eigenreducted;

// Printing
//cout << "SpectralBandsComponentsPCA - Rotate SpectralBandsComponentsTransposed to PCA space" << endl;
//SpectralBandsComponentsPCA.Print();

// Clear SpectralBandsComponentsTransposed: it will not be used bellow
  SpectralBandsComponentsTransposed.Clear();
 
// Initialize one more column to SpectralBandsComponentsPCA to incorporate sum restriction to equations; results auxMatrix
  TEAGN_TRUE_OR_RETURN(auxMatrix.Init(SpectralBandsComponentsPCA.Nrow(), SpectralBandsComponentsPCA.Ncol() + 1), "Error initializing auxMatrix");
 
  for (j = 0; j < ((unsigned int)SpectralBandsComponentsPCA.Nrow()); j++)
  {
    for (i = 0; i <  ((unsigned int)SpectralBandsComponentsPCA.Ncol() ); i++)
      auxMatrix(j, i) = SpectralBandsComponentsPCA(j, i);
    auxMatrix (j, SpectralBandsComponentsPCA.Ncol()) = 1.0;
  }
// Printing
//cout << "Initialize one more column to SpectralBandsComponentsPCA to incorporate sum restriction to equation" << endl;
//auxMatrix.Print();

// Clear SpectralBandsComponentsPCA for future use
  SpectralBandsComponentsPCA.Clear(); 
 
// Transpose auxMatrix; results SpectralBandsComponentsPCA
  TEAGN_TRUE_OR_RETURN(auxMatrix.Transpose(SpectralBandsComponentsPCA), "Error transposing auxMatrix to SpectralBandsComponentsPCA");

// Printing
//cout << "Transpose matrix SpectralBandsComponentsPCA" << endl;
//SpectralBandsComponentsPCA.Print();

// Clear auxMatrix for future use
  auxMatrix.Clear();
 
// Invert matrix SpectralBandsComponentsPCA; results SpectralBandsComponentsPCA
  std::vector<int> lines(componentsNumber, 0);

   TEAGN_TRUE_OR_RETURN(SGaussElimination(SpectralBandsComponentsPCA, lines, componentsNumber), "Error in gauss elimination");

// Printing
//cout << "Invert matrix SpectralBandsComponentsPCA" << endl;
//SpectralBandsComponentsPCA.Print();

//----- Part II : mathematical processing dependend on image data -----//
 
// Initialize matrixes that will help to prepare vector Y
  TEAGN_TRUE_OR_RETURN(auxMatrix.Init(1, spectralBandsNumber, 0.0), "Error initializing auxMatrix");
 
  TeMatrix ymat;
  TEAGN_TRUE_OR_RETURN(ymat.Init(componentsNumberLessOne, componentsNumberLessOne, 0.0), "Error initializing ymat");
 
// Initialize proportion vector X and image dependent values for the linear equations Y  
  std::vector<double>  x(componentsNumber, 0.0),
    y(componentsNumber, 0.0);
  for (i = 0; i < componentsNumber; i++)
  {
    x[i] = 0.0;
    y[i] = 0.0;
  }

// Initialize current line and column    
  std::vector<int> colin(spectralBandsNumber, 0);
  int  colout,
// Auxiliates the transformation of proportions to [0, 255]
    prop;

// It's should be class variables
  std::vector<int> linesIn(spectralBandsNumber, 0),
    colsIn(spectralBandsNumber, 0);

// It's should be class variables
  TeMatrix  rasterLinesIn,
			rasterLinesOut;
  rasterLinesIn.Init(spectralBandsNumber, rastersColumns, 0.0);
  rasterLinesOut.Init(componentsNumber, rastersColumns, 0.0);
  std::vector<double> spectralBandsError(spectralBandsNumber, 0.0);

// Initialization of variables declared above
  for (i = 0; i < spectralBandsNumber; i++)
  {
    colin[i] = 0;
    linesIn[i] = 0;
    colsIn[i] = 0;
    spectralBandsError[i] = 0.0;
  }

// Start computing proportions for each line lines
  TePDIPIManager progress( "Computing MixModel proportions", rastersLines,
    progress_interface_enabled_ );

  for (int linout = 0; linout < rastersLines;  linout++)
  {
    // Read input line for each band
    for (i = 0; i < spectralBandsNumber; i++)
    {
      for (j = 0; j < ( (unsigned int)rastersColumns ); j++)
      {
        double p;
        input_rasters[i]->getElement(j, linesIn[i], p, bands[ i ] );
        rasterLinesIn(i, j) = p;
      }
      // Update next line to be read for band i
      linesIn[i]++;

      // Reinitialize first column for band i
      colin[i] = colsIn[i];
    }

    // Compute proportions for each column
           for (colout = 0; colout < rastersColumns; colout++)
    {
      // Prepare y
      for (i = 0; i < spectralBandsNumber; i++)
        auxMatrix(0,i) = (double) (rasterLinesIn(i, colin[i]))/255. - componentsMean[i];
 
      ymat = auxMatrix * eigenreducted;
    
      for (i = 0; i < componentsNumber - 1; i++)
        y[i] = ymat(0,i);
      
      // Compute proportions
      SFowardBackSubstitution(SpectralBandsComponentsPCA, y, componentsNumber, lines, componentsNumber, x, componentsNumber);
 
      // Store proportion in buffers  
      for (i = 0; i < componentsNumber; i++)
      {
        prop = (short) (x[i]*100 + 100);
        rasterLinesOut(i, colout) = (unsigned char)prop;
      }
 
//----- It's should be a function (ComputeErrors.begin) -----//

      double   aux, error;
      // Verifify if it is necessary to compute the error
      if (computeErrorRasters || computeAverageError)
      {
        for (i = 0; i < spectralBandsNumber; i++)
        {
          // Compute digital value from the proportions
          aux = 0.0;
          for (j = 0; j < componentsNumber; j++)
            aux += x[j] * SpectralBandsComponents(i,j);
      
          // Compute error as module of the difference between the original value and aux
          error = (long)(rasterLinesIn(i, colin[i]))/255.0 - aux;
          if (error < 0)
            error = -1 * error;
          if (computeErrorRasters)
            rasterLinesIn(i, colout) = error * 255.0;
          if (computeAverageError)
            spectralBandsError[i] += error*255.;
        }
      }

//----- It's should be a function (ComputeErrors.end) -----//
      // Update current column number
      for (i = 0; i < spectralBandsNumber; i++)
        colin[i]++;
 
     } // End of column processing
 
    // Write processed line to disk
    for (i = 0; i < componentsNumber; i++)
      for (j = 0; j < ((unsigned int)rastersColumns); j++)
        output_rasters[i]->setElement(j, linout, rasterLinesOut(i, j));


//----- It's should be a function (StoreErrorRasters.begin) -----//
    // Store output error rasters
    if (computeErrorRasters)
    {
      // Write the output error images lines
      for (i = 0; i < componentsNumber; i++)
        for (j = 0; j < ((unsigned int)rastersColumns); j++)
                  output_error_rasters[i]->setElement(j, linout, rasterLinesIn(i, j));
    }
//----- It's should be a function (StoreErrorRasters.end) -----//

    progress.Increment();
  } // End of line processing
  
  progress.Toggle( false );

//----- It's should be a function (ComputeAverageError.begin) -----//
// Verifify if it is necessary to compute the error
  averageError = 0.0;
  if (computeAverageError)
  {
    // Compute total number of pixels in the output image
    int numpix = rastersLines*rastersColumns;
    for (i = 0; i < spectralBandsNumber; i++)
    {
      // Compute avarege band error taking accumulated band error 
      spectralBandsError[i] = spectralBandsError[i]/numpix;
      // Accumulate avarege error
      averageError += spectralBandsError[i];
    }
    // Compute total error taking the accumulated average error
    if (!(spectralBandsNumber == 0))
    {
      averageError = averageError/spectralBandsNumber;
    }
// Printing
//    cout << endl << averageError << endl;
  }
// Printing
//  for(k = 0; k < spectralBandsNumber; k++)
//  {
//    cout << spectralBandsError[k] << " ";
//  }
//----- It's should be a function (ComputeAverageError.end) -----//

  return true;
}
