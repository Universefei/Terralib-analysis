 
#include "TePDIRadarKuanFilterFactory.hpp"
#include "TePDIRadarKuanFilter.hpp"
#include "../kernel/TeAgnostic.h"

TePDIRadarKuanFilterFactory::TePDIRadarKuanFilterFactory()
: TePDIAlgorithmFactory( std::string( "TePDIRadarKuanFilter" ) )
{
};      

TePDIRadarKuanFilterFactory::~TePDIRadarKuanFilterFactory()
{
};


TePDIAlgorithm* TePDIRadarKuanFilterFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIRadarKuanFilter();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
