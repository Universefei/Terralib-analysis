#include "TePDIRgbPaletteFunctions.hpp"

#include "../kernel/TeAgnostic.h"

#include "../kernel/TeUtils.h"

#include <math.h>

namespace TePDIRgbPaletteFunctions
{

  TePDIRgbPalette::pointer createLSBPalette( unsigned int levels )
  {
    TEAGN_TRUE_OR_THROW( levels > 0, "Invalid supplied Levels" );
  
    unsigned int channel_levels =
      (unsigned int) ceil( TeCubicRoot( (double)levels ) );
  
    unsigned int channel_step =
      (unsigned int) floor( 256. / (float)(channel_levels + 1) );
  
    TePDIRgbPalette::pointer outPal( new TePDIRgbPalette );
  
    unsigned int level = 0;
    
    TePDIRgbPaletteNode temp_node;
  
    while( level < levels ) {
      if( temp_node.red_ > 255 ) {
        temp_node.red_ = temp_node.red_ % 255;
  
        temp_node.green_ += channel_step;
  
        if( temp_node.green_ > 255 ) {
          temp_node.green_ = temp_node.green_ % 255;
  
          temp_node.blue_ += channel_step;
  
          if( temp_node.blue_ > 255 ) {
            temp_node.blue_ = temp_node.blue_ % 255;
          }
        }
      }
      
      (*outPal)[ level ] = temp_node;
  
      temp_node.red_ += channel_step;
  
      ++level;
    }
  
    return outPal;
  }
  
};


