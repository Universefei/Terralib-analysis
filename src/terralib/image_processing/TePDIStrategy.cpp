#include "TePDIStrategy.hpp"
#include "../kernel/TeAgnostic.h"

TePDIStrategy::TePDIStrategy()
{
  progress_interface_enabled_ = true;
};      


TePDIStrategy::~TePDIStrategy()
{
};


TePDIStrategy* TePDIStrategy::DefaultObject( const TePDIParameters& )
{
  TEAGN_LOG_AND_THROW( "Trying to build an invalid strategy instance" );
  return 0;
}


bool TePDIStrategy::Apply( const TePDIParameters& params )
{
  TEAGN_TRUE_OR_RETURN( CheckParameters( params ), 
    "Parameter checking failed" );
    
  return Implementation( params );
}


void TePDIStrategy::ToggleProgInt( bool enabled )
{
  progress_interface_enabled_ = enabled;
}

