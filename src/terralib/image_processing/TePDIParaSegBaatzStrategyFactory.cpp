#include "TePDIParaSegBaatzStrategyFactory.hpp"
#include "TePDIParaSegBaatzStrategy.hpp"

namespace
{
  TePDIParaSegBaatzStrategyFactory tePDIParaSegBaatzStrategyFactoryInstance_;
}

TePDIParaSegBaatzStrategyFactory::TePDIParaSegBaatzStrategyFactory()
  : TePDIParaSegStrategyFactory( "Baatz" )
{
}

TePDIParaSegBaatzStrategyFactory::~TePDIParaSegBaatzStrategyFactory()
{
}

TePDIParaSegStrategy* TePDIParaSegBaatzStrategyFactory::build( 
  const TePDIParaSegStrategyFactoryParams& arg )
{
  return new TePDIParaSegBaatzStrategy( arg.stratParams_ );
}


