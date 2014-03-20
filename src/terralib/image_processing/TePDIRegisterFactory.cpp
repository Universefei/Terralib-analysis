 
#include "TePDIRegisterFactory.hpp"
#include "TePDIRegister.hpp"
#include "../kernel/TeAgnostic.h"


TePDIRegisterFactory::TePDIRegisterFactory()
: TePDIAlgorithmFactory( std::string( "TePDIRegister" ) )
{
};      


TePDIRegisterFactory::~TePDIRegisterFactory()
{
};


TePDIAlgorithm* TePDIRegisterFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIRegister();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
