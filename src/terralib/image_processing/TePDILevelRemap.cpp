#include "TePDILevelRemap.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"
#include "TePDIStatistic.hpp"
#include "TePDIMatrix.hpp"

#include "../kernel/TeUtils.h"
#include "../kernel/TeDefines.h"

#include <math.h>
#include <float.h>

TePDILevelRemap::TePDILevelRemap()
{
}


TePDILevelRemap::~TePDILevelRemap()
{
}



void TePDILevelRemap::ResetState( const TePDIParameters& params )
{
  TePDIParameters dummy_params = params;

  histo_cache_.clear();
}


bool TePDILevelRemap::RemapLevels(
  TePDITypes::TePDIRasterPtrType& inRaster,
  remap_func_3_ptr_type remapp_func,
  int in_channel,
  int out_channel,
  double gain,
  double offset,
  bool normalize_output,
  TePDITypes::TePDIRasterPtrType& outRaster )
{
  TEAGN_TRUE_OR_THROW( inRaster.isActive(),
    "inRaster inactive" );
  TEAGN_TRUE_OR_THROW( outRaster.isActive(),
    "outRaster inactive" );
  TEAGN_TRUE_OR_THROW( 
    ( inRaster->params().status_ != TeRasterParams::TeNotReady ),
    "inRaster not ready" );
  TEAGN_TRUE_OR_THROW( 
    ( outRaster->params().status_ != TeRasterParams::TeNotReady ),
    "outRaster not ready" );
  TEAGN_CHECK_EQUAL( inRaster->params().nlines_,
    outRaster->params().nlines_,
    "Lines number mismatch between input and output image" );
  TEAGN_CHECK_EQUAL( inRaster->params().ncols_,
    outRaster->params().ncols_,
    "Columns number mismatch between input and output image" );
  TEAGN_TRUE_OR_THROW( in_channel < inRaster->nBands(), "Invalid input band" );
  TEAGN_TRUE_OR_THROW( out_channel < outRaster->nBands(), "Invalid output band" );
  
  if( normalize_output ) {
    TEAGN_TRUE_OR_THROW( 
      ( outRaster->params().dataType_[ out_channel ] != TeDOUBLE ),
      "Cannot apply normalization for TeDOUBLE pixel type output bands" );
  }
  
  TeRaster* inRasterNPtr = inRaster.nakedPointer();
  TeRaster* outRasterNPtr = outRaster.nakedPointer();
  const int in_raster_lines = inRaster->params().nlines_;
  const int in_raster_columns = inRaster->params().ncols_;  
  
  /* Guessing channel bounds */
  
  double in_channel_min_level = 0;
  double in_channel_max_level = 0;

  TEAGN_TRUE_OR_THROW( TePDIUtils::TeGetRasterMinMaxBounds(
    inRaster, in_channel, in_channel_min_level,
    in_channel_max_level ), 
    "Unable to get raster input channel level bounds" );  

  double out_channel_min_level = 0;
  double out_channel_max_level = 0;

  TEAGN_TRUE_OR_THROW( TePDIUtils::TeGetRasterMinMaxBounds(
    outRaster, out_channel, out_channel_min_level,
    out_channel_max_level ), 
    "Unable to get raster output channel level bounds" );
    
  /* Guessing dummy use */
  
  bool inRaster_uses_dummy = inRaster->params().useDummy_;
  double inRaster_dummy = 0;
  if( inRaster_uses_dummy ) {
    inRaster_dummy = inRaster->params().dummy_[ in_channel ];
  }
  
  bool outRaster_uses_dummy = outRaster->params().useDummy_;
  double outRaster_dummy = 0;
  if( outRaster_uses_dummy ) {
    outRaster_dummy = outRaster->params().dummy_[ out_channel ];
  }
  
  /* Level remapping */
  
  if( normalize_output ) {
    /* Guessing normalization factors */
  
    double norm_min_level = DBL_MAX;
    double norm_max_level = -1.0 * norm_min_level;
    double norm_factor = 0;
    double norm_off = 0;
    
    double current_level = 0;
    
    TePDIMatrix< double > mapped_matrix;
    TEAGN_TRUE_OR_THROW( mapped_matrix.Reset( in_raster_lines, 
      in_raster_columns, TePDIMatrix< double >::AutoMemPol ),
      "Mapped levels matrix reset error" );
    
    StartProgInt( "Guessing normalization factors...", in_raster_lines );
    
    int line = 0;
    int column = 0;
      
    for( line = 0 ; line < in_raster_lines ; ++line ) {
      TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
    
      for( column = 0 ; column < in_raster_columns ; ++column ) {
        if( ! inRasterNPtr->getElement( column, line, current_level, 
          in_channel ) ) {
          
          TEAGN_TRUE_OR_RETURN( inRaster_uses_dummy, "Raster read error" );
          
          current_level = inRaster_dummy;
        }
        
        current_level = remapp_func( current_level, gain, offset ); 
          
        mapped_matrix( line, column ) = current_level;
          
        if( current_level < norm_min_level ) {
          norm_min_level = current_level;
        } 
        if( current_level > norm_max_level ) {
          norm_max_level = current_level;
        }                  
      }
    }
       
    StopProgInt();
    
    norm_factor = ( out_channel_max_level - out_channel_min_level ) /
      ( norm_max_level - norm_min_level );
    norm_off = out_channel_min_level - norm_min_level;
    
    /* Output Level remapping */
    
    double output_level = 0;
    double matrix_level = 0;
    
    StartProgInt( "Remapping Levels...", in_raster_lines );
  
    for( line = 0 ; line < in_raster_lines ; ++line ) {
      TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
    
      for( column = 0 ; column < in_raster_columns ; ++column ) {
        matrix_level = mapped_matrix( line, column );
        output_level = ( matrix_level + norm_off ) * norm_factor;
        
        TEAGN_TRUE_OR_RETURN( outRasterNPtr->setElement( column, line,
          output_level, out_channel ),
          "Level remmaping error at " + Te2String( line ) +
          "," + Te2String( column ) );            
      }
    }
    
    StopProgInt();
  } else {
    /* Level remapping */
  
    double current_level;
    double output_level;
    
    StartProgInt( "Remapping Levels...", in_raster_lines );
  
    int line = 0;
    int column = 0;
    for( line = 0 ; line < in_raster_lines ; ++line ) {
      TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
    
      for( column = 0 ; column < in_raster_columns ; ++column ) {
        if( inRasterNPtr->getElement( column, line, current_level,
            in_channel ) ) {
  
          /* Finding mapped level by using the level remapping function */
  
          output_level = remapp_func( current_level, gain, offset );
  
          /* Level range filtering */
          
          if( output_level < out_channel_min_level ) {
            output_level = out_channel_min_level;
          } else if( output_level > out_channel_max_level ) {
            output_level = out_channel_max_level;
          }
  
          TEAGN_TRUE_OR_RETURN( outRasterNPtr->setElement( column, line,
            output_level, out_channel ),
            "Level remmaping error at " + Te2String( line ) +
            "," + Te2String( column ) );
        } else {
          TEAGN_TRUE_OR_RETURN( inRaster_uses_dummy, "Raster read error" );
            
          TEAGN_TRUE_OR_RETURN( outRasterNPtr->setElement( column, line,
            outRaster_dummy, out_channel ),
            "Level remmaping error at " + Te2String( line ) +
            "," + Te2String( column ) );          
        }
      }
    }
    
    StopProgInt();
  }
  
  return true;
}


bool TePDILevelRemap::RemapLevels(
  TePDITypes::TePDIRasterPtrType& inRaster1,
  TePDITypes::TePDIRasterPtrType& inRaster2,
  remap_func_4_ptr_type remapp_func,
  int in_channel1,
  int in_channel2,
  int out_channel,
  double gain,
  double offset,
  bool normalize_output,
  TePDITypes::TePDIRasterPtrType& outRaster )
{
  TEAGN_TRUE_OR_THROW( inRaster1.isActive(),
    "inRaster1 inactive" );
  TEAGN_TRUE_OR_THROW( inRaster2.isActive(),
    "inRaster2 inactive" );
  TEAGN_TRUE_OR_THROW( outRaster.isActive(),
    "outRaster inactive" );
  TEAGN_TRUE_OR_THROW( 
    ( inRaster1->params().status_ != TeRasterParams::TeNotReady ),
    "inRaster1 not ready" );
  TEAGN_TRUE_OR_THROW( 
    ( inRaster2->params().status_ != TeRasterParams::TeNotReady ),
    "inRaster2 not ready" );
  TEAGN_TRUE_OR_THROW( 
    ( outRaster->params().status_ != TeRasterParams::TeNotReady ),
    "outRaster not ready" );
  TEAGN_CHECK_EQUAL( inRaster1->params().nlines_,
    outRaster->params().nlines_,
    "Lines number mismatch between input1 and output image" );
  TEAGN_CHECK_EQUAL( inRaster1->params().ncols_,
    outRaster->params().ncols_,
    "Columns number mismatch between input1 and output image" );
  TEAGN_CHECK_EQUAL( inRaster2->params().nlines_,
    outRaster->params().nlines_,
    "Lines number mismatch between input2 and output image" );
  TEAGN_CHECK_EQUAL( inRaster2->params().ncols_,
    outRaster->params().ncols_,
    "Columns number mismatch between input2 and output image" );
  TEAGN_TRUE_OR_THROW( in_channel1 < inRaster1->nBands(), "Invalid input1 band" );
  TEAGN_TRUE_OR_THROW( in_channel2 < inRaster2->nBands(), "Invalid input2 band" );
  TEAGN_TRUE_OR_THROW( out_channel < outRaster->nBands(), "Invalid output band" );
  
  if( normalize_output ) {
    TEAGN_TRUE_OR_THROW( 
      ( outRaster->params().dataType_[ out_channel ] != TeDOUBLE ),
      "Cannot apply normalization for TeDOUBLE pixel type output bands" );
  }
  
  TeRaster* inRaster1NPtr = inRaster1.nakedPointer();
  TeRaster* inRaster2NPtr = inRaster2.nakedPointer();
  TeRaster* outRasterNPtr = outRaster.nakedPointer();
  const int in_raster_lines = inRaster1->params().nlines_;
  const int in_raster_columns = inRaster1->params().ncols_;    
  
  /* Guessing dummy use */
  
  bool inRaster1_uses_dummy = inRaster1->params().useDummy_;
  bool inRaster2_uses_dummy = inRaster2->params().useDummy_;
  bool outRaster_uses_dummy = outRaster->params().useDummy_;
  double outRaster_dummy = 0;
  if( outRaster_uses_dummy ) {
    outRaster_dummy = outRaster->params().dummy_[ out_channel ];
  }   
  
  /* Guessing channel bounds */
  
  double in_channel1_min_level = 0;
  double in_channel1_max_level = 0;

  TEAGN_TRUE_OR_THROW( TePDIUtils::TeGetRasterMinMaxBounds(
    inRaster1, in_channel1, in_channel1_min_level,
    in_channel1_max_level ), 
    "Unable to get raster1 input channel level bounds" );  
    
  double in_channel2_min_level = 0;
  double in_channel2_max_level = 0;

  TEAGN_TRUE_OR_THROW( TePDIUtils::TeGetRasterMinMaxBounds(
    inRaster2, in_channel2, in_channel2_min_level,
    in_channel2_max_level ), 
    "Unable to get raster2 input channel level bounds" );  

  double out_channel_min_level = 0;
  double out_channel_max_level = 0;

  TEAGN_TRUE_OR_THROW( TePDIUtils::TeGetRasterMinMaxBounds(
    outRaster, out_channel, out_channel_min_level,
    out_channel_max_level ), 
    "Unable to get raster output channel level bounds" );
    
  /* Level remapping */
    
  if( normalize_output ) {
    /* Guessing normalization factors */
    
    double norm_factor = 0;
    double norm_off = 0;
    
    TePDIMatrix< double > mapped_matrix;
    TEAGN_TRUE_OR_THROW( mapped_matrix.Reset( in_raster_lines, 
      in_raster_columns, TePDIMatrix< double >::AutoMemPol ),
      "Mapped levels matrix reset error" );    
  
    StartProgInt( "Guessing normalization factors...", in_raster_lines );
  
    double norm_min_level = DBL_MAX;
    double norm_max_level = -1.0 * norm_min_level;
    int line = 0;
    int column = 0;
    double value1 = 0;
    double value2= 0;
    bool got_element1 = false;
    bool got_element2 = false;
    double mapped_level = 0;
    
    for( line = 0 ; line < in_raster_lines ; ++line ) {
      TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
  
      for( column = 0 ; column < in_raster_columns ; ++column ) {
        got_element1 = inRaster1NPtr->getElement( column, line, value1, 
          in_channel1 );
        got_element2 = inRaster2NPtr->getElement( column, line, value2, 
          in_channel2 );
          
        if( got_element1 && got_element2 ) {
          mapped_level = remapp_func( value1, value2 , gain, offset );    
        
          mapped_matrix( line, column ) = mapped_level;
            
          if( mapped_level < norm_min_level ) {
            norm_min_level = mapped_level;
          } 
          if( mapped_level > norm_max_level ) {
            norm_max_level = mapped_level;
          }
        } else {
          if( got_element1 ) {
            TEAGN_TRUE_OR_RETURN( inRaster2_uses_dummy, 
              "Raster2 read error" );
          } else {
            TEAGN_TRUE_OR_RETURN( inRaster1_uses_dummy, 
              "Raster1 read error" );
          }        
        
          mapped_matrix( line, column ) = outRaster_dummy;
        }
      }
    }
     
    StopProgInt();
    
    norm_factor = ( out_channel_max_level - out_channel_min_level ) /
      ( norm_max_level - norm_min_level );
    norm_off = out_channel_min_level - norm_min_level;
    
    /* Remapping levels */
    
    double output_level = 0;
    
    StartProgInt( "Remapping Levels...", in_raster_lines );
    
    for( line = 0 ; line < in_raster_lines ; ++line ) {
      TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
    
      for( column = 0 ; column < in_raster_columns ; ++column ) {
        output_level = mapped_matrix( line, column );
              
        output_level = ( output_level + norm_off ) * norm_factor;
        
        TEAGN_TRUE_OR_RETURN( outRasterNPtr->setElement( column, line,
          output_level, out_channel ),
          "Level remmaping error at " + Te2String( line ) +
          "," + Te2String( column ) );        
      }  
    }
  } else {
    int line = 0;
    int column = 0;  
    double current_level1 = 0;
    double current_level2 = 0;
    double output_level = 0;
    bool got_element1 = false;
    bool got_element2 = false;
    
    StartProgInt( "Remapping Levels...", in_raster_lines );
  
    for( line = 0 ; line < in_raster_lines ; ++line ) {
      TEAGN_FALSE_OR_RETURN( UpdateProgInt( line ), "Canceled by the user" );
    
      for( column = 0 ; column < in_raster_columns ; ++column ) {
        got_element1 = inRaster1NPtr->getElement( column, line, 
          current_level1, in_channel1 );
        got_element2 = inRaster2NPtr->getElement( column, line, 
          current_level2, in_channel2 );
              
        if( got_element1 && got_element2 ) {
  
          /* Finding mapped level by using the level remapping function */
  
          output_level = remapp_func( current_level1, current_level2 , gain, 
            offset );
  
          /* Level range filtering */
          
          if( output_level < out_channel_min_level ) {
            output_level = out_channel_min_level;
          } else if( output_level > out_channel_max_level ) {
            output_level = out_channel_max_level;
          }

          TEAGN_TRUE_OR_RETURN( outRasterNPtr->setElement( column, line,
            output_level, out_channel ),
            "Level remmaping error at " + Te2String( line ) +
            "," + Te2String( column ) );
        } else {
          if( got_element1 ) {
            TEAGN_TRUE_OR_RETURN( inRaster2_uses_dummy, 
              "Raster2 read error" );
          } else {
            TEAGN_TRUE_OR_RETURN( inRaster1_uses_dummy, 
              "Raster1 read error" );
          }
            
          TEAGN_TRUE_OR_RETURN( outRaster->setElement( column, line,
            outRaster_dummy, out_channel ),
            "Level remmaping error at " + Te2String( line ) +
            "," + Te2String( column ) );
        }
      }  
    }
  }
  
  return true;
}


void TePDILevelRemap::BuildHistograms(
  TePDITypes::TePDIRasterPtrType& inRaster,
  unsigned int histo_levels,
  std::vector< int >& channels,
  bool force )
{
  TEAGN_TRUE_OR_THROW( inRaster.isActive(),
    "inRaster inactive" );
  TEAGN_TRUE_OR_THROW( inRaster->params().status_ != TeRasterParams::TeNotReady,
    "inRaster not ready" );

  std::pair< TeRaster*, unsigned int  > mapkey;
  mapkey.first = inRaster.nakedPointer();

  for( unsigned int channels_index = 0 ; channels_index < channels.size() ;
       ++channels_index ) {

    TEAGN_TRUE_OR_THROW( channels[ channels_index ] < inRaster->nBands(),
      "Trying to creat histogram from an invalid band" );

    mapkey.second = channels_index;

    if( ( histo_cache_.find( mapkey ) ==  histo_cache_.end() ) || force ) {

      TePDIHistogram::pointer temp_hist( new TePDIHistogram );

      TEAGN_TRUE_OR_THROW( temp_hist->reset( inRaster,
        channels[ channels_index ], histo_levels, 
        TeBoxPixelIn ),
        "Histogram Generation Error" );

      histo_cache_[ mapkey ] = temp_hist;
    }
  }
}


TePDIHistogram::pointer TePDILevelRemap::GetHistRef(
  TePDITypes::TePDIRasterPtrType& inRaster,
  unsigned int band )
{
  std::pair< TeRaster*, unsigned int  > mapkey;
  mapkey.first = inRaster.nakedPointer();
  mapkey.second = band;

  TEAGN_TRUE_OR_THROW(
    ( histo_cache_.find( mapkey ) != histo_cache_.end() ),
    "Histogram not stored inside cache" );

  return histo_cache_[ mapkey ];
}


