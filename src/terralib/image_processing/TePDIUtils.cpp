#include "TePDIUtils.hpp"

#include "TePDIInterpolator.hpp"
#include "TePDIPIManager.hpp"

#include "../kernel/TeRaster.h"
#include "../kernel/TeRasterParams.h"
#include "../kernel/TeDataTypes.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeDefines.h"
#include "../kernel/TeVectorRemap.h"
#include "../kernel/TeRasterTransform.h"
#include "../kernel/TeRasterRemap.h"
#include "../kernel/TeProgress.h"
#include "../kernel/TeAgnostic.h"
#include "../kernel/TeDecoderJPEG.h"
#include "../kernel/TeDecoder.h"


#include <float.h>
#include <math.h>
#include <stdio.h>


namespace TePDIUtils{


  bool TeAllocRAMRaster(
    TePDITypes::TePDIRasterPtrType& template_raster,
    TePDITypes::TePDIRasterPtrType& RAMRaster,
    unsigned int bands, bool force_new_dt, bool force_mm,
    TeDataType pixel_type )
  {
    TEAGN_TRUE_OR_RETURN( template_raster.isActive(),
      "Inactive Input Raster" );
    TEAGN_TRUE_OR_RETURN( template_raster->params().status_ != TeRasterParams::TeNotReady,
      "Input not Ready" );

    TeRasterParams temp_params = template_raster->params();

    if( bands != 0 ) {
      temp_params.nBands( bands );
    }

    if( force_new_dt ) {
      temp_params.setDataType( pixel_type, -1 );
    }

    return TeAllocRAMRaster( RAMRaster, temp_params, force_mm );
  }


  bool TeAllocRAMRaster(
    TePDITypes::TePDIRasterPtrType& RAMRaster,
    unsigned int bands, unsigned int lines, unsigned int columns,
    bool force_mm, TeDataType pixel_type, TePDIRgbPalette* palette )
  {
    TEAGN_TRUE_OR_RETURN( bands > 0, "Invalid bands number" );
    TEAGN_TRUE_OR_RETURN( lines > 0, "Invalid lines number" );
    TEAGN_TRUE_OR_RETURN( columns > 0, "Invalid columns number" );

    TeRasterParams temp_params;

    temp_params.nBands( bands );

    temp_params.setDataType( pixel_type, -1 );
    
    temp_params.setNLinesNColumns( lines, columns );
    
    if( palette != 0 ) {
      temp_params.setPhotometric( TeRasterParams::TePallete );

      temp_params.lutr_.clear();
      temp_params.lutg_.clear();
      temp_params.lutb_.clear();

      TePDIRgbPalette::iterator pal_it = palette->begin();
      TePDIRgbPalette::iterator pal_it_end = palette->end();

      for( unsigned int lut_index = 0 ; lut_index < palette->size() ;
           ++lut_index ) {

        temp_params.lutr_.push_back( (unsigned short)pal_it->second.red_ );
        temp_params.lutg_.push_back( (unsigned short)pal_it->second.green_ );
        temp_params.lutb_.push_back( (unsigned short)pal_it->second.blue_ );

        ++pal_it;
      }
    }    

    return TeAllocRAMRaster( RAMRaster, temp_params, force_mm );
  }
  
  bool TeAllocRAMRaster( TePDITypes::TePDIRasterPtrType& RAMRaster,
    const TeRasterParams& params, bool force_mm )
  {
    return TeAllocRAMRaster( RAMRaster, params,
      ( force_mm ? TePDIUtilsDiskMemPol : TePDIUtilsAutoMemPol ) );
  }

  
  bool TeAllocRAMRaster( TePDITypes::TePDIRasterPtrType& RAMRaster,
    const TeRasterParams& params, TePDIUtilsMemPol mempol )
  {
    TeRasterParams internal_params = params;
    internal_params.mode_ = 'c';
    internal_params.decoderIdentifier_ = "SMARTMEM";
    
    switch( mempol )
    {
      case TePDIUtilsAutoMemPol :
      {
        // use the default
        break;
      }
      case TePDIUtilsRamMemPol :
      {
        internal_params.decoderParams_.push_back( std::pair< std::string, 
          std::string >( "TeDecoderSmartMem_MaxMemPercentUsage", "100" ) );
        break;
      }      
      case TePDIUtilsDiskMemPol :
      {
        internal_params.decoderParams_.push_back( std::pair< std::string, 
          std::string >( "TeDecoderSmartMem_MaxMemPercentUsage", "0" ) );
        break;
      }     
      default :
      {
        TEAGN_LOG_AND_THROW( "Invalid memory policy" );
        break;
      }
    }
    
    RAMRaster.reset( new TeRaster( internal_params ) );
  
    return RAMRaster->init();
  }
  
  bool TeAllocRAMRaster( const TeRasterParams& params,
      TePDITypes::TePDIRasterPtrType& rasterPointer )
  {
    return TeAllocRAMRaster( rasterPointer, params, 
      TePDIUtilsAutoMemPol );
  }  
  
  bool TeRaster2Geotiff(
    const TePDITypes::TePDIRasterPtrType& in_raster,
    const std::string& file_name )
  {
    return TeRaster2Geotiff( in_raster, file_name, false );
  }  
    

  bool TeRaster2Geotiff(
    const TePDITypes::TePDIRasterPtrType& in_raster,
    const std::string& file_name, TeDataType pixel_type )
  {
    TEAGN_TRUE_OR_RETURN( in_raster.isActive(),
      "Inactive Input Raster" );
    TEAGN_TRUE_OR_RETURN( in_raster->params().status_ != 
      TeRasterParams::TeNotReady, "Input not Ready" );
    TEAGN_TRUE_OR_RETURN( file_name.size() != 0,
      "Invalid file name" );

    TeRasterParams temp_params = in_raster->params();

    temp_params.mode_ = 'c';
    temp_params.fileName_ = file_name;
    temp_params.decoderIdentifier_ = "TIF";
    temp_params.setDataType( pixel_type, -1 );
    
    /* Creating disk output raster */
    
    TeRaster outRaster( temp_params );

    TEAGN_TRUE_OR_RETURN( outRaster.init(),
      "Unable to init GeoTIFF Raster" );
      
    const unsigned int nlines = outRaster.params().nlines_;
    const unsigned int ncols = outRaster.params().ncols_;
    const unsigned int nbands = outRaster.params().nBands();
    unsigned int line = 0 , col = 0 , band = 0;
    double value = 0.0;
    TeRaster& ir = (*in_raster);
    
    for( line = 0 ; line < nlines ; ++line )
      for( band = 0 ; band < nbands ; ++band )
        for( col = 0 ; col < ncols ; ++col )
        {
          ir.getElement( col, line, value, band );
          outRaster.setElement( col, line, value, band );
        }
      
    return true;
  }

  bool TeRaster2Geotiff(
    const TePDITypes::TePDIRasterPtrType& in_raster,
    const std::string& file_name, bool enable_progress )
  {
    TEAGN_TRUE_OR_RETURN( in_raster.isActive(),
      "Inactive Input Raster" );
    TEAGN_TRUE_OR_RETURN( in_raster->params().status_ != 
      TeRasterParams::TeNotReady, "Input not Ready" );
    TEAGN_TRUE_OR_RETURN( file_name.size() != 0,
      "Invalid file name" );

    TeRasterParams temp_params = in_raster->params();

    temp_params.mode_ = 'c';
    temp_params.fileName_ = file_name;
    temp_params.decoderIdentifier_ = "TIF";
    
    /* Creating disk output raster */
    
    TeRaster outRaster( temp_params );

    TEAGN_TRUE_OR_RETURN( outRaster.init(),
      "Unable to init GeoTIFF Raster" );
      
    const unsigned int nlines = outRaster.params().nlines_;
    const unsigned int ncols = outRaster.params().ncols_;
    const unsigned int nbands = outRaster.params().nBands();
    unsigned int line = 0 , col = 0 , band = 0;
    double value = 0.0;
    TeRaster& ir = (*in_raster);
    
    TePDIPIManager progress( "Generating GeoTIFF raster", 
      nlines, enable_progress );      
    
    for( line = 0 ; line < nlines ; ++line )
    {
      for( band = 0 ; band < nbands ; ++band )
      {
        for( col = 0 ; col < ncols ; ++col )
        {
          ir.getElement( col, line, value, band );
          outRaster.setElement( col, line, value, band );
        }
      }
       
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" )
    }
      
    return true;
  }  
  
  bool TeRaster2RGBGeotiff(
    const TePDITypes::TePDIRasterPtrType& in_raster,
    const std::string& file_name, bool enable_progress )
  {
    TEAGN_TRUE_OR_RETURN( in_raster.isActive(),
      "Inactive Input Raster" );
    TEAGN_TRUE_OR_RETURN( in_raster->params().status_ != 
      TeRasterParams::TeNotReady, "Input not Ready" );
    TEAGN_TRUE_OR_RETURN( in_raster->params().nBands() == 3,
      "Invalid input raster" );      
    TEAGN_TRUE_OR_RETURN( file_name.size() != 0,
      "Invalid file name" );

    TeRasterParams temp_params = in_raster->params();
    temp_params.setPhotometric( TeRasterParams::TeRGB, -1 );
    temp_params.mode_ = 'c';
    temp_params.fileName_ = file_name;
    temp_params.decoderIdentifier_ = "TIF";
    
    /* Creating disk output raster */
    
    TeRaster outRaster( temp_params );
    TEAGN_TRUE_OR_RETURN( outRaster.init(),
      "Unable to init GeoTIFF Raster" );
      
    const unsigned int nlines = outRaster.params().nlines_;
    const unsigned int ncols = outRaster.params().ncols_;
    const unsigned int nbands = outRaster.params().nBands();
    unsigned int line = 0 , col = 0 , band = 0;
    double value = 0.0;
    TeRaster& ir = (*in_raster);
    
    TePDIPIManager progress( "Generating RGB GeoTIFF raster", 
      nlines, enable_progress );      
    
    for( line = 0 ; line < nlines ; ++line )
    {
      for( band = 0 ; band < nbands ; ++band )
      {
        for( col = 0 ; col < ncols ; ++col )
        {
          ir.getElement( col, line, value, band );
          outRaster.setElement( col, line, value, band );
        }
      }
       
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" )
    }
      
    return true;
  }  
  
  bool TeRaster2Jpeg(
    const TePDITypes::TePDIRasterPtrType& in_raster,
    const std::string& file_name, bool enable_progress,
    unsigned int qualityFactor )
  {
    TEAGN_TRUE_OR_RETURN( in_raster.isActive(),
      "Inactive Input Raster" );
    TEAGN_TRUE_OR_RETURN( in_raster->params().status_ != 
      TeRasterParams::TeNotReady, "Input not Ready" );
    TEAGN_TRUE_OR_RETURN( file_name.size() != 0,
      "Invalid file name" );
      
    // Create a decoder instance

    TeRasterParams temp_params = in_raster->params();

    temp_params.mode_ = 'c';
    temp_params.fileName_ = file_name;
    temp_params.decoderIdentifier_ = "JPEG";
    
    TeDecoderJPEG* decPtr = dynamic_cast< TeDecoderJPEG* >(
      TeDecoderFactory::make( "JPEG", temp_params ) );
    TEAGN_TRUE_OR_RETURN( decPtr != 0, "Unable to build a decoder instance" );    
    decPtr->setQualityFactor( qualityFactor );
    
    /* Creating disk output raster */
    
    TeRaster outRaster;
    outRaster.setDecoder( decPtr );

    TEAGN_TRUE_OR_RETURN( outRaster.init(),
      "Unable to init Jpeg Raster" );
      
    // Copying data
      
    const unsigned int nlines = outRaster.params().nlines_;
    const unsigned int ncols = outRaster.params().ncols_;
    const unsigned int nbands = outRaster.params().nBands();
    unsigned int line = 0 , col = 0 , band = 0;
    double value = 0.0;
    TeRaster& ir = (*in_raster);
    
    TePDIPIManager progress( "Generating Jpeg raster", 
      nlines, enable_progress );      
    
    for( line = 0 ; line < nlines ; ++line )
    {
      for( band = 0 ; band < nbands ; ++band )
      {
        for( col = 0 ; col < ncols ; ++col )
        {
          ir.getElement( col, line, value, band );
          outRaster.setElement( col, line, value, band );
        }
      }
       
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" )
    }
      
    return true;
  }


  bool TeCopyRasterPixels(
    TePDITypes::TePDIRasterPtrType& source_raster,
    unsigned int source_band,
    TePDITypes::TePDIRasterPtrType& target_raster,
    unsigned int target_band,
    bool discretize_levels,
    bool trunc_levels )
  {
    TEAGN_TRUE_OR_RETURN( source_raster.isActive(),
      "Inactive Input Raster" );
    TEAGN_TRUE_OR_RETURN( target_raster.isActive(),
      "Inactive Output Raster" );
    TEAGN_TRUE_OR_RETURN( source_raster->params().status_ != TeRasterParams::TeNotReady,
      "Input not Ready" );
    TEAGN_TRUE_OR_RETURN( target_raster->params().status_ == TeRasterParams::TeReadyToWrite,
      "OutPut not Ready" );

    TeRasterParams& source_params = source_raster->params();
    TeRasterParams& target_params = target_raster->params();

    TEAGN_TRUE_OR_RETURN( (int)source_band < source_params.nBands(),
      "Invalid source band" );
    TEAGN_TRUE_OR_RETURN( (int)target_band < target_params.nBands(),
      "Invalid target band" );
    TEAGN_TRUE_OR_RETURN( (
      ( source_params.nlines_ == target_params.nlines_ ) &&
      ( source_params.ncols_ == target_params.ncols_ ) ),
      "Input and Output Raster's have different dimentions" );
      
    /* Dummy support */
    
    const bool source_raster_uses_dummy = source_raster->params().useDummy_;
    double source_raster_dummy = 0;
    if( source_raster_uses_dummy ) {
      source_raster_dummy = source_raster->params().dummy_[ source_band ];
    }
    
    const bool target_raster_uses_dummy = target_raster->params().useDummy_;
    double target_raster_dummy = 0;
    if( target_raster_uses_dummy ) {
      target_raster_dummy = target_raster->params().dummy_[ target_band ];
    } else if( source_raster_uses_dummy ) {
      target_raster_dummy = source_raster_dummy;
    }

    /* Defining the valid values range base on the output raster pixel type */

    double min_value = 0;
    double max_value = 0;

    TEAGN_TRUE_OR_RETURN(
      TeGetRasterMinMaxBounds( target_raster, target_band, min_value, max_value ),
      "Unable to get raster level bounds" );

    /* Pixel by pixel copy loop */

    double pixel_value;

    for( int line = 0 ; line < source_params.nlines_ ; ++line ) {
      for( int column = 0 ; column < source_params.ncols_ ; ++column ) {
        if( source_raster->getElement( column, line, pixel_value, 
          source_band ) ) {
          
          if( discretize_levels ) {
            pixel_value = TeRound( pixel_value );
          }

          if( trunc_levels ) {
            if( pixel_value < min_value ) {
              pixel_value = min_value;
            } else if( pixel_value > max_value ) {
              pixel_value = max_value;
            }
          }

          TEAGN_TRUE_OR_RETURN( target_raster->setElement( column, line,
            pixel_value, target_band ), "Pixel copy error" );
        } else {
          TEAGN_TRUE_OR_RETURN( source_raster_uses_dummy,
            "Unable to read from input raster" );
            
          TEAGN_TRUE_OR_RETURN( target_raster->setElement( column, line,
            target_raster_dummy, target_band ), "Pixel copy error" );          
        }
      }
    }

    return true;
  }


  bool TeCopyRasterBands(
    TePDITypes::TePDIRasterPtrType& source_raster,
    TePDITypes::TePDIRasterPtrType& target_raster,
    bool discretize_levels,
    bool trunc_levels )
  {
    TEAGN_TRUE_OR_RETURN( source_raster.isActive(),
      "Inactive Input Raster" );
    TEAGN_TRUE_OR_RETURN( target_raster.isActive(),
      "Inactive Output Raster" );
    TEAGN_TRUE_OR_RETURN( source_raster->params().status_ != TeRasterParams::TeNotReady,
      "Input not Ready" );
    TEAGN_TRUE_OR_RETURN( target_raster->params().status_ == TeRasterParams::TeReadyToWrite,
      "OutPut not Ready" );

    TeRasterParams& source_params = source_raster->params();
    TeRasterParams& target_params = target_raster->params();

    TEAGN_TRUE_OR_RETURN(
      ( ( source_params.nBands() == target_params.nBands() ) &&
      ( source_params.nlines_ == target_params.nlines_ ) &&
      ( source_params.ncols_ == target_params.ncols_ ) ),
      "Input and Output Raster's have different dimentions" );

    for( int band = 0 ; band < source_params.nBands() ; ++band ) {
      TEAGN_TRUE_OR_RETURN( TeCopyRasterPixels( source_raster, band, target_raster,
        band, discretize_levels,trunc_levels ), "Unable to copy band " +
        Te2String( band ) );
    }

    return true;
  }

  
  bool TeGetRasterMinMaxBounds( 
    const TePDITypes::TePDIRasterPtrType& raster,
    unsigned int band, double& min, double& max )
  {
    TEAGN_TRUE_OR_RETURN( raster.isActive(),
      "Inactive Input Raster" );

    TeRasterParams& params = raster->params();

    TEAGN_TRUE_OR_RETURN( (int)band < params.nBands(),
      "Invalid source band" );

    switch( params.dataType_[ band ] ) {
      case TeBIT :
      {
        max = 1;
        min = 0;
        break;
      }
      case TeUNSIGNEDCHAR :
      {
        max = pow( 2., (double)sizeof( unsigned char ) * 8. ) - 1;
        min = 0;
        break;
      }
      case TeUNSIGNEDSHORT :
      {
        max = pow( 2., (double)sizeof( short ) * 8. ) - 1;
        min = 0;
        break;
      }
      case TeUNSIGNEDLONG :
      {
        max = pow( 2., (double)sizeof( unsigned long ) * 8. ) - 1;
        min = 0;
        break;
      }
      case TeCHAR :
      {
        max = ( pow( 2., (double)sizeof( char ) * 8. ) ) / 2;
        min = -1 * max;
        break;
      }
      case TeSHORT :
      {
        max = ( pow( 2., (double)sizeof( short ) * 8. ) ) / 2;
        min = -1 * max;
        break;
      }
      case TeINTEGER :
      {
        max = ( pow( 2., (double)sizeof( int ) * 8. ) ) / 2;
        min = -1 * max;
        break;
      }
      case TeLONG :
      {
        max = ( pow( 2., (double)sizeof( long ) * 8. ) ) / 2;
        min = -1 * max;
        break;
      }
      case TeFLOAT :
      {
        max = FLT_MAX;
        min = -1 * max;
        break;
      }
      case TeDOUBLE :
      {
        max = DBL_MAX;
        min = -1 * max;
        break;
      }
      default :
      {
        TEAGN_LOG_AND_RETURN( "Invalid raster pixel type" );
        break;
      }
    }

    return true;
  }
  
  
  unsigned long int TeGetRasterReqMem( TeRasterParams& params )
  {
    unsigned long int req_mem = 0;

    for( int band = 0 ; band < params.nBands() ; ++band ) {
      req_mem += params.elementSize( band ) * 
        params.nlines_ * params.ncols_;
    }
    
    req_mem += sizeof( unsigned short ) * params.lutr_.size();
    req_mem += sizeof( unsigned short ) * params.lutg_.size();
    req_mem += sizeof( unsigned short ) * params.lutb_.size();  
    
    return req_mem;
  }

  
  bool rasterCompare( TePDITypes::TePDIRasterPtrType& raster1,
    TePDITypes::TePDIRasterPtrType& raster2 )
  {
    TEAGN_TRUE_OR_THROW( raster1.isActive(), "Raster1 not active" );
    TEAGN_TRUE_OR_THROW( raster2.isActive(), "Raster2 not active" );
    TEAGN_TRUE_OR_THROW( raster1->params().status_ != TeRasterParams::TeNotReady,
      "Raster1 not ready" );
    TEAGN_TRUE_OR_THROW( raster2->params().status_ != TeRasterParams::TeNotReady,
      "Raster2 not ready" );

    TeRasterParams par1 = raster1->params();
    TeRasterParams par2 = raster2->params();

    if( ( par1.nBands() != par2.nBands() ) ||
        ( par1.ncols_ != par2.ncols_ ) ||
        ( par1.nlines_ != par2.nlines_ ) ) {

      return false;
    }

    for( int bandt = 0 ; bandt < par1.nBands() ; ++bandt ) {
      if( ( par1.dataType_[ bandt ] != par2.dataType_[ bandt ] ) ||
          ( par1.photometric_[ bandt ] != par2.photometric_[ bandt ] ) ) {

        return false;
      }
    }

    unsigned int lines = ( unsigned int ) par1.nlines_;
    unsigned int columns = ( unsigned int ) par1.ncols_;
    double value1 = 0;
    double value2 = 0;

    for( int band = 0 ; band < par1.nBands() ; ++band ) {
      for( unsigned int line = 0 ; line < lines ; ++line ) {
        for( unsigned int column = 0 ; column < columns ; ++column ) {
          if( raster1->getElement( column, line, value1, band ) !=
              raster2->getElement( column, line, value2, band ) ) {

            return false;
          }

          if( value1 != value2 ) {
            return false;
          }
        }
      }
    }

    return true;
  }
  
  
  bool IsFloatBand( const TePDITypes::TePDIRasterPtrType& raster, 
    unsigned int band  )
  {
    TEAGN_TRUE_OR_THROW( raster.isActive(),
      "Inactive Input Raster" );

    TeRasterParams& params = raster->params();

    TEAGN_TRUE_OR_THROW( (int)band < params.nBands(),
      "Invalid source band" );

    switch( params.dataType_[ band ] ) {
      case TeBIT :
      {
        return false;
        break;
      }
      case TeUNSIGNEDCHAR :
      {
        return false;
        break;
      }
      case TeUNSIGNEDSHORT :
      {
        return false;
        break;
      }
      case TeUNSIGNEDLONG :
      {
        return false;
        break;
      }
      case TeCHAR :
      {
        return false;
        break;
      }
      case TeSHORT :
      {
        return false;
        break;
      }
      case TeINTEGER :
      {
        return false;
        break;
      }
      case TeLONG :
      {
        return false;
        break;
      }
      case TeFLOAT :
      {
        return true;
        break;
      }
      case TeDOUBLE :
      {
        return true;
        break;
      }
      default :
      {
        TEAGN_LOG_AND_THROW( "Invalid raster pixel type" );
        return false;
        break;
      }
    }
  }
  
  
  void MapCoords2RasterIndexes( const TeCoord2D& g_in, 
    const TePDITypes::TePDIRasterPtrType& raster, TeCoord2D& g_out )
  {
    TEAGN_TRUE_OR_THROW( raster.isActive(), "Inactive raster" );
    
    g_out = raster->coord2Index( g_in );
  };
  
  
  void MapCoords2RasterIndexes( const TeBox& box_in, 
      const TePDITypes::TePDIRasterPtrType& raster, TeBox& box_out )
  {
    TEAGN_TRUE_OR_THROW( raster.isActive(), "Inactive raster" );
    
    TeBox temp_box( raster->coord2Index( box_in.lowerLeft() ),
      raster->coord2Index( box_in.upperRight() ) );
      
    box_out = temp_box;
  };  
  
  
  void MapRasterIndexes2Coords( const TeCoord2D& g_in, 
    const TePDITypes::TePDIRasterPtrType& raster, TeCoord2D& g_out )
  {
    TEAGN_TRUE_OR_THROW( raster.isActive(), "Inactive raster" );
      
    g_out = raster->index2Coord( g_in );
  };  
  
  
  void MapRasterIndexes2Coords( const TeBox& box_in, 
      const TePDITypes::TePDIRasterPtrType& raster, TeBox& box_out )
  {
    TEAGN_TRUE_OR_THROW( raster.isActive(), "Inactive raster" );
    
    TeBox temp_box( raster->index2Coord( box_in.lowerLeft() ),
      raster->index2Coord( box_in.upperRight() ) );
      
    box_out = temp_box;
  };   
  
  
  void makeSegmentSet( const TeLinearRing& g_in, TeLineSet& g_out )
  {
    g_out.clear();
    
    const int bound = g_in.size() - 1 ;
    
    for( int index = 0 ; index < bound ; ++index ) {
      TeLine2D line;
      
      line.add( g_in[ index ] );
      line.add( g_in[ index + 1 ] );
      
      g_out.add( line );
    }
  }
  
  
  bool IsDiscreteLut( const TePDITypes::TePDILutType& lut )
  {
    TEAGN_TRUE_OR_THROW( ( lut.size() > 0 ), "Emptry lut" );
  
    TePDITypes::TePDILutType::const_iterator it = lut.begin();
    const TePDITypes::TePDILutType::const_iterator it_end = lut.end();
  
    while( it != it_end ) {
      if( it->first != floor( it->first ) ) {
        return false;
      }
      
      ++it;
    }
    
    return true;
  }
  
  bool buildDetailedBox( TeRaster& raster, TePolygon& box )
  {
    box.clear();
    
    TEAGN_TRUE_OR_RETURN( raster.params().status_ != 
      TeRasterParams::TeNotReady, "raster inactive" );
    
    TeLine2D line2D;
    double lines = (double)raster.params().nlines_;
    double cols = (double)raster.params().ncols_;
    double offset = 0;
    
    /* Adding top box border ( direction left -> right )*/
    
    for( offset = 0 ; offset < cols ; ++offset ) {
      line2D.add( raster.index2Coord( TeCoord2D( offset, 0 ) ) );
    }
    
    /* Adding right box border ( direction up -> down) */
    
    for( offset = 1 ; offset < lines ; ++offset ) {
      line2D.add( raster.index2Coord( TeCoord2D( cols - 1, offset ) ) );
    }
    
    /* Adding bottom box border ( direction right ->left )*/
    
    for( offset = 1 ; offset < cols ; ++offset ) {
      line2D.add( raster.index2Coord( 
        TeCoord2D( cols - 1 - offset, lines - 1 ) ) );
    }      
    
    /* Adding left box border ( bottom -> top )*/
    
    for( offset = 1 ; offset < lines ; ++offset ) {
      line2D.add( raster.index2Coord( 
        TeCoord2D( 0, lines - 1 - offset ) ) );
    }
        
    TeLinearRing lr( line2D );    
    box.add( lr );
    
    return true;
  }
  
  bool buildDetailedBox( const TePDITypes::TePDIRasterPtrType& raster,
    TePolygon& box )
  {
    return buildDetailedBox( *raster, box );
  }
  
  bool buildDetailedBBox( TeRaster& raster, TePolygon& box )
  {
    box.clear();
    
    TEAGN_TRUE_OR_RETURN( raster.params().status_ != 
      TeRasterParams::TeNotReady, "raster inactive" );
    
    TeLine2D line2D;
    const double lines = (double)raster.params().nlines_;
    const double cols = (double)raster.params().ncols_;
    double offset = 0;
    
    /* Adding top box border ( direction left -> right )*/
    
    for( offset = 0 ; offset <= cols ; ++offset ) {
      line2D.add( raster.index2Coord( TeCoord2D( ( offset - 0.5 ), -0.5 ) ) );
    }
    
    /* Adding right box border ( direction up -> down) */
    
    for( offset = 0 ; offset < lines ; ++offset ) {
      line2D.add( raster.index2Coord( 
        TeCoord2D( ( cols - 0.5 ), offset + 0.5) ) );
    }
    
    /* Adding bottom box border ( direction right ->left )*/
    
    for( offset = 0 ; offset < cols ; ++offset ) {
      line2D.add( raster.index2Coord( 
        TeCoord2D( ( cols - 1.5 - offset ), ( lines - 0.5 ) ) ) );
    }      
    
    /* Adding left box border ( bottom -> top )*/
    
    for( offset = 0 ; offset < lines ; ++offset ) {
      line2D.add( raster.index2Coord( 
        TeCoord2D( -0.5, ( lines - 1.5 - offset ) ) ) );
    }
        
    TeLinearRing lr( line2D );    
    box.add( lr );
    
    return true;
  }  
  
  bool buildDetailedBBox( const TePDITypes::TePDIRasterPtrType& raster,
    TePolygon& box )
  {
    return buildDetailedBBox( *raster, box );
  }  
    
  bool reprojectRaster( TeRaster& input_raster,
    unsigned int outNLines, unsigned int outNCols, 
    TeProjection& target_projection,
    bool enableProgress, bool enableOptimization,
    TeRaster& output_raster )
  {
    TEAGN_TRUE_OR_RETURN( 
      input_raster.params().status_ != TeRasterParams::TeNotReady,
      "input_raster not ready" );
    TEAGN_TRUE_OR_RETURN( 
      output_raster.params().status_ != TeRasterParams::TeNotReady,
      "output_raster not ready" ); 
    TEAGN_TRUE_OR_RETURN( ( input_raster.params().projection() != 0 ),
      "Missing input_raster projection" )
    TEAGN_TRUE_OR_RETURN( 
      ( input_raster.params().projection()->name() != "NoProjection" ),
      "Invalid input_raster projection" );
      
    /* Building the remmaped box */
      
    TePolygon original_bbox_pol;
    TEAGN_TRUE_OR_RETURN( TePDIUtils::buildDetailedBBox( input_raster,
      original_bbox_pol ), 
      "Unable to build detailed box" );
        
    TePolygon reprojected_bbox_pol;  
    TeVectorRemap( original_bbox_pol, input_raster.params().projection(),
      reprojected_bbox_pol, &target_projection );    
      
    /* Remmaping raster */
  
    TeRasterParams output_raster_params = output_raster.params();
    
    output_raster_params.projection( &target_projection );
    
    output_raster_params.nBands( input_raster.params().nBands() );
    output_raster_params.setDataType( 
      input_raster.params().dataType_[ 0 ] );
    output_raster_params.setPhotometric( 
      input_raster.params().photometric_[ 0 ] );
    if( input_raster.params().useDummy_ ) {
      output_raster_params.setDummy( 
        input_raster.params().dummy_[ 0 ], -1 );
    }
    
    TeBox reprojected_bbox = reprojected_bbox_pol.box();
    output_raster_params.boundingBoxLinesColumns( reprojected_bbox.x1(), 
      reprojected_bbox.y1(), reprojected_bbox.x2(), reprojected_bbox.y2(), 
      outNLines, outNCols );
      
    TEAGN_TRUE_OR_RETURN( output_raster.init( output_raster_params ),
      "Output raster init error" );
      
    TeRasterTransform transf( &input_raster, &output_raster );
    transf.setTransfFunction( TeRasterTransform::TeBand2Band );
      
    TeRasterRemap remap( &input_raster,  &output_raster );
    remap.setTransformer( &transf );
    remap.setOptmizedReprojection( enableOptimization );
    
    TEAGN_TRUE_OR_RETURN( remap.apply( enableProgress ),
      "Raster remapping error" );
      
    return true;
  }
  
  bool reprojectRaster( TeRaster& input_raster,
    TeProjection& target_projection, double target_resx, double target_resy,
    bool enableProgress, bool enableOptimization,
    TeRaster& output_raster )
  {
    TEAGN_TRUE_OR_RETURN( 
      input_raster.params().status_ != TeRasterParams::TeNotReady,
      "input_raster not ready" );
    TEAGN_TRUE_OR_RETURN( 
      output_raster.params().status_ != TeRasterParams::TeNotReady,
      "output_raster not ready" ); 
    TEAGN_TRUE_OR_RETURN( ( input_raster.params().projection() != 0 ),
      "Missing input_raster projection" )
    TEAGN_TRUE_OR_RETURN( 
      ( input_raster.params().projection()->name() != "NoProjection" ),
      "Invalid input_raster projection" );
      
    /* Building the remmaped box */
      
    TePolygon original_bbox_pol;
    TEAGN_TRUE_OR_RETURN( TePDIUtils::buildDetailedBBox( input_raster,
      original_bbox_pol ), 
      "Unable to build detailed box" );
        
    TePolygon reprojected_bbox_pol;  
    TeVectorRemap( original_bbox_pol, input_raster.params().projection(),
      reprojected_bbox_pol, &target_projection );    
      
    // Number of output lines
    
    const unsigned int outNLines = (unsigned int)ceil(
      reprojected_bbox_pol.box().height() / target_resy );
    const unsigned int outNCols = (unsigned int)ceil(
      reprojected_bbox_pol.box().width() / target_resx );
      
    return reprojectRaster( input_raster, outNLines, outNCols,
      target_projection, enableProgress, enableOptimization, output_raster);
  }  
  
  short TeRelation( const TeCoord2D& c, const TePDITileIndexer& indexer )
  {
    TePDITileIndexer::TeTileSegIndex* tindex_ptr;
    const TePolygon& pol = indexer.getPol();
    
    indexer.getTile( c.y(), &tindex_ptr );
    
    if( tindex_ptr == 0 ) {
      return TeOUTSIDE;
    }
    
    register double ty, tx;
    register bool inside_flag = false;
    register int yflag0, yflag1;
  
    TeLinearRing::iterator vtx0, vtx1;
  
    tx = c.x();
    ty = c.y();
    
    for( unsigned int i = 0 ; i < tindex_ptr->size() ; ++i ) {
      TEAGN_DEBUG_CONDITION( ( ( *tindex_ptr )[ i ].first < pol.size() ),
        "Invalid ring index" );
      TEAGN_DEBUG_CONDITION( ( ( ( *tindex_ptr )[ i ].second + 1 ) < 
        pol[ ( *tindex_ptr )[ i ].first ].size() ),
        "Invalid segment index" );
    
      const TeCoord2D& vtx0 = 
        pol[ ( *tindex_ptr )[ i ].first ][ ( *tindex_ptr )[ i ].second ];
      const TeCoord2D& vtx1 = 
      pol[ ( *tindex_ptr )[ i ].first ][ ( *tindex_ptr )[ i ].second + 1 ];
      
      yflag0 = (vtx0.y() >= ty);
      
      yflag1 = (vtx1.y() >= ty);
    
      if(yflag0 != yflag1)
      {
        /* TODO - Check Boundary case */
      
        if(((vtx1.y() - ty) * (vtx0.x() - vtx1.x()) >=
              (vtx1.x() - tx) * (vtx0.y() - vtx1.y())) == yflag1)
        {
          inside_flag = !inside_flag ;
        }
      }
    
    }
    
    if( inside_flag ) {
      return TeINSIDE;
    }
    
    return TeOUTSIDE;
  }  
  
  
  void TeSegSetIntersection( const TeLineSet& ls1, const TeLineSet& ls2,
    TePointSet& ps )
  {
    ps.clear();
    
    const unsigned int ls1_size = ls1.size();
    const unsigned int ls2_size = ls2.size();
    
    unsigned int ls1_index = 0;
    unsigned int ls2_index = 0;
     
    bool line1_is_vertical = false;
    double line1_vertical_x_value = 0;
    double line1_m = 0;
    double line1_q = 0;
    
    double line2_m = 0;
    double line2_q = 0;    
    
    TeCoord2D candidate_pt;
    double canditate_pt_x = 0;
    bool candidate_pt_found = false;
    
    double line1_min_x = 0;
    double line1_min_y = 0;
    double line1_max_x = 0;
    double line1_max_y = 0;
    
    double line2_min_x = 0;
    double line2_min_y = 0;
    double line2_max_x = 0;
    double line2_max_y = 0;
    
    for( ls1_index = 0 ; ls1_index < ls1_size ; ++ls1_index ) {
      const TeLine2D& line1 = ls1[ ls1_index ];
      
      TEAGN_TRUE_OR_THROW( ( line1.size() == 2 ), "Invalid line size" );
      
      const TeCoord2D& line1_p1 = line1[ 0 ];
      const TeCoord2D& line1_p2 = line1[ 1 ];
      
      line1_min_x = MIN( line1_p1.x(), line1_p2.x() );
      line1_min_y = MIN( line1_p1.y(), line1_p2.y() );
      line1_max_x = MAX( line1_p1.x(), line1_p2.x() );
      line1_max_y = MAX( line1_p1.y(), line1_p2.y() );
      
      if( line1_p1.x() == line1_p2.x() ) {
        line1_is_vertical = true;
        
        line1_vertical_x_value = line1_p1.x();
      } else {
        line1_is_vertical = false;
        
        line1_m = ( line1_p1.y() - line1_p2.y() ) /
          ( line1_p1.x() - line1_p2.x() );
        line1_q = line1_p1.y() - ( line1_m * line1_p1.x() );
      }
    
      for( ls2_index = 0 ; ls2_index < ls2_size ; ++ls2_index ) {
        candidate_pt_found = false;
        const TeLine2D& line2 = ls2[ ls2_index ];
        
        TEAGN_TRUE_OR_THROW( ( line2.size() == 2 ), "Invalid line size" );
        
        const TeCoord2D& line2_p1 = line2[ 0 ];
        const TeCoord2D& line2_p2 = line2[ 1 ];
        
        if( line2_p1.x() == line2_p2.x() ) {
          if( ! line1_is_vertical ) {
            candidate_pt.setXY( line2_p1.x(), ( line1_m * line2_p1.x() ) +
              line1_q );
            candidate_pt_found = true;
          }
        } else {
          line2_m = ( line2_p1.y() - line2_p2.y() ) /
            ( line2_p1.x() - line2_p2.x() );
          line2_q = line2_p1.y() - ( line2_m * line2_p1.x() );        
        
          if( line1_is_vertical ) {
            candidate_pt.setXY( line1_vertical_x_value, 
              ( line2_m * line1_vertical_x_value ) + line2_q );
            candidate_pt_found = true;
          } else {
            if( line1_m != line2_m ) {
              canditate_pt_x = (  line2_q - line1_q ) / ( line1_m - line2_m );
              candidate_pt.setXY( canditate_pt_x, ( line2_m * canditate_pt_x ) + 
                line2_q );
              candidate_pt_found = true;
            }
          }
        }
        
        if( candidate_pt_found ) {
          line2_min_x = MIN( line2_p1.x(), line2_p2.x() );
          line2_min_y = MIN( line2_p1.y(), line2_p2.y() );
          line2_max_x = MAX( line2_p1.x(), line2_p2.x() );
          line2_max_y = MAX( line2_p1.y(), line2_p2.y() );        
          
          if( ( candidate_pt.x() >= line1_min_x  ) &&
            ( candidate_pt.y() >= line1_min_y  ) &&
            ( candidate_pt.x() <= line1_max_x  ) &&
            ( candidate_pt.y() <= line1_max_y  ) &&
            ( candidate_pt.x() >= line2_min_x  ) &&
            ( candidate_pt.y() >= line2_min_y  ) &&
            ( candidate_pt.x() <= line2_max_x  ) &&
            ( candidate_pt.y() <= line2_max_y  ) ) {
            
            ps.add( candidate_pt );
          }
          
        }
      }
    }
  
  }
  
  TeDataType chooseBestPixelType( TePDITypes::TePDIRasterVectorType& rasters )
  {
    TeDataType best_dt = TeUNSIGNEDCHAR;
    double best_min = 0;
    double best_max = 255;
  
    TePDITypes::TePDIRasterVectorType::iterator it = rasters.begin();
    TePDITypes::TePDIRasterVectorType::iterator it_end = rasters.end();
    
    double curr_min = 0;
    double curr_max = 0;
    
    unsigned int bands_number = 0;
    unsigned int curr_band = 0;
    
    while( it != it_end ) {
      bands_number = (*it)->params().nBands();
      
      curr_band = 0;
      
      while( curr_band < bands_number ) {
        if( TeGetRasterMinMaxBounds( *it, curr_band, curr_min, curr_max ) ) {
          if( curr_min == best_min ) {
            if( curr_max > best_max ) {
              best_dt = (*it)->params().dataType_[ curr_band ];
              best_max = curr_max;
              best_min = curr_min;
            }
          } else if( curr_max == best_max ) {
            if( curr_min < best_min ) {
              best_dt = (*it)->params().dataType_[ curr_band ];
              best_max = curr_max;
              best_min = curr_min;
            }
          } else {
            if( ( curr_min < best_min ) || ( curr_max > best_max ) ) {
              best_dt = (*it)->params().dataType_[ curr_band ];
              best_max = curr_max;
              best_min = curr_min;
            }
          }
        }
        
        ++curr_band;
      }
      
      ++it;
    }
    
    return best_dt;
  }
  
  
  void getBandMinMaxValues( const TePDITypes::TePDIRasterPtrType& raster, 
    unsigned int channel, bool progress_enabled, unsigned int sample_step,  
    double& min, double& max )
  {
    TEAGN_TRUE_OR_THROW( raster.isActive(),
      "Inactive Input Raster" );

    TeRasterParams& params = raster->params();

    TEAGN_TRUE_OR_THROW( (int)channel < params.nBands(),
      "Invalid input channel" );  
  
    if ( params.decoderIdentifier_ == "DB" ) {
      
      min = params.vmin_[ channel ];
      max = params.vmax_[ channel ];
    } else {
      min = DBL_MAX;
      max = ( -1.0 ) * DBL_MAX;  
      
      const unsigned int in_lines_number = params.nlines_;
      const unsigned int in_columns_number = params.ncols_;
      const bool use_dummy = params.useDummy_;
  
      TePDIPIManager progress( "Guessing channel bounds...", 
        (unsigned int)ceil( ( (double)in_lines_number ) / 
        ( (double)( sample_step + 1 ) ) ), progress_enabled );      
    
      unsigned int rasterline = 0;
      unsigned int rastercolumn = 0;
      double current_raster_level = 0;
      TeRaster& in_raster_ref = *(raster.nakedPointer());
      
      for( rasterline = 0 ; rasterline < in_lines_number ; 
        rasterline += ( 1 + sample_step ) ) {
        
        for( rastercolumn = 0 ; rastercolumn < in_columns_number ; 
          rastercolumn += ( 1 + sample_step ) ) 
        {  
          if( in_raster_ref.getElement( rastercolumn, rasterline,
              current_raster_level, channel ) ) {
              
            if( current_raster_level > max ) {
              max = current_raster_level;
            }
            if( current_raster_level < min ) {
              min = current_raster_level;
            }
          } else {
            TEAGN_TRUE_OR_THROW( use_dummy,
              "Unable to read from input raster at line=" +
              Te2String( rasterline ) + " column=" +
              Te2String( rastercolumn ) + "channel=" +
              Te2String( channel ) );
          }
        }
        
        progress.Increment();
      }
    }      
  }
  
  
  bool convert2MultiBand( const TePDITypes::TePDIRasterPtrType& in_raster,
    bool progress_enabled, TePDITypes::TePDIRasterPtrType& out_raster )
  {
    TEAGN_TRUE_OR_THROW( in_raster.isActive(), 
      "in_raster pointer inactive" );
    TEAGN_TRUE_OR_THROW( ( in_raster->params().projection() != 0 ), 
      "in_raster do not have a projection" );
      
    /* Checking if we need a conversion */
      
    int in_raster_band = 0;
    bool conversion_required = false;
        
    for( in_raster_band = 0 ; in_raster_band < in_raster->params().nBands() ;
      ++in_raster_band ) {
      
      if( ( in_raster->params().photometric_[ in_raster_band ] !=
        TeRasterParams::TeMultiBand ) &&
        ( in_raster->params().photometric_[ in_raster_band ] !=
        TeRasterParams::TeRGB ) ) {
      
        conversion_required = true;
        in_raster_band = in_raster->params().nBands();
      }
    }
  
    if( conversion_required ) {
      TeRasterParams out_raster_params;
    
      if( out_raster.isActive() ) {
        TEAGN_TRUE_OR_THROW( ( out_raster->params().status_ != 
          TeRasterParams::TeNotReady ), "out_raster not ready" );
          
        out_raster_params = out_raster->params();
      }
      
      if( in_raster->params().photometric_[ 0 ] == 
        TeRasterParams::TePallete ) { 
                 
        out_raster_params.nBands( 3 );
      } else {
        out_raster_params.nBands( in_raster->params().nBands() );
      }
                  
      TeSharedPtr< TeProjection > proj( TeProjectionFactory::make( 
        in_raster->projection()->params() ) );          
      out_raster_params.projection( proj.nakedPointer() );      
      
      out_raster_params.boxResolution( in_raster->params().box().x1(), 
        in_raster->params().box().y1(), in_raster->params().box().x2(), 
        in_raster->params().box().y2(), 
        in_raster->params().resx_, in_raster->params().resy_ );      
      
      out_raster_params.setPhotometric( TeRasterParams::TeMultiBand );
      
      out_raster_params.setDataType( in_raster->params().dataType_[ 0 ], -1 );
      
      if( in_raster->params().useDummy_ ) {
        out_raster_params.setDummy( in_raster->params().dummy_[ 0 ], -1 );
      } else {
        out_raster_params.useDummy_ = false;
      }
        
      if( out_raster.isActive() ) {
        TEAGN_TRUE_OR_RETURN( out_raster->init( out_raster_params ),
          "Output raster init error" );
      } else {
        TEAGN_TRUE_OR_RETURN( TeAllocRAMRaster( out_raster,
          out_raster_params, TePDIUtilsAutoMemPol ),
          "Output raster allocation error" );
      }
          
      /* Creating the transformer instance */
        
      TeRasterTransform transformer( in_raster.nakedPointer(), 
        out_raster.nakedPointer() );
          
      if( in_raster->params().photometric_[ 0 ] == 
        TeRasterParams::TePallete ) {
          
        transformer.setTransfFunction( TeRasterTransform::TePall2Three );
      } else {
        transformer.setTransfFunction( TeRasterTransform::TeBand2Band );
      }
        
      /* Creating the remapper instance */
          
      TeRasterRemap remapper( in_raster.nakedPointer(), 
        out_raster.nakedPointer(), progress_enabled );
        
      remapper.setTransformer( &transformer );
      
      TEAGN_TRUE_OR_RETURN( remapper.apply( progress_enabled),
        "Raster remapping error" );
    } else {
      out_raster = in_raster;
    }
    
    return true;
  }
  
  bool compareGeometryPoints( const TeLinearRing& geom1, 
    const TeLinearRing& geom2 )
  {
    unsigned int geom1_size = geom1.size();
    
    if( geom1_size == geom2.size() ) {
    
      unsigned int index2 = 0;
      bool point_found = false;
      
      for( unsigned int index1 = 0 ; index1 < geom1_size ; ++index1 ) {
        point_found = false;
        
        for( index2 = 0 ; index2 < geom1_size ; ++index2 ) {
          if( geom1[ index1 ] == geom2[ index2 ] ) {
            point_found = true;
            break;    
          }
        }
        
        if( ! point_found ) {
          return false;
        }
      }
      
      return true;
    } else {
      return false;
    }
  }
  

  bool resampleRasterByRes( const TePDITypes::TePDIRasterPtrType& input_raster,
    const TePDITypes::TePDIRasterPtrType& output_raster,
    double x_resolution_ratio, double y_resolution_ratio, 
    TePDIInterpolator::InterpMethod interpol, 
    bool enable_progress ) 
  {
      
    TEAGN_TRUE_OR_RETURN( input_raster.isActive(), "Inactive input raster" );
    TEAGN_TRUE_OR_RETURN( 
      input_raster->params().status_ != TeRasterParams::TeNotReady,
      "Input raster not ready" )
    
    TEAGN_TRUE_OR_RETURN( output_raster.isActive(), 
      "Inactive output raster" );
    TEAGN_TRUE_OR_RETURN( 
      output_raster->params().status_ != TeRasterParams::TeNotReady,
      "Output raster not ready" )
      
    TEAGN_TRUE_OR_RETURN( ( x_resolution_ratio > 0.0 ), "Invalid X ratio" )
    TEAGN_TRUE_OR_RETURN( ( y_resolution_ratio > 0.0 ), "Invalid X ratio" )
    
    unsigned int out_lines = TeRound( 
      ( (double)input_raster->params().nlines_ ) / 
      y_resolution_ratio );
    unsigned int out_cols = TeRound( 
      ( (double)input_raster->params().ncols_ ) / 
      x_resolution_ratio );      
    
    return resampleRasterByLinsCols( input_raster, output_raster, 
      out_lines, out_cols, enable_progress, interpol );
  }
  
  bool resampleRasterByLinsCols( 
    const TePDITypes::TePDIRasterPtrType& input_raster,
    const TePDITypes::TePDIRasterPtrType& output_raster,
    unsigned int out_lines, 
    unsigned int out_cols,
    bool enable_progress, 
    TePDIInterpolator::InterpMethod interpol ) 
  {
    std::vector< unsigned int > inputBands;
    
    for( int idx = 0 ; idx < input_raster->params().nBands() ;
      ++idx )
    {
      inputBands.push_back( (unsigned int)idx );
    }
    
    return resampleRasterByLinsCols( input_raster, inputBands,
      out_lines, out_cols, enable_progress, interpol, output_raster );
  }


  bool resampleRasterByLinsCols( 
    const TePDITypes::TePDIRasterPtrType& input_raster,
    const std::vector< unsigned int >& inputBands,
    unsigned int out_lines, 
    unsigned int out_cols, 
    bool enable_progress,
    TePDIInterpolator::InterpMethod interpol,
    const TePDITypes::TePDIRasterPtrType& output_raster ) 
  {
      
    TEAGN_TRUE_OR_THROW( input_raster.isActive(), "Inactive input raster" );
    TEAGN_TRUE_OR_RETURN( 
      input_raster->params().status_ != TeRasterParams::TeNotReady,
      "Input raster not ready" )
    
    TEAGN_TRUE_OR_THROW( output_raster.isActive(), 
      "Inactive output raster" );
    TEAGN_TRUE_OR_RETURN( 
      output_raster->params().status_ != TeRasterParams::TeNotReady,
      "Output raster not ready" )
      
    TEAGN_TRUE_OR_RETURN( ( out_lines > 0 ), "Invalid out_lines" )
    TEAGN_TRUE_OR_RETURN( ( out_cols > 0 ), "Invalid out_cols" )
    
    /* Reseting the output raster to the new geometry */
    
    {
      TeRasterParams new_output_raster_params = output_raster->params();
      
      new_output_raster_params.nBands( inputBands.size() );
      new_output_raster_params.setPhotometric( 
        input_raster->params().photometric_[ 0 ], -1 );
      new_output_raster_params.boxLinesColumns( 
        input_raster->params().box().x1(), 
        input_raster->params().box().y1(), 
        input_raster->params().box().x2(), 
        input_raster->params().box().y2(), 
        out_lines, out_cols );
      new_output_raster_params.projection( 
        input_raster->params().projection() );
        
      TEAGN_TRUE_OR_RETURN( output_raster->init( 
        new_output_raster_params ),
        "Output raster reset error" )
  
      TEAGN_DEBUG_CONDITION( 
        ( output_raster->params().nlines_ == (int)out_lines ),
        "Invalid nlines_" )
      TEAGN_DEBUG_CONDITION( 
        ( output_raster->params().ncols_ == (int)out_cols ),
        "Invalid ncols_" )
    }
      
    /* interpolating pixel values */
    
    const TeRasterParams& output_raster_params = 
      output_raster->params();
    
    const unsigned int out_bands = (unsigned int)
      output_raster_params.nBands();
    
    double in_col = 0;
    double in_line = 0;
    unsigned int out_col = 0;
    unsigned int out_line = 0;
    unsigned int out_band = 0;
    double value = 0;
    const double y_resolution_ratio = 
      ( (double) input_raster->params().nlines_ ) /
      ( (double) out_lines );
    const double x_resolution_ratio = 
      ( (double) input_raster->params().ncols_ ) /
      ( (double) out_cols );
    
    double dummy_value = 0;
    if( input_raster->params().useDummy_ ) {
      dummy_value = input_raster->params().dummy_[ 0 ];
    }
    
    TeRaster& output_raster_ref = *output_raster;
    
    TePDIInterpolator interpolator;
    TEAGN_TRUE_OR_RETURN( interpolator.reset( input_raster, 
      interpol,
      dummy_value), "Interpolator reset error" )
    unsigned int inputBand = 0;  
    TePDIPIManager progress( "Resampling raster", 
      ( out_bands * out_lines ), enable_progress );
    double outMax = 0;
    double outMin = 0;      
    
    for( out_band = 0 ; out_band < out_bands ; ++out_band ) 
    {
      TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds( output_raster,
        out_band, outMin, outMax ), "Internal error" );
          
      inputBand = inputBands[ out_band ];
      
      for( out_line = 0 ; out_line < out_lines ; ++out_line ) {
        in_line = ( (double)out_line) * y_resolution_ratio;
      
        for( out_col = 0 ; out_col < out_cols ; ++out_col ) {
          in_col = ( (double)out_col) * x_resolution_ratio;
        
          interpolator.interpolate( in_line, in_col, inputBand, 
            value );
            
          value = MIN( value, outMax );
          value = MAX( value, outMin );                
      
          TEAGN_TRUE_OR_RETURN( output_raster_ref.setElement( 
            out_col, out_line, value, out_band ), 
            "Output raster writing error" )
        }
        
        TEAGN_FALSE_OR_RETURN( progress.Increment(), 
          "Canceled by user" );
      }
    }
    
    return true;
  }
  
  
  bool composeRaster( 
    const TePDITypes::TePDIRasterVectorType& input_rasters,
    const std::vector< unsigned int >& input_channels,
    TePDITypes::TePDIRasterPtrType& output_raster,
    bool enable_progress )
  {
    TEAGN_TRUE_OR_THROW( ( input_rasters.size() == input_channels.size() ),
      "Invalid input channels number" )
    TEAGN_TRUE_OR_THROW( output_raster.isActive(), 
      "Inactive output raster" );      
  
    if( input_rasters.size() == 0 )
    { 
      output_raster.reset();
      return true;
    }
    else
    {
      /* Checking reference raster */
      
      TEAGN_TRUE_OR_THROW( input_rasters[ 0 ].isActive(),
        "Invalid input reference raster pointer" )
            
      TeRasterParams ref_params = input_rasters[ 0 ]->params();
      
      TEAGN_TRUE_OR_THROW( 
        ( ref_params.status_ != TeRasterParams::TeNotReady ),
        "Invalid input reference raster" )
      
      /* creating the output parameters */
      
      {
        TeRasterParams newoutpars = output_raster->params();
        TeBox ref_bbox = ref_params.boundingBox();
        newoutpars.boundingBoxLinesColumns( 
          ref_bbox.x1(), 
          ref_bbox.y1(), 
          ref_bbox.x2(), 
          ref_bbox.y2(), 
          ref_params.nlines_, ref_params.ncols_ );
        newoutpars.nBands( input_rasters.size() );
        newoutpars.projection( ref_params.projection() );
        newoutpars.setPhotometric( TeRasterParams::TeMultiBand, -1 );
        
        for( unsigned int input_channels_idx = 0 ; 
          input_channels_idx < input_channels.size() ; ++input_channels_idx )
        {
          /* Checking input raster pointers */
          
          TEAGN_TRUE_OR_THROW( 
            ( input_rasters[ input_channels_idx ].isActive() ),
            "Invalid input raster pointer" )
            
          /* Checking input raster status */
          
          TEAGN_TRUE_OR_THROW( 
            ( input_rasters[ input_channels_idx ]->params().status_ != 
            TeRasterParams::TeNotReady ),
            "Invalid input raster" )            
                      
          /* Checking input_channels indexes */
          
          const int& curr_input_channel = input_channels[ 
            input_channels_idx ];
          
          TEAGN_TRUE_OR_THROW( 
            ( curr_input_channel < 
            input_rasters[ input_channels_idx ]->nBands() ),
            "Invalid band index" )
            
          /* Checking input raster lines and columns */
          
          TEAGN_TRUE_OR_THROW( 
            ( input_rasters[ input_channels_idx ]->params().nlines_ == 
            ref_params.nlines_ ),
            "Invalid input raster lines number" )     
          TEAGN_TRUE_OR_THROW( 
            ( input_rasters[ input_channels_idx ]->params().ncols_ == 
            ref_params.ncols_ ),
            "Invalid input raster columns number" )                                 
          
          /* Seting the output data type */
          
          newoutpars.setDataType(
            input_rasters[ input_channels_idx ]->params().dataType_[ 
            curr_input_channel ], (int)input_channels_idx );
        }
        
        /* Reseting output raster */
        
        TEAGN_TRUE_OR_RETURN( output_raster->init( newoutpars ),
          "Error initiating output raster" )
          
        /* Copying data */
        
        const unsigned int lines_per_raster = (unsigned int)
          ref_params.nlines_;
        const unsigned int cols_per_raster = (unsigned int)
          ref_params.ncols_;
        TeRaster& out_raster_ref = *output_raster;
        unsigned int curr_line = 0;
        unsigned int curr_col = 0;
        double value = 0;
          
        TePDIPIManager progress( "Composing raster", 
          input_channels.size() * lines_per_raster, enable_progress );            
        
        for( unsigned int input_channels_idx = 0 ; 
          input_channels_idx < input_channels.size() ; ++input_channels_idx )
        {
          const unsigned int& curr_input_channel = input_channels[ 
            input_channels_idx ];
          TeRaster& cur_in_raster = 
            *( input_rasters[ input_channels_idx ] );
            
          for( curr_line = 0 ; curr_line < lines_per_raster ; ++curr_line )
          {
            for( curr_col = 0 ; curr_col < cols_per_raster ; ++curr_col )
            {
              cur_in_raster.getElement( curr_col, curr_line, value, 
                curr_input_channel );
              TEAGN_TRUE_OR_RETURN( out_raster_ref.setElement( curr_col, 
                curr_line, value, input_channels_idx ),
                "Error writting output raster" );
            }
            
            TEAGN_FALSE_OR_RETURN( progress.Increment(),
              "Canceled by the user" )
          }
        }        
      }
      
      return true;
    }
  }
  
  bool decomposeRaster( TeRaster& inputRaster,
    const std::vector< unsigned int >& inputChannels,
    TePDITypes::TePDIRasterVectorType& outputRasters,
    bool enableProgress )
  {
    TEAGN_TRUE_OR_RETURN( inputRaster.params().status_ != 
      TeRasterParams::TeNotReady,
      "Input raster not ready" )
      
    outputRasters.clear();
      
    TeRasterParams newRasterParams = inputRaster.params();
    newRasterParams.nBands( 1 );
    const unsigned int nLines = (unsigned int)inputRaster.params().nlines_;
    const unsigned int nCols = (unsigned int)inputRaster.params().ncols_;
    unsigned int col = 0;
    unsigned int line = 0;
    double value = 0;
    
    TePDIPIManager progress( "Decomposing raster", inputChannels.size() * 
      inputRaster.params().nlines_, enableProgress ); 
      
    for( unsigned int iCIdx = 0 ; iCIdx < inputChannels.size() ; ++iCIdx )
    {
      const unsigned int& inputChannel = inputChannels[ iCIdx ];
      TEAGN_TRUE_OR_RETURN( ((int)inputChannel) < inputRaster.params().nBands(),
        "Invalid input channel " + Te2String( inputChannel ) );
        
      newRasterParams.setDataType( inputRaster.params().dataType_[ 
        inputChannel ], -1 );
        
      TePDITypes::TePDIRasterPtrType newRasterPtr;
      TEAGN_TRUE_OR_RETURN( TeAllocRAMRaster( newRasterParams,
        newRasterPtr ), "Raster alloc error" )
        
      TeRaster& outRaster = (*newRasterPtr);
        
      for( line = 0 ; line < nLines ; ++line )
      {
        for( col = 0 ; col < nCols ; ++col )
        {
          inputRaster.getElement( col, line, value, inputChannel );
          TEAGN_TRUE_OR_RETURN( outRaster.setElement( col, line, value, 
            0 ), "Error writing raster" );
        }
        
        TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" )
      }
      
      outputRasters.push_back( newRasterPtr );
    }
    
    return true;
  }
  
  bool loadRaster( const std::string& fileName, 
      TePDITypes::TePDIRasterPtrType& memRasterPtr, bool enable_progress )
  {
    TeRaster inRaster( fileName, 'r' );
    TEAGN_TRUE_OR_RETURN( inRaster.init(), "Input disk raster init error" );
    
    TEAGN_TRUE_OR_RETURN( TeAllocRAMRaster( memRasterPtr, inRaster.params(), 
      TePDIUtilsAutoMemPol ), "Memory raster allocation error" );
    
    unsigned int band = 0;
    const unsigned int bandsN = (unsigned int)inRaster.params().nBands();
    unsigned int line = 0;
    const unsigned int linesN = (unsigned int)inRaster.params().nlines_;
    unsigned int col = 0;
    const unsigned int colsN = (unsigned int)inRaster.params().ncols_;
    TeRaster& memRaster = *memRasterPtr;
    double value = 0;
    
    TePDIPIManager progress( "Loading raster", 
      bandsN * linesN, enable_progress );       
    
    for( band = 0 ; band < bandsN ; ++band )
      for( line = 0 ; line < linesN ; ++line )
      {
        for( col = 0 ; col < colsN ; ++col )
        {
          inRaster.getElement( col, line, value, band );
          memRaster.setElement( col, line, value, band );
        }
        
        TEAGN_FALSE_OR_RETURN( progress.Increment(),  
          "Canceled by the user" );
      }
    
    return true;
  }
  
} // namespace TeUtils

