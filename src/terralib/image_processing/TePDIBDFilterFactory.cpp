 
#include "TePDIBDFilterFactory.hpp"
#include "TePDIBDFilter.hpp"
#include "../kernel/TeAgnostic.h"

TePDIBDFilterFactory::TePDIBDFilterFactory()
: TePDIAlgorithmFactory( std::string( "TePDIBDFilter" ) )
{
};      

TePDIBDFilterFactory::~TePDIBDFilterFactory()
{
};


TePDIAlgorithm* TePDIBDFilterFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIBDFilter();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
