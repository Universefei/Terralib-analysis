#include "TePDIRasterRUBuffer.hpp"

#include "../kernel/TeAgnostic.h"

#include <limits.h>

TePDIRasterRUBuffer::TePDIRasterRUBuffer()
{
  rasterPtr_ = 0;
  rasterChn_ = 0;
  bufferPtr_ = 0;
  bufferLines_ = 0;
  bufferCols_ = 0;
  lastReadedLine_ = UINT_MAX;
}


TePDIRasterRUBuffer::~TePDIRasterRUBuffer()
{
  clear();
}

bool TePDIRasterRUBuffer::reset( TeRaster& inputRaster, 
  unsigned int inRasterChn, unsigned int nLines )
{
  clear();
  
  TEAGN_TRUE_OR_RETURN( inputRaster.params().status_ != 
    TeRasterParams::TeNotReady, "Raster not ready" )
  TEAGN_TRUE_OR_RETURN( inputRaster.params().status_ != 
    TeRasterParams::TeNotReady, "Raster not ready" )  
  TEAGN_TRUE_OR_RETURN( inputRaster.params().nlines_ >= 
    (int)nLines, "Invalid raster lines number" )    

  try
  {
    bufferPtr_ = new double*[ nLines ];
  }
  catch(...)
  {
    return false;
  }
  if( bufferPtr_ == 0 ) 
  {
    return false;
  }
  
  bufferLines_ = nLines;
  
  for( unsigned int line = 0 ; line < nLines ; ++line )
    bufferPtr_[ line ] = 0;
  
  try
  {
    bufferCols_ = inputRaster.params().ncols_;
    
    for( unsigned int line = 0 ; line < nLines ; ++line )
    {
      bufferPtr_[ line ] = new double[ bufferCols_ ];
      
      if( bufferPtr_[ line ] == 0 )
      {
        clear();
        return false;
      }
    }
  }
  catch(...)
  {
    clear();
    return false;
  }
  
  rasterPtr_ = &inputRaster;
  rasterChn_ = inRasterChn;
  
  return true;
}

bool TePDIRasterRUBuffer::roolUp()
{
  TEAGN_DEBUG_CONDITION( rasterPtr_ != 0, "No input raster" )
  TEAGN_DEBUG_CONDITION( bufferPtr_ != 0, "No internal buffer" )
  
  if( ( (int)lastReadedLine_  ) == ( rasterPtr_->params().nlines_ 
    - 1 ) ) 
  {
    return false;
  }
  else
  {
    // roll-up
    
    double* firstLine = bufferPtr_[ 0 ];
    
    unsigned int lastLineIdx = bufferLines_ - 1;
    
    for( unsigned int line = 0 ; line < lastLineIdx ; ++line )
      bufferPtr_[ line ] = bufferPtr_[ line + 1 ];
                          
    bufferPtr_[ lastLineIdx ] = firstLine;
    
    // load the next line
    
    if( lastReadedLine_ == UINT_MAX )
    {
      lastReadedLine_ = 0;
    }
    else
    {
      ++lastReadedLine_;
    }
    
    for( unsigned int col = 0 ; col < bufferCols_ ; ++col )
    {
      rasterPtr_->getElement( col, lastReadedLine_, firstLine[ col ], 
        rasterChn_ );
    }    
    
    return true;
  }
}

double** TePDIRasterRUBuffer::getBufferPtr()
{
  TEAGN_DEBUG_CONDITION( bufferPtr_ != 0, "No internal buffer" )
  return bufferPtr_;
}

void TePDIRasterRUBuffer::clear()
{
  if( bufferPtr_ )
  {
    for( unsigned int line = 0 ; line < bufferLines_ ; ++line )
      if( bufferPtr_[ line ] ) delete[] bufferPtr_[ line ];
    
    delete[] bufferPtr_;
  }
  
  bufferPtr_ = 0;
  bufferLines_ = 0;
  bufferCols_ = 0;
  rasterPtr_ = 0;
  rasterChn_ = 0;
  lastReadedLine_ = UINT_MAX;
}
