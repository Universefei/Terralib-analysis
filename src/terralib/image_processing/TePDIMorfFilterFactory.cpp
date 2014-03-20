 
#include "TePDIMorfFilterFactory.hpp"
#include "TePDIMorfFilter.hpp"
#include "../kernel/TeAgnostic.h"

TePDIMorfFilterFactory::TePDIMorfFilterFactory()
: TePDIAlgorithmFactory( std::string( "TePDIMorfFilter" ) )
{
};      

TePDIMorfFilterFactory::~TePDIMorfFilterFactory()
{
};


TePDIAlgorithm* TePDIMorfFilterFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIMorfFilter();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
