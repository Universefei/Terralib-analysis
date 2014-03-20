 
#include "TePDIStrategyFactory.hpp"

TePDIStrategyFactory::TePDIStrategyFactory( const std::string& factoryName )
: TeFactory< TePDIStrategy, TePDIParameters >( factoryName )
{
};      

TePDIStrategyFactory::~TePDIStrategyFactory()
{
};
