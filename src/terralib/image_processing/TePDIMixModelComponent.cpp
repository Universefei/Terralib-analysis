#include "TePDIMixModelComponent.hpp"

TePDIMixModelComponent::TePDIMixModelComponent(string l)
{
	label = l;
}

TePDIMixModelComponent::~TePDIMixModelComponent()
{
}

bool TePDIMixModelComponent::insertPixel(unsigned int bn, double p)
{
	pixels.insert(pair<unsigned int, double>(bn, p));

	return true;
}

bool TePDIMixModelComponent::removePixel(unsigned int bn)
{
	map<unsigned int, double>::iterator it = pixels.find(bn);

	if (it == pixels.end())
		return false;

	pixels.erase(bn);

	return true;
}

double TePDIMixModelComponent::getPixel(unsigned int bn)
{
	map<unsigned int, double>::iterator it = pixels.find(bn);

	if (it == pixels.end())
		return -1.0;

	return it->second;
}

unsigned TePDIMixModelComponent::getSize()
{
	return pixels.size();
}

void TePDIMixModelComponent::clearPixels()
{
	pixels.clear();
}

bool TePDIMixModelComponent::operator==( 
  const TePDIMixModelComponent& external ) const
{
  return ( ( label == external.label ) && ( pixels == external.pixels ) );
}
