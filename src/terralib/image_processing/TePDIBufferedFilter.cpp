#include "TePDIBufferedFilter.hpp"

#include "../kernel/TeAgnostic.h"

TePDIBufferedFilter::TePDIBufferedFilter()
{
  init_conv_buf();
  init_maskmatrix();
}


TePDIBufferedFilter::~TePDIBufferedFilter()
{
  reset_conv_buf( 0, 0 );
  TePDIFilterMask::deleteWeightsMatrix( temp_maskmatrix_, temp_maskmatrix_lines_ );
}


void TePDIBufferedFilter::ResetState( const TePDIParameters& params )
{
  TePDIParameters dummy_params = params;

  reset_conv_buf( 0, 0 );
  TePDIFilterMask::deleteWeightsMatrix( temp_maskmatrix_, temp_maskmatrix_lines_ );
}


void TePDIBufferedFilter::init_conv_buf()
{
  conv_buf_ = 0;
  conv_buf_lines_ = 0;
  conv_buf_columns_ = 0;
}


void TePDIBufferedFilter::init_maskmatrix()
{
  temp_maskmatrix_ = 0;
  temp_maskmatrix_lines_ = 0;
  temp_maskmatrix_columns_ = 0;
}


void TePDIBufferedFilter::reset_conv_buf( unsigned int lines, unsigned int columns )
{
  if( conv_buf_ != 0 ) {
    for( unsigned int line = 0 ; line < conv_buf_lines_ ; ++line ) {
      delete[] conv_buf_[ line ];
    }

    delete[] conv_buf_;

    conv_buf_ = 0;
    conv_buf_lines_ = 0;
    conv_buf_columns_ = 0;
  }

  if( ( lines > 0 ) && ( columns > 0 ) ) {
    conv_buf_ = new double*[ lines ];

    TEAGN_TRUE_OR_THROW( conv_buf_ != 0, "Memory allocation error" );

    for( unsigned int line = 0 ; line < lines ; ++line ) {
      conv_buf_[ line ] = new double[ columns ];

      TEAGN_TRUE_OR_THROW( conv_buf_[ line ] != 0, "Memory allocation error" );
    }

    conv_buf_lines_ = lines;
    conv_buf_columns_ = columns;
  }
}


void TePDIBufferedFilter::up_conv_buf( TePDITypes::TePDIRasterPtrType& inRaster,
  unsigned int line, unsigned int band )
{
  TEAGN_DEBUG_CONDITION( inRaster.isActive(), "Inactive pointer" );
  
  TeRaster* inRaster_nptr = inRaster.nakedPointer();

  TEAGN_DEBUG_CONDITION( ( inRaster_nptr->params().nlines_ > (int)line ),
    "Trying to get a non existent line from raster" );
  TEAGN_DEBUG_CONDITION( ( inRaster_nptr->params().ncols_ == 
    (int)conv_buf_columns_ ),
    "Buffer columns number not equal to raster columns" );
  TEAGN_DEBUG_CONDITION( ( (int)band < inRaster_nptr->params().nBands() ),
    "Invalid Band" );
    
  bool dummy_used = inRaster_nptr->params().useDummy_;
  double dummy_value = 0;
  if( dummy_used ) {
    dummy_value = inRaster_nptr->params().dummy_[ band ];
  }

  /* Buffer roll up */

  conv_buf_roolup( 1 );

  /* Updating the last line */

  unsigned int conv_buf_last_line = conv_buf_lines_ - 1;

  for( unsigned int bufcolumn = 0 ; bufcolumn < conv_buf_columns_ ; ++bufcolumn ) {
    if( ! inRaster_nptr->getElement( bufcolumn, line,
        conv_buf_[ conv_buf_last_line ][ bufcolumn ], band ) ) {
        
      TEAGN_TRUE_OR_THROW( dummy_used, "Unable to read from input raster" );
      
      conv_buf_[ conv_buf_last_line ][ bufcolumn ] = dummy_value;
    }
  }
}


void TePDIBufferedFilter::reset_maskmatrix( TePDIFilterMask::pointer& mask )
{
  TePDIFilterMask::deleteWeightsMatrix( temp_maskmatrix_, temp_maskmatrix_lines_ );

  temp_maskmatrix_ = mask->getWeightsMatrix();
  temp_maskmatrix_lines_ = mask->lines();
  temp_maskmatrix_columns_ = mask->columns();
}


void TePDIBufferedFilter::conv_buf_estatistics( unsigned int start_line,
  unsigned int start_column, unsigned int width, unsigned int height, double& mean,
  double& variance )
{
  TEAGN_DEBUG_CONDITION( ( conv_buf_lines_ >= ( start_line + height ) ),
    "Invalid convolution buffer lines" );
  TEAGN_DEBUG_CONDITION( ( conv_buf_columns_ >= ( start_column + width ) ),
    "Invalid convolution buffer columns" );

  mean = 0;
  variance = 0;

  unsigned int line_bound = start_line + height;
  unsigned int column_bound = start_column + width;
  double buf_value;

  for( unsigned int line = start_line ; line < line_bound ; ++line ) {
    for( unsigned int column = start_column ; column < column_bound ; ++column ) {
      buf_value = conv_buf_[ line ][ column ];
      mean += buf_value;
      variance += ( buf_value * buf_value );
    }
  }

  mean =  mean / ((double)width)  / ((double)height);
  variance = ( variance / ((double)width) / ((double)height) ) -
             ( mean * mean );
}


void TePDIBufferedFilter::conv_buf_roolup( unsigned int count )
{
  TEAGN_DEBUG_CONDITION( ( conv_buf_lines_ > 0 ), "Invalid convolution buffer lines" );

  double* first_buf_line_ptr;
  unsigned int conv_buf_last_line = conv_buf_lines_ - 1;
  unsigned int bufline;

  for( unsigned int cur_count = 0 ; cur_count < count ; ++cur_count ) {
    first_buf_line_ptr = conv_buf_[ 0 ];

    for( bufline = 1 ; bufline < conv_buf_lines_ ; ++bufline ) {
      conv_buf_[ bufline - 1 ] = conv_buf_[ bufline ];
    }

    conv_buf_[ conv_buf_last_line ] = first_buf_line_ptr;
  }
}
