 
#include "TePDILinearFilterFactory.hpp"
#include "TePDILinearFilter.hpp"
#include "../kernel/TeAgnostic.h"

TePDILinearFilterFactory::TePDILinearFilterFactory()
: TePDIAlgorithmFactory( std::string( "TePDILinearFilter" ) )
{
};      

TePDILinearFilterFactory::~TePDILinearFilterFactory()
{
};


TePDIAlgorithm* TePDILinearFilterFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDILinearFilter();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}

