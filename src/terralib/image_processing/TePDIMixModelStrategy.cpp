#include "TePDIMixModelStrategy.hpp"
#include "../kernel/TeAgnostic.h"


TePDIMixModelStrategy::TePDIMixModelStrategy()
{
};      


TePDIMixModelStrategy::~TePDIMixModelStrategy()
{
};


TePDIMixModelStrategy* TePDIMixModelStrategy::DefaultObject( 
  const TePDIParameters& )
{
  TEAGN_LOG_AND_THROW( 
    "Trying to build an invalid mixmodel strategy instance" );
  return 0; 
}

