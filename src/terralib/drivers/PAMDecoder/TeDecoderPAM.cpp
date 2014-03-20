#include "TeDecoderPAM.h"

#include "../../kernel/TeAgnostic.h"
#include "../../kernel/TeUtils.h"

extern "C"
{
  #include <netpbm/pam.h>
}

#include <assert.h>
#include <string.h>


TeDecoderPAM::TeDecoderPAM ()
{ 
  initVars();
  
  params_.decoderIdentifier_ = "PAM";
};


TeDecoderPAM::TeDecoderPAM ( const TeRasterParams& par )
{
  initVars();
  
  params_ = par; 
  params_.decoderIdentifier_ = "PAM"; 
};



TeDecoderPAM::~TeDecoderPAM ()
{
  /* Writing data to disk file */
  
  if ( ( ( params_.mode_ == 'c' ) || ( params_.mode_ == 'w') ) &&
    ( params_.status_ == TeRasterParams::TeReadyToWrite ) &&
    ( params_.nBands() > 0 ) && ( params_.nlines_ > 0 ) && 
    ( params_.ncols_ > 0 ) && ( ! params_.fileName_.empty() ) ) {
    
    /* Defining the PAM Handler */
    
    pam pam_file_handler;
    
    pam_file_handler.size = sizeof( pam );
    pam_file_handler.len = sizeof( pam );
    
    std::string extension = TeConvertToUpperCase( TeGetExtension( 
      params_.fileName_.c_str() ) );
      
    if( extension == "PBM" ) {
      pam_file_handler.format = PBM_FORMAT;
      strcpy( pam_file_handler.tuple_type, "BLACKANDWHITE" );
    } else if( extension == "PGM" ) {
      pam_file_handler.format = PGM_FORMAT;
      strcpy( pam_file_handler.tuple_type, "GRAYSCALE" );
    } else if( extension == "PPM" ) {
      pam_file_handler.format = PPM_FORMAT;
      strcpy( pam_file_handler.tuple_type, "RGB" );
    } else {
      TEAGN_LOG_AND_THROW( "Invalid file format" )
    }
    
    pam_file_handler.plainformat = 0;
    
    pam_file_handler.height = params_.nlines_;
    pam_file_handler.width = params_.ncols_;
    pam_file_handler.depth = params_.nBands();
    
    switch( params_.dataType_[ 0 ] ) {
      case TeUNSIGNEDCHAR :
      {
        pam_file_handler.maxval = 255;
        pam_file_handler.bytes_per_sample = 1;
        break;
      }
      case TeUNSIGNEDSHORT :
      {
        pam_file_handler.maxval = 65535;
        pam_file_handler.bytes_per_sample = 2;
        break;
      }      
      default :
      {
        TEAGN_LOG_AND_THROW( "Invalid data type" )
        break;
      }
    }
    
    pam_file_handler.allocation_depth = params_.nBands();    
  
    FILE* image_file_ptr = fopen( params_.fileName_.c_str(), "wb" );
    if( image_file_ptr != 0 ) {
      pam_file_handler.file = image_file_ptr;
      
      pnm_writepaminit( &pam_file_handler );    
      
      /* encoding data into file */
        
      tuple* tuple_ptr = pnm_allocpamrow( &pam_file_handler );
      if( tuple_ptr == 0 ) {
        fclose( image_file_ptr );
        TEAGN_LOG_AND_THROW( "TeDecoderPAM init error" )
      }
        
      int line = 0;
      int col = 0;
      unsigned int channel = 0; 
      double value = 0;
        
      TeProgressBase* prog_interf = TeProgress::instance();
      if( prog_interf != 0 ) {
        prog_interf->setMessage( "Encoding image" );
        prog_interf->setTotalSteps( 100 );
      }
        
      for( line = 0 ; line < pam_file_handler.height ; ++line ) {
        if( prog_interf != 0 ) {
          prog_interf->setProgress( (int)( 100.0 * ( (double)line) / 
            ((double)pam_file_handler.height ) ) );
        }
        
        for( col = 0 ; col < pam_file_handler.width ; ++col ) {
        
          for( channel = 0 ; channel < pam_file_handler.depth ; ++channel ) {
          
            getElement( col, line, value, channel );
            tuple_ptr[ col ][ channel ] = (sample)value;
          }
        }        
        
        pnm_writepamrow( &pam_file_handler, tuple_ptr );
      }
        
      if( prog_interf != 0 ) {
        prog_interf->reset();
      }
        
      pnm_freepamrow( tuple_ptr );      
    
      fclose( image_file_ptr );
    }  
  }
  
  /* Free the memory structures */

  clear();
}

void TeDecoderPAM::init()
{
  clear();
  
  params_.status_ = TeRasterParams::TeNotReady;
  
  if (params_.mode_ == 'c' || params_.mode_ == 'w') // creating a new file
  {
    bool params_checks_ok = true;
    
    std::string extension = TeConvertToUpperCase( TeGetExtension( 
      params_.fileName_.c_str() ) );
      
    /* Checking extension */  
      
    if( ( extension == "PPM" ) && ( params_.nBands() != 3 ) ) {
      params_checks_ok = false;
    }
    
    /* Checking data type */
    
    for( int band_index = 0 ; band_index < params_.nBands() ;
      ++band_index ) {
      
      if( ( params_.dataType_[ band_index ] != TeUNSIGNEDCHAR ) &&
        ( params_.dataType_[ band_index ] != TeUNSIGNEDSHORT ) ) {
        
        TEAGN_LOGERR( "Invalid data type ( band " + Te2String( band_index ) +
          ")" )
        
        params_checks_ok = false;
        break;
      }
    }
    
    if( params_checks_ok ) {
      TeDecoderSmartMem::init();
    }
  } else if (params_.mode_ == 'r') {
    /* Creating PAM handler */
    
    FILE* image_file_ptr = fopen( params_.fileName_.c_str(), "rb" );
    if( image_file_ptr != 0 ) {
      pam pam_file_handler;
      
      pnm_readpaminit( image_file_ptr, &pam_file_handler, 
        PAM_STRUCT_SIZE(tuple_type) );
        
      if( ( pam_file_handler.height != 0  && ( pam_file_handler.width != 0 ) 
        ) ) {
        /* updating the current parameters */
        
        if( pam_file_handler.bytes_per_sample == 1 ) {
          params_.setDataType( TeUNSIGNEDCHAR, -1 );  
        } else if( pam_file_handler.bytes_per_sample == 2 ) {
          params_.setDataType( TeUNSIGNEDSHORT, -1 );  
        } else  {
          fclose( image_file_ptr );
          TEAGN_LOG_AND_THROW( "Invalid bytes_per_sample value" )
        }
        
        params_.nBands( pam_file_handler.depth );
        
        std::string tuple_type( pam_file_handler.tuple_type );
        if( tuple_type.find( "RGB" ) < tuple_type.size() ) {
          params_.setPhotometric( TeRasterParams::TeRGB );
        } else {
          params_.setPhotometric( TeRasterParams::TeMultiBand );
        }
        
        params_.useDummy_ = false;
        
        params_.setNLinesNColumns( pam_file_handler.height, 
          pam_file_handler.width );
          
        /* Creating the new memory data structures */
        
        TeDecoderSmartMem::init();
        
        if( params_.status_ != TeRasterParams::TeNotReady ) {
          /* Decoding data into memory */
          
          tuple* tuple_ptr = pnm_allocpamrow( &pam_file_handler );
          if( tuple_ptr == 0 ) {
            fclose( image_file_ptr );
            TEAGN_LOG_AND_THROW( "TeDecoderPAM init error" )
          }
          
          int line = 0;
          int col = 0;
          unsigned int channel = 0; 
          
          TeProgressBase* prog_interf = TeProgress::instance();
          if( prog_interf != 0 ) {
            prog_interf->setMessage( "Decoding image" );
            prog_interf->setTotalSteps( 100 );
          }
          
          for( line = 0 ; line < pam_file_handler.height ; ++line ) {
            if( prog_interf != 0 ) {
              prog_interf->setProgress( (int)( 100.0 * ( (double)line ) / 
                ( (double)pam_file_handler.height ) ) );
            }
          
            pnm_readpamrow( &pam_file_handler, tuple_ptr );
              
            for( col = 0 ; col < pam_file_handler.width ; ++col ) {
              for( channel = 0 ; channel < pam_file_handler.depth ; ++channel ) {
                setElement( col, line, (double)tuple_ptr[ col ][ channel ], 
                  channel );
              }
            }
          }
          
          if( prog_interf != 0 ) {
            prog_interf->reset();
          }
          
          pnm_freepamrow( tuple_ptr );
        
          params_.status_ = TeRasterParams::TeReadyToRead;
        }
      }
      
      fclose( image_file_ptr );
    }
  }
}


bool registerPAMDecoder()
{
  static TeDecoderPAMFactory TeDecoderPAMFactory_instance("PAM");
  
  TeDecoderFactory::instanceName2Dec()["PBM"] = "PAM";  
  TeDecoderFactory::instanceName2Dec()["pbm"] = "PAM";

  TeDecoderFactory::instanceName2Dec()["PGM"] = "PAM";  
  TeDecoderFactory::instanceName2Dec()["pgm"] = "PAM";

  TeDecoderFactory::instanceName2Dec()["PPM"] = "PAM";  
  TeDecoderFactory::instanceName2Dec()["ppm"] = "PAM";
  
  return true;  
};
