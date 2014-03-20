#include "TePDIMathFunctions.hpp"
#include "../kernel/TeAgnostic.h"

#include <math.h>

namespace TePDIMathFunctions {

  unsigned long int DecimLevelSize( unsigned long int level,
    unsigned long int elements )
  {
    TEAGN_TRUE_OR_THROW( ( level <= DecimLevels( elements ) ),
      "Invalid required level for the current elements number" );
    
    unsigned long int current_level = 0;
    unsigned long int size = elements;
      
    do {
      if( size < 2 ) {
        break;
      } else {
        size = ( ( size + ( size % 2 ) ) / 2 );
      }
        
      ++current_level;
    } while( current_level != level );
      
    return size;
  }
  
}

