#include "TePDIMixModelSpectralBandList.hpp"

#include <cstdio>

TePDIMixModelSpectralBandList::TePDIMixModelSpectralBandList()
{
}

TePDIMixModelSpectralBandList::~TePDIMixModelSpectralBandList()
{
}

bool TePDIMixModelSpectralBandList::insertSpectralBand(TePDIMixModelSpectralBand sb)
{
	bands.push_back(sb);

	return true;
}

bool TePDIMixModelSpectralBandList::loadSpectralBandsFromFile(string filename)
{
	FILE *fsb = fopen(filename.c_str(), "r");

	if (fsb != NULL)
	{
		TEAGN_LOGERR("ok\n");
		unsigned int	s,
				b;
		double	ll,
			ul;
		char l[256];
		while (fscanf(fsb, "%d %d %lf %lf %s", &s, &b, &ll, &ul, l) >= 5)
		{
			insertSpectralBand(TePDIMixModelSpectralBand(s, b, ll, ul, l));
		}
		fclose(fsb);
		return true;
	}
	else
		return false;
}

unsigned int TePDIMixModelSpectralBandList::getSize()
{
	return bands.size();
}

string TePDIMixModelSpectralBandList::getSpectralBandLabel(unsigned int b)
{
	return bands[b].getLabel();
}

TePDIMixModelSpectralBand TePDIMixModelSpectralBandList::getSpectralBand(unsigned int b)
{
	return bands[b];
}

bool TePDIMixModelSpectralBandList::operator==( 
  const TePDIMixModelSpectralBandList& external ) const
{
  return ( bands == external.bands );
}
