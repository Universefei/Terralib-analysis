 
#include "TePDIContrastFactory.hpp"
#include "TePDIContrast.hpp"
#include "../kernel/TeAgnostic.h"

TePDIContrastFactory::TePDIContrastFactory()
: TePDIAlgorithmFactory( std::string( "TePDIContrast" ) )
{
};      

TePDIContrastFactory::~TePDIContrastFactory()
{
};


TePDIAlgorithm* TePDIContrastFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIContrast();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
