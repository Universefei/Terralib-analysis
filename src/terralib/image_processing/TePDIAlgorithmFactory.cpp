 
#include "TePDIAlgorithmFactory.hpp"

TePDIAlgorithmFactory::TePDIAlgorithmFactory( const std::string& factoryName )
: TeFactory< TePDIAlgorithm, TePDIParameters >( factoryName )
{
};      

TePDIAlgorithmFactory::~TePDIAlgorithmFactory()
{
};
