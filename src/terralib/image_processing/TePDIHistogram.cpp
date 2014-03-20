#include "TePDIHistogram.hpp"

#include "TePDIUtils.hpp"
#include "TePDIMatrix.hpp"
#include "../kernel/TeAgnostic.h"
#include "TePDIPIManager.hpp"

#include "../kernel/TeUtils.h"
#include "../kernel/TePrecision.h"

#include <float.h>
#include <limits.h>

TePDIHistogram::TePDIHistogram()
{
  init();
}


TePDIHistogram::TePDIHistogram( const TePDIHistogram& external )
: TePDIHMapSpec()
{
  init();
  
  operator=( external );
}


TePDIHistogram::TePDIHistogram( 
  const std::map< double, unsigned int >& external )
{
  init();
  
  operator=( external );
}


TePDIHistogram::~TePDIHistogram()
{
}


bool TePDIHistogram::SetH(
  const TePDITypes::TePDIRasterPtrType& in_raster,
  unsigned int band,
  TeStrategicIterator iterstrat,
  const TeSharedPtr< TePolygonSet>& polsetptr )
{
  TEAGN_DEBUG_CONDITION( in_raster.isActive(),
    "Inactive raster pointer" );
  TEAGN_DEBUG_CONDITION( 
    in_raster->params().status_ != TeRasterParams::TeNotReady,
    "Raster not ready to read" );
  TEAGN_DEBUG_CONDITION( (int)band < in_raster->nBands(),
    "Invalid band" );
  TEAGN_DEBUG_CONDITION( ( ! TePDIUtils::IsFloatBand( in_raster, band ) ),
    "Unable to build non-interpolated histogram from float pixels type" );
    
  /* Optimization for 8 / 16 bit images */
  
  if( ( in_raster->params().dataType_[ band ] == TeUNSIGNEDCHAR ) ||
    ( in_raster->params().dataType_[ band ] == TeCHAR ) ) {
    return Set8BitH( in_raster, band, iterstrat, polsetptr );
  } else if( ( in_raster->params().dataType_[ band ] == TeUNSIGNEDSHORT ) ||
    ( in_raster->params().dataType_[ band ] == TeSHORT ) ) {
    return Set16BitH( in_raster, band, iterstrat, polsetptr );
  }
  
  TePDIHMapSpec::clear();
  
  const unsigned long int progress_steps = 
    getProgressSteps( polsetptr, in_raster->params().resy_ );  

  /* Dummy use check */
  
  bool in_raster_donot_uses_dummy = ! in_raster->params().useDummy_;
  double in_raster_dummy = 0;
  
  if( ! in_raster_donot_uses_dummy ) {
    in_raster_dummy = in_raster->params().dummy_[ band ];
  }    
    
  /* Building ordered histogram */
  
  {
    TePDIPIManager progress( "Generating histogram", 
      progress_steps, progress_int_enabled_ );
  
    double current_raster_level = 0;
    TeRaster::iteratorPoly input_raster_it;
    std::map< double, unsigned int >::iterator it;
    unsigned int curr_line = 0;
    unsigned int last_line = 0;
    
    for( unsigned int local_polset_index = 0 ; 
      local_polset_index < polsetptr->size() ; 
      ++local_polset_index ) {
      
      input_raster_it = in_raster->begin( 
        (*polsetptr)[ local_polset_index ], iterstrat, 0 );
      
      while( ! input_raster_it.end() ) 
      {
        curr_line = input_raster_it.currentLine();
      
        current_raster_level = input_raster_it*( band );
        
        if( in_raster_donot_uses_dummy || 
          ( current_raster_level != in_raster_dummy ) ) 
        {
          ++( operator[]( current_raster_level ) );
        }
        
        if( curr_line != last_line ) {
          TEAGN_FALSE_OR_RETURN( progress.Increment(), 
            "Canceled by the user" );
          
          last_line = curr_line;
        }
        
        ++input_raster_it;
      }
    }  
  }

  return true;
}


bool TePDIHistogram::reset(
  const TePDITypes::TePDIRasterPtrType& in_raster,
  unsigned int band,
  unsigned int levels,
  TeStrategicIterator iterstrat,
  const TeSharedPtr< TePolygonSet>& polsetptr )
{
  TEAGN_TRUE_OR_RETURN( in_raster.isActive(),
    "Inactive raster pointer" );
  TEAGN_TRUE_OR_RETURN( 
    in_raster->params().status_ != TeRasterParams::TeNotReady,
    "Raster not ready to read" );
  TEAGN_TRUE_OR_RETURN( (int)band < in_raster->nBands(),
    "Invalid band" );
    
  /* Building the local restriction polygon set */
  
  TeSharedPtr< TePolygonSet> local_polsetptr;
    
  if( polsetptr.isActive() ) {
    local_polsetptr = polsetptr;
  } else {
    local_polsetptr.reset( new TePolygonSet );
    
    TeBox rasterbox( in_raster->params().boundingBox() );
    
    local_polsetptr->add( polygonFromBox( rasterbox ) );
  }
  
  /* For palette based rasters another algorithm is required */
  
  if( in_raster->params().photometric_[ band ] == 
    TeRasterParams::TePallete ) {
   
    return SetPaletteBasedH( in_raster, band, iterstrat, local_polsetptr ); 
  }
    
  /* Optimization - If the raster does not contains float pixels for 
     this band, then we do not need an interpolated histogram */
    
  if( ( ! TePDIUtils::IsFloatBand( in_raster, band ) ) &&
      ( levels == 0 ) ) {
      
    return SetH( in_raster, band, iterstrat, local_polsetptr );
  }
  
  TePDIHMapSpec::clear();
  
  const unsigned long int progress_steps = 
    getProgressSteps( local_polsetptr, in_raster->params().resy_ );
  
  /* Dummy use check */
  
  bool in_raster_donot_uses_dummy = ! in_raster->params().useDummy_;
  double in_raster_dummy = 0;
  
  if( ! in_raster_donot_uses_dummy ) {
    in_raster_dummy = in_raster->params().dummy_[ band ];
  }
  
  /* Finding the lowest end highest level */
  
  double lowest_level = 0;
  double highest_level = 0;  
    
  if ( in_raster->params().decoderIdentifier_ == "DB" ) {
    lowest_level = in_raster->params().vmin_[ band ];
    highest_level = in_raster->params().vmax_[ band ];

    // Return if there is no pixel data to read 

    if( in_raster->params().useDummy_ && ( lowest_level == in_raster_dummy ) &&
      ( highest_level == in_raster_dummy ) )
    {
      return true;
    }
  } else {
    lowest_level = DBL_MAX;
    highest_level = ( -1.0 ) * DBL_MAX;  

    bool lh_levels_set = false; // lowest_level and highest_level values set
    
    TeRaster::iteratorPoly input_raster_it;
    TePDIPIManager progress( "Finding histogram range", progress_steps, 
      progress_int_enabled_ );
    double current_raster_level = 0;
    unsigned int curr_line = 0;
    unsigned int last_line = 0;    
    
    for( unsigned int local_polset_index = 0 ; 
      local_polset_index < local_polsetptr->size() ; 
      ++local_polset_index ) {
      
      input_raster_it = in_raster->begin( 
        (*local_polsetptr)[ local_polset_index ], iterstrat, 0 );
      
      while( ! input_raster_it.end() ) {
        current_raster_level = input_raster_it*( band );
        curr_line = input_raster_it.currentLine();
        
        if( in_raster_donot_uses_dummy || 
          ( current_raster_level != in_raster_dummy ) ) {

          lh_levels_set = true;

          if( current_raster_level > highest_level ) {
            highest_level = current_raster_level;
          }
          if( current_raster_level < lowest_level ) {
            lowest_level = current_raster_level;
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
  
  /* Calculating the histogram step and computed levels */

  double step = 1;
  unsigned int hist_computed_levels = 1;

  if( highest_level != lowest_level )
  {
    if( levels == 0 ) {
      /* Auto step feature */
      TEAGN_TRUE_OR_RETURN( ( ( highest_level - lowest_level ) > 1.0 ),
        "Histogram level range is too short for auto-step feature"
        " lowest_level=" + Te2String( lowest_level ) + " highest_level=" +
        Te2String( highest_level ) );
  
      hist_computed_levels = 1 +
        ( unsigned int ) floor( highest_level - lowest_level );
    } else {
      hist_computed_levels = levels;
    }
  
    TEAGN_TRUE_OR_RETURN( hist_computed_levels > 0,
      "Invalid levels number" );
  
    step = ( highest_level - lowest_level ) /
          ( (double)( hist_computed_levels - 1 ) );
  }

  /* Ordered Histogram buffer allocation */
  
  TePDIMatrix< double > hbuffer; /* line 0 - pixel level, 
                                    line 1 - pixels count for that level */
  TEAGN_TRUE_OR_RETURN( 
    hbuffer.Reset( 2, hist_computed_levels, 
    TePDIMatrix< double >::AutoMemPol ),
    "Unable to create histogram internal buffer" );
  {
    hbuffer( 0, 0 ) = lowest_level;
    hbuffer( 1, 0 ) = 0;
        
    for( unsigned int hbuffer_col = 1 ; 
      ( hbuffer_col < hbuffer.GetColumns() ); 
      ++hbuffer_col ) {
    
      hbuffer( 0, hbuffer_col ) = hbuffer( 0, hbuffer_col - 1 ) + step;
      hbuffer( 1, hbuffer_col ) = 0;
    }

    // Fixing the last value precision

    hbuffer( 0, hbuffer.GetColumns() - 1 ) = highest_level;
  }
  
  /* Interpolated histogram generation */
  
  {
    double current_raster_level = 0;
    unsigned int found_index = 0;
    
    TePDIPIManager progress( "Generating interpolated histogram", 
      progress_steps, progress_int_enabled_ );
      
    unsigned int curr_line = 0;
    unsigned int last_line = 0;      

    bool valid_pixel_found = false; // at least one valid pixel found
    
    for( unsigned int local_polset_index = 0 ; 
      local_polset_index < local_polsetptr->size() ; 
      ++local_polset_index ) {
      
      TeRaster::iteratorPoly input_raster_it = in_raster->begin( 
        (*local_polsetptr)[ local_polset_index ], iterstrat, 0 );    
    
      while( ! input_raster_it.end() ) {
        current_raster_level = input_raster_it*( band );
        curr_line = input_raster_it.currentLine();
        
        if( in_raster_donot_uses_dummy || 
          ( current_raster_level != in_raster_dummy ) ) {        

          valid_pixel_found = true;

          found_index = (unsigned int) TeRound( 
            ( current_raster_level - lowest_level ) / step );          
          
          TEAGN_DEBUG_CONDITION( ( found_index < hist_computed_levels ),
            "Invalid generated index position " + Te2String( found_index ) );
        
          hbuffer( 1, found_index ) = hbuffer( 1, found_index ) + 1;
        }
        
        if( curr_line != last_line ) {
          TEAGN_FALSE_OR_RETURN( progress.Increment(), 
            "Canceled by the user" );
          
          last_line = curr_line;
        }
        
        ++input_raster_it;
      }
    }

    // Return if no valid pixel was read

    if( ! valid_pixel_found )
    {
      return true;
    }
  }
      
  /* Transfering values from internal buffer to internal map */  
  
  for( unsigned int hbuffer_col = 0 ; hbuffer_col < hbuffer.GetColumns() ; 
    ++hbuffer_col ) 
  {
    if( hbuffer( 1, hbuffer_col ) != 0 )
    {
      operator[]( hbuffer( 0, hbuffer_col ) ) = 
        ( unsigned int ) hbuffer( 1, hbuffer_col );
    }
  }
  
  return true;
}


bool TePDIHistogram::hasFixedStep() const
{
  TEAGN_TRUE_OR_THROW( size() > 0, "Trying to use an empty histogram" );
  
  const_iterator it = begin();
  const_iterator it_end = end();
  
  std::vector< double > steps;
  std::vector< double >::iterator steps_it;
  std::vector< double >::iterator steps_it_end;
  
  double curr_step = 0;
  double last_value = it->first;
  bool step_found = false;
  
  ++it;
  
  while( it != it_end ) {
    curr_step = it->first - last_value;
    
    steps_it = steps.begin();
    steps_it_end = steps.end();
    
    step_found = false;
    
    while( steps_it != steps_it_end ) {
      if( (*steps_it) == curr_step ) {
        step_found = true;
        break;
      }
      
      ++steps_it;
    }
    
    if( ! step_found ) {
      steps.push_back( curr_step );
    }
    
    last_value = it->first;
    
    ++it;
  }
  
  return ( ( steps.size() <= 1 ) ? true : false );
}


bool TePDIHistogram::IsDiscrete() const
{
  TEAGN_TRUE_OR_THROW( size() > 0, "Trying to use an empty histogram" );
  
  const_iterator it = begin();
  const_iterator it_end = end();

  while( it != it_end ) {
    if( (it->first) != ( (double)TeRound(it->first) ) ) {
      return false;
    }
    
    ++it;
  }
  
  return true;
}


bool TePDIHistogram::Discretize()
{
  TEAGN_TRUE_OR_RETURN( size() > 0, "Trying to use an empty histogram" );
  TEAGN_TRUE_OR_RETURN( ( ( GetMaxLevel() - GetMinLevel() ) > 1 ),
    "The current histogram range do not allow discretization" );

  if( IsDiscrete() ) {
    return true;
  }
  
  TePDIPIManager progress( "Discretizing histogram...", size() * 2,
    progress_int_enabled_ );

  unsigned int progress_step = 0; 

  iterator it = begin();
  iterator it_end = end();

  TePDIHistogram temp_hist;

  while( it != it_end ) {
    progress.Update( progress_step );
    ++progress_step;  
  
    temp_hist[ TeRound( it->first ) ] = 0;

    ++it;
  }
  
  it = begin();
  
  while( it != it_end ) {
    progress.Update( progress_step );
    ++progress_step;  
  
    temp_hist[ TeRound( it->first ) ] += it->second;

    ++it;
  }

  TePDIHMapSpec::clear();
  
  operator=( temp_hist );

  progress.Toggle( false );

  return true;
}


double TePDIHistogram::GetMinLevel() const
{
  TEAGN_TRUE_OR_THROW( size() > 0, "Trying to use an empty histogram" );
  
  return begin()->first;
}


double TePDIHistogram::GetMaxLevel() const
{
  TEAGN_TRUE_OR_THROW( size() > 0, "Trying to use an empty histogram" );
  
  const_iterator it = end();
  --it;
  
  return it->first;
}


unsigned int TePDIHistogram::GetMinCount() const
{
  TEAGN_TRUE_OR_THROW( size() > 0, "Trying to use an empty histogram" );
  
  const_iterator it = begin();
  const_iterator it_end = end();
  
  unsigned int min_count = INT_MAX;

  while( it != it_end ) {
    if( (it->second) < min_count ) {
      min_count = it->second;
    }
    
    ++it;
  }
  
  return min_count;  
}


unsigned int TePDIHistogram::GetMaxCount() const
{
  TEAGN_TRUE_OR_THROW( size() > 0, "Trying to use an empty histogram" );
  
  const_iterator it = begin();
  const_iterator it_end = end();
  
  unsigned int max_count = 0;

  while( it != it_end ) {
    if( (it->second) > max_count ) {
      max_count = it->second;
    }
    
    ++it;
  }
  
  return max_count;  
}


unsigned int TePDIHistogram::getTotalCount() const
{
  const_iterator it = begin();
  const_iterator it_end = end();
  
  unsigned int total_count = 0;

  while( it != it_end ) {
    total_count += it->second;
    
    ++it;
  }
  
  return total_count;  
}


void TePDIHistogram::clear()
{
  TePDIHMapSpec::clear();
  init();
}


const TePDIHistogram& TePDIHistogram::operator=( 
  const TePDIHistogram& external )
{
  clear();
  
  TePDIHMapSpec::operator=( external );

  return *this;
}


const TePDIHistogram& TePDIHistogram::operator=( 
  const std::map< double, unsigned int >& external )
{
  clear();
  
  std::map< double, unsigned int >::const_iterator ext_it = external.begin();
  std::map< double, unsigned int >::const_iterator ext_it_end = 
    external.end();
    
  if( ext_it != ext_it_end ) {
    /* Verifying if the external histogram has floating point values 
       and guessing min and max */
    
    bool has_floating_point_values = false;
    
    const double min = ext_it->first;
      
    while( ext_it != ext_it_end ) {
      if( ext_it->first != TeRound( ext_it->first ) ) {
        has_floating_point_values = true;
      }
      
      ++ext_it;
    }
    
    --ext_it;
    const double max = ext_it->first;
    
    if( has_floating_point_values ) {
      /* building primary interpolated histogram */
    
      double step_size = ( max - min ) / ( (double)external.size() - 1 );
      
      for( unsigned int step = 0 ; step < external.size() ; ++step ) {
        operator[]( ( (double)step ) * step_size ) = 0;
      }
      
      /* Interpolating values */
      
      ext_it = external.begin();
      TePDIHistogram::iterator hist_it;
      TePDIHistogram::iterator hist_it_aux;
      double left_value = 0;
      double right_value = 0;
      double curr_value = 0;
      
      while( ext_it != ext_it_end ) {
        curr_value = ext_it->first;
      
        hist_it = find( curr_value );
        
        if( hist_it == end() ) {
          hist_it = upper_bound( curr_value );
          
          right_value = hist_it->first;
          
          hist_it_aux = hist_it;
          --hist_it_aux;
          left_value = hist_it_aux->first;
          
          if( ( right_value - curr_value ) > ( curr_value - left_value ) ) {
            hist_it_aux->second += ext_it->second;
          } else {
            hist_it->second += ext_it->second;
          }
        } else {
          hist_it->second += ext_it->second;
        }
        
        ++ext_it;
      }
    } else {
      /* Building discrete histogram */
      
      unsigned int min_ui = ( (unsigned int) min );
      unsigned int max_ui = ( (unsigned int)max );
      
      unsigned int steps = 1 + max_ui - min_ui;
      
      if( steps == 1 ) {
        operator[]( external.begin()->first ) = external.begin()->second;
      } else {
        for( unsigned int level = min_ui ; level <= max_ui ; ++level ) {
          ext_it = external.find( (double)level );
          
          if( ext_it != ext_it_end ) {
            operator[]( (double)level ) = ext_it->second;
          } else {
            operator[]( (double)level ) = 0;  
          }
        }
      }
    }
  }
  
  return *this;
}

bool TePDIHistogram::operator==( const TePDIHistogram& external ) const
{
  if( size() == external.size() )
  {
    const_iterator myIt = begin();
    const_iterator myIt_end = end();
    const_iterator extIt = external.begin();
    
    while( myIt != myIt_end )
    {
      if( ( myIt->first != extIt->first ) || ( myIt->second != 
        extIt->second ) )
      {
        return false;
      }
      
      ++extIt;
      ++myIt;
    }
    
    return true;
  }
  else
  {
    return false;
  }
}

unsigned int TePDIHistogram::size() const
{
  return (unsigned int)TePDIHMapSpec::size();
}

std::string TePDIHistogram::toString() const
{
  std::string outStr;
  
  const_iterator it = begin();
  const_iterator it_end = end();
  
  while( it != it_end ) 
  {
    outStr.append( "[" + Te2String( it->first, 2 ) + "->" +
      Te2String( it->second ) + "]" );
    
    ++it;
  }
  
  return outStr;
}

void TePDIHistogram::init()
{
  progress_int_enabled_ = true;
}


void TePDIHistogram::ToggleProgressInt( bool enabled )
{
  progress_int_enabled_ = enabled;
}


bool TePDIHistogram::SetPaletteBasedH(
  const TePDITypes::TePDIRasterPtrType& in_raster,
  unsigned int band,
  TeStrategicIterator iterstrat,
  const TeSharedPtr< TePolygonSet>& polsetptr )
{
  TEAGN_DEBUG_CONDITION( in_raster.isActive(),
    "Inactive raster pointer" );
  TEAGN_DEBUG_CONDITION( 
    in_raster->params().status_ != TeRasterParams::TeNotReady,
    "Raster not ready to read" );
  TEAGN_DEBUG_CONDITION( ( (int)band < in_raster->nBands() ),
    "Invalid band" );

  TEAGN_TRUE_OR_RETURN( ( band < 3 ), "Invalid band" );
  TEAGN_TRUE_OR_RETURN( (
    ( in_raster->params().dataType_[ band ] == TeUNSIGNEDCHAR ) ||
    ( in_raster->params().dataType_[ band ] == TeUNSIGNEDSHORT ) ||
    ( in_raster->params().dataType_[ band ] == TeUNSIGNEDLONG ) ),
    "Invalid pixel data type" );
  TEAGN_TRUE_OR_RETURN( ( in_raster->params().photometric_[ band ] == 
    TeRasterParams::TePallete ), "Incorrect photometric" )
    
  TePDIHMapSpec::clear();

  const unsigned long int progress_steps = 
    getProgressSteps( polsetptr, in_raster->params().resy_ );    

  /* Dummy use check */
  
  bool in_raster_donot_uses_dummy = ! in_raster->params().useDummy_;
  double in_raster_dummy = 0;
  
  if( ! in_raster_donot_uses_dummy ) {
    in_raster_dummy = in_raster->params().dummy_[ band ];
  }
  
  /* Defining the current lut pointer */
  
  vector< unsigned short >* lut_ptr = 0;
  
  if( band == 0 ) {
    lut_ptr = &( in_raster->params().lutr_ );
  } else if( band == 1 ) {
    lut_ptr = &( in_raster->params().lutg_ );
  } else {
    lut_ptr = &( in_raster->params().lutb_ );
  }
    
  /* Building ordered histogram */
  
  {
    TePDIPIManager progress( "Generating histogram", 
      progress_steps, progress_int_enabled_ );
  
    double current_lut_index = 0;
    unsigned short current_raster_level = 0;
    TeRaster::iteratorPoly input_raster_it;
    unsigned int curr_line = 0;
    unsigned int last_line = 0;
    
    for( unsigned int local_polset_index = 0 ; 
      local_polset_index < polsetptr->size() ; 
      ++local_polset_index ) {
      
      input_raster_it = in_raster->begin( 
        (*polsetptr)[ local_polset_index ], iterstrat, 0 );
      
      while( ! input_raster_it.end() ) {
        curr_line = input_raster_it.currentLine();
      
        current_lut_index = input_raster_it*( band );
        
        if( in_raster_donot_uses_dummy || 
          ( current_lut_index != in_raster_dummy ) ) {
          
          TEAGN_DEBUG_CONDITION( ( ( ( unsigned long int )current_lut_index ) 
            < lut_ptr->size() ), "Invalid lut index" )
          
          current_raster_level = (*lut_ptr)[ ( unsigned long int )
            current_lut_index ];
            
          ++( operator[]( current_raster_level ) );

        }
        
        if( curr_line != last_line ) {
          TEAGN_FALSE_OR_RETURN( progress.Increment(), 
            "Canceled by the user" );
          
          last_line = curr_line;
        }
        
        ++input_raster_it;
      }
    }  
  }

  return true;
}


bool TePDIHistogram::Set8BitH(
  const TePDITypes::TePDIRasterPtrType& in_raster,
  unsigned int band,
  TeStrategicIterator iterstrat,
  const TeSharedPtr< TePolygonSet>& polsetptr )
{
  TEAGN_DEBUG_CONDITION( in_raster.isActive(),
    "Inactive raster pointer" );
  TEAGN_DEBUG_CONDITION( 
    in_raster->params().status_ != TeRasterParams::TeNotReady,
    "Raster not ready to read" );
  TEAGN_DEBUG_CONDITION( (int)band < in_raster->nBands(),
    "Invalid band" );
  TEAGN_DEBUG_CONDITION( ( ! TePDIUtils::IsFloatBand( in_raster, band ) ),
    "Unable to build non-interpolated histogram from float pixels type" );
  TEAGN_DEBUG_CONDITION( (
    ( in_raster->params().dataType_[ band ] == TeUNSIGNEDCHAR ) ||
    in_raster->params().dataType_[ band ] == TeCHAR ),
    "Invalid pixel data type" );
    
  TePDIHMapSpec::clear();

//  unsigned int in_lines_number = in_raster->params().nlines_;
//  unsigned int in_columns_number = in_raster->params().ncols_;
  const unsigned long int progress_steps = 
    getProgressSteps( polsetptr, in_raster->params().resy_ );    
    
  /* Defining level offset based on data bype */
  
  unsigned int level_offset = 0;
  
  if( in_raster->params().dataType_[ band ] == TeCHAR ) {
    level_offset = 127; /*( 256 / 2 ) - 1 */
  }
    
  /* Building histogram */
  
  unsigned int init_hist[ 256 ];
  unsigned int hist_index = 0;
  
  for( hist_index = 0 ; hist_index < 256 ; ++hist_index ) {
    init_hist[ hist_index ] = 0;
  }
  
  /* Analysing raster */
  
  {
    TePDIPIManager progress( "Generating histogram", progress_steps, 
      progress_int_enabled_ );
    int int_pixel_value = 0;
  
    double current_raster_level = 0;
    TeRaster::iteratorPoly input_raster_it;
    std::map< double, unsigned int >::iterator it;
    TeRaster& input_raster_ref = *in_raster;
    
    unsigned int curr_line = 0;
    unsigned int curr_col = 0;
    unsigned int last_line = 0;    
    
    for( unsigned int local_polset_index = 0 ; 
      local_polset_index < polsetptr->size() ; 
      ++local_polset_index ) {
      
      input_raster_it = input_raster_ref.begin( 
        (*polsetptr)[ local_polset_index ], iterstrat, 0 );
      
      while( ! input_raster_it.end() ) {
        curr_line = input_raster_it.currentLine();
        curr_col = input_raster_it.currentColumn();
        
        if( input_raster_ref.getElement( curr_col, curr_line, 
          current_raster_level, band ) ) 
        {
          int_pixel_value = ( int ) current_raster_level;
          
          TEAGN_DEBUG_CONDITION( ( int_pixel_value + level_offset ) < 256,
            "Invalid histogram position" )
            
          ++( init_hist[ int_pixel_value + level_offset ] );
        }
        
        if( curr_line != last_line ) {
          TEAGN_FALSE_OR_RETURN( progress.Increment(), 
            "Canceled by the user" );
          
          last_line = curr_line;
        }
        
        ++input_raster_it;
      }
    }  
  }  
 
  /* Building ordered histogram */
  
  for( hist_index = 0; hist_index < 256 ; 
       ++hist_index ) 
  {
    if( init_hist[ hist_index ] )
    {
      operator[]( ( (double)hist_index ) - ( (double)level_offset ) ) = 
        init_hist[ hist_index ];
    }
  }
  
  return true;
}


bool TePDIHistogram::Set16BitH(
  const TePDITypes::TePDIRasterPtrType& in_raster,
  unsigned int band,
  TeStrategicIterator iterstrat,
  const TeSharedPtr< TePolygonSet>& polsetptr )
{
  TEAGN_DEBUG_CONDITION( in_raster.isActive(),
    "Inactive raster pointer" );
  TEAGN_DEBUG_CONDITION( 
    in_raster->params().status_ != TeRasterParams::TeNotReady,
    "Raster not ready to read" );
  TEAGN_DEBUG_CONDITION( (int)band < in_raster->nBands(),
    "Invalid band" );
  TEAGN_DEBUG_CONDITION( ( ! TePDIUtils::IsFloatBand( in_raster, band ) ),
    "Unable to build non-interpolated histogram from float pixels type" );
  TEAGN_DEBUG_CONDITION( (
    ( in_raster->params().dataType_[ band ] == TeSHORT ) ||
    ( in_raster->params().dataType_[ band ] == TeUNSIGNEDSHORT ) ),
    "Invalid pixel data type" );
    
  TePDIHMapSpec::clear();

//  unsigned int in_lines_number = in_raster->params().nlines_;
//  unsigned int in_columns_number = in_raster->params().ncols_;
  const unsigned long int progress_steps = 
    getProgressSteps( polsetptr, in_raster->params().resy_ );   
  
  /* Dummy use check */
  
  bool in_raster_donot_uses_dummy = ! in_raster->params().useDummy_;
  double in_raster_dummy = 0;
  
  if( ! in_raster_donot_uses_dummy ) {
    in_raster_dummy = in_raster->params().dummy_[ band ];
  }     
    
  /* Defining level offset base on data bype */
  
  unsigned int level_offset = 0;
  
  if( in_raster->params().dataType_[ band ] == TeSHORT ) {
    level_offset = 32767; /*( 256 / 2 ) - 1 */
  }    
    
  /* Building histogram */
  
  unsigned int init_hist[ 65536 ];
  unsigned int hist_index = 0;
  
  for( hist_index = 0 ; hist_index < 65536 ; ++hist_index ) {
    init_hist[ hist_index ] = 0;
  }
  
  /* Analysing raster */
  
  {
    TePDIPIManager progress( "Generating histogram", progress_steps,
      progress_int_enabled_ );
    int int_pixel_value = 0;
    
    unsigned int curr_line = 0;
    unsigned int last_line = 0;    
  
    double current_raster_level = 0;
    TeRaster::iteratorPoly input_raster_it;
    std::map< double, unsigned int >::iterator it;
    
    for( unsigned int local_polset_index = 0 ; 
      local_polset_index < polsetptr->size() ; 
      ++local_polset_index ) {
      
      input_raster_it = in_raster->begin( 
        (*polsetptr)[ local_polset_index ], iterstrat, 0 );
      
      while( ! input_raster_it.end() ) {
        current_raster_level = input_raster_it*( band );
        curr_line = input_raster_it.currentLine();
        
        if( in_raster_donot_uses_dummy || 
          ( current_raster_level != in_raster_dummy ) ) {
          
          int_pixel_value = ( int ) current_raster_level;
            
          init_hist[ int_pixel_value + level_offset ] = init_hist[ 
            int_pixel_value + level_offset ] + 1;  
        }
        
        if( curr_line != last_line ) {
          TEAGN_FALSE_OR_RETURN( progress.Increment(), 
            "Canceled by the user" );
          
          last_line = curr_line;
        }
        
        ++input_raster_it;
      }
    }  
  }    
  
  /* Building ordered histogram */
  
  for( hist_index = 0; hist_index < 65536 ; ++hist_index ) 
  {
    if( init_hist[ hist_index ] )
    {
      operator[]( ( (double)hist_index ) - ( (double)level_offset ) ) = 
        init_hist[ hist_index ];
    }
  }
  
  return true;  
}


unsigned long int TePDIHistogram::getProgressSteps( 
  const TeSharedPtr< TePolygonSet>& polsetptr, double resy )
{
  unsigned long int steps = 0;

  for( unsigned int ps_index = 0 ; ps_index < polsetptr->size() ; 
    ++ps_index ) {
    
    const TeBox& polbox( (*polsetptr)[ ps_index ].box() );
    steps += ( unsigned long int)ABS( ( polbox.y2() - polbox.y1() ) / resy );
  }
  
  return steps;
}

