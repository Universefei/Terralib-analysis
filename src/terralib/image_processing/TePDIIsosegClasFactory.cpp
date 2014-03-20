
#include "TePDIIsosegClasFactory.hpp"
#include "TePDIIsosegClas.hpp"
#include "../kernel/TeAgnostic.h"


TePDIIsosegClasFactory::TePDIIsosegClasFactory()
: TePDIAlgorithmFactory( std::string( "TePDIIsosegClas" ) )
{
}

TePDIIsosegClasFactory::~TePDIIsosegClasFactory()
{
}


TePDIAlgorithm* TePDIIsosegClasFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIIsosegClas();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
