#include "TePDIPrincoMixModelSF.hpp"
#include "TePDIPrincoMixModelStrategy.hpp"

TePDIPrincoMixModelSF::TePDIPrincoMixModelSF()
: TePDIMixModelStratFactory( std::string( "princo" ) )
{
};


TePDIPrincoMixModelSF::~TePDIPrincoMixModelSF()
{
};


TePDIMixModelStrategy* TePDIPrincoMixModelSF::build ( const TePDIParameters& )
{
  TePDIMixModelStrategy* instance_ptr = new TePDIPrincoMixModelStrategy();
  
  return instance_ptr;
}

