#include "TePDIHants.hpp"

#include "TePDITypes.hpp"
#include "TePDIUtils.hpp"

#include <string>
#include <math.h>
#include <float.h>

#ifdef TE_USE_FFTW
  #include "fftw3.h"
#endif

TePDIHants::TePDIHants()
{
}


TePDIHants::~TePDIHants()
{
}


void TePDIHants::ResetState( const TePDIParameters& )
{
}

#ifdef TE_USE_FFTW
  // auxiliary globals for FFTW
  fftw_plan pf, pb; // forward (fft),  backward (ifft)
  fftw_complex *in, *out;

  // Hants parameters
  int *freqs;
  int nfreqs;
  double *range;
  double FET;
  int TAT, iMAX;

  // auxiliaries for Hants algorithm
  int *tmpvint, *tmpvint2;
  double *tmpvdouble, *tmpvdouble2, *filter, *components;

  // data size: number of lines, columns and bands (time-series size)
  int nl, nc, nb;

  void double2fftwcomplex(int n, double* in, fftw_complex* out) {
      int i;
      for (i = 0; i < n; i++) {
          out[i][0] = in[i];    //real
          out[i][1] = 0;  //imaginary
      }
  }
  void fftwcomplex2double(int n, fftw_complex* in, double* out) {
      int i;
      for (i = 0; i < n; i++) {
          out[i] = in[i][0];    //real
      }
  }
  void fftwcomplex2double2(int n, fftw_complex* in, double* out) {
      int i;
      for (i = 0; i < n; i++) {
          out[2*i] = in[i][0];    //real
          out[2*i+1] = in[i][1];  //imaginary
      }
  }

  void initGlobals() {
      int n = nb;
      // allocate memory for FFTW
      in = (fftw_complex*) fftw_malloc(n * sizeof(fftw_complex));
      out = (fftw_complex*) fftw_malloc(n * sizeof(fftw_complex));
      pf = fftw_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
      pb = fftw_plan_dft_1d(n, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

      // auxiliaries
      tmpvint = new int[n];
      tmpvint2 = new int[n];
      tmpvdouble = new double[n];
      tmpvdouble2 = new double[n];

      // harmonics of the selected frequencies after performing hants algorithm
      components = new double[2*nfreqs];
      
      // create filter based on specified frequencies
      filter = new double[n];
      int *distf = tmpvint;
      distf[0] = 0;
      int i, j;
      for (i = 1; i <= n/2; i++) {
          distf[i] = distf[n-i] = i;
      }
      for (i = 0; i < n; i++)
          filter[i] = 0;
      for (i = 0; i < nfreqs; i++)
          for (j = 0; j < n; j++)
              if (distf[j] == freqs[i])
                  filter[j] = 1;
  }
  void freeGlobals() {
      fftw_destroy_plan(pf);
      fftw_destroy_plan(pb);
      fftw_free(in); 
      fftw_free(out);

      delete [] tmpvint;
      delete [] tmpvint2;
      delete [] tmpvdouble;
      delete [] tmpvdouble2;
      delete [] filter;
      delete [] components;
  }

  double* hants(double *data, double *smNDVI) {
      int n = nb;
  //printf("data:"); printd(n, data);
      // (1) filter data based on specified frequencies
      double2fftwcomplex(n, data, in);
      fftw_execute(pf);
      fftw_complex *datafft = in;
      for (int i = 0; i < n; i++) {
          datafft[i][0] = (out[i][0]*filter[i])/(double)(n);
          datafft[i][1] = (out[i][1]*filter[i])/(double)(n);
      }
  //printf("datafft*filter:");printc(n, datafft);
      fftw_execute(pb);
      fftw_complex *data_back = out;
  //printf("data_back:");printc(n, data_back);

  //printf("bad_index: ");
      int totbad = 0;
      int *bad_index = tmpvint;
      // (2) search for out-of-range points and overwrite with reconstructed points
      for (int i = 0; i < n; i++) 
          if (data[i] < range[0] || data[i] > range[1]) {
              data[i] = data_back[i][0];
              bad_index[totbad] = i;
              totbad++;
  //printf(" %d",i);
          }
  //printf("\ndata:");printd(n, data);
      
      // (3) loop until all points are within FET tolerance
      //     or the maximum nr of points that may be discarded is reached
      for(int i = 0;;i++) {
  //printf("i=%d\n",i);
  //printf("data:"); printd(n, data);
          double2fftwcomplex(n, data, in);
          fftw_execute(pf);
          fftw_complex *datafft = in;
          for (int j = 0; j < n; j++) {
              datafft[j][0] = (out[j][0]*filter[j])/(double)(n);
              datafft[j][1] = (out[j][1]*filter[j])/(double)(n);
          }
  //printf("datafft*filter:");printc(n, datafft);
          fftw_execute(pb);
          fftw_complex *data_back = out;
  //printf("data_back:");printc(n, data_back);
          // find elements which deviate more than the FET value
          for (int j = 0; j < n; j++) {
              if (data[j] <= fabs(data_back[j][0]) - FET) {
                  // overwrite corrupted data points with reconstructed points
                  data[j] = fabs(data_back[j][0]);
                  bad_index[totbad] = j;
                  totbad++;
              }
          }
  //printf("bad_index: "); printi(totbad, bad_index);
      for (int j = 0; j < totbad; j++) 
          data[bad_index[j]] = fabs(data_back[bad_index[j]][0]);
  //printf("data:"); printd(n, data);
          if (totbad <= 0 || totbad >= TAT || i >= iMAX)
              break;
      }

      int *good_index = tmpvint2;
      for (int i = 0; i < n; i++)
          good_index[i] = 1;
      for (int j = 0; j < totbad; j++)
          good_index[bad_index[j]] = 0;
      double accumgood = 0;
      int totgood = 0;
      for (int i = 0; i < n; i++)
          if (good_index[i]) {
              accumgood = accumgood + data[i];
              totgood++;
          }
      double avgNDVI = accumgood/(double)totgood;

      // (4) Loop until the profile stabilizes without searching for corrupt data points.
      // Update the points that have been marked as 'bad_data' after each inverse
      // transform and repeat this until the shift in the data is below 1% of the
      // average NDVI value.
      if (totgood > 0) {
          for(int i = 0;;i++) {
  //printf("\nj=%d\n",i);
  //printf("data:"); printd(n, data);
              double2fftwcomplex(n, data, in);
              fftw_execute(pf);
              fftw_complex *datafft = in;
              for (int j = 0; j < n; j++) {
                  datafft[j][0] = (out[j][0]*filter[j])/(double)(n);
                  datafft[j][1] = (out[j][1]*filter[j])/(double)(n);
              }
              // multiply the FFT coeffients by two in order to have the correct amplitudes, except for the central (zero) harmonic
              components[0] = out[freqs[0]][0]/(double)(n);
              components[1] = out[freqs[0]][1]/(double)(n);
              for (int j = 1; j < nfreqs; j++) {
                  components[2*j] = (out[freqs[j]][0]/(double)(n))*2.0;
                  components[2*j+1] = (out[freqs[j]][1]/(double)(n))*2.0;
              }
  //printf("datafft*filter:");printc(n, datafft);
              fftw_execute(pb);
              fftw_complex *data_back = out;
  //printf("data_back:");printc(n, data_back);

              // calculated difference between previous and current loop
              double diff = 0;
              if (totbad > 0)
                  for (int j = 0; j < totbad; j++) {
                      diff = diff + fabs(data[bad_index[j]]-fabs(data_back[bad_index[j]][0]));
                      // overwrite corrupted data points detected in previous loop with reconstructed points
                      data[bad_index[j]] = fabs(data_back[bad_index[j]][0]);
                  }
  //printf("diff=%g    (avgNDVI/100.0=%g)\n",diff,avgNDVI/100.0);
  //printf("bad_index: "); printi(totbad, bad_index);
  //printf("data:"); printd(n, data);
              if (diff < avgNDVI/100.0 || i >= iMAX-1) {
                  break;
              }
          }
      }
      
  //printf("components:");printd2(nfreqs, components);
      fftwcomplex2double(n, data_back, smNDVI);
  //printf("smNDVI:\n");printd(n, smNDVI);
      return components;
  }

  // inRaster: input NDVI values; the size of the time series is represented by the number of bands
  // mask: 1 = do perform, 0 = do not perform algorithm on current position/pixel
  // outRaster: smoothed NDVI values
  // components: output raster for selected FFT components
  void hants(TePDITypes::TePDIRasterPtrType inRaster, TePDITypes::TePDIRasterPtrType maskRaster, TePDITypes::TePDIRasterPtrType smNDVIRaster, TePDITypes::TePDIRasterPtrType compRaster) {
      initGlobals();
      double *data = tmpvdouble;
      double *smNDVI = tmpvdouble2;
      double *fftComponents;
      double current_level;
      int i, j, k;
      for (i=0; i < nl; i++)
          for (j=0; j < nc; j++) {
              maskRaster->getElement( j, i, current_level, 0 );
              int perform = (int)current_level;
              if (perform) {
                  for (k=0; k < nb; k++) {
                      inRaster->getElement( j, i, current_level, k );
                      data[k] = current_level;
                  }
                  fftComponents = hants(data,smNDVI);
                  for (k=0; k < nb; k++) {
                      current_level = smNDVI[k];
                      smNDVIRaster->setElement( j, i, current_level, k );
                  }
                  for (k=0; k < 2*nfreqs; k++) {
                      current_level = fftComponents[k];
                      compRaster->setElement( j, i, current_level, k );
                  }
              }
              if ((i*nc+j) % (int)((nl*nc)/10) == 0)
                  printf("%d%%\n",(((i*nc+j)/((int)((nl*nc)/10)))+1)*10);
          }
      freeGlobals();
  }
#endif

bool TePDIHants::CheckParameters( const TePDIParameters& parameters ) const
{
  #ifdef TE_USE_FFTW
    /* Checking input raster */
    TePDITypes::TePDIRasterPtrType input_raster;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_raster", 
      input_raster ),
      "Missing parameter: input_raster" );
    TEAGN_TRUE_OR_RETURN( input_raster.isActive(),
      "Invalid parameter: input_raster inactive" );
    TEAGN_TRUE_OR_RETURN( input_raster->params().status_ != 
      TeRasterParams::TeNotReady, 
      "Invalid parameter: input_raster not ready" );

    /* Checking output_raster */
    TePDITypes::TePDIRasterPtrType output_raster;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "output_raster", 
      output_raster ),
      "Missing parameter: output_raster" );
    TEAGN_TRUE_OR_RETURN( output_raster.isActive(),
      "Invalid parameter: output_raster inactive" );
    TEAGN_TRUE_OR_RETURN( output_raster->params().status_ != 
      TeRasterParams::TeNotReady, 
      "Invalid parameter: output_raster not ready" );    

    /* checking time-series size */
    TEAGN_TRUE_OR_RETURN(
      (input_raster->nBands() > 1),
      "Invalid number of channels: input_raster" );
    TEAGN_TRUE_OR_RETURN(
      (output_raster->nBands() == input_raster->nBands()),
      "Invalid number of channels: output_raster" );

    /* checking image dimensions */
    int ih = input_raster->params().nlines_;
    int iw = input_raster->params().ncols_;
    int oh = output_raster->params().nlines_;
    int ow = output_raster->params().ncols_;
    TEAGN_TRUE_OR_RETURN(
      (ih == oh && iw == ow),
      "Invalid image dimensions: input_raster and output_raster with different sizes" );

    TePDITypes::TePDIRasterPtrType mask_raster;
    if (params_.GetParameter( "mask_raster", mask_raster )) {
        int mh = mask_raster->params().nlines_;
        int mw = mask_raster->params().ncols_;
        TEAGN_TRUE_OR_RETURN(
          (ih == mh && iw == mw),
          "Invalid image dimensions: mask_raster and input_raster with different sizes" );
    }
    TePDITypes::TePDIRasterPtrType components_raster;
    if (params_.GetParameter( "components_raster", components_raster )) {
        int ch = components_raster->params().nlines_;
        int cw = components_raster->params().ncols_;
        TEAGN_TRUE_OR_RETURN(
          (ih == ch && iw == cw),
          "Invalid image dimensions: components_raster and input_raster with different sizes" );
//        int nb = components_raster->params().nBands();
    }

    return true;
  #else
    return false;
  #endif
}

bool TePDIHants::RunImplementation()
{
  #ifdef TE_USE_FFTW
    // default Hants parameters
    int tmpvi[] = {0,1,2,3};
    freqs = tmpvi;
    nfreqs = 4;
    double tmpvd[] = { 1000,10000 };
    range = tmpvd;
    FET = 1500;
    TAT=1;
    iMAX=15;

    /* Retrieves parameters: input and output rasters, and Hants parameters */
    TePDITypes::TePDIRasterPtrType input_raster;
    params_.GetParameter( "input_raster", input_raster );
    TePDITypes::TePDIRasterPtrType output_raster;
    params_.GetParameter( "output_raster", output_raster );
    
    nl = input_raster->params().nlines_;
    nc = input_raster->params().ncols_;
    nb = input_raster->params().nBands();

    int *tmpvi2;
    if (params_.GetParameter( "freqs", tmpvi2 ))
        freqs = tmpvi2;
    int tmpi;
    if (params_.GetParameter( "nfreqs", tmpi ))
        nfreqs = tmpi;
    double *tmpvd2;
    if (params_.GetParameter( "range", tmpvd2 ))
        range = tmpvd2;
    double tmpd;
    if (params_.GetParameter( "FET", tmpd ))
        FET = tmpd;
    if (params_.GetParameter( "TAT", tmpi ))
        TAT = tmpi;
    if (params_.GetParameter( "iMAX", tmpi ))
        iMAX = tmpi;

    // if mask is not specified, we assume all 1's to process all the pixels in the data/input raster
    int i, j;
    TePDITypes::TePDIRasterPtrType mask_raster;
    if (!params_.GetParameter( "mask_raster", mask_raster )) {
        TeRasterParams r_params;
        r_params.nBands( 1 );
        r_params.setDataType( TeSHORT );
        r_params.boundingBoxLinesColumns( 0, 0, nl, nc, nl, nc ); 
        TePDIUtils::TeAllocRAMRaster( mask_raster, r_params, TePDIUtils::TePDIUtilsRamMemPol );
        for (i=0; i < nl; i++)
            for (j=0; j < nc; j++)
                mask_raster->setElement( j, i, 1.0, 0 );
    }

    // output raster for selected FFT components
    TePDITypes::TePDIRasterPtrType components_raster;
    if (!params_.GetParameter( "components_raster", components_raster )) {
        TeRasterParams r_params;
        r_params.nBands( 2*nfreqs );
        r_params.setDataType( TeDOUBLE );
        r_params.boundingBoxLinesColumns( 0, 0, nl, nc, nl, nc ); 
        TePDIUtils::TeAllocRAMRaster( components_raster, r_params, TePDIUtils::TePDIUtilsRamMemPol );
    }
    hants(input_raster, mask_raster, output_raster, components_raster);
    
    return true;
  #else
    return false;
  #endif
}

