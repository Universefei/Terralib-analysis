
#include "TePDISensorSimulatorFactory.hpp"
#include "TePDISensorSimulator.hpp"

#include "../kernel/TeAgnostic.h"


TePDISensorSimulatorFactory::TePDISensorSimulatorFactory()
: TePDIAlgorithmFactory( std::string( "TePDISensorSimulator" ) )
{
};      


TePDISensorSimulatorFactory::~TePDISensorSimulatorFactory()
{
};


TePDIAlgorithm* TePDISensorSimulatorFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDISensorSimulator();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
