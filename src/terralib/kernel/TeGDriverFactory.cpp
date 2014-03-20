#include "TeGDriverFactory.h"
#include "TeUtils.h"

TeGDriverParams::TeGDriverParams(const string & fileType, const string & fileName) 
{
	fileType_ = fileType;
	fileName_ = fileName;
	writing_  = false;
}

string TeGDriverParams::getProductId() const 
{
	std::string extension = TeGetExtension(fileName_.c_str());
	if(extension.empty())
	{
		std::string factName = TeGDriverFactory::FindFactory(fileName_);
		if(factName.empty() == false)
		{
			return factName;
		}
	}
	else
	{
		extension = TeConvertToLowerCase(extension);
	
		const std::vector<TeGeoFileInfo>& geoFilesInfo = TeGDriverFactory::GetGeoFilesInfo();
		for(unsigned int i = 0; i < geoFilesInfo.size(); ++i)
		{
			const std::string& e = geoFilesInfo[i].getExtension();
			if(extension == e)
				return geoFilesInfo[i].getId();
		}
	}

	return fileType_; // not found! for while...
}

// Static geo files info that can be handled by TerraLib.
std::vector<TeGeoFileInfo> TeGDriverFactory::_geoFilesInfo;

TeGDriverFactory::TeGDriverFactory(const string & facName) :
TeAbstractFactory<TeGeoDataDriver, TeGDriverParams, string>(facName)
{}

TeGDriverFactory::~TeGDriverFactory() 
{}

const std::vector<TeGeoFileInfo>& TeGDriverFactory::GetGeoFilesInfo()
{
	if(!_geoFilesInfo.empty())
		return _geoFilesInfo;

	TeFactoryMap& facts = TeAbstractFactory<TeGeoDataDriver, TeGDriverParams, string>::instance();
	TeFactoryMap::iterator it;
	for(it = facts.begin(); it != facts.end(); ++it)
	{
		TeGDriverFactory* f = dynamic_cast<TeGDriverFactory*>(it->second);
		if(f == 0)
			continue;
		f->loadGeoFilesInfo();
	}

	return _geoFilesInfo;
}

std::string TeGDriverFactory::FindFactory(const std::string& fileName)
{
	TeFactoryMap& facts = TeAbstractFactory<TeGeoDataDriver, TeGDriverParams, string>::instance();
	TeFactoryMap::iterator it;
	for(it = facts.begin(); it != facts.end(); ++it)
	{
		TeGDriverFactory* f = dynamic_cast<TeGDriverFactory*>(it->second);
		if(f == 0)
			continue;
		if(f->acceptFile(fileName))
		{
			return it->first;
		}
	}

	return "";
}

bool TeGDriverFactory::acceptFile(const std::string& fileName)
{
	return false;
}