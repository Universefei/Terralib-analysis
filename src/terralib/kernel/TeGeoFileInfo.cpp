#include "TeGeoFileInfo.h"
#include "TeUtils.h"

// STL
#include <iostream>

TeGeoFileInfo::TeGeoFileInfo() :
_id(""),
_description(""),
_extension(""),
_hasCreateSupport(false)
{}

TeGeoFileInfo::~TeGeoFileInfo()
{}

const std::string& TeGeoFileInfo::getId() const
{
	return _id;
}

const std::string& TeGeoFileInfo::getDescription() const
{
	return _description;
}

const std::string& TeGeoFileInfo::getExtension() const
{
	return _extension;
}

bool TeGeoFileInfo::hasCreateSupport() const
{
	return _hasCreateSupport;
}

void TeGeoFileInfo::setId(const std::string& id)
{
	_id = id;
}

void TeGeoFileInfo::setDescription(const std::string& d)
{
	_description = d;
}

void TeGeoFileInfo::setExtension(const std::string& e)
{
	_extension = TeConvertToLowerCase(e);
}

void TeGeoFileInfo::setCreateSupport(bool b)
{
	_hasCreateSupport = b;
}

void TeGeoFileInfo::print() const
{
	std::cout << "- TerraLib Internal Driver Id: " << _id << std::endl;
	std::cout << "- Full Name: " << _description << std::endl;
	
	std::string s;
	_extension.empty() ? s = "empty" : s = _extension;
	std::cout << "- Associated extension: " << s << std::endl;
	
	_hasCreateSupport ? s = "yes" : s = "no";
	std::cout << "- Create support: " << s << std::endl;

	std::cout << "- File filter: " << getFilter() << "\n" << std::endl;
}

std::string TeGeoFileInfo::getFilter() const
{
	std::string filter = _description + " ";
	filter += "(";
	if(_extension.empty())
		filter += "*.*";
	else
		filter += "*." + _extension;
	filter += ")";

	return filter;
}
