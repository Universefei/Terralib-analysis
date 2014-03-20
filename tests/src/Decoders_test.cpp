/* ------------------------------------------------------- */

// Uncomment this line to enable the PAM decoder test
//#define ENABLE_PAMDECODER_TEST

/* ------------------------------------------------------- */

#include "TeTestsBase.hpp"

#include <terralib/kernel/TeDecoderSmartMem.h>
#include <terralib/kernel/TeDecoderMemory.h>
#include <terralib/kernel/TeDecoderMemoryMap.h>

#ifdef ENABLE_PAMDECODER_TEST
  #include <terralib/kernel/TeDecoderPAM.h>
#endif

#include <terralib/kernel/TeException.h>
#include <terralib/kernel/TeUtils.h>
#include <terralib/kernel/TeInitRasterDecoders.h>
#include <terralib/kernel/TeAgnostic.h>
#include <terralib/kernel/TeProgress.h>
#include <terralib/kernel/TeStdIOProgress.h>

#include <stdlib.h>


void DecoderSmartMem_test()
{
  /* Choosing the data size base on the current avaliable free RAM
     plus 2 bands to force the allocation of mapped memory files */
     
  const int columns = 100;
  const int lines = 100;
  const unsigned int datasize = sizeof(unsigned long int);
  const unsigned long int curr_free_ram = (unsigned long int)
    ( 0.05 * (double)MIN( ( TeGetTotalVirtualMemory() / 2.0 ) - 
      TeGetUsedVirtualMemory(), TeGetTotalPhysicalMemory() ) );
  const int bands = ( ( 2 * curr_free_ram ) / 
    ( lines * columns * datasize ) ) + 1;  
  
  TeRasterParams params;
  params.setNLinesNColumns( lines, columns );
  params.nBands( bands );
  params.setDataType( TeUNSIGNEDLONG, -1 );
  params.setDummy( 0, -1 );
  params.decoderIdentifier_ = "SMARTMEM";
  
  TeRasterParams::DecoderParamT decParm;
  decParm.first = "MaxMemPercentUsage";
  decParm.second = "5";
  params.decoderParams_.push_back( decParm );
  
  TeSharedPtr< TeDecoderSmartMem > decoder( (TeDecoderSmartMem*)
    TeDecoderFactory::make( params ) );
  if( ! decoder.isActive() ) {
    throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder instantiation error", 
      false ) );
  }  
  
  decoder->init();
  
  if( decoder->params().status_ != TeRasterParams::TeReadyToRead ) {
    throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder initialization error", 
      false ) );    
  }  
  
  double value_in_double = 0;
  double value_out_double = 0;
  int band = 0;
  int line = 0;
  int col = 0;
  
  // Testing decoder overloaded methods
  
  for( band = 0 ; band < bands ; ++band ) {
    for( line = 0 ; line < lines ; ++line ) {
      for( col = 0 ; col < columns ; ++col ) {
        if( ! decoder->setElement( col, line, value_in_double, band ) ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "setElement error", false ) );
        }

        value_out_double = 0;
        
        if( ! decoder->getElement( col, line, value_out_double, band ) ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "getElement error", 
            false ) );
        }
                        
        TEAGN_CHECK_EPS( value_out_double, value_in_double, 0.0, 
          "Invalid value read band=" + Te2String( band ) + " line=" + 
          Te2String( line ) + " col=" + Te2String( col ) )
        
        value_in_double += 1.0;
      }
    }  
  }
  
  value_in_double = 0;
  value_out_double = 0;
  
  for( band = 0 ; band < bands ; ++band ) {
    for( line = 0 ; line < lines ; ++line ) {
      for( col = 0 ; col < columns ; ++col ) {
        if( ! decoder->getElement( col, line, value_out_double, band ) ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "getElement error", 
            false ) );
        }
        
        TEAGN_CHECK_EPS( value_out_double, value_in_double, 0.0, 
          "Invalid value read band=" + Te2String( band ) + " line=" + 
          Te2String( line ) + " col=" + Te2String( col ) )
        
        value_in_double += 1.0;
      }
    }  
  }
  
  // Testing decoder optmized methods
  
  unsigned int value_in_uint = 0;
  unsigned int value_out_uint = 0;
  
  for( band = 0 ; band < bands ; ++band ) 
  {
    for( line = 0 ; line < lines ; ++line ) 
    {
      for( col = 0 ; col < columns ; ++col ) 
      {
        decoder->setValue( col, line, value_in_uint, band );
        
        value_in_uint += 1.0;
      }
    }  
  }  
  
  value_in_uint = 0;
  value_out_uint = 0; 
  unsigned int* linePtr = 0;  
  
  for( band = 0 ; band < bands ; ++band ) 
  {
    for( line = 0 ; line < lines ; ++line ) 
    {
      linePtr = (unsigned int*)decoder->getScanLine( line, band );
      TEAGN_TRUE_OR_THROW( linePtr, "Invalid line pointer" );
      
      for( col = 0 ; col < columns ; ++col ) 
      {
        decoder->getValue( col, line, value_out_uint, band );
        
        TEAGN_CHECK_EPS( value_out_uint, value_in_uint, 0.0, 
          "Invalid value read band=" + Te2String( band ) + " line=" + 
          Te2String( line ) + " col=" + Te2String( col ) )
          
        TEAGN_CHECK_EPS( value_out_uint, linePtr[ col ], 0.0, 
          "Invalid value read band=" + Te2String( band ) + " line=" + 
          Te2String( line ) + " col=" + Te2String( col ) )        
        
        value_in_uint += 1.0;
      }
    }  
  }   
}


void DecoderMemory_test()
{
  const unsigned int columns = 2000;
  const unsigned int lines = 2000;
  const unsigned int bands = 10;
  
  TeRasterParams params;
  params.setNLinesNColumns( lines, columns );
  params.nBands( bands );
  params.setDataType( TeDOUBLE, -1 );
  params.setDummy( 0, -1 );
  params.decoderIdentifier_ = "MEM";
  
  TeSharedPtr< TeDecoder > decoder( TeDecoderFactory::make( params ) );
  if( ! decoder.isActive() ) {
    throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder instantiation error", 
      false ) );
  } 
  
  decoder->init();
  
  if( decoder->params().status_ != TeRasterParams::TeReadyToWrite ) {
    throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder initialization error", 
      false ) );    
  }  
  
  double value_in = 0;
  unsigned int band = 0;
  unsigned int line = 0;
  unsigned int col = 0;
  
  for( band = 0 ; band < bands ; ++band ) {
    for( line = 0 ; line < lines ; ++line ) {
      for( col = 0 ; col < columns ; ++col ) {
        if( ! decoder->setElement( col, line, value_in, band ) ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "setElement error", false ) );
        }
        
        value_in += 1.0;
      }
    }  
  }
  
  value_in = 0;
  double value_out = 0;
  
  for( band = 0 ; band < bands ; ++band ) {
    for( line = 0 ; line < lines ; ++line ) {
      for( col = 0 ; col < columns ; ++col ) {
        if( ! decoder->getElement( col, line, value_out, band ) ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "getElement error", 
            false ) );
        }
        
        if( value_out != value_in ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "invalid value read", 
            false ) );
        }
        
        value_in += 1.0;
      }
    }  
  }
}


void TeDecoderTIFF_test()
{
  const unsigned int columns = 5;
  const unsigned int lines = 5;
  const unsigned int bands = 2;

  {
    TeRasterParams params;
    params.nBands( bands );
    params.setDataType( TeUNSIGNEDCHAR, -1 );
    params.setDummy( 0, -1 );
    params.fileName_ = "TeDecoderTIF_test.tif";
    params.mode_ = 'c';
    params.decoderIdentifier_ = "TIF";
    params.setNLinesNColumns( lines, columns );
    
    TeSharedPtr< TeDecoder > decoder( TeDecoderFactory::make( params ) );
    if( ! decoder.isActive() ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder instantiation error", 
        false ) );
    }  
    
    decoder->init();
    
    if( decoder->params().status_ != TeRasterParams::TeReadyToWrite ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder initialization error", 
        false ) );    
    }
    
    double value_out = 0;
    unsigned int band = 0;
    unsigned int line = 0;
    unsigned int col = 0;
    
    for( band = 0 ; band < bands ; ++band ) {
      for( line = 0 ; line < lines ; ++line ) {
        for( col = 0 ; col < columns ; ++col ) {
          if( ! decoder->setElement( col, line, value_out, band ) ) {
            throw( TeException( UNKNOWN_ERROR_TYPE , 
              "setElement error at col=" +
              Te2String( col ) + " line=" + Te2String( line ),
              false ) );
          }
          
          value_out += 1.0;
        }
      }  
    }
    
    decoder.reset();
  }
  
  {
    TeRasterParams params;
    params.fileName_ = "TeDecoderTIF_test.tif";
    params.mode_ = 'r';
    params.decoderIdentifier_ = "TIF";
    
    TeSharedPtr< TeDecoder > decoder( TeDecoderFactory::make( params ) );
    if( ! decoder.isActive() ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder instantiation error", 
        false ) );
    }  
    
    decoder->init();
    
    if( decoder->params().status_ != TeRasterParams::TeReadyToRead ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder initialization error", 
        false ) );    
    }
    if( decoder->params().ncols_ != (int)columns ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid columsn number " +
        Te2String( decoder->params().ncols_ ), 
        false ) );    
    }    
    if( decoder->params().nlines_ != (int)lines ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid lines number " +
        Te2String( decoder->params().nlines_ ), 
        false ) );    
    }    
    if( decoder->params().nBands() != (int)bands ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid channels number " +
        Te2String( decoder->params().nBands() ), 
        false ) );    
    }    
    if( decoder->params().dataType_[ 0 ] != TeUNSIGNEDCHAR ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid data type", 
        false ) );    
    }     
    
    double value_in = 0;
    double value_counter = 0;
    unsigned int band = 0;
    unsigned int line = 0;
    unsigned int col = 0;
    
    for( band = 0 ; band < bands ; ++band ) {
      for( line = 0 ; line < lines ; ++line ) {
        for( col = 0 ; col < columns ; ++col ) {
          if( ! decoder->getElement( col, line, value_in, band ) ) {
            throw( TeException( UNKNOWN_ERROR_TYPE , "getElement error", 
              false ) );
          }
          
          if( value_counter != value_in ) {
            throw( TeException( UNKNOWN_ERROR_TYPE , "invalid read value" 
              " col=" + Te2String( col ) + " line=" + Te2String( line ) +
              " value_in=" + Te2String( value_in ) + 
              " value_counter=" + Te2String( value_counter ), 
              false ) );
          }
          
          value_counter += 1.0;
        }
      }  
    }
    
    decoder.reset();
  }  
}

#ifdef ENABLE_PAMDECODER_TEST
  void TeDecoderPAM_test()
  {
    const unsigned int columns = 10;
    const unsigned int lines = 5;
    const unsigned int bands = 3;
  
    {
      TeRasterParams params;
      params.setNLinesNColumns( lines, columns );
      params.nBands( bands );
      params.setDataType( TeUNSIGNEDCHAR, -1 );
      params.setDummy( 0, -1 );
      params.fileName_ = "TeDecoderPAM_test.ppm";
      params.mode_ = 'c';
      params.decoderIdentifier_ = "PAM";
      
      TeSharedPtr< TeDecoder > decoder( TeDecoderFactory::make( params ) );
      if( ! decoder.isActive() ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder instantiation error", 
          false ) );
      }  
      
      decoder->init();
      
      if( decoder->params().status_ != TeRasterParams::TeReadyToWrite ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder initialization error", 
          false ) );    
      }
      
      double value_in = 0;
      double value_out = 0;
      unsigned int band = 0;
      unsigned int line = 0;
      unsigned int col = 0;
      
      for( band = 0 ; band < bands ; ++band ) {
        for( line = 0 ; line < lines ; ++line ) {
          for( col = 0 ; col < columns ; ++col ) {
            if( ! decoder->setElement( col, line, value_out, band ) ) {
              throw( TeException( UNKNOWN_ERROR_TYPE , 
                "setElement error at col=" +
                Te2String( col ) + " line=" + Te2String( line ),
                false ) );
            }
            if( ! decoder->getElement( col, line, value_in, band ) ) {
              throw( TeException( UNKNOWN_ERROR_TYPE , 
                "getElement error at col=" +
                Te2String( col ) + " line=" + Te2String( line ),
                false ) );
            }
            if( value_in != value_out ) {
              throw( TeException( UNKNOWN_ERROR_TYPE , 
                "invalid stored value at col=" +
                Te2String( col ) + " line=" + Te2String( line ) +
                " value_in=" + Te2String( value_in ) + 
                " value_out=" + Te2String( value_out ),
                false ) );              
            }
            
            value_out += 1.0;
          }
        }  
      }
      
      decoder.reset();
    }
    
    {
      TeRasterParams params;
      params.fileName_ = "TeDecoderPAM_test.ppm";
      params.mode_ = 'r';
      params.decoderIdentifier_ = "PAM";
      
      TeSharedPtr< TeDecoder > decoder( TeDecoderFactory::make( params ) );
      if( ! decoder.isActive() ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder instantiation error", 
          false ) );
      }  
      
      decoder->init();
      
      if( decoder->params().status_ != TeRasterParams::TeReadyToRead ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder initialization error", 
          false ) );    
      }
      if( decoder->params().ncols_ != (int)columns ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid columsn number " +
          Te2String( decoder->params().ncols_ ), 
          false ) );    
      }    
      if( decoder->params().nlines_ != (int)lines ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid lines number " +
          Te2String( decoder->params().nlines_ ), 
          false ) );    
      }    
      if( decoder->params().nBands() != 3 ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid channels number " +
          Te2String( decoder->params().nBands() ), 
          false ) );    
      }    
      if( decoder->params().dataType_[ 0 ] != TeUNSIGNEDCHAR ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid data type", 
          false ) );    
      }     
      
      double value_in = 0;
      double value_counter = 0;
      unsigned int band = 0;
      unsigned int line = 0;
      unsigned int col = 0;
      
      for( band = 0 ; band < bands ; ++band ) {
        for( line = 0 ; line < lines ; ++line ) {
          for( col = 0 ; col < columns ; ++col ) {
            if( ! decoder->getElement( col, line, value_in, band ) ) {
              throw( TeException( UNKNOWN_ERROR_TYPE , "getElement error", 
                false ) );
            }
            
            if( value_counter != value_in ) {
              throw( TeException( UNKNOWN_ERROR_TYPE , "invalid read value" 
                " col=" + Te2String( col ) + " line=" + Te2String( line ) +
                " value_in=" + Te2String( value_in ) + 
                " value_counter=" + Te2String( value_counter ), 
                false ) );
            }
            
            value_counter += 1.0;
          }
        }  
      }
      
      decoder.reset();
    }  
  }
#endif

void DecoderMemoryMap_test()
{
  const unsigned int columns = 100;
  const unsigned int lines = 100;
  const unsigned int bands = 2;
  
  TeRasterParams params;
  params.setNLinesNColumns( lines, columns );
  params.nBands( bands );
  params.setDataType( TeINTEGER, -1 );
  params.setDummy( 0, -1 );
  params.decoderIdentifier_ = "MEMMAP";
  params.mode_ = 'c';
  params.fileName_ = TETESTSBINPATH "/DecoderMemoryMap_test.bin";
  
  TeSharedPtr< TeDecoder > decoder( TeDecoderFactory::make( params ) );
  if( ! decoder.isActive() ) {
    throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder instantiation error", 
      false ) );
  }  
  
  decoder->init();
  
  if( decoder->params().status_ != TeRasterParams::TeReadyToWrite ) {
    throw( TeException( UNKNOWN_ERROR_TYPE , "Decoder initialization error", 
      false ) );    
  }  
  
  double value_in = 0;
  unsigned int band = 0;
  unsigned int line = 0;
  unsigned int col = 0;
  
  for( band = 0 ; band < bands ; ++band ) {
    for( line = 0 ; line < lines ; ++line ) {
      for( col = 0 ; col < columns ; ++col ) {
        if( ! decoder->setElement( col, line, value_in, band ) ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "setElement error", false ) );
        }
        
        value_in += 1.0;
      }
    }  
  }
  
  value_in = 0;
  double value_out = 0;
  
  for( band = 0 ; band < bands ; ++band ) {
    for( line = 0 ; line < lines ; ++line ) {
      for( col = 0 ; col < columns ; ++col ) {
        if( ! decoder->getElement( col, line, value_out, band ) ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "getElement error", 
            false ) );
        }
        
        if( value_out != value_in ) {
          throw( TeException( UNKNOWN_ERROR_TYPE , "invalid value read", 
            false ) );
        }
        
        value_in += 1.0;
      }
    }  
  }
}


int main()
{
  std::cout << std::endl << "Test started." << std::endl;

  TEAGN_DEBUG_MODE_CHECK;

  TeStdIOProgress pi;
  TeProgress::setProgressInterf( dynamic_cast< TeProgressBase* >( &pi ) );   

  TeInitRasterDecoders();

  DecoderMemoryMap_test();
  DecoderSmartMem_test();
  DecoderMemory_test();
  TeDecoderTIFF_test();

  #ifdef ENABLE_PAMDECODER_TEST
    TeDecoderPAM_test();
  #endif
  
  std::cout << std::endl << "Test OK" << std::endl;
  return EXIT_SUCCESS;
}
