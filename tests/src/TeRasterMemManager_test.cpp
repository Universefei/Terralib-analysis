
#include <terralib/kernel/TeRasterMemManager.h>
#include <terralib/kernel/TeUtils.h>

#include <terralib/kernel/TeAgnostic.h>

#include <stdlib.h>


void rammem_test()
{
  const unsigned int nlines = 3;
  const unsigned int ncols = 3;
  const unsigned int nbands = 3;
  const unsigned int datasize = sizeof(double);
  
  /* Creating the manager instance */
  
  std::vector< unsigned int > tiles_sizes;
  
  for( unsigned int band_index = 0 ; band_index < nbands ; ++band_index ) {
    tiles_sizes.push_back( datasize * ncols );
  }
  
  TeRasterMemManager mem_manager;
  if( ! mem_manager.reset( nbands, nlines, tiles_sizes,
    TeRasterMemManager::RAMMemPol, 100, 0 ) ) {
    
    throw TeException( UNKNOWN_ERROR_TYPE, 
      "Reset error" );
  }
  
  /* Testing memory */
  
  unsigned int band = 0;
  unsigned int line = 0;
  unsigned int col = 0;
  double value_counter = 0.0;
  
  for( band = 0 ; band < nbands ; ++band ) {
    for( line = 0 ; line < nlines ; ++line ) {
      double* double_line_ptr = (double*) mem_manager.getTilePointer( band,
        line );
      if( double_line_ptr == 0 ) {
        throw TeException( UNKNOWN_ERROR_TYPE, 
          "Invalid double_line_ptr pointer" );
      }        
    
      for( col = 0 ; col < ncols ; ++col ) {
        double_line_ptr[ col ] = value_counter;
        
        ++value_counter;
      }
    }
  }

  value_counter = 0.0;
  
  for( band = 0 ; band < nbands ; ++band ) {
    for( line = 0 ; line < nlines ; ++line ) {
      double* double_line_ptr = (double*) mem_manager.getTilePointer( band,
        line );
      if( double_line_ptr == 0 ) {
        throw TeException( UNKNOWN_ERROR_TYPE, 
          "Invalid double_line_ptr pointer" );
      }    
    
      for( col = 0 ; col < ncols ; ++col ) {
        if( double_line_ptr[ col ] != value_counter ) {
          throw TeException( UNKNOWN_ERROR_TYPE, 
            "Invalid value value_counter=" + Te2String( value_counter ) +
            " read_value=" + 
            Te2String( double_line_ptr[ col ] ) );
        }
        
        ++value_counter;
      }
    }
  }
  
  mem_manager.clear();
}

void diskmem_test()
{
  const unsigned int nlines = 3;
  const unsigned int ncols = 3;
  const unsigned int nbands = 3;
  const unsigned int datasize = sizeof(double);
  
  /* Creating the manager instance */
  
  std::vector< unsigned int > tiles_sizes;
  
  for( unsigned int band_index = 0 ; band_index < nbands ; ++band_index ) {
    tiles_sizes.push_back( datasize * ncols );
  }
  
  TeRasterMemManager mem_manager;
  if( ! mem_manager.reset( nbands, nlines, tiles_sizes,
    TeRasterMemManager::DiskMemPol, 0, 2ul * 1024ul * 1024ul * 1024ul ) ) {
    
    throw TeException( UNKNOWN_ERROR_TYPE, 
      "Reset error" );
  }
  
  /* Testing memory */
  
  unsigned int band = 0;
  unsigned int line = 0;
  unsigned int col = 0;
  double value_counter = 0.0;
  
  for( band = 0 ; band < nbands ; ++band ) {
    for( line = 0 ; line < nlines ; ++line ) {
      double* double_line_ptr = (double*) mem_manager.getTilePointer( band,
        line );
      if( double_line_ptr == 0 ) {
        throw TeException( UNKNOWN_ERROR_TYPE, 
          "Invalid double_line_ptr pointer" );
      }        
    
      for( col = 0 ; col < ncols ; ++col ) {
        double_line_ptr[ col ] = value_counter;
        
        ++value_counter;
      }
    }
  }

  value_counter = 0.0;
  
  for( band = 0 ; band < nbands ; ++band ) {
    for( line = 0 ; line < nlines ; ++line ) {
      double* double_line_ptr = (double*) mem_manager.getTilePointer( band,
        line );
      if( double_line_ptr == 0 ) {
        throw TeException( UNKNOWN_ERROR_TYPE, 
          "Invalid double_line_ptr pointer" );
      }    
    
      for( col = 0 ; col < ncols ; ++col ) {
        if( double_line_ptr[ col ] != value_counter ) {
          throw TeException( UNKNOWN_ERROR_TYPE, 
            "Invalid value value_counter=" + Te2String( value_counter ) +
            " read_value=" + 
            Te2String( double_line_ptr[ col ] ) );
        }
        
        ++value_counter;
      }
    }
  }
  
  mem_manager.clear();
}

void automem_test()
{
  const long int currFreeRam = MIN( ( TeGetTotalVirtualMemory() / 2.0 ) - 
      TeGetUsedVirtualMemory(), TeGetTotalPhysicalMemory() );
  const long int maxRAM = (long int)( 0.05 * ( (long double)currFreeRam ) );
  const unsigned int elementSize = sizeof(unsigned int);
  const unsigned int nlines = 100;
  const unsigned int ncols = 100;
  const unsigned int nbands = (unsigned int)( ((long double)maxRAM) / 
    (long double)( elementSize * nlines * ncols ) );
  
  /* Creating the manager instance */
  
  std::vector< unsigned int > tiles_sizes;
  
  for( unsigned int band_index = 0 ; band_index < nbands ; ++band_index ) {
    tiles_sizes.push_back( elementSize * ncols );
  }
  
  TeRasterMemManager mem_manager;
  if( ! mem_manager.reset( nbands, nlines, tiles_sizes,
    TeRasterMemManager::AutoMemPol, 2, 2ul * 1024ul * 1024ul * 1024ul ) ) {
    
    throw TeException( UNKNOWN_ERROR_TYPE, 
      "Reset error" );
  }
  
  /* Testing memory */
  
  unsigned int band = 0;
  unsigned int line = 0;
  unsigned int col = 0;
  unsigned int value_counter = 0.0;
  
  for( band = 0 ; band < nbands ; ++band ) {
    for( line = 0 ; line < nlines ; ++line ) {
      unsigned int* line_ptr = (unsigned int*) mem_manager.getTilePointer( band,
        line );
      if( line_ptr == 0 ) {
        throw TeException( UNKNOWN_ERROR_TYPE, 
          "Invalid double_line_ptr pointer" );
      }        
    
      for( col = 0 ; col < ncols ; ++col ) {
        line_ptr[ col ] = value_counter;
        
        ++value_counter;
      }
    }
  }

  value_counter = 0.0;
  
  for( band = 0 ; band < nbands ; ++band ) {
    for( line = 0 ; line < nlines ; ++line ) {
      unsigned int* line_ptr = (unsigned int*) mem_manager.getTilePointer( band,
        line );
      if( line_ptr == 0 ) {
        throw TeException( UNKNOWN_ERROR_TYPE, 
          "Invalid double_line_ptr pointer" );
      }    
      
      for( col = 0 ; col < ncols ; ++col ) {
        TEAGN_CHECK_EPS( line_ptr[ col ], value_counter, 0.0,
            "Invalid value_counter col=" + Te2String( col ) +
            " line=" + Te2String( line ) + " band=" +
            Te2String( band ) )
        
        ++value_counter;
      }
    }
  }
  
  mem_manager.clear();
}


int main()
{
  TEAGN_LOGMSG( "Test started." );

  TEAGN_DEBUG_MODE_CHECK;

  rammem_test();
  diskmem_test();
  automem_test();

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
