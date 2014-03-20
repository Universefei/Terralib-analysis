#include "TePDIJointHistogram.hpp"

#include "TePDIPIManager.hpp"

#include "../kernel/TeOverlay.h"
#include "../kernel/TeBox.h"
#include "../kernel/TeGeometry.h"
#include "../kernel/TeAgnostic.h"

TePDIJointHistogram::TePDIJointHistogram()
{
  progressEnabled_ = true;

  init_jhist_ = 0;
  init_jhist_lines_ = 0;
  init_jhist_cols_ = 0;
  init_r1hist_ = 0;
  init_r2hist_ = 0;
}


TePDIJointHistogram::TePDIJointHistogram( const TePDIJointHistogram& external )
{
  operator=( external );
}


TePDIJointHistogram::~TePDIJointHistogram()
{
  clear();
}

const TePDIJointHistogram& TePDIJointHistogram::operator=( const 
  TePDIJointHistogram& external )    
{
  clear();

  jHistContainer_ = external.jHistContainer_;
  r1Histogram_ = external.r1Histogram_;
  r2Histogram_ = external.r2Histogram_;
  
  return external;
}

void TePDIJointHistogram::toggleProgress( bool enabled )
{
  progressEnabled_ = enabled;
}

bool TePDIJointHistogram::update( TeRaster& inputRaster1, 
  unsigned int inputRasterChn1, TeRaster& inputRaster2, 
  unsigned int inputRasterChn2, TeStrategicIterator iterStrat, 
  unsigned int levels, const TePolygonSet& restricPolSet )
{
  TEAGN_TRUE_OR_RETURN( inputRaster1.params().status_ != 
    TeRasterParams::TeNotReady, "Raster1 not ready" )
  TEAGN_TRUE_OR_RETURN( inputRaster2.params().status_ != 
    TeRasterParams::TeNotReady, "Raster2 not ready" )
    
  TEAGN_TRUE_OR_RETURN( inputRaster1.params().resx_ ==
    inputRaster2.params().resx_, "X resolution mismatch" )
  TEAGN_TRUE_OR_RETURN( inputRaster1.params().resy_ ==
    inputRaster2.params().resy_, "Y resolution mismatch" )
      
  TEAGN_TRUE_OR_RETURN( (int)inputRasterChn1 < inputRaster1.nBands(),
    "Invalid channel" )
  TEAGN_TRUE_OR_RETURN( (int)inputRasterChn2 < inputRaster2.nBands(),
    "Invalid channel" )
      
  clear();
    
  // Calc the intersection between the restriction polygon set,
  // and both rasters bounding box
     
  TePolygonSet interPS;
  {
    TePolygon pol1 = polygonFromBox( inputRaster1.params().boundingBox() );
    TePolygon pol2 = polygonFromBox( inputRaster2.params().boundingBox() );
    
    TePolygonSet ps1;
    ps1.add( pol1 );
    
    TePolygonSet ps2;
    ps2.add( pol2 );
    
    TePolygonSet boxesInterPS;
    TEAGN_TRUE_OR_RETURN( TeOVERLAY::TeIntersection( ps1, ps2, boxesInterPS ),
      "Unable to find the intersection between both raster bounding boxes" );
    
    if( restricPolSet.size() == 0 )
    {
      interPS = boxesInterPS;
    }
    else
    {
      TEAGN_TRUE_OR_RETURN( TeOVERLAY::TeIntersection( boxesInterPS, restricPolSet, 
        interPS ), "Unable to find the intersection between both raster bounding boxes" );
    }
  }
    
  // Pallete based rasters not supported
    
  if( ( inputRaster1.params().photometric_[ inputRasterChn1 ] == 
    TeRasterParams::TePallete ) ||
    ( inputRaster2.params().photometric_[ inputRasterChn2 ] == 
    TeRasterParams::TePallete ) ) 
  {
    TEAGN_LOG_AND_RETURN( "Pallete based rasters not supported" )
  }
    
  // Optimization for floating point rasters

  if( ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeDOUBLE ) ||
      ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeFLOAT ) ||
      ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeDOUBLE ) ||
      ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeFLOAT ) )
  {
    try
    {
      return updateFloat( inputRaster1, inputRasterChn1, inputRaster2, 
        inputRasterChn2, iterStrat, levels, interPS );
    }
    catch(...)
    {
      TEAGN_LOG_AND_RETURN( "Not enought memory" );
    }
  }
  
  // Optmization for integer rasters
		  
  if( ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeINTEGER ) ||
      ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeUNSIGNEDLONG ) ||
      ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeLONG ) ||
      ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeSHORT ) ||
      ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeUNSIGNEDSHORT ) ||
      ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeINTEGER ) ||
      ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeUNSIGNEDLONG ) ||
      ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeLONG ) ||
      ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeSHORT ) ||
      ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeUNSIGNEDSHORT ) )
  {
    return updateInteger( inputRaster1, inputRasterChn1, inputRaster2, 
      inputRasterChn2, iterStrat, interPS );
  }  
  
  // Optimization for 8 bits or 16 bits
  
  if( ( ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeCHAR ) ||
      ( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeUNSIGNEDCHAR ) ) &&
      ( ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeCHAR ) ||
      ( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeUNSIGNEDCHAR ) ) )
  {
    return update8Bits( inputRaster1, inputRasterChn1, inputRaster2, 
      inputRasterChn2, iterStrat, interPS );
  }  
  
  TEAGN_LOG_AND_THROW( "Invalid data type" );
  return false;
}

void TePDIJointHistogram::clear()
{
  jHistContainer_.clear();
  r1Histogram_.clear();
  r2Histogram_.clear();

  clearTempHistBuffers();
}

const TePDIHistogram& TePDIJointHistogram::getRaster1Hist() const
{
  return r1Histogram_;
}

const TePDIHistogram& TePDIJointHistogram::getRaster2Hist() const
{
  return r2Histogram_;
}

TePDIJointHistogram::const_iterator TePDIJointHistogram::begin() const
{
  return jHistContainer_.begin();
}

TePDIJointHistogram::const_iterator TePDIJointHistogram::end() const
{
  return jHistContainer_.end();
}

unsigned int TePDIJointHistogram::size() const
{
  return (unsigned int)jHistContainer_.size();
}

void TePDIJointHistogram::getInverseJHist( TePDIJointHistogram& external )
{
  external.clear();
  external.r1Histogram_ = r2Histogram_;
  external.r2Histogram_ = r1Histogram_;

  ConteinerT::const_iterator cit = jHistContainer_.begin();
  ConteinerT::const_iterator cit_end = jHistContainer_.end();
  std::pair< double, double > tempPair;
  while( cit != cit_end )
  {
    tempPair.first = cit->first.second;
    tempPair.second = cit->first.first;
    external.jHistContainer_[ tempPair ] = cit->second;
    
    ++cit;
  }
}

unsigned long int TePDIJointHistogram::getFreqSum() const
{
  ConteinerT::const_iterator cit = jHistContainer_.begin();
  ConteinerT::const_iterator cit_end = jHistContainer_.end();

  unsigned long int sum = 0;

  while( cit != cit_end )
  {
    sum += (unsigned long int)cit->second;
    
    ++cit;
  }
  
  return sum;
}

std::string TePDIJointHistogram::toString() const
{
  std::string outStr;
  
  const_iterator it = begin();
  const_iterator it_end = end();
  
  while( it != it_end ) 
  {
    outStr.append( "[" + Te2String( it->first.first, 2 ) + "," +
      Te2String( it->first.second, 2 ) + "->" +
      Te2String( it->second ) + "]" );
    
    ++it;
  }
  
  return outStr;
}

bool TePDIJointHistogram::updateFloat( TeRaster& inputRaster1, 
  unsigned int inputRasterChn1, TeRaster& inputRaster2, 
  unsigned int inputRasterChn2, TeStrategicIterator iterStrat, 
  unsigned int levels, const TePolygonSet& restricPolSet )
{
  const unsigned long int progress_steps = 
    getProgressSteps( restricPolSet, inputRaster1.params().resy_ );   
  
  /* Finding raster 1 lowest end highest levels */
  
  double r1_lowest_level = 0;
  double r1_highest_level = 0;  
    
  if ( inputRaster1.params().decoderIdentifier_ == "DB" ) {
    r1_lowest_level = inputRaster1.params().vmin_[ inputRasterChn1 ];
    r1_highest_level = inputRaster1.params().vmax_[ inputRasterChn1 ];

    // Return if there is no pixel data to read 

    if( inputRaster1.params().useDummy_ && ( r1_lowest_level == 
      inputRaster1.params().dummy_[ inputRasterChn1 ] ) && ( r1_highest_level == 
      inputRaster1.params().dummy_[ inputRasterChn1 ] ) )
    {
      return true;
    }
  } else {
    r1_lowest_level = DBL_MAX;
    r1_highest_level = ( -1.0 ) * DBL_MAX;  

    bool lh_levels_set = false; // lowest_level and highest_level values set
    
    TeRaster::iteratorPoly input_raster_it;
    TePDIPIManager progress( "Finding histogram range", progress_steps, 
      progressEnabled_ );
    double current_raster_level = 0;
    unsigned int curr_line = 0;
    unsigned int curr_col = 0;
    unsigned int last_line = 0;    
    
    for( unsigned int local_polset_index = 0 ; local_polset_index < 
      restricPolSet.size() ; ++local_polset_index ) 
    {
      input_raster_it = inputRaster1.begin( restricPolSet[ local_polset_index ], 
        iterStrat, 0 );
      
      while( ! input_raster_it.end() ) {
        curr_line = input_raster_it.currentLine();
        curr_col = input_raster_it.currentColumn();
        
        if( inputRaster1.getElement( curr_col, curr_line, current_raster_level,
          inputRasterChn1 ) )
        {
          lh_levels_set = true;

          if( current_raster_level > r1_highest_level ) {
            r1_highest_level = current_raster_level;
          }
          if( current_raster_level < r1_lowest_level ) {
            r1_lowest_level = current_raster_level;
          }
        }
        
        if( curr_line != last_line ) {
          TEAGN_FALSE_OR_RETURN( progress.Increment(), 
            "Canceled by the user" );
          
          last_line = curr_line;
        }
        
        ++input_raster_it;
      }
    }

    // Return if no pixel data was read 

    if( ! lh_levels_set )
    {
      return true;
    }
  }   
   
  /* Finding raster 2 lowest end highest levels */
  
  double r2_lowest_level = 0;
  double r2_highest_level = 0;  
    
  if ( inputRaster2.params().decoderIdentifier_ == "DB" ) {
    r2_lowest_level = inputRaster2.params().vmin_[ inputRasterChn2 ];
    r2_highest_level = inputRaster2.params().vmax_[ inputRasterChn2 ];

    // Return if there is no pixel data to read 

    if( inputRaster2.params().useDummy_ && ( r2_lowest_level == 
      inputRaster2.params().dummy_[ inputRasterChn2 ] ) && ( r2_highest_level == 
      inputRaster2.params().dummy_[ inputRasterChn2 ] ) )
    {
      return true;
    }
  } else {
    r2_lowest_level = DBL_MAX;
    r2_highest_level = ( -1.0 ) * DBL_MAX;  

    bool lh_levels_set = false; // lowest_level and highest_level values set
    
    TeRaster::iteratorPoly input_raster_it;
    TePDIPIManager progress( "Finding histogram range", progress_steps, 
      progressEnabled_ );
    double current_raster_level = 0;
    unsigned int curr_line = 0;
    unsigned int curr_col = 0;
    unsigned int last_line = 0;    
    
    for( unsigned int local_polset_index = 0 ; local_polset_index < 
      restricPolSet.size() ; ++local_polset_index ) 
    {
      input_raster_it = inputRaster2.begin( restricPolSet[ local_polset_index ], 
        iterStrat, 0 );
      
      while( ! input_raster_it.end() ) {
        curr_line = input_raster_it.currentLine();
        curr_col = input_raster_it.currentColumn();
        
        if( inputRaster2.getElement( curr_col, curr_line, current_raster_level,
          inputRasterChn2 ) )
        {
          lh_levels_set = true;

          if( current_raster_level > r2_highest_level ) {
            r2_highest_level = current_raster_level;
          }
          if( current_raster_level < r2_lowest_level ) {
            r2_lowest_level = current_raster_level;
          }
        }
        
        if( curr_line != last_line ) {
          TEAGN_FALSE_OR_RETURN( progress.Increment(), 
            "Canceled by the user" );
          
          last_line = curr_line;
        }
        
        ++input_raster_it;
      }
    }

    // Return if no pixel data was read 

    if( ! lh_levels_set )
    {
      return true;
    }
  }      
     
  /* Calculating the raster 1 histogram step and computed levels */

  double r1_step = 1;
  unsigned int r1_hist_computed_levels = 1;

  if( r1_highest_level != r1_lowest_level )
  {
    if( levels == 0 ) {
      /* Auto step feature */
      TEAGN_TRUE_OR_RETURN( ( ( r1_highest_level - r1_lowest_level ) > 1.0 ),
        "Histogram level range is too short for auto-step feature"
        " lowest_level=" + Te2String( r1_lowest_level ) + " highest_level=" +
        Te2String( r1_highest_level ) );
  
      r1_hist_computed_levels = 1 +
        ( unsigned int ) floor( r1_highest_level - r1_lowest_level );
    } else {
      r1_hist_computed_levels = levels;
    }
  
    TEAGN_TRUE_OR_RETURN( r1_hist_computed_levels > 0,
      "Invalid levels number" );
  
    r1_step = ( r1_highest_level - r1_lowest_level ) /
          ( (double)( r1_hist_computed_levels - 1 ) );
  }
  
  /* Calculating the raster 2 histogram step and computed levels */

  double r2_step = 1;
  unsigned int r2_hist_computed_levels = 1;

  if( r2_highest_level != r2_lowest_level )
  {
    if( levels == 0 ) {
      /* Auto step feature */
      TEAGN_TRUE_OR_RETURN( ( ( r2_highest_level - r2_lowest_level ) > 1.0 ),
        "Histogram level range is too short for auto-step feature"
        " lowest_level=" + Te2String( r2_lowest_level ) + " highest_level=" +
        Te2String( r2_highest_level ) );
  
      r2_hist_computed_levels = 1 +
        ( unsigned int ) floor( r2_highest_level - r2_lowest_level );
    } else {
      r2_hist_computed_levels = levels;
    }
  
    TEAGN_TRUE_OR_RETURN( r2_hist_computed_levels > 0,
      "Invalid levels number" );
  
    r2_step = ( r2_highest_level - r2_lowest_level ) /
      ( (double)( r2_hist_computed_levels - 1 ) );
  }  
     
   /* Building initial histograms */
   
   init_jhist_ = new unsigned int*[ r1_hist_computed_levels ];
   init_jhist_lines_ = r1_hist_computed_levels;

   init_r1hist_ = new unsigned int[ r1_hist_computed_levels ];
   init_r2hist_ = new unsigned int[ r2_hist_computed_levels ];
   {
     unsigned int hist_index1 = 0;
     unsigned int hist_index2 = 0;
     
     for( hist_index1 = 0 ; hist_index1 < r1_hist_computed_levels ; ++hist_index1 )
     {
       init_r1hist_[ hist_index1 ] = 0;
       init_jhist_[ hist_index1 ] = new unsigned int[ r2_hist_computed_levels ];
       
       for( hist_index2 = 0 ; hist_index2 < r2_hist_computed_levels ; ++hist_index2 )
       {
         init_jhist_[ hist_index1 ][ hist_index2 ] = 0;
       }
     }
       
     for( hist_index2 = 0 ; hist_index2 < r2_hist_computed_levels ; ++hist_index2 )
     {
       init_r2hist_[ hist_index2 ] = 0;
     }
   }
     
   /* Analysing raster */
   
   {
     const unsigned long int progress_steps = 
       getProgressSteps( restricPolSet, inputRaster1.params().resy_ ); 
     
     TePDIPIManager progress( "Generating histogram", progress_steps, 
       progressEnabled_ );
   
     TeRaster::iteratorPoly r1_it;
     TeRaster::iteratorPoly r2_it;
     
     long int r1_line = 0;
     long int r1_col = 0;
     long int last_r1_line = 0;
     
     long int r2_line_off = 0;
     long int r2_col_off = 0;
     
     double r1_value_double = 0;
     double r2_value_double = 0;
     unsigned int r1_value_int = 0;
     unsigned int r2_value_int = 0;
     
     bool gotvalue1 = false;
     bool gotvalue2 = false;
     
     for( unsigned int polset_index = 0 ; polset_index < 
       restricPolSet.size() ; ++polset_index ) 
     {
       r1_it = inputRaster1.begin( restricPolSet[ polset_index ], 
         iterStrat, 0 );
       r2_it = inputRaster2.begin( restricPolSet[ polset_index ], 
         iterStrat, 0 );
       
       if( ( ! r1_it.end() ) && ( ! r2_it.end() ) )
       {
         r1_line = r1_it.currentLine();
         r1_col = r1_it.currentColumn();
         
         r2_line_off = r1_line - r2_it.currentLine();
         r2_col_off = r1_col - r2_it.currentColumn();         
  
         while( ! r1_it.end() ) {
           gotvalue1 = inputRaster1.getElement( r1_col, r1_line, 
             r1_value_double, inputRasterChn1 );
           gotvalue2 = inputRaster2.getElement( r1_col - r2_col_off, 
             r1_line - r2_line_off, r2_value_double, inputRasterChn2 );
           
           if( gotvalue1 )
           {
             r1_value_int = (unsigned int) TeRound( ( r1_value_double - 
               r1_lowest_level ) / r1_step );
             TEAGN_DEBUG_CONDITION( r1_value_int < r1_hist_computed_levels,
               "Invalid histogram index");

             ++( init_r1hist_[ r1_value_int ] );
           }
           
           if( gotvalue2 )
           {
             r2_value_int = (unsigned int) TeRound( ( r2_value_double - 
               r2_lowest_level ) / r2_step );
             TEAGN_DEBUG_CONDITION( r2_value_int < r2_hist_computed_levels,
               "Invalid histogram index");
           
             ++( init_r2hist_[ r2_value_int ] );
             
             if( gotvalue1 )
             {
               ++( init_jhist_[ r1_value_int ][ r2_value_int ] );          
             }             
           }
           
           if( r1_line != last_r1_line ) {
             TEAGN_FALSE_OR_RETURN( progress.Increment(), 
               "Canceled by the user" );
             
             last_r1_line = r1_line;
           }
           
           ++r1_it;
           r1_line = r1_it.currentLine();
           r1_col = r1_it.currentColumn();           
         }
       }
     }  
   }    
  
   /* Building ordered joint histogram */
   
   {
     unsigned int hist_index1 = 0;
     unsigned int hist_index2 = 0;
     std::pair< double, double > temp_pair;
     
     for( hist_index1 = 0 ; hist_index1 < r1_hist_computed_levels ; 
       ++hist_index1 )
     {
       if( hist_index1 == ( r1_hist_computed_levels - 1 ) )
       {
         temp_pair.first = r1_highest_level;
       }
       else
       {
         temp_pair.first = ( r1_step * (double)hist_index1 ) + 
           r1_lowest_level;
       }
       
       for( hist_index2 = 0 ; hist_index2 < r2_hist_computed_levels ;
         ++hist_index2 )
       { 
         if( init_jhist_[ hist_index1 ][ hist_index2 ] )  
         {
           if( hist_index2 == ( r2_hist_computed_levels - 1 ) )
           {
             temp_pair.second = r2_highest_level;
           }
           else
           {
             temp_pair.second = ( r2_step * (double)hist_index2 ) + 
               r2_lowest_level;
           }     
             
           jHistContainer_[ temp_pair ] = init_jhist_[ hist_index1 ][ 
             hist_index2 ];
         }
       }
     }
   }
      
   /* Building ordered raster 1 histogram */
   
   {
     for( unsigned int hist_index = 0 ; hist_index < r1_hist_computed_levels ; 
       ++hist_index )
     {
       if( init_r1hist_[ hist_index ] )
       {
         if( hist_index == ( r1_hist_computed_levels - 1 ) )
         {
           r1Histogram_[ r1_highest_level ] = init_r1hist_[ hist_index ];
         }
         else
         {
           r1Histogram_[ ( r1_step * (double)hist_index ) + r1_lowest_level ] =
             init_r1hist_[ hist_index ];
         }
       }
     }
   }   
   
   /* Building ordered raster 2 histogram */
   
   {
     for( unsigned int hist_index = 0 ; hist_index < r2_hist_computed_levels ; 
       ++hist_index )
     {
       if( init_r2hist_[ hist_index ] ) 
       {
         if( hist_index == ( r2_hist_computed_levels - 1 ) )
         {
           r2Histogram_[ r2_highest_level ] = init_r2hist_[ hist_index ];
         }
         else
         {
           r2Histogram_[ ( r2_step * (double)hist_index ) + r2_lowest_level ] =
             init_r2hist_[ hist_index ];
         }
       }
     }
   }  

   clearTempHistBuffers(); 
      
   return true;
}

bool TePDIJointHistogram::updateInteger( TeRaster& inputRaster1, 
  unsigned int inputRasterChn1, TeRaster& inputRaster2, 
  unsigned int inputRasterChn2, TeStrategicIterator iterStrat, 
  const TePolygonSet& restricPolSet )
{
   /* Analysing raster */
   
   {
     const unsigned long int progress_steps = 
       getProgressSteps( restricPolSet, inputRaster1.params().resy_ ); 
     
     TePDIPIManager progress( "Generating histogram", progress_steps, 
       progressEnabled_ );
   
     TeRaster::iteratorPoly r1_it;
     TeRaster::iteratorPoly r2_it;
     
     long int r1_line = 0;
     long int r1_col = 0;
     long int last_r1_line = 0;
     
     long int r2_line_off = 0;
     long int r2_col_off = 0;
     
     bool gotvalue1 = false;
     bool gotvalue2 = false;

     std::pair< double, double > tempPair;
     
     for( unsigned int polset_index = 0 ; polset_index < 
       restricPolSet.size() ; ++polset_index ) 
     {
       r1_it = inputRaster1.begin( restricPolSet[ polset_index ], 
         iterStrat, 0 );
       r2_it = inputRaster2.begin( restricPolSet[ polset_index ], 
         iterStrat, 0 );
       
       if( ( ! r1_it.end() ) && ( ! r2_it.end() ) )
       {
         r1_line = r1_it.currentLine();
         r1_col = r1_it.currentColumn();
         
         r2_line_off = r1_line - r2_it.currentLine();
         r2_col_off = r1_col - r2_it.currentColumn();         
  
         while( ! r1_it.end() ) {
           gotvalue1 = inputRaster1.getElement( r1_col, r1_line, 
             tempPair.first, inputRasterChn1 );
           gotvalue2 = inputRaster2.getElement( r1_col - r2_col_off, 
             r1_line - r2_line_off, tempPair.second, inputRasterChn2 );
           
           if( gotvalue1 )
           {
             ++( r1Histogram_[ tempPair.first ] );
           }
           
           if( gotvalue2 )
           {
             ++( r2Histogram_[ tempPair.second ] );
             
             if( gotvalue1 )
             {
               ++( jHistContainer_[ tempPair ] );          
             }             
           }
           
           if( r1_line != last_r1_line ) {
             TEAGN_FALSE_OR_RETURN( progress.Increment(), 
               "Canceled by the user" );
             
             last_r1_line = r1_line;
           }
           
           ++r1_it;
           r1_line = r1_it.currentLine();
           r1_col = r1_it.currentColumn();           
         }
       }
     }  
   }    

   clearTempHistBuffers();
   
   return true;
}

bool TePDIJointHistogram::update8Bits( TeRaster& inputRaster1, 
  unsigned int inputRasterChn1, TeRaster& inputRaster2, 
  unsigned int inputRasterChn2, TeStrategicIterator iterStrat, 
  const TePolygonSet& restricPolSet )
{
   /* Defining level offset based on data bype */
   
   unsigned int level_offset1 = 0;
   
   if( inputRaster1.params().dataType_[ inputRasterChn1 ] == TeCHAR ) {
     level_offset1 = 127; /*( 256 / 2 ) - 1 */
   }
   
   unsigned int level_offset2 = 0;
   
   if( inputRaster2.params().dataType_[ inputRasterChn2 ] == TeCHAR ) {
     level_offset2 = 127; /*( 256 / 2 ) - 1 */
   }   
     
   /* Building initial histograms */
   
   init_jhist_ = new unsigned int*[ 256 ];
   init_jhist_lines_ = 256;

   init_r1hist_ = new unsigned int[ 256 ];
   init_r2hist_ = new unsigned int[ 256 ];
   {
     unsigned int hist_index1 = 0;
     unsigned int hist_index2 = 0;
     
     for( hist_index1 = 0 ; hist_index1 < 256 ; ++hist_index1 )
     {
       init_r1hist_[ hist_index1 ] = 0;
       init_jhist_[ hist_index1 ] = new unsigned int[ 256 ];
       
       for( hist_index2 = 0 ; hist_index2 < 256 ; ++hist_index2 )
       {
         init_jhist_[ hist_index1 ][ hist_index2 ] = 0;
       }
     }

     for( hist_index2 = 0 ; hist_index2 < 256 ; ++hist_index2 )
     {
       init_r2hist_[ hist_index2 ] = 0;
     }
   }
   
   /* Analysing raster */
   
   {
     const unsigned long int progress_steps = 
       getProgressSteps( restricPolSet, inputRaster1.params().resy_ ); 
     
     TePDIPIManager progress( "Generating histogram", progress_steps, 
       progressEnabled_ );
   
     TeRaster::iteratorPoly r1_it;
     TeRaster::iteratorPoly r2_it;
     
     long int r1_line = 0;
     long int r1_col = 0;
     long int last_r1_line = 0;
     
     long int r2_line_off = 0;
     long int r2_col_off = 0;
     
     double r1_value_double = 0;
     double r2_value_double = 0;
     int r1_value_int = 0;
     int r2_value_int = 0;
     
     bool gotvalue1 = false;
     bool gotvalue2 = false;
   
     for( unsigned int polset_index = 0 ; polset_index < 
       restricPolSet.size() ; ++polset_index ) 
     {
       r1_it = inputRaster1.begin( restricPolSet[ polset_index ], 
         iterStrat, 0 );
       r2_it = inputRaster2.begin( restricPolSet[ polset_index ], 
         iterStrat, 0 );
       
       if( ( ! r1_it.end() ) && ( ! r2_it.end() ) )
       {
         r1_line = r1_it.currentLine();
         r1_col = r1_it.currentColumn();
         
         r2_line_off = r1_line - r2_it.currentLine();
         r2_col_off = r1_col - r2_it.currentColumn();         
  
         while( ! r1_it.end() ) {
           gotvalue1 = inputRaster1.getElement( r1_col, r1_line, 
             r1_value_double, inputRasterChn1 );
           gotvalue2 = inputRaster2.getElement( r1_col - r2_col_off, 
             r1_line - r2_line_off, r2_value_double, inputRasterChn2 );
           
           if( gotvalue1 )
           {
             r1_value_int = ( int ) r1_value_double;
             TEAGN_DEBUG_CONDITION( (r1_value_int + level_offset1) < 256,
               "Invalid histogram index");

             ++( init_r1hist_[ r1_value_int + level_offset1 ] );
           }
           
           if( gotvalue2 )
           {
             r2_value_int = ( int ) r2_value_double;
             TEAGN_DEBUG_CONDITION( (r2_value_int + level_offset2) < 256,
               "Invalid histogram index");
           
             ++( init_r2hist_[ r2_value_int + level_offset2 ] );
             
             if( gotvalue1 )
             {
               ++( init_jhist_[ r1_value_int + level_offset1 ][ r2_value_int + 
                 level_offset2 ] );  
             }             
           }
           
           if( r1_line != last_r1_line ) {
             TEAGN_FALSE_OR_RETURN( progress.Increment(), 
               "Canceled by the user" );
             
             last_r1_line = r1_line;
           }
           
           ++r1_it;
           r1_line = r1_it.currentLine();
           r1_col = r1_it.currentColumn();           
         }
       }
     }  
   }    
   
   /* Building ordered joint histogram */
   
   {
     unsigned int hist_index1 = 0;
     unsigned int hist_index2 = 0;
     std::pair< double, double > temp_pair;
     
     for( hist_index1 = 0 ; hist_index1 < 256 ; ++hist_index1 )
     {
       for( hist_index2 = 0 ; hist_index2 < 256 ; ++hist_index2 )
       { 
         if( init_jhist_[ hist_index1 ][ hist_index2 ] ) 
         {
           temp_pair.first = ( (double)hist_index1 ) - ( (double)level_offset1 );
           temp_pair.second = ( (double)hist_index2 ) - ( (double)level_offset2 );
           
           jHistContainer_[ temp_pair ] = init_jhist_[ hist_index1 ][ hist_index2 ];
         }
       }
     }
   }
   
   /* Building ordered raster 1 histogram */
   
   {
     unsigned int hist_index = 0;
     
     for( hist_index = 0 ; hist_index < 256 ; ++hist_index )
     {
       if( init_r1hist_[ hist_index ] )
       {
         r1Histogram_[ ( (double)hist_index ) - ( (double)level_offset1 ) ] =
          init_r1hist_[ hist_index ];
       }
     }
   }   
   
   /* Building ordered raster 2 histogram */
   
   {
     unsigned int hist_index = 0;
     
     for( hist_index = 0 ; hist_index < 256 ; ++hist_index )
     {
       if( init_r2hist_[ hist_index ] )
       {
         r2Histogram_[ ( (double)hist_index ) - ( (double)level_offset2 ) ] =
           init_r2hist_[ hist_index ];
       }
     }
   }    

   clearTempHistBuffers();
   
   return true;
}

unsigned long int TePDIJointHistogram::getProgressSteps( 
  const TePolygonSet& polset, double resy )
{
  unsigned long int steps = 0;

  for( unsigned int ps_index = 0 ; ps_index < polset.size() ; 
    ++ps_index ) {
    
    const TeBox& polbox( polset[ ps_index ].box() );
    steps += ( unsigned long int)ABS( ( polbox.y2() - polbox.y1() ) / resy );
  }
  
  return steps;
}

void TePDIJointHistogram::clearTempHistBuffers()
{
  if( init_jhist_ )
  {
    for( unsigned int line = 0 ; line < init_jhist_lines_ ; ++line )
    {
      if( init_jhist_[ line ] ) delete[]( init_jhist_[ line ] );
    }

    delete[]( init_jhist_ );

    init_jhist_ = 0;
    init_jhist_lines_ = 0;
  }

  if( init_r1hist_ )
  {
    delete[]( init_r1hist_ );
    init_r1hist_ = 0;
  }

  if( init_r2hist_ )
  {
    delete[]( init_r2hist_ );
    init_r2hist_ = 0;
	}
}
