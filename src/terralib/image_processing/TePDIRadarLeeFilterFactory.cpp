 
#include "TePDIRadarLeeFilterFactory.hpp"
#include "TePDIRadarLeeFilter.hpp"
#include "../kernel/TeAgnostic.h"

TePDIRadarLeeFilterFactory::TePDIRadarLeeFilterFactory()
: TePDIAlgorithmFactory( std::string( "TePDIRadarLeeFilter" ) )
{
};      

TePDIRadarLeeFilterFactory::~TePDIRadarLeeFilterFactory()
{
};


TePDIAlgorithm* TePDIRadarLeeFilterFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIRadarLeeFilter();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
