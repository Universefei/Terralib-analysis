 
#include "TePDIParaSegStrategyFactory.hpp"

TePDIParaSegStrategyFactory::TePDIParaSegStrategyFactory( 
  const std::string& factoryName )
: TeFactory< TePDIParaSegStrategy, TePDIParaSegStrategyFactoryParams >( 
  factoryName )
{
};      

TePDIParaSegStrategyFactory::~TePDIParaSegStrategyFactory()
{
};

