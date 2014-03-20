 
#include "TePDIPrincipalComponentsFactory.hpp"
#include "TePDIPrincipalComponents.hpp"
#include "../kernel/TeAgnostic.h"

TePDIPrincipalComponentsFactory::TePDIPrincipalComponentsFactory()
: TePDIAlgorithmFactory( std::string( "TePDIPrincipalComponents" ) )
{
};      

TePDIPrincipalComponentsFactory::~TePDIPrincipalComponentsFactory()
{
};


TePDIAlgorithm* TePDIPrincipalComponentsFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIPrincipalComponents();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
