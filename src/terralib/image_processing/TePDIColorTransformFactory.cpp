 
#include "TePDIColorTransformFactory.hpp"
#include "TePDIColorTransform.hpp"
#include "../kernel/TeAgnostic.h"

TePDIColorTransformFactory::TePDIColorTransformFactory()
: TePDIAlgorithmFactory( std::string( "TePDIColorTransform" ) )
{
};      

TePDIColorTransformFactory::~TePDIColorTransformFactory()
{
};


TePDIAlgorithm* TePDIColorTransformFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIColorTransform();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
