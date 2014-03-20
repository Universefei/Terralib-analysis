#include "TeDecoder.h"

TeDecoder::TeDecoder()
{

}

TeDecoder::TeDecoder(const TeRasterParams& par) : params_(par) 
{

}

TeDecoder::~TeDecoder()
{

}

TeRasterParams& TeDecoder::params()
{ 
	return params_; 
}

void TeDecoder::updateParams(TeRasterParams& par)
{	
	params_ = par; 
}

bool TeDecoder::setElementRGB(int col, int lin, double Rval, double Gval, double Bval, unsigned int)
{
	if (!setElement(col,lin,Rval,0))
		return false;
	if (!setElement(col,lin,Gval,1))
		return false;		
	if (!setElement(col,lin,Bval,2))
		return false;
	return true;
}

TeCoord2D TeDecoder::coord2Index (const TeCoord2D &pt)
{	
	return params_.coord2Index(pt);	
}

TeCoord2D TeDecoder::index2Coord (const TeCoord2D &pt)
{	
	return params_.index2Coord(pt);	
}

void TeDecoder::init( TeRasterParams& par )
{
	params_= par; init();
}

TeDecoder* TeDecoder::DefaultObject( const TeRasterParams& par)
{
	return TeDecoderFactory::loadDecoder(par);
}

bool TeDecoder::selectBlocks(TeBox& /* bb */, int /* resFac */, TeRasterParams& /*parBlock*/)  
{ 
	return false;
}

int TeDecoder::numberOfSelectedBlocks()
{
	return 0;
}	
	

bool TeDecoder::getSelectedRasterBlock(TeDecoderMemory* /* memDec */) 
{ 
	return false; 
}


void TeDecoder::clearBlockSelection()
{

}


int TeDecoder::bestResolution(TeBox& /*bb*/, int /*ncols*/, int /*nlines*/, TeProjection* /*proj*/)
{	
	return 1; 
}


int TeDecoder::bestResolution(double /*res*/)
{	
	return 1; 
}

TeDecoderFactory::TeDecoderFactory(const string& name) : TeFactory<TeDecoder,TeRasterParams>(name)
{
}

TeDecoderFactory::~TeDecoderFactory()
{

}

TeDecoder* TeDecoderFactory::loadDecoder(const TeRasterParams& params)
{
	TeFactoryMap& facts = TeFactory<TeDecoder, TeRasterParams>::instance();
	TeFactoryMap::iterator it;

	for(it = facts.begin(); it != facts.end(); ++it)
	{
		TeDecoderFactory* f = dynamic_cast<TeDecoderFactory*>(it->second);

		if(f == 0)
			continue;

		if(f->acceptFile(params.fileName_))
		{
			return  f->build(params);
		}
	}

	return NULL;
}

bool TeDecoderFactory::acceptFile(const std::string& fileName)
{
	return false;
}

