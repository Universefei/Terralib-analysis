 
#include "TePDIArithmeticFactory.hpp"
#include "TePDIArithmetic.hpp"
#include "../kernel/TeAgnostic.h"

TePDIArithmeticFactory::TePDIArithmeticFactory()
: TePDIAlgorithmFactory( std::string( "TePDIArithmetic" ) )
{
};      

TePDIArithmeticFactory::~TePDIArithmeticFactory()
{
};


TePDIAlgorithm* TePDIArithmeticFactory::build ( const TePDIParameters& arg )
{
  TePDIAlgorithm* instance_ptr = new TePDIArithmetic();
  
  TEAGN_TRUE_OR_THROW( instance_ptr->Reset( arg ),
    "Invalid parameters" );
  
  return instance_ptr;
}

