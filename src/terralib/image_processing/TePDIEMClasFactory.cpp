
#include "TePDIEMClasFactory.hpp"
#include "TePDIEMClas.hpp"
#include "../kernel/TeAgnostic.h"


TePDIEMClasFactory::TePDIEMClasFactory()
: TePDIAlgorithmFactory( std::string( "TePDIEMClas" ) )
{
}

TePDIEMClasFactory::~TePDIEMClasFactory()
{
}


TePDIAlgorithm* TePDIEMClasFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIEMClas();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
