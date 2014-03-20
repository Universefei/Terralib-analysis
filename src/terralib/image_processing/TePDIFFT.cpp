#include "TePDIFFT.hpp"
#include "TePDITypes.hpp"
#include "TePDIUtils.hpp"

#include <string>
#include <math.h>
#include <float.h>

#ifdef TE_USE_FFTW
  #include "fftw3.h"
#endif

TePDIFFT::TePDIFFT()
{
}


TePDIFFT::~TePDIFFT()
{
}


void TePDIFFT::ResetState( const TePDIParameters& )
{
}


bool TePDIFFT::CheckParameters( const TePDIParameters& parameters ) const
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

    /* checking: 2 channels to store real and imaginary parts of a complex number */
    int numchannels = 2;
    TEAGN_TRUE_OR_RETURN(
      (input_raster->nBands() >= numchannels),
      "Invalid number of channels: input_raster" );
    TEAGN_TRUE_OR_RETURN(
      (output_raster->nBands() >= numchannels),
      "Invalid number of channels: output_raster" );

    /* checking image dimensions */
    int ih = input_raster->params().nlines_;
    int iw = input_raster->params().ncols_;
    int oh = input_raster->params().nlines_;
    int ow = input_raster->params().ncols_;
    TEAGN_TRUE_OR_RETURN(
      (ih == oh && iw == ow),
      "Invalid image dimensions: input_raster and output_raster with different sizes" );

    return true;
  #else
    return false;
  #endif
}


bool TePDIFFT::RunImplementation()
{
  #ifdef TE_USE_FFTW
    /* Retrieves parameters: input and output rasters, fft (+1) or ifft (-1) flags */
    TePDITypes::TePDIRasterPtrType input_raster;
    params_.GetParameter( "input_raster", input_raster );
    TePDITypes::TePDIRasterPtrType output_raster;
    params_.GetParameter( "output_raster", output_raster );
    int fft_ifft_flag = FFTW_FORWARD, tmp = 0;
    if (params_.GetParameter( "fft_ifft_flag", tmp ))
        if (tmp < 0)
            fft_ifft_flag = FFTW_BACKWARD;
    

    /* allocates memory for FFTW, allowing fast memory access */
    int m = input_raster->params().nlines_;
    int n = input_raster->params().ncols_;
    fftw_complex *in = (fftw_complex*) fftw_malloc(m*n * sizeof(fftw_complex));
    fftw_complex *out = (fftw_complex*) fftw_malloc(m*n * sizeof(fftw_complex));

    /* copies input data for fast memory access by FFTW */
    int i, j, c;
    double current_level;
    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            for (c = 0; c < 2; c++) {
                input_raster->getElement( j, i, current_level, c );
                in[(i*n)+j][c] = current_level;
            }

    /* executes FFTW */
    fftw_plan p = fftw_plan_dft_2d(m, n, in, out, fft_ifft_flag, FFTW_ESTIMATE);
    fftw_execute(p);

    /* copies result to output raster */
    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++) 
            for (c = 0; c < 2; c++) {
                if (fft_ifft_flag == FFTW_BACKWARD)
                    out[(i*n)+j][c] = out[(i*n)+j][c] / (double)(m*n);
                current_level = out[(i*n)+j][c];
                output_raster->setElement( j, i, current_level, c );
            }
            
    /* deallocates memory */
    fftw_destroy_plan(p);
    fftw_free(in); 
    fftw_free(out);

    return true;
  #else
    return false;
  #endif
}

