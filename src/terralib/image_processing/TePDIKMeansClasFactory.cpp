
#include "TePDIKMeansClasFactory.hpp"
#include "TePDIKMeansClas.hpp"
#include "../kernel/TeAgnostic.h"


TePDIKMeansClasFactory::TePDIKMeansClasFactory()
: TePDIAlgorithmFactory( std::string( "TePDIKMeansClas" ) )
{
}

TePDIKMeansClasFactory::~TePDIKMeansClasFactory()
{
}


TePDIAlgorithm* TePDIKMeansClasFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIKMeansClas();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}
