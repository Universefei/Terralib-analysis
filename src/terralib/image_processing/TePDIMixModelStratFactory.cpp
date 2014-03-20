 
#include "TePDIMixModelStratFactory.hpp"

TePDIMixModelStratFactory::TePDIMixModelStratFactory( 
  const std::string& factoryName )
: TeFactory< TePDIMixModelStrategy, TePDIParameters >( factoryName )
{
};      

TePDIMixModelStratFactory::~TePDIMixModelStratFactory()
{
};
