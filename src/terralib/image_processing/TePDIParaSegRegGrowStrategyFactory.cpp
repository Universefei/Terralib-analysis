#include "TePDIParaSegRegGrowStrategyFactory.hpp"
#include "TePDIParaSegRegGrowStrategy.hpp"

namespace
{
  TePDIParaSegRegGrowStrategyFactory tePDIParaSegRegGrowStrategyFactoryInstance_;
}

TePDIParaSegRegGrowStrategyFactory::TePDIParaSegRegGrowStrategyFactory()
  : TePDIParaSegStrategyFactory( "RegionGrowing" )
{
}

TePDIParaSegRegGrowStrategyFactory::~TePDIParaSegRegGrowStrategyFactory()
{
}

TePDIParaSegStrategy* TePDIParaSegRegGrowStrategyFactory::build( 
  const TePDIParaSegStrategyFactoryParams& arg )
{
  return new TePDIParaSegRegGrowStrategy( arg.stratParams_ );
}


