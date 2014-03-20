#include "TePDIMixModelComponentList.hpp"

TePDIMixModelComponentList::TePDIMixModelComponentList()
{
}

TePDIMixModelComponentList::~TePDIMixModelComponentList()
{
}

bool TePDIMixModelComponentList::insertComponent(unsigned int cn, TePDIMixModelComponent c)
{
	components.insert(pair<unsigned int, TePDIMixModelComponent>(cn, c));

	return true;
}

bool TePDIMixModelComponentList::insertComponent(unsigned int cn, string l)
{
	components.insert(pair<unsigned int, TePDIMixModelComponent>(cn, TePDIMixModelComponent(l)));

	return true;
}

bool TePDIMixModelComponentList::removeComponent(unsigned int cn)
{
	components.erase(cn);

	return true;
}

unsigned int TePDIMixModelComponentList::getSize()
{
	return components.size();
}

bool TePDIMixModelComponentList::insertPixel(unsigned int cn, unsigned int bn, double p)
{
	map<unsigned int, TePDIMixModelComponent>::iterator it = components.find(cn);

	if (it == components.end())
		return false;

	it->second.insertPixel(bn, p);

	return true;
}

bool TePDIMixModelComponentList::removePixel(unsigned int cn, unsigned int bn)
{
	map<unsigned int, TePDIMixModelComponent>::iterator it = components.find(cn);

	if (it == components.end())
		return false;

	it->second.removePixel(bn);

	return true;
}

double TePDIMixModelComponentList::getPixel(unsigned int cn, unsigned int bn)
{
	map<unsigned int, TePDIMixModelComponent>::iterator it = components.find(cn);

	if (it == components.end())
		return -1.0;

	return it->second.getPixel(bn);
}

int TePDIMixModelComponentList::getComponentSize(unsigned cn)
{
	map<unsigned int, TePDIMixModelComponent>::iterator it = components.find(cn);

	if (it == components.end())
		return -1;

	return it->second.getSize();
}

bool TePDIMixModelComponentList::operator==( 
  const TePDIMixModelComponentList& external ) const
{
  return ( components == external.components );
}
