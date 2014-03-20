#include "TePDIMixModelSpectralBand.hpp"

TePDIMixModelSpectralBand::TePDIMixModelSpectralBand(unsigned int s, unsigned int b, double ll, double ul, string l)
{
	sensor = s;
	band = b;
	lowerlimit = ll;
	upperlimit = ul;
	label = l;
}

TePDIMixModelSpectralBand::~TePDIMixModelSpectralBand()
{
}

unsigned int TePDIMixModelSpectralBand::getSensor()
{
	return sensor;
}

unsigned int TePDIMixModelSpectralBand::getBand()
{
	return band;
}

double TePDIMixModelSpectralBand::getLowerLimit()
{
	return lowerlimit;
}

double TePDIMixModelSpectralBand::getUpperLimit()
{
	return upperlimit;
}

string TePDIMixModelSpectralBand::getLabel()
{
	return label;
}

bool TePDIMixModelSpectralBand::operator==( 
  const TePDIMixModelSpectralBand& external ) const
{
  return ( ( sensor == external.sensor ) && ( band == external.band ) 
    && ( lowerlimit == external.lowerlimit ) 
    && ( upperlimit == external.upperlimit ) 
    && ( label == external.label ) );
}

