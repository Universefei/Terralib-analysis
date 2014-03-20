 
#include "TePDIRegGrowSegFactory.hpp"
#include "TePDIRegGrowSeg.hpp"
#include "../kernel/TeAgnostic.h"


TePDIRegGrowSegFactory::TePDIRegGrowSegFactory()
: TePDIAlgorithmFactory( std::string( "TePDIRegGrowSeg" ) )
{
}


TePDIRegGrowSegFactory::~TePDIRegGrowSegFactory()
{
}


TePDIAlgorithm* TePDIRegGrowSegFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIRegGrowSeg();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
