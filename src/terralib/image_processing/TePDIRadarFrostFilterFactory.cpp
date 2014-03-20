 
#include "TePDIRadarFrostFilterFactory.hpp"
#include "TePDIRadarFrostFilter.hpp"
#include "../kernel/TeAgnostic.h"

TePDIRadarFrostFilterFactory::TePDIRadarFrostFilterFactory()
: TePDIAlgorithmFactory( std::string( "TePDIRadarFrostFilter" ) )
{
};      

TePDIRadarFrostFilterFactory::~TePDIRadarFrostFilterFactory()
{
};


TePDIAlgorithm* TePDIRadarFrostFilterFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIRadarFrostFilter();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
