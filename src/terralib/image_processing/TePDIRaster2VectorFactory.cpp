 
#include "TePDIRaster2VectorFactory.hpp"
#include "TePDIRaster2Vector.hpp"
#include "../kernel/TeAgnostic.h"

TePDIRaster2VectorFactory::TePDIRaster2VectorFactory()
: TePDIAlgorithmFactory( std::string( "TePDIRaster2Vector" ) )
{
};      

TePDIRaster2VectorFactory::~TePDIRaster2VectorFactory()
{
};


TePDIAlgorithm* TePDIRaster2VectorFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIRaster2Vector();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
